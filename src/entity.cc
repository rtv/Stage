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
 * Desc: Base class for every entity.
 * Author: Richard Vaughan, Andrew Howard
 * Date: 7 Dec 2000
 * CVS info: $Id: entity.cc,v 1.100.2.14 2003-02-12 08:48:48 rtv Exp $
 */
#if HAVE_CONFIG_H
  #include <config.h>
#endif

#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <float.h> // for FLT_MAX
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/file.h>

#include <iostream>

//#define DEBUG
//#define VERBOSE
#undef DEBUG
#undef VERBOSE
//#define RENDER_INITIAL_BOUNDING_BOXES

#include "sio.h" // for SIOPropString()
#include "entity.hh"
#include "raytrace.hh"
#include "gui.hh"

// init the static vars shared by all entities
 // everyone shares these vars 
CMatrix* CEntity::matrix = NULL;
bool CEntity::enable_gui = true;
CEntity* CEntity::root = NULL;
double CEntity::simtime = 0.0; // start the clock at zero time
double CEntity::timestep = 0.01; // default 10ms update

///////////////////////////////////////////////////////////////////////////
// main constructor
// Requires a pointer to the parent and a pointer to the world.
CEntity::CEntity( int id, char* token, char* color, CEntity* parent )
{
  this->m_parent_entity = parent;
  
  // TODO = inherit our parent's color by default
  //if( m_parent_entity )
  //color = m_parent_entity->color;
  //else
  this->color = ::LookupColor(color); 

  this->stage_id = id;

  // init the child list data
  // do this early - ie. before calling any functions that access our children
  this->child_list = this->prev = this->next = NULL;

  // Set our default name the same
  strncpy( this->token, token, STG_TOKEN_MAX );
  strncpy( this->name, token, STG_TOKEN_MAX );
  
  // Set default pose
  this->local_px = this->local_py = this->local_pth = 0;

  // Set global velocity to stopped
  this->vx = this->vy = this->vth = 0;
  
  // unmoveably MASSIVE! by default
  this->mass = 1000.0;

  // default no-voltage.
  this->volts = -1;    

  // Set the default geometry
  this->size_x = this->size_y = 1.0;
  this->origin_x = this->origin_y = 0;

  // TODO
  m_local = true; 
 
  // zero our IO buffer headers
  memset( &buffer_data, 0, sizeof(buffer_data) );
  memset( &buffer_cfg, 0, sizeof(buffer_data) );
  memset( &buffer_cmd, 0, sizeof(buffer_data) );

  // initialize our shapes 
  this->rects = NULL;
  this->rect_count = 0;
 
  // by default, all non-root entities have a single rectangle
  if( m_parent_entity )
    {
      // rectangles - start with a single rect
      // it is automagically scaled to fit the size of the entity
      this->rects = new stage_rotrect_t[1];
      this->rects[0].x = 0.0;
      this->rects[0].y = 0.0;
      this->rects[0].a = 0.0;
      this->rects[0].w = 1.0;
      this->rects[0].h = 1.0;
      this->rect_count = 1;
    }
      
  // sets our rect boundary members
  this->DetectRectBounds();
      
  // the default entity is a colored box
  vision_return = true; 
  laser_return = LaserVisible;
  sonar_return = true;
  obstacle_return = true;
  idar_return = IDARReflect;
  puck_return = false;
  idar_return = IDARReflect;
  fiducial_return = FiducialNone; // not a recognized fiducial
  gripper_return = GripperDisabled;

  // Set the initial mapped pose to a dummy value
  this->map_px = this->map_py = this->map_pth = 0;

  m_dependent_attached = false;

  m_interval = 0.1; // update interval in seconds 
  m_last_update = -FLT_MAX; // initialized 
  
  // init the ptr to GUI-specific data
  this->gui_data = NULL;

  // clear any garbage subscription data
  for( int c=0; c<STG_MAX_CONNECTIONS; c++ )
    DestroyConnection(c);


#ifdef INCLUDE_RTK2
  // Default figures for drawing the entity.
  this->fig = NULL;
  this->fig_label = NULL;
  this->fig_grid = NULL;
  
  grid_major = 1.0;
  grid_minor = 0.2;
  grid_enable = false;

  // By default, we can both translate and rotate the entity.
  this->movemask = RTK_MOVE_TRANS | RTK_MOVE_ROT;
#endif

  // attach to my parent
  if( m_parent_entity )
    m_parent_entity->AddChild( this );
  else
    {
      // ROOT gets set up specially
      PRINT_WARN1( "ROOT ENTITY %p", this );  
      CEntity::root = this; // phear me!
      
      size_x = 10.0; // a 10m world by default
      size_y = 10.0;
      
      // the global origin is the bottom left corner of the root object
      origin_x = size_x/2.0;
      origin_y = size_y/2.0;
      
      /// default 5cm resolution passed into matrix
      double ppm = 20.0;
      PRINT_DEBUG3( "Creating a matrix [%.2fx%.2f]m at %.2f ppm",
		    size_x, size_y, ppm );
      
      assert( matrix = new CMatrix( size_x, size_y, ppm, 1) );
      
#ifdef INCLUDE_RTK2
      grid_enable = true;
#endif
    }
}


