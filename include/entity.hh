///////////////////////////////////////////////////////////////////////////
//
// File: entity.hh
// Author: Andrew Howard
// Date: 04 Dec 2000
// Desc: Base class for movable objects
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/include/entity.hh,v $
//  $Author: vaughan $
//  $Revision: 1.8 $
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

#include "guiexport.hh" // relic!
//#include "messages.h"

#ifdef INCLUDE_RTK
#include "rtk_ui.hh"
#endif

// Forward declare the world class
//
class CWorld;


///////////////////////////////////////////////////////////////////////////
// The basic object class
//
class CEntity
{
    // Minimal constructor
    // Requires a pointer to the parent and a pointer to the world.
    //
    public: CEntity(CWorld *world, CEntity *parent_object);
  
  StageType m_stage_type; // distinct from the player types found in messages.h

    // Destructor
    //
    public: virtual ~CEntity();

    // Load the object from an argument list
    //
    public: virtual bool Load(int argc, char **argv);

    // Save the object to an argument list
    //
    public: virtual bool Save(int &argc, char **argv);

    // Initialise object
    //
    public: virtual bool Startup( void ); 

    // Set the io pointers correctly
    //
    public: virtual bool SetupIOPointers( char* io );

    // Finalize object
    //
    public: virtual void Shutdown();
    
    // Fetch commands & truth, update the object, then publish truth
    //
  //public: virtual void ImportUpdateExport();

    // Update the object's device-specific representation
    //
    public: virtual void Update( double sim_time );

    // Draw ourselves into the world rep
    //
  public: virtual void Map(bool render)
  { puts( "DEFAULT MAP" ); };
    
 
    // Convert local to global coords
    //
    public: void LocalToGlobal(double &px, double &py, double &pth);

    // Convert global to local coords
    //
    public: void GlobalToLocal(double &px, double &py, double &pth);

    // Set the objects pose in the parent cs
    //
    public: void SetPose(double px, double py, double pth);

    // Get the objects pose in the parent cs
    //
    public: void GetPose(double &px, double &py, double &pth);

    // Get the objects pose in the global cs
    //
    public: void SetGlobalPose(double px, double py, double pth);

    // Get the objects pose in the global cs
    //
    public: void GetGlobalPose(double &px, double &py, double &pth);

  // flag is set when a dependent device is  attached to this device
    public: bool m_dependent_attached;

public: int truth_poked;

    // Line in the world description file
    //
    public: int m_line;
    public: int m_column;

    // Type of this object
    //
    public: char m_type[64];
    
    // Id of this object
    //
    public: char m_id[64];

    // Pointer to world
    //
    public: CWorld *m_world;
    
    // Pointer to parent object
    // i.e. the object this object is attached to.
    //
    public: CEntity *m_parent_object;

    // Pointer the default object
    // i.e. the object that would-be children of this object should attach to.
    // This will usually be the object itself.
    //
    public: CEntity *m_default_object;

    // Object pose in local cs (ie relative to parent)
    //
    private: double m_lx, m_ly, m_lth;

    // dimensions
    //
    public: double m_size_x, m_size_y;
    public: double m_offset_x, m_offset_y; // offset center of rotation

    // Object color description (for display)(and maybe vision sensors later?)
    //
    private: char m_color_desc[128];

  // the apparent color of this robot to a vision system like ACTS 
    // (not yet implemented - get to it!)
    public: int m_channel; 

    // how often to update this device, in seconds
    // all devices check this before updating their data
    // instances can modify this in response to config file or messages
    protected: double m_interval; 
    protected: double m_last_update;

  //////////////////////////////////////////////////////////////////////
  // PLAYER IO STUFF
  
    // Port and index numbers for player
    // identify this device as belonging to the Player on port N at index M
    //
    public: int m_player_port; // N
    public: int m_player_index; // M
    public: int m_player_type; // one of the device types from messages.h
  
    public: int SharedMemorySize( void );

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

    // Commanded speed
    //
    protected: double m_com_vr, m_com_vth;
    protected: double m_mass;

    public: double GetSpeed() { return(m_com_vr); }
    public: double GetMass() { return(m_mass); }
    
    // shouldn't really have this, but...
    public: void SetSpeed(double speed) { m_com_vr=speed; }

    // struct that holds data for external GUI rendering
    // i made this public so that an object can get another object's
    // type - BPG
    //protected: ExportData exp;
    public: ExportData exp;
   
    // compose and return the export data structure
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
  public: void ComposeTruth( stage_truth_t* truth );

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
    public: RTK_COLOR m_color;
    
#endif
};


#endif





