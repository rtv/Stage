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
 * Desc: Simulates the broadcast device (just a place-holder really; most of
 *       the work is done by Player).
 * Author: Andrew Howard
 * Date: 5 Dec 2000
 * CVS info: $Id: broadcastdevice.cc,v 1.14 2002-08-16 06:18:35 gerkey Exp $
 */

#include <stage.h>
#include "world.hh"
#include "broadcastdevice.hh"


///////////////////////////////////////////////////////////////////////////
// Default constructor
//
CBroadcastDevice::CBroadcastDevice(CWorld *world, CEntity *parent)
    : CEntity(world, parent )
{
  // Set the Player IO sizes.  We need to have both request and reply
  // queues, but dont use data or command buffers.  I've arbitrarily
  // set the queue length to 10, so the could overflow if there are
  // lots of messages flying.
  m_data_len = 0;
  m_command_len = 0;
  m_config_len = 10;
  m_reply_len = 10;

  m_player.code = PLAYER_COMMS_CODE;
  this->stage_type = BroadcastType;
}