///////////////////////////////////////////////////////////////////////////
// Destructor
CEntity::~CEntity()
{
}

void CEntity::AddChild( CEntity* child )
{
  //printf( "appending entity %p to parent %p\n", child, this );
  
  STAGE_LIST_APPEND( this->child_list, child ); 
}

void CEntity::GetBoundingBox( double &xmin, double &ymin,
			      double &xmax, double &ymax )
{
  double x[4];
  double y[4];

  double dx = size_x / 2.0;
  double dy = size_y / 2.0;
  double dummy = 0.0;
  
  x[0] = origin_x + dx;
  y[0] = origin_y + dy;
  this->LocalToGlobal( x[0], y[0], dummy );

  x[1] = origin_x + dx;
  y[1] = origin_y - dy;
  this->LocalToGlobal( x[1], y[1], dummy );

  x[2] = origin_x - dx;
  y[2] = origin_y + dy;
  this->LocalToGlobal( x[2], y[2], dummy );

  x[3] = origin_x - dx;
  y[3] = origin_y - dy;
  this->LocalToGlobal( x[3], y[3], dummy );

  //double ox, oy, oth;
  //GetGlobalPose( ox, oy, oth );
  //printf( "origin: %.2f,%.2f,%.2f \n", ox, oy, oth );
  //printf( "corners: \n" );
  //for( int c=0; c<4; c++ )
  //printf( "\t%.2f,%.2f\n", x[c], y[c] );
  
  // find the smallest values and we're done
  //xmin = xmax = x[0];
  for( int c=0; c<4; c++ )
  {
    if( x[c] < xmin ) xmin = x[c];
    if( x[c] > xmax ) xmax = x[c];
  }

  //ymin = ymax = y[0];
  for( int c=0; c<4; c++ )
  {
    if( y[c] < ymin ) ymin = y[c];
    if( y[c] > ymax ) ymax = y[c];
  } 

  //printf( "before children: %.2f %.2f %.2f %.2f\n",
  //  xmin, ymin, xmax, ymax );

  CHILDLOOP( ch )
    ch->GetBoundingBox(xmin, ymin, xmax, ymax ); 

  //printf( "after children: %.2f %.2f %.2f %.2f\n",
  //  xmin, ymin, xmax, ymax );

}

// this is called very rapidly from the main loop
// it allows the entity to perform some actions between clock increments
// (such handling config requests to increase synchronous IO performance)
int CEntity::Sync()
{ 
  // default - do nothing but call the children
  CHILDLOOP( ch ) 
    if( ch->Sync() == -1 )
      return -1;

  return 0;
};


void CEntity::MapFamily()
{
  Map();
  
  CHILDLOOP( ch )
    ch->MapFamily();
}

void CEntity::UnMapFamily()
{
  UnMap();
  
  CHILDLOOP( ch )
    ch->UnMapFamily();
}


///////////////////////////////////////////////////////////////////////////
// Startup routine
// A virtual function that lets entities do some initialization after
// everything has been loaded.
int CEntity::Startup( void )
{
  PRINT_WARN( "entity starting up" );
  
  Map();
  
  int success = 0;

  CHILDLOOP( ch )
    success += ch->Startup();

  if( success < 0 )
    return -1; // fail 
  else
    return 0; // success
}


///////////////////////////////////////////////////////////////////////////
// Shutdown routine
int CEntity::Shutdown()
{
  PRINT_WARN( "entity shutting down" );

  UnMap();
  
  int success = 0;
  // recursively shutdown our children
  CHILDLOOP( ch ) 
    success += ch->Shutdown();
  
  if( success < 0 )
    return -1; // fail 
  else
    return 0; // success
}

