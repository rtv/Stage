///////////////////////////////////////////////////////////////////////////
//
// File: world.cc
// Author: Richard Vaughan, Andrew Howard
// Date: 7 Dec 2000
// Desc: top level class that contains everything
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/world.cc,v $
//  $Author: ahoward $
//  $Revision: 1.68.2.5 $
//
///////////////////////////////////////////////////////////////////////////

#include <sys/time.h>
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

extern long g_bytes_input, g_bytes_output;

int g_timer_expired = 0;

//#include <stage.h>
#include "world.hh"
// for the definition of CPlayerDevice, used to treat them specially
// below
#include "playerdevice.hh" 

#include "truthserver.hh"

// thread entry function, defined in envserver.cc
void* EnvServer( void* );

#define SEMKEY 2000

// allocate chunks of 32 pointers for entity storage
const int OBJECT_ALLOC_SIZE = 32;

#define WATCH_RATES
//#define DEBUG 
//#define VERBOSE
#undef DEBUG 
#undef VERBOSE

// right now i have a single fixed filename for the mmapped IO with Player
// eventually we'll be able to run multiple stages, each with a different
// suffix - for now its IOFILENAME.$USER
#define IOFILENAME "/tmp/stageIO"

// dummy timer signal func
void TimerHandler( int val )
{
  g_timer_expired++;
  //printf( "\ng_timer_expired: %d\n", g_timer_expired );
}  


// Main.cc calls constructor, then Load(), then Startup(), then starts thread
// at CWorld::Main();

///////////////////////////////////////////////////////////////////////////
// Default constructor
CWorld::CWorld()
{
  // seed the random number generator
  srand48( time(NULL) );

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
    
  // REMOVE memset( channel, 0, sizeof(StageColor) * ACTS_NUM_CHANNELS );
    
  // initialize the channel mapping to something reasonable
  //channel[0].red = 255;
  //channel[1].green = 255;
  //channel[2].blue = 255;

#ifdef INCLUDE_RTK
  // disable XS by default in rtkstage
  m_run_xs = false;
#else
  // enable XS by default in stage
  m_run_xs = true; 
#endif

  // Initialise world filename
  m_filename[0] = 0;
    
  // Initialise clocks
  m_start_time = m_sim_time = 0;
  memset( &m_sim_timeval, 0, sizeof( struct timeval ) );
    
  // Initialise object list
  m_object_count = 0;
    
  // Initialise configuration variables
  this->ppm = 20;
  this->scale = 100;

  memset( m_env_file, 0, sizeof(m_env_file));

  // start with no key
  bzero(m_auth_key,sizeof(m_auth_key));

}


///////////////////////////////////////////////////////////////////////////
// Destructor
CWorld::~CWorld()
{
    if( matrix )
      delete matrix;

    // zap the mmap IO point in the filesystem
    // XX fix this to test success later
    unlink( tmpName );
}

