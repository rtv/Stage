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
//  $Revision: 1.4.2.18 $
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
#include "stage.h"
#include "world.hh"
#include "objectfactory.hh"


///////////////////////////////////////////////////////////////////////////
// Default constructor
//
CWorld::CWorld()
{
    // seed the random number generator
    srand48( time(NULL) );

    // Initialise grids
    //
    m_bimg = NULL;
    m_img = NULL;
    m_laser_img = NULL;
    m_beacon_img = NULL;
    m_vision_img = NULL;

    // Initialise clocks
    //
    m_start_time = m_sim_time = 0;

    memset(m_env_file, 0, sizeof(m_env_file));
}


///////////////////////////////////////////////////////////////////////////
// Destructor
//
CWorld::~CWorld()
{
    if (m_bimg)
        delete m_bimg;
    if (m_img)
        delete m_img;
    if (m_laser_img)
        delete m_laser_img;
    if (m_beacon_img)
        delete m_beacon_img;
    if (m_vision_img)
        delete m_vision_img;
}


///////////////////////////////////////////////////////////////////////////
// Load the world
//
bool CWorld::Load(const char *filename)
{
    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        printf("unable to open world file %s; ignoring\n", (char*) filename);
        return false;
    }

    int count = 0;
    CObject *parent = NULL;
    
    while (true)
    {
        // Get a line
        //
        char line[1024];
        if (fgets(line, sizeof(line), file) == NULL)
            break;
        count++;

        // Parse the line into tokens
        // Tokens are delimited by spaces
        //
        int argc = 0;
        char *argv[400];
        char *token = line;
        while (true)
        {
            assert((size_t) argc < sizeof(argv) / sizeof(argv[0]));
            argv[argc] = strsep(&token, " \t\n");
            if (token == NULL)
                break;
            if (argv[argc][0] != 0)
                argc++;
        }

        // Debugging -- dump the tokens
        //
        for (int i = 0; i < argc; i++)
            printf("[%s]", (char*) argv[i]);
        printf("\n");

        // Skip blank lines
        //
        if (argc == 0)
            continue;
        
        // Ignore comment lines
        //
        if (argv[0][0] == '#')
            continue;

        // Look for open block tokens
        //
        if (strcmp(argv[0], "{") == 0)
        {
            if (m_object_count > 0)
                parent = m_object[m_object_count - 1];
            else
                printf("line %d : misplaced '{'\n", (int) count + 1);
        }

        // Look for close block tokens
        //
        else if (strcmp(argv[0], "}") == 0)
        {
            if (parent != NULL)
                parent = parent->m_parent;
            else
                printf("line %d : extra '}'\n", (int) count + 1);
        }

        // Parse "set" command
        // set <variable> = <value>
        //
        else if (argc == 4 && strcmp(argv[0], "set") == 0 && strcmp(argv[2], "=") == 0)
        {
            if (strcmp(argv[1], "environment_file") == 0)
                strcpy(m_env_file, argv[3]);
            else if (strcmp(argv[1], "pixels_per_meter") == 0)
                ppm = atof(argv[3]);
            else
                printf("line %d : variable %s is not defined\n",
                       (int) count + 1, (char*) argv[1]);  
        }

        // Parse "create" command
        // create <type> ...
        //
        else if (argc >= 2 && strcmp(argv[0], "create") == 0)
        {
            // Create the object
            //
            CObject *object = ::CreateObject(argv[1], this, parent);
            if (object != NULL)
            {
                // Let the object initialise itself
                //
                object->init(argc - 2, argv + 2);

                // Add to list of objects
                //
                m_object[m_object_count++] = object;
            }
            else
                printf("line %d : object type %s is not defined\n",
                       (int) count + 1, (char*) argv[1]);
        }
    }
    
    fclose(file);
    return true;
}