int CEntity::Move( double vx, double vy, double va, double step )
{
  //fprintf( stderr, "%.2f %.2f %.2f   %.2f %.2f %.2f\n", 
  //   px -1, py -1, pa, odo_px, odo_py, odo_pa );
  
  // Compute movement deltas
  // This is a zero-th order approximation
  double dx = step * vx * cos(local_pth) - step * vy * sin(local_pth);
  double dy = step * vx * sin(local_pth) + step * vy * cos(local_pth);
  double da = step * va;
  
  // compute a new pose by shifting us a little from the current pose
  double qx = local_px + dx;
  double qy = local_py + dy;
  double qa = local_pth + da;

  // Check for collisions
  // and accept the new pose if ok
  //if (TestCollision(qx, qy, qa ) != NULL)
  //{
  //SetGlobalVel(0, 0, 0);
  //this->stall = true;
  // }
  //else
  {
    // set pose now takes care of marking us dirty
    SetPose(qx, qy, qa);
    //SetGlobalVel(vx, vy, va);
        
    // Compute the new odometric pose
    // we currently have PERFECT odometry. yum!
    //this->odo_px += step * vx * cos(this->odo_pa) + step * vy * sin(this->odo_pa);
    //this->odo_py += step * vx * sin(this->odo_pa) + step * vy * cos(this->odo_pa);
    //this->odo_pa += step * va;
    
    //this->stall = false;
  }
  
  return 0; // success
}
///////////////////////////////////////////////////////////////////////////
// Update the entity's representation
int CEntity::Update()
{
  // recursively update our children
  CHILDLOOP( ch ) 
    if( ch->Update() == -1 )
      return( -1 );
  
  // if we have some speed, move!
  if( vx || vy || vth )
    return Move( vx, vy, vth, CEntity::timestep );
  
  return 0;
}


///////////////////////////////////////////////////////////////////////////
// Render the entity into the world
void CEntity::Map(double px, double py, double pth)
{
  // get the pose in local coords
  this->GlobalToLocal( px, py, pth );

  // add our center of rotation offsets
  px += origin_x;
  py += origin_y;

   // convert back to global coords
  this->LocalToGlobal( px, py, pth );

  this->map_px = px;
  this->map_py = py;
  this->map_pth = pth;


  MapEx(map_px, map_py, map_pth, true);
}

void CEntity::Map( void )
{
  double x,y,a;
  GetGlobalPose( x,y,a );
  Map(x,y,a);
}

///////////////////////////////////////////////////////////////////////////
// Remove the entity from the world
void CEntity::UnMap()
{
  MapEx(this->map_px, this->map_py, this->map_pth, false);
}


///////////////////////////////////////////////////////////////////////////
// Remap ourself if we have moved
void CEntity::ReMap(double px, double py, double pth)
{
  assert( matrix );
  
  // if we haven't moved, do nothing
  if (fabs(px - this->map_px) < 1 / matrix->ppm &&
      fabs(py - this->map_py) < 1 / matrix->ppm &&
      pth == this->map_pth)
    return;
  
  // otherwise erase the old render and draw a new one
  UnMap();
  Map(px, py, pth);
}


///////////////////////////////////////////////////////////////////////////
// Primitive rendering function
void CEntity::MapEx(double px, double py, double pth, bool render)
{
    RenderRects( render );
}


///////////////////////////////////////////////////////////////////////////
// Check to see if the given pose will yield a collision with obstacles.
// Returns a pointer to the first entity we are in collision with, and stores
// the location of the hit in hitx,hity
// Returns NULL if not collisions.
// This function is useful for writing position devices.
CEntity *CEntity::TestCollision(double px, double py, double pth)
{
  // TODO - raytrace along our rectangles - more expensive, but most vehicles
  // will just be a single rect, so we're back where we started.
  
  /*  for( int r=0; r < rect_count; r++ )
    {
      double rx = rectangles[r].x;
      double ry = rectangles[r].y;
      double ra = rectangles[r].a;
      double rw = rectangles[r].w;
      double rh = rectangles[r].h;
            
      double qx = rx + origin_x * cos(rth) - this->origin_y * sin(rth);
      double qy = ry + origin_y * sin(rth) + this->origin_y * cos(rth);
      double qth = rth;
      double sx = this->size_x;
      double sy = this->size_y;
     

      CRectangleIterator rit( qx, qy, qth, sx, sy, matrix );

      CEntity* ent;
      while( (ent = rit.GetNextEntity()) )
      {
        if( ent != this && !IsDescendent(ent) && ent->obstacle_return )
          return ent;
      }
      return NULL;
    }
    case ShapeCircle:
    {
      CCircleIterator rit( px, py, sx / 2, matrix );

      CEntity* ent;
      while( (ent = rit.GetNextEntity()) )
      {
        if( ent != this && !IsDescendent(ent)  && ent->obstacle_return )
          return ent;
      }
      return NULL;
    }
    case ShapeNone:
      break;
      }*/

  return NULL;
}


