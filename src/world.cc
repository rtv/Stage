/*
 *  Stage : a multi-robot simulator.
 *  Copyright (C) 2001, 2002 Richard Vaughan, Andrew Howard and Brian Gerkey.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */
/*
 * Desc: top level class that contains everything
 * Author: Richard Vaughan, Andrew Howard
 * Date: 7 Dec 2000
 * CVS info: $Id: world.cc,v 1.97 2002-06-09 00:33:02 inspectorg Exp $
 */

//#undef DEBUG
//#undef VERBOSE
//#define DEBUG 
//#define VERBOSE

#include <errno.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <signal.h>
#include <math.h>
#include <fstream.h>
#include <iostream.h>
#include <unistd.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pwd.h>
#include <stdio.h>
#include <libgen.h>  // for dirname
#include <netdb.h>

bool usage = false;

void PrintUsage(); // defined in main.cc
void StageQuit();

long int g_bytes_output = 0;
long int g_bytes_input = 0;

int g_timer_expired = 0;

#include "world.hh"
#include "fixedobstacle.hh"

// allocate chunks of 32 pointers for entity storage
const int OBJECT_ALLOC_SIZE = 32;

#define WATCH_RATES

// dummy timer signal func
void TimerHandler( int val )
{
  g_timer_expired++;
  //printf( "\ng_timer_expired: %d\n", g_timer_expired );
}  

// Main.cc calls constructor, then Load(), then Startup(),
// Main(), then Shutdown()

///////////////////////////////////////////////////////////////////////////
// Default constructor
CWorld::CWorld( int argc, char** argv )
{
  // seed the random number generator
  srand48( time(NULL) );

  // Initialise configuration variables
  this->ppm = 20;

  // matrix is created by a StageIO object
  this->matrix = NULL;

  // stop time of zero means run forever
  m_stoptime = 0;
  
  m_clock = 0;

  // invalid file descriptor initially
  m_log_fd = -1;

  m_external_sync_required = false;
  m_instance = 0;

  // AddEntity() handles allocation of storage for entities
  // just initialize stuff here
  m_entity = NULL;
  m_entity_alloc = 0;
  m_entity_count = 0;

  //rtp_player = NULL;

  m_log_output = false;
  m_console_output = true;
    
  // real time mode by default
  // if real_timestep is zero, we run as fast as possible
  m_real_timestep = 0.1; //seconds
  m_sim_timestep = 0.1; //seconds; - 10Hz default rate 
  m_step_num = 0;

  // Allow the simulation to run
  //
  m_enable = true;

  if( gethostname( m_hostname, sizeof(m_hostname)) == -1)
  {
    perror( "Stage: couldn't get hostname. Quitting." );
    exit( -1 );
  }

  /* now strip off domain */
  char* first_dot;
  strncpy(m_hostname_short, m_hostname,HOSTNAME_SIZE);
  if( (first_dot = strchr(m_hostname_short,'.') ))
    *first_dot = '\0';
      
  // get the IP of our host
  struct hostent* info = gethostbyname( m_hostname );
  
  if( info )
  { // make sure this looks like a regular internet address
    assert( info->h_length == 4 );
    assert( info->h_addrtype == AF_INET );
      
    // copy the address out
    memcpy( &m_hostaddr.s_addr, info->h_addr_list[0], 4 ); 
  }
  else
  {
    PRINT_ERR1( "failed to resolve IP for local hostname \"%s\"\n", 
                m_hostname );
  }
  
  // just to be reassuring, print the host details
  printf( "[Host %s(%s)]",
          m_hostname_short, inet_ntoa( m_hostaddr ) );
  
  m_send_idar_packets = false;

  // Run the gui by default
  this->enable_gui = true;

  // default color database file
  strcpy( m_color_database_filename, COLOR_DATABASE );

  // Initialise clocks
  m_start_time = m_sim_time = 0;
  memset( &m_sim_timeval, 0, sizeof( struct timeval ) );
    
  // Initialise entity list
  m_entity_count = 0;
  
  // start with no key
  bzero(m_auth_key,sizeof(m_auth_key));
 
  // give the command line a chance to override the default values
  // we just set
  ParseCmdline( argc, argv );
}


