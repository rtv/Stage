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
 * CVS: $Id: p_laser.cc,v 1.2 2008-01-15 01:25:42 rtv Exp $
 */

// DOCUMENTATION ------------------------------------------------------------

/** @addtogroup player
@par AIO interface
- PLAYER_AIO_CMD_STATE
- PLAYER_AIO_DATA_STATE
*/

// CODE ----------------------------------------------------------------------

#include "p_driver.h"

InterfaceAio::InterfaceAio( player_devaddr_t addr,
				StgDriver* driver,
				ConfigFile* cf,
				int section )
  : InterfaceModel( addr, driver, cf, section, Stg::MODEL_TYPE_LOADCELL ) // TODO: AIO should not always mean a loadcell
{
}

void InterfaceAio::Publish( void )
{
  Stg::ModelLoadCell* mod = (Stg::ModelLoadCell*)this->mod;
  float voltage = mod->GetVoltage();

  player_aio_data data;
  data.voltages_count = 1; // Just one value
  data.voltages = &voltage; // Might look bad but player does a deep copy so it should be ok.

  // Write data
  this->driver->Publish(this->addr,
  			PLAYER_MSGTYPE_DATA,
  			PLAYER_AIO_DATA_STATE,
  			(void*)&data, sizeof(data), NULL);
}

int InterfaceAio::ProcessMessage(QueuePointer & resp_queue,
				   player_msghdr_t* hdr,
				   void* data)
{
  Stg::ModelLoadCell* mod = (Stg::ModelLoadCell*)this->mod;

  // Is it a request to set the voltage?
  if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ,
                           PLAYER_AIO_CMD_STATE,
                           this->addr))
  {
	player_aio_cmd* cmd = (player_aio_cmd*)data;
	PRINT_WARN2( "Got request to set AIO voltage %d to %f. Should not happen, AIO currently only works for load cells!\n", cmd->id, cmd->voltage );
  }
}
