/*************************************************************************
 * world.cc - top level class that contains and updates robots
 * RTV
 * $Id: world.cc,v 1.4.2.6 2000-12-07 22:17:10 ahoward Exp $
 ************************************************************************/

#include <X11/Xlib.h>
#include <assert.h>
#include <fstream.h>
#include <iomanip.h> 
#include <math.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/sem.h>

#define ENABLE_TRACE 1

#include "world.hh"
#include "offsets.h"

#ifndef INCLUDE_RTK
#include "win.h"
#endif

//#undef DEBUG
//#define DEBUG
//#define VERBOSE

const double MILLION = 1000000.0;
const double TWOPI = 6.283185307;


CWorld::CWorld()
        : CObject(this, NULL)
{
    // *** WARNING -- no overflow check
    // Set our id
    //
    strcpy(m_id, "world");

    bots = NULL;

    // initialize the filenames
    bgFile[0] = 0;
    posFile[0] = 0;
  
    paused = false;
  
    // seed the random number generator
    srand48( time(NULL) );

    // Initialise grids
    //
    bimg = NULL;
    img = NULL;
    m_laser_img = NULL;
    m_vision_img = NULL;

    bots = NULL;

    // start the internal clock
    struct timeval tv;
    gettimeofday( &tv, NULL );
    timeNow = timeBegan = tv.tv_sec + (tv.tv_usec/MILLION);

    #ifndef INCLUDE_RTK
        refreshBackground = true;
        Draw(); // this will draw everything properly before we start up
    #endif
}


///////////////////////////////////////////////////////////////////////////
// Destructor
//
CWorld::~CWorld()
{
    // *** WARNING There are lots of things that should be delete here!!
    //
    // ahoward

    if (bimg)
        delete bimg;
    if (img)
        delete img;
    if (m_laser_img)
        delete m_laser_img;
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
    strcpy(bgFile, CSTR(cfg->ReadString("environment_file", "", "")));
    strcpy(posFile, CSTR(cfg->ReadString("position_file", "", "")));
    
    cfg->EndSection();

    // report the files we're using
    cout << "[" << bgFile << "][" << posFile << "]" << endl;
    
    // Initialise the world grids
    //
    if (!StartupGrids())
        return false;

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
    CObject::Update();
    
    struct timeval tv;
      
    gettimeofday( &tv, NULL );
    timeNow = tv.tv_sec + (tv.tv_usec/MILLION);
    timeStep = (timeNow - timeThen); // real time
    //timeStep = 0.01; // step time;   
    //cout << timeStep << endl;
    timeThen = timeNow;

    // use a simple cludge to fix stutters caused by machine load or I/O
    if( timeStep > 0.1 ) 
	{
        TRACE2("MAX TIMESTEP EXCEEDED %f > %f", (double) timeStep, (double) 0.1);
        timeStep = 0.1;
	}

    /* *** HACK -- should reinstate this somewhere ahoward
       if( !runDown ) runStart = timeNow;
       else if( (quitTime > 0) && (timeNow > (runStart + quitTime) ) )
       exit( 0 );
    */
}


///////////////////////////////////////////////////////////////////////////
// Initialise the world grids
//
bool CWorld::StartupGrids()
{
    TRACE0("initialising grids");

    // create a new background image from the pnm file
    bimg = new Nimage( bgFile );
    //cout << "ok." << endl;

    width = bimg->width;
    height = bimg->height;

    // draw an outline around the background image
    bimg->draw_box( 0,0,width-1,height-1, 0xFF );
  
    // create a new forground image copied from the background
    img = new Nimage( bimg );
    
    // Clear laser image
    //
    m_laser_img = new Nimage(width, height);
    m_laser_img->clear(0);
    
    // Copy fixed obstacles into laser rep
    //
    for (int y = 0; y < img->height; y++)
    {
        for (int x = 0; x < img->width; x++)
        {
            if (img->get_pixel(x, y) != 0)
                m_laser_img->set_pixel(x, y, 1);
        }
    }

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
            return img->get_pixel(ix, iy);
        case layer_laser:
            return m_laser_img->get_pixel(ix, iy);
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
            img->set_pixel(ix, iy, value);
            break;
        case layer_laser:
            m_laser_img->set_pixel(ix, iy, value);
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
            img->draw_rect(rect, value);
            break;
        case layer_laser:
            m_laser_img->draw_rect(rect, value);
            break;
        case layer_vision:
            m_vision_img->draw_rect(rect, value);
            break;
    }
}



#ifndef INCLUDE_RTK

void CWorld::SavePos( void )
{
  ofstream out( posFile );

  for( CPlayerRobot* r = bots; r; r = r->next )
  {
    double px, py, pth;
    r->GetGlobalPose(px, py, pth);
    out <<  px <<  '\t' << py << '\t' << pth << endl;
  }
}

float diff( float a, float b )
{
  if( a > b ) return a-b;
  return b-a;
}

CPlayerRobot* CWorld::NearestRobot( float x, float y )
{
  CPlayerRobot* nearest;
  float dist, far = 999999.9;
  
  for( CPlayerRobot* r = bots; r; r = r->next )
  {
    dist = hypot( r->x -  x, r->y -  y );
    
    if( dist < far ) 
      {
	nearest = r;
	far = dist;
      }
  }
  
  return nearest;
}

#else

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
    for (int y = 0; y < img->height; y++)
    {
        for (int x = 0; x < img->width; x++)
        {
            if (img->get_pixel(x, y) != 0)
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
            img = this->img;
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