///////////////////////////////////////////////////////////////////////////
// Destructor
CWorld::~CWorld()
{
  if (wall)
    delete wall;

  if( matrix )
    delete matrix;

  for( int m=0; m < m_entity_count; m++ )
    delete m_entity[m];
}

int CWorld::CountDirtyOnConnection( int con )
{
  char dummydata[MAX_PROPERTY_DATA_LEN];
  int count = 0;

  //puts( "Counting dirty properties" );
  // count the number of dirty properties on this connection 
  for( int i=0; i < GetEntityCount(); i++ )
    for( int p=0; p < MAX_NUM_PROPERTIES; p++ )
    {  
      // is the entity marked dirty for this connection & property?
      if( GetEntity(i)->m_dirty[con][p] ) 
      {
        // if this property has any data  
        if( GetEntity(i)->GetProperty( (EntityProperty)p, dummydata ) > 0 )
          count++; // we count it as dirty
      }
    }

  return count;
}

///////////////////////////////////////////////////////////////////////////
// Parse the command line
bool CWorld::ParseCmdline(int argc, char **argv)
{
  for( int a=1; a<argc-1; a++ )
  {
    // USAGE
    if( (strcmp( argv[a], "-?" ) == 0) || 
        (strcmp( argv[a], "--help") == 0) )
    {
      PrintUsage();
      return false;
    }
      
    // LOGGING
    if( strcmp( argv[a], "-l" ) == 0 )
    {
      m_log_output = true;
      strncpy( m_log_filename, argv[a+1], 255 );
      printf( "[Logfile %s]", m_log_filename );
	  
      //store the command line for logging later
      memset( m_cmdline, 0, sizeof(m_cmdline) );
	  
      for( int g=0; g<argc; g++ )
	    {
	      strcat( m_cmdline, argv[g] );
	      strcat( m_cmdline, " " );
	    }
	  
      a++;
	  
      // open the log file and write out a header
      LogOutputHeader();
    }
      
    // DIS/ENABLE GUI
    if( strcmp( argv[a], "-g" ) == 0 )
    {
      this->enable_gui = false;
      printf( "[No GUI]" );
    }
    else if( strcmp( argv[a], "+g" ) == 0 )
    {
      this->enable_gui = true;
      printf( "[GUI]" );
    }
      
      
    // SET GOAL REAL CYCLE TIME
    // Stage will attempt to update at this speed
    else if( strcmp( argv[a], "-u" ) == 0 )
    {
      m_real_timestep = atof(argv[a+1]);
      printf( "[Real time per cycle %f sec]", m_real_timestep );
      a++;
    }
      
    // SET SIMULATED UPDATE CYCLE
    // one cycle simulates this much time
    else if( strcmp( argv[a], "-v" ) == 0 )
    {
      m_sim_timestep = atof(argv[a+1]);
      printf( "[Simulated time per cycle %f sec]", m_sim_timestep );
      a++;
    }
      
    // DISABLE console output
    if( strcmp( argv[a], "-q" ) == 0 )
    {
      m_console_output = false;
      printf( "[Quiet]" );
    }
      

    // change the server port 
    //else if( strcmp( argv[a], "-p" ) == 0 )
    //{
    //  m_pose_port = atoi(argv[a+1]);
    //  printf( "[Port %d]", m_pose_port );
    //  a++;
    //}

    // SWITCH ON SYNCHRONIZED (distributed) MODE
    // if this option is given, Stage will only run when connected
    // to an external synchronous pose connection
    else if( strcmp( argv[a], "-s" ) == 0 )
    {
      m_external_sync_required = true; 
      m_enable = false; // don't run until we have a sync connection
      printf( "[External Sync]");
    }      

    else if(!strcmp(argv[a], "-time"))
    {
      m_stoptime = atoi(argv[++a]);
      printf("setting time to: %d\n",m_stoptime);
    }

    // ENABLE RTP - sensor data is sent in rtp format
    // if( (strcmp( argv[a], "-r" ) == 0 ) || 
    // ( strcmp( argv[a], "--rtp" ) == 0 ))
    //  	{
    //  	  rtp_player = new CRTPPlayer( argv[a+1] );
      
    //  	  printf( "World rtp player @ %p\n", rtp_player );
      
    //  	  printf( "[RTP %s]", argv[a+1] );
    //  	  a++;
    //  	}
    // ENABLE IDAR packets to be sent to XS
    //if( strcmp( argv[a], "-i" ) == 0 )
    //{
    //  m_send_idar_packets = true;
    //  printf( "[IDAR->XS]" );
    //}
      

    //else if( strcmp( argv[a], "-id" ) == 0 )
    //{
    //  memset( m_hostname, 0, 64 );
    //  strncpy( m_hostname, argv[a+1], 64 );
    //  printf( "[ID %s]", m_hostname ); fflush( stdout );
    //  a++;
    //}
  }
    
  return true;
}


