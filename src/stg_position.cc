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
 * $Id: stg_position.cc,v 1.1 2004-09-16 06:54:28 rtv Exp $
 */

#include <stdlib.h>

#include "playercommon.h"
#include "drivertable.h"
#include "player.h"
#include "stageclient.h"

// CLASS FOR POSITION INTERFACE //////////////////////////////////////////
class StgPosition:public Stage1p4
{
public:
  StgPosition( ConfigFile* cf, int section );
  
  // override GetData
  virtual size_t GetData(player_device_id_t id,
			 void* dest, size_t len,
			 struct timeval* timestamp);
  
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
  PLAYER_MSG0( "STG_POSITION CONSTRUCTOR" );
}

Driver* StgPosition_Init( ConfigFile* cf, int section)
{
  PLAYER_MSG0( "STG_POSITION INIT" );
  return((Driver*)(new StgPosition( cf, section)));
}
	      
void StgPosition_Register(DriverTable* table)
{
  table->AddDriver("stg_position",  StgPosition_Init);
}

size_t StgPosition::GetData( player_device_id_t id,
			     void* dest, size_t len,
			     struct timeval* timestamp)
{
  stg_property_t* prop = stg_model_get_prop_cached( model, STG_PROP_DATA );
  
  if( prop && prop->len == sizeof(stg_position_data_t) )      
    {      
      stg_position_data_t* posdat = (stg_position_data_t*)prop->data;
      
      //PLAYER_MSG3( "get data posdat %.2f %.2f %.2f", posdat->x, posdat->y, posdat->a );
      player_position_data_t position_data;
      memset( &position_data, 0, sizeof(position_data) );
      // pack the data into player format
      position_data.xpos = ntohl((int32_t)(1000.0 * posdat->pose.x));
      position_data.ypos = ntohl((int32_t)(1000.0 * posdat->pose.y));
      position_data.yaw = ntohl((int32_t)(RTOD(posdat->pose.a)));
      
      // speeds
      position_data.xspeed= ntohl((int32_t)(1000.0 * posdat->velocity.x)); // mm/sec
      position_data.yspeed = ntohl((int32_t)(1000.0 * posdat->velocity.y));// mm/sec
      position_data.yawspeed = ntohl((int32_t)(RTOD(posdat->velocity.a))); // deg/sec
      
      position_data.stall = posdat->stall;// ? 1 : 0;
      
      //printf( "getdata called at %lu ms\n", stage_client->stagetime );
      
      // publish this data
      Driver::PutData( &position_data, sizeof(position_data), NULL); 
    }
  
  // now inherit the standard data-getting behavior 
  return Driver::GetData( id,dest,len,timestamp);
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

      stg_model_prop_delta( this->model, STG_PROP_COMMAND, 
			    &cmd, sizeof(cmd) ) ;
    }
  else
    PLAYER_ERROR2( "wrong size position command packet (%d/%d bytes)",
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
	stg_geom_t geom;
	if( stg_model_prop_get( this->model, STG_PROP_GEOM, &geom,sizeof(geom)))
	  PLAYER_ERROR( "error requesting STG_PROP_GEOM" );
	
	// fill in the geometry data formatted player-like
	player_position_geom_t pgeom;
	pgeom.pose[0] = ntohs((uint16_t)(1000.0 * geom.pose.x));
	pgeom.pose[1] = ntohs((uint16_t)(1000.0 * geom.pose.y));
	pgeom.pose[2] = ntohs((uint16_t)RTOD(geom.pose.a));
	
	pgeom.size[0] = ntohs((uint16_t)(1000.0 * geom.size.x)); 
	pgeom.size[1] = ntohs((uint16_t)(1000.0 * geom.size.y)); 
	
	PutReply( client, 
		  PLAYER_MSGTYPE_RESP_ACK, 
		  &pgeom, sizeof(pgeom), timestamp );
      }
      break;
      
    case PLAYER_POSITION_RESET_ODOM_REQ:
      {
	PRINT_DEBUG( "sending reset odom request" );
	unsigned char msg = STG_MM_POSITION_RESETODOM;
	stg_model_message( this->model, &msg, 1 );
	PutReply( client, PLAYER_MSGTYPE_RESP_ACK, NULL );
      }
      break;

    default:
      {
	PLAYER_WARN1( "stg_position doesn't support config id %d", buf[0] );
        if (PutReply(client, PLAYER_MSGTYPE_RESP_NACK, NULL) != 0)
          PLAYER_ERROR("PutReply() failed");
        break;
      }
    }
  
  return(0);
}

