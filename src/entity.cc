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
 * CVS info: $Id: entity.cc,v 1.100.2.8 2003-02-07 05:30:34 rtv Exp $
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
#define VERBOSE
//#undef DEBUG
//#undef VERBOSE
//#define RENDER_INITIAL_BOUNDING_BOXES

#include "entity.hh"
#include "raytrace.hh"
#include "gui.hh"
#include "library.hh"

extern rtk_canvas_t* canvas;
extern rtk_app_t* app;

// init the static vars shared by all entities
 // everyone shares these vars 
CMatrix* CEntity::matrix = NULL;
bool CEntity::enable_gui = true;
CEntity* CEntity::root = NULL;
double CEntity::simtime = 0.0; // start the clock at zero time

///////////////////////////////////////////////////////////////////////////
// main constructor
// Requires a pointer to the parent and a pointer to the world.
CEntity::CEntity( LibraryItem* libit, int id, CEntity* parent )
{
  this->m_parent_entity = parent;
  
  // TODO = inherit our parent's color by default
  //if( m_parent_entity )
  //color = m_parent_entity->color;
  //else
  color = libit->color; //look in library for our color

  this->stage_id = id;
  this->lib_entry = libit; // from here we can find our file token and type number

  // init the child list data
  this->child_list = this->prev = this->next = NULL;

  // Set our default name
  this->name[0] = 0;

  // Set default pose
  this->local_px = this->local_py = this->local_pth = 0;

  // Set global velocity to stopped
  this->vx = this->vy = this->vth = 0;

  // unmoveably MASSIVE! by default
  this->mass = 1000.0;
    
  // Set the default geometry
  this->size_x = this->size_y = 1.0;
  this->origin_x = this->origin_y = 0;

  // TODO
  m_local = true; 
  
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

  // we start out NOT dirty - no one gets deltas unless they ask for 'em.
  SetDirty( 0 );
  
  m_last_pixel_x = m_last_pixel_y = m_last_degree = 0;

  m_interval = 0.1; // update interval in seconds 
  m_last_update = -FLT_MAX; // initialized 

  // init the ptr to GUI-specific data
  this->gui_data = NULL;

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
void CEntity::Sync()
{ 
  // default - do nothing but call the children
  CHILDLOOP( ch ) ch->Sync(); 
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
bool CEntity::Startup( void )
{
  PRINT_WARN( "entity starting up" );
  
  Map();
  
  CHILDLOOP( ch )
    ch->Startup();
  
  return true;
}


///////////////////////////////////////////////////////////////////////////
// Shutdown routine
void CEntity::Shutdown()
{
  PRINT_WARN( "entity shutting down" );
  
  // recursively shutdown our children
  CHILDLOOP( ch ) ch->Shutdown();

  UnMap();

  return;
}


///////////////////////////////////////////////////////////////////////////
// Update the entity's representation
void CEntity::Update( double sim_time )
{
  //PRINT_DEBUG( "UPDATE" );

  // recursively update our children
  CHILDLOOP( ch ) ch->Update( sim_time );    

  //GuiEntityUpdate( this );
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
  double qx = px + this->origin_x * cos(pth) - this->origin_y * sin(pth);
  double qy = py + this->origin_y * sin(pth) + this->origin_y * cos(pth);
  double qth = pth;
  double sx = this->size_x;
  double sy = this->size_y;


  // TODO - raytrace along our rectangles - more expensive, but most vehicles
  // will just be a single rect, so we're back where we started.
  /*
  switch( this->shape ) 
  {
    case ShapeRect:
    {
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
      
      SetProperty( -1, STG_PROP_ENTITY_POSE, &pose, sizeof(pose) ); 
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
  m_dirty[con][prop] = v;
};

void CEntity::SetDirty( char v )
{
  memset( m_dirty, v, 
	  sizeof(char) * STG_MAX_CONNECTIONS * STG_PROPERTY_COUNT );
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


int CEntity::SetProperty( int con, stage_prop_id_t property, 
			  void* value, size_t len )
{
  PRINT_DEBUG3( "setting prop %d (%d bytes) for ent %p",
		property, len, this );

  assert( value );
  assert( len > 0 );
  assert( (int)len < MAX_PROPERTY_DATA_LEN );
  
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


int CEntity::GetProperty( stage_prop_id_t property, void* value )
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
    default:
      // printf( "Stage Warning: attempting to get unknown property %d\n", 
      //      property );
    break;
  }

  return retval;
}

// write the entity tree onto the console
void CEntity::Print( char* prefix )
{
  double ox, oy, oth;
  this->GetGlobalPose( ox, oy, oth );

  printf( "%s type: %s global: [%.2f,%.2f,%.2f]"
	  " local: [%.2f,%.2f,%.2f] vision_return %d )", 
	  prefix,
	  this->lib_entry->token,
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
    ch->Print( buf );
}


// subscribe to / unsubscribe from the device
// these don't do anything by default, but are overridden by CPlayerEntity
void CEntity::Subscribe()
{ 
  //puts( "SUB" );
};

void CEntity::Unsubscribe()
{ 
  //puts( "UNSUB" );
};
  
int CEntity::Subscribed()
{ 
  return 0;
};


// these versions sub/unsub to this device and all its decendants
void CEntity::FamilySubscribe()
{ 
  CHILDLOOP( ch ) ch->FamilySubscribe(); 
};

void CEntity::FamilyUnsubscribe()
{ 
  CHILDLOOP( ch ) ch->FamilyUnsubscribe(); 
};


void CEntity::GetStatusString( char* buf, int buflen )
{
  double x, y, th;
  this->GetGlobalPose( x, y, th );
  
  // check for overflow
  assert( -1 != 
	  snprintf( buf, buflen, 
		    "Pose(%.2f,%.2f,%.2f) Stage(%d:%d(%s))",
		    x, y, th, 
		    this->stage_id,
		    this->lib_entry->type_num,
		    this->lib_entry->token ) );
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



#ifdef INCLUDE_RTK2

///////////////////////////////////////////////////////////////////////////
// Initialise the rtk gui
void CEntity::RtkStartup()
{
  // Create a figure representing this entity
  if( m_parent_entity == NULL )
    this->fig = rtk_fig_create( canvas, NULL, 50);
  else
    this->fig = rtk_fig_create( canvas, m_parent_entity->fig, 50);

  assert( this->fig );

  // Set the mouse handler
  this->fig->userdata = this;
  rtk_fig_add_mouse_handler(this->fig, StaticRtkOnMouse);

  // add this device to the world's device menu 
  //this->m_world->AddToDeviceMenu( this, true); 
    
  // visible by default
  rtk_fig_show( this->fig, true );

  // Set the color
  rtk_fig_color_rgb32(this->fig, this->color);

  if( m_parent_entity == NULL )
    // the root device is drawn from the corner, not the center
    rtk_fig_origin( this->fig, local_px, local_py, local_pth );
  else
    // put the figure's origin at the entity's position
    rtk_fig_origin( this->fig, local_px, local_py, local_pth );



#ifdef RENDER_INITIAL_BOUNDING_BOXES
  double xmin, ymin, xmax, ymax;
  xmin = ymin = 999999.9;
  xmax = ymax = 0.0;
  this->GetBoundingBox( xmin, ymin, xmax, ymax );
  
  rtk_fig_t* boundaries = rtk_fig_create( canvas, NULL, 99);
  double width = xmax - xmin;
  double height = ymax - ymin;
  double xcenter = xmin + width/2.0;
  double ycenter = ymin + height/2.0;

  rtk_fig_rectangle( boundaries, xcenter, ycenter, 0, width, height, 0 ); 

   
#endif
   
  // everything except the root object has a label
  //if( m_parent_entity )
  //{
    // Create the label
    // By default, the label is not shown
    this->fig_label = rtk_fig_create( canvas, this->fig, 51);
    rtk_fig_show(this->fig_label, true); // TODO - re-hide the label    
    rtk_fig_movemask(this->fig_label, 0);
      
    char label[1024];
    char tmp[1024];
      
    label[0] = 0;
    snprintf(tmp, sizeof(tmp), "%s %s", 
             this->name, this->lib_entry->token );
    strncat(label, tmp, sizeof(label));
      
    rtk_fig_color_rgb32(this->fig, this->color);
    rtk_fig_text(this->fig_label,  0.75 * size_x,  0.75 * size_y, 0, label);
      
    // attach the label to the main figure
    // rtk will draw the label when the mouse goes over the figure
    // TODO: FIX
    //this->fig->mouseover_fig = fig_label;
    
    // we can be moved only if we are on the root node
    if (m_parent_entity != CEntity::root )
      rtk_fig_movemask(this->fig, 0);
    else
      rtk_fig_movemask(this->fig, this->movemask);  

    //}

 
  if( this->grid_enable )
    {
      this->fig_grid = rtk_fig_create(canvas, this->fig, -49);
      if ( this->grid_minor > 0)
	{
	  rtk_fig_color( this->fig_grid, 0.9, 0.9, 0.9);
	  rtk_fig_grid( this->fig_grid, this->origin_x, this->origin_y, 
			this->size_x, this->size_y, grid_minor);
	}
      if ( this->grid_major > 0)
	{
	  rtk_fig_color( this->fig_grid, 0.75, 0.75, 0.75);
	  rtk_fig_grid( this->fig_grid, this->origin_x, this->origin_y, 
			this->size_x, this->size_y, grid_major);
	}
      rtk_fig_show( this->fig_grid, 1);
    }
  else
    this->fig_grid = NULL;
  

  PRINT_DEBUG1( "rendering %d rectangles", this->rect_count );
  
  double scalex = size_x / rects_max_x;
  double scaley = size_y / rects_max_y;
  
  //rtk_fig_origin( this->fig, local_px, local_py, local_pth );
  
  // now create GUI rects
  int r;
  for( r=0; r<this->rect_count; r++ )
    {
      double x, y, a, w, h;
      
      x = ((this->rects[r].x + this->rects[r].w/2.0) * scalex) - size_x/2.0;
      y = ((this->rects[r].y + this->rects[r].h/2.0)* scaley) - size_y/2.0;
      a = this->rects[r].a;
      w = this->rects[r].w * scalex;
      h = this->rects[r].h * scaley;
      
      rtk_fig_rectangle(this->fig, x, y, a, w, h, 0 ); 
    }

  // draw a boundary rectangle around the root device
  if( this == CEntity::root )
    rtk_fig_rectangle( this->fig, size_x/2.0, size_y/2.0, 0.0, 
		       size_x, size_y, 0 );
  
  // do our children after we're set
  //CHILDLOOP( child ) child->RtkStartup();
  PRINT_DEBUG( "RTK STARTUP DONE" );
}



///////////////////////////////////////////////////////////////////////////
// Finalise the rtk gui
void CEntity::RtkShutdown()
{
  PRINT_DEBUG1( "RTK SHUTDOWN %d", this->stage_id );
  
  // do our children first
  //CHILDLOOP( child ) GuiShutdown (child->RtkStartup();

  // Clean up the figure we created
  if( this->fig ) rtk_fig_destroy(this->fig);
  if( this->fig_label ) rtk_fig_destroy(this->fig_label);
  if( this->fig_grid ) rtk_fig_destroy( this->fig_grid );

  this->fig = NULL;
  this->fig_label = NULL;
  this->fig_grid = NULL;
} 


///////////////////////////////////////////////////////////////////////////
// Update the rtk gui
void CEntity::RtkUpdate()
{
  PRINT_WARN1( "RTK update for ent %d", this->stage_id );
  //CHILDLOOP( child ) child->RtkUpdate();

  // TODO this is nasty and inefficient - figure out a better way to
  // do this  

  // if we're not looking at this device, hide it 
  //if( !m_world->ShowDeviceBody( this->lib_entry->type_num ) )
  //{
  //rtk_fig_show(this->fig, false);
  //}
  //else // we need to show and update this figure
  {
    rtk_fig_show( this->fig, true );  }
}


///////////////////////////////////////////////////////////////////////////
// Process mouse events
void CEntity::RtkOnMouse(rtk_fig_t *fig, int event, int mode)
{
  double px, py, pth;

  switch (event)
  {
    case RTK_EVENT_PRESS:
    case RTK_EVENT_MOTION:
    case RTK_EVENT_RELEASE:
      rtk_fig_get_origin(fig, &px, &py, &pth);
      this->SetGlobalPose(px, py, pth);
      break;

    default:
      break;
  }

  return;
}


///////////////////////////////////////////////////////////////////////////
// Process mouse events (static callback)
void CEntity::StaticRtkOnMouse(rtk_fig_t *fig, int event, int mode)
{
  CEntity *entity;
  entity = (CEntity*) fig->userdata;
  entity->RtkOnMouse(fig, event, mode);
  return;
}


#endif
