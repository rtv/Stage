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
 * CVS info: $Id: entity.hh,v 1.15.2.1 2003-01-31 22:35:15 rtv Exp $
 */

#ifndef _ENTITY_HH
#define _ENTITY_HH

#if HAVE_CONFIG_H
  #include <config.h>
#endif
#include <sys/types.h>
#if HAVE_STDINT_H
  #include <stdint.h>
#endif


#include "stage.h"
#include "stage_types.hh"
#include "colors.hh"
#include "library.hh"

#include <netdb.h>
#include <string.h> // for strncpy(3)
#include <sys/types.h>
#include <netinet/in.h>

#ifdef INCLUDE_RTK2
#include "rtk.h"
#endif

//#include "library.hh"
//extern Library* lib;

// Forward declare the world class
//class CWorld;
//class CWorldFile;

///////////////////////////////////////////////////////////////////////////
// The basic moveable object class
class CEntity
{
  // Minimal constructor Requires a pointer to the library entry for
  // this type, a pointer to the world, and a parent
public: CEntity( int id, int parent_id );
  
  // a static named constructor - a pointer to this function is given
  // to the Library object and paired with a string.  When the string
  // is seen in the worldfile, this function is called to create an
  // instance of this entity

  // entities can now be instatiated without deriving a subclass so
  // can be used as obstacles, fiducials etc by configuration from the
  // world file
public: static CEntity* Creator( LibraryItem *libit,  CWorld *world, CEntity *parent )
  {
    return( new CEntity( libit, world, parent ) );
  };
  
  // a linked list of other entities attached to this one
public: CEntity* child_list;
public: CEntity* prev;
public: CEntity* next;
protected:  void AddChild( CEntity* child );


  // everyone shares these vars 
  static double ppm; 
  static CMatrix* matrix;
  static bool enable_gui;

  // this is unique for each object, and is equal to its position in
  // the world's child list
public: stage_id_t stage_id;

  public: void Print( char* prefix );
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
  

  // Update the entity's device-specific representation
  // this is called every time the simulation clock increments
  public: virtual void Update( double sim_time );

  // this is called very rapidly from the main loop
  // it allows the entity to perform some actions between clock increments
  // (such handling config requests to increase synchronous IO performance)
public: virtual void Sync();

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

  // writes a description of this device into the buffer
public: virtual void GetStatusString( char* buf, int buflen );
  
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

public: void GetBoundingBox( double &xmin, double &ymin,
			     double &xmax, double &ymax );
  
  // See if the given entity is one of our descendents
  public: bool IsDescendent(CEntity *entity);

  // subscribe to / unsubscribe from the device
  // these don't do anything by default, but are overridden by CPlayerEntity  
public: virtual int Subscribed();
public: virtual void Subscribe();
public: virtual void Unsubscribe();
  
  // these versions sub/unsub to this device and all its decendants
public: virtual void FamilySubscribe();
public: virtual void FamilyUnsubscribe();

  // Pointer to world
  public: CWorld *m_world;
    
  // Pointer to parent entity
  // i.e. the entity this entity is attached to.
  public: CEntity *m_parent_entity;

  // Pointer the default entity
  // i.e. the entity that would-be children of this entity should attach to.
  // This will usually be the entity itself.
  private: CEntity *m_default_entity;

  // change the parent
  public: void SetParent( CEntity* parent );

  // get the parent
  public: CEntity* GetParent( void ){ return( this->m_parent_entity ); };

  // The section in the world file that describes this entity
  public: int worldfile_section;

  // return a pointer to this or a child if it matches the worldfile section
  CEntity* FindSectionEntity( int section );
    

  // the worldfile token that caused this entity to be created
  // it is set in the constructor (which is called by the library) 
  //protected: char token[STAGE_TOKEN_MAX]; 

  // this is the library's entry for this device, which contains the
  // object's type number, worldfile token, etc.  this can also be
  // used as a type identifier, as it is unique for each library entry
  LibraryItem* lib_entry; 
  
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
  // Set these appropriately to have this entity 'seen' by
  // the relevant sensor.
  public: bool obstacle_return;
  public: bool puck_return;
  public: bool sonar_return;  
  public: bool vision_return;
  public: LaserReturn laser_return;
  public: IDARReturn idar_return;
  public: GripperReturn gripper_return;
  public: int fiducial_return; 

  // the full path name of this device in the filesystem
  //public: char device_filename[256]; 

  // a filedescriptor for this device's file, used for locking
  //private: int m_fd;

  // flag is set when a dependent device is  attached to this device
  public: bool m_dependent_attached;

  // Initial pose in local cs (ie relative to parent)
  protected: double init_px, init_py, init_pth;
  
  // Pose in local cs (ie relative to parent)
  protected: double local_px, local_py, local_pth;

  // Velocity in global cs (for coliision calculations)
  protected: double vx, vy, vth;

  // The last mapped pose in global cs
  protected: double map_px, map_py, map_pth;
  
  // how often to update this device, in seconds
  // all devices check this before updating their data
  // instances can modify this in response to config file or messages
  protected: double m_interval; 
  protected: double m_last_update;


  ///////////////////////////////////////////////////////////////////////
  // DISTRIBUTED STAGE STUFF

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

  // the IP address of the host that manages this entity
  // replaced the hostname 'cos it's smaller and faster in comparisons
  public: struct in_addr m_hostaddr;

  // flag is true iff this entity is updated by this host
  public: bool m_local; 
  
  // functions for drawing this entity in GUIs
#ifdef INCLUDE_RTK2
  // Initialise the rtk gui
  public: virtual void RtkStartup();

  // Finalise the rtk gui
  public: virtual void RtkShutdown();

  // Update the rtk gui
  public: virtual void RtkUpdate();

  // Process mouse events
  public: virtual void RtkOnMouse(rtk_fig_t *fig, int event, int mode);
  
  // Process mouse events (static callback)
  protected: static void StaticRtkOnMouse(rtk_fig_t *fig, int event, int mode);
  
  // Default figure handle
  public: rtk_fig_t *fig, *fig_label;

  // Default movement mask
  protected: int movemask;

  // callbacks - we ask the rtk figures call these when things happen to 'em.
  /* TODO: fix
  static void staticSetGlobalPose( void* ent, double x, double y, double th );
  static void staticSelect( void* ent );
  static void staticUnselect( void* ent );
  */
#endif
  
  // calls the GUI hook to startup this object, then recusively calls
  // the children's GuiStartup()
public: virtual void GuiStartup( void );

public: void *gui_data; 

};


// macro loops over the children
#define CHILDLOOP( ch )\
for( CEntity* ch = this->child_list; ch; ch = ch->next )
    

#endif