/////////////////////////////////////////////////////////////////////////
// -- create the memory map for IPC with Player 
bool CWorld::InitSharedMemoryIO( void )
{ 
  
  int mem = 0, obmem = 0;
  for (int i = 0; i < m_object_count; i++)
  {
    obmem = m_object[i]->SharedMemorySize();
    mem += obmem;
#ifdef DEBUG
    printf( "m_object[%d] (%p) needs %d (subtotal %d) bytes\n",
            i, m_object[i], obmem, mem ); 
#endif
  }
  
  // amount of memory to reserve per robot for Player IO
  // plus a slot for the time on the end
  size_t areaSize = (size_t)mem + sizeof( struct timeval );

#ifdef DEBUG
  cout << "Shared memory allocation: " << areaSize 
       << " (" << mem << ")" << endl;
#endif

  // get the user's name
  struct passwd* user_info = getpwuid( getuid() );

  int tfd = 0;

  while( tfd < 1 )
  {
    // make a filename for mmapped IO from a base name and the user's name
    sprintf( tmpName, "%s.%s.%d", 
             IOFILENAME, user_info->pw_name, m_instance++ );
      
    // try to open a temporary file
    // this'll fail if the file exists
    //  while( (tfd = open( tmpName, O_RDWR | O_CREAT | //O_EXCL |
    //	      O_TRUNC, S_IRUSR | S_IWUSR ) == -1 ))
    tfd = open( tmpName, O_RDWR | O_CREAT | O_EXCL |
                O_TRUNC, S_IRUSR | S_IWUSR );
  }
  
  if( m_instance > 1 ) 
    printf( "[Instance %d]", m_instance-1 );
  
  // make the file the right size
  if( ftruncate( tfd, areaSize ) < 0 )
  {
    perror( "Failed to set file size" );
    return false;
  }
  
  playerIO = (caddr_t)mmap( NULL, areaSize, PROT_READ | PROT_WRITE, MAP_SHARED,
                            tfd, (off_t) 0);

  if (playerIO == MAP_FAILED )
  {
    perror( "Failed to map memory" );
    return false;
  }
  
  // the time is at the end in the buffer past all the objects
  m_time_io = (struct timeval*)(((char*)playerIO) + mem);

  close( tfd ); // can close fd once mapped
  
  // Initialise entire space
  //
  memset(playerIO, 0, areaSize);
  
  // Create the lock object for the shared mem
  //
  if ( !CreateShmemLock() )
  {
    perror( "failed to create lock for shared memory" );
    return false;
  }
  
#ifdef DEBUG
  cout << "Successfully mapped shared memory." << endl;  
#endif  


  // Setup IO for all the objects - MUST DO THIS AFTER MAPPING SHARING MEMORY
  // each is passed a pointer to the start of its space in shared memory
  int entityOffset = 0;
  
  for (int i = 0; i < m_object_count; i++)
  {
    if (!m_object[i]->SetupIOPointers( playerIO + entityOffset ) )
    {
      cout << "Object " << (int)(m_object[i]) 
           << " failed SetupIOPointers()" << endl;
      return false;
    }
    entityOffset += m_object[i]->SharedMemorySize();
  }
  
  return true;
}


bool CWorld::StartupPlayer( void )
{
  // startup any player devices - hacky!
  // we need to have fixed up all the shared memory and pointers before these go

  // count the number of Players on this host, and record the index
  // of the first one.
  // this lets us know that we should start the one Player and go through
  // the next loop to give a port list...
  int first_player_idx = -1;
  int player_count = 0;
  for (int i = 0; i < m_object_count; i++)
  {
    if(m_object[i]->m_stage_type == PlayerType && 
       CheckHostname(m_object[i]->m_hostname))
    {
      player_count++;
      if(first_player_idx < 0)
        first_player_idx = i;
    }
  }

  if( player_count > 0 )
  {
    int pipe_out_fd;
    FILE* pipe_out;
  
    //printf( "Starting Player listening on %d ports\n", player_count );
  
    if((pipe_out_fd = ((CPlayerDevice*)m_object[first_player_idx])->
        StartupPlayer(player_count)) == -1)
    {
      cout << "PlayerDevice failed StartupPlayer()" << endl;
      return false;
    }
  
    // make it a stream
    if(!(pipe_out = fdopen(pipe_out_fd,"w")))
    {
      perror("CWorld::Startup(): fdopen() failed");
      return false;
    }
  
    // now pipe everbody's port into player
    for(int i = 0; i < m_object_count; i++)
    {
      // again, choose Player type objects managed by this host
      if(m_object[i]->m_stage_type == PlayerType &&
         CheckHostname(m_object[i]->m_hostname))
	    {
	      //printf( "Stage: piping portnum %d to Player\n",  m_object[i]->m_player_port );
	  
	      fprintf(pipe_out,"%d\n", m_object[i]->m_player_port);
	      fflush(pipe_out);
	    }
    }
  
    /*
      if(!fclose(pipe_out))
      {
      perror("CWorld::Startup(): fclose() failed");
      return false;
      }
    */
  }

  return true;
}


