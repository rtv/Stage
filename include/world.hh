///////////////////////////////////////////////////////////////////////////
//
// File: world.hh
// Author: Richard Vaughan, Andrew Howard
// Date: 28 Nov 2000
// Desc: top level class that contains everything
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/include/world.hh,v $
//  $Author: ahoward $
//  $Revision: 1.1.2.6 $
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

#ifndef WORLD_HH
#define WORLD_HH

#include "playerrobot.hh"
#include "image.h"

#if INCLUDE_RTK
#include "rtk-ui.hh"
#endif

// forward declaration
//
class CWorldWin;
class CObject;


// Enumerated type describing the layers in the world rep
//
enum EWorldLayer
{
    layer_obstacle = 0,
    layer_laser = 1,
    layer_vision = 2
};


// World class
//
class CWorld : public CObject
{
    // Default constructor
    //
    public: CWorld();

    // Destructor
    //
    public: virtual ~CWorld();
    
    // Startup routine -- creates objects in the world
    //
    public: virtual bool Startup(RtkCfgFile *cfg);

    // Shutdown routine -- deletes objects in the world
    //
    public: virtual void Shutdown();

    // Update everything
    //
    public: virtual void Update();
    
    // Initialise the world grids
    //
    private: bool InitGrids(const char *env_file);
  
    // Get a cell from the world grid
    //
    public: BYTE GetCell(double px, double py, EWorldLayer layer);

    // Set a cell in the world grid
    //
    public: void SetCell(double px, double py, EWorldLayer layer, BYTE value);

    // Set a rectangle in the world grid
    //
    public: void SetRectangle(double px, double py, double pth,
                              double dx, double dy, EWorldLayer layer, BYTE value);

    // Obstacle data
    //
    private: Nimage* m_bimg; // background image 
    private: Nimage* m_img; //foreground img;

    // Laser image
    // Store laser data (obstacles and beacons
    //
    private: Nimage *m_laser_img;

    // Vision image
    // Stores vision data
    //
    private: Nimage *m_vision_img;

    ///////////////////////////////////////////////////////
    // old stuff here -- ahoward
    // some of these are no longer used.

    public:
    int width, height, depth; 
    float ppm;

    double timeStep, timeNow, timeThen, timeBegan;

#ifdef INCLUDE_RTK

    // UI property message handler
    //
    public: virtual void OnUiProperty(RtkUiPropertyData* pData);

    // Process GUI update messages
    //
    public: virtual void OnUiUpdate(RtkUiDrawData *pData);

    // Process GUI mouse messages
    //
    public: virtual void OnUiMouse(RtkUiMouseData *pData);

    // Draw the background; i.e. things that dont move
    //
    private: void DrawBackground(RtkUiDrawData *pData);

    // Draw the laser layer
    //
    private: void DrawLayer(RtkUiDrawData *pData, EWorldLayer layer);
  
#endif
};

#endif




