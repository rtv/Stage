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
//  $Revision: 1.21.2.2 $
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
#include <iostream.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pwd.h>

//#include <stage.h>
#include "world.hh"

#define SEMKEY 2000

#define WATCH_RATES
//#define DEBUG 
//#define VERBOSE
#undef DEBUG 
#undef VERBOSE

// right now i have a single fixed filename for the mmapped IO with Player
// eventually we'll be able to run multiple stages, each with a different
// suffix - for now its IOFILENAME.$USER
#define IOFILENAME "/tmp/stageIO"

// main.cc calls constructor, then Load(), then Startup(), then starts thread
// at CWorld::Main();

///////////////////////////////////////////////////////////////////////////
// Default constructor
//
CWorld::CWorld()
{
    // seed the random number generator
    srand48( time(NULL) );

    // Allow the simulation to run
    //
    m_enable = true;
    
    // Initialise world filename
    //
    m_filename[0] = 0;
    
    // Initialise grids
    //
    m_bimg = NULL;
    m_obs_img = NULL;
    m_laser_img = NULL;
    m_vision_img = NULL;
    m_puck_img = NULL;

    // Initialise clocks
    //
    m_start_time = m_sim_time = 0;

    // Initialise configuration variables
    //
    m_laser_res = 0;
    
    memset( m_env_file, 0, sizeof(m_env_file));

    // the current truth is unpublished
    m_truth_is_current = false;

    // start with no key
    bzero(m_auth_key,sizeof(m_auth_key));

}


///////////////////////////////////////////////////////////////////////////
// Destructor
//
CWorld::~CWorld()
{
    if (m_bimg)
        delete m_bimg;
    if (m_obs_img)
        delete m_obs_img;
    if (m_laser_img)
        delete m_laser_img;
    if (m_vision_img)
        delete m_vision_img;
    if (m_puck_img)
        delete m_puck_img;
    
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
  size_t areaSize = (size_t)mem;//TOTAL_SHARED_MEMORY_BUFFER_SIZE;

#ifdef DEBUG
  cout << "Shared memory allocation: " << areaSize 
       << " (" << mem << ")" << endl;
#endif

  // HMMM, WE REALLY SHOULD THINK ABOUT ALLOWING STAGE TO RUN MORE
  // THAN ONE COPY PER MACHINE! REQUIRES SOME INTELLIGENT PORT# HANDLING

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
    // Initialise the laser beacon rep
    //
    InitLaserBeacon();
    
    // Initialise the broadcast queue
    //
    InitBroadcast();

    // Initialise the real time clock
    // Note that we really do need to set the start time to zero first!
    //
    m_start_time = 0;
    m_start_time = GetRealTime();
    m_last_time = 0;

    // Initialise the simulator clock
    //
    m_sim_time = 0;
    m_max_timestep = 0.1;

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
    
    // Start the world thread
    //
    if (!StartThread())
        return false;

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
    
    // Make our priority real low
    //
    //nice(10);
    
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
    
    while (true)
    {
        // Check for thread cancellation
        //
        pthread_testcancel();

        // Dont hog all the cycles -- sleep for 10ms
        //
	// RTV - linux seems to usleep for a minimum of 10ms.
	// so values below 10000 microseconds here don't make any difference.
	// by usleeping at all here you get a max 50Hz update rate.
        usleep(1000);

        // Update the world
        //
        if (world->m_enable)
	  world->Update();
	
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
    }
}


