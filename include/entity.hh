///////////////////////////////////////////////////////////////////////////
//
// File: entity.hh
// Author: Andrew Howard, Richard Vaughan
// Date: 04 Dec 2000
// Desc: Base class for movable objects
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/include/entity.hh,v $
//  $Author: rtv $
//  $Revision: 1.40 $
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


#ifndef ENTITY_HH
#define ENTITY_HH

#include "stage.h"
#include "stage_types.hh"
#include "truthserver.hh"
#include "colors.hh"
#include "rtp.h"

#include "guiexport.hh" // relic!

#ifdef INCLUDE_RTK2
#include "rtk.h"
#endif

// Forward declare the world class
class CWorld;
class CWorldFile;


///////////////////////////////////////////////////////////////////////////
// The basic object class
class CEntity
{
  // Minimal constructor
  // Requires a pointer to the parent and a pointer to the world.
public: CEntity(CWorld *world, CEntity *parent_object );

  // Destructor
  public: virtual ~CEntity();

  // Load the entity from the worldfile
  public: virtual bool Load(CWorldFile *worldfile, int section);
  
  // Save the entity to the worldfile
  public: virtual bool Save(CWorldFile *worldfile, int section);
  
  // Initialise object
  public: virtual bool Startup( void ); 
  
  // Finalize object
  public: virtual void Shutdown();

  // Get the shared memory size
  private: int SharedMemorySize( void );

  // set and unset the semaphore that protects this entity's shared memory 
protected: bool CEntity::Lock( void );
protected: bool CEntity::Unlock( void );
  // pointer to the  semaphore in the shared memory
  //private: sem_t* m_lock; 

  // IO access to this device is controlled by an advisory lock 
  // on this byte in the shared lock file
  private: int lock_byte; 

  // Update the object's device-specific representation
  public: virtual void Update( double sim_time );

  // Render the entity into the world
  protected: void Map(double px, double py, double pth);

  // Remove the entity from the world
  protected: void UnMap();

  // Remove the entity at its current pose and remap it at a new pose.
  protected: void ReMap(double px, double py, double pth);
  
  // Primitive rendering function using internally
  private: void MapEx(double px, double py, double pth, bool render);

  // Check to see if the given pose will yield a collision with obstacles.
  // Returns a pointer to the first entity we are in collision with.
  // Returns NULL if no collisions.
  // This function is useful for writing position devices.
  protected: virtual CEntity *TestCollision(double px, double py, double pth );
  
  // same; also records the position of the hit
  protected: virtual CEntity *TestCollision(double px, double py, double pth, 
                                            double &hitx, double &hity );

  // Set the color of the entity
  public: void SetColor(const char *desc);
  
  // Convert local to global coords
  public: void LocalToGlobal(double &px, double &py, double &pth);

  // Convert global to local coords
  public: void GlobalToLocal(double &px, double &py, double &pth);

  // Set the objects pose in the parent cs
  public: void SetPose(double px, double py, double pth);

  // Get the objects pose in the parent cs
  public: void GetPose(double &px, double &py, double &pth);

  // Get the objects pose in the global cs
  public: void SetGlobalPose(double px, double py, double pth);

  // Get the objects pose in the global cs
  public: void GetGlobalPose(double &px, double &py, double &pth);

  // Set the objects velocity in the global cs
  // (I cant be bothered implementing local velocities - AH)
  public: void SetGlobalVel(double vx, double vy, double vth);

  // Get the objects velocity in the global cs
  // (I cant be bothered implementing local velocities - AH)
  public: void GetGlobalVel(double &vx, double &vy, double &vth);

  // Get the object mass
  public: double GetMass() { return(m_mass); }
  
  // See if the given entity is one of our descendents
  public: bool IsDescendent(CEntity *entity);

  // Pointer to world
  public: CWorld *m_world;
    
  // Pointer to parent object
  // i.e. the object this object is attached to.
  public: CEntity *m_parent_object;

  // Pointer the default object
  // i.e. the object that would-be children of this object should attach to.
  // This will usually be the object itself.
  public: CEntity *m_default_object;

  // The section in the world file that describes this entity
  public: int worldfile_section;

  // Type of this object
  public: StageType m_stage_type; 

  // Our shape and geometry
  public: StageShape shape;
  public: double origin_x, origin_y;
  public: double size_x, size_y;

  // Our color
  public: StageColor color;

  // Descriptive name for this object
  public: const char *name;

  // Object mass (for collision calculations)
  protected: double m_mass;
  
