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
 * CVS info: $Id: entity.hh,v 1.15.2.19 2003-02-24 04:47:12 rtv Exp $
 */

#ifndef _ENTITY_HH
#define _ENTITY_HH

#include "stage.h"
#include "colors.hh"
#include "library.hh"

#include <netinet/in.h> // for struct in_addr

#ifdef INCLUDE_RTK2
#include "rtk.h"
#endif

#include "gui.hh"

// Forward declare
class CMatrix;

///////////////////////////////////////////////////////////////////////////
// The basic moveable object class
class CEntity
{
  // Minimal constructor Requires a pointer to the library entry for
  // this type, a pointer to the world, and a parent
public: CEntity(  int id, char* token, char* color, CEntity* parent );

public: static CEntity* Creator( int id, char* token, char* color, CEntity* parent )
  {
    return( new CEntity( id, token, color, parent ) );
  };

  // a linked list of other entities attached to this one
public: CEntity* child_list;
public: CEntity* prev;
public: CEntity* next;
protected:  void AddChild( CEntity* child );



  // everyone shares these vars 
public:
  //static double ppm; 
  static CMatrix* matrix;
  static bool enable_gui;

public: 
  static double simtime; // the simulation time in seconds
  static double timestep; // the duration of one update in seconds

public:
  static CEntity* root; // global reference to the base object

  // this is unique for each object, and is equal to its position in
  // the world's child list
public: stage_id_t stage_id;

  // Destructor
  public: virtual ~CEntity();

  //int Subs(){ return sub_count; };
  //int sub_count;

  // SUBCLASS INTERFACE ------------------------------------------
protected:
  stage_buffer_t buffer_data;
  stage_buffer_t buffer_cmd;

public: 
  // Initialise entity  
  virtual int Startup( void ); 
  // Finalize entity
  virtual int Shutdown();  


  // Update the entity's device-specific representation
  // this is called every time the simulation clock increments
  virtual int Update();
  
  // this is called very rapidly from the main loop
  // it allows the entity to perform some actions between clock increments
  // (such handling config requests to increase synchronous IO performance)
  virtual int Sync();
  
protected: 
  virtual int SetData( char* data, size_t len ); // receive data
  virtual int GetData( char* data, size_t* len ); // get data data
  virtual int SetCommand( char* data, size_t len ); // receive command
  virtual int GetCommand( char* data, size_t* len ); // get command data
  // END ---------------------------------------------------------
  
  // Get/set properties
public: virtual int Property( int con,
		      stage_prop_id_t property, 
		      void* value, size_t len, 
		      stage_buffer_t* reply );
  
protected:
  stage_rotrect_t* rects;
  int rect_count;
  
  // copies the array of rects into this entity, allocating storage
  // and setting the rects_max members correctly
  void SetRects( stage_rotrect_t* rects, int num );
  
  // scale an array of rects into a unit square
  void NormalizeRects( stage_rotrect_t* rects, int num );
  
  // convert the rotated rectangle into global coords, taking into account
  // the entities pose and the rectangle scaling
  void GetGlobalRect( stage_rotrect_t* dest, stage_rotrect_t* src );
  
  // bool controls whether rects are added to or removed from the matrix
  void RenderRects(  bool render );
  
  // Render the entity into the world
  protected: void Map(double px, double py, double pth);
  
  // calls Map(double,double,double) with the current global pose
  protected: void Map( void );

  // Remove the entity from the world
  protected: void UnMap();

  // Remove the entity at its current pose and remap it at a new pose.
  protected: void ReMap(double px, double py, double pth);

  // maps myself and my children, recursively
  protected: void MapFamily(void);
  // unmaps myself and my children, recursively
  protected: void UnMapFamily(void);
 
  // Primitive rendering function using internally
  private: void MapEx(double px, double py, double pth, bool render);

  // Check to see if the given pose will yield a collision with obstacles.
  // Returns a pointer to the first entity we are in collision with.
  // Returns NULL if no collisions. records the position of the hit in
  // hitx and hity, if non-null
protected: CEntity *TestCollision(double px, double py, double pth,
				  double *hity = NULL, double* hity = NULL);  
  
  // writes a description of this device into the buffer
public: void GetStatusString( char* buf, int buflen );
  
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
  
  // these versions sub/unsub to this device and all its decendants
public: void FamilySubscribe();
public: void FamilyUnsubscribe();

  // Pointer to parent entity
  // i.e. the entity this entity is attached to.
  public: CEntity *m_parent_entity;

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
  public: char token[STG_TOKEN_MAX]; 
  
  // Our shape and geometry
  public: double origin_x, origin_y;
  public: double size_x, size_y;

  // Our color
  public: StageColor color;

  // Descriptive name for this entity
  public: char name[256];

  // Entity mass (for collision calculations)
  protected: double mass;
  
  // stored charge (-1 for objects where this is silly) 
  protected: double volts;

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

  // print a tree of info about this entity on the fd
  public: void Print( int fd, char* prefix );
 
  // move using these velocities times the timestep
  protected: int Move( double vx, double vy, double va, double timestep );

  ///////////////////////////////////////////////////////////////////////
  // DISTRIBUTED STAGE STUFF
public: 
  stage_subscription_t subscriptions[ STG_MAX_CONNECTIONS][STG_PROPERTY_COUNT];
  void Subscribe( int con, stage_prop_id_t *props, int prop_count );
  void Unsubscribe( int con, stage_prop_id_t *props, int prop_count );
  bool IsSubscribed( stage_prop_id_t );
  
  // set the dirty flag for each property for each connection
  void SetDirty( char v);
  void SetDirty( stage_prop_id_t prop, char v );
  void SetDirty( int con, char v );
  void SetDirty( int con, stage_prop_id_t prop, char v );
  
  void DestroyConnection( int con );
  
  // the IP address of the host that manages this entity
  // replaced the hostname 'cos it's smaller and faster in comparisons
  public: struct in_addr m_hostaddr;

  // flag is true iff this entity is updated by this host
  public: bool m_local; 

public: int fig_count;
  
  // functions for drawing this entity in GUIs
#ifdef INCLUDE_RTK2
  // Initialise the rtk gui
  public: virtual int RtkStartup();

  // Finalise the rtk gui
  public: virtual void RtkShutdown();

  // Update the rtk gui
  public: virtual int RtkUpdate();

  // Default figure handle
  public: rtk_fig_t *fig, *fig_label, *fig_grid;
  
  bool grid_enable;
  double grid_major, grid_minor;

  // Default movement mask
  protected: int movemask;

  // callbacks - we ask the rtk figures call these when things happen to 'em.
  /* TODO: fix
  static void staticSetGlobalPose( void* ent, double x, double y, double th );
  static void staticSelect( void* ent );
  static void staticUnselect( void* ent );
  */
#endif

public: void *gui_data; 

};


// macro loops over the children
#define CHILDLOOP( ch )\
for( CEntity* ch = this->child_list; ch; ch = ch->next )
    

#endif




