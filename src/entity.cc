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
 * Desc: Base class for every moveable entity.
 * Author: Richard Vaughan, Andrew Howard
 * Date: 7 Dec 2000
 * CVS info: $Id: entity.cc,v 1.80 2002-08-30 18:17:28 rtv Exp $
 */

#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <values.h>  // for MAXFLOAT
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/file.h>

#include <iostream>


//#define DEBUG
//#define VERBOSE
//#undef DEBUG
//#undef VERBOSE
//#define RENDER_INITIAL_BOUNDING_BOXES

#include "entity.hh"
#include "raytrace.hh"
#include "world.hh"
#include "worldfile.hh"


#ifdef INCLUDE_RTK2
// CALLBACK FUNCTION WRAPPERS ////////////////////////////////////////////
// called by rtk to tweak Stage devices

void CEntity::staticSetGlobalPose( void* ent, double x, double y, double th )
{
  ((CEntity*)ent)->SetGlobalPose( x, y, th );
}

#endif

///////////////////////////////////////////////////////////////////////////
// main constructor
// Requires a pointer to the parent and a pointer to the world.
CEntity::CEntity(CWorld *world, CEntity *parent_entity )
{
  assert( world );
  
  m_world = world; 
  m_parent_entity = parent_entity;
  m_default_entity = this;

  // attach to my parent
  if( m_parent_entity )
    m_parent_entity->AddChild( this );
  else
    PRINT_DEBUG1( "ROOT ENTITY %p\n", this );
  
  // register with the world
  m_world->RegisterEntity( this );

  // init the child list data
  this->child_list = this->prev = this->next = NULL;

  // Set our default type (will be reset by subclass).
  this->stage_type = NullType;

  // Set the filename used for this device's mmap
  //this->device_filename[0] = 0;

  // Set our default name
  this->name[0] = 0;

  // Set default pose
  this->local_px = this->local_py = this->local_pth = 0;

  // Set global velocity to stopped
  this->vx = this->vy = this->vth = 0;

  // unmoveably MASSIVE! by default
  this->mass = 1000.0;
    
  // Set the default shape and geometry
  this->shape = ShapeNone;
  this->size_x = this->size_y = 0;
  this->origin_x = this->origin_y = 0;

  // Set the default color
  this->color = 0xFF0000;

  m_local = false; 
  
  // by default, entities don't show up in any sensors
  // these must be enabled explicitly in each subclass
  obstacle_return = false;
  sonar_return = false;
  puck_return = false;
  vision_return = false;
  laser_return = LaserTransparent;
  idar_return = IDARTransparent;

  // Set the initial mapped pose to a dummy value
  this->map_px = this->map_py = this->map_pth = 0;

  m_dependent_attached = false;

  // we start out NOT dirty - no one gets deltas unless they ask for 'em.
  SetDirty( 0 );
  
  m_last_pixel_x = m_last_pixel_y = m_last_degree = 0;

  m_interval = 0.1; // update interval in seconds 
  m_last_update = -MAXFLOAT; // initialized 

#ifdef INCLUDE_RTK2
  // Default figures for drawing the entity.
  this->fig = NULL;
  this->fig_label = NULL;

  // By default, we can both translate and rotate the entity.
  this->movemask = RTK_MOVE_TRANS | RTK_MOVE_ROT;
#endif
}


///////////////////////////////////////////////////////////////////////////
// Destructor
CEntity::~CEntity()
{
  //close( m_fd ); 
}