///////////////////////////////////////////////////////////////////////////
// Startup routine 
bool CWorld::Startup()
{  
  // Initialise the real time clock
  // Note that we really do need to set the start time to zero first!
  m_start_time = 0;
  m_start_time = GetRealTime();
    
  // Initialise the rate counter
  //
  m_update_ratio = 1;
  m_update_rate = 0;
  
  // Initialise the message router
  //
#ifdef INCLUDE_RTK
  m_router->add_sink(RTK_UI_DRAW, (void (*) (void*, void*)) &OnUiDraw, this);
  m_router->add_sink(RTK_UI_MOUSE, (void (*) (void*, void*)) &OnUiMouse, this);
  m_router->add_sink(RTK_UI_PROPERTY, (void (*) (void*, void*)) &OnUiProperty, this);
  m_router->add_sink(RTK_UI_BUTTON, (void (*) (void*, void*)) &OnUiButton, this);
#endif
  

  // kick off the pose and envirnment servers, unless we disabled them earlier
  if( m_run_pose_server )  SetupPoseServer();
  
  if( m_run_environment_server )
  {
    pthread_t tid_dummy;
    pthread_create(&tid_dummy, NULL, &EnvServer, (void *)NULL );  
      
    // env server sets this flag when it's ready
    while( !m_env_server_ready )
      usleep( 100000 );
  }
  
  // spawn an XS process, unless we disabled it (rtkstage disables xs by default)
  if( m_run_xs && m_run_pose_server && m_run_environment_server ) 
    SpawnXS();
  
  // Startup all the objects
  // this lets them calculate how much shared memory they'll need
  for (int i = 0; i < m_object_count; i++)
  {
    if( !m_object[i]->Startup() )
    {
      cout << "Object " << (int)(m_object[i]) 
           << " failed Startup()" << endl;
      return false;
    }
    fflush( stdout );
  }
  
  // Create the shared memory map
  InitSharedMemoryIO();
  
  // exec and set up Player
  StartupPlayer();

  // Start the world thread
  if (!StartThread())  
    return false;
  
  return true;
}
    

///////////////////////////////////////////////////////////////////////////
// Shutdown routine 
void CWorld::Shutdown()
{
  // Stop the world thread
  StopThread();
    
  // Shutdown all the objects
  for (int i = 0; i < m_object_count; i++)
  {
    if(m_object[i])
      m_object[i]->Shutdown();
  }
}


///////////////////////////////////////////////////////////////////////////
// Start world thread (will call Main)
bool CWorld::StartThread()
{
  // Start the simulator thread
  if (pthread_create(&m_thread, NULL, &Main, this) < 0)
    return false;
  return true;
}


///////////////////////////////////////////////////////////////////////////
// Stop world thread
void CWorld::StopThread()
{
  // Send a cancellation request to the thread
  pthread_cancel(m_thread);

  // Wait forever for thread to terminate
  pthread_join(m_thread, NULL);
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
void* CWorld::Main(void *arg)
{
  CWorld *world = (CWorld*) arg;
    
  // Defer cancel requests so we can handle them properly
  pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);

  // Block signals handled by gui
  sigblock(SIGINT);
  sigblock(SIGQUIT);
  sigblock(SIGHUP);

#ifdef INCLUDE_RTK
  // Initialise the GUI
  double ui_time = 0;
#endif

  // this MUST BE DONE IN THIS THREAD!
  world->StartTimer( world->m_real_timestep );
  
  // for logging statistics
  double loop_start = world->GetRealTime();
  
  while (true)
  {
    // Set the timer flag depending on the current mode.
    // If we're NOT in realtime mode, the timer is ALWAYS expired
    // so we run as fast as possible
    world->m_real_timestep > 0.0 ? g_timer_expired = 0 : g_timer_expired = 1;
      
    // Check for thread cancellation
    pthread_testcancel();
      
    // look for new connections to the poseserver
    world->ListenForPoseConnections();

    // calculate new world state
    if (world->m_enable) 
      world->Update();
    else
      usleep( 100000 ); // stops us hogging the machine while we're paused

    // for logging statistics
    double update_time = world->GetRealTime();
      
    if( world->m_pose_connection_count == 0 )
    {      
      world->m_step_num++;
	  
      // if we have spare time, sleep until it runs out
      // AH: I had to increase the sleep time to 10 seconds
      // to allow for very slow simulations (less < 1Hz).
      // This entire loop probably needs to be restructured.
      if( g_timer_expired < 1 ) sleep( 10 ); 
    }
    else // handle the connections
    {
      world->PoseWrite(); // writes out anything that is dirty
      world->PoseRead();
    }
      
    // for logging statistics
    double loop_end = world->GetRealTime(); 
    double loop_time = loop_end - loop_start;
    double sleep_time = loop_end - update_time;

    // set this here so we're timing the output time too.
    loop_start = world->GetRealTime();
      
    // generate console and logfile output
    if( world->m_enable ) world->Output( loop_time, sleep_time );
      
    // dump the contents of the matrix to a file
    //world->matrix->dump();
    //getchar();	

#ifdef INCLUDE_RTK
    // Update the GUI every 100ms
    if (world->GetRealTime() - ui_time > 0.050)
    {
      ui_time = world->GetRealTime();
      world->m_router->send_message(RTK_UI_FORCE_UPDATE, NULL);
    }
#endif
  }
}


