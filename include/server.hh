
// server.hh
// RTV 23 May 2002
//
// Class provides a network server for Stage internals
// used by external GUIs (XS) and distributed Stage modules
//
// $Id: server.hh,v 1.5 2002-08-22 02:04:38 rtv Exp $

#ifndef _SERVER_H
#define _SERVER_H

#include <sys/poll.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

#include "player.h" // from player
#include "stage_types.hh"
#include "worldfile.hh"
#include "world.hh"


typedef	struct sockaddr SA; // useful abbreviation

// the server reads a header to discover which type of data follows...
enum HeaderType { PosePackets, 
		  EntityPackets, 
		  PropertyPackets, 
		  EnvironmentPackets,
		  MatrixPacket,
		  BackgroundPacket,
		  PixelPackets, 
		  Continue, 
		  ContinueTime, 
		  StageCommand,
		  DownloadComplete
};

typedef struct
{
  HeaderType type; // see enum above
  // the meaning of the data field varies with the message type:
  // for _Packets types, this gives the number of packets to follow
  // for Command types, specifies the command
  // for Continue types, is not used.
  uint32_t data;   
} __attribute ((packed)) stage_header_t;

// COMMANDS - no packet follows; the header's data field is set to one
// of these
enum cmd_t { SAVEc = 1, LOADc, PAUSEc, DOWNLOADc, SUBSCRIBEc };

// this allows us to set a property for an entity
// pretty much any data member of an entity can be set
// using the entity's ::SetProperty() method
typedef struct
{
  uint16_t id; // identify the entity
  EntityProperty property; // identify the property
  uint16_t len; // the property uses this much data (to follow)
} __attribute ((packed)) stage_property_t;

// a client that receives this packet should create a new entity
// using the CWorld::EntityFactory()
typedef struct
{
  int32_t id;
  int32_t parent;
  StageType type;
} __attribute ((packed)) stage_entity_t;

typedef struct
{
  int32_t sizex;
  int32_t sizey;
} __attribute ((packed)) stage_matrix_t;

typedef struct
{
  int32_t sizex;
  int32_t sizey;
  double scale;
} __attribute ((packed)) stage_background_t;

// pose changes are so common that we have a special message for them,
// rather than setting the [x,y,th] properties seperately. i've taken
// out the parent_id field for compactness - you must set that
// property directly - it doesn't happen too often anyway
typedef struct
{
  int16_t stage_id;
  // i changed the x and y to signed ints so that XS can handle negative
  // local coords -BPG
  int32_t x, y; // mm, mm 
  int16_t th; // degrees
  //char echo_request;
  //int16_t parent_id;
} __attribute ((packed)) stage_pose_t;

// XX might have some more special messages to cut down on the property changes
//
//  // describe the background image (CFixedObstacle)
//  // might kill this and just use the properties instead...
//  typedef struct
//  {
//    uint16_t width, height, ppm;
//    //uint16_t num_objects;
//  } __attribute ((packed)) stage_env_t;

//  // set a pixel in the CFixedObstacle
//  // might have to extend this 
//  typedef struct
//  {
//    uint16_t x, y;
//  } __attribute ((packed)) stage_pixel_t;


// this class extends CWorld with a bunch of read/write functions.
// it is never instantiated, but subclassed by Client and Server
class CStageIO : public CWorld
{
private:  

public:
  // simple constructor
  CStageIO( int argc, char** argv, Library* lib );
  ~CStageIO( void );

  // THIS IS THE EXTERNAL INTERFACE TO THE WORLD, SHARED BY ALL WORLD
  // DESCENDANTS
  virtual int Read( void );
  virtual void Update( void );
  virtual void Write( void );
  //virtual void Shutdown( void );


protected:
  ///////////////////////////////////////////////////////////
  // IO data
  int m_port; // the port number of the server server
 
  // the number of synchronous pose connections
  int m_sync_counter; 

  // flag is unset when a DownloadComplete packet is received
  bool m_downloading; 

  // poll data for each pose connection
  struct pollfd m_pose_connections[ MAX_POSE_CONNECTIONS ];

  // subsciption data for each connection 
  char m_dirty_subscribe[ MAX_POSE_CONNECTIONS ];
  // record the type of each connection (sync/async) 
  char m_conn_type[ MAX_POSE_CONNECTIONS ];
  
  // the number of pose connections
  int m_pose_connection_count;

  // called when a connection's fd looks bad - closes the
  // fd and tidies up the connection arrays
  void DestroyConnection( int con );

  // acts on commands
  void HandleCommand( int con, cmd_t cmd );

  // if player has changed the subscription count, we make the property dirty
  void CheckForDirtyPlayerSubscriptions( void );

  // these are the basic IO fucntions
  int ReadPacket( int fd, char* buf, size_t len );
  int WritePacket( int fd, char* buf, size_t len );
  