void CEntity::AddChild( CEntity* child )
{
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


// return a pointer to this or a child if it matches the worldfile section
CEntity* CEntity::FindSectionEntity( int section )
{
  PRINT_DEBUG2( "find section %d. my section %d\n", 
		section, this->worldfile_section );

  if( section == this->worldfile_section )
    return this;
  
  CEntity* found = NULL;

  // otherwise, recursively check our children
  CHILDLOOP( ch )
    {
      found = ch->FindSectionEntity( section ); 
      if( found ) break;
    }
  
  return found;
  
}



///////////////////////////////////////////////////////////////////////////
// Load the entity from the world file
bool CEntity::Load(CWorldFile *worldfile, int section)
{
  // Read the name
  strcpy( this->name, worldfile->ReadString(section, "name", "") );
      
  // Read the pose
  this->init_px = worldfile->ReadTupleLength(section, "pose", 0, 0);
  this->init_py = worldfile->ReadTupleLength(section, "pose", 1, 0);
  this->init_pth = worldfile->ReadTupleAngle(section, "pose", 2, 0);
  SetPose(this->init_px, this->init_py, this->init_pth);

  // Read the shape
  const char *shape_desc = worldfile->ReadString(section, "shape", NULL);
  if (shape_desc)
  {
    if (strcmp(shape_desc, "rect") == 0)
      this->shape = ShapeRect;
    else if (strcmp(shape_desc, "circle") == 0)
      this->shape = ShapeCircle;
    else
      PRINT_WARN1("invalid shape desc [%s]; using default", shape_desc);
  }

  // Read the size
  this->size_x = worldfile->ReadTupleLength(section, "size", 0, this->size_x);
  this->size_y = worldfile->ReadTupleLength(section, "size", 1, this->size_y);

  // Read the origin offsets (for moving center of rotation)
  this->origin_x = worldfile->ReadTupleLength(section, "offset", 0, this->origin_x);
  this->origin_y = worldfile->ReadTupleLength(section, "offset", 1, this->origin_y);

  // Read the entity color
  this->color = worldfile->ReadColor(section, "color", this->color);

  const char *rvalue;
  
  // Obstacle return values
  if (this->obstacle_return)
    rvalue = "visible";
  else
    rvalue = "invisible";
  rvalue = worldfile->ReadString(section, "obstacle_return", rvalue);
  if (strcmp(rvalue, "visible") == 0)
    this->obstacle_return = 1;
  else
    this->obstacle_return = 0;

  // Sonar return values
  if (this->sonar_return)
    rvalue = "visible";
  else
    rvalue = "invisible";
  rvalue = worldfile->ReadString(section, "sonar_return", rvalue);
  if (strcmp(rvalue, "visible") == 0)
    this->sonar_return = 1;
  else
    this->sonar_return = 0;

  // Vision return values
  if (this->vision_return)
    rvalue = "visible";
  else
    rvalue = "invisible";
  rvalue = worldfile->ReadString(section, "vision_return", rvalue);
  if (strcmp(rvalue, "visible") == 0)
    this->vision_return = 1;
  else
    this->vision_return = 0;
  
  // Laser return values
  if (this->laser_return == LaserBright)
    rvalue = "bright";
  else if (this->laser_return == LaserVisible)
    rvalue = "visible";
  else
    rvalue = "invisible";
  rvalue = worldfile->ReadString(section, "laser_return", rvalue);
  if (strcmp(rvalue, "bright") == 0)
    this->laser_return = LaserBright;
  else if (strcmp(rvalue, "visible") == 0)
    this->laser_return = LaserVisible;
  else
    this->laser_return = LaserTransparent;
    
  return true;
}


///////////////////////////////////////////////////////////////////////////
// Save the entity to the world file
bool CEntity::Save(CWorldFile *worldfile, int section)
{
  // Write the pose (but only if it has changed)
  double px, py, pth;
  GetPose(px, py, pth);

  //printf( "pose: %.2f %.2f %.2f   init:  %.2f %.2f %.2f\n",
  //  px, py, pth, init_px, init_py, init_pth );

  if (px != this->init_px || py != this->init_py || pth != this->init_pth)
  {
    worldfile->WriteTupleLength(section, "pose", 0, px);
    worldfile->WriteTupleLength(section, "pose", 1, py);
    worldfile->WriteTupleAngle(section, "pose", 2, pth);
  }

  CHILDLOOP( ch ) ch->Save( worldfile, ch->worldfile_section );
  
  return true;
}


///////////////////////////////////////////////////////////////////////////
// Startup routine
// A virtual function that lets entities do some initialization after
// everything has been loaded.
bool CEntity::Startup( void )
{
  PRINT_DEBUG2("STARTUP %s %s", 
	       this->m_world->lib->StringFromType( this->stage_type ),
	       m_parent_entity ? "" : "- ROOT" );
  
 CHILDLOOP( ch )
    ch->Startup();

#ifdef INCLUDE_RTK2
  // Initialise the rtk gui
 //RtkStartup();
#endif

  PRINT_DEBUG( "STARTUP DONE" );

  return true;
}


///////////////////////////////////////////////////////////////////////////
// Shutdown routine
void CEntity::Shutdown()
{
  PRINT_DEBUG( "entity shutting down" );
  
  // recursively shutdown our children
  CHILDLOOP( ch ) ch->Shutdown();


#ifdef INCLUDE_RTK2
  // Clean up the figure we created
  rtk_fig_destroy(this->fig);
#endif

  return;
}


///////////////////////////////////////////////////////////////////////////
// Update the entity's representation
void CEntity::Update( double sim_time )
{
  //PRINT_DEBUG( "UPDATE" );

  // recursively update our children
  CHILDLOOP( ch ) ch->Update( sim_time );  
}


///////////////////////////////////////////////////////////////////////////
// Render the entity into the world
void CEntity::Map(double px, double py, double pth)
{
  //printf( "\nMAP\t%.2f %.2f %.2f\n",
  //  px, py, pth );

  // shift the center of our image by the offset
 
  //printf( "map global %.2f %.2f %.2f\n",
  //  px, py, pth );

  // get the pose in local coords
  this->GlobalToLocal( px, py, pth );

  //printf( "map local %.2f %.2f %.2f\n",
  //  px, py, pth );

  // add our center of rotation offsets
  px += origin_x;
  py += origin_y;

  //printf( "map local shifted %.2f %.2f %.2f\n",
  //  px, py, pth );

   // convert back to global coords
  this->LocalToGlobal( px, py, pth );

  this->map_px = px;
  this->map_py = py;
  this->map_pth = pth;


  MapEx(map_px, map_py, map_pth, true);
}


///////////////////////////////////////////////////////////////////////////
// Remove the entity from the world
void CEntity::UnMap()
{
  //printf( "UNMAP\t%.2f %.2f %.2f\n",
  //  map_px, map_py, map_pth );

  MapEx(this->map_px, this->map_py, this->map_pth, false);
}


///////////////////////////////////////////////////////////////////////////
// Remap ourself if we have moved
void CEntity::ReMap(double px, double py, double pth)
{
  // if we haven't moved, do nothing
  if (fabs(px - this->map_px) < 1 / m_world->ppm &&
      fabs(py - this->map_py) < 1 / m_world->ppm &&
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
  //printf( "mapex %.2f %.2f %.2f\n",
  //  px, py, pth );
  
  switch (this->shape)
    {
    case ShapeRect:
      m_world->SetRectangle(px, py, pth, 
			    this->size_x, this->size_y, this, render);
      break;
    case ShapeCircle:
      m_world->SetCircle(px, py, this->size_x / 2, this, render);
      break;
    case ShapeNone:
      break;
    }
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

  switch( this->shape ) 
  {
    case ShapeRect:
    {
      CRectangleIterator rit( qx, qy, qth, sx, sy, m_world->ppm, m_world->matrix );

      CEntity* ent;
      while( (ent = rit.GetNextEntity()) )
      {
        if( ent != this && ent->obstacle_return )
	    return ent;
      }
      return NULL;
    }
    case ShapeCircle:
    {
      CCircleIterator rit( px, py, sx / 2, m_world->ppm, m_world->matrix );

      CEntity* ent;
      while( (ent = rit.GetNextEntity()) )
      {
        if( ent != this && ent->obstacle_return )
	    return ent;
      }
      return NULL;
    }
  case ShapeNone:
    break;
  }
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

  switch( this->shape ) 
  {
    case ShapeRect:
    {
      CRectangleIterator rit( qx, qy, qth, sx, sy, m_world->ppm, m_world->matrix );

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
      CCircleIterator rit( px, py, sx / 2, m_world->ppm, m_world->matrix );

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
  
  if( this->local_px != px ) 
    SetProperty( -1, PropPoseX, &px, sizeof(px) );
  
  if( this->local_py != py ) 
    SetProperty( -1, PropPoseY, &py, sizeof(py) );
  
  if( this->local_pth != pth ) 
    SetProperty( -1, PropPoseTh, &pth, sizeof(pth) );  
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
  for( int i=0; i< ENTITY_LAST_PROPERTY; i++ )
    SetDirty( con, (EntityProperty)i, v );
}

void CEntity::SetDirty( EntityProperty prop, char v )
{
  for( int i=0; i<MAX_POSE_CONNECTIONS; i++ )
    SetDirty( i, prop, v );
};

void CEntity::SetDirty( int con, EntityProperty prop, char v )
{
  m_dirty[con][prop] = v;
};

void CEntity::SetDirty( char v )
{
  memset( m_dirty, v, 
	  sizeof(char) * MAX_POSE_CONNECTIONS * ENTITY_LAST_PROPERTY );
};

///////////////////////////////////////////////////////////////////////////
// change the parent 
void CEntity::SetParent(CEntity* new_parent) 
{ 
  m_parent_entity = new_parent; 
};


int CEntity::SetProperty( int con, EntityProperty property, 
			  void* value, size_t len )
{
  assert( value );
  assert( len > 0 );
  assert( (int)len < MAX_PROPERTY_DATA_LEN );

  bool refresh_figure = false;
  bool move_figure = false;
  
  switch( property )
  {
    case PropParent:
      // TODO - fix this
      // get a pointer to the (*value)'th entity - that's our new parent
      //this->m_parent_entity = m_world->GetEntity( *(int*)value );     
      break;
    case PropSizeX:
      memcpy( &size_x, (double*)value, sizeof(size_x) );
      // force the device to re-render itself
      refresh_figure = true;
      break;
    case PropSizeY:
      memcpy( &size_y, (double*)value, sizeof(size_y) );
      // force the device to re-render itself
      refresh_figure = true;
      break;
    case PropPoseX:
      memcpy( &local_px, (double*)value, sizeof(local_px) );
      move_figure = true;
      break;
    case PropPoseY:
      memcpy( &local_py, (double*)value, sizeof(local_py) );
      move_figure = true;
      break;
    case PropPoseTh:
      // normalize theta
      local_pth = atan2( sin(*(double*)value), cos(*(double*)value));
      //memcpy( &local_pth, (double*)value, sizeof(local_pth) );
      move_figure = true;
      break;
    case PropOriginX:
      memcpy( &origin_x, (double*)value, sizeof(origin_x) );
      refresh_figure = true;
      break;
    case PropOriginY:
      memcpy( &origin_y, (double*)value, sizeof(origin_y) );
      refresh_figure = true;
      break;
    case PropName:
      strcpy( name, (char*)value );
      // force the device to re-render itself
      refresh_figure = true;
      break;
    case PropColor:
      memcpy( &color, (StageColor*)value, sizeof(color) );
      // force the device to re-render itself
      refresh_figure = true;
      break;
    case PropShape:
      memcpy( &shape, (StageShape*)value, sizeof(shape) );
      // force the device to re-render itself
      refresh_figure = true;
      break;
    case PropLaserReturn:
      memcpy( &laser_return, (LaserReturn*)value, sizeof(laser_return) );
      break;
    case PropIdarReturn:
      memcpy( &idar_return, (IDARReturn*)value, sizeof(idar_return) );
      break;
    case PropSonarReturn:
      memcpy( &sonar_return, (bool*)value, sizeof(sonar_return) );
      break;
    case PropObstacleReturn:
      memcpy( &obstacle_return, (bool*)value, sizeof(obstacle_return) );
      break;
    case PropVisionReturn:
      memcpy( &vision_return, (bool*)value, sizeof(vision_return));
      break;
    case PropPuckReturn:
      memcpy( &puck_return, (bool*)value, sizeof(puck_return) );
      break;

    default:
      printf( "Stage Warning: attempting to set unknown property %d\n", 
              property );
      break;
  }
  
  // indicate that the property is dirty on all _but_ the connection
  // it came from - that way it gets propogated onto to other clients
  // and everyone stays in sync. (assuming no recursive connections...)
  
  this->SetDirty( property, 1 ); // dirty on all cons
  
  if( con != -1 ) // unless this was a local change 
    this->SetDirty( con, property, 0 ); // clean on this con

#ifdef INCLUDE_RTK2
  if( refresh_figure )
  {
    RtkShutdown();
    RtkStartup();
  }
  
  if( move_figure && this->fig )
    rtk_fig_origin(this->fig, local_px, local_py, local_pth );
#endif 

  return 0;
}


int CEntity::GetProperty( EntityProperty property, void* value )
{
  assert( value );

  // indicate no data - this should be overridden below
  int retval = 0;

  switch( property )
  {
    case PropParent:
      // TODO - fix
      // find the parent's position in the world's entity array
      // if parent pointer is null or otherwise invalid, index is -1 
      //{ int parent_index = m_world->GetEntityIndex( m_parent_entity );
      //memcpy( value, &parent_index, sizeof(parent_index) );
      //retval = sizeof(parent_index); }
    break;
    case PropSizeX:
      memcpy( value, &size_x, sizeof(size_x) );
      retval = sizeof(size_x);
      break;
    case PropSizeY:
      memcpy( value, &size_y, sizeof(size_y) );
      retval = sizeof(size_y);
      break;
    case PropPoseX:
      memcpy( value, &local_px, sizeof(local_px) );
      retval = sizeof(local_px);
      break;
    case PropPoseY:
      memcpy( value, &local_py, sizeof(local_py) );
      retval = sizeof(local_py);
      break;
    case PropPoseTh:
      memcpy( value, &local_pth, sizeof(local_pth) );
      retval = sizeof(local_pth);
      break;
    case PropOriginX:
      memcpy( value, &origin_x, sizeof(origin_x) );
      retval = sizeof(origin_x);
      break;
      break;
    case PropOriginY:
      memcpy( value, &origin_y, sizeof(origin_y) );
      retval = sizeof(origin_y);
      break;
    case PropName:
      strcpy( (char*)value, name );
      retval = strlen(name);
      break;
    case PropColor:
      memcpy( value, &color, sizeof(color) );
      retval = sizeof(color);
      break;
    case PropShape:
      memcpy( value, &shape, sizeof(shape) );
      retval = sizeof(shape);
      break;
    case PropLaserReturn:
      memcpy( value, &laser_return, sizeof(laser_return) );
      retval = sizeof(laser_return);
      break;
    case PropSonarReturn:
      memcpy( value, &sonar_return, sizeof(sonar_return) );
      retval = sizeof(sonar_return);
      break;
    case PropIdarReturn:
      memcpy( value, &idar_return, sizeof(idar_return) );
      retval = sizeof(idar_return);
      break;
    case PropObstacleReturn:
      memcpy( value, &obstacle_return, sizeof(obstacle_return) );
      retval = sizeof(obstacle_return);
      break;
    case PropVisionReturn:
      memcpy( value, &vision_return, sizeof(vision_return) );
      retval = sizeof(vision_return);
      break;
    case PropPuckReturn:
      memcpy( value, &puck_return, sizeof(puck_return) );
      retval = sizeof(puck_return);
      break;
    default:
      printf( "Stage Warning: attempting to get unknown property %d\n", 
	      property );
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
	  " local: [%.2f,%.2f,%.2f] )", 
	  prefix,
	  m_world->lib->StringFromType( this->stage_type ),
	  ox, oy, oth,
	  local_px, local_py, local_pth );
	  
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


#ifdef INCLUDE_RTK2

///////////////////////////////////////////////////////////////////////////
// Initialise the rtk gui
void CEntity::RtkStartup()
{
  PRINT_DEBUG2("RTK STARTUP %s %s",
	       this->m_world->lib->StringFromType( this->stage_type ),
	       m_parent_entity ? "" : " - ROOT" );

  // Create a figure representing this entity
  if( m_parent_entity == NULL )
    this->fig = rtk_fig_create(m_world->canvas, NULL, 50);
  else
    this->fig = rtk_fig_create(m_world->canvas, m_parent_entity->fig, 50);
  
  this->fig->thing = (void*)this;
  this->fig->origin_callback = staticSetGlobalPose;
  this->fig->select_callback = NULL;
  this->fig->unselect_callback = NULL;


  // add this device to the world's device menu 
  this->m_world->AddToDeviceMenu( this, true); 
    
  // visible by default
  rtk_fig_show( this->fig, true );

  // Set the color
  rtk_fig_color_rgb32(this->fig, this->color);

  // put the figure's origin at the entity's position
  rtk_fig_origin( this->fig, local_px, local_py, local_pth );


#ifdef RENDER_INITIAL_BOUNDING_BOXES
  double xmin, ymin, xmax, ymax;
  xmin = ymin = 999999.9;
  xmax = ymax = 0.0;
  this->GetBoundingBox( xmin, ymin, xmax, ymax );
  
  rtk_fig_t* boundaries = rtk_fig_create( m_world->canvas, NULL, 99);
   double width = xmax - xmin;
  double height = ymax - ymin;
  double xcenter = xmin + width/2.0;
  double ycenter = ymin + height/2.0;

  rtk_fig_rectangle( boundaries, xcenter, ycenter, 0, width, height, 0 ); 

   
#endif
   
  // draw the shape using the center of rotation offsets
  switch (this->shape)
  {
    case ShapeRect:
      rtk_fig_rectangle(this->fig, 
			this->origin_x, this->origin_y, 0, 
			this->size_x, this->size_y, false);
      break;
    case ShapeCircle:
      rtk_fig_ellipse(this->fig, 
		      this->origin_x, this->origin_y, 0,  
		      this->size_x, this->size_y, false);
      break;
    case ShapeNone: // no shape
      break;
  }
  

  // everything except the root object has a label
  if( m_parent_entity )
    {
      // Create the label
      // By default, the label is not shown
      this->fig_label = rtk_fig_create(m_world->canvas, this->fig, 51);
      rtk_fig_show(this->fig_label, false);    
      rtk_fig_movemask(this->fig_label, 0);
      
      char label[1024];
      char tmp[1024];
      
      label[0] = 0;
      snprintf(tmp, sizeof(tmp), "%s %s", 
	       this->name, this->m_world->lib->StringFromType( this->stage_type ) );
      strncat(label, tmp, sizeof(label));
      
      rtk_fig_color_rgb32(this->fig, this->color);
      rtk_fig_text(this->fig_label,  0.75 * size_x,  0.75 * size_y, 0, label);
      
      // attach the label to the main figure
      // rtk will draw the label when the mouse goes over the figure
      this->fig->mouseover_fig = fig_label;
      
      // we can be moved only if we are on the root node
      if (m_parent_entity != this->m_world->GetRoot() )
	rtk_fig_movemask(this->fig, 0);
      else
	rtk_fig_movemask(this->fig, this->movemask);  
    }

  // do our children after we're set
  CHILDLOOP( child ) child->RtkStartup();
  PRINT_DEBUG( "RTK STARTUP DONE" );
}



///////////////////////////////////////////////////////////////////////////
// Finalise the rtk gui
void CEntity::RtkShutdown()
{
  // Clean up the figure we created
  rtk_fig_destroy(this->fig);
  rtk_fig_destroy(this->fig_label);
} 


///////////////////////////////////////////////////////////////////////////
// Update the rtk gui
void CEntity::RtkUpdate()
{
  CHILDLOOP( child ) child->RtkUpdate();

  // TODO this is nasty and inefficient - figure out a better way to
  // do this  

  // if we're not looking at this device, hide it 
  if( !m_world->ShowDeviceBody( this->stage_type) )
  {
    rtk_fig_show(this->fig, false);
  }
  else // we need to show and update this figure
  {
    rtk_fig_show( this->fig, true );
  }
}
#endif