///////////////////////////////////////////////////////////////////////////
// same as the above method, but stores the hit location in hitx, hity
CEntity *CEntity::TestCollision(double px, double py, double pth, 
				double &hitx, double &hity )
{
  double qx = px + this->origin_x * cos(pth) - this->origin_y * sin(pth);
  double qy = py + this->origin_y * sin(pth) + this->origin_y * cos(pth);
  double qth = pth;
  double sx = this->size_x;
  double sy = this->size_y;


  /*  switch( this->shape ) 
  {
    case ShapeRect:
    {
      CRectangleIterator rit( qx, qy, qth, sx, sy, matrix );

      CEntity* ent;
      while( (ent = rit.GetNextEntity()) )
      {
        if( ent != this && ent->obstacle_return )
        {
          rit.GetPos( hitx, hity );
          return ent;
        }
      }
      return NULL;
    }
    case ShapeCircle:
    {
      CCircleIterator rit( px, py, sx / 2, matrix );

      CEntity* ent;
      while( (ent = rit.GetNextEntity()) )
      {
        if( ent != this && ent->obstacle_return )
        {
          rit.GetPos( hitx, hity );
          return ent;
        }
      }
      return NULL;
    }
  case ShapeNone: // handle the null case
      break;
  }
  */

  return NULL;

}


///////////////////////////////////////////////////////////////////////////
// Convert local to global coords
void CEntity::LocalToGlobal(double &px, double &py, double &pth)
{
  // Get the pose of our origin wrt global cs
  double ox, oy, oth;
  GetGlobalPose(ox, oy, oth);

  // Compute pose based on the parent's pose
  double sx = ox + px * cos(oth) - py * sin(oth);
  double sy = oy + px * sin(oth) + py * cos(oth);
  double sth = oth + pth;

  px = sx;
  py = sy;
  pth = sth;
}


///////////////////////////////////////////////////////////////////////////
// Convert global to local coords
//
void CEntity::GlobalToLocal(double &px, double &py, double &pth)
{
  // Get the pose of our origin wrt global cs
  double ox, oy, oth;
  GetGlobalPose(ox, oy, oth);

  // Compute pose based on the parent's pose
  double sx =  (px - ox) * cos(oth) + (py - oy) * sin(oth);
  double sy = -(px - ox) * sin(oth) + (py - oy) * cos(oth);
  double sth = pth - oth;

  px = sx;
  py = sy;
  pth = sth;
}


///////////////////////////////////////////////////////////////////////////
// Set the entitys pose in the parent cs
void CEntity::SetPose(double px, double py, double pth)
{
  // if the new position is different, call SetProperty to make the change.
  // the -1 indicates that this change is dirty on all connections
  
  // only change the pose if these are different to the current pose
  if( this->local_px != px || this->local_py != py || this->local_pth != pth )
    {
      stage_pose_t pose;
      pose.x = px;
      pose.y = py;
      pose.a = pth;
      
      SetProperty( -1, STG_PROP_ENTITY_POSE, (char*)&pose, sizeof(pose) ); 
    }
}


///////////////////////////////////////////////////////////////////////////
// Get the entitys pose in the parent cs
void CEntity::GetPose(double &px, double &py, double &pth)
{
  px = this->local_px;
  py = this->local_py;
  pth = this->local_pth;
}


///////////////////////////////////////////////////////////////////////////
// Set the entitys pose in the global cs
void CEntity::SetGlobalPose(double px, double py, double pth)
{
  // Get the pose of our parent in the global cs
  double ox = 0;
  double oy = 0;
  double oth = 0;
  
  if (m_parent_entity) m_parent_entity->GetGlobalPose(ox, oy, oth);
  
  // Compute our pose in the local cs
  double new_x  =  (px - ox) * cos(oth) + (py - oy) * sin(oth);
  double new_y  = -(px - ox) * sin(oth) + (py - oy) * cos(oth);
  double new_th = pth - oth;
  
  SetPose( new_x, new_y, new_th );
}


///////////////////////////////////////////////////////////////////////////
// Get the entitys pose in the global cs
void CEntity::GetGlobalPose(double &px, double &py, double &pth)
{
  // Get the pose of our parent in the global cs
  double ox = 0;
  double oy = 0;
  double oth = 0;
  if (m_parent_entity)
    m_parent_entity->GetGlobalPose(ox, oy, oth);
    
  // Compute our pose in the global cs
  px = ox + this->local_px * cos(oth) - this->local_py * sin(oth);
  py = oy + this->local_px * sin(oth) + this->local_py * cos(oth);
  pth = oth + this->local_pth;
}


////////////////////////////////////////////////////////////////////////////
// Set the entitys velocity in the global cs
void CEntity::SetGlobalVel(double vx, double vy, double vth)
{
  this->vx = vx;
  this->vy = vy;
  this->vth = vth;
}