///////////////////////////////////////////////////////////////////////////
// Startup routine 
bool CWorld::Startup()
{  
  PRINT_DEBUG( "** STARTUP **" );
  
  // we must have at least one entity to play with!
  assert( m_entity_count > 0 );
  
  // Initialise the real time clock
  // Note that we really do need to set the start time to zero first!
  m_start_time = 0;
  m_start_time = GetRealTime();
    
  // Initialise the rate counter
  m_update_ratio = 1;
  m_update_rate = 0;

  // Start the GUI
  if (this->enable_gui)
    RtkStartup();
  
  // start the real-time interrupts going
  StartTimer( m_real_timestep );
  
  PRINT_DEBUG( "** STARTUP DONE **" );
  return true;
}
    

///////////////////////////////////////////////////////////////////////////
// Shutdown routine 
void CWorld::Shutdown()
{
  PRINT_DEBUG( "world shutting down" );

  // Stop the GUI
  if (this->enable_gui)
    RtkShutdown();
  
  // Shutdown all the entities
  // Devices will unlink their device files
  for (int i = 0; i < m_entity_count; i++)
  {
    if(m_entity[i])
      m_entity[i]->Shutdown();
  }
  
  // Shutdown the wall
  if(this->wall)
    this->wall->Shutdown();
}




void CWorld::StartTimer( double interval )
{
  // set up the interval timer
  //
  // set a timer to go off every few ms. in realtime mode we'll sleep
  // in between if there's nothing else to do. 

  //install signal handler for timing
  if( signal( SIGALRM, &TimerHandler ) == SIG_ERR )
  {
    PRINT_ERR("failed to install signal handler");
    exit( -1 );
  }

  //start timer with chosen interval (specified in milliseconds)
  struct itimerval tick;
  // seconds
  tick.it_value.tv_sec = tick.it_interval.tv_sec = (long)floor(interval);
  // microseconds
  tick.it_value.tv_usec = tick.it_interval.tv_usec = 
    (long)fmod( interval * MILLION, MILLION); 
  
  if( setitimer( ITIMER_REAL, &tick, 0 ) == -1 )
  {
    PRINT_ERR("failed to set timer");
    exit( -1 );
  }
}


