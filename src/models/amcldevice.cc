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
 * Desc: Dummy adaptive MCL device (the real driver is implemented in Player)
 * Author: Andrew Howard
 * Date: 6 Feb 2003
 * $Id: amcldevice.cc,v 1.1 2003-02-08 07:54:42 inspectorg Exp $
 */

#include "amcldevice.hh"

// a static named constructor - a pointer to this function is given
// to the Library object and paired with a string.  When the string
// is seen in the worldfile, this function is called to create an
// instance of this entity
CAdaptiveMCLDevice* CAdaptiveMCLDevice::Creator(LibraryItem *libit, CWorld *world, CEntity *parent)
{
  return new CAdaptiveMCLDevice(libit, world, parent);
}


// constructor
CAdaptiveMCLDevice::CAdaptiveMCLDevice(LibraryItem* libit, CWorld *world, CEntity *parent)
    : CPlayerEntity(libit, world, parent)
{
    // set the Player IO sizes correctly for this type of Entity
  m_data_len = 0;
  m_command_len = 0;
  m_config_len = 0;
  m_reply_len = 0;

  // set the device code
  m_player.code = PLAYER_LOCALIZE_CODE;

  // This is not a real object, so by default we dont see it
  this->obstacle_return = false;
  this->sonar_return = false;
  this->puck_return = false;
  this->vision_return = false;
  this->idar_return = IDARTransparent;
  this->laser_return = LaserTransparent;

  return;
}