////////////////////////////////////////////////////////////////////////////
// Get the entitys velocity in the global cs
void CEntity::GetGlobalVel(double &vx, double &vy, double &vth)
{
  vx = this->vx;
  vy = this->vy;
  vth = this->vth;
}


////////////////////////////////////////////////////////////////////////////
// See if the given entity is one of our descendents
bool CEntity::IsDescendent(CEntity *entity)
{
  while (entity)
  {
    entity = entity->m_parent_entity;
    if (entity == this)
      return true;
  }
  return false;
}

void CEntity::SetDirty( int con, char v )
{
  for( int i=0; i< STG_PROPERTY_COUNT; i++ )
    SetDirty( con, (stage_prop_id_t)i, v );
}

void CEntity::SetDirty( stage_prop_id_t prop, char v )
{
  for( int i=0; i< STG_MAX_CONNECTIONS; i++ )
    SetDirty( i, prop, v );
};

void CEntity::SetDirty( int con, stage_prop_id_t prop, char v )
{
  subscriptions[con][prop].dirty = v;
};

// make EVERYTHING dirty
void CEntity::SetDirty( char v )
{
  for( int i=0; i< STG_MAX_CONNECTIONS; i++ )
    SetDirty( i, v ); //  set all props on this channel to dirty = v
};

///////////////////////////////////////////////////////////////////////////
// change the parent 
void CEntity::SetParent(CEntity* new_parent) 
{ 
  m_parent_entity = new_parent; 
};


// sets our local rectangle bounds members to the extreme values
// of the rectangle array members
void CEntity::DetectRectBounds(void)
{
  // find the bounds
  rects_max_x = rects_max_y = 0.0;
  
  int r;
  for( r=0; r<this->rect_count; r++ )
    {
      if( (rects[r].x+rects[r].w)  > rects_max_x ) 
	rects_max_x = (rects[r].x+rects[r].w);
      
      if( (rects[r].y+rects[r].h)  > rects_max_y ) 
	rects_max_y = (rects[r].y+rects[r].h);
    }
}


void CEntity::Subscribe( int con, stage_prop_id_t* props, int prop_count )  
{
  
  for( int p=0; p<prop_count; p++ )
    {
      stage_prop_id_t prop_code = props[p];
      
      if( prop_code == -1 )
	PRINT_WARN( "subscribe to all properties not implemented" );
      else
	{
	  PRINT_WARN3( "subscribing to ent %d property %s on connection %d",
		       stage_id, SIOPropString(prop_code), con );
	  // register the subscription on this channel, for this property
	  subscriptions[con][prop_code].subscribed = 1;
	  subscriptions[con][prop_code].dirty = 1;
	}
    }
}


void CEntity::Unsubscribe( int con,  stage_prop_id_t* props, int prop_count )  
{
  
  for( int p=0; p<prop_count; p++ )
    {
      stage_prop_id_t prop_code = props[p];
      
      if( prop_code == -1 )
	PRINT_WARN( "unsubscribe to all properties not implemented" );
      else
	{
	  PRINT_WARN2( "unsubscribing from property %s on connection %d",
		       SIOPropString(prop_code), con );
	  // register the subscription on this channel, for this property
	  subscriptions[con][prop_code].subscribed = 0;
	  subscriptions[con][prop_code].dirty = 0;
	}
    }
}

// clear the subscription data for this channel on me and my children
void CEntity::DestroyConnection( int con )
{
  memset( subscriptions[con], 0, 
	  STG_PROPERTY_COUNT*sizeof(stage_subscription_t) );

  CHILDLOOP(ch)
    ch->DestroyConnection( con );
}


