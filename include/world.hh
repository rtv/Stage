///////////////////////////////////////////////////////////////////////////
//
// File: world.hh
// Author: Richard Vaughan, Andrew Howard
// Date: 28 Nov 2000
// Desc: top level class that contains everything
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/include/world.hh,v $
//  $Author: inspectorg $
//  $Revision: 1.38 $
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
#include "worldfile.hh"

// standard template library container
// This *must* be loaded after everything else, otherwise
// is conflicts with the string template in rtk, *if* you are
// using GCC3.01.  ahoward
//#include <queue> 

#define DEBUG

#if INCLUDE_RTK
#include "rtk_ui.hh"
#endif

#if INCLUDE_RTK2
#include "rtk.h"
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
  public: CMatrix *matrix;

  // Name of worldfile
  private: const char *worldfilename;
  
  // Object encapsulating world description file
  private: CWorldFile worldfile;

  // Properties of the underlying matrix representation
  // for the world.
  // The resolution is specified in pixels-per-meter.
  public: double ppm;

  // Entity representing the fixed environment
  private: CEntity *wall;
  
  public:
  // timing
  //bool m_realtime_mode;
  double m_real_timestep; // the time between wake-up signals in msec
  double m_sim_timestep; // the sim time increment in seconds
  // change ratio of these to run stage faster or slower than real time.

  uint32_t m_step_num; // the number of cycles executed, from 0

  // Thread control
  private: pthread_t m_thread;

  // when to shutdown (in seconds)
  private: int m_stoptime;
  
  // Enable flag -- world only updates while this is set
  private: bool m_enable;
  
  // the hostname of this computer
  public: char m_hostname[ HOSTNAME_SIZE ];
  public: char m_hostname_short[ HOSTNAME_SIZE ];  // same thing, w/out domain
  
  // pose server stuff ---------------------------------------------
  private:
  
  // data for the server-server's listening socket
  struct pollfd m_pose_listen;
  // data for each pose connection
  struct pollfd m_pose_connections[ MAX_POSE_CONNECTIONS ];
  // more data for each pose connection
  //int m_connection_continue[ MAX_POSE_CONNECTIONS ];
  
  // the number of pose connections
  int m_pose_connection_count;
  // the number of synchronous pose connections
  int m_sync_counter; 
  // record the type of each connection (sync/async) 
  char m_conn_type[ MAX_POSE_CONNECTIONS ];

  void ListenForPoseConnections( void );
  void SetupPoseServer( void );
  void DestroyConnection( int con );

  void PoseRead( void );
  
  void Output( double loop_duration, double sleep_duration );
  void LogOutputHeader( void );

  void LogOutput( double freq,
                  double loop_duration, double sleep_duration,
                  double avg_loop_duration, double avg_sleep_duration,
                  unsigned int bytes_in, unsigned int bytes_out,
                  unsigned int total_bytes_in, unsigned int total_bytes_out );

  void ConsoleOutput( double freq,
                      double loop_duration, double sleep_duration,
                      double avg_loop_duration, double avg_sleep_duration,
                      unsigned int bytes_in, unsigned int bytes_out,
                      double avg_data );
    
  void StartTimer(double interval);
  bool ReadHeader( stage_header_t *hdr, int con );
  void PoseWrite();
  void ReadPosePacket( uint32_t num_poses, int con ); 
  void InputPose( stage_pose_t &pose, int connection );
  void PrintPose( stage_pose_t &pose );
  void PrintSendBuffer( char* send_buf, size_t len );
  
  void PrintTruth( stage_truth_t &truth );
  void HandlePoseConnections( void );
  

  //-----------------------------------------------------------------------
  // Timing
  // Real time at which simulation started.
  // The time according to the simulator (m_sim_time <= m_start_time).
  private: double m_start_time, m_last_time;
  private: double m_sim_time;
  // the same as m_sim_time but in timeval format
  public: struct timeval m_sim_timeval; 
  
  public: bool m_log_output, m_console_output;

  int m_log_fd; // logging file descriptor
  char m_log_filename[256]; // path to the log file
  char m_cmdline[512]; // a string copy of the command line that started stage

  //private: double m_max_timestep;
  
  // Update rate (just for diagnostics)
  private: double m_update_ratio;
  private: double m_update_rate;
  
  // Name of the file the world was loaded from
  private: char m_filename[256];
    
  // color definitions
  private: char m_color_database_filename[256];

  // Object list
  private: int m_object_count;
  private: int m_object_alloc;
  private: CEntity **m_object;

  // Authentication key
  public: char m_auth_key[PLAYER_KEYLEN];

  // if the user is running more than one copy of stage, this number
  // identifies this instance uniquely
  int m_instance;

  // The PID of the one player instances
  private: pid_t player_pid;


  ///////////////////////////////////////////////////////////////////////////
  // Configuration variables

  // flags that control servers
  public: bool m_env_server_ready;
  private: bool m_run_environment_server;
  private: bool m_run_pose_server;
 
  private: bool m_external_sync_required;
  

  // flag that controls spawning of xs
  private: bool m_run_xs;
    
  // the pose server port
  public: int m_pose_port;

  // the environment server port
  public: int m_env_port;

  public: bool ParseCmdline( int argv, char* argv[] );
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

  // Start the one player instance
  private: bool StartupPlayer( void );

  // Stop the one player instance
  private: void ShutdownPlayer( void );

  // attempt to spawn an XS process
  private: void SpawnXS( void );
  
  // main update loop
  public: void Main( void );

  // Update everything
  private: void Update();

  // Add an object to the world
  public: void AddObject(CEntity *object);

  // fill the Stagecolor structure by looking up the color name in the database
  // return false if failed
  public: int ColorFromString( StageColor* color, const char* colorString );


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

  // Update a rectangle in the matrix
  // Set add = true to add the entity, set add = false to remove it.
  public: void SetRectangle(double px, double py, double pth,
                            double dx, double dy, CEntity* ent, bool add );

  // Update a circle in the matrix
  // Set add = true to add the entity, set add = false to remove it.
  public: void SetCircle(double px, double py, double pr,
                         CEntity* ent, bool add );
  
  
  ////////////////////////////////////////////////////////////////
  // shared memory management for interfacing with Player

  public: char m_device_dir[ 512 ]; //device  directory name 

  private: char clockName[ 512 ]; // path of mmap node in filesystem
  public: char* ClockFilename( void ){ return clockName; };
  public: char* DeviceDirectory( void ){ return m_device_dir; };
  
  private: bool CreateClockDevice( void );
  // export the time in this buffer
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
  //public: void* GetShmem() {return playerIO;};
    
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

  // returns true if the given hostname matches our hostname, false otherwise
  bool CheckHostname(char* host);


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

#ifdef INCLUDE_RTK2
  // Initialise the GUI
  private: bool LoadGUI(CWorldFile *worldfile);

  // Start the GUI
  private: bool StartupGUI();

  // Stop the GUI
  private: void ShutdownGUI();

  // Basic GUI elements
  public: rtk_app_t *app;
  public: rtk_canvas_t *canvas;

  // Some menu items
  private: rtk_menuitem_t *save_menuitem;
#endif
  
};

#endif

