///////////////////////////////////////////////////////////////////////////
// Update the world
extern double g_clockstarttime;
void CWorld::Update()
{  
  // Update the simulation time (in both formats)
  m_sim_time = m_step_num * m_sim_timestep;
  m_sim_timeval.tv_sec = (long)floor(m_sim_time);
  m_sim_timeval.tv_usec = (long)((m_sim_time - floor(m_sim_time)) * MILLION); 

  // is it time to stop?
  if(g_clockstarttime >= 0 && (m_sim_time - g_clockstarttime) >= m_stoptime)
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
// Initialise the world grids
bool CWorld::InitGrids(const char *env_file)
{
  // create a new background image from the pnm file
  // If we cant find it under the given name,
  // we look for a zipped version.
  
  // we use the image class just for its nice loading stuff.
  // this could be moved into the matrix pretty easily
  Nimage* img = new Nimage;

  // Try to guess the file type from the extension
  int len = strlen( env_file );
  if( len > 0 )
  {
    if (strcmp(&(env_file[len - 4]), ".fig") == 0)
    {
      if (!img->load_fig(env_file, this->ppm, this->scale))
        return false;
    }
    else if( strcmp( &(env_file[ len - 7 ]), ".pnm.gz" ) == 0 )
    {
      if (!img->load_pnm_gz(env_file))
        return false;
    }
    else if (!img->load_pnm(env_file))
    {
	    char zipname[128];
	    strcpy(zipname, env_file);
	    strcat(zipname, ".gz");
	    if (!img->load_pnm_gz(zipname))
	      return false;
    }
  }
  else
  {
    PRINT_ERR("no world image file");
    return false;
  }

  int width = img->width;
  int height = img->height;
  
  // draw an outline around the background image
  img->draw_box( 0,0,width-1,height-1, 0xFF );
  
  matrix = new CMatrix( width, height );

  wall = new CEntity( this, 0 );
  
  // set up the wall object, as it doesn't have a special class
  wall->m_stage_type = WallType;
  wall->laser_return = LaserSomething;
  wall->sonar_return = true;
  wall->obstacle_return = true;
  wall->vision_return = true;
  wall->puck_return = false; // we trade velocities with pucks

  wall->m_color_desc = WALL_COLOR;
  ColorFromString( &(wall->m_color), WALL_COLOR );
  
  // Copy fixed obstacles into matrix
  //
  for (int y = 0; y < img->height; y++)
    for (int x = 0; x < img->width; x++)
      if (img->get_pixel(x, y) != 0) matrix->set_cell( x,y,wall );

  // kill it!
  if( img ) delete img;
  
  return true;
}


///////////////////////////////////////////////////////////////////////////
// Draw (or undraw) and rectangle in the world grid
void CWorld::SetRectangle(double px, double py, double pth,
                          double dx, double dy, CEntity* ent )
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
    
  matrix->draw_rect( rect, ent );
}


