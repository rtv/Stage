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
//  $Revision: 1.1.2.23 $
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

#include <pthread.h>
#include <stdint.h>

#include "image.h"
#include "entity.hh"

#if INCLUDE_RTK
#include "rtk_ui.hh"
#endif

#if INCLUDE_XGUI
#include "xgui.hh"
#endif

// forward declaration
//
class CXGui;
class CEntity;
class CBroadcastDevice;


// Enumerated type describing the layers in the world rep
//
enum EWorldLayer
{
    layer_obstacle = 0,
    layer_laser = 1,
    layer_vision = 2,
};


// World class
//
class CWorld
{
    // Default constructor
    //
    public: CWorld();

    // Destructor
    //
    public: virtual ~CWorld();

    // Load the world
    //
    public: bool Load(const char *filename);
    
    // Save the world
    //
    public: bool Save(const char *filename);
    
    // Initialise the world
    //
    public: bool Startup();

    // Shutdown the world
    //
    public: void Shutdown();

    // Start world thread (will call Main)
    //
    public: bool StartThread();

    // Stop world thread
    //
    public: void StopThread();

    // Thread entry point for the world
    //
    public: static void* Main(void *arg);

    // Update everything
    //
    private: void Update();

    // Add an object to the world
    //
    public: void AddObject(CEntity *object);

    // Return the object nearest a given position
    //
    public: CEntity* NearestObject( double x, double y );

    //////////////////////////////////////////////////////////////////////////
    // Time functions
    
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

    
    ///////////////////////////////////////////////////////////////////////////
    // Laser beacon device stuff

    // Initialise laser beacon representation
    //
    private: void InitLaserBeacon();

    // Add a laser beacon to the world
    // Returns an index for the beacon
    //
    public: int AddLaserBeacon(int id);
    
    // Set the position of a laser beacon
    //
    public: void SetLaserBeacon(int index, double px, double py, double pth);

    // Get the position of a laser beacon
    //
    public: bool GetLaserBeacon(int index, int *id, double *px, double *py, double *pth);

    // Array of laser beacons
    //
    private: int m_laserbeacon_count;
    private: struct
    {
        int m_id;
        double m_px, m_py, m_pth;
    } m_laserbeacon[1000];
    
    
    ///////////////////////////////////////////////////////////////////////////
    // Broadcast device functions

    // Initialise the broadcast device list
    //
    public: void InitBroadcast();

    // Add a broadcast device to the list
    //
    public: void AddBroadcastDevice(CBroadcastDevice *device);

    // Remove a broadcast device from the list
    //
    public: void RemoveBroadcastDevice(CBroadcastDevice *device);

    // Get a broadcast device from the list
    //
    public: CBroadcastDevice* GetBroadcastDevice(int i);

    // Private list of devices
    //
    private: int m_broadcast_count;
    private: CBroadcastDevice *m_broadcast[256];
   

    ///////////////////////////////////////////////////////////////////////////
    // Misc vars

    // Thread control
    //
    private: pthread_t m_thread;

    // Enable flag -- world only updates while this is set
    //
    private: bool m_enable;
    
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

    // Name of the file the world was loaded from
    //
    private: char m_filename[256];
    
    // Name of environment bitmap
    //
    private: char m_env_file[256];

    // Object list
    //
    private: int m_object_count;
    private: CEntity *m_object[1024];
    
    // Obstacle data
    //
    private: Nimage* m_bimg; // background image 
    private: Nimage* m_img; //foreground img;

    // Laser image
    // Store laser data (obstacles and beacons)
    //
    private: Nimage *m_laser_img;

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

    // Initialise rtk
    //
    public: void InitRtk(RtkMsgRouter *router);

    // Process GUI update messages
    //
    public: static void OnUiDraw(CWorld *world, RtkUiDrawData *data);

    // Process GUI mouse messages
    //
    public: static void OnUiMouse(CWorld *world, RtkUiMouseData *data);

    // UI property message handler
    //
    public: static void OnUiProperty(CWorld *world, RtkUiPropertyData* data);

    // UI button message handler
    //
    public: static void OnUiButton(CWorld *world, RtkUiButtonData* data);
    
    // Draw the background; i.e. things that dont move
    //
    private: void DrawBackground(RtkUiDrawData *data);

    // Draw the laser layer
    //
    private: void draw_layer(RtkUiDrawData *data, EWorldLayer layer);

    // Move an object using the mouse
    //
    private: void MouseMove(RtkUiMouseData *data);
    
    // RTK message router
    //
    private: RtkMsgRouter *m_router;
  
#endif

#ifdef INCLUDE_XGUI
public:
  CXGui* win;
  Nimage* GetBackgroundImage( void ){ return m_bimg; };
  Nimage* GetForegroundImage( void ){ return m_img; };
  Nimage* GetLaserImage( void ){ return m_laser_img; };
  Nimage* GetVisionImage( void ){ return m_vision_img; };
#endif

};

#endif
















