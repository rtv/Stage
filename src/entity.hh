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
 * CVS info: $Id: entity.hh,v 1.15.2.23 2003-08-09 00:58:34 rtv Exp $
 */

#ifndef _ENTITY_HH
#define _ENTITY_HH


#if HAVE_CONFIG_H
// for conditional GUI building
// it'd be good to not need this here as it causes big rebuilds... rtv. 
# include <config.h>
#endif

#include "stage.h"
#include "world.hh"
#include "rtkgui.hh"
#include "colors.h"


// plain functions for tree manipulation
CEntity* stg_ent_next_sibling( CEntity* ent );
CEntity* stg_ent_first_child( CEntity* ent );
CEntity* stg_ent_parent( CEntity* ent );
CWorld* stg_world( CEntity* ent );
CEntity* stg_world_first_child( CWorld* world );

// Forward declare
class CMatrix;
class CWorld;
///////////////////////////////////////////////////////////////////////////
// The basic moveable object class
class CEntity
{

public: 
  CEntity(  stg_entity_create_t* init );

  int id;
  bool running;
  GNode* node;  
  GString *name, *token;
  
  // infrastructure stuff
private:
  guint read_tag, update_tag, idle_tag;

protected:

  //static int next_available_id;
  stg_entity_create_t last_child_created;

public:
  // static stuff shared by all instances 
  static gboolean stg_update_signal( gpointer ptr );
    
  gboolean ChildCreate( stg_entity_create_t* childdata );
 
  // all the data required to render this object sensibly
  stg_gui_model_t* guimod;
  
public:
  // Destructor
  public: virtual ~CEntity();

public: 
  // Initialise entity  
  virtual int Startup(); 
  // Finalize entity
  virtual int Shutdown();  

  // Update the entity's device-specific representation
  // this is called by a callback installed in CEntity::Startup()
  virtual gboolean Update();
  
  virtual int SetProperty( stg_prop_id_t ptype, void* data, size_t len );
  virtual stg_property_t* GetProperty( stg_prop_id_t ptype );

protected:

  // type-specific initialization
  void InitWall( void );
  void InitLaser( void );
  void InitPosition( void );

  stg_rotrect_array_t* rect_array;

  // copies the array of rects into this entity, allocating storage
  // and setting the rects_max members correctly
  void SetRects( stg_rotrect_t* rects, int num_rects );
  
  // scale an array of rects into a unit square
  void NormalizeRects( stg_rotrect_t* rects, int num_rects );
  
  // convert the rotated rectangle into global coords, taking into account
  // the entities pose and the rectangle scaling
  void GetGlobalRect( stg_rotrect_t* dest, stg_rotrect_t* src );
  
  // bool controls whether rects are added to or removed from the matrix
  void RenderRects(  bool render );

public:
  
  int GetNumRects( void )
  { return( this->rect_array == NULL ?  0 : this->rect_array->rect_count );};
  
  stg_rotrect_t* GetRects( void )
  { return( this->rect_array == NULL ? NULL : this->rect_array->rects ); };
  
  stg_rotrect_t* GetRect( int i )
  { return( this->rect_array == NULL? NULL : &this->rect_array->rects[i] ); };
  
  // Render the entity into the world
  //  protected: void Map(double px, double py, double pth);
protected: void Map( stg_pose_t* pose );
  
  // calls Map(double,double,double) with the current global pose
  protected: void Map( void );

  // Remove the entity from the world
  protected: void UnMap();

  // Remove the entity at its current pose and remap it at a new pose.
  // protected: void ReMap(double px, double py, double pth);
  protected: void ReMap( stg_pose_t* pose);

  // maps myself and my children, recursively
  protected: void MapFamily(void);
  // unmaps myself and my children, recursively
  protected: void UnMapFamily(void);
 
  // Primitive rendering function using internally
  private: void MapEx( stg_pose_t* pose, bool render);

  // Check to see if the given pose will yield a collision with obstacles.
  // Returns a pointer to the first entity we are in collision with.
  // Returns NULL if no collisions. records the position of the hit in
  // hitx and hity, if non-null
protected: CEntity *TestCollision( double *hity = NULL, double* hity = NULL);  
  
  // writes a description of this device into the buffer
public: void GetStatusString( char* buf, int buflen );
  
  // Convert local to global coords
public: void LocalToGlobal( double &x, double &y, double &a );
public: void LocalToGlobal( stg_pose_t* pose );
  
  // Convert global to local coords
public: void GlobalToLocal( stg_pose_t* pose );
  
  // set the entity's dimensions
public: void SetSize( stg_size_t* sz );

  virtual void GetSize( stg_size_t* sz )
  { memcpy( sz, &this->size, sizeof(stg_size_t) ); }


