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
//  $Revision: 1.31 $
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
#include <sys/poll.h>

#include "messages.h" //from player
#include "image.hh"
#include "entity.hh"
#include "truthserver.hh"
#include "playercommon.h"
#include "matrix.hh"

// standard template library container
// This *must* be loaded after everything else, otherwise
// is conflicts with the string template in rtk, *if* you are
// using GCC3.01.  ahoward
//#include <queue> 

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

public:
  // timing
  int m_timer_interval; // the goal real-time interval between stage updates
  int m_timestep; // the sim clock is incremented this much each cycle
  // change ratio of these to run stage faster or slower than real time.

  // Thread control
private: pthread_t m_thread;
  
  // Enable flag -- world only updates while this is set
private: bool m_enable;
  
  // the hostname of this computer
public: char m_hostname[ HOSTNAME_SIZE ];
  
  // pose server stuff ---------------------------------------------
private:
  
  // data for the server-server's listening socket
  struct pollfd m_pose_listen;
  // data for each pose connection
  struct pollfd m_pose_connections[ MAX_POSE_CONNECTIONS ];
  // the number of pose connections
  int m_pose_connection_count;
  
  void ListenForPoseConnections( void );
  void SetupPoseServer( void );
  void DestroyConnection( int con );

  void PoseRead();
  void PoseWrite();
  void InputPose( stage_pose_t &pose );
  void PrintPose( stage_pose_t &pose );
  void PrintSendBuffer( char* send_buf, size_t len );
  
  void PrintTruth( stage_truth_t &truth );

  //-----------------------------------------------------------------------
  // Timing
  // Real time at which simulation started.
  // The time according to the simulator (m_sim_time <= m_start_time).
private: double m_start_time, m_last_time;
private: double m_sim_time;
// the same as m_sim_time but in timeval format
public: struct timeval m_sim_timeval; 

private: double m_max_timestep;
  
  // Update rate (just for diagnostics)
private: double m_update_ratio;
private: double m_update_rate;
  
  // Name of the file the world was loaded from
private: char m_filename[256];
    
  // Name of environment bitmap
private: char m_env_file[256];

  // color definitions
private: char m_color_database_filename[256];

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

  // flags that control servers 
private: bool m_run_environment_server;
private: bool m_run_pose_server;

  // flag that controls spawning of xs
private: bool m_run_xs;
    
  // *** HACK -- this should be made private.  ahoward
public: float ppm;

  // these are set in CWorld::Load() by the 
  //    "set units = {m|dm|cm|mm}"
  //    "set angles = {degrees|radians}" 
  // commands, and are referened in CEntity::Load() to set poses accordingly.
public: float unit_multiplier;
public: float angle_multiplier;
  
  // Scale of fig-based world file
    private: double scale;

  // the pose server port
public: int m_pose_port;

  // the environment server port
public: int m_env_port;

  // an array that maps vision device channels to colors.
  // gets given to the visiondevice constructor which makes a copy
  // the channels are set to defaults
  // which can be overridden in the .world file
  StageColor channel[ ACTS_NUM_CHANNELS ];

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

  // attempt to spawn an XS process
private: void SpawnXS( void );

// fill the Stagecolor structure by looking up the color name in the database
  // return false if failed
public: int ColorFromString( StageColor* color, char* colorString );


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
  // points to a slot at the end of the io buffer where we'll set the time
public: struct timeval* m_time_io;
  
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
  
private: CEntity* CreateObject(const char *type, CEntity *parent);

  /////////////////////////////////////////////////////////////////////////////
  // access methods
  
public:
  double GetWidth( void )
  { if( matrix ) return matrix->width; else return 0; };
  
  double GetHeight( void )
  { if( matrix ) return matrix->height; else return 0; };
  
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

  // Draw the debugging info
  //
private: void DrawDebug(RtkUiDrawData *data, int options);

  // Move an object using the mouse
  //
private: void MouseMove(RtkUiMouseData *data);

  // Debugging options
  //
private: enum {DBG_OBSTACLES, DBG_PUCKS, DBG_LASER, DBG_SONAR, DBG_VISION};
    
  // RTK message router
  //
private: RtkMsgRouter *m_router;
  
#endif
};

#endif

