int CEntity::SetProperty( int con, stage_prop_id_t property, 
			  char* value, size_t len )
{
  PRINT_DEBUG3( "setting prop %s (%d bytes) for ent %d",
		SIOPropString(property), (int)len, stage_id );

  assert( value );
  //assert( len > 0 );
  assert( len < (size_t)STG_PROPERTY_DATA_MAX );
  
  switch( property )
    {
    case STG_PROP_ENTITY_PPM:
      PRINT_WARN( "setting PPM" );
      assert(len == sizeof(double) );
      
      if( CEntity::matrix )
	{
	  double ppm = *((double*)value);
	  CEntity::matrix->Resize( size_x, size_y, ppm );      
	  this->MapFamily();
	}
      else
	PRINT_WARN( "trying to set ppm for non-existent matrix" );
      break;

    case STG_PROP_ENTITY_VOLTAGE:
      PRINT_WARN( "setting voltage" );
      assert(len == sizeof(double) );
      this->volts = *((double*)value);
      break;
      
    case STG_PROP_ENTITY_PARENT:
      // TODO - fix this
      // get a pointer to the (*value)'th entity - that's our new parent
      //this->m_parent_entity = m_world->GetEntity( *(int*)value );     
      PRINT_WARN( "STG_PROP_ENTITY_PARENT not implemented" );
      break;

    case STG_PROP_ENTITY_POSE:
      {
	assert( len == sizeof(stage_pose_t) );

	UnMap();

	stage_pose_t* pose = (stage_pose_t*)value;      
	local_px = pose->x;
	local_py = pose->y;
	local_pth = pose->a;

	Map();

      }
      break;

    case STG_PROP_ENTITY_VELOCITY:
      {
	assert( len == sizeof(stage_velocity_t) );
	stage_velocity_t* vel = (stage_velocity_t*)value;      
	vx = vel->x;
	vy = vel->y;
	vth = vel->a;
      }
      break;
      
    case STG_PROP_ENTITY_SIZE:
      {
	assert( len == sizeof(stage_size_t) );

	UnMap();

	stage_size_t* sz = (stage_size_t*)value;
	size_x = sz->x;
	size_y = sz->y;

	if( this == CEntity::root )
	  {
	    PRINT_WARN( "setting size of ROOT" );
	    
	    // shift root so the origin is in the bottom left corner
	    origin_x = size_x/2.0;
	    origin_y = size_y/2.0;
	    
	    // resize the matrix
	    CEntity::matrix->Resize( size_x, size_y, CEntity::matrix->ppm );      
	    MapFamily(); // matrix re-render everything
	  }
	else
	  Map();
      }
      break;
      
    case STG_PROP_ENTITY_ORIGIN:
      {
	assert( len == sizeof(stage_size_t) );
	
	UnMapFamily();
	
	stage_size_t* og = (stage_size_t*)value;
	origin_x = og->x;
	origin_y = og->y; 
	
	MapFamily();
      }
      break;
    
    case STG_PROP_ENTITY_RECTS:
      {
	// delete any previous rectangles we might have
	if( this->rects )
	  {
	    RenderRects( false ); // remove them from the matrix
	    delete[] rects;
	  }
	
	// we just got this many rectangles
	this->rect_count = len / sizeof(stage_rotrect_t);
	
	// make space for the new rects
	this->rects = new stage_rotrect_t[ this->rect_count ];
	
	// copy the rects into our local storage
	memcpy( this->rects, value, len );
	
	DetectRectBounds();

	//PRINT_WARN2( "bounds %.2f %.2f", rects_max_x, rects_max_y );

	RenderRects( true );
      }
      break;

    case STG_PROP_ENTITY_SUBSCRIBE:
      // *value is an array of integer property codes that request
      // subscriptions on this channel
      PRINT_DEBUG2( "received SUBSCRIBE for %d properties on %d",
		    (int)(len/sizeof(int)), con );
      this->Subscribe( con, (stage_prop_id_t*)value, len/sizeof(int) );
      break;
      
    case STG_PROP_ENTITY_UNSUBSCRIBE:
      // *value is an array of integer property codes that request
      // subscriptions on this channel
      PRINT_DEBUG2( "received UNSUBSCRIBE for %d properties on %d",
		    (int)(len/sizeof(int)), con );
      
      this->Unsubscribe( con, (stage_prop_id_t*)value, len/sizeof(int) );
      break;

    case STG_PROP_ENTITY_NAME:
      assert( len <= STG_TOKEN_MAX );
      strncpy( name, (char*)value, STG_TOKEN_MAX );
      break;
      
    case STG_PROP_ENTITY_COLOR:
      assert( len == sizeof(StageColor) );
      memcpy( &color, (StageColor*)value, sizeof(color) );
      break;
            
    case STG_PROP_ENTITY_LASERRETURN:
      assert( len == sizeof(LaserReturn) );
      memcpy( &laser_return, (LaserReturn*)value, sizeof(laser_return) );
      break;

    case STG_PROP_ENTITY_IDARRETURN:
      assert( len == sizeof(IDARReturn) );
      memcpy( &idar_return, (IDARReturn*)value, sizeof(idar_return) );
      break;

    case STG_PROP_ENTITY_SONARRETURN:
      assert( len == sizeof(bool) );
      memcpy( &sonar_return, (bool*)value, sizeof(sonar_return) );
      break;

    case STG_PROP_ENTITY_OBSTACLERETURN:
      assert( len == sizeof(bool) );
      memcpy( &obstacle_return, (bool*)value, sizeof(obstacle_return) );
      break;

    case STG_PROP_ENTITY_VISIONRETURN:
      assert( len == sizeof(bool) );
      memcpy( &vision_return, (bool*)value, sizeof(vision_return));
      break;

    case STG_PROP_ENTITY_PUCKRETURN:
      assert( len == sizeof(bool) );
      memcpy( &puck_return, (bool*)value, sizeof(puck_return) );
      break;


      // DISPATCH COMMANDS TO DEVICES
    case STG_PROP_ENTITY_COMMAND:
      this->BufferPacket( &buffer_cmd, value, len );
      this->SetCommand( value, len ); // virtual function is overridden in subclasses
      break;
      
    case STG_PROP_ENTITY_CONFIG:
      this->BufferPacket( &buffer_cfg, value, len );
      this->SetConfig( value, len ); // virtual function is overriden in subclasses
      break;
      
    case STG_PROP_ENTITY_DATA:
      this->BufferPacket( &buffer_data, value, len );
      this->SetData( value, len ); // virtual function is overriden in subclasses
      break;
      
    default:
      //printf( "Stage Warning: attempting to set unknown property %d\n", 
      //      property );
      break;
  }
  
  // indicate that the property is dirty on all _but_ the connection
  // it came from - that way it gets propogated onto to other clients
  // and everyone stays in sync. (assuming no recursive connections...)
  
  this->SetDirty( property, 1 ); // dirty on all cons
  
  if( con != -1 ) // unless this was a local change 
    this->SetDirty( con, property, 0 ); // clean on this con

  
  // update the GUI with the new property
  //if( enable_gui )
  GuiEntityPropertyChange( this, property );
  
  return 0;
}


