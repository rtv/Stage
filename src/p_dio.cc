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
 * Author: David Olsen
 * Date: 25 October 2008
 * CVS: $Id: p_power.cc,v 1.4 2007-08-23 19:58:49 gerkey Exp $
 */


#include "p_driver.h"

/** @addtogroup player 
@par Dio interface
- PLAYER_DIO_DATA_VALUES
*/

extern "C" { 
  int indicator_init( stg_model_t* mod );
}

InterfaceDio::InterfaceDio( player_devaddr_t addr, 
				  StgDriver* driver,
				  ConfigFile* cf,
				  int section )
  : InterfaceModel( addr, driver, cf, section, indicator_init )
{ 
}


void InterfaceDio::Publish( void )
{
  size_t len = mod->data_len;
  stg_indicator_data_t* data = (stg_indicator_data_t*)mod->data;
  
  // translate stage data to player data packet
  player_dio_data_t pdio;
  pdio.count = 1;
  pdio.bits = data->on;
 
  // TODO: Publish?
  /*this->driver->Publish(this->addr,
			PLAYER_MSGTYPE_DATA,
			PLAYER_DIO_DATA_VALUES,
			(void*)&pdio, sizeof(pdio), NULL);*/
}

int InterfaceDio::ProcessMessage(QueuePointer &resp_queue,
				    player_msghdr_t* hdr,
				    void* data)
{
	if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_CMD, 
                               PLAYER_DIO_CMD_VALUES, 
	                           this->addr))
	{
		player_dio_cmd* pdio = (player_dio_cmd*)data;

		stg_dio_cmd_t cmd;
		cmd.count = pdio->count;
		cmd.digout = pdio->digout;
		stg_model_set_cmd( this->mod, &cmd, sizeof(cmd));
		return 0;
	}

  // Don't know how to handle this message.
  PRINT_WARN2( "stage dio doesn't support message %d:%d.",
	       hdr->type, hdr->subtype);
  return(-1);
}


