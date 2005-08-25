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
 * CVS: $Id: p_position.cc,v 1.7 2005-08-25 19:15:24 gerkey Exp $
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
//#include "playerclient.h"

extern "C" { 
int position_init( stg_model_t* mod );
}

InterfacePosition::InterfacePosition(  player_devaddr_t addr, 
				       StgDriver* driver,
				       ConfigFile* cf,
				       int section )
						   
  : InterfaceModel( addr, driver, cf, section, position_init )
{
  //puts( "InterfacePosition constructor" );
}

int InterfacePosition::ProcessMessage(MessageQueue* resp_queue,
                                      player_msghdr_t* hdr,
                                      void* data)
{
  // Is it a new motor command?
  if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_CMD, 
                           PLAYER_POSITION2D_CMD_STATE, 
                           this->addr))
  {
    if( hdr->size == sizeof(player_position2d_cmd_t) )
    {
      // convert from Player to Stage format
      player_position2d_cmd_t* pcmd = (player_position2d_cmd_t*)data;

      stg_position_cmd_t scmd; 
      memset( &scmd, 0, sizeof(scmd));

      switch( pcmd->type )
      {
        case 0: // velocity mode	  
          scmd.x = pcmd->vel.px;
          scmd.y = pcmd->vel.py;
          scmd.a = pcmd->vel.pa;
          scmd.mode = STG_POSITION_CONTROL_VELOCITY;
          break;

        case 1: // position mode
          scmd.x = pcmd->pos.px;
          scmd.y = pcmd->pos.py;
          scmd.a = pcmd->pos.pa;
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
                  (int)hdr->size, (int)sizeof(player_position2d_cmd_t) );
    return(0);
  }
  // Is it a request for position geometry?
  else if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ, 
                                PLAYER_POSITION2D_REQ_GET_GEOM, 
                                this->addr))
  {
    if(hdr->size == 0)
    {
      stg_geom_t geom;
      stg_model_get_geom( this->mod,&geom );

      // fill in the geometry data formatted player-like
      player_position2d_geom_t pgeom;
      pgeom.pose.px = geom.pose.x;
      pgeom.pose.py = geom.pose.y;
      pgeom.pose.pa = geom.pose.a;

      pgeom.size.sl = geom.size.x; 
      pgeom.size.sw = geom.size.y; 

      this->driver->Publish( this->addr, resp_queue,
                             PLAYER_MSGTYPE_RESP_ACK, 
                             PLAYER_POSITION2D_REQ_GET_GEOM,
                             (void*)&pgeom, sizeof(pgeom), NULL );
      return(0);
    }
    else
    {
      PRINT_ERR2("config request len is invalid (%d != %d)", 
		 (int)hdr->size, 0);
      return(-1);
    }
  }
  // Is it a request to reset odometry?
  else if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ, 
                                PLAYER_POSITION2D_REQ_RESET_ODOM, 
                                this->addr))
  {
    if(hdr->size == 0)
    {
      PRINT_DEBUG( "resetting odometry" );

      stg_pose_t origin;
      memset(&origin,0,sizeof(origin));
      stg_model_position_set_odom( this->mod, &origin );

      this->driver->Publish( this->addr, resp_queue, 
                             PLAYER_MSGTYPE_RESP_ACK,
                             PLAYER_POSITION2D_REQ_RESET_ODOM );
      return(0);
    }
    else
    {
      PRINT_ERR2("config request len is invalid (%d != %d)", 
                 (int)hdr->size, 0);
      return(-1);
    }
  }
  // Is it a request to set odometry?
  else if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ, 
                                PLAYER_POSITION2D_REQ_SET_ODOM, 
                                this->addr))
  {
    if(hdr->size == sizeof(player_position2d_set_odom_req_t))
    {
      player_position2d_set_odom_req_t* req = 
              (player_position2d_set_odom_req_t*)data;

      stg_pose_t pose;
      pose.x = req->pose.px;
      pose.y = req->pose.py;
      pose.a = req->pose.pa;

      stg_model_position_set_odom( this->mod, &pose );

      PRINT_DEBUG3( "set odometry to (%.2f,%.2f,%.2f)",
                    pose.x,
                    pose.y,
                    pose.a );

      this->driver->Publish( this->addr, resp_queue, 
                             PLAYER_MSGTYPE_RESP_ACK, 
                             PLAYER_POSITION2D_REQ_SET_ODOM );
      return(0);
    }
    else
    {
      PRINT_ERR2("config request len is invalid (%d != %d)", 
                 (int)hdr->size, (int)sizeof(player_position2d_set_odom_req_t));
      return(-1);
    }
  }
  // Is it a request to enable motor power?
  else if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ, 
                                PLAYER_POSITION2D_REQ_MOTOR_POWER, 
                                this->addr))
  {
    if(hdr->size == sizeof(player_position2d_power_config_t))
    {
      player_position2d_power_config_t* req = 
              (player_position2d_power_config_t*)data;

      int motors_on = req->state;

      PRINT_WARN1( "Stage ignores motor power state (%d)",
                   motors_on );
      this->driver->Publish( this->addr, resp_queue, 
                             PLAYER_MSGTYPE_RESP_ACK, 
                             PLAYER_POSITION2D_REQ_MOTOR_POWER );
      return(0);
    }
    else
    {
      PRINT_ERR2("config request len is invalid (%d != %d)", 
                 (int)hdr->size, (int)sizeof(player_position2d_power_config_t));
      return(-1);
    }
  }
  // Is it a request to switch control mode?
  else if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ, 
                                PLAYER_POSITION2D_REQ_POSITION_MODE, 
                                this->addr))
  {
    if(hdr->size == sizeof(player_position2d_position_mode_req_t))
    {
      player_position2d_position_mode_req_t* req = 
              (player_position2d_position_mode_req_t*)data;

      stg_position_control_mode_t mode = (stg_position_control_mode_t)req->state;
      stg_model_set_property( mod, "position_control", &mode, sizeof(mode));

      PRINT_WARN2( "Put model %s into %s control mode", this->mod->token, mod ? "POSITION" : "VELOCITY" );

      this->driver->Publish( this->addr, resp_queue, 
                             PLAYER_MSGTYPE_RESP_ACK, 
                             PLAYER_POSITION2D_REQ_POSITION_MODE );
      return(0);
    }
    else
    {
      PRINT_ERR2("config request len is invalid (%d != %d)", 
                 (int)hdr->size, 
                 (int)sizeof(player_position2d_position_mode_req_t));
      return(-1);
    }
  }
  else
  {
    // Don't know how to handle this message.
    PRINT_WARN2( "stg_position doesn't support msg with type/subtype %d/%d",
		 hdr->type, hdr->subtype);
    return(-1);
  }
}

