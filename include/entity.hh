///////////////////////////////////////////////////////////////////////////
//
// File: entity.hh
// Author: Andrew Howard
// Date: 04 Dec 2000
// Desc: Base class for movable objects
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/include/entity.hh,v $
//  $Author: inspectorg $
//  $Revision: 1.28 $
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

#include "guiexport.hh" // relic!
//#include "messages.h"

#ifdef INCLUDE_RTK
#include "rtk_ui.hh"
#endif

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
  public: CEntity(CWorld *world, CEntity *parent_object);

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

  // Set the io pointers correctly
  private: bool SetupIOPointers( char* io );

  // Get the shared memory size
  private: int SharedMemorySize( void );

  // Update the object's device-specific representation
  public: virtual void Update( double sim_time );

  // Draw ourselves into the world rep
  //REMOVE public: virtual void Map(bool render)
  //  { puts( "DEFAULT MAP" ); };
    
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

  // Shape of this object
  protected: const char *shape_desc;
  protected: StageShape shape;

  // Color of this object
  public: const char* m_color_desc;
  public: StageColor m_color;

  // Descriptive name for this object
  public: const char *m_name;

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
  public: bool idar_return;

  // the full path name of this device in the filesystem
  public: char m_device_filename[256]; 
  
  // flag is set when a dependent device is  attached to this device
  public: bool m_dependent_attached;

  // Object pose in local cs (ie relative to parent)
  private: double m_lx, m_ly, m_lth;

  // dimensions
  public: double m_size_x, m_size_y;
  public: double m_offset_x, m_offset_y; // offset center of rotation

  // The last mapped pose
  protected: double m_map_px, m_map_py, m_map_pth;

  // Object velocity (for coliision calculations)
  protected: double vx, vy, vth;
  
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

  //////////////////////////////////////////////////////////////////////
  // PLAYER IO STUFF
  
  // Port and index numbers for player
  // identify this device as belonging to the Player on port N at index M
  //
  public: int m_player_port; // N
  public: int m_player_index; // M
  public: int m_player_type; // one of the device types from messages.h

  // this is the name of the computer responsible for updating this object
  public: char m_hostname[ HOSTNAME_SIZE ];

  // flag is true iff this entity is updated by this host
  public: bool m_local; 

  // Write to the data buffer
  // Returns the number of bytes copied
  // timestamp should be the time the data was created/sensed. if timestamp
  //   is 0, then current time is used
  //
  protected: size_t PutData( void* data, size_t len );

  // Read from the data buffer
  // Returns the number of bytes copied
  //
  public: size_t GetData( void* data, size_t len );

  // struct that holds data for external GUI rendering
  // i made this public so that an object can get another object's
  // type - BPG
  //protected: ExportData exp;
  public: ExportData exp;
   
  // compose and return the exported data
  //
  protected: size_t GetCommand( void* command, size_t len);

  // Read from the configuration buffer
  // Returns the number of bytes copied
  //
  protected: size_t GetConfig( void* config, size_t len);

  // See if the device is subscribed
  // returns the number of current subscriptions
  protected: int Subscribed();


  // builds a truth packet for this entity
  public: void ComposeTruth( stage_truth_t* truth, int index );

  // base address for this entity's records in shared memory
  //

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

  //////////////////////////////////////////////////////////////////////

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

  // Return true if mouse is over object
  //
  protected: bool IsMouseReady() {return m_mouse_ready;};

  // Move object with the mouse
  //
  public: bool MouseMove(RtkUiMouseData *pData);
    
  // Mouse must be withing this radius for interaction
  //
  protected: double m_mouse_radius;

  // Flag set of object is draggable
  //
  protected: bool m_draggable;

  // Flag set of mouse is within radius
  //
  private: bool m_mouse_ready;

  // Flag set if object is being dragged
  //
  private: bool m_dragging;

  // Object color
  //
  public: RTK_COLOR m_rtk_color;
    
#endif

#ifdef INCLUDE_RTK2
  // Default figure handle
  protected: rtk_fig_t *fig;

  // Default movement mask
  protected: int movemask;
#endif
};


#endif





