///////////////////////////////////////////////////////////////////////////
//
// File: world.cc
// Author: Richard Vaughan, Andrew Howard
// Date: 7 Dec 2000
// Desc: top level class that contains everything
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/world.cc,v $
//  $Author: vaughan $
//  $Revision: 1.52 $
//
// Usage:
//  (empty)
//
// Theory of operation:
//  (empty)
//
// Known bugs:
//  (empty)
//
// Possible enhancements:
//  (empty)
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

extern int g_interval, g_timestep; // from main.cc

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

// flag set by cmd line to disable xs
extern bool global_no_gui;

// dummy timer signal func
void TimerHandler( int val )
{
  // do nothing! just wake from sleep
  //cout << "WAKE!" << flush;
}  


// main.cc calls constructor, then Load(), then Startup(), then starts thread
// at CWorld::Main();

///////////////////////////////////////////////////////////////////////////
// Default constructor
//
CWorld::CWorld()
{
    // seed the random number generator
    srand48( time(NULL) );

    // AddObject() handles allocation of storage for entities
    // just initialize stuff here
    m_object = NULL;
    m_object_alloc = 0;
    m_object_count = 0;

    // defaults at top of main.cc can be tweaked by command line
    m_timer_interval = g_interval; 
    m_timestep = g_timestep; 


    // Allow the simulation to run
    //
    m_enable = true;

    // init the truth server data structures
    m_truth_connection_count = 0;
    memset( m_truth_connections, 0, sizeof(struct pollfd) * MAX_TRUTH_CONNECTIONS );

    // name this computer
    if( gethostname( m_hostname, sizeof(m_hostname)) == -1)
      {
	perror( "XS: couldn't get hostname. Quitting." );
	exit( -1 );
      }
    
    /* now, strip down to just the hostname */
    char* first_dot;
    if( (first_dot = strchr(m_hostname,'.') ))
      *first_dot = '\0';
    
    //printf( "[Host %s]", m_hostname );
    
    // enable external services by default
    m_run_environment_server = true;
    m_run_truth_server = true;
    
    // default color database file
    strcpy( m_color_database_filename, COLOR_DATABASE );
    
    memset( channel, 0, sizeof(StageColor) * ACTS_NUM_CHANNELS );
    
    // initialize the channel mapping to something reasonable
    channel[0].red = 255;
    channel[1].green = 255;
    channel[2].blue = 255;

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
    //
    //gettimeofday( &m_sim_timeval, 0 );
    
    m_sim_timeval.tv_sec = 90;
    m_sim_timeval.tv_usec = 210;

    m_start_time = m_sim_time = 
      (double)m_sim_timeval.tv_sec + (double)(m_sim_timeval.tv_usec * MILLION);

    // Initialise object list
    //
    m_object_count = 0;
    
    // Initialise configuration variables
    //
    this->ppm = 20;
    this->scale = 100;
    m_laser_res = 0;
    unit_multiplier = 1.0;  // default to meters
    angle_multiplier = M_PI/180.0;  // default to degrees
    
    memset( m_env_file, 0, sizeof(m_env_file));

    // start with no key
    bzero(m_auth_key,sizeof(m_auth_key));

}


///////////////////////////////////////////////////////////////////////////
// Destructor
//
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
//
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

    // make a filename for mmapped IO from a base name and the user's name
  struct passwd* user_info = getpwuid( getuid() );
  sprintf( tmpName, "%s.%s", IOFILENAME, user_info->pw_name );

  // XX handle this better later?
  unlink( tmpName ); // if the file exists, trash it.

  int tfd = open( tmpName, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR );
  
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

  return true;
}

