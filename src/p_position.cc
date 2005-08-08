/*
 *  Player - One Hell of a Robot Server
 *  Copyright (C) 2004, 2005 Richard Vaughan
 *                      
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
 * Desc: A plugin driver for Player that gives access to Stage devices.
 * Author: Richard Vaughan
 * Date: 10 December 2004
 * CVS: $Id: p_position.cc,v 1.6 2005-08-08 19:00:37 rtv Exp $
 */

// DOCUMENTATION ------------------------------------------------------------

/** @addtogroup player 
@par Position interface

- Command
- Data
- Configs
 - PLAYER_POSITION_SET_ODOM_REQ
 - PLAYER_POSITION_RESET_ODOM_REQ
 - PLAYER_POSITION_GET_GEOM_REQ
 - PLAYER_POSITION_MOTOR_POWER_REQ
 - PLAYER_POSITION_VELOCITY_MODE_REQ
*/

// CODE ----------------------------------------------------------------------

#include "p_driver.h"
#include "playerclient.h"

extern "C" { 
int position_init( stg_model_t* mod );
}

InterfacePosition::InterfacePosition(  player_device_id_t id, 
				       StgDriver* driver,
				       ConfigFile* cf,
				       int section )
						   
  : InterfaceModel( id, driver, cf, section, position_init )
{
  //puts( "InterfacePosition constructor" );
  this->data_len = sizeof(player_position_data_t);
  this->cmd_len = sizeof(player_position_cmd_t);  
}

void InterfacePosition::Command( void* src, size_t len )
{
  if( len == sizeof(player_position_cmd_t) )
    {
      // convert from Player to Stage format
      player_position_cmd_t* pcmd = (player_position_cmd_t*)src;
      
      stg_position_cmd_t scmd; 
      memset( &scmd, 0, sizeof(scmd));

      switch( pcmd->type )
	{
	case 0: // velocity mode	  
	  scmd.x = ((double)((int32_t)ntohl(pcmd->xspeed))) / 1000.0;
	  scmd.y = ((double)((int32_t)ntohl(pcmd->yspeed))) / 1000.0;
	  scmd.a = DTOR((double)((int32_t)ntohl(pcmd->yawspeed)));
	  scmd.mode = STG_POSITION_CONTROL_VELOCITY;
	  break;

	case 1: // position mode
	  scmd.x = ((double)((int32_t)ntohl(pcmd->xpos))) / 1000.0;
	  scmd.y = ((double)((int32_t)ntohl(pcmd->ypos))) / 1000.0;
	  scmd.a = DTOR((double)((int32_t)ntohl(pcmd->yaw)));
	  scmd.mode = STG_POSITION_CONTROL_POSITION;
	  break;

	default:
	  PRINT_WARN1( "unrecognized player position control type %d\n", pcmd->type );
	  break;
	}	

      stg_model_set_property( this->mod, "position_cmd", &scmd, sizeof(scmd));

    }
  else
    PRINT_ERR2( "wrong size position command packet (%d/%d bytes)",
		(int)len, (int)sizeof(player_position_cmd_t) );
}


//int PositionData( stg_model_t* mod, char* name, void* data, size_t len, void* userp )
void InterfacePosition::Publish( void )
{
  //puts( "publishing position data" ); 
  
  player_position_data_t ppd;
  memset( &ppd, 0, sizeof(ppd) );
  
  stg_position_data_t* data = (stg_position_data_t*)
    stg_model_get_property_fixed( this->mod, "position_data",
				  sizeof(stg_position_data_t ));
  assert(data);
  
  //printf( "stage position data: %.2f,%.2f,%.2f\n",
  //  data->pose.x, data->pose.y, data->pose.a );

  // pack the data into player format
  ppd.xpos = ntohl((int32_t)(1000.0 * data->pose.x));
  ppd.ypos = ntohl((int32_t)(1000.0 * data->pose.y));
  ppd.yaw = ntohl((int32_t)(RTOD(data->pose.a)));
  
  // speeds
  stg_velocity_t* vel = (stg_velocity_t*)
    stg_model_get_property_fixed( this->mod, "velocity",sizeof(stg_velocity_t));
  assert(vel);
  
  ppd.xspeed= ntohl((int32_t)(1000.0 * vel->x)); // mm/sec
  ppd.yspeed = ntohl((int32_t)(1000.0 * vel->y));// mm/sec
  ppd.yawspeed = ntohl((int32_t)(RTOD(vel->a))); // deg/sec
  
  stg_position_stall_t* stall= (stg_position_stall_t*)
    stg_model_get_property_fixed( this->mod, "position_stall",sizeof(stg_position_stall_t));
  assert(stall);
  
  // etc
  ppd.stall = *stall;
  
  // publish this data
  this->driver->PutData( this->id, &ppd, sizeof(ppd), NULL);      
}

