/*
 *  Stage : a multi-robot simulator.
 *  Copyright (C) 2001, 2002 Richard Vaughan, Andrew Howard and Brian Gerkey.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */
/*
 * Desc: Base class for movable entities.
 * Author: Richard Vaughan, Andrew Howard
 * Date: 04 Dec 2000
 * CVS info: $Id: entity.hh,v 1.52.4.1 2002-07-08 02:36:48 rtv Exp $
 */

#ifndef ENTITY_HH
#define ENTITY_HH

#include <stdint.h>

#include "stage.h"
#include "stage_types.hh"
#include "colors.hh"
#include "rtp.h"

#ifdef INCLUDE_RTK2
#include "rtk.h"
#endif

// Forward declare the world class
class CWorld;
class CWorldFile;

// these properties can be set with the SetProperty() method
enum EntityProperty
{
  PropParent, 
  PropSizeX, 
  PropSizeY, 
  PropPoseX, 
  PropPoseY, 
  PropPoseTh, 
  PropOriginX, 
  PropOriginY, 
  PropName,
  PropPlayerId,
  PropPlayerSubscriptions,
  PropColor, 
  PropShape, 
  PropLaserReturn,
  PropSonarReturn,
  PropIdarReturn, 
  PropObstacleReturn, 
  PropVisionReturn, 
  PropPuckReturn,
  PropCommand,
  PropData,
  PropConfig,
  PropReply,
  ENTITY_LAST_PROPERTY // this must be the final property - we use it
 // as a count of the number of properties.
};

// (currently) static memory allocation for getting and setting properties
//const int MAX_NUM_PROPERTIES = 30;
const int MAX_PROPERTY_DATA_LEN = 20000;

///////////////////////////////////////////////////////////////////////////
// The basic moveable object class
class CEntity
{
  // Minimal constructor
  // Requires a pointer to the parent and a pointer to the world.
  public: CEntity(CWorld *world, CEntity *parent_entity );

  // Destructor
  public: virtual ~CEntity();

  // Load the entity from the worldfile
  public: virtual bool Load(CWorldFile *worldfile, int section);
  
  // Save the entity to the worldfile
  public: virtual bool Save(CWorldFile *worldfile, int section);
  
  // Initialise entity
  public: virtual bool Startup( void ); 
  
  // Finalize entity
  public: virtual void Shutdown();

  // Get/set properties
  public: virtual int SetProperty( int con,
                                   EntityProperty property, void* value, size_t len );
  public: virtual int GetProperty( EntityProperty property, void* value );
  
  // Get the shared memory size
  private: int SharedMemorySize( void );

  // set and unset the semaphore that protects this entity's shared memory 
  protected: bool Lock( void );
  protected: bool Unlock( void );
  
  // pointer to the  semaphore in the shared memory
  //private: sem_t* m_lock; 

  // IO access to this device is controlled by an advisory lock 
  // on this byte in the shared lock file
  private: int lock_byte; 

  // Update the entity's device-specific representation
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

  // Convert local to global coords
  public: void LocalToGlobal(double &px, double &py, double &pth);

  // Convert global to local coords
  public: void GlobalToLocal(double &px, double &py, double &pth);

  // Set the entitys pose in the parent cs
  public: void SetPose(double px, double py, double pth);

  // Get the entitys pose in the parent cs
  public: void GetPose(double &px, double &py, double &pth);

  // Get the entitys pose in the global cs
  public: void SetGlobalPose(double px, double py, double pth);

  // Get the entitys pose in the global cs
  public: void GetGlobalPose(double &px, double &py, double &pth);

  // Set the entitys velocity in the global cs
  // (I cant be bothered implementing local velocities - AH)
  public: void SetGlobalVel(double vx, double vy, double vth);

  // Get the entitys velocity in the global cs
  // (I cant be bothered implementing local velocities - AH)
  public: void GetGlobalVel(double &vx, double &vy, double &vth);

  // Get the entity mass
  public: double GetMass() { return (this->mass); }
  
  // See if the given entity is one of our descendents
  public: bool IsDescendent(CEntity *entity);

  // Pointer to world
  public: CWorld *m_world;
    
  // Pointer to parent entity
  // i.e. the entity this entity is attached to.
  public: CEntity *m_parent_entity;

  // Pointer the default entity
  // i.e. the entity that would-be children of this entity should attach to.
  // This will usually be the entity itself.
  public: CEntity *m_default_entity;

  // The section in the world file that describes this entity
  public: int worldfile_section;

  // Type of this entity
  public: StageType stage_type; 

  // Our shape and geometry
  public: StageShape shape;
  public: double origin_x, origin_y;
  public: double size_x, size_y;

  // Our color
  public: StageColor color;

  // Descriptive name for this entity
  public: char name[256];

