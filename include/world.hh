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
//  $Revision: 1.1.2.10 $
//
// Usage:
//  (empty)
//
// Theory of operation:
//  The world object stores all simulation-wide data.  Sensors, for example,
//  interogate the world to detect objects, robots, etc.
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
    layer_beacon = 2,
    layer_vision = 3,
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

    // Get the simulation time
    // Returns time in sec since simulation started
    //
    public: double GetTime();

    // Get the real time
    // Returns time in sec since simulation started
    //
    public: double GetRealTime();

    ///////////////////////////////////////////////////////////////////////////
    // Grid-based world functions
    
    // Initialise the world grids
    //
    private: bool InitGrids(const char *env_file);
  
    // Get a cell from the world grid
    //
    public: uint8_t GetCell(double px, double py, EWorldLayer layer);

    // Set a cell in the world grid
    //
    public: void SetCell(double px, double py, EWorldLayer layer, uint8_t value);

    // Set a rectangle in the world grid
    //
    public: void SetRectangle(double px, double py, double pth,
                              double dx, double dy, EWorldLayer layer, uint8_t value);

    //
    ///////////////////////////////////////////////////////////////////////////

    
    ///////////////////////////////////////////////////////////////////////////
    // Broadcast device functions

    // Initialise the broadcast queue
    //
    public: void InitBroadcast();

    // Add a packet to the broadcast queue
    //
    public: void PutBroadcast(uint8_t *buffer, size_t bufflen);

    // Get a packet from the broadcast queue
    //
    public: size_t GetBroadcast(int *index, uint8_t *buffer, size_t bufflen);

    // The broadcast queue
    //
    private: int m_broadcast_first, m_broadcast_last, m_broadcast_size;
    private: size_t m_broadcast_len[128];
    private: uint8_t m_broadcast_data[128][4096];
    
    //
    ///////////////////////////////////////////////////////////////////////////
    

    // Timing
    // Real time at which simulation started.
    // The time according to the simulator (m_sim_time <= m_start_time).
    //
    private: double m_start_time, m_last_time;
    private: double m_sim_time;
    private: double m_max_timestep;

    // Update rate (just for diagnostics)
    //
    private: double m_update_ratio;
    private: double m_update_rate;
    
    // Obstacle data
    //
    private: Nimage* m_bimg; // background image 
    private: Nimage* m_img; //foreground img;

    // Laser image
    // Store laser data (obstacles and beacons)
    //
    private: Nimage *m_laser_img;

    // Laser beacon image
    // Stores laser beacon id's
    //
    private: Nimage *m_beacon_img;

    // Vision image
    // Stores vision data
    //
    private: Nimage *m_vision_img;

    ///////////////////////////////////////////////////////
    // old stuff here -- ahoward
    // some of these are no longer used.

    private: int width, height, depth;

    // *** HACK -- this should be made private.  ahoward
    //
    public: float ppm;

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