///////////////////////////////////////////////////////////////////////////
// Startup routine 
//
bool CWorld::Startup()
{  
  // we must have at least one object to play with!
  assert( m_object_count > 0 );
  
  // Initialise the broadcast queue
  //
  InitBroadcast();

  // Initialise the real time clock
  // Note that we really do need to set the start time to zero first!
  //
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
  

  // kick off the truth and envirnment servers, unless we disabled them earlier
  if( m_run_environment_server )
    {
      pthread_t tid_dummy;
      pthread_create(&tid_dummy, NULL, &EnvServer, (void *)NULL );  
    }
  
  if( m_run_truth_server )  SetupTruthServer();
  
  // spawn an XS process, unless we disabled it (rtkstage disables xs by default)
  if( !global_no_gui && m_run_xs && m_run_truth_server && m_run_environment_server ) SpawnXS();
  
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
  //
  InitSharedMemoryIO();
  
  // we lock up the memory here
  //LockShmem(); 

  // Setup IO for all the objects - MUST DO THIS AFTER MAPPING SHARING MEMORY
  // each is passed a pointer to the start of its space in shared memory
  int entityOffset = 0;
  
  int first_player_idx = -1;
  int player_count = 0;
  int i;
  for (i = 0; i < m_object_count; i++)
  {
    if (!m_object[i]->SetupIOPointers( playerIO + entityOffset ) )
    {
      cout << "Object " << (int)(m_object[i]) 
              << " failed SetupIOPointers()" << endl;
      return false;
    }
    entityOffset += m_object[i]->SharedMemorySize();


    // count the number of Players on this host, and record the index
    // of the first one.
    // this lets us know that we should start the one Player and go through
    // the next loop to give a port list...
    if(m_object[i]->m_stage_type == PlayerType && 
       (strcmp( m_object[i]->m_hostname, m_hostname ) == 0 ) 
      )
    {
      player_count++;
      if(first_player_idx < 0)
         first_player_idx = i;
    }
  }
  
  // startup any player devices - hacky!
  // we need to have fixed up all the shared memory and pointers before these go
  if(player_count)
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
    for(i = 0; i < m_object_count; i++)
    {
      // again, choose Player type objects managed by this host
      if(m_object[i]->m_stage_type == PlayerType &&
	 (strcmp( m_object[i]->m_hostname, m_hostname ) == 0 ) )
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
  
  // Start the world thread
  //
  if (!StartThread())  
  return false;
  
  //Main( this );
    
  return true;
}
    

///////////////////////////////////////////////////////////////////////////
// Shutdown routine 
//
void CWorld::Shutdown()
{
    // Stop the world thread
    //
    StopThread();
    
    // Shutdown all the objects
    //
    for (int i = 0; i < m_object_count; i++)
    {
      if(m_object[i])
        m_object[i]->Shutdown();
    }
}


///////////////////////////////////////////////////////////////////////////
// Start world thread (will call Main)
//
bool CWorld::StartThread()
{
    // Start the simulator thread
    // 
    if (pthread_create(&m_thread, NULL, &Main, this) < 0)
        return false;
    return true;
}


///////////////////////////////////////////////////////////////////////////
// Stop world thread
//
void CWorld::StopThread()
{
    // Send a cancellation request to the thread
    //
    pthread_cancel(m_thread);

    // Wait forever for thread to terminate
    //
    pthread_join(m_thread, NULL);
}


///////////////////////////////////////////////////////////////////////////
// Thread entry point for the world
//
void* CWorld::Main(void *arg)
{
  CWorld *world = (CWorld*) arg;
    
  // Defer cancel requests so we can handle them properly
  //
  pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);

  // Block signals handled by gui
  //
  sigblock(SIGINT);
  sigblock(SIGQUIT);
  sigblock(SIGHUP);

  // Initialise the GUI
  //
#ifdef INCLUDE_RTK
  double ui_time = 0;
#endif

  // set a timer to go off every few ms. we'll sleep in
  // between if there's nothing else to do
  // sleep will return (almost) immediately if the
  // timer has already gone off. 

  //install signal handler for timing
  if( signal( SIGALRM, &TimerHandler ) == SIG_ERR )
    {
      cout << "Failed to install signal handler" << endl;
      exit( -1 );
    }
    
  //start timer with chosen interval (specified in milliseconds)
  struct itimerval tick;
  // seconds
  tick.it_value.tv_sec = tick.it_interval.tv_sec = 0;
  //    world->m_timer_interval / 1000;
  // microseconds
  tick.it_value.tv_usec = tick.it_interval.tv_usec = 
    world->m_timer_interval * 1000; 
     
  if( setitimer( ITIMER_REAL, &tick, 0 ) == -1 )
    {
      cout << "failed to set timer" << endl;;
      exit( -1 );
    }
    
  while (true)
    {
      // Check for thread cancellation
      //
      pthread_testcancel();

      // look for new connections to the truthserver
      world->ListenForTruthConnections();

      world->TruthRead();
      
      // Update the world
      //
      if (world->m_enable) world->Update();

      world->TruthWrite();


      // dump the contents of the matrix to a file
      //world->matrix->dump();
      //getchar();	

      /* *** HACK -- should reinstate this somewhere ahoward
	 if( !runDown ) runStart = timeNow;
	 else if( (quitTime > 0) && (timeNow > (runStart + quitTime) ) )
	 exit( 0 );
      */
        
      // Update the GUI every 100ms
      //
#ifdef INCLUDE_RTK
      if (world->GetRealTime() - ui_time > 0.050)
        {
	  ui_time = world->GetRealTime();
	  world->m_router->send_message(RTK_UI_FORCE_UPDATE, NULL);
        }
#endif
      // go to sleep until a signal occurs (or a whole second goes by (no way!))
      sleep( 1 ); 
    }
}


