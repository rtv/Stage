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

#include "p_driver.h"

//#include "playerclient.h" // for the dumb pioneer gripper command defines

/** @addtogroup player
@par Gripper interface
- PLAYER_GRIPPER_DATA_STATE
- PLAYER_GRIPPER_CMD_STATE
- PLAYER_GRIPPER_REQ_GET_GEOM
*/

#include "p_driver.h"
using namespace Stg;

InterfaceGripper::InterfaceGripper(player_devaddr_t addr, StgDriver *driver, ConfigFile *cf,
                                   int section)
    : InterfaceModel(addr, driver, cf, section, "gripper")
{
  // nothing to do
}

void InterfaceGripper::Publish(void)
{
  ModelGripper *gmod = reinterpret_cast<ModelGripper *>(this->mod);

  player_gripper_data_t pdata;
  memset(&pdata, 0, sizeof(pdata));

  // set the proper bits
  pdata.beams = 0;
  pdata.beams |= gmod->GetConfig().beam[0] ? 0x04 : 0x00;
  pdata.beams |= gmod->GetConfig().beam[1] ? 0x08 : 0x00;

  switch (gmod->GetConfig().paddles) {
  case ModelGripper::PADDLE_OPEN: pdata.state = PLAYER_GRIPPER_STATE_OPEN; break;
  case ModelGripper::PADDLE_CLOSED: pdata.state = PLAYER_GRIPPER_STATE_CLOSED; break;
  case ModelGripper::PADDLE_OPENING:
  case ModelGripper::PADDLE_CLOSING: pdata.state = PLAYER_GRIPPER_STATE_MOVING; break;
  default: pdata.state = PLAYER_GRIPPER_STATE_ERROR;
  }

  // Write data
  this->driver->Publish(this->addr, PLAYER_MSGTYPE_DATA, PLAYER_GRIPPER_DATA_STATE, (void *)&pdata);
}

int InterfaceGripper::ProcessMessage(QueuePointer &resp_queue, player_msghdr_t *hdr, void *data)
{
  ModelGripper *gmod = reinterpret_cast<ModelGripper *>(this->mod);

  if (Message::MatchMessage(hdr, PLAYER_MSGTYPE_CMD, PLAYER_GRIPPER_CMD_OPEN, this->addr)) {
    gmod->CommandOpen();
    return 0;
  } else if (Message::MatchMessage(hdr, PLAYER_MSGTYPE_CMD, PLAYER_GRIPPER_CMD_CLOSE, this->addr)) {
    gmod->CommandClose();
    return 0;
  }

  if (Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ, PLAYER_GRIPPER_REQ_GET_GEOM, this->addr)) {
    Geom geom = this->mod->GetGeom();
    Pose pose = this->mod->GetPose();

    player_gripper_geom_t pgeom;
    memset(&pgeom, 0, sizeof(pgeom));

    pgeom.pose.px = pose.x;
    pgeom.pose.py = pose.y;
    pgeom.pose.pz = pose.z;
    pgeom.pose.pyaw = pose.a;

    pgeom.outer_size.sl = geom.size.x;
    pgeom.outer_size.sw = geom.size.y;
    pgeom.outer_size.sh = geom.size.z;

    pgeom.num_beams = 2;

    this->driver->Publish(this->addr, resp_queue, PLAYER_MSGTYPE_RESP_ACK,
                          PLAYER_GRIPPER_REQ_GET_GEOM, (void *)&pgeom);
    return (0);
  }

  PRINT_WARN2("stage gripper doesn't support message id:%d/%d", hdr->type, hdr->subtype);
  return -1;
}
