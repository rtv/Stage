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
@par ActArray interface
- PLAYER_ACTARRAY_REQ_GET_GEOM
- PLAYER_ACTARRAY_CMD_POS
- PLAYER_ACTARRAY_CMD_SPEED
- PLAYER_ACTARRAY_DATA_STATE
*/

// CODE ----------------------------------------------------------------------

#include "p_driver.h"
//#include "playerclient.h"

InterfaceActArray::InterfaceActArray(player_devaddr_t addr, StgDriver *driver, ConfigFile *cf,
                                     int section)

    : InterfaceModel(addr, driver, cf, section, "actuator")
{
  // puts( "InterfacePosition constructor" );
}

int InterfaceActArray::ProcessMessage(QueuePointer &resp_queue, player_msghdr_t *hdr, void *data)
{
  Stg::ModelActuator *actmod = (Stg::ModelActuator *)this->mod;

  // Is it a new motor command?
  if (Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ, PLAYER_ACTARRAY_REQ_GET_GEOM, this->addr)) {
    Stg::Geom geom = actmod->GetGeom();

    // fill in the geometry data formatted player-like
    // TODO actually get real data here
    player_actarray_actuatorgeom_t actuator = { 0 };
    actuator.min = actmod->GetMinPosition();
    actuator.max = actmod->GetMaxPosition();
    actuator.type = PLAYER_ACTARRAY_TYPE_LINEAR;

    player_actarray_geom_t pgeom = { 0 };
    pgeom.base_pos.px = geom.pose.x;
    pgeom.base_pos.py = geom.pose.y;
    pgeom.base_pos.pz = geom.pose.z;

    pgeom.base_orientation.pyaw = geom.pose.a;

    pgeom.actuators_count = 1;
    pgeom.actuators = &actuator;

    this->driver->Publish(this->addr, resp_queue, PLAYER_MSGTYPE_RESP_ACK,
                          PLAYER_ACTARRAY_REQ_GET_GEOM, (void *)&pgeom);
    return 0;
  }
  // Is it a request to reset odometry?
  else if (Message::MatchMessage(hdr, PLAYER_MSGTYPE_CMD, PLAYER_ACTARRAY_CMD_POS, this->addr)) {
    player_actarray_position_cmd_t &cmd = *reinterpret_cast<player_actarray_position_cmd_t *>(data);
    actmod->GoTo(cmd.position);
    return 0;
  }
  // Is it a request to reset odometry?
  else if (Message::MatchMessage(hdr, PLAYER_MSGTYPE_CMD, PLAYER_ACTARRAY_CMD_SPEED, this->addr)) {
    player_actarray_speed_cmd_t &cmd = *reinterpret_cast<player_actarray_speed_cmd_t *>(data);
    actmod->SetSpeed(cmd.speed);
    return 0;
  }
  // else

  // Don't know how to handle this message.
  PRINT_WARN2("actuator doesn't support msg with type %d subtype %d", hdr->type, hdr->subtype);
  return (-1);
}

void InterfaceActArray::Publish(void)
{
  Stg::ModelActuator *actmod = (Stg::ModelActuator *)this->mod;

  // TODO fill in values here
  player_actarray_actuator_t act = { 0 };
  act.position = actmod->GetPosition();
  act.speed = actmod->GetSpeed();
  if (act.speed != 0)
    act.state = PLAYER_ACTARRAY_ACTSTATE_MOVING;
  else
    act.state = PLAYER_ACTARRAY_ACTSTATE_IDLE;

  player_actarray_data_t actdata = { 0 };
  actdata.actuators_count = 1;
  actdata.actuators = &act;

  // publish this data
  this->driver->Publish(this->addr, PLAYER_MSGTYPE_DATA, PLAYER_ACTARRAY_DATA_STATE,
                        (void *)&actdata);
}
