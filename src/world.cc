///////////////////////////////////////////////////////////////////////////
//
// File: world.cc
// Author: Richard Vaughan, Andrew Howard
// Date: 7 Dec 2000
// Desc: top level class that contains everything
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/world.cc,v $
//  $Author: gerkey $
//  $Revision: 1.16 $
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

#include <unistd.h>
#include <sys/time.h>
#include <signal.h>
#include <math.h>
#include <iostream.h>
#include "stage.h"
#include "world.hh"

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
    
    memset(m_env_file, 0, sizeof(m_env_file));

    // Initialise the server list
    // Note that this should NOT be dont in StartUp
    //
    InitServer();
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
}


///////////////////////////////////////////////////////////////////////////
// Startup routine 
//
bool CWorld::Startup()
{
    // Initialise the world grids
    //
    if (!InitGrids(m_env_file))
        return false;
    
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

#ifdef INCLUDE_XGUI
    win = new CXGui( this );
    assert( win );

    // must call this at least once before the world is updated so that
    // things are drawn correctly in the first place
    if( win ) win->HandleEvent();
#endif


    // Start all the objects
    //
    for (int i = 0; i < m_object_count; i++)
    {
        if (!m_object[i]->Startup())
            return false;
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
        usleep(10000);

        // Update the world
        //
        if (world->m_enable)
            world->Update();

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

    // Keep track of the sim/real time ratio
    // This is done as a moving window filter so we can see
    // the change over time.
    //
    double a = 0.1;
    m_update_ratio = (1 - a) * m_update_ratio + a * (simtimestep / timestep);
    
    // Keep track of the update rate
    // This is done as a moving window filter so we can see
    // the change over time.
    // Note that we must use the *real* timestep to get sensible results.
    //
    m_update_rate = (1 - a) * m_update_rate + a * (1 / timestep);

    /*
    if((GetRealTime() - last_output_time) >= 1.0)
    {
      printf("Sim time:%8.3fs Real time:%8.3fs Sim/Real:%8.3f Update:%8.3fHz\r",
             m_sim_time, GetRealTime(), m_update_ratio, m_update_rate);
      last_output_time = GetRealTime();
      fflush(stdout);
    }
    */


    // might move this elsewhere and call it less often
    // - will test for responsiveness.
    // must be before the first 
#ifdef INCLUDE_XGUI
    if( win ) win->HandleEvent();
    static double xgui_time = 0;
#endif

    // Do the actual work -- update the objects
    //
    for (int i = 0; i < m_object_count; i++)
   {

     m_object[i]->Update();

#ifdef INCLUDE_XGUI
     double tm = GetRealTime();
     if ( tm - xgui_time > 0.05 ) // update GUI at 20Hz
	  {
            xgui_time = GetRealTime();
	    
	    for (int i = 0; i < m_object_count; i++)
	      win->ImportExportData( m_object[i]->ImportExportData( 0 ) );    
	  }
#endif   
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

    width = m_bimg->width;
    height = m_bimg->height;

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
                
    return true;
}


///////////////////////////////////////////////////////////////////////////
// Get a cell from the world grid
//
uint8_t CWorld::GetCell(double px, double py, EWorldLayer layer)
{
    // Convert from world to image coords
    //
    int ix = (int) (px * ppm);
    int iy = height - (int) (py * ppm);

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
    int iy = height - (int) (py * ppm);

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
    rect.toply = height - (int) (ty * ppm);

    tx = px - cx - sy;
    ty = py - sx + cy;
    rect.toprx = (int) (tx * ppm);
    rect.topry = height - (int) (ty * ppm);

    tx = px - cx + sy;
    ty = py - sx - cy;
    rect.botlx = (int) (tx * ppm);
    rect.botly = height - (int) (ty * ppm);

    tx = px + cx + sy;
    ty = py + sx - cy;
    rect.botrx = (int) (tx * ppm);
    rect.botry = height - (int) (ty * ppm);
    
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
    rect.toply = height - (int) (ty * ppm);

    tx = px - cx - sy;
    ty = py - sx + cy;
    rect.toprx = (int) (tx * ppm);
    rect.topry = height - (int) (ty * ppm);

    tx = px - cx + sy;
    ty = py - sx - cy;
    rect.botlx = (int) (tx * ppm);
    rect.botly = height - (int) (ty * ppm);

    tx = px + cx + sy;
    ty = py + sx - cy;
    rect.botrx = (int) (tx * ppm);
    rect.botry = height - (int) (ty * ppm);
    
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
    int y = height - (int) (py * ppm);
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


///////////////////////////////////////////////////////////////////////////
// Initialise the player server list
//
void CWorld::InitServer()
{
    m_server_count = 0;
}


///////////////////////////////////////////////////////////////////////////
// Register a server by its port number
// Returns false if the number is alread taken
//
bool CWorld::AddServer(int port, CPlayerServer *server)
{
    if (m_server_count >= ARRAYSIZE(m_servers))
        return false;
    if (FindServer(port) != NULL)
        return false;

    m_servers[m_server_count].m_port = port;
    m_servers[m_server_count].m_server = server;
    m_server_count++;
    return true;
}
    

///////////////////////////////////////////////////////////////////////////
// Lookup a server using its port number
//
CPlayerServer *CWorld::FindServer(int port)
{
    for (int i = 0; i < m_server_count; i++)
    {
        if (m_servers[i].m_port == port)
            return m_servers[i].m_server;
    }
    return NULL;
}

///////////////////////////////////////////////////////////////////////////
// Initialise puck representation
//
void CWorld::InitPuck()
{
    m_puck_count = 0;
}


///////////////////////////////////////////////////////////////////////////
// Add a puck to the world
// Returns an index for puck
//
int CWorld::AddPuck(CEntity* puck)
{
    assert(m_puck_count < ARRAYSIZE(m_puck));
    int index = m_puck_count++;
    m_puck[index].puck = puck;
    return index;
}


///////////////////////////////////////////////////////////////////////////
// Get the pointer to a puck
//
CEntity* CWorld::GetPuck(int index)
{
    if (index < 0 || index >= m_puck_count)
        return NULL;
    return(m_puck[index].puck);
}


///////////////////////////////////////////////////////////////////////////
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
    assert(m_laserbeacon_count < ARRAYSIZE(m_laserbeacon));
    int index = m_laserbeacon_count++;
    m_laserbeacon[index].m_id = id;
    return index;
}


///////////////////////////////////////////////////////////////////////////
// Set the position of a laser beacon
//
void CWorld::SetLaserBeacon(int index, double px, double py, double pth)
{
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
                double py = (double) (height - y) / ppm;
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
                double py = (double) (height - y) / ppm;
                double s = 1.0 / ppm;
                data->rectangle(px, py, px + s, py + s);
            }
        }
    }
}


#endif
