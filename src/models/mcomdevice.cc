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
 * Desc: Simulates the mcom device (just a place-holder really; most of
 *       the work is done by Player).
 * Author: Reed Hedges <reed@zerohour.net> (Umass/Amherst), based on code by
 *              Andrew Howard
 * Date: Feb 2003
 * CVS info: $Id: mcomdevice.cc,v 1.1.2.1.2.1 2003-12-05 02:08:28 gerkey Exp $
 */

#include <stage1p3.h>
#include "mcomdevice.hh"

///////////////////////////////////////////////////////////////////////////
// Default constructor
//
CMComDevice::CMComDevice(LibraryItem* libit,CWorld *world, CEntity *parent)
    : CPlayerEntity(libit,world, parent )
{
  m_data_len = sizeof(player_mcom_data_t);
  m_command_len = 0;
  m_config_len = 20;
  m_reply_len = 20;

  m_player.code = PLAYER_MCOM_CODE;
}
