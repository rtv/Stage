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
 * CVS info: $Id: world.cc,v 1.129 2002-10-25 22:48:09 rtv Exp $
 */
#if HAVE_CONFIG_H
  #include <config.h>
#endif
#if HAVE_STRINGS_H
  #include <strings.h>
#endif

//#undef DEBUG
//#undef VERBOSE
//#define DEBUG 
//#define VERBOSE

#include <errno.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <signal.h>
#include <math.h>
#include <unistd.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pwd.h>
#include <stdio.h>
#include <netdb.h>

#include <fstream>
#include <iostream>

#include "world.hh"
#include "playerdevice.hh"
#include "library.hh"
#include "gui.hh"
#include "bitmap.hh"

bool usage = false;

void PrintUsage(); // defined in main.cc
void StageQuit();

long int g_bytes_output = 0;
long int g_bytes_input = 0;

int g_timer_events = 0;

// allocate chunks of 32 pointers for entity storage
const int OBJECT_ALLOC_SIZE = 32;

// dummy timer signal func
void TimerHandler( int val )
{
  //puts( "TIMER HANDLER" );
  g_timer_events++;

  // re-install signal handler for timing
  if( signal( SIGALRM, &TimerHandler ) == SIG_ERR )
    {
      PRINT_ERR("failed to install signal handler");
      exit( -1 );
    }

  //printf( "\ng_timer_expired: %d\n", g_timer_expired );
}  

// static member init
CEntity* CWorld::root = NULL;

///////////////////////////////////////////////////////////////////////////
// Default constructor
CWorld::CWorld( int argc, char** argv, Library* lib )
{
  // store the params locally
  this->argc = argc;
  this->argv = argv;
  this->lib = lib;

  // seed the random number generator
#if HAVE_SRAND48
  srand48( time(NULL) );
#endif

  // Initialise configuration variables
  this->ppm = 20;

  // matrix is created by a StageIO object
  this->matrix = NULL;

  // if we are a server, this gets set in the server's constructor
  this->worldfile = NULL;

  // stop time of zero means run forever
  m_stoptime = 0;
  
  // invalid file descriptor initially
  m_log_fd = -1;

  m_instance = 0;

  // just initialize stuff here

  this->entities = NULL;
  this->entity_count = 0;
  this->entities_size = 0;

  rtp_player = NULL;

  m_log_output = false; // enable with -l <filename>
  m_console_output = false; // enable with -o
    
  // real time mode by default
  // if real_timestep is zero, we run as fast as possible
  m_real_timestep = 0.1; //seconds
  m_sim_timestep = 0.1; //seconds; - 10Hz default rate 
  m_step_num = 0;

  // start paused
  //
  m_enable = false;

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
  
  // Run the gui by default
  this->enable_gui = true;

  // default color database file
  strcpy( m_color_database_filename, COLOR_DATABASE );

  // Initialise clocks
  m_start_time = m_sim_time = 0;
  memset( &m_sim_timeval, 0, sizeof( struct timeval ) );
    
  // Initialise entity list
  this->entity_count = 0;
  
  // start with no key
  bzero(m_auth_key,sizeof(m_auth_key));
 

  assert(lib);
  //lib->Print();


  // Construct a fixed obstacle representing the boundary of the 
  // the environment - this the root for all other entities
  assert( root = (CEntity*)new CBitmap(this, NULL) );

  // give the GUI a go at the command line too
  if( enable_gui )  GuiInit( argc, argv ); 
 
}


///////////////////////////////////////////////////////////////////////////
// Destructor
CWorld::~CWorld()
{
  if( matrix )
    delete matrix;

  if( root )
    delete root;
}