///////////////////////////////////////////////////////////////////////////
// Update the world
//
void CWorld::Update()
{
  //static double last_output_time = 0;

    // Compute elapsed real time
    //
    double timestep = GetRealTime() - m_last_time;
    m_last_time += timestep;
    
    // Place an upper bound on the simulator timestep
    // This may make us run slower than real-time.
    //
    double simtimestep = timestep;
    if (timestep > m_max_timestep)
    {
        //PRINT_MSG2("warning: max timestep exceeded (%f > %f)",
                   //(double) simtimestep, (double) m_max_timestep);
        simtimestep = m_max_timestep;
    }

    // Update the simulation time
    //
    m_sim_time += simtimestep;

#ifdef WATCH_RATES
    // Keep track of the sim/real time ratio
    // This is done as a moving window filter so we can see
    // the change over time.
    //
    double a = 0.05;
    m_update_ratio = (1 - a) * m_update_ratio + a * (simtimestep / timestep);
    
    // Keep track of the update rate
    // This is done as a moving window filter so we can see
    // the change over time.
    // Note that we must use the *real* timestep to get sensible results.
    //
    m_update_rate = (1 - a) * m_update_rate + a * (1 / timestep);

//      if((GetRealTime() - last_output_time) >= 1.0)
//      {
//        printf("Sim time:%8.3fs Real time:%8.3fs Sim/Real:%8.3f Update:%8.3fHz\r",
//               m_sim_time, GetRealTime(), m_update_ratio, m_update_rate);
//        last_output_time = GetRealTime();
//        fflush(stdout);
//      }


    static int period = 0;
    if( (++period %= 20)  == 0 )
      {
	printf( "  %.2f Hz (%.2f)\r", m_update_rate, m_update_ratio );
	fflush( stdout );
      }
#endif

    // import any truths that were queued up by the truth server thread
    //printf( "update_queue size: %d\n", update_queue.size() );

    while( !input_queue.empty() )
      {
	stage_truth_t truth = input_queue.front(); // copy the front object
	input_queue.pop(); // remove the front object

//  #ifdef DEBUG
//  	printf( "De-queued Truth: "
//  		"(%d,%d,%d) parent (%d,%d,%d) [%d,%d,%d]\n", 
//  		truth.id.port, 
//  		truth.id.type, 
//  		truth.id.index,
//  		truth.parent.port, 
//  		truth.parent.type, 
//  		truth.parent.index,
//  		truth.x, truth.y, truth.th );
	
//  	fflush( stdout );
//  #endif

	// find the matching entity
	// (this implies that these ID fields cannot be changed externally)
	
	// see if this is a stage directive 
	
	if( truth.stage_id == 0 ) // its a command for the engine!
	  {
	    switch( truth.x ) // used to identify the command
	      {
		//case LOADc: Load( m_filename ); break;
	      case SAVEc: Save( m_filename ); break;
	      case PAUSEc: m_enable = !m_enable; break;
	      default: printf( "Stage Warning: "
			       "Received unknown command (%d); ignoring.\n",
			       truth.x );
	      }
	      continue;
	  }

	assert( (CEntity*)truth.stage_id ); // should be good -otherwise a bug

	//printf( "PTR: %d\n", truth.stage_id ); fflush( stdout );

	CEntity* ent = (CEntity*)truth.stage_id;
 
	  //GetEntityByID( truth.id.port, 
	  //      truth.id.type,
	  //    truth.id.index );

	assert( ent ); // there really ought to be one!
	
	//ent->Map( false );
	
	// update the entity with the truth
	ent->SetGlobalPose( truth.x/1000.0, truth.y/1000.0, 
			    DTOR(truth.th) );
	
	ent->truth_poked = 1;

	//ent->Map( true );

	// the parent may have been changed - NYI
	//ent->parent->port = truth.parent.port;
	//ent->parent.type = truth.parent.type;
	//ent->parent.index = truth.parent.index;

	// width and height could be changed here too if necessary

	//ent->m_publish_truth = true; // re-export this new truth
      }
    
    // Do the actual work -- update the objects 
    for (int i = 0; i < m_object_count; i++)
      {
	// update
	m_object[i]->Update( m_sim_time ); 
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
    
  m_bimg = new Nimage;
  
  int len = strlen( env_file );
  
  if( len > 0 )
    {
      if( strcmp( &(env_file[ len - 3 ]), ".gz" ) == 0 )
	{
	  if (!m_bimg->load_pnm_gz(env_file))
	    return false;
	}
      else
	if (!m_bimg->load_pnm(env_file))
	  {
	    char zipname[128];
	    strcpy(zipname, env_file);
	    strcat(zipname, ".gz");
	    if (!m_bimg->load_pnm_gz(zipname))
	      return false;
	  }
    }
  else
    {
      cout << "Error: no world image file supplied. Quitting." << endl;
      exit( 1 );
    }

  int width = m_bimg->width;
  int height = m_bimg->height;
  
  // draw an outline around the background image
  m_bimg->draw_box( 0,0,width-1,height-1, 0xFF );
  
  // Clear obstacle image
  //
  m_obs_img = new Nimage(width, height);
  m_obs_img->clear(0);
    
  // Clear laser image
  //
  m_laser_img = new Nimage(width, height);
  m_laser_img->clear(0);
  
  // Copy fixed obstacles into laser rep
  //
  for (int y = 0; y < m_bimg->height; y++)
    {
        for (int x = 0; x < m_bimg->width; x++)
        {
            if (m_bimg->get_pixel(x, y) != 0)
            {
                m_obs_img->set_pixel(x, y, 0xFF);
                m_laser_img->set_pixel(x, y, 0xFF);
            }
        }
    }

  // Clear vision image
  //
  m_vision_img = new Nimage(width, height);
  m_vision_img->clear(0);
    
  // Clear puck image
  //
  m_puck_img = new Nimage(width, height);
  m_puck_img->clear(0);
                

  matrix = new CMatrix( width, height );

  wall = new CEntity( this, 0 );
  
  wall->m_stage_type = NullType;

  wall->laser_return = 1;
  wall->sonar_return = 1;
  wall->obstacle_return = 1;

  // Copy fixed obstacles into matrix
  //
  for (int y = 0; y < m_bimg->height; y++)
    for (int x = 0; x < m_bimg->width; x++)
      if (m_bimg->get_pixel(x, y) != 0) matrix->set_cell( x,y,wall );

  return true;
}


///////////////////////////////////////////////////////////////////////////
// Get a cell from the world grid
//
CEntity** CWorld::GetEntityAtCell(double px, double py )
{
  cout << "DOUBLE CALLED" << endl;

  // Convert from world to image coords
  //
  int ix = (int) (px * ppm);
  int iy = (int) (py * ppm);

  CEntity** ent = matrix->get_cell( ix, iy );

  //if( ent )
  //printf( "reading %p at %d,%d\n", ent, ix, iy );

  return ent;

}

///////////////////////////////////////////////////////////////////////////
// Get a cell from the world grid
//
CEntity** CWorld::GetEntityAtCell( int ix, int iy )
{
  //cout << endl << ix << ' ' << iy << flush;
  CEntity** ent = matrix->get_cell( ix, iy );
  
  //if( ent )
  //printf( "reading %p at %d,%d\n", ent, ix, iy );
  
  return ent;

}

///////////////////////////////////////////////////////////////////////////
// Get a cell from the world grid
//
void CWorld::SetEntityAtCell( CEntity* ent, double px, double py )
{
  // Convert from world to image coords
  //
  int ix = (int) (px * ppm);
  int iy = (int) (py * ppm);
  
    matrix->set_cell( ix, iy, ent );
    
  printf( "setting %p at %d,%d\n", ent, ix, iy );

}

///////////////////////////////////////////////////////////////////////////
// Get a cell from the world grid
//
void CWorld::SetEntityAtCell( CEntity* ent, int ix, int iy )
{
  matrix->set_cell( ix, iy, ent );
 
  printf( "setting %p at %d,%d\n", ent, ix, iy );
}


///////////////////////////////////////////////////////////////////////////
// Get a cell from the world grid
//
uint8_t CWorld::GetCell(double px, double py, EWorldLayer layer)
{
    // Convert from world to image coords
    //
    int ix = (int) (px * ppm);
    int iy = (int) (py * ppm);

    // This could be cleaned up by having an array of images
    //
    switch (layer)
    {
        case layer_obstacle:
            return m_obs_img->get_pixel(ix, iy);
        case layer_laser:
            return m_laser_img->get_pixel(ix, iy);
        case layer_vision:
            return m_vision_img->get_pixel(ix, iy);
        case layer_puck:
            return m_puck_img->get_pixel(ix, iy);
    }
    return 0;
}


///////////////////////////////////////////////////////////////////////////
// Set a cell in the world grid
//
void CWorld::SetCell(double px, double py, EWorldLayer layer, uint8_t value)
{
    // Convert from world to image coords
    //
    int ix = (int) (px * ppm);
    int iy = (int) (py * ppm);

    // This could be cleaned up by having an array of images
    //
    switch (layer)
    {
        case layer_obstacle:
            m_obs_img->set_pixel(ix, iy, value);
            break;
        case layer_laser:
            m_laser_img->set_pixel(ix, iy, value);
            break;
        case layer_vision:
            m_vision_img->set_pixel(ix, iy, value);
            break;
        case layer_puck:
            m_puck_img->set_pixel(ix, iy, value);
            break;
    }
}


///////////////////////////////////////////////////////////////////////////
// Get a rectangle in the world grid
//
uint8_t CWorld::GetRectangle(double px, double py, double pth,
                             double dx, double dy, EWorldLayer layer)
{
    Rect rect;
    double tx, ty;

    dx /= 2;
    dy /= 2;

    double cx = dx * cos(pth);
    double cy = dy * cos(pth);
    double sx = dx * sin(pth);
    double sy = dy * sin(pth);
    
    // This could be faster
    //
    tx = px + cx - sy;
    ty = py + sx + cy;
    rect.toplx = (int) (tx * ppm);
    rect.toply = (int) (ty * ppm);

    tx = px - cx - sy;
    ty = py - sx + cy;
    rect.toprx = (int) (tx * ppm);
    rect.topry = (int) (ty * ppm);

    tx = px - cx + sy;
    ty = py - sx - cy;
    rect.botlx = (int) (tx * ppm);
    rect.botly = (int) (ty * ppm);

    tx = px + cx + sy;
    ty = py + sx - cy;
    rect.botrx = (int) (tx * ppm);
    rect.botry = (int) (ty * ppm);
    
    // This could be cleaned up by having an array of images
    //
    switch (layer)
    {
        case layer_obstacle:
            return m_obs_img->rect_detect(rect);
        case layer_laser:
            return m_laser_img->rect_detect(rect);
        case layer_vision:
            return m_vision_img->rect_detect(rect);
        case layer_puck:
            return m_puck_img->rect_detect(rect);
    }
    return 0;
}


void CWorld::SetRectangle(double px, double py, double pth,
                          double dx, double dy, CEntity* ent )
{
    Rect rect;
    double tx, ty;

    dx /= 2.0;
    dy /= 2.0;

    double cx = dx * cos(pth);
    double cy = dy * cos(pth);
    double sx = dx * sin(pth);
    double sy = dy * sin(pth);
    
    // This could be faster
    //
    tx = px + cx - sy;
    ty = py + sx + cy;
    rect.toplx = (int) (tx * ppm);
    rect.toply = (int) (ty * ppm);

    tx = px - cx - sy;
    ty = py - sx + cy;
    rect.toprx = (int) (tx * ppm);
    rect.topry = (int) (ty * ppm);

    tx = px - cx + sy;
    ty = py - sx - cy;
    rect.botlx = (int) (tx * ppm);
    rect.botly = (int) (ty * ppm);

    tx = px + cx + sy;
    ty = py + sx - cy;
    rect.botrx = (int) (tx * ppm);
    rect.botry = (int) (ty * ppm);
    
    matrix->draw_rect( rect, ent );
}

///////////////////////////////////////////////////////////////////////////
// Set a rectangle in the world grid
//
void CWorld::SetRectangle(double px, double py, double pth,
                          double dx, double dy, EWorldLayer layer, uint8_t value)
{
    Rect rect;
    double tx, ty;

    dx /= 2;
    dy /= 2;

    double cx = dx * cos(pth);
    double cy = dy * cos(pth);
    double sx = dx * sin(pth);
    double sy = dy * sin(pth);
    
    // This could be faster
    //
    tx = px + cx - sy;
    ty = py + sx + cy;
    rect.toplx = (int) (tx * ppm);
    rect.toply = (int) (ty * ppm);

    tx = px - cx - sy;
    ty = py - sx + cy;
    rect.toprx = (int) (tx * ppm);
    rect.topry = (int) (ty * ppm);

    tx = px - cx + sy;
    ty = py - sx - cy;
    rect.botlx = (int) (tx * ppm);
    rect.botly = (int) (ty * ppm);

    tx = px + cx + sy;
    ty = py + sx - cy;
    rect.botrx = (int) (tx * ppm);
    rect.botry = (int) (ty * ppm);
    
    // This could be cleaned up by having an array of images
    //
    switch (layer)
    {
        case layer_obstacle:
            m_obs_img->draw_rect(rect, value);
            break;
        case layer_laser:
            m_laser_img->draw_rect(rect, value);
            break;
        case layer_vision:
            m_vision_img->draw_rect(rect, value);
            break;
        case layer_puck:
            m_puck_img->draw_rect(rect, value);
            break;
    }
}
    
///////////////////////////////////////////////////////////////////////////
// Set a circle in the world grid
//
void CWorld::SetCircle(double px, double py, double pr,
                       EWorldLayer layer, uint8_t value)
{
    // Convert from world to image coords
    //
    int x = (int) (px * ppm);
    int y = (int) (py * ppm);
    int r = (int) (pr * ppm);
    
    // This could be cleaned up by having an array of images
    //
    switch (layer)
    {
        case layer_obstacle:
            m_obs_img->draw_circle(x,y,r,value);
            break;
        case layer_laser:
            m_laser_img->draw_circle(x,y,r,value);
            break;
        case layer_vision:
            m_vision_img->draw_circle(x,y,r,value);
            break;
        case layer_puck:
            m_puck_img->draw_circle(x,y,r,value);
            break;
    }
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
    assert(m_object_count < ARRAYSIZE(m_object));
    m_object[m_object_count++] = object;
}


///////////////////////////////////////////////////////////////////////////
// Find the object nearest to the mouse
//
CEntity* CWorld::NearestObject( double x, double y )
{
  CEntity* nearest = 0;
  double dist, far = 9999999.9;

  double ox, oy, oth;

  for (int i = 0; i < m_object_count; i++)
    {
      m_object[i]->GetGlobalPose( ox, oy, oth );;
      
      dist = hypot( ox-x, oy-y );
      
      if( dist < far ) 
	{
	  nearest = m_object[i];
	  far = dist;
	}
    }
  
  return nearest;
}

// Initialise laser beacon representation
//
void CWorld::InitLaserBeacon()
{
    m_laserbeacon_count = 0;
}


///////////////////////////////////////////////////////////////////////////
// Add a laser beacon to the world
// Returns an index for the beacon
//
int CWorld::AddLaserBeacon(int id)
{
  //puts( "ADDING A LASER BEACON" );

    assert(m_laserbeacon_count < ARRAYSIZE(m_laserbeacon));
    int index = m_laserbeacon_count++;
    m_laserbeacon[index].m_id = id;

    //printf( "now have %d beacons\n", index );
    return index;
}


///////////////////////////////////////////////////////////////////////////
// Set the position of a laser beacon
//
void CWorld::SetLaserBeacon(int index, double px, double py, double pth)
{
  //  printf( "SETTING BEACON POS %.2f %.2f %.2f\n", 
  //  px, py, pth );
 
  ASSERT(index >= 0 && index < m_laserbeacon_count);
    m_laserbeacon[index].m_px = px;
    m_laserbeacon[index].m_py = py;
    m_laserbeacon[index].m_pth = pth;
}


///////////////////////////////////////////////////////////////////////////
// Get the position of a laser beacon
//
bool CWorld::GetLaserBeacon(int index, int *id, double *px, double *py, double *pth)
{
     if (index < 0 || index >= m_laserbeacon_count)
        return false;
    *id = m_laserbeacon[index].m_id;
    *px = m_laserbeacon[index].m_px;
    *py = m_laserbeacon[index].m_py;
    *pth = m_laserbeacon[index].m_pth;
    return true;
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
        world->draw_layer(data, layer_obstacle);
        
    if (data->draw_layer("laser", false))
        world->draw_layer(data, layer_laser);

    if (data->draw_layer("vision", false))
        world->draw_layer(data, layer_vision);

    if (data->draw_layer("puck", false))
        world->draw_layer(data, layer_puck);
    
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
    for (int y = 0; y < m_bimg->height; y++)
    {
        for (int x = 0; x < m_bimg->width; x++)
        {
            if (m_bimg->get_pixel(x, y) != 0)
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
void CWorld::draw_layer(RtkUiDrawData *data, EWorldLayer layer)
{
    Nimage *img = NULL;

    // This could be cleaned up by having an array of images
    //
    switch (layer)
    {
        case layer_obstacle:
            img = m_obs_img;
            break;
        case layer_laser:
            img = m_laser_img;
            break;
        case layer_vision:
            img = m_vision_img;
            break;            
        case layer_puck:
            img = m_puck_img;
            break;            
        default:
            return;
    }

    data->set_color(RTK_RGB(0, 0, 255));
    
    // Loop through the image and draw points individually.
    // Yeah, it's slow, but only happens for debugging
    //
    for (int y = 0; y < img->height; y++)
    {
        for (int x = 0; x < img->width; x++)
        {
            if (img->get_pixel(x, y) != 0)
            {
                double px = (double) x / ppm;
                double py = (double) y / ppm;
                double s = 1.0 / ppm;
                data->rectangle(px, py, px + s, py + s);
            }
        }
    }
}

#endif // INCLUDE_RTK