int CEntity::GetProperty( stage_prop_id_t property, void* value  )
{
  //PRINT_DEBUG1( "finding property %d", property );
  //printf( "finding property %d", property );

  assert( value );

  // indicate no data - this should be overridden below
  int retval = 0;
  
  switch( property )
    {
    case STG_PROP_ENTITY_PARENT:
      // TODO - fix
      // find the parent's position in the world's entity array
      // if parent pointer is null or otherwise invalid, index is -1 
      //{ int parent_index = m_world->GetEntityIndex( m_parent_entity );
      
      { 
	int parent_index = -1;
	
	if( m_parent_entity )
	  parent_index = m_parent_entity->stage_id ;
	
	memcpy( value, &parent_index, sizeof(parent_index) );
	retval = sizeof(parent_index); 
      }
      break;
      
    case STG_PROP_ENTITY_SIZE:
      {
	stage_size_t sz;
	sz.x = size_x;
	sz.y = size_y;
	
	memcpy( value, &sz, sizeof(sz) );
	retval = sizeof(sz);
      }
      break;
      
    case STG_PROP_ENTITY_POSE:
      {
	stage_pose_t pose;
	pose.x = local_px;
	pose.y = local_py;
	pose.a = local_pth;
	
	memcpy( value, &pose, sizeof(pose) );
	retval = sizeof(pose);
      }

    case STG_PROP_ENTITY_ORIGIN:
      memcpy( value, &origin_x, sizeof(origin_x) );
      retval = sizeof(origin_x);
      break;

    case STG_PROP_ENTITY_NAME:
      strcpy( (char*)value, name );
      retval = strlen(name);
      break;

    case STG_PROP_ENTITY_COLOR:
      memcpy( value, &color, sizeof(color) );
      retval = sizeof(color);
      break;

    case STG_PROP_ENTITY_LASERRETURN:
      memcpy( value, &laser_return, sizeof(laser_return) );
      retval = sizeof(laser_return);
      break;

    case STG_PROP_ENTITY_SONARRETURN:
      memcpy( value, &sonar_return, sizeof(sonar_return) );
      retval = sizeof(sonar_return);
      break;

    case STG_PROP_ENTITY_IDARRETURN:
      memcpy( value, &idar_return, sizeof(idar_return) );
      retval = sizeof(idar_return);
      break;

    case STG_PROP_ENTITY_OBSTACLERETURN:
      memcpy( value, &obstacle_return, sizeof(obstacle_return) );
      retval = sizeof(obstacle_return);
      break;

    case STG_PROP_ENTITY_VISIONRETURN:
      memcpy( value, &vision_return, sizeof(vision_return) );
      retval = sizeof(vision_return);
      break;

    case STG_PROP_ENTITY_PUCKRETURN:
      memcpy( value, &puck_return, sizeof(puck_return) );
      retval = sizeof(puck_return);
      break;

    case STG_PROP_ENTITY_DATA:
      memcpy( value, buffer_data.data, buffer_data.len );
      retval = buffer_data.len;
      break;

    default:
      PRINT_WARN2( "attempting to get unknown property %p from model %d\n", 
		   SIOPropString, this->stage_id );
      break;
  }

  return retval;
}