///////////////////////////////////////////////////////////////////////////
// Save objects to a file
//
bool CWorld::Save(const char *filename)
{
    FILE *file = fopen(filename, "w+");
    if (file == NULL)
    {
        printf("unable to create world file %s; ignoring\n", (char*) filename);
        return false;
    }

    int depth = 0;
    for (int i = 0; i < m_object_count; i++)
    {
        CObject *object = m_object[i];

        // Put in start/end block tokens
        //
        while (object->m_depth > depth)
        {
            fprintf(file, "{\n");
            depth++;
        }
        while (object->m_depth < depth)
        {
            fprintf(file, "}\n");
            depth--;
        }

        // Let object prepare line to save
        //
        char line[1024];
        memset(line, 0, sizeof(line));
        object->Save(line, sizeof(line));

        // Write line to file
        //
        for (int j = 0; j < depth; j++)
            fputs("    ", file);
        fputs(line, file);
        fputs("\n", file);
    }

    // Close any trailing blocks
    //
    while (depth > 0)
    {
        fprintf(file, "}\n");
        depth--;
    }
   
    fclose(file);
    return true;
}


///////////////////////////////////////////////////////////////////////////
// Startup routine 
//
bool CWorld::Startup()
{
    // Initialise the world grids
    //
    printf("[%s]\n", m_env_file);
    if (!InitGrids(m_env_file))
        return false;

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
        m_object[i]->Shutdown();
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
void* CWorld::Main(CWorld *world)
{
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
            world->m_router->send_message(RTK_UI_FORCE_UPDATE, NULL);
            ui_time = world->GetRealTime();
        }
        #endif
    }
}


///////////////////////////////////////////////////////////////////////////
// Update the world
//
void CWorld::Update()
{
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
        printf("warning: max timestep exceeded (%f > %f)\n",
               (double) simtimestep, (double) m_max_timestep);
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

    // Do the actual work -- update the objects
    //
    for (int i = 0; i < m_object_count; i++)
        m_object[i]->Update();
}


///////////////////////////////////////////////////////////////////////////
// Get the sim time
// Returns time in sec since simulation started
//
double CWorld::get_time()
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
    m_bimg = new Nimage( env_file );
    //cout << "ok." << endl;

    width = m_bimg->width;
    height = m_bimg->height;

    // draw an outline around the background image
    m_bimg->draw_box( 0,0,width-1,height-1, 0xFF );
  
    // create a new forground image copied from the background
    m_img = new Nimage( m_bimg );
    
    // Clear laser image
    //
    m_laser_img = new Nimage(width, height);
    m_laser_img->clear(0);
    
    // Copy fixed obstacles into laser rep
    //
    for (int y = 0; y < m_img->height; y++)
    {
        for (int x = 0; x < m_img->width; x++)
        {
            if (m_img->get_pixel(x, y) != 0)
                m_laser_img->set_pixel(x, y, 1);
        }
    }

    // Clear beacon image
    //
    m_beacon_img = new Nimage(width, height);
    m_beacon_img->clear(0);
    
    // Clear vision image
    //
    m_vision_img = new Nimage(width, height);
    m_vision_img->clear(0);
                
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
            return m_img->get_pixel(ix, iy);
        case layer_laser:
            return m_laser_img->get_pixel(ix, iy);
        case layer_beacon:
            return m_beacon_img->get_pixel(ix, iy);
        case layer_vision:
            return m_vision_img->get_pixel(ix, iy);
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
            m_img->set_pixel(ix, iy, value);
            break;
        case layer_laser:
            m_laser_img->set_pixel(ix, iy, value);
            break;
        case layer_beacon:
            m_beacon_img->set_pixel(ix, iy, value);
            break;
        case layer_vision:
            m_vision_img->set_pixel(ix, iy, value);
            break;
    }
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
            m_img->draw_rect(rect, value);
            break;
        case layer_laser:
            m_laser_img->draw_rect(rect, value);
            break;
        case layer_beacon:
            m_beacon_img->draw_rect(rect, value);
            break;
        case layer_vision:
            m_vision_img->draw_rect(rect, value);
            break;
    }
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
    
    RtkFormat1(value, "%7.3f", (double) world->get_time());
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

    if (data->push_button("save"))
        world->Save("temp.world"); // ***
    
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
// Draw the laser layer
//
void CWorld::draw_layer(RtkUiDrawData *data, EWorldLayer layer)
{
    Nimage *img = NULL;

    // This could be cleaned up by having an array of images
    //
    switch (layer)
    {
        case layer_obstacle:
            img = m_img;
            break;
        case layer_laser:
            img = m_laser_img;
            break;
        default:
            return;
    }
    
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
                data->set_pixel(px, py, RGB(0, 0, 255));
            }
        }
    }
}


#endif