///////////////////////////////////////////////////////////////////////////
// Update the world
void CWorld::Update(void)
{
  //PRINT_DEBUG( "** Update **" );
  //assert( arg == 0 );
  
  static double loop_start = GetRealTime();

  // Set the timer flag depending on the current mode.
  // If we're NOT in realtime mode, the timer is ALWAYS expired
  // so we run as fast as possible
  m_real_timestep > 0.0 ? g_timer_expired = 0 : g_timer_expired = 1;
            
  // calculate new world state
  if (m_enable) 
  {
    // Update the simulation time (in both formats)
    m_sim_time = m_step_num * m_sim_timestep;
    m_sim_timeval.tv_sec = (long)floor(m_sim_time);
    m_sim_timeval.tv_usec = (long)((m_sim_time - floor(m_sim_time)) * MILLION); 
    // is it time to stop?
    if(m_stoptime && m_sim_time >= m_stoptime)
      system("kill `cat stage.pid`");
      
    // copy the timeval into the player io buffer. use the first
    // entity's info

    if( m_clock ) // if we're managing a clock
    {
      // TODO - turn the player clock back on - move this into the server?
      sem_wait( &m_clock->lock );
      m_clock->time = m_sim_timeval;
      sem_post( &m_clock->lock );
    }

    // Do the actual work -- update the entities 
    for (int i = 0; i < m_entity_count; i++)
    {
      // if this host manages this entity
      if( m_entity[i]->m_local )
        m_entity[i]->Update( m_sim_time ); // update the device model
	  
#ifdef INCLUDE_RTK2
      // update the GUI, whether we manage this device or not
      if (this->enable_gui)
        m_entity[i]->RtkUpdate();
#endif
    };
#ifdef INCLUDE_RTK2
    if (this->enable_gui)
      RtkUpdate();      
#endif
  }
  else // the model isn't running - update the GUI and go to sleep
  {
#ifdef INCLUDE_RTK2
    if (this->enable_gui) 
      RtkUpdate();      
#endif

    PRINT_DEBUG( "SLEEPING - DISABLED" );
    usleep( 100000 ); // stops us hogging the machine while we're paused
  }
  
  // for logging statistics
  double update_time = GetRealTime();
  // for logging statistics
  double loop_end = GetRealTime(); 
  double loop_time = loop_end - loop_start;
  double sleep_time = loop_end - update_time;
      
  // set this here so we're timing the output time too.
  loop_start = GetRealTime();
      
  // generate console and logfile output
  if( m_enable ) Output( loop_time, sleep_time );

  // dump the contents of the matrix to a file
  //world->matrix->dump();
  //getchar();	
}


///////////////////////////////////////////////////////////////////////////
// Get the sim time
// Returns time in sec since simulation started
double CWorld::GetTime()
{
  return m_sim_time;
}


///////////////////////////////////////////////////////////////////////////
// Get the real time
// Returns time in sec since simulation started
double CWorld::GetRealTime()
{
  struct timeval tv;
  gettimeofday( &tv, NULL );
  double time = tv.tv_sec + (tv.tv_usec / 1000000.0);
  return time - m_start_time;
}


///////////////////////////////////////////////////////////////////////////
// Set a rectangle in the world grid
void CWorld::SetRectangle(double px, double py, double pth,
                          double dx, double dy, CEntity* ent, bool add)
{
  Rect rect;

  dx /= 2.0;
  dy /= 2.0;

  double cx = dx * cos(pth);
  double cy = dy * cos(pth);
  double sx = dx * sin(pth);
  double sy = dy * sin(pth);
    
  rect.toplx = (int) ((px + cx - sy) * ppm);
  rect.toply = (int) ((py + sx + cy) * ppm);

  rect.toprx = (int) ((px + cx + sy) * ppm);
  rect.topry = (int) ((py + sx - cy) * ppm);

  rect.botlx = (int) ((px - cx - sy) * ppm);
  rect.botly = (int) ((py - sx + cy) * ppm);

  rect.botrx = (int) ((px - cx + sy) * ppm);
  rect.botry = (int) ((py - sx - cy) * ppm);
    
  matrix->draw_rect( rect, ent, add );
}


///////////////////////////////////////////////////////////////////////////
// Set a circle in the world grid
void CWorld::SetCircle(double px, double py, double pr, CEntity* ent,
                       bool add )
{
  // Convert from world to image coords
  int x = (int) (px * ppm);
  int y = (int) (py * ppm);
  int r = (int) (pr * ppm);
    
  matrix->draw_circle( x,y,r,ent, add);
}


///////////////////////////////////////////////////////////////////////////
// Add an entity to the world
//
void CWorld::AddEntity(CEntity *entity)
{
  // if we've run out of space (or we never allocated any)
  if( (!m_entity) || (!( m_entity_count < m_entity_alloc )) )
  {
    // allocate some more
    CEntity** more_space = new CEntity*[ m_entity_alloc + OBJECT_ALLOC_SIZE ];
      
    // copy the old data into the new space
    memcpy( more_space, m_entity, m_entity_count * sizeof( CEntity* ) );
     
    // delete the original
    if( m_entity ) delete [] m_entity;
      
    // bring in the new
    m_entity = more_space;
 
    // record the total amount of space
    m_entity_alloc += OBJECT_ALLOC_SIZE;
  }
  
  // insert the entity and increment the count
  m_entity[m_entity_count++] = entity;
}