void InterfacePosition::Configure( void* client, void* buffer, size_t len  )
{
  //printf("got position request\n");
  
  // switch on the config type (first byte)
  uint8_t* buf = (uint8_t*)buffer;
  switch( buf[0] )
    {  
    case PLAYER_POSITION_GET_GEOM_REQ:
      {
	stg_geom_t geom;
	stg_model_get_geom( this->mod,&geom );

	// fill in the geometry data formatted player-like
	player_position_geom_t pgeom;
	pgeom.subtype = PLAYER_POSITION_GET_GEOM_REQ;
	pgeom.pose[0] = ntohs((uint16_t)(1000.0 * geom.pose.x));
	pgeom.pose[1] = ntohs((uint16_t)(1000.0 * geom.pose.y));
	pgeom.pose[2] = ntohs((uint16_t)RTOD(geom.pose.a));
	
	pgeom.size[0] = ntohs((uint16_t)(1000.0 * geom.size.x)); 
	pgeom.size[1] = ntohs((uint16_t)(1000.0 * geom.size.y)); 
	
	this->driver->PutReply( this->id, client, 
				PLAYER_MSGTYPE_RESP_ACK, 
				&pgeom, sizeof(pgeom), NULL );
      }
      break;
      
    case PLAYER_POSITION_RESET_ODOM_REQ:
      {
	PRINT_DEBUG( "resetting odometry" );
	
	stg_pose_t origin;
	memset(&origin,0,sizeof(origin));
	stg_model_position_set_odom( this->mod, &origin );
	
	this->driver->PutReply( this->id, client, PLAYER_MSGTYPE_RESP_ACK, NULL );
      }
      break;

    case PLAYER_POSITION_SET_ODOM_REQ:
      {
	player_position_set_odom_req_t* req = 
	  (player_position_set_odom_req_t*)buffer;
	
	PRINT_DEBUG3( "setting odometry to (%.d,%d,%.2f)",
		      (int32_t)ntohl(req->x),
		      (int32_t)ntohl(req->y),
		      DTOR((int32_t)ntohl(req->theta)) );
	
	stg_pose_t pose;
	pose.x = ((double)(int32_t)ntohl(req->x)) / 1000.0;
	pose.y = ((double)(int32_t)ntohl(req->y)) / 1000.0;
	pose.a = DTOR( (double)(int32_t)ntohl(req->theta) );

	stg_model_position_set_odom( this->mod, &pose );
	
	PRINT_DEBUG3( "set odometry to (%.2f,%.2f,%.2f)",
		      pose.x,
		      pose.y,
		      pose.a );

	this->driver->PutReply( this->id, client, PLAYER_MSGTYPE_RESP_ACK, NULL );
      }
      break;

    case PLAYER_POSITION_MOTOR_POWER_REQ:
      {
	player_position_power_config_t* req = 
	  (player_position_power_config_t*)buffer;

	int motors_on = req->value;

	PRINT_WARN1( "Stage ignores motor power state (%d)",
		     motors_on );
	this->driver->PutReply( this->id, client, PLAYER_MSGTYPE_RESP_ACK, NULL );
      }
      break;

    case PLAYER_POSITION_POSITION_MODE_REQ:
      {
	player_position_position_mode_req_t* req = 
	  (player_position_position_mode_req_t*)buffer;
	
	stg_position_control_mode_t mode = (stg_position_control_mode_t)req->state;
	stg_model_set_property( mod, "position_control", &mode, sizeof(mode));
	
	PRINT_WARN2( "Put model %s into %s control mode", this->mod->token, mod ? "POSITION" : "VELOCITY" );
      
	this->driver->PutReply( this->id, client, PLAYER_MSGTYPE_RESP_ACK, NULL );
      }
      break;

    default:
      {
	PRINT_WARN1( "stg_position doesn't support config id %d", buf[0] );
        if( this->driver->PutReply( this->id, 
				    client, 
				    PLAYER_MSGTYPE_RESP_NACK, NULL) != 0)
          DRIVER_ERROR("PutReply() failed");
        break;
      }
    }
}

