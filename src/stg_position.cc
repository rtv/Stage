/*
 *  Stage-1.4 driver for Player
 *  Copyright (C) 2003  Richard Vaughan (rtv) vaughan@hrl.com 
 *
 * This program is free software; you can redistribute it and/or modify
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
 * $Id: stg_position.cc,v 1.4 2004-12-03 01:32:57 rtv Exp $
 */

#define DEBUG

#include <stdlib.h>
#include "stg_driver.h"

// CLASS FOR POSITION INTERFACE //////////////////////////////////////////
class StgPosition:public Stage1p4
{
public:
  StgPosition( ConfigFile* cf, int section );
  
  static void PublishData( void* ptr );

  /// override PutCommand
  virtual void PutCommand(player_device_id_t id,
			  void* src, size_t len,
			  struct timeval* timestamp);
  
  // override PutConfig
  virtual int PutConfig(player_device_id_t id, void *client, 
			void* src, size_t len,
			struct timeval* timestamp);
protected:
};


// METHODS ///////////////////////////////////////////////////////////////

StgPosition::StgPosition( ConfigFile* cf, int section ) 
  : Stage1p4( cf, section, PLAYER_POSITION_CODE, PLAYER_ALL_MODE,
	      sizeof(player_position_data_t), sizeof(player_position_cmd_t), 1, 1 )
{
  //PLAYER_MSG0( "STG_POSITION CONSTRUCTOR" );

  this->model->data_notify = StgPosition::PublishData;
  this->model->data_notify_arg = this;
}

Driver* StgPosition_Init( ConfigFile* cf, int section)
{
  //PLAYER_MSG0( "STG_POSITION INIT" );
  return((Driver*)(new StgPosition( cf, section)));
}
	      
void StgPosition_Register(DriverTable* table)
{
  table->AddDriver("stg_position",  StgPosition_Init);
}

// this is called in the simulation thread to stick our data into
// Player
void StgPosition::PublishData( void* ptr )
{
  //puts( "publishing position data" );

  StgPosition* pos = (StgPosition*)ptr;

  size_t datalen=0;
  stg_position_data_t* data = (stg_position_data_t*)
    stg_model_get_data( pos->model, &datalen );
  
  if( data && datalen == sizeof(stg_position_data_t) )      
    {      
      
      //PLAYER_MSG3( "get data data %.2f %.2f %.2f", data->x, data->y, data->a );
      player_position_data_t position_data;
      memset( &position_data, 0, sizeof(position_data) );
      // pack the data into player format
      position_data.xpos = ntohl((int32_t)(1000.0 * data->pose.x));
      position_data.ypos = ntohl((int32_t)(1000.0 * data->pose.y));
      position_data.yaw = ntohl((int32_t)(RTOD(data->pose.a)));
      
      // speeds
      position_data.xspeed= ntohl((int32_t)(1000.0 * data->velocity.x)); // mm/sec
      position_data.yspeed = ntohl((int32_t)(1000.0 * data->velocity.y));// mm/sec
      position_data.yawspeed = ntohl((int32_t)(RTOD(data->velocity.a))); // deg/sec
      
      position_data.stall = data->stall;// ? 1 : 0;
      
      //printf( "getdata called at %lu ms\n", stage_client->stagetime );
      
      // publish this data
      pos->PutData( &position_data, sizeof(position_data), NULL); 
    }
}
  

void  StgPosition::PutCommand(player_device_id_t id,
			      void* src, size_t len,
			      struct timeval* timestamp)
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
      
      stg_model_set_command( this->model, &cmd, sizeof(cmd) ) ;
    }
  else
    PRINT_ERR2( "wrong size position command packet (%d/%d bytes)",
		   (int)len, (int)sizeof(player_position_cmd_t) );
}


int StgPosition::PutConfig( player_device_id_t id, void *client, 
			    void* src, size_t len,
			    struct timeval* timestamp)
{
  // switch on the config type (first byte)
  uint8_t* buf = (uint8_t*)src;
  switch( buf[0] )
    {  
    case PLAYER_POSITION_GET_GEOM_REQ:
      {
	stg_geom_t* geom = stg_model_get_geom( this->model );
	assert(geom);

	// fill in the geometry data formatted player-like
	player_position_geom_t pgeom;
	pgeom.pose[0] = ntohs((uint16_t)(1000.0 * geom->pose.x));
	pgeom.pose[1] = ntohs((uint16_t)(1000.0 * geom->pose.y));
	pgeom.pose[2] = ntohs((uint16_t)RTOD(geom->pose.a));
	
	pgeom.size[0] = ntohs((uint16_t)(1000.0 * geom->size.x)); 
	pgeom.size[1] = ntohs((uint16_t)(1000.0 * geom->size.y)); 
	
	PutReply( client, 
		  PLAYER_MSGTYPE_RESP_ACK, 
		  &pgeom, sizeof(pgeom), timestamp );
      }
      break;
      
    case PLAYER_POSITION_RESET_ODOM_REQ:
      {
	PRINT_DEBUG( "resetting odometry" );
	
	stg_pose_t origin;
	memset(&origin,0,sizeof(origin));
	stg_model_set_odom( this->model, &origin );
	
	PutReply( client, PLAYER_MSGTYPE_RESP_ACK, NULL );
      }
      break;

    case PLAYER_POSITION_SET_ODOM_REQ:
      {
	player_position_set_odom_req_t* req = 
	  (player_position_set_odom_req_t*)src;
	
	PRINT_DEBUG3( "setting odometry to (%.d,%d,%.2f)",
		      (int32_t)ntohl(req->x),
		      (int32_t)ntohl(req->y),
		      DTOR((int32_t)ntohl(req->theta)) );
	
	stg_pose_t pose;
	pose.x = ((double)(int32_t)ntohl(req->x)) / 1000.0;
	pose.y = ((double)(int32_t)ntohl(req->y)) / 1000.0;
	pose.a = DTOR( (double)(int32_t)ntohl(req->theta) );

	stg_model_set_odom( this->model, &pose );
	
	PRINT_DEBUG3( "set odometry to (%.2f,%.2f,%.2f)",
		      pose.x,
		      pose.y,
		      pose.a );

	PRINT_DEBUG3( "set odometry to (%.2f,%.2f,%.2f)",
		      this->model->odom.x,
		      this->model->odom.y,
		      this->model->odom.a );
	
	PutReply( client, PLAYER_MSGTYPE_RESP_ACK, NULL );
      }
      break;

    case PLAYER_POSITION_MOTOR_POWER_REQ:
      {
	player_position_power_config_t* req = 
	  (player_position_power_config_t*)src;

	int motors_on = req->value;

	PRINT_WARN1( "Stage ignores motor power state (%d)",
		     motors_on );
	PutReply( client, PLAYER_MSGTYPE_RESP_ACK, NULL );
      }
      break;

    default:
      {
	PRINT_WARN1( "stg_position doesn't support config id %d", buf[0] );
        if (PutReply(client, PLAYER_MSGTYPE_RESP_NACK, NULL) != 0)
          DRIVER_ERROR("PutReply() failed");
        break;
      }
    }
  
  return(0);
}

