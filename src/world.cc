///////////////////////////////////////////////////////////////////////////
//
// File: world.cc
// Author: Richard Vaughan, Andrew Howard
// Date: 7 Dec 2000
// Desc: top level class that contains everything
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/world.cc,v $
//  $Author: inspectorg $
//  $Revision: 1.70 $
//
///////////////////////////////////////////////////////////////////////////

#include <errno.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <signal.h>
#include <math.h>
#include <fstream.h>
#include <iostream.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pwd.h>
#include <stdio.h>
#include <libgen.h>  // for dirname/#define DEBUG

bool usage = false;
void PrintUsage(); // defined in main.cc

extern long g_bytes_input, g_bytes_output;

int g_timer_expired = 0;

#define DEBUG 
#define VERBOSE

//#include <stage.h>
#include "world.hh"
// for the definition of CPlayerDevice, used to treat them specially
// below
#include "playerdevice.hh" 
#include "truthserver.hh"
#include "fixedobstacle.hh"

// thread entry function, defined in envserver.cc
void* EnvServer( void* );

#define SEMKEY 2000

// allocate chunks of 32 pointers for entity storage
const int OBJECT_ALLOC_SIZE = 32;

#define WATCH_RATES


// this is the root filename for stage devices
// this is appended with the user name and instance number
// so in practice you get something like /tmp/stageIO.vaughan.0
// currently only the zero instance is supported in player
#define IOFILENAME "/tmp/stageIO"

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
CWorld::CWorld()
{
  // seed the random number generator
  srand48( time(NULL) );

  // Initialise configuration variables
  // REMOVE this->size_x = 10;
  // REMOVE this->size_y = 10;
  this->ppm = 20;
  this->wall = NULL;
  
  // stop time of zero means run forever
  m_stoptime = 0;

  // invalid file descriptor initially
  m_log_fd = -1;

  m_external_sync_required = false;
  m_env_server_ready = false;
  m_instance = 0;

  // AddObject() handles allocation of storage for entities
  // just initialize stuff here
  m_object = NULL;
  m_object_alloc = 0;
  m_object_count = 0;

  m_log_output = false;
  m_console_output = true;

  //m_realtime_mode = true;
  // defaults time steps can be tweaked by command line or config file
    
  // real time mode by default
  // if real_timestep is zero, we run as fast as possible
  m_real_timestep = 0.1; //seconds
  m_sim_timestep = 0.1; //seconds; - 10Hz default rate 
  m_step_num = 0;

  // Allow the simulation to run
  //
  m_enable = true;

  // set the server ports to default values
  m_pose_port = DEFAULT_POSE_PORT;
  m_env_port = DEFAULT_ENV_PORT;
    
  // init the pose server data structures
  m_pose_connection_count = 0;
  memset( m_pose_connections, 0, 
          sizeof(struct pollfd) * MAX_POSE_CONNECTIONS );
  memset( m_conn_type, 0, sizeof(char) * MAX_POSE_CONNECTIONS );
    
  m_sync_counter = 0;

  if( gethostname( m_hostname, sizeof(m_hostname)) == -1)
  {
    perror( "XS: couldn't get hostname. Quitting." );
    exit( -1 );
  }
  /* now strip off domain */
  char* first_dot;
  strncpy(m_hostname_short, m_hostname,HOSTNAME_SIZE);
  if( (first_dot = strchr(m_hostname_short,'.') ))
    *first_dot = '\0';
    
  //printf( "\nCWorld::m_hostname: %s, m_hostname_short %s\n", 
  //    m_hostname, m_hostname_short );

  // enable external services by default
  m_run_environment_server = true;
  m_run_pose_server = true;
    
  // default color database file
  strcpy( m_color_database_filename, COLOR_DATABASE );
    
#ifdef INCLUDE_RTK
  // disable XS by default in rtkstage
  m_run_xs = false;
#else
  // enable XS by default in stage
  m_run_xs = true; 
#endif

  // Initialise world filename
  //
  m_filename[0] = 0;
    
  // Initialise clocks
  m_start_time = m_sim_time = 0;
  memset( &m_sim_timeval, 0, sizeof( struct timeval ) );
    
  // Initialise object list
  m_object_count = 0;
    
  // start with no key
  bzero(m_auth_key,sizeof(m_auth_key));
}