void InterfacePosition::Publish( void )
{
  //puts( "publishing position data" ); 
  
  player_position2d_data_t ppd;
  memset( &ppd, 0, sizeof(ppd) );
  
  stg_position_data_t* data = (stg_position_data_t*)
    stg_model_get_property_fixed( this->mod, "position_data",
				  sizeof(stg_position_data_t ));
  assert(data);
  
  //printf( "stage position data: %.2f,%.2f,%.2f\n",
  //  data->pose.x, data->pose.y, data->pose.a );

  // pack the data into player format
  ppd.pos.px = data->pose.x;
  ppd.pos.py = data->pose.y;
  ppd.pos.pa = data->pose.a;
  
  // speeds
  stg_velocity_t* vel = (stg_velocity_t*)
    stg_model_get_property_fixed( this->mod, "velocity",sizeof(stg_velocity_t));
  assert(vel);
  
  ppd.vel.px= vel->x;
  ppd.vel.py = vel->y;
  ppd.vel.pa = vel->a;
  
  stg_position_stall_t* stall= (stg_position_stall_t*)
    stg_model_get_property_fixed( this->mod, "position_stall",sizeof(stg_position_stall_t));
  assert(stall);
  
  // etc
  ppd.stall = *stall;
  
  // publish this data
  this->driver->Publish( this->addr, NULL,
                         PLAYER_MSGTYPE_DATA, PLAYER_POSITION2D_DATA_STATE,
                         (void*)&ppd, sizeof(ppd), NULL);
}