  // these are wrappers for the packet functions
  // with useful size checks and type conversions
  int ReadHeader( int fd, stage_header_t* hdr  );
  int ReadProperty( int fd, stage_property_t* prop, 
		    char* data, size_t len );
  int ReadEntity( int fd, stage_entity_t* ent );
  int ReadMatrix( int fd );
  int ReadBackground( int fd );

  int WriteHeader( int fd, HeaderType type, uint32_t data );  
  int WriteCommand( int fd, cmd_t cmd );
  int WriteProperty( int fd, stage_property_t* prop, 
		     char* data, size_t len );
  int WriteEntity( int fd, stage_entity_t* ent );

  int WriteMatrix( int fd );
  int WriteBackground( int fd );
  int WriteSubscriptions( int fd );

  // these call the above functions multiple times, Getting and
  // Setting the properties of the entities, etc.
  int ReadProperties( int con, int fd, int num );
  int ReadEntities( int fd, int num );
  int WriteEntities( int fd );
};

class CStageServer : public CStageIO
{
  public:
  CStageServer( int argc, char** argv, Library* lib );
  ~CStageServer( void );
  
  // THIS IS THE EXTERNAL INTERFACE TO THE WORLD, SHARED BY ALL WORLD
  // DESCENDANTS
  virtual int Read( void );

  // check to seee what player has done, then inherits parent's Write()
  virtual void Write( void );
  //virtual void Update( void );
  virtual void Shutdown( void );

  
  ////////////////////////////////////////////////////////////////////
  // CONFIGURATION FILE 
  private: char worldfilename[512];
  
  // Object encapsulating world description file
  private: CWorldFile worldfile;

  // parse and set configs from the argument list
  private: bool ParseCmdLine( int argc, char** argv );

  // Load the world file
  private: bool LoadFile( char* filename );

  // Save the world file
  private: virtual bool SaveFile( char* filename );
  
  ///////////////////////////////////////////////////////////////////
  // SERVER STUFF
  // data for the server-server's listening socket
  struct pollfd m_pose_listen;

  bool SetupConnectionServer( void );
  void ListenForConnections( void );

  /////////////////////////////////////////////////////////////////
  // Player & interface
  // Manage the single player instance
  private: bool StartupPlayer( void );
  private: void ShutdownPlayer( void );
  // The PID of the one player instances
  private: pid_t player_pid;  
  // flag controls whether Player is spawned - set on the command line
  private: bool m_run_player; 

  // shared memory management for interfacing with Player
  public: char* ClockFilename( void ){ return clockName; };
  public: char* DeviceDirectory( void ){ return m_device_dir; };
  
  private: char clockName[PATH_MAX]; // path of mmap node in filesystem
  private: bool CreateClockDevice( void );
  
  // CREATE THE DEVICE DIRECTORY for external IO
  private: bool CreateDeviceDirectory( void );

  //////////////////////////////////////////////////////////////////////
  // RECORD LOCKING 
  // device IO is protected by record locking a single byte of this
  // file for each entity

  // the filename of the lock file
  private: char m_locks_name[PATH_MAX];

// creates the file m_device_dir/devices.lock,  m_object_count bytes long
  // stores the filename in m_locks_name and the fd in m_locks_fd
  private: bool CreateLockFile( void );
 
  //////////////////////////////////////////////////////////////////
  // WHEN LINUX SUPPORTS PROCESS-SHARED SEMAPHORES WE'LL USE THEM AND
  // SCRUB THE RECORD LOCKING - this code will make it happen...

  // Create a single semaphore to sync access to the shared memory segments
  //private: bool CreateShmemLock();

  // Get a pointer to shared mem area
  //public: void* GetShmem() {return playerIO;};
    
  // lock the shared mem area
  //public: bool LockShmem( void );

  // Unlock the shared mem area
  //public: void UnlockShmem( void );

  ///////////////////////////////////////////////////////////////////
  // RTV - I'm working on RTP functionality
  //CRTPPlayer* rtp_player;
};

class CStageClient : public CStageIO
{
public:
  CStageClient( int argc, char** argv, Library* lib );
  ~CStageClient( void );
   
  // THIS IS THE EXTERNAL INTERFACE TO THE WORLD, SHARED BY ALL WORLD
  // DESCENDANTS
  //virtual int Read( void );
  //virtual void Write( void );
  //virtual void Update( void );
  //virtual void Shutdown( void );

  // Save the world file
  protected: virtual bool SaveFile( char* filename );

private:
    
  int WriteCommand( cmd_t cmd )
  { return CStageIO::WriteCommand( m_pose_connections[0].fd, cmd ); };

  // the hostname of the machine we're connecting to
  char m_remotehost[PATH_MAX];
};

#endif // _SERVER_H


