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
//  $Revision: 1.4.2.13 $
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

#define ENABLE_TRACE 0

#include <sys/time.h>
#include "world.hh"
#include "offsets.h"


///////////////////////////////////////////////////////////////////////////
// Default constructor
//
CWorld::CWorld()
        : CObject(this, NULL)
{
    // *** WARNING -- no overflow check
    // Set our id
    //
    strcpy(m_id, "world");
  
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

    /* *** REMOVE
    // start the internal clock
    struct timeval tv;
    gettimeofday( &tv, NULL );
    timeNow = timeThen = timeBegan = tv.tv_sec + (tv.tv_usec/MILLION);
    */
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
// Startup routine 
//
bool CWorld::Startup(RtkCfgFile *cfg)
{
    // Call the objects startup function
    //
    if (!CObject::Startup(cfg))
        return false;

    // Load useful settings from config file
    //
    cfg->BeginSection("world");
    ppm = cfg->ReadInt("pixels_per_meter", 25, "");
    RtkString env_file = cfg->ReadString("environment_file", "", "");
    RtkString pos_file = cfg->ReadString("position_file", "", "");
    cfg->EndSection();

    // Display the files we are reading
    //
    printf("[%s] [%s]\n", CSTR(env_file), CSTR(pos_file));
    
    // Initialise the world grids
    //
    if (!InitGrids(CSTR(env_file)))
        return false;

    // Initialise the broadcast queue
    //
    InitBroadcast();

    // Read robot positions from file
    //
    /* *** TODO
    if (!ReadPos(pos_file))
        return false;
    */

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

    return true;
}


///////////////////////////////////////////////////////////////////////////
// Shutdown routine 
//
void CWorld::Shutdown()
{        
    CObject::Shutdown();
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
        TRACE2("MAX TIMESTEP EXCEEDED %f > %f", (double) simtimestep, (double) m_max_timestep);
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
    
    CObject::Update();

    /* *** HACK -- should reinstate this somewhere ahoward
       if( !runDown ) runStart = timeNow;
       else if( (quitTime > 0) && (timeNow > (runStart + quitTime) ) )
       exit( 0 );
    */
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
    TRACE0("initialising grids");

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
BYTE CWorld::GetCell(double px, double py, EWorldLayer layer)
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
void CWorld::SetCell(double px, double py, EWorldLayer layer, BYTE value)
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
                          double dx, double dy, EWorldLayer layer, BYTE value)
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
    m_broadcast_first = 0;
    m_broadcast_last = -1;
    m_broadcast_size = ARRAYSIZE(m_broadcast_data);

    memset(m_broadcast_len, 0, sizeof(m_broadcast_len));
    memset(m_broadcast_data, 0, sizeof(m_broadcast_data));
}


///////////////////////////////////////////////////////////////////////////
// Add a packet to the broadcast queue
//
void CWorld::PutBroadcast(BYTE *buffer, size_t bufflen)
{
    int index = m_broadcast_last++;

    // Make this a circular queue -- things at the front get overwritten
    //
    if (m_broadcast_last - m_broadcast_first + 1 > m_broadcast_size)
        m_broadcast_first++;
    ASSERT(m_broadcast_last - m_broadcast_first + 1 <= m_broadcast_size);
    
    BYTE *packet = m_broadcast_data[index % m_broadcast_size];
    size_t *packetlen = &m_broadcast_len[index % m_broadcast_size];
    
    // Check for buffer overflow
    //
    *packetlen = bufflen;
    if (*packetlen > ARRAYSIZE(m_broadcast_data[0]))
    {
        *packetlen = ARRAYSIZE(m_broadcast_data[0]);
        TRACE0("warning : data buffer too large; data has been truncated");
    }
   
    // Copy the data
    //
    memcpy(packet, buffer, *packetlen);
}


///////////////////////////////////////////////////////////////////////////
// Get a packet from the broadcast queue
//
size_t CWorld::GetBroadcast(int *index, BYTE *buffer, size_t bufflen)
{
    if (*index < m_broadcast_first)
        *index = m_broadcast_first;
    if (*index > m_broadcast_last)
        *index = m_broadcast_last;

    // See if there is anything in the queue
    //
    if (m_broadcast_last - m_broadcast_first + 1 == 0)
        return 0;

    BYTE *packet = m_broadcast_data[(*index) % m_broadcast_size];
    size_t packetlen = m_broadcast_len[(*index) % m_broadcast_size];

    // Check for buffer overflow
    //
    if (packetlen > bufflen)
    {
        packetlen = bufflen;
        TRACE0("warning : data buffer too small; data has been truncated");
    }

    // Copy the data
    //
    memcpy(buffer, packet, packetlen);
    return packetlen;
}


#ifdef INCLUDE_RTK


///////////////////////////////////////////////////////////////////////////
// UI property message handler
//
void CWorld::OnUiProperty(RtkUiPropertyData* pData)
{
    CObject::OnUiProperty(pData);

    RtkString value;

    RtkFormat1(value, "%7.3f", (double) GetTime());
    pData->AddItemText("Simulation time (s)", CSTR(value), "");
    
    RtkFormat1(value, "%7.3f", (double) GetRealTime());
    pData->AddItemText("Real time (s)", CSTR(value), "");

    RtkFormat1(value, "%7.3f", (double) m_update_ratio);
    pData->AddItemText("Sim/real time", CSTR(value), ""); 
    
    RtkFormat1(value, "%7.3f", (double) m_update_rate);
    pData->AddItemText("Update rate (Hz)", CSTR(value), ""); 
}


///////////////////////////////////////////////////////////////////////////
// Process GUI update messages
//
void CWorld::OnUiUpdate(RtkUiDrawData *pData)
{
    // Draw the background
    //
    if (pData->DrawBack("global"))
        DrawBackground(pData);

    // Draw grid layers (for debugging)
    //
    pData->BeginSection("global", "debugging");

    if (pData->DrawLayer("obstacle", false))
        DrawLayer(pData, layer_obstacle);
        
    if (pData->DrawLayer("laser", false))
        DrawLayer(pData, layer_laser);

    if (pData->DrawLayer("vision", false))
        DrawLayer(pData, layer_vision);
    
    pData->EndSection();

    // Get children to process message
    //
    CObject::OnUiUpdate(pData);
}


///////////////////////////////////////////////////////////////////////////
// Process GUI mouse messages
//
void CWorld::OnUiMouse(RtkUiMouseData *pData)
{
    // Get children to process message
    //
    CObject::OnUiMouse(pData);
}


///////////////////////////////////////////////////////////////////////////
// Draw the background; i.e. things that dont move
//
void CWorld::DrawBackground(RtkUiDrawData *pData)
{
    TRACE0("drawing background");

    pData->SetColor(RGB(0, 0, 0));
    
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
                pData->Rectangle(px, py, px + s, py + s);
            }
        }
    }
}


///////////////////////////////////////////////////////////////////////////
// Draw the laser layer
//
void CWorld::DrawLayer(RtkUiDrawData *pData, EWorldLayer layer)
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
                pData->SetPixel(px, py, RGB(0, 0, 255));
            }
        }
    }
}


#endif




