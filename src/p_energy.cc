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
 * CVS: $Id: p_energy.cc,v 1.1 2005-07-15 03:41:37 rtv Exp $
 */


#include "p_driver.h"

//
// ENERGY INTERFACE
//

void EnergyConfig( Interface* device, 
		   void* client, 
		   void* src, 
		   size_t len )
{
  //printf("got energy request\n");
  
  // switch on the config type (first byte)
  uint8_t* buf = (uint8_t*)src;
  switch( buf[0] )
    {  
    default:
      {
	PRINT_WARN1( "stage energy model doesn't support config id %d\n", buf[0] );
	if( device->driver->PutReply( device->id, client, PLAYER_MSGTYPE_RESP_NACK, NULL)
	    != 0)
	  DRIVER_ERROR("PutReply() failed");
	break;
      }      
    }
}

void EnergyData( Interface* device, void* data, size_t len )
{  
  if( len == sizeof(stg_energy_data_t) )
    {
      stg_energy_data_t* edata = (stg_energy_data_t*)data;
      
      // translate stage data to player data packet
      player_energy_data_t pen;
      pen.mjoules = htonl( (int32_t)(edata->stored * 1000.0));
      pen.mwatts  = htonl( (int32_t)(edata->output_watts * 1000.0));
      pen.charging = (uint8_t)( edata->charging );      

      device->driver->PutData( device->id, &pen, sizeof(pen), NULL); 
    }
  else
    PRINT_ERR2( "energy device returned wrong size data (%d/%d)", 
		(int)len, (int)sizeof(stg_energy_data_t));
}