///////////////////////////////////////////////////////////////////////////
// Parse the command line
bool CWorld::ParseCmdLine(int argc, char **argv)
{
  for( int a=1; a<argc; a++ )
    {   
      // USAGE
      if( (strcmp( argv[a], "-?" ) == 0) || 
	  (strcmp( argv[a], "--help") == 0) )
	{
	  PrintUsage();
	  exit(0); // bail right here
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
    
      // SET GOAL REAL CYCLE TIME
      // Stage will attempt to update at this speed
      if( strcmp( argv[a], "-u" ) == 0 )
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
      if( strcmp( argv[a], "-o" ) == 0 )
	{
	  m_console_output = true;
	  printf( "[Console Output]" );
	}
      
      if(!strcmp(argv[a], "-t"))
	{
	  m_stoptime = atoi(argv[++a]);
	  printf("[Stop time: %d]",m_stoptime);
	}

      // ENABLE RTP - sensor data is sent in rtp format
      if( (strcmp( argv[a], "-r" ) == 0 ) || 
	  ( strcmp( argv[a], "--rtp" ) == 0 ))
	{
	  assert( rtp_player = new CRTPPlayer( argv[a+1] ) );
	
	  //printf( "World rtp player @ %p\n", rtp_player );
	  
      	  printf( "[RTP %s]", argv[a+1] );
      	  a++;
	  

	}
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
  
  // give the command line a chance to override the default values
  // we just set
  if( !ParseCmdLine( this->argc, this->argv )) 
    {
      quit = true;
      return false;
    }
  
  // we must have at least one entity to play with!
  // they should have been created by server or client before here
  if( this->entity_count < 1 )
    {
      puts( "\nStage: No entities defined in world file. Nothing to simulate!" );
      return false;
    }

  // Initialise the real time clock
  // Note that we really do need to set the start time to zero first!
  m_start_time = 0;
  m_start_time = GetRealTime();
    
  // Initialise the rate counter
  m_update_ratio = 1;
  m_update_rate = 0;
    
  if( m_real_timestep > 0.0 ) // if we're in real-time mode
    StartTimer( m_real_timestep ); // start the real-time interrupts going
  
#ifdef DEBUG
  //root->Print( "" );
#endif

  // use the generic hook to start the GUI
  if( this->enable_gui ) GuiWorldStartup( this );


  // Startup all the entities. they will create and initialize their
  // device files and Gui stuff
  if( !root->Startup() )
    {
      PRINT_ERR("Root Entity startup failed" );
      quit = true;
      return false;
    }


  PRINT_DEBUG( "** STARTUP DONE **" );
  return true;
}
    

///////////////////////////////////////////////////////////////////////////
// Shutdown routine 
bool CWorld::Shutdown()
{
  PRINT_DEBUG( "world shutting down" );
  
  // use the hook to shutdown the GUI
  if( this->enable_gui ) GuiWorldShutdown( this );
  
  // Shutdown all the entities
  // Devices will unlink their device files
  root->Shutdown();

  return true;
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

  //printf( "interval: %f\n", interval );

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
  
  //while( !quit )
    
      // if the sim isn't running, we pause briefly and return
      if( !m_enable )
	{
	  usleep( 100000 );
	  return;
	}

      // is it time to stop?
      if(m_stoptime && m_sim_time >= m_stoptime)
	{   
	  //system("kill `cat stage.pid`");
	  quit = true;
	  return;
	}
      
      // otherwise we're running - calculate new world state
      
      // let the entities do anything they want to do between clock increments
      root->Sync(); 
      
      // if the timer has gone off recently or we're in fast mode
      // we increment the clock and do the time-based updates
      if( g_timer_events > 0 || m_real_timestep == 0 )
	{          
	  // set the current time and
	  // update the entities managed by this host at this time 
	  root->Update(this->SetClock( m_real_timestep, m_step_num ));
	  
	  // all the entities are done, now let's draw the graphics
	  if( this->enable_gui )
	    GuiWorldUpdate( this );
	  
	  if( g_timer_events > 0 )
	    g_timer_events--; // we've handled this timer event
	  
	  // increase the time step counter
	  m_step_num++; 

	}
      
      Output(); // perform console and log output
      
      // if there's nothing pending and we're not in fast mode, we let go
      // of the processor (on linux gives us around a 10ms cycle time)
      if( g_timer_events < 1 && m_real_timestep > 0.0 ) 
	usleep( 0 );
      
      // dump the contents of the matrix to a file for debugging
      //world->matrix->dump();
      //getchar();	
}


double CWorld::SetClock( double interval, uint32_t step_num )
{	  
  // Update the simulation time (in both formats)
  m_sim_time = step_num * interval;
  m_sim_timeval.tv_sec = (long)floor(m_sim_time);
  m_sim_timeval.tv_usec = (long)((m_sim_time-floor(m_sim_time)) * MILLION); 

  return m_sim_time;
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
    
  //printf( "draw_rect %d,%d %d,%d %d,%d %d,%d\n",
  //  rect.toplx, rect.toply,
  //  rect.toprx, rect.topry,
  //  rect.botlx, rect.botly,
  //  rect.botrx, rect.botry );

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
// Add an entity to the array
// returns its array index which the entity uses as a unique ID
stage_id_t CWorld::RegisterEntity( CEntity* ent )
{
  // if we have no space left in the array, allocate another chunk
  if (this->entity_count >= this->entities_size)
    { 
      this->entities_size += 100; // a good chunk - one realloc will do for most people
      this->entities = (CEntity**)
	realloc(this->entities, this->entities_size * sizeof(this->entities[0]));
    }
  
  stage_id_t id = entity_count++; // record the id BEFORE incrementing it
  
  this->entities[id] = ent; // store the pointer
  
  return id; // return the unique id/index
}

// return the entity with matching id/index, or NULL if no match
CEntity* CWorld::GetEntity( int id )
{ 
  // bounds check
  if( id < 0 || id >= this->entity_count )
    return NULL;
  
  return( this->entities[id] );
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


// sleep until a signal goes off
// return the time in seconds we spent asleep
double CWorld::Pause()
{
  // we're too busy to sleep!
  if( m_real_timestep == 0 || --g_timer_events > 0  )
    return 0;
  
  // otherwise

  double sleep_start = GetRealTime();

  pause(); // wait for the signal

  return( GetRealTime() - sleep_start );
}


void CWorld::Output()
{
  // comms used
  static unsigned long last_input = 0;
  static unsigned long last_output = 0;
  unsigned int bytes_in = g_bytes_input - last_input;
  unsigned int bytes_out = g_bytes_output - last_output;
  static int bytes_accumulator = 0;
  
  // count the data
  bytes_accumulator += bytes_in + bytes_out;

  // measure frequency & bandwidth
  static double freq = 0.0;
  static double bandw = 0.0;

  static int updates = 0;
  static double lasttime = GetRealTime();
  double interval = GetRealTime() - lasttime;

  // count this update
  updates++;
  
  if( interval > 2.0 ) // measure freq + bandwidth every 2 seconds
    {
      lasttime += interval;

      bandw = (double)bytes_accumulator / interval;
      bytes_accumulator = 0;
      
      freq = (double)updates / interval;
      updates = 0;    
    }
  
  if( m_console_output )
    ConsoleOutput( freq, bytes_in, bytes_out, bandw );
  
  
  if( m_log_output ) 
    LogOutput( freq, bytes_in, bytes_out, g_bytes_input, g_bytes_output );
  
  last_input = g_bytes_input;
  last_output = g_bytes_output; 
}

void CWorld::ConsoleOutput( double freq, 
			    unsigned int bytes_in, unsigned int bytes_out,
			    double avg_data)
{
  printf( " Time: %8.1f - %7.1fHz - [%4u/%4u] %8.2f b/sec\r", 
  //printf( "\n Time: %8.1f - %7.1fHz - [%4u/%4u] %8.2f b/sec\n", 
	  m_sim_time, 
          freq,
	  bytes_in, bytes_out, 
          avg_data );
  
  fflush( stdout );
  
}

bool CWorld::Load( void )
{
  ///////////////////////////////////////////////////////////////////////
  // LOAD THE CONFIGURATION FOR THE GUI 
  // we call this *after* the world has loaded, so we can configure the menus
  // correctly

  if(this->enable_gui ) GuiLoad( this );
  
  return true; // success
}

bool CWorld::Save( )
{
  if( this->enable_gui ) GuiSave( this );

  return true; // success
}

void CWorld::LogOutput( double freq,
			unsigned int bytes_in, unsigned int bytes_out, 
			unsigned int total_bytes_in, 
			unsigned int total_bytes_out )
{  
  assert( m_log_fd > 0 );
  
  char line[512];
  sprintf( line,
           "%u\t\t%.3f\t%u\t%u\t%u\t%u\n", 
           m_step_num, m_sim_time, // step and time
           //loop_duration, // real cycle time in ms
           //sleep_duration, // real sleep time in ms
           //m_sim_timestep / sleep_duration, // ratio
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
      
  char* tmstr = ctime((const time_t*)&t.tv_sec);
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
           this->entity_count );
      
  write( m_log_fd, line, strlen(line) );
}

// return the entity nearest the specified point, but not more than range m away,
// that has the specified parent 
CEntity* CWorld::GetNearestChildWithinRange( double x, double y, double range, 
					     CEntity* parent )
{

  //printf( "Searching from %.2f,%.2f for children of %p\n", x, y, parent );

  CEntity* nearest = NULL;
  
  double px, py, pth;
  double d;

  for( int c=0; c<this->entity_count; c++ )
    {
      CEntity* ent = this->entities[c];
      
      // we can only select items with root a parent
      if( ent->m_parent_entity != parent )
	continue;
      
      ent->GetGlobalPose( px, py, pth );
      
      d = hypot( py - y, px - x );
      
      //printf( "Entity type %d is %.2fm away at  %.2f,%.2f\n", 
      //      ent->stage_type, d, px, py );
      
      if( d < range )
	{
	  range = d;
	  nearest = ent;
	}
    }
  
  //if( nearest )
  //printf ( "Nearest is type %d\n", nearest->stage_type );
  //else
  //puts( "no entity within range" );

  return nearest;
}

// return the entity nearest the specified point, but not more than range m away,
// that has the specified parent 
CEntity* CWorld::GetNearestEntityWithinRange( double x, double y, double range )		
{
  CEntity* nearest = NULL;
  double px, py, pth;
  double d;
  
  for( int c=0; c<this->entity_count; c++ )
    {
      CEntity* ent = this->entities[c];
      
      ent->GetGlobalPose( px, py, pth );
      
      d = hypot( py - y, px - x );
      
      printf( "Entity type %d is %.2fm away at  %.2f,%.2f\n", 
	      ent->stage_type, d, px, py );
      
      if( d < range )
	{
	  range = d;
	  nearest = ent;
	}
    }
  
  if( nearest )
    printf ( "Nearest is type %d\n", nearest->stage_type );
  else
    puts( "no entity within range" );
  
  return nearest;
}

///////////////////////////////////////////////////////////////////////////
// lock the shared mem
//
bool CWorld::LockByte( int offset )
{
  return true; // success
}

///////////////////////////////////////////////////////////////////////////
// unlock the shared mem
//
bool CWorld::UnlockByte( int offset )
{
  return true; // success
}
