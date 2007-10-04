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
 * CVS: $Id: p_position.cc,v 1.1.2.1 2007-10-04 01:17:03 rtv Exp $
 */
// DOCUMENTATION ------------------------------------------------------------

/** @addtogroup player 
@par Position interface
- PLAYER_POSITION2D_CMD_POS
- PLAYER_POSITION2D_CMD_VEL
- PLAYER_POSITION2D_CMD_CAR
- PLAYER_POSITION2D_DATA_STATE
- PLAYER_POSITION2D_REQ_GET_GEOM
- PLAYER_POSITION2D_REQ_MOTOR_POWER
- PLAYER_POSITION2D_REQ_RESET_ODOM
- PLAYER_POSITION2D_REQ_SET_ODOM
- PLAYER_POSITION2D_REQ_POSITION_MODE
*/

// CODE ----------------------------------------------------------------------

#include "p_driver.h"
//#include "playerclient.h"


InterfacePosition::InterfacePosition(  player_devaddr_t addr, 
				       StgDriver* driver,
				       ConfigFile* cf,
				       int section )
						   
  : InterfaceModel( addr, driver, cf, section, "position" )
{
  //puts( "InterfacePosition constructor" );
}

int InterfacePosition::ProcessMessage(MessageQueue* resp_queue,
                                      player_msghdr_t* hdr,
                                      void* data)
{
  StgModelPosition* mod = (StgModelPosition*)this->mod;

  // Is it a new motor command?
  if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_CMD, 
                           PLAYER_POSITION2D_CMD_VEL, 
                           this->addr))
  {
    // convert from Player to Stage format
    player_position2d_cmd_vel_t* pcmd = (player_position2d_cmd_vel_t*)data;

    //stg_position_cmd_t scmd; 
    //memset( &scmd, 0, sizeof(scmd));

    mod->goal.x = pcmd->vel.px;
    mod->goal.y = pcmd->vel.py;
    mod->goal.a = pcmd->vel.pa;
    mod->control_mode = STG_POSITION_CONTROL_VELOCITY;

    return 0;
  }

  // Is it a new motor command?
  else if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_CMD, 
                           PLAYER_POSITION2D_CMD_POS, 
                           this->addr))
  {
    // convert from Player to Stage format
    player_position2d_cmd_pos_t* pcmd = (player_position2d_cmd_pos_t*)data;

    stg_position_cmd_t scmd; 
    memset( &scmd, 0, sizeof(scmd));

    mod->goal.x = pcmd->pos.px;
    mod->goal.y = pcmd->pos.py;
    mod->goal.a = pcmd->pos.pa;
    mod->control_mode = STG_POSITION_CONTROL_POSITION;

    return 0;
  }

  // Is it a new motor command?
  else if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_CMD, 
                           PLAYER_POSITION2D_CMD_CAR, 
                           this->addr))
  {
    // convert from Player to Stage format
    player_position2d_cmd_car_t* pcmd = (player_position2d_cmd_car_t*)data;

    stg_position_cmd_t scmd; 
    memset( &scmd, 0, sizeof(scmd));

    mod->goal.x = pcmd->velocity;
    mod->goal.y = 0;
    mod->goal.a = pcmd->angle;
    mod->control_mode = STG_POSITION_CONTROL_VELOCITY;

    return 0;
  }
 
  // Is it a request for position geometry?
  else if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ, 
                                PLAYER_POSITION2D_REQ_GET_GEOM, 
                                this->addr))
  {
    if(hdr->size == 0)
    {
      stg_geom_t geom;
      this->mod->GetGeom( &geom );

      // fill in the geometry data formatted player-like
      player_position2d_geom_t pgeom;
      pgeom.pose.px = geom.pose.x;
      pgeom.pose.py = geom.pose.y;
      pgeom.pose.pyaw = geom.pose.a;

      pgeom.size.sl = geom.size.x; 
      pgeom.size.sw = geom.size.y; 

      this->driver->Publish( this->addr, resp_queue,
                             PLAYER_MSGTYPE_RESP_ACK, 
                             PLAYER_POSITION2D_REQ_GET_GEOM,
                             (void*)&pgeom, sizeof(pgeom), NULL );
      return 0;
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
      PRINT_WARN( "reset odom not implemented" );
      PRINT_DEBUG( "resetting odometry" );

      stg_pose_t origin;
      memset(&origin,0,sizeof(origin));
      //stg_model_position_set_odom( this->mod, &origin );

      this->driver->Publish( this->addr, resp_queue, 
                             PLAYER_MSGTYPE_RESP_ACK,
                             PLAYER_POSITION2D_REQ_RESET_ODOM );
      return 0;
    }
    else
    {
      PRINT_ERR2("config request len is invalid (%d != %d)", 
                 (int)hdr->size, 0);
      return -1;
    }
  }
  // Is it a request to set odometry?
  else if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ, 
                                PLAYER_POSITION2D_REQ_SET_ODOM, 
                                this->addr))
  {
    if(hdr->size == sizeof(player_position2d_set_odom_req_t))
    {
      PRINT_WARN( "set odom not implemented" );

      player_position2d_set_odom_req_t* req = 
              (player_position2d_set_odom_req_t*)data;

      stg_pose_t pose;
      pose.x = req->pose.px;
      pose.y = req->pose.py;
      pose.a = req->pose.pa;

      //stg_model_position_set_odom( this->mod, &pose );

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
      PRINT_WARN( "set control mode not implemented") ;

      player_position2d_position_mode_req_t* req = 
              (player_position2d_position_mode_req_t*)data;

      stg_position_control_mode_t mode = (stg_position_control_mode_t)req->state;

      // XX should this be in cfg instead?
      //cmd->mode = mode;
      //model_change( mod, &mod->cmd );

      

      //stg_model_set_property( mod, "position_control", &mode, sizeof(mode));

      PRINT_WARN2( "Put model %s into %s control mode", this->mod->Token(), mod ? "POSITION" : "VELOCITY" );

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
  
  //else
  
  // Don't know how to handle this message.
  PRINT_WARN2( "stg_position doesn't support msg with type %d subtype %d",
	       hdr->type, hdr->subtype);
  return(-1);
}

void InterfacePosition::Publish( void )
{
  //puts( "publishing position data" ); 

  StgModelPosition* mod = (StgModelPosition*)this->mod;
  
  //printf( "stage position data: %.2f,%.2f,%.2f\n",
  //  data->pose.x, data->pose.y, data->pose.a );
  
  player_position2d_data_t ppd;
  memset( &ppd, 0, sizeof(ppd) );
  
  // pack the data into player format
  // packing by hand allows for type conversions
  ppd.pos.px = mod->est_pose.x;
  ppd.pos.py = mod->est_pose.y;
  //ppd.pos.pz = mod->est_pose.z;
  ppd.pos.pa = mod->est_pose.a;
  
  ppd.vel.px = mod->velocity.x;
  ppd.vel.py = mod->velocity.y;
  //ppd.vel.pz = mod->velocity.z;
  ppd.vel.pa = mod->velocity.a;
  
  // etc
  ppd.stall = this->mod->Stall();
  
  // publish this data
  this->driver->Publish( this->addr, NULL,
			 PLAYER_MSGTYPE_DATA, PLAYER_POSITION2D_DATA_STATE,
			 (void*)&ppd, sizeof(ppd), NULL);
}
