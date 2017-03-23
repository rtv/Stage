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
 * CVS: $Id$
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
using namespace Stg;

InterfacePosition::InterfacePosition(player_devaddr_t addr, StgDriver *driver, ConfigFile *cf,
                                     int section)

    : InterfaceModel(addr, driver, cf, section, "position")
{
  // puts( "InterfacePosition constructor" );
}

int InterfacePosition::ProcessMessage(QueuePointer &resp_queue, player_msghdr_t *hdr, void *data)
{
  ModelPosition *mod = (ModelPosition *)this->mod;

  // Is it a new motor command?
  if (Message::MatchMessage(hdr, PLAYER_MSGTYPE_CMD, PLAYER_POSITION2D_CMD_VEL, this->addr)) {
    // convert from Player to Stage format
    player_position2d_cmd_vel_t *pcmd = (player_position2d_cmd_vel_t *)data;

    // position_cmd_t scmd;
    // memset( &scmd, 0, sizeof(scmd));

    mod->SetSpeed(pcmd->vel.px, pcmd->vel.py, pcmd->vel.pa);

    return 0;
  }

  // Is it a new motor command?
  else if (Message::MatchMessage(hdr, PLAYER_MSGTYPE_CMD, PLAYER_POSITION2D_CMD_POS, this->addr)) {
    // convert from Player to Stage format
    player_position2d_cmd_pos_t *pcmd = (player_position2d_cmd_pos_t *)data;

    mod->GoTo(pcmd->pos.px, pcmd->pos.py, pcmd->pos.pa);
    return 0;
  }

  // Is it a new motor command?
  else if (Message::MatchMessage(hdr, PLAYER_MSGTYPE_CMD, PLAYER_POSITION2D_CMD_CAR, this->addr)) {
    // convert from Player to Stage format
    player_position2d_cmd_car_t *pcmd = (player_position2d_cmd_car_t *)data;

    mod->SetSpeed(pcmd->velocity, 0, pcmd->angle);
    return 0;
  }

  // Is it a request for position geometry?
  else if (Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ, PLAYER_POSITION2D_REQ_GET_GEOM,
                                 this->addr)) {
    if (hdr->size == 0) {
      Geom geom = this->mod->GetGeom();

      // fill in the geometry data formatted player-like
      player_position2d_geom_t pgeom;
      pgeom.pose.px = geom.pose.x;
      pgeom.pose.py = geom.pose.y;
      pgeom.pose.pyaw = geom.pose.a;

      pgeom.size.sl = geom.size.x;
      pgeom.size.sw = geom.size.y;

      this->driver->Publish(this->addr, resp_queue, PLAYER_MSGTYPE_RESP_ACK,
                            PLAYER_POSITION2D_REQ_GET_GEOM, (void *)&pgeom, sizeof(pgeom), NULL);
      return 0;
    } else {
      PRINT_ERR2("config request len is invalid (%d != %d)", (int)hdr->size, 0);
      return (-1);
    }
  }
  // Is it a request to reset odometry?
  else if (Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ, PLAYER_POSITION2D_REQ_RESET_ODOM,
                                 this->addr)) {
    if (hdr->size == 0) {
      PRINT_DEBUG("resetting odometry");

      mod->est_pose.x = 0;
      mod->est_pose.y = 0;
      mod->est_pose.z = 0;
      mod->est_pose.a = 0;

      this->driver->Publish(this->addr, resp_queue, PLAYER_MSGTYPE_RESP_ACK,
                            PLAYER_POSITION2D_REQ_RESET_ODOM);
      return 0;
    } else {
      PRINT_ERR2("config request len is invalid (%d != %d)", (int)hdr->size, 0);
      return -1;
    }
  }
  // Is it a request to set odometry?
  else if (Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ, PLAYER_POSITION2D_REQ_SET_ODOM,
                                 this->addr)) {
    if (hdr->size == sizeof(player_position2d_set_odom_req_t)) {
      player_position2d_set_odom_req_t *req = (player_position2d_set_odom_req_t *)data;

      mod->est_pose.x = req->pose.px;
      mod->est_pose.y = req->pose.py;
      // mod->est_pose.z = req->pose.pz;
      mod->est_pose.a = req->pose.pa;

      PRINT_DEBUG3("set odometry to (%.2f,%.2f,%.2f)", pose.x, pose.y, pose.a);

      this->driver->Publish(this->addr, resp_queue, PLAYER_MSGTYPE_RESP_ACK,
                            PLAYER_POSITION2D_REQ_SET_ODOM);
      return (0);
    } else {
      PRINT_ERR2("config request len is invalid (%d != %d)", (int)hdr->size,
                 (int)sizeof(player_position2d_set_odom_req_t));
      return (-1);
    }
  }
  // Is it a request to enable motor power?
  else if (Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ, PLAYER_POSITION2D_REQ_MOTOR_POWER,
                                 this->addr)) {
    if (hdr->size == sizeof(player_position2d_power_config_t)) {
      player_position2d_power_config_t *req = (player_position2d_power_config_t *)data;

      int motors_on = req->state;

      PRINT_WARN1("Stage ignores motor power state (%d)", motors_on);
      this->driver->Publish(this->addr, resp_queue, PLAYER_MSGTYPE_RESP_ACK,
                            PLAYER_POSITION2D_REQ_MOTOR_POWER);
      return (0);
    } else {
      PRINT_ERR2("config request len is invalid (%d != %d)", (int)hdr->size,
                 (int)sizeof(player_position2d_power_config_t));
      return (-1);
    }
  }
  // Is it a request to switch control mode?
  else if (Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ, PLAYER_POSITION2D_REQ_POSITION_MODE,
                                 this->addr)) {
    if (hdr->size == sizeof(player_position2d_position_mode_req_t)) {
      // player_position2d_position_mode_req_t* req =
      //      (player_position2d_position_mode_req_t*)data;

      // position_control_mode_t mode = (position_control_mode_t)req->state;

      PRINT_WARN2("Put model %s into %s control mode", this->mod->Token(),
                  mod ? "POSITION" : "VELOCITY");
      PRINT_WARN("set control mode not yet implemented");

      this->driver->Publish(this->addr, resp_queue, PLAYER_MSGTYPE_RESP_ACK,
                            PLAYER_POSITION2D_REQ_POSITION_MODE);
      return (0);
    } else {
      PRINT_ERR2("config request len is invalid (%d != %d)", (int)hdr->size,
                 (int)sizeof(player_position2d_position_mode_req_t));
      return (-1);
    }
  }

  // else

  // Don't know how to handle this message.
  PRINT_WARN2("position doesn't support msg with type %d subtype %d", hdr->type, hdr->subtype);
  return (-1);
}

void InterfacePosition::Publish(void)
{
  // puts( "publishing position data" );

  ModelPosition *mod = (ModelPosition *)this->mod;

  // printf( "stage position data: %.2f,%.2f,%.2f\n",
  //  data->pose.x, data->pose.y, data->pose.a );

  player_position2d_data_t ppd;
  bzero(&ppd, sizeof(ppd));

  // pack the data into player format
  // packing by hand allows for type conversions
  ppd.pos.px = mod->est_pose.x;
  ppd.pos.py = mod->est_pose.y;
  // ppd.pos.pz = mod->est_pose.z;
  ppd.pos.pa = mod->est_pose.a;

  Velocity v = mod->GetVelocity();

  ppd.vel.px = v.x;
  ppd.vel.py = v.y;
  ppd.vel.pa = v.a;

  // etc
  ppd.stall = this->mod->Stalled();

  // publish this data
  this->driver->Publish(this->addr, PLAYER_MSGTYPE_DATA, PLAYER_POSITION2D_DATA_STATE, (void *)&ppd,
                        sizeof(ppd), NULL);
}
