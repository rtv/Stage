///////////////////////////////////////////////////////////////////////////
//
// File: world.hh
// Author: Richard Vaughan, Andrew Howard
// Date: 28 Nov 2000
// Desc: top level class that contains everything
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/include/world.hh,v $
//  $Author: vaughan $
//  $Revision: 1.14.2.8 $
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

#include <stddef.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <queue> // standard template library container

#include "image.hh"
#include "entity.hh"
#include "truthserver.hh"
#include "playercommon.h"
#include "matrix.hh"

#define DEBUG

#if INCLUDE_RTK
#include "rtk_ui.hh"
#endif

// forward declaration
//
class CEntity;
class CBroadcastDevice;

// World class
//
class CWorld
{
public: 
  
  // Default constructor
  CWorld();
  
  // Destructor
  virtual ~CWorld();
  
  // the main world-model data structure
  CMatrix *matrix;  
  
  // a general-purpose obstacle entity, used as a brick in the wall
  CEntity* wall;

  // background image
public: Nimage* m_bimg; 
  
  // Thread control
private: pthread_t m_thread;
  
  // Enable flag -- world only updates while this is set
private: bool m_enable;
  
  // Timing
  // Real time at which simulation started.
  // The time according to the simulator (m_sim_time <= m_start_time).
private: double m_start_time, m_last_time;
private: double m_sim_time;
private: double m_max_timestep;
  
  // Update rate (just for diagnostics)
private: double m_update_ratio;
private: double m_update_rate;
  
  // Name of the file the world was loaded from
private: char m_filename[256];
    
  // Name of environment bitmap
private: char m_env_file[256];

  // Object list
private: int m_object_count;
private: int m_object_alloc;
private: CEntity **m_object;

  // Authentication key
public: char m_auth_key[PLAYER_KEYLEN];

  ///////////////////////////////////////////////////////////////////////////
  // Configuration variables

  // Resolution at which to generate laser data
public: double m_laser_res;
public: double m_vision_res; // NYI
    
  // *** HACK -- this should be made private.  ahoward
public: float ppm;
  
  // the truth server queues up any truth updates here, ready to be imported
  // in the main thread
public:
  std::queue<stage_truth_t> input_queue;
  std::queue<stage_truth_t> output_queue; // likewise for outputs
  

    // Load the world
public:  bool Load(const char *filename);
public:  bool Load(){ return Load(m_filename); };
  
  // Save the world
  //
public: bool Save(const char *filename);
public: bool Save() {return Save(m_filename);};
  
  // Initialise the world
public: bool Startup();
  
  // Shutdown the world
public: void Shutdown();
  
  // Start world thread (will call Main)
public: bool StartThread();
  
  // Stop world thread
public: void StopThread();
  
  // Thread entry point for the world
public: static void* Main(void *arg);

  // Update everything
private: void Update();

  // Add an object to the world
public: void AddObject(CEntity *object);

  //////////////////////////////////////////////////////////////////////////
  // Time functions
    
  // Get the simulation time - Returns time in sec since simulation started
public: double GetTime();

  // Get the real time - Returns time in sec since simulation started
public: double GetRealTime();

  ///////////////////////////////////////////////////////////////////////////
  // matrix-based world functions
  
  // Initialise the matrix - loading the bitmap world description
private: bool InitGrids(const char *env_file);
  
public: void SetRectangle(double px, double py, double pth,
			  double dx, double dy, CEntity* ent );
  
public: void SetCircle(double px, double py, double pr,
		       CEntity* ent );
  
  ///////////////////////////////////////////////////////////////////////////
  // Broadcast device functions
  
  // Initialise the broadcast device list
public: void InitBroadcast();
  
  // Add a broadcast device to the list
public: void AddBroadcastDevice(CBroadcastDevice *device);
  
  // Remove a broadcast device from the list
public: void RemoveBroadcastDevice(CBroadcastDevice *device);

  // Get a broadcast device from the list
public: CBroadcastDevice* GetBroadcastDevice(int i);

  // Private list of devices
private: int m_broadcast_count;
private: CBroadcastDevice *m_broadcast[256];
  
  ////////////////////////////////////////////////////////////////
  // shared memory management for interfacing with Player

private: char tmpName[ 512 ]; // path of mmap node in filesystem
public: char* PlayerIOFilename( void ){ return tmpName; };
  
private: caddr_t playerIO;  
private: bool InitSharedMemoryIO( void );
  
private: key_t semKey;
private: int semid; // semaphore access for shared mem locking


  //////////////////////////////////////////////////////////////////
  // methods

  // return a string that names this type of object
public: char* CWorld::StringType( StageType t );
  
  // Create a single semaphore to sync access to the shared memory segments
private: bool CreateShmemLock();

  // Get a pointer to shared mem area
public: void* GetShmem() {return playerIO;};
    
  // lock the shared mem area
public: bool LockShmem( void );

  // Unlock the shared mem area
public: void UnlockShmem( void );
  
public: CEntity* GetEntityByID( int port, int type, int index );
  

  /////////////////////////////////////////////////////////////////////////////
  // access methods
  
public:
  double GetWidth( void )
  { if( m_bimg ) return m_bimg->width; else return 0; };
  
  double GetHeight( void )
  { if( m_bimg ) return m_bimg->height; else return 0; };
  
  int GetObjectCount( void )
  { return m_object_count; };
  
  CEntity* GetObject( int i )
  { if( i < m_object_count ) return (m_object[i]); else  return 0; }; 


  // RTK STUFF ----------------------------------------------------------------
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
};

#endif

