// returns true if the given hostname matches our hostname, false otherwise
//  bool CWorld::CheckHostname(char* host)
//  {
//    //printf( "checking %s against (%s and %s) ", 
//    //  host, m_hostname, m_hostname_short ); 

//    if(!strcmp(m_hostname,host) || !strcmp(m_hostname_short,host))
//    {
//      //PRINT_DEBUG( "TRUE" );
//      return true;
//    }
//    else
//    {
//      //PRINT_DEBUG( "FALSE" );
//      return false;
//    }
//  }


void CWorld::Output( double loop_duration, double sleep_duration )
{
  // time taken
  double gain = 0.1;
  
  static double avg_loop_duration = m_real_timestep;
  static double avg_sleep_duration = m_real_timestep;
  avg_loop_duration = (1.0-gain)*avg_loop_duration + gain*loop_duration;
  avg_sleep_duration = (1.0-gain)*avg_sleep_duration + gain*sleep_duration;

  // comms used
  static unsigned long last_input = 0;
  static unsigned long last_output = 0;
  unsigned int bytes_in = g_bytes_input - last_input;
  unsigned int bytes_out = g_bytes_output - last_output;
  
  // measure frequency & bandwidth
  static double freq = 0.0;
  static double bandw = 0.0;

  static int updates = 0;
  static int bytes = 0;
  static double lasttime = GetRealTime();
  double interval = GetRealTime() - lasttime;

  // count this update
  updates++;
  
  // count the data
  bytes += bytes_in + bytes_out;

  if( interval > 1.0 )
  {
    lasttime += interval;

    freq = (double)updates / interval;
    bandw = (double)bytes / interval;
      
    updates = 0;
    bytes = 0;
  }

  //ConsoleOutput( freq, loop_duration, sleep_duration, 
  //             avg_loop_duration, avg_sleep_duration,
  //             bytes_in, bytes_out, bandw );

  
  if( m_log_output ) 
    LogOutput( freq, loop_duration, sleep_duration, 
               avg_loop_duration, avg_sleep_duration,
               bytes_in, bytes_out, g_bytes_input, g_bytes_output );
  
  last_input = g_bytes_input;
  last_output = g_bytes_output; 
}

void CWorld::ConsoleOutput( double freq, 
			    double loop_duration, double sleep_duration,
			    double avg_loop_duration, 
			    double avg_sleep_duration,
			    unsigned int bytes_in, unsigned int bytes_out,
			    double avg_data)
{
  printf( " Time: %8.1f - %7.1fHz - "
          "[%3.0f/%3.0f] [%3.0f/%3.0f] [%4u/%4u] %8.2f b/sec\r", 
          m_sim_time, 
          freq,
          loop_duration * 1000.0, 
          sleep_duration * 1000.0, 
          avg_loop_duration * 1000.0, 
          avg_sleep_duration * 1000.0,  
          bytes_in, bytes_out, 
          avg_data );
  
  fflush( stdout );
  
}


void CWorld::LogOutput( double freq,
			double loop_duration, double sleep_duration,
			double avg_loop_duration, double avg_sleep_duration,
			unsigned int bytes_in, unsigned int bytes_out, 
			unsigned int total_bytes_in, 
			unsigned int total_bytes_out )
{  
  assert( m_log_fd > 0 );
  
  char line[512];
  sprintf( line,
           "%u\t\t%.3f\t\t%.6f\t%.6f\t%.6f\t%u\t%u\t%u\t%u\n", 
           m_step_num, m_sim_time, // step and time
           loop_duration, // real cycle time in ms
           sleep_duration, // real sleep time in ms
           m_sim_timestep / loop_duration, // ratio
           bytes_in, // bytes in this cycle
           bytes_out, // bytes out this cycle
           total_bytes_in,  // total bytes in
           total_bytes_out); // total bytes out
  
  write( m_log_fd, line, strlen(line) );
}