///////////////////////////////////////////////////////////////////////////
// Update the world
//
void CWorld::Update()
{
  // this much simulated time has passed since the last update
  //
  double timestep = m_timestep / 1000.0;
  
  // Update the simulation time (in both formats)
  //
  m_sim_time += timestep;
  //m_sim_timeval.tv_sec = (int)floor(m_sim_time);
  //m_sim_timeval.tv_usec = (int)(m_sim_time - floor(m_sim_time)) * MILLION; 

#ifdef WATCH_RATES
    // Keep track of the sim/real time ratio
    // This is done as a moving window filter so we can see
    // the change over time.
  //
  
  static double last_real_time;
  double real_timestep = GetRealTime() - last_real_time;
  
  
  last_real_time += real_timestep;
  
  double a = 0.05;
  m_update_ratio = (1 - a) * m_update_ratio + a * ( timestep / real_timestep );
  
  // Keep track of the update rate
  // This is done as a moving window filter so we can see
  // the change over time.
  // Note that we must use the *real* timestep to get sensible results.
  //
  m_update_rate = (1 - a) * m_update_rate + a * (1.0 / real_timestep);
  
  static int period = 0;
  if( (++period %= 20)  == 0 )
    {
      printf( " %4.0f Hz (%2.1f)\r", m_update_rate,  m_update_ratio );
      fflush( stdout );
    }
#endif
  
  // copy the timeval into the player io buffer
  // use the first object's info
  LockShmem();
  *m_time_io = m_sim_timeval;
  UnlockShmem();

  // Do the actual work -- update the objects 
  for (int i = 0; i < m_object_count; i++)
    {
      // if this host manages this object
      if( m_object[i]->m_local )
	m_object[i]->Update( m_sim_time ); // update it 
    }
}


///////////////////////////////////////////////////////////////////////////
// Get the sim time
// Returns time in sec since simulation started
//
double CWorld::GetTime()
{
    return m_sim_time;
}


///////////////////////////////////////////////////////////////////////////
// Get the real time
// Returns time in sec since simulation started
//
double CWorld::GetRealTime()
{
    struct timeval tv;
    gettimeofday( &tv, NULL );
    double time = tv.tv_sec + (tv.tv_usec / 1000000.0);
    return time - m_start_time;
}


///////////////////////////////////////////////////////////////////////////
// Initialise the world grids
//
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
    cout << "Error: no world image file supplied. Quitting." << endl;
    exit( 1 );
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
  wall->channel_return = 0; // opaque!
  wall->puck_return = false; // we trade velocities with pucks

  strcpy( wall->m_color_desc, WALL_COLOR );
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
// Initialise the broadcast queue
//
void CWorld::InitBroadcast()
{
    m_broadcast_count = 0;
}


///////////////////////////////////////////////////////////////////////////
// Add a broadcast device to the list
//
void CWorld::AddBroadcastDevice(CBroadcastDevice *device)
{
    if (m_broadcast_count >= ARRAYSIZE(m_broadcast))
    {
        printf("warning -- no room in broadcast device list\n");
        return;
    }
    m_broadcast[m_broadcast_count++] = device;
}


///////////////////////////////////////////////////////////////////////////
// Remove a broadcast device from the list
//
void CWorld::RemoveBroadcastDevice(CBroadcastDevice *device)
{
    for (int i = 0; i < m_broadcast_count; i++)
    {
        if (m_broadcast[i] == device)
        {
            memmove(m_broadcast + i,
                    m_broadcast + i + 1,
                    (m_broadcast_count - i - 1) * sizeof(m_broadcast[0]));
            return;
        }
    }
    printf("warning -- broadcast device not found\n");
}


///////////////////////////////////////////////////////////////////////////
// Get a broadcast device from the list
//
CBroadcastDevice* CWorld::GetBroadcastDevice(int i)
{
    if (i < 0 || i >= ARRAYSIZE(m_broadcast))
        return NULL;
    return m_broadcast[i];
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
        printf( "Unable to create semaphore\n" );
        return false;
    }
    if( semctl( semid, 0, SETVAL, argument ) < 0 )
    {
        printf( "Failed to set semaphore value\n" );
        return false;
    }

#ifdef DEBUG
    printf( "Create semaphore: semkey=%d semid=%d\n", semKey, semid );
    
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
      printf("unlock failed return value = %d\n", (int) retval);
}


// return the matching device
CEntity* CWorld::GetEntityByID( int port, int type, int index )
{
  for( int i=0; i<m_object_count; i++ )
    {
      if( m_object[i]->m_player_type == type &&
	  m_object[i]->m_player_port == port &&
	  m_object[i]->m_player_index == index )
	  
	return m_object[i]; // success!
    }
  return 0; // failed!
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
	  sprintf( envbuf, "%d", global_environment_port );

	  char truthbuf[32];
	  sprintf( truthbuf, "%d", global_truth_port );
	  
	  // we assume xs is in the current path
	  if( execlp( "xs", "xs",
		      "-ep", envbuf,
		      "-tp", truthbuf, NULL ) < 0 )
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

int CWorld::ColorFromString( StageColor* color, char* colorString )
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