  // Set the entity's pose in the parent cs
  //public: void SetPose(double px, double py, double pth);
public: void SetPose( stg_pose_t* pose );
  
  // Get the entity's pose in the parent cs
public: void GetPose( stg_pose_t* pose );

public: void GetOrigin( stg_pose_t* pose );
public: void SetOrigin( stg_pose_t* pose );

  // Get the entity's pose in the global cs
public: void SetGlobalPose( stg_pose_t* pose );
  
  // Get the entity's pose in the global cs
public: void GetGlobalPose( stg_pose_t* pose );
  //public: void GetGlobalPose( double &x, double &y, double &a );
  
  // Set the entity's velocity in the local cs
public: void SetVelocity(stg_velocity_t* vel);
  
  // Get the entity's velocity in the localcs
public: void GetVelocity(stg_velocity_t* vel);
  
  // Get the entity mass
  public: double GetMass() { return (this->mass); }

public: void GetBoundingBox( double &xmin, double &ymin,
			     double &xmax, double &ymax );
  
  // See if the given entity is one of our descendents
  public: bool IsDescendent(CEntity *entity);
  
public: 
  
  CWorld* GetWorld();
  CMatrix* GetMatrix();

  // The section in the world file that describes this entity
  public: int worldfile_section;

  // return a pointer to this or a child if it matches the worldfile section
  CEntity* FindSectionEntity( int section );
      
  // Our shape and geometry origin specifies my body's offset from my
  // location (e.g for moving the center of rotation in pioneers
public: stg_pose_t pose_origin;
public: stg_size_t size;
  
  // Our color
  public: stg_color_t color;

  // Entity mass (for collision calculations)
  protected: double mass;
  
  // stored charge (-1 for objects where this is silly) 
  protected: double volts;

  // set if object collided and had its velocity zeroed
public: bool stall;

  // Sensor return values
  // Set these appropriately to have this entity 'seen' by
  // the relevant sensor.
public: int obstacle_return;
public: int puck_return;
public: int sonar_return;  
public: int vision_return;
public: stg_laser_return_t laser_return;
public: stg_idar_return_t idar_return;
public: stg_gripper_return_t gripper_return;
public: int fiducial_return; 
public: int neighbor_return;
  
  // flag is set when a dependent device is  attached to this device
  //public: bool m_dependent_attached;

  // Initial pose in local cs (ie relative to parent)
protected: stg_pose_t pose_init;
  
  // Pose in local cs (ie relative to parent)
protected: stg_pose_t pose_local;
  
  // The last mapped pose in global cs
protected: stg_pose_t pose_map;
  
  // Velocity in local cs 
  protected: stg_velocity_t velocity;

  
  // how often to update this device, in seconds
  protected: double interval; 

  // print a tree of info about this entity on the fd
  public: void Print( int fd, char* prefix );
  
  // move using these velocities times the timestep
protected: 
  //virtual int Move( double vx, double vy, double va, double timestep );
  virtual int Move( stg_velocity_t* vel, double timestep );

public:  bool OcclusionTest(CEntity* ent );

  public: 
  GArray* transducers;
  void UpdateTransducers( void );

  // this structure specifies our laser scan
  stg_laser_data_t laser_data;
  // performs a scan, filling in the structure's sample values
  void UpdateLaserData( stg_laser_data_t* laser );
  
  stg_bounds_t bounds_neighbor;
  void GetNeighbors( GArray** neighbor_array );
  
  // STG_PROP_ENTITY_POWER - non-zero:on, zero:off
  int power_on;
  
  // STG_PROP_ENTITY_RANGEBOUNDS - limits of sensor range in meters 
  double min_range, max_range;
  
  // user-configurable GUI settings
  bool mouseable;
  bool draw_nose;
};

#ifdef DEBUG
#define BASE_DEBUG(S) printf("[%d:%s (%s) %p] "S" (%s %s)\n",this->id,this->name->str,this->token->str,this,__FILE__,__FUNCTION__);
#define BASE_DEBUG1(S,A) printf("[%d:%s (%s) %p] "S" (%s %s)\n",this->id,this->name->str,this->token->str,this,A,__FILE__,__FUNCTION__);
#define BASE_DEBUG2(S,A,B) printf("[%d:%s (%s) %p] "S" (%s %s)\n",this->id,this->name->str,this->token->str,this,A,B,__FILE__,__FUNCTION__);
#define BASE_DEBUG3(S,A,B,C) printf("[%d:%s (%s) %p] "S" (%s %s)\n",this->id,this->name->str,this->token->str,this,A,B,C,__FILE__,__FUNCTION__);
#else
#define BASE_DEBUG(S)
#define BASE_DEBUG1(S,A)
#define BASE_DEBUG2(S,A,B)
#define BASE_DEBUG3(S,A,B,C)
#endif



#endif




