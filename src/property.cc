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
 * Desc: CEntity method that handles getting, setting and querying
 * properties. It's one big ol'function, so it gets its own file.
 * Author: Richard Vaughan
 * Date: 22 Feb 2003
 * CVS info: $Id: property.cc,v 1.1.2.2 2003-02-24 04:47:12 rtv Exp $
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define DEBUG

#include "sio.h" 
#include "matrix.hh"
#include "entity.hh"

int CEntity::Property( int con, stage_prop_id_t property, 
		       void* value, size_t len, stage_buffer_t* reply )
{
  PRINT_DEBUG2( "ent %d prop %s", stage_id, SIOPropString(property) );
  
  if( value )
    PRINT_DEBUG2( "SET %s %d bytes", SIOPropString(property), len );
  
  if( reply )
    PRINT_DEBUG1( "GET %s", SIOPropString(property) );
  
  switch( property )
    {
    case STG_PROP_ENTITY_PPM:
      if( value )
	{
	  PRINT_WARN( "setting PPM" );
	  assert(len == sizeof(double) );
	  
	  double ppm = *((double*)value);
	  
	  if( CEntity::matrix )
	    {
	      CEntity::matrix->Resize( size_x, size_y, ppm );      
	      this->MapFamily();	  
	    }
	  else
	    PRINT_WARN( "trying to set ppm for non-existent matrix" );
	}
      
      // if a reply is needed, add the current PPM to the buffer
      if( reply )
	{
	  double ppm = -1;
	  
	  if( CEntity::matrix )
	    ppm = CEntity::matrix->ppm;
	  
	  SIOBufferProperty( reply, this->stage_id, STG_PROP_ENTITY_PPM,
			     &ppm, sizeof(ppm), STG_ISREPLY );
	}
	      break;
      
    case STG_PROP_ENTITY_VOLTAGE:
      if( value )
	{
	  PRINT_WARN( "setting voltage" );
	  assert(len == sizeof(double) );
	  this->volts = *((double*)value);
	}
      if( reply )
	SIOBufferProperty( reply, this->stage_id, STG_PROP_ENTITY_PPM,
			   &this->volts, sizeof(this->volts), STG_ISREPLY);
      break;
      
    case STG_PROP_ENTITY_PARENT:
      // TODO - fix this
      // get a pointer to the (*value)'th entity - that's our new parent
      //this->m_parent_entity = m_world->GetEntity( *(int*)value );     
      PRINT_WARN( "STG_PROP_ENTITY_PARENT not implemented" );
      break;
      
    case STG_PROP_ENTITY_POSE:
      if( value )
	{
	  assert( len == sizeof(stage_pose_t) );
	  
	  UnMap();
	  
	  stage_pose_t* pose = (stage_pose_t*)value;      
	  local_px = pose->x;
	  local_py = pose->y;
	  local_pth = pose->a;
	  
	  Map(); // redraw myself  
	}
      
      if( reply )
	{
	  stage_pose_t pose;
	  pose.x = local_px;
	  pose.y = local_py;
	  pose.a = local_pth;
	  
	  PRINT_DEBUG2( "buffering POSE %lu bytes in %p", 
		       (unsigned long)sizeof(pose), reply );
	  
	  SIOBufferProperty( reply, this->stage_id, STG_PROP_ENTITY_POSE,
			     &pose, sizeof(pose), STG_ISREPLY );

	  SIODebugBuffer( reply );
	}
      break;
      
    case STG_PROP_ENTITY_VELOCITY:
      if( value )
	{
	  assert( len == sizeof(stage_velocity_t) );
	  stage_velocity_t* vel = (stage_velocity_t*)value;      
	  vx = vel->x;
	  vy = vel->y;
	  vth = vel->a;
	}

      if( reply )
	{
	  stage_velocity_t vel;
	  vel.x = this->vx;
	  vel.y = this->vy;
	  vel.a = this->vth;
	  
	  SIOBufferProperty( reply, this->stage_id, STG_PROP_ENTITY_VELOCITY,
			     &vel, sizeof(vel), STG_ISREPLY );
	}
      
      break;
      
    case STG_PROP_ENTITY_SIZE:
      if( value )
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

      if( reply )
	{
	  stage_size_t sz;
	  sz.x = size_x;
	  sz.y = size_y;
	  
	  SIOBufferProperty( reply, this->stage_id, STG_PROP_ENTITY_SIZE,
			     &sz, sizeof(sz), STG_ISREPLY );
	}

      break;
      
    case STG_PROP_ENTITY_ORIGIN:
      if( value )
	{
	  assert( len == sizeof(stage_size_t) );
	  
	  UnMapFamily();
	  
	  stage_size_t* og = (stage_size_t*)value;
	  origin_x = og->x;
	  origin_y = og->y; 
	  
	  MapFamily();
	}
      
      if( reply )
	{
	  stage_size_t sz;
	  sz.x = origin_x;
	  sz.y = origin_y;
	  
	  SIOBufferProperty( reply, this->stage_id, STG_PROP_ENTITY_ORIGIN,
			     &sz, sizeof(sz), STG_ISREPLY );
	}
      break;
    
    case STG_PROP_ENTITY_RECTS:
      if( value )
	{
	  this->SetRects( (stage_rotrect_t*)value, 
			  len/sizeof(stage_rotrect_t) );
	}

      if( reply )
	{
	  SIOBufferProperty( reply, this->stage_id, STG_PROP_ENTITY_RECTS,
			     this->rects, 
			     this->rect_count * sizeof(stage_rotrect_t), 
			     STG_ISREPLY );
	}
      break;

    case STG_PROP_ENTITY_SUBSCRIBE:
      if( value )
	{
	  // *value is an array of integer property codes that request
	  // subscriptions on this channel
	  PRINT_DEBUG2( "received SUBSCRIBE for %d properties on %d",
			(int)(len/sizeof(int)), con );
	  this->Subscribe( con, (stage_prop_id_t*)value, len/sizeof(int) );
	}
      
      if( reply ) // nothing interesting to reply here, just a confirm
	{
	  SIOBufferProperty( reply, this->stage_id, 
			     STG_PROP_ENTITY_SUBSCRIBE,
			     NULL, 0, STG_ISREPLY ); 
	}
      
      break;
      
    case STG_PROP_ENTITY_UNSUBSCRIBE:
      if( value )
	{
	  // *value is an array of integer property codes that request
	  // subscriptions on this channel
	  PRINT_DEBUG2( "received UNSUBSCRIBE for %d properties on %d",
			(int)(len/sizeof(int)), con );
	  
	  this->Unsubscribe( con, (stage_prop_id_t*)value, len/sizeof(int) );
	}

      if( reply ) // nothing interesting to reply here, just a confirm
	{
	  SIOBufferProperty( reply, this->stage_id,
			     STG_PROP_ENTITY_UNSUBSCRIBE,
			     NULL, 0, STG_ISREPLY ); 
	}      
      break;
      
    case STG_PROP_ENTITY_NAME:
      if( value )
	{
	  assert( len <= STG_TOKEN_MAX );
	  strncpy( name, (char*)value, STG_TOKEN_MAX );
	}
      
      if( reply )
	{
	  SIOBufferProperty( reply, this->stage_id, STG_PROP_ENTITY_NAME,
			     name, STG_TOKEN_MAX, STG_ISREPLY ); 
	}
      break;
      
    case STG_PROP_ENTITY_COLOR:
      if( value )
	{
	  assert( len == sizeof(StageColor) );
	  memcpy( &color, (StageColor*)value, sizeof(color) );
	}
       if( reply ) 
	 {
	   SIOBufferProperty( reply, this->stage_id, STG_PROP_ENTITY_COLOR,
			      &color, sizeof(color), STG_ISREPLY ); 
	 }      
     break;
            
    case STG_PROP_ENTITY_LASERRETURN:
      if( value )
	{
	  assert( len == sizeof(LaserReturn) );
	  memcpy( &laser_return, (LaserReturn*)value, sizeof(laser_return) );
	}
      if( reply ) 
	{
	  SIOBufferProperty( reply, this->stage_id, 
			     STG_PROP_ENTITY_LASERRETURN,
			     &laser_return, sizeof(laser_return), 
			     STG_ISREPLY ); 
	}      
      break;
      
    case STG_PROP_ENTITY_IDARRETURN:
      if( value )
	{
	  assert( len == sizeof(IDARReturn) );
	  memcpy( &idar_return, (IDARReturn*)value, sizeof(idar_return) );
	}
      if( reply ) 
	{
	  SIOBufferProperty( reply, this->stage_id, 
			     STG_PROP_ENTITY_IDARRETURN,
			     &idar_return, sizeof(idar_return), 
			     STG_ISREPLY ); 
	}      
      break;
      
    case STG_PROP_ENTITY_SONARRETURN:
      if( value )
	{
	  assert( len == sizeof(bool) );
	  memcpy( &sonar_return, (bool*)value, sizeof(sonar_return) );
	}
      if( reply ) 
	{
	  SIOBufferProperty( reply, this->stage_id, 
			     STG_PROP_ENTITY_SONARRETURN,
			     &sonar_return, sizeof(sonar_return),
			     STG_ISREPLY ); 
	}      
      break;
      
    case STG_PROP_ENTITY_OBSTACLERETURN:
      if( value )
	{
	  assert( len == sizeof(bool) );
	  memcpy( &obstacle_return, (bool*)value, sizeof(obstacle_return) );
	}
      if( reply ) 
	{
	  SIOBufferProperty( reply, this->stage_id, 
			     STG_PROP_ENTITY_OBSTACLERETURN,
			     &obstacle_return, sizeof(obstacle_return), 
			     STG_ISREPLY ); 
	}      
      break;
      
    case STG_PROP_ENTITY_VISIONRETURN:
      if( value )
	{
	  assert( len == sizeof(bool) );
	  memcpy( &vision_return, (bool*)value, sizeof(vision_return));
	}
      if( reply ) 
	{
	  SIOBufferProperty( reply, this->stage_id, 
			     STG_PROP_ENTITY_VISIONRETURN,
			     &vision_return, sizeof(vision_return),
			     STG_ISREPLY ); 
	}      
      break;
      
    case STG_PROP_ENTITY_PUCKRETURN:
      if( value )
	{
	  assert( len == sizeof(bool) );
	  memcpy( &puck_return, (bool*)value, sizeof(puck_return) );
	}
      if( reply ) 
	{
	  SIOBufferProperty( reply, this->stage_id, 
			     STG_PROP_ENTITY_PUCKRETURN,
			     &puck_return, sizeof(puck_return)
			     , STG_ISREPLY ); 
	}      
      break;
      
      
      // DISPATCH COMMANDS TO DEVICES
    case STG_PROP_ENTITY_COMMAND:
      if( value )
	{
	  // store the data in case someone fetches it
	  if( buffer_cmd.len != len )
	    buffer_cmd.data = (char*)realloc( buffer_cmd.data, len );
	  memcpy( buffer_cmd.data, value, len );
	  buffer_cmd.len = len;

	  // virtual function is overridden in subclasses.  the
	  // subclass handles the command in their own special way
	  this->SetCommand( (char*)value, len ); 
	}
      if( reply ) 
	{
	  // reply with the contents of the command buffer
	  SIOBufferProperty( reply, this->stage_id, 
			     STG_PROP_ENTITY_COMMAND,
			     buffer_cmd.data, buffer_cmd.len, 
			     STG_ISREPLY );
	}      
      break;
      
    case STG_PROP_ENTITY_DATA:
      if( value )
	{
	  // store the data in case someone fetches it
	  if( buffer_data.len != len )
	    buffer_data.data = (char*)realloc( buffer_data.data, len );
	  memcpy( buffer_data.data, value, len );
	  buffer_data.len = len;
	  
	  // virtual function is overriden in subclasses. the subclass
	  // may want to react to someone setting its data.
	  this->SetData( (char*)value, len ); 
	}
      if( reply ) 
	{
	  //PRINT_WARN( " about to buffer data prop" );
	  // reply with the contents of the data buffer
	  SIOBufferProperty( reply, this->stage_id, 
			     STG_PROP_ENTITY_DATA,
			     buffer_data.data, buffer_data.len,
			     STG_ISREPLY );
	  //PRINT_WARN( "done" );
	}      
      break;
      
    default:
      //printf( "Stage Warning: attempting to set unknown property %d\n", 
      //      property );
      break;
    }
  
  
  // if there was some incoming property change
  if( value ) 
    {
      // indicate that the property is dirty on all _but_ the connection
      // it came from - that way it gets propogated onto to other clients
      // and everyone stays in sync. (assuming no recursive connections...)
      this->SetDirty( property, 1 ); // dirty on all cons
      
      if( con != -1 ) // unless this was a local change 
	this->SetDirty( con, property, 0 ); // clean on this con
      
      
      // update the GUI with the new property
      //if( enable_gui )
      GuiEntityPropertyChange( this, property );
    }

  return 0;
}