///////////////////////////////////////////////////////////////////////////
// Set a circle in the world grid
//
void CWorld::SetCircle(double px, double py, double pr,
                       CEntity* ent )
{
  // Convert from world to image coords
  //
  int x = (int) (px * ppm);
  int y = (int) (py * ppm);
  int r = (int) (pr * ppm);
    
  matrix->draw_circle( x,y,r,ent );
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
void CWorld::InitRtk(RtkMsgRouter *router)
{
  m_router = router;
}


///////////////////////////////////////////////////////////////////////////
// Process GUI update messages
void CWorld::OnUiDraw(CWorld *world, RtkUiDrawData *data)
{
  // Draw the background
  if (data->draw_back("global"))
    world->DrawBackground(data);

  // Draw grid layers (for debugging)
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
  for (int i = 0; i < world->m_object_count; i++)
    world->m_object[i]->OnUiUpdate(data);
}


///////////////////////////////////////////////////////////////////////////
// Process GUI mouse messages
void CWorld::OnUiMouse(CWorld *world, RtkUiMouseData *data)
{
  data->begin_section("global", "move");

  // Create a mouse mode (used by objects)
  data->mouse_mode("object");
    
  data->end_section();

  // Update the children
  for (int i = 0; i < world->m_object_count; i++)
    world->m_object[i]->OnUiMouse(data);
}


///////////////////////////////////////////////////////////////////////////
// UI property message handler
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
  for (int i = 0; i < world->m_object_count; i++)
    world->m_object[i]->OnUiProperty(data);
}


///////////////////////////////////////////////////////////////////////////
// UI button message handler
void CWorld::OnUiButton(CWorld *world, RtkUiButtonData* data)
{
  data->begin_section("default", "world");

  if (data->check_button("enable", world->m_enable))
    world->m_enable = !world->m_enable;

  if (data->push_button("save"))
    world->Save(NULL);

  data->end_section();
}


///////////////////////////////////////////////////////////////////////////
// Draw the background; i.e. things that dont move
void CWorld::DrawBackground(RtkUiDrawData *data)
{
  data->set_color(RGB(0, 0, 0));
    
  // Loop through the image and draw points individually.
  // Yeah, it's slow, but only happens once.
  for (int y = 0; y < matrix->height; y++)
  {
    for (int x = 0; x < matrix->width; x++)
    {
      if( matrix->is_type(x, y, WallType ) )
      {
	      double px = (double) x / ppm;
	      double py = (double)  y / ppm;
	      double s = 1.0 / ppm;
	      data->rectangle(px, py, px + s, py + s, true);
      }
    }
  }
}


///////////////////////////////////////////////////////////////////////////
// Draw the various layers
void CWorld::DrawDebug(RtkUiDrawData *data, int options)
{
  data->set_color(RTK_RGB(0, 0, 255));
    
  // Loop through the image and draw points individually.
  // Yeah, it's slow, but only happens for debugging
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
          if (entity[i]->vision_return)
            data->rectangle(px, py, px + s, py + s);
        if (options == DBG_SONAR)
          if (entity[i]->sonar_return)
            data->rectangle(px, py, px + s, py + s);
      }
    }
  }
}

#endif // INCLUDE_RTK

  
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
  double gain = 0.05;
  
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
//    printf( " Time: %8.1f - %7.1fHz - [%3.0f/%3.0f] [%3.0f/%3.0f] [%4u/%4u] %8.2f b/sec\r", 
//  	  m_sim_time, 
//  	  freq,
//  	  loop_duration * 1000.0, 
//  	  sleep_duration * 1000.0, 
//  	  avg_loop_duration * 1000.0, 
//  	  avg_sleep_duration * 1000.0,  
//  	  bytes_in, bytes_out, 
//  	  avg_data );

  printf( " Time: %8.1f - %7.2fHz - %8.2f bytes/sec                     \r", 
          m_sim_time, 
          freq,
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




//  // return the matching device
//  CEntity* CWorld::GetEntityByID( int port, int type, int index )
//  {
//    for( int i=0; i<m_object_count; i++ )
//      {
//        if( m_object[i]->m_player_type == type &&
//  	  m_object[i]->m_player_port == port &&
//  	  m_object[i]->m_player_index == index )
	  
//  	return m_object[i]; // success!
//      }
//    return 0; // failed!
//  }