void CWorld::LogOutputHeader( void )  
{
  int log_instance = 0;
  while( m_log_fd < 0 )
	{
	  char fname[256];
	  sprintf( fname, "%s.%d", m_log_filename, log_instance++ );
	  m_log_fd = open( fname, O_CREAT | O_EXCL | O_WRONLY, 
                     S_IREAD | S_IWRITE );
	}

  struct timeval t;
  gettimeofday( &t, 0 );
      
  // count the locally managed entities
  int m=0;
  for( int c=0; c<m_entity_count; c++ )
    if( m_entity[c]->m_local ) m++;
      
  char* tmstr = ctime( &t.tv_sec);
  tmstr[ strlen(tmstr)-1 ] = 0; // delete the newline
      
  char line[512];
  sprintf( line,
           "# Stage output log\n#\n"
           "# Command:\t%s\n"
           "# Date:\t\t%s\n"
           "# Host:\t\t%s\n"
           //"# Bitmap:\t%s\n"
           "# Timestep(ms):\t%d\n"
           "# Entities:\t%d of %d\n#\n"
           "#STEP\t\tSIMTIME(s)\tINTERVAL(s)\tSLEEP(s)\tRATIO\t"
           "\tINPUT\tOUTPUT\tITOTAL\tOTOTAL\n",
           m_cmdline, 
           tmstr, 
           m_hostname, 
           //worldfilename,
           (int)(m_sim_timestep * 1000.0),
           m, 
           m_entity_count );
      
  write( m_log_fd, line, strlen(line) );
}

char* CWorld::StringType( StageType t )
{
  switch( t )
  {
    case NullType: return "None"; 
    case WallType: return "Wall"; break;
    case PlayerType: return "Player"; 
    case MiscType: return "Misc"; 
    case PositionType: return "Position"; 
    case SonarType: return "Sonar"; 
    case LaserTurretType: return "Laser"; 
    case VisionType: return "Vision"; 
    case PtzType: return "PTZ"; 
    case BoxType: return "Box"; 
    case LaserBeaconType: return "LaserBcn"; 
    case LBDType: return "LBD"; 
    case VisionBeaconType: return "VisionBcn"; 
    case GripperType: return "Gripper"; 
    case AudioType: return "Audio"; 
    case BroadcastType: return "Bcast"; 
    case SpeechType: return "Speech"; 
    case TruthType: return "Truth"; 
    case GpsType: return "GPS"; 
    case PuckType: return "Puck"; 
    case OccupancyType: return "Occupancy"; 
    case IDARType: return "IDAR";
  }	 
  return( "unknown" );
}

#ifdef INCLUDE_RTK2