///////////////////////////////////////////////////////////////////////////
// Destructor
CWorld::~CWorld()
{
  if (wall)
    delete wall;

  if( matrix )
    delete matrix;

  for( int m=0; m < m_object_count; m++ )
    delete m_object[m];
}


///////////////////////////////////////////////////////////////////////////
// Parse the command line
bool CWorld::ParseCmdline(int argc, char **argv)
{
  if( argc < 2 )
  {
    usage = true;
    exit( -1 );
  }
  
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
    else if( strcmp( argv[a], "-l" ) == 0 )
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
      
    // FAST MODE - run as fast as possible - don't attempt t match real time
    if( strcmp( argv[a], "-fast" ) == 0 )
    {
      m_real_timestep = 0.0;
      printf( "[Fast]" );
    }

    // DIS/ENABLE XS
    if( strcmp( argv[a], "-xs" ) == 0 )
    {
      m_run_xs = false;
      printf( "[No GUI]" );
    }
    else if( strcmp( argv[a], "+xs" ) == 0 )
    {
      m_run_xs = true;
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

    // change the pose port 
    else if( strcmp( argv[a], "-tp" ) == 0 )
    {
      m_pose_port = atoi(argv[a+1]);
      printf( "[Pose %d]", m_pose_port );
      a++;
    }

    // change the environment port
    else if( strcmp( argv[a], "-ep" ) == 0 )
    {
      m_env_port = atoi(argv[a+1]);
      printf( "[Env %d]", m_env_port );
      a++;
    }

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
// Load the world
bool CWorld::Load(const char *filename)
{
  printf( "[World %s]", filename );
  fflush( stdout );

  // the default hostname is this host's name
  char current_hostname[ HOSTNAME_SIZE ];
  strcpy( current_hostname, m_hostname );
  
  // Load and parse the world file
  if (!this->worldfile.Load(filename))
    return false;

  // Make sure there is an "environment" section
  int section = this->worldfile.LookupSection("environment");
  if (section < 0)
  {
    PRINT_ERR("no environment specified");
    return false;
  }
  
  // Construct a single fixed obstacle representing
  // the environment.
  this->wall = new CFixedObstacle(this, NULL);
  
  // Load the settings for this object
  if (!this->wall->Load(&this->worldfile, section))
    return false;

  // Get the resolution of the environment (meters per pixel in the file)
  this->ppm = 1 / this->worldfile.ReadLength(section, "resolution", 1 / this->ppm);

  // Initialise the matrix, now that we know its size
  int w = (int) ceil(this->wall->m_size_x * this->ppm);
  int h = (int) ceil(this->wall->m_size_y * this->ppm);
  this->matrix = new CMatrix(w, h, 1);
  
  // Get the authorization key to pass to player
  const char *authkey = this->worldfile.ReadString(0, "auth_key", "");
  strncpy(m_auth_key, authkey, sizeof(m_auth_key));
  m_auth_key[sizeof(m_auth_key)-1] = '\0';

  // Get the real update interval
  m_real_timestep = this->worldfile.ReadFloat(0, "real_timestep", 0.100);

  // Get the simulated update interval
  m_sim_timestep = this->worldfile.ReadFloat(0, "sim_timestep", 0.100);
  
  // Iterate through sections and create objects as needs be
  for (int section = 1; section < this->worldfile.GetSectionCount(); section++)
  {
    // Find out what type of object this is,
    // and what line it came from.
    const char *type = this->worldfile.GetSectionType(section);
    int line = this->worldfile.ReadInt(section, "line", -1);

    // Ignore some types, since we have already dealt with them
    if (strcmp(type, "environment") == 0)
      continue;

    // Find the parent object
    CEntity *parent = NULL;
    int psection = this->worldfile.GetSectionParent(section);
    for (int i = 0; i < GetObjectCount(); i++)
    {
      CEntity *object = GetObject(i);
      if (GetObject(i)->worldfile_section == psection)
        parent = object;
    }

    // Create the object
    CEntity *object = CreateObject(type, parent);
    if (object != NULL)
    {      
      // Store which section it came from (so we know where to
      // save it to later).
      object->worldfile_section = section;

      // Work out whether or not its a local object
      // if the object has this host's name,
      // set the local flag to show that this computer must
      // update the object
      // NEEDS FIXING AH
      strncpy( object->m_hostname, current_hostname, HOSTNAME_SIZE );
      if(CheckHostname(object->m_hostname))
        object->m_local = true;
      else
        object->m_local = false;
      
      // Let the object load itself
      if (!object->Load(&this->worldfile, section))
        return false;

      // Add to list of objects
      AddObject(object);
    }
    else
      PRINT_ERR2("line %d : unrecognized type [%s]", line, type);
  }

#ifdef INCLUDE_RTK2
  // Initialize the GUI, but dont start it yet
  if (!LoadGUI(&this->worldfile))
    return false;
#endif

  // See if there was anything we didnt understand in the world file
  this->worldfile.WarnUnused();
  
  return true;
}


///////////////////////////////////////////////////////////////////////////
// Save objects to a file
bool CWorld::Save(const char *filename)
{
  // Let each object save itself
  for (int i = 0; i < GetObjectCount(); i++)
  {
    CEntity *object = GetObject(i);
    if (!object->Save(&this->worldfile, object->worldfile_section))
      return false;
  }

  // Save everything
  if (!this->worldfile.Save(filename))
    return false;

  return true;
}


///////////////////////////////////////////////////////////////////////////
// Startup routine 
bool CWorld::Startup()
{  
  PRINT_DEBUG( "** STARTUP **" );

  // we must have at least one object to play with!
  assert( m_object_count > 0 );

  // Initialise the wall object
  if (!this->wall->Startup())
    return false;
    
  // Initialise the real time clock
  // Note that we really do need to set the start time to zero first!
  m_start_time = 0;
  m_start_time = GetRealTime();
    
  // Initialise the rate counter
  m_update_ratio = 1;
  m_update_rate = 0;
  
  // kick off the pose and envirnment servers, unless we disabled them earlier
  if( m_run_pose_server )
    SetupPoseServer();
  
  if( m_run_environment_server )
  {
    pthread_t tid_dummy;
    pthread_create(&tid_dummy, NULL, &EnvServer, (void *)NULL );  
      
    // env server sets this flag when it's ready
    while( !m_env_server_ready )
      usleep( 100000 );
  }
  
  // Create the device directory and clock 
  CreateClockDevice();
  
  // Startup all the objects
  // Devices will create and initialize their device files
  for (int i = 0; i < m_object_count; i++)
  {
    if( !m_object[i]->Startup() )
    {
      PRINT_ERR("object startup failed");
      return false;
    }
  }
  
  // exec and set up Player
  StartupPlayer();
  
  // spawn an XS process, unless we disabled it (rtkstage disables xs by default)
  if( m_run_xs && m_run_pose_server && m_run_environment_server ) 
    SpawnXS();

#ifdef INCLUDE_RTK2
  // Start the GUI
  StartupGUI();
#endif
  
  // start the real-time interrupts going
  StartTimer( m_real_timestep );
  
#ifdef INCLUDE_RTK
  // Initialise the message router
  m_router->add_sink(RTK_UI_DRAW, (void (*) (void*, void*)) &OnUiDraw, this);
  m_router->add_sink(RTK_UI_MOUSE, (void (*) (void*, void*)) &OnUiMouse, this);
  m_router->add_sink(RTK_UI_PROPERTY, (void (*) (void*, void*)) &OnUiProperty, this);
  m_router->add_sink(RTK_UI_BUTTON, (void (*) (void*, void*)) &OnUiButton, this);
#endif
 
  PRINT_DEBUG( "** STARTUP DONE **" );
  return true;
}
    

///////////////////////////////////////////////////////////////////////////
// Shutdown routine 
void CWorld::Shutdown()
{
#ifdef INCLUDE_RTK2
  // Stop the GUI
  ShutdownGUI();
#endif

  // Shutdown player
  ShutdownPlayer();
  
  // Shutdown all the objects
  // Devices will unlink their device files
  for (int i = 0; i < m_object_count; i++)
  {
    if(m_object[i])
      m_object[i]->Shutdown();
  }
  
  // zap the clock device 
  unlink( clockName );

  // Shutdown the wall
  this->wall->Shutdown();

  // delete the device directory
  if( rmdir( m_device_dir ) != 0 )
    PRINT_WARN1("failed to delete device directory: [%s]", strerror(errno));
}


///////////////////////////////////////////////////////////////////////////
// Start the single instance of player
bool CWorld::StartupPlayer( void )
{
  PRINT_DEBUG( "** STARTUP PLAYER **" );

  // startup any player devices - no longer hacky! - RTV
  // we need to have fixed up all the shared memory and pointers already
  
  // count the number of Players on this host
  int player_count = 0;
  for (int i = 0; i < m_object_count; i++)
    if(m_object[i]->m_stage_type == PlayerType && 
       CheckHostname(m_object[i]->m_hostname))
      player_count++;
  
  // if there is at least 1 player device, we start a copy of Player
  // running.
  if (player_count == 0)
    return true;

  // ----------------------------------------------------------------------
  // fork off a player process to handle robot I/O
  if( (this->player_pid = fork()) < 0 )
  {
    PRINT_ERR1("error forking for player: [%s]", strerror(errno));
    return false;
  }

  // If we are the nmew child process...
  if (this->player_pid == 0)
  {
    // pass in the number of ports player should find in the IO directory
    // We tell it how many unique ports it should find in the
    // io directory, just as a sanity check.
    char portBuf[32];
    sprintf( portBuf, "%d", (int) player_count );

    // release controlling tty so Player doesn't get signals
    setpgrp();

    // we assume Player is in the current path
    if( execlp( "player", "player",
                //"-ports", portBuf, 
                "-stage", DeviceDirectory(), 
                (strlen(m_auth_key)) ? "-key" : NULL,
                (strlen(m_auth_key)) ? m_auth_key : NULL,
                NULL) < 0 )
    {
      PRINT_ERR1("error executing stage [%s]\n"
                 "Make sure player is in your path.", strerror(errno));
      if(!kill(getppid(),SIGINT))
      {
        PRINT_ERR1("error killing player: [%s]", strerror(errno));
        exit(-1);
      }
    }
  }
  
  PRINT_DEBUG( "** STARTUP PLAYER DONE **" );
  return true;
}


///////////////////////////////////////////////////////////////////////////
// Stop player instance
void CWorld::ShutdownPlayer()
{
  int wait_retval;

  if(this->player_pid)
  {
    if(kill(this->player_pid,SIGTERM))
      PRINT_ERR1("error killing player: [%s]", strerror(errno));
    if((wait_retval = waitpid(this->player_pid,NULL,0)) == -1)
      PRINT_ERR1("waitpid failed: [%s]", strerror(errno));
    this->player_pid = 0;
  }
}


/////////////////////////////////////////////////////////////////////////
// Clock device -- create the memory map for IPC with Player 
bool CWorld::CreateClockDevice( void )
{ 
  // get the user's name
  struct passwd* user_info = getpwuid( getuid() );

  //int tfd = 0;

  // make a device directory from a base name and the user's name
  sprintf( m_device_dir, "%s.%s.%d", 
           IOFILENAME, user_info->pw_name, m_instance++ );
  
  if( mkdir( m_device_dir, S_IRWXU | S_IRWXG | S_IRWXO ) == -1 )
  {
    if( errno != EEXIST )
    {
      perror( "Stage: failed to make device directory" );
      exit( -1 );
    }
  }

  char hostdir[256];
  sprintf( hostdir, "%s", m_device_dir );

  if( mkdir( hostdir, S_IRWXU | S_IRWXG | S_IRWXO ) == -1 )
  {
    if( errno != EEXIST )
    {
      perror( "Stage: failed to make host directory" );
      exit( -1 );
    }
  }
  
  //printf( "Created directories %s\n", hostdir );
  //printf( "[Instance %d]", m_instance-1 );

  // create the time device

  size_t clocksize = sizeof( struct timeval );
  
  sprintf( clockName, "%s/clock", m_device_dir );
  //sprintf( clockName, "/tmp/clock");
  
#ifdef DEBUG
  cout << "Creating clock device at " << clockName << endl;
#endif

  int tfd=-1;
  if( (tfd = open( clockName, O_RDWR | O_CREAT | O_TRUNC, 
                   S_IRUSR | S_IWUSR )) < 0 )
  {
    perror("Failed to create clock device" );
    exit( -1 );
  } 
  
  // make the file the right size
  off_t sz = clocksize;

  if( ftruncate( tfd, sz ) < 0 )
  {
    perror( "Failed to set clock file size" );
    return false;
  }
  
  int r;
  if( (r = write( tfd, &m_sim_timeval, sizeof( m_sim_timeval ))) < 0 )
    perror( "failed to write time into clock device" );
  
  m_time_io = (struct timeval*)mmap( NULL, clocksize, 
                                     PROT_READ | PROT_WRITE, MAP_SHARED,
                                     tfd, (off_t) 0);
  if (m_time_io == MAP_FAILED )
  {
    perror( "Failed to map memory" );
    return false;
  }
  
  close( tfd ); // can close fd once mapped
  
  // Initialise space
  //
  memset( m_time_io, 0, clocksize );
  
  // Create the lock object for the shared mem
  //
  if ( !CreateShmemLock() )
  {
    perror( "failed to create lock for shared memory" );
    return false;
  }
  
#ifdef DEBUG
  cout << "Successfully mapped clock device." << endl;  
#endif  
  
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
    cout << "Failed to install signal handler" << endl;
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
    cout << "failed to set timer" << endl;;
    exit( -1 );
  }
}


///////////////////////////////////////////////////////////////////////////
// Thread entry point for the world
//
void CWorld::Main(void)
{
  //puts( "** MAIN **" );
  //assert( arg == 0 );
  
  static double loop_start = GetRealTime();

#ifdef INCLUDE_RTK
  // RTV - moved this init - nasty to rtkstage?
  double ui_time = 0;
#endif
  
  //while (true)
  // {
  // Set the timer flag depending on the current mode.
  // If we're NOT in realtime mode, the timer is ALWAYS expired
  // so we run as fast as possible
  m_real_timestep > 0.0 ? g_timer_expired = 0 : g_timer_expired = 1;
      
  // Check for thread cancellation
  //pthread_testcancel();
      
  // look for new connections to the poseserver
  ListenForPoseConnections();

  // calculate new world state
  if (m_enable) 
    Update();
  else
    usleep( 100000 ); // stops us hogging the machine while we're paused

  // for logging statistics
  double update_time = GetRealTime();
      
  if( m_pose_connection_count == 0 )
	{      
	  m_step_num++;
	  
	  // if we have spare time, sleep until it runs out
	  if( g_timer_expired < 1 ) sleep( 1 ); 
	}
  else // handle the connections
	{
	  PoseWrite(); // writes out anything that is dirty
	  PoseRead();
	}
      
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

  // Update the GUI every 100ms
  //
#ifdef INCLUDE_RTK
  if (GetRealTime() - ui_time > 0.050)
  {
	  ui_time = GetRealTime();
	  m_router->send_message(RTK_UI_FORCE_UPDATE, NULL);
  }
#endif
  // }
}


///////////////////////////////////////////////////////////////////////////
// Update the world
void CWorld::Update()
{  
  // Update the simulation time (in both formats)
  //
  m_sim_time = m_step_num * m_sim_timestep;
  m_sim_timeval.tv_sec = (long)floor(m_sim_time);
  m_sim_timeval.tv_usec = (long)((m_sim_time - floor(m_sim_time)) * MILLION); 

  // is it time to stop?
  if(m_stoptime && m_sim_time >= m_stoptime)
    system("kill `cat stage.pid`");

  // copy the timeval into the player io buffer
  // use the first object's info
  LockShmem();
  *m_time_io = m_sim_timeval;
  UnlockShmem();

  // Do the actual work -- update the objects 
  for (int i = 0; i < m_object_count; i++)
  {
    // if this host manages this object
    //if( m_object[i]->m_local )
    m_object[i]->Update( m_sim_time ); // update it 
  };
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
    //
    int x = (int) (px * ppm);
    int y = (int) (py * ppm);
    int r = (int) (pr * ppm);
    
    matrix->draw_circle( x,y,r,ent, add);
}


///////////////////////////////////////////////////////////////////////////
// Add an object to the world
//
void CWorld::AddObject(CEntity *object)
{
  // if we've run out of space (or we never allocated any)
  if( (!m_object) || (!( m_object_count < m_object_alloc )) )
    {
      // allocate some more
      CEntity** more_space = new CEntity*[ m_object_alloc + OBJECT_ALLOC_SIZE ];
      
      // copy the old data into the new space
      memcpy( more_space, m_object, m_object_count * sizeof( CEntity* ) );
     
      // delete the original
      if( m_object ) delete [] m_object;
      
      // bring in the new
      m_object = more_space;
 
      // record the total amount of space
      m_object_alloc += OBJECT_ALLOC_SIZE;
    }
  
  // insert the object and increment the count
  m_object[m_object_count++] = object;
}


///////////////////////////////////////////////////////////////////////////
// lock the shared mem
// Returns true on success
//
bool CWorld::LockShmem( void )
{
  struct sembuf ops[1];

  ops[0].sem_num = 0;
  ops[0].sem_op = 1;
  ops[0].sem_flg = 0;

#ifdef DEBUG
  // printf( "Lock semaphore: %d %d\n" , semKey, semid );
  //fflush( stdout );
#endif

  int retval = semop( semid, ops, 1 );
  if (retval != 0)
  {
      printf("lock failed return value = %d\n", (int) retval);
      return false;
  }
  return true;
}

///////////////////////////////////////////////////////////////////////////
// Create a single semaphore to sync access to the shared memory segments
//
bool CWorld::CreateShmemLock()
{
    semKey = SEMKEY;

    union semun
    {
        int val;
        struct semid_ds *buf;
        ushort *array;
    } argument;

    argument.val = 0; // initial semaphore value
    semid = semget( semKey, 1, 0666 | IPC_CREAT );

    if( semid < 0 ) // semget failed
    {
        printf( "Stage: Unable to create semaphore\n" );
        return false;
    }
    if( semctl( semid, 0, SETVAL, argument ) < 0 )
    {
        printf( "Stage: Failed to set semaphore value\n" );
        return false;
    }

#ifdef DEBUG
    printf( "Stage: Create semaphore: semkey=%d semid=%d\n", semKey, semid );
    
#endif

    return true;
}

///////////////////////////////////////////////////////////////////////////
// unlock the shared mem
//
void CWorld::UnlockShmem( void )
{
  struct sembuf ops[1];

  ops[0].sem_num = 0;
  ops[0].sem_op = -1;
  ops[0].sem_flg = 0;

#ifdef DEBUG
  //printf( "Unlock semaphore: %d %d\n", semKey, semid );
  // fflush( stdout );
#endif

  int retval = semop( semid, ops, 1 );
  if (retval != 0)
      printf("Stage: unlock failed return value = %d\n", (int) retval);
}


#ifdef INCLUDE_RTK

/////////////////////////////////////////////////////////////////////////
// Initialise rtk
//
void CWorld::InitRtk(RtkMsgRouter *router)
{
    m_router = router;
}


///////////////////////////////////////////////////////////////////////////
// Process GUI update messages
//
void CWorld::OnUiDraw(CWorld *world, RtkUiDrawData *data)
{
    // Draw the background
    //
    if (data->draw_back("global"))
        world->DrawBackground(data);

    // Draw grid layers (for debugging)
    //
    data->begin_section("global", "debugging");

    if (data->draw_layer("obstacle", false))
        world->DrawDebug(data, DBG_OBSTACLES);        

    if (data->draw_layer("puck", false))
        world->DrawDebug(data, DBG_PUCKS);

    if (data->draw_layer("laser", false))
        world->DrawDebug(data, DBG_LASER);

    if (data->draw_layer("vision", false))
        world->DrawDebug(data, DBG_VISION);

    if (data->draw_layer("sonar", false))
        world->DrawDebug(data, DBG_SONAR);
    
    data->end_section();

    // Draw the children
    //
    for (int i = 0; i < world->m_object_count; i++)
        world->m_object[i]->OnUiUpdate(data);
}


///////////////////////////////////////////////////////////////////////////
// Process GUI mouse messages
//
void CWorld::OnUiMouse(CWorld *world, RtkUiMouseData *data)
{
    data->begin_section("global", "move");

    // Create a mouse mode (used by objects)
    //
    data->mouse_mode("object");
    
    data->end_section();

    // Update the children
    //
    for (int i = 0; i < world->m_object_count; i++)
        world->m_object[i]->OnUiMouse(data);
}


///////////////////////////////////////////////////////////////////////////
// UI property message handler
//
void CWorld::OnUiProperty(CWorld *world, RtkUiPropertyData* data)
{
    RtkString value;

    data->begin_section("default", "world");
    
    RtkFormat1(value, "%7.3f", (double) world->GetTime());
    data->add_item_text("Simulation time (s)", CSTR(value), "");
    
    RtkFormat1(value, "%7.3f", (double) world->GetRealTime());
    data->add_item_text("Real time (s)", CSTR(value), "");

    RtkFormat1(value, "%7.3f", (double) world->m_update_ratio);
    data->add_item_text("Sim/real time", CSTR(value), ""); 
    
    RtkFormat1(value, "%7.3f", (double) world->m_update_rate);
    data->add_item_text("Update rate (Hz)", CSTR(value), "");

    data->end_section();

    // Update the children
    //
    for (int i = 0; i < world->m_object_count; i++)
        world->m_object[i]->OnUiProperty(data);
}


///////////////////////////////////////////////////////////////////////////
// UI button message handler
//
void CWorld::OnUiButton(CWorld *world, RtkUiButtonData* data)
{
    data->begin_section("default", "world");

    if (data->check_button("enable", world->m_enable))
        world->m_enable = !world->m_enable;

    if (data->push_button("save"))
        world->Save(world->m_filename);
    
    data->end_section();
}


///////////////////////////////////////////////////////////////////////////
// Draw the background; i.e. things that dont move
//
void CWorld::DrawBackground(RtkUiDrawData *data)
{
  RTK_TRACE0("drawing background");

  data->set_color(RGB(0, 0, 0));
    
  // Loop through the image and draw points individually.
  // Yeah, it's slow, but only happens once.
  //
  for (int y = 0; y < matrix->height; y++)
  {
    for (int x = 0; x < matrix->width; x++)
    {
      if( matrix->is_type(x, y, WallType ) )
      {
	      double px = (double) x / ppm;
	      double py = (double)  y / ppm;
	      double s = 1.0 / ppm;
	      data->rectangle(px, py, px + s, py + s);
      }
    }
  }
}


///////////////////////////////////////////////////////////////////////////
// Draw the various layers
//
void CWorld::DrawDebug(RtkUiDrawData *data, int options)
{
  data->set_color(RTK_RGB(0, 0, 255));
    
  // Loop through the image and draw points individually.
  // Yeah, it's slow, but only happens for debugging
  //
  for (int y = 0; y < matrix->get_height(); y++)
  {
    for (int x = 0; x < matrix->get_width(); x++)
    {
      CEntity **entity = matrix->get_cell(x, y);

      for (int i = 0; entity[i] != NULL; i++)
      {
        double px = (double) x / ppm;
        double py = (double) y / ppm;
        double s = 1.0 / ppm;

        if (options == DBG_OBSTACLES)
          if (entity[i]->obstacle_return)
            data->rectangle(px, py, px + s, py + s);
        if (options == DBG_PUCKS)
          if (entity[i]->puck_return)
            data->rectangle(px, py, px + s, py + s);
        if (options == DBG_LASER)
          if (entity[i]->laser_return)
            data->rectangle(px, py, px + s, py + s);
        if (options == DBG_VISION)
          if (entity[i]->channel_return)
            data->rectangle(px, py, px + s, py + s);
        if (options == DBG_SONAR)
          if (entity[i]->sonar_return)
            data->rectangle(px, py, px + s, py + s);
      }
    }
  }
}

#endif // INCLUDE_RTK

#ifdef INCLUDE_RTK2

// Initialise the GUI
bool CWorld::LoadGUI(CWorldFile *worldfile)
{
  int section = worldfile->LookupSection("rtk");

  // Size in pixels
  int sx = (int) worldfile->ReadTupleFloat(section, "size", 0, this->matrix->width);
  int sy = (int) worldfile->ReadTupleFloat(section, "size", 1, this->matrix->height);

  // Scale
  double scale = worldfile->ReadLength(section, "scale", 1 / this->ppm);
  
  // Size in meters
  double dx = sx * scale;
  double dy = sy * scale;
  
  this->app = rtk_app_create();
  rtk_app_size(this->app, sx, sy);
  rtk_app_refresh_rate(this->app, 10);
  
  this->canvas = rtk_canvas_create(this->app);
  rtk_canvas_size(this->canvas, sx, sy);
  rtk_canvas_scale(this->canvas, dx / sx, dy / sy);
  rtk_canvas_origin(this->canvas, dx / 2, dy / 2);

  return true;
}

// Start the GUI
bool CWorld::StartupGUI()
{
  rtk_app_start(this->app);
  return true;
}

// Stop the GUI
void CWorld::ShutdownGUI()
{
  rtk_app_stop(this->app);
}

#endif

  
char* CWorld::StringType( StageType t )
{
  switch( t )
  {
    case NullType: return "None"; 
    case WallType: return "Wall"; break;
    case PlayerType: return "Player"; 
    case MiscType: return "Misc"; 
    case RectRobotType: return "RectRobot"; 
    case RoundRobotType: return "RoundRobot"; 
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
  
// attempts to spawn an XS process. No harm done if it fails
void CWorld::SpawnXS( void )
{
  int pid = 0;
  
  // ----------------------------------------------------------------------
  // fork off an xs process
  if( (pid = fork()) < 0 )
    cerr << "fork error in SpawnXS()" << endl;
  else
  {
    if( pid == 0 ) // new child process
    {
      char envbuf[32];
      sprintf( envbuf, "%d", m_env_port );

      char posebuf[32];
      sprintf( posebuf, "%d", m_pose_port );
	  
      // we assume xs is in the current path
      if( execlp( "xs", "xs",
                  "-ep", envbuf,
                  "-tp", posebuf, NULL ) < 0 )
	    {
	      cerr << "exec failed in SpawnXS(): make sure XS can be found"
          " in the current path."
             << endl;
	      exit( -1 ); // die!
	    }
    }
    else
    {
      //puts( "[XS]" );
      //fflush( stdout );
    }
  }
}

int CWorld::ColorFromString( StageColor* color, const char* colorString )
{
  ifstream db( m_color_database_filename );
  
  if( !db )
    cerr << "Failed to load color database file " << m_color_database_filename << endl; 

  int red, green, blue;
  
  //cout << "Searching for " << colorString << " in " <<  m_color_database_filename << endl; 

  while( !db.eof() && !db.bad() )
  {
    char c;
    char line[255];
    //char name[255];
      
    db.get( line, 255, '\n' ); // read in a description string
      
    //cout << "Read entry: " << line << endl;

    if( db.get( c ) && c != '\n' )
      cout << "Warning: line too long in color database file" << endl;
      
    if( line[0] == '!' || line[0] == '#' || line[0] == '%' ) 
      continue; // it's a macro or comment line - ignore the line
      
    int chars_matched = 0;
    sscanf( line, "%d %d %d %n", &red, &green, &blue, &chars_matched );
      
    // name points to the rest of the line, after we've matched out the colors
    char* name = line + chars_matched;

    //printf( "Parsed: %d %d %d :%s\n", red, green, blue, name );
    //fflush( stdout );

    if( strcmp( name, colorString ) == 0 ) // the name matches!
    {
      color->red = (unsigned short)red;
      color->green = (unsigned short)green;
      color->blue = (unsigned short)blue;
	  
      db.close();
	  
      return true;
    }
  }
  return false;
}  

// returns true if the given hostname matches our hostname, false otherwise
bool CWorld::CheckHostname(char* host)
{
  //printf( "checking %s against (%s and %s) ", 
  //  host, m_hostname, m_hostname_short ); 

  if(!strcmp(m_hostname,host) || !strcmp(m_hostname_short,host))
  {
    //puts( "TRUE" );
    return true;
  }
  else
  {
    //puts( "FALSE" );
    return false;
  }
}


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

  ConsoleOutput( freq, loop_duration, sleep_duration, 
		 avg_loop_duration, avg_sleep_duration,
		 bytes_in, bytes_out, bandw );
  
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
      
      // count the locally managed objects
      int m=0;
      for( int c=0; c<m_object_count; c++ )
	if( m_object[c]->m_local ) m++;
      
      char* tmstr = ctime( &t.tv_sec);
      tmstr[ strlen(tmstr)-1 ] = 0; // delete the newline
      
      char line[512];
      sprintf( line,
	       "# Stage output log\n#\n"
	       "# Command:\t%s\n"
	       "# Date:\t\t%s\n"
	       "# Host:\t\t%s\n"
	       "# Bitmap:\t%s\n"
	       "# Timestep(ms):\t%d\n"
	       "# Objects:\t%d of %d\n#\n"
	       "#STEP\t\tSIMTIME(s)\tINTERVAL(s)\tSLEEP(s)\tRATIO\t"
	       "\tINPUT\tOUTPUT\tITOTAL\tOTOTAL\n",
	       m_cmdline, 
	       tmstr, 
	       m_hostname, 
	       m_filename,
	       (int)(m_sim_timestep * 1000.0),
	       m, 
	       m_object_count );
      
      write( m_log_fd, line, strlen(line) );
}