// write the entity tree onto the console
void CEntity::Print( int fd, char* prefix )
{
  // TODO - write onto the fd, rather than stdout
  double ox, oy, oth;
  this->GetGlobalPose( ox, oy, oth );

  printf( "%s id: %d name: %s type: %s global: [%.2f,%.2f,%.2f]"
	  " local: [%.2f,%.2f,%.2f] vision_return %d )", 
	  prefix,
	  this->stage_id,
	  this->name,
	  this->token,
	  ox, oy, oth,
	  local_px, local_py, local_pth,
	  this->vision_return );
	  
  if( this->m_parent_entity == NULL )
    puts( " - ROOT" );
  else
    puts( "" );

  // add an indent to the prefix
  
  char* buf = new char[ strlen(prefix) + 1 ];
  sprintf( buf, "\t%s", prefix );

  CHILDLOOP( ch )
    ch->Print( fd, buf );
}

// return true if this property is subscribed on any connection
bool CEntity::IsSubscribed( stage_prop_id_t prop )
{
  for( int con=0; con < STG_MAX_CONNECTIONS; con++ )
    if( subscriptions[con][prop].subscribed )
      return true;
  
  return false;
}

/*
// these versions sub/unsub to this device and all its decendants
void CEntity::FamilySubscribe()
{ 
  CHILDLOOP( ch ) 
    ch->FamilySubscribe(); 

  sub_count++;
};

void CEntity::FamilyUnsubscribe()
{ 
  CHILDLOOP( ch ) 
    ch->FamilyUnsubscribe(); 
  
  sub_count--;
};
*/


void CEntity::GetStatusString( char* buf, int buflen )
{
  double x, y, th;
  this->GetGlobalPose( x, y, th );
  
  // check for overflow
  assert( -1 != 
	  snprintf( buf, buflen, 
		    "Pose(%.2f,%.2f,%.2f) Stage(%d:%s)",
		    x, y, th, 
		    this->stage_id,
		    this->token ) );
}  


void CEntity::RenderRects( bool render )
{
  // TODO: find the scaling
  double scalex = size_x / rects_max_x;
  double scaley = size_y / rects_max_y;
  
  double x,y,a,w,h;
  
  int r;
  for( r=0; r<this->rect_count; r++ )
    {
      x = ((this->rects[r].x + this->rects[r].w/2.0) * scalex) - size_x/2.0;
      y = ((this->rects[r].y + this->rects[r].h/2.0)* scaley) - size_y/2.0;
      a = this->rects[r].a;
      w = this->rects[r].w * scalex;
      h = this->rects[r].h * scaley;
      
      LocalToGlobal( x,y,a );    

      CEntity::matrix->SetRectangle( x, y, a, w, h, this, render );
    }
  
  // draw a boundary rectangle around the root device
  if( this == CEntity::root )
    CEntity::matrix->SetRectangle( size_x/2.0, size_y/2.0, 0.0, 
				   size_x, size_y, this, render );
  
}


int CEntity::BufferPacket( stage_buffer_t* buf, char* data, size_t len )
{
  PRINT_DEBUG2( "pushing %d bytes into stage_buffer_t at %p",
		(int)len, buf );

  // if the buffer is too small
  if( len > buf->len )
    //if( len != buf->len ) // alternative: if buf is not exactly the right size
    {
      // make it the right size
      buf->data = (char*)realloc( buf->data, len );
      buf->len = len;
    }

  // copy the data into our buffer
  memcpy( buf->data, data, len );
  return 0;
}


int CEntity::SetCommand( char* data, size_t len )
{
  PRINT_WARN2( "ent %d received %d bytes of command, but does nothing", 
	       this->stage_id, (int)len );   
  return 0;
}

int CEntity::SetConfig( char* data, size_t len )
{
  PRINT_WARN2( "ent %d received %d bytes of config, but does nothing with it", 
	       this->stage_id, (int)len );   
  return 0;
}

int CEntity::SetData( char* data, size_t len )
{
  PRINT_WARN2( "ent %d received %d bytes of data", 
	       this->stage_id, (int)len ); 
  return 0;
}

int CEntity::GetCommand( char* data, size_t* len )
{
  memcpy( data, &(buffer_cmd.data), buffer_cmd.len );
  *len = buffer_cmd.len;
  return 0;
}

int CEntity::GetConfig( char* data, size_t* len )
{
  memcpy( data, &(buffer_cfg.data), buffer_cfg.len );
  *len = buffer_cfg.len;
  return 0;
}

int CEntity::GetData( char* data, size_t* len )
{
  //PRINT_DEBUG4( "pasting ent %d's data (%d bytes) from %p into %p",
  //	this->stage_id, buffer_data.len, buffer_data.data, data ); 
  
  memcpy( data, buffer_data.data, buffer_data.len );
  *len = buffer_data.len;
  return 0;
}