// Initialise the GUI
// TODO: fix this for client/server operation.
bool CWorld::RtkLoad(CWorldFile *worldfile)
{
  int sx, sy;
  double scale, dx, dy;
  double ox, oy;
  double minor, major;
  
  if (worldfile != NULL)
  {
    int section = worldfile->LookupEntity("rtk");

    // Size of canvas in pixels
    sx = (int) worldfile->ReadTupleFloat(section, "size", 0, this->matrix->width);
    sy = (int) worldfile->ReadTupleFloat(section, "size", 1, this->matrix->height);
  
    // Scale of the pixels
    scale = worldfile->ReadLength(section, "scale", 1 / this->ppm);
  
    // Size in meters
    dx = sx * scale;
    dy = sy * scale;

    // Origin of the canvas
    ox = worldfile->ReadTupleLength(section, "origin", 0, dx / 2);
    oy = worldfile->ReadTupleLength(section, "origin", 1, dy / 2);

    // Grid spacing
    minor = worldfile->ReadTupleLength(section, "grid", 0, 0.2);
    major = worldfile->ReadTupleLength(section, "grid", 1, 1.0);
  }
  else
  {
    // Size of canvas in pixels
    sx = (int)this->matrix->width;
    sy = (int)this->matrix->height;
  
    // Scale of the pixels
    scale = ((CFixedObstacle*)this->wall)->scale;
  
    // Size in meters
    dx = sx * scale;
    dy = sy * scale;

    // Origin of the canvas
    ox = dx / 2;
    oy = dy / 2;

    // Grid spacing
    minor = 0.2;
    major = 1.0;
  }
  
  this->app = rtk_app_create();
  rtk_app_refresh_rate(this->app, 10);
  
  this->canvas = rtk_canvas_create(this->app);
  rtk_canvas_size(this->canvas, sx, sy);
  rtk_canvas_scale(this->canvas, scale, scale);
  rtk_canvas_origin(this->canvas, ox, oy);

  // Add some menu items
  this->file_menu = rtk_menu_create(this->canvas, "File");
  this->save_menuitem = rtk_menuitem_create(this->file_menu, "Save", 0);
  this->export_menuitem = rtk_menuitem_create(this->file_menu, "Export", 0);
  this->exit_menuitem = rtk_menuitem_create(this->file_menu, "Exit", 0);
  this->export_count = 0;

  // Create the view menu
  this->view_menu = rtk_menu_create(this->canvas, "View");
  this->grid_item = rtk_menuitem_create(this->view_menu, "Grid", 1);

  // Set the initial view menu state
  rtk_menuitem_check(this->grid_item, 1);

  // Create the grid
  this->fig_grid = rtk_fig_create(this->canvas, NULL, -49);
  if (minor > 0)
  {
    rtk_fig_color(this->fig_grid, 0.9, 0.9, 0.9);
    rtk_fig_grid(this->fig_grid, ox, oy, dx, dy, minor);
  }
  if (major > 0)
  {
    rtk_fig_color(this->fig_grid, 0.75, 0.75, 0.75);
    rtk_fig_grid(this->fig_grid, ox, oy, dx, dy, major);
  }

  return true;
}

// Save the GUI
bool CWorld::RtkSave(CWorldFile *worldfile)
{
  int section = worldfile->LookupEntity("rtk");

  // Size of canvas in pixels
  int sx, sy;
  rtk_canvas_get_size(this->canvas, &sx, &sy);
  worldfile->WriteTupleFloat(section, "size", 0, sx);
  worldfile->WriteTupleFloat(section, "size", 1, sy);

  // Origin of the canvas
  double ox, oy;
  rtk_canvas_get_origin(this->canvas, &ox, &oy);
  worldfile->WriteTupleLength(section, "origin", 0, ox);
  worldfile->WriteTupleLength(section, "origin", 1, oy);

  // Scale of the canvas
  double scale;
  rtk_canvas_get_scale(this->canvas, &scale, &scale);
  worldfile->WriteLength(section, "scale", scale);
  
  return true;
}


// Start the GUI
bool CWorld::RtkStartup()
{
  PRINT_DEBUG( "** STARTUP GUI **" );

  rtk_app_start(this->app);
  return true;
}


// Stop the GUI
void CWorld::RtkShutdown()
{
  rtk_app_stop(this->app);
}


// Update the GUI
void CWorld::RtkUpdate()
{
  // See if we need to quit the program
  if (rtk_menuitem_isactivated(this->exit_menuitem))
    ::quit = 1;
  if (rtk_canvas_isclosed(this->canvas))
    ::quit = 1;

  // Save the world file
  if (rtk_menuitem_isactivated(this->save_menuitem))
    SaveFile(NULL);

  // Handle export menu item
  // TODO - fold in XS's postscript and pnm export here
  if (rtk_menuitem_isactivated(this->export_menuitem))
  {
    char filename[128];
    snprintf(filename, sizeof(filename), 
             "rtkstage-%04d.fig", this->export_count++);
    PRINT_MSG1("exporting canvas to [%s]", filename);
    rtk_canvas_export(this->canvas, filename);
  }

  // Show or hide the grid
  if (rtk_menuitem_ischecked(this->grid_item))
    rtk_fig_show(this->fig_grid, 1);
  else
    rtk_fig_show(this->fig_grid, 0);
}
#endif


