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
@par Localize interface
- PLAYER_LOCALIZE_DATA_HYPOTHS
*/

// CODE ----------------------------------------------------------------------

#include "p_driver.h" 


InterfaceLocalize::InterfaceLocalize( player_devaddr_t addr, 
				StgDriver* driver,
				ConfigFile* cf,
				int section )
  : InterfaceModel( addr, driver, cf, section, NULL )
{ 
}

void InterfaceLocalize::Publish( void )
{
  player_localize_data_t loc;
  memset( &loc, 0, sizeof(loc));

  Pose pose;
  model_get_pose( this->mod, &pose );

  // only 1 hypoth - it's the truth!
  loc.hypoths_count = 1;
  loc.hypoths[0].mean.px = pose.x;
  loc.hypoths[0].mean.py = pose.y;
  loc.hypoths[0].mean.pa = pose.a;

  // Write localize data
  this->driver->Publish(this->addr, NULL,
			PLAYER_MSGTYPE_DATA,
			PLAYER_LOCALIZE_DATA_HYPOTHS,
			(void*)&loc, sizeof(loc), NULL);
}


int InterfaceLocalize::ProcessMessage(MessageQueue* resp_queue,
				      player_msghdr_t* hdr,
				      void* data)
{
  // Don't know how to handle this message.
  PRINT_WARN2( "stage localize doesn't support message %d:%d.",
	       hdr->type, hdr->subtype);
  return(-1);
}