  // Sensor return values
  // Set these to true to have this object 'seen' by
  // the relevant sensor.
  public: bool obstacle_return;
  public: bool puck_return;
  public: bool sonar_return;  
  public: bool vision_return;
  public: LaserReturn laser_return;
  public: IDARReturn idar_return;

  // the full path name of this device in the filesystem
  public: char m_device_filename[256]; 

  // a filedescriptor for this device's file, used for locking
  //private: int m_fd;

  // flag is set when a dependent device is  attached to this device
  public: bool m_dependent_attached;

  // Initial pose in local cs (ie relative to parent)
  private: double init_px, init_py, init_pth;
  
  // Pose in local cs (ie relative to parent)
  private: double local_px, local_py, local_pth;

  // Velocity in global cs (for coliision calculations)
  protected: double vx, vy, vth;

  // The last mapped pose in global cs
  protected: double map_px, map_py, map_pth;
  
  // how often to update this device, in seconds
  // all devices check this before updating their data
  // instances can modify this in response to config file or messages
  protected: double m_interval; 
  protected: double m_last_update;

  //public: stage_truth_t truth, old_truth;
  public: bool m_dirty[ MAX_POSE_CONNECTIONS ];

  // if we've moved at least 1 pixel or 1 degree, then set the dirty
  // flag
  public: void MakeDirtyIfPixelChanged( void );

  public: void MakeDirty( void )
    {
      memset( m_dirty, true, sizeof(m_dirty[0]) * MAX_POSE_CONNECTIONS );
    };

  public: void MakeClean( void )
    {
      memset( m_dirty, false, sizeof(m_dirty[0]) * MAX_POSE_CONNECTIONS );
    };
  
  // these store the last pose we sent out from the pose server
  // to be tested when setting the dirty flag to see if we really
  // need to send a new pose
  public: int m_last_pixel_x, m_last_pixel_y, m_last_degree;
  
  // recursive function that ORs an ent's dirty array with those of
  // all it's ancestors 
  public: void InheritDirtyFromParent( int con_count );

  ///////////////////////////////////////////////////////////////////////
  // DISTRIBUTED STAGE STUFF

  // the IP address of the host that manages this object
  // replaced the hostname 'cos it's smaller and faster in comparisons
  public: struct in_addr m_hostaddr;

  // flag is true iff this entity is updated by this host
  public: bool m_local; 
  
  ///////////////////////////////////////////////////////////////////////
  // RTP stuff

  CRTPPlayer* rtp_p;

  //////////////////////////////////////////////////////////////////////
  // PLAYER IO STUFF
  
  // Port and index numbers for player
  // identify this device as belonging to the Player on port N at index M
  public: int m_player_port; // N
  public: int m_player_index; // M
  public: int m_player_type; // one of the device types from messages.h

  // Write to the data buffer
  // Returns the number of bytes copied
  // timestamp should be the time the data was created/sensed. if timestamp
  //   is 0, then current time is used
  protected: size_t PutData( void* data, size_t len );

  // Read from the data buffer
  // Returns the number of bytes copied
  public: size_t GetData( void* data, size_t len );

  // struct that holds data for external GUI rendering
  // i made this public so that an object can get another object's
  // type - BPG
  //protected: ExportData exp;
  public: ExportData exp;
   
  // compose and return the exported data
  protected: size_t GetCommand( void* command, size_t len);

  // Read from the configuration buffer
  // Returns the number of bytes copied
  protected: size_t GetConfig( void* config, size_t len);

  // See if the device is subscribed
  // returns the number of current subscriptions
  protected: int Subscribed();

  // builds a truth packet for this entity
  public: void ComposeTruth( stage_truth_t* truth, int index );

  // Pointers into shared mmap for the IO structures
  // the io buffer is allocated by the World 
  // after it has loaded all the Entities (so it knows how 
  // much to allocate). Then the world calls Startup() to allocate 
  // the local storage for each entity. 

  protected: player_stage_info_t *m_info_io;
  protected: uint8_t *m_data_io; 
  protected: uint8_t *m_command_io;
  protected: uint8_t *m_config_io;

  // the sizes of these buffers in bytes
  protected: size_t m_data_len;
  protected: size_t m_command_len;
  protected: size_t m_config_len;
  protected: size_t m_info_len;

#ifdef INCLUDE_RTK2
  // Initialise the rtk gui
  private: void RtkStartup();

  // Finalise the rtk gui
  private: void RtkShutdown();

  // Update the rtk gui
  private: void RtkUpdate();

  // Get a string describing the Stage type of the entity
  private: const char *RtkGetStageType();
  
  // Default figure handle
  protected: rtk_fig_t *fig, *fig_label;

  // Default movement mask
  protected: int movemask;
#endif
};


#endif





