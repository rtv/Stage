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
 * CVS: $Id: p_position.cc,v 1.2 2005-07-22 21:02:02 rtv Exp $
 */


#include "p_driver.h"

#include "playerclient.h"



// POSITION INTERFACE -----------------------------------------------------------------------------



InterfacePosition::InterfacePosition(  player_device_id_t id, 
				       StgDriver* driver,
				       ConfigFile* cf,
				       int section )
						   
  : InterfaceModel( id, driver, cf, section, STG_MODEL_POSITION )
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
      
      // only velocity control mode works yet
      stg_position_cmd_t cmd; 
      cmd.x = ((double)((int32_t)ntohl(pcmd->xspeed))) / 1000.0;
      cmd.y = ((double)((int32_t)ntohl(pcmd->yspeed))) / 1000.0;
      cmd.a = DTOR((double)((int32_t)ntohl(pcmd->yawspeed)));
      cmd.mode = STG_POSITION_CONTROL_VELOCITY;
      
      // TODO
      // only set the command if it's different from the old one
      // (saves aquiring a mutex lock from the sim thread)
            
      //if( memcmp( &cmd, buf, sizeof(cmd) ))
	{	  
	  //static int g=0;
	  //printf( "setting command %d\n", g++ );
	  //stg_model_set_command( device->mod, &cmd, sizeof(cmd) ) ;
	  stg_model_set_property( this->mod, "position_cmd", &cmd, sizeof(cmd));
	}
    }
  else
    PRINT_ERR2( "wrong size position command packet (%d/%d bytes)",
		(int)len, (int)sizeof(player_position_cmd_t) );
}


//int PositionData( stg_model_t* mod, char* name, void* data, size_t len, void* userp )
void InterfacePosition::Publish( void )
{
  //puts( "publishing position data" ); 
  
  //Interface* device = (Interface*)userp;
  
  stg_position_data_t* stgdata = (stg_position_data_t*)
    stg_model_get_property_fixed( this->mod, "position_data", 
				  sizeof(stg_position_data_t ));
  
  //if( len == sizeof(stg_position_data_t) )      
  //  {      
      //stg_position_data_t* stgdata = (stg_position_data_t*)data;
                  
      //PLAYER_MSG3( "get data data %.2f %.2f %.2f", stgdatax, stgdatay, stgdataa );
      player_position_data_t position_data;
      memset( &position_data, 0, sizeof(position_data) );

      // pack the data into player format
      position_data.xpos = ntohl((int32_t)(1000.0 * stgdata->pose.x));
      position_data.ypos = ntohl((int32_t)(1000.0 * stgdata->pose.y));
      position_data.yaw = ntohl((int32_t)(RTOD(stgdata->pose.a)));
      
      // speeds
      position_data.xspeed= ntohl((int32_t)(1000.0 * stgdata->velocity.x)); // mm/sec
      position_data.yspeed = ntohl((int32_t)(1000.0 * stgdata->velocity.y));// mm/sec
      position_data.yawspeed = ntohl((int32_t)(RTOD(stgdata->velocity.a))); // deg/sec
      
      // etc
      position_data.stall = stgdata->stall;// ? 1 : 0;
      
      // publish this data
      this->driver->PutData( this->id, &position_data, sizeof(position_data), NULL);      
      //}
      //else
      //PRINT_ERR2( "wrong size position data (%d/%d bytes)",
      //	(int)len, (int)sizeof(player_position_data_t) );
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