  // Entity mass (for collision calculations)
  protected: double mass;
  
  // Sensor return values
  // Set these to true to have this entity 'seen' by
  // the relevant sensor.
  public: bool obstacle_return;
  public: bool puck_return;
  public: bool sonar_return;  
  public: bool vision_return;
  public: LaserReturn laser_return;
  public: IDARReturn idar_return;

  // the full path name of this device in the filesystem
  public: char device_filename[256]; 

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
  public: char m_dirty[ MAX_POSE_CONNECTIONS ][ ENTITY_LAST_PROPERTY ];

  // set the dirty flag for each property for each connection
  public: void SetDirty( char v);
  public: void SetDirty( EntityProperty prop, char v );
  public: void SetDirty( int con, char v );
  public: void SetDirty( int con, EntityProperty prop, char v );
  
  // these store the last pose we sent out from the pose server
  // to be tested when setting the dirty flag to see if we really
  // need to send a new pose
  public: int m_last_pixel_x, m_last_pixel_y, m_last_degree;
  
  // recursive function that ORs an ent's dirty array with those of
  // all it's ancestors 
  public: void InheritDirtyFromParent( int con_count );

  ///////////////////////////////////////////////////////////////////////
  // DISTRIBUTED STAGE STUFF

  // the IP address of the host that manages this entity
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
  //public: int m_player_port; // N
  //public: int m_player_index; // M
  //public: int m_player_type; // one of the device types from messages.h
  public: player_device_id_t m_player;

  // basic external IO functions

  // copies data from src to dest (if there is room), updates the
  // timestamp with the current time, sets the availability value to
  // the number of bytes copied
  size_t PutIOData( void* dest, size_t dest_len,  
                    void* src, size_t src_len,
                    uint32_t* ts_sec, uint32_t* ts_usec, uint32_t* avail );
  
  // copies data from src to dest if there is room and if there is the
  // right amount of data available
  size_t GetIOData( void* dest, size_t dest_len,  
                    void* src, uint32_t* avail );
    
  // specific external IO functions

  // Write to the data buffer
  // Returns the number of bytes copied
  // timestamp should be the time the data was created/sensed. if timestamp
  //   is 0, then current time is used
  protected: size_t PutData( void* data, size_t len );

  // Read from the data buffer
  // Returns the number of bytes copied
  public: size_t GetData( void* data, size_t len );
   
  // gets a command from the mmapped command buffer
  // returns the number of bytes copied (on success, this is == len)
  protected: size_t GetCommand( void* command, size_t len);

  // returns number of bytes copied
  protected: size_t PutCommand( void* command, size_t len);

  // Read from the configuration buffer
  // Returns the number of bytes copied
  protected: size_t GetConfig(void** client, void* config, size_t len);

  // Push a configuration reply onto the outgoing queue.
  // Returns 0 on success, non-zero on error.
  protected: size_t PutReply(void* client, unsigned short type,
                             struct timeval* ts, void* reply, size_t len);

  // A short form for zero-length replies.
  // Returns 0 on success, non-zero on error.
  protected: size_t PutReply(void* client, unsigned short type);


  // See if the device is subscribed
  // returns the number of current subscriptions
  //private int player_subs;
   public: int Subscribed();

  // subscribe to / unsubscribe from the device
  // these are used when one device (e.g., lbd) depends on another (e.g.,
  // laser)
  public: void Subscribe();
  public: void Unsubscribe();

  // packages and sends data via rtp
  protected: void AnnounceDataViaRTP( void* data, size_t len );

  // Pointers into shared mmap for the IO structures
  // the io buffer is allocated by the World 
  // after it has loaded all the Entities (so it knows how 
  // much to allocate). Then the world calls Startup() to allocate 
  // the local storage for each entity. 

  protected: player_stage_info_t *m_info_io;
  protected: uint8_t *m_data_io; 
  protected: uint8_t *m_command_io;
  protected: uint8_t *m_config_io;
  protected: uint8_t *m_reply_io;

  // the sizes of these buffers in bytes
  protected: size_t m_data_len;
  protected: size_t m_command_len;
  protected: size_t m_config_len;
  protected: size_t m_reply_len;
  protected: size_t m_info_len;

  // we'll use these entitys to access the configuration request/reply queues
  protected: PlayerQueue* m_reqqueue;
  protected: PlayerQueue* m_repqueue;

  // functions for drawing this entity in GUIs
#ifdef INCLUDE_RTK2
  // Initialise the rtk gui
  protected: virtual void RtkStartup();

  // Finalise the rtk gui
  protected: virtual void RtkShutdown();

  // Update the rtk gui
  public: virtual void RtkUpdate();

  // Default figure handle
  public: rtk_fig_t *fig, *fig_label;

  // Default movement mask
  protected: int movemask;
#endif
};


#endif





