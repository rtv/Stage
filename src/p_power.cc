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
 * CVS: $Id: p_power.cc,v 1.1 2005-12-05 08:11:16 rtv Exp $
 */


#include "p_driver.h"

//
// POWER INTERFACE
//

extern "C" { 
  int energy_init( stg_model_t* mod );
}

InterfacePower::InterfacePower( player_devaddr_t addr, 
				  StgDriver* driver,
				  ConfigFile* cf,
				  int section )
  : InterfaceModel( addr, driver, cf, section, energy_init )
{ 
  // nothing to see here. move along.
}


void InterfacePower::Publish( void )
{
  size_t len = 0;
  stg_energy_data_t* data = (stg_energy_data_t*)
    stg_model_get_property_fixed( this->mod, "energy_data", 
				  sizeof(stg_energy_data_t) );
  
  stg_energy_config_t* cfg = (stg_energy_config_t*)
    stg_model_get_property_fixed( this->mod, "energy_config", 
				  sizeof(stg_energy_config_t) );
  
  // translate stage data to player data packet
  player_power_data_t pen;
  pen.valid = 0;

  pen.valid |= PLAYER_POWER_MASK_VOLTS;
  pen.joules = data->stored;

  pen.valid |= PLAYER_POWER_MASK_WATTS;
  pen.watts  = data->output_watts;

  pen.valid |= PLAYER_POWER_MASK_CHARGING;
  pen.charging = data->charging;

  pen.valid |= PLAYER_POWER_MASK_PERCENT;
  pen.percent = 100.0 * data->stored / cfg->capacity;
  
  printf( "player power data: valid %d joules %.2f watts %.2f percent %.2f charging %d\n",
	  pen.valid, pen.joules, pen.watts, pen.percent, pen.charging );
 
  this->driver->Publish(this->addr, NULL,
			PLAYER_MSGTYPE_DATA,
			PLAYER_POWER_DATA_STATE,
			(void*)&pen, sizeof(pen), NULL);
}

int InterfacePower::ProcessMessage(MessageQueue* resp_queue,
				    player_msghdr_t* hdr,
				    void* data)
{
  // Is it a request to set the laser's config?
  if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ, 
                           PLAYER_LASER_REQ_SET_CONFIG, 
                           this->addr))
  {

  }
 //    player_laser_config_t* plc = (player_laser_config_t*)data;

//     if( hdr->size == sizeof(player_laser_config_t) )
//     {
//       // TODO
//       // int intensity = plc->intensity;

//       PRINT_DEBUG3( "requested laser config:\n %f %f %f",
// 		    RTOD(plc->min_angle), RTOD(plc->max_angle), 
// 		    plc->resolution/1e2);

//       stg_laser_config_t *current = (stg_laser_config_t*)
// 	      stg_model_get_property_fixed( this->mod, "laser_cfg", 
// 					    sizeof(stg_laser_config_t));
//       assert( current );

//       stg_laser_config_t slc;
//       // copy the existing config
//       memcpy( &slc, current, sizeof(slc));

//       // tweak the parts that player knows about
//       slc.fov = plc->max_angle - plc->min_angle;
//       slc.samples = (int)(slc.fov / DTOR(plc->resolution/1e2));

//       PRINT_DEBUG2( "setting laser config: fov %.2f samples %d", 
// 		    slc.fov, slc.samples );

//       stg_model_set_property( this->mod, "laser_cfg", 
// 			      &slc, sizeof(slc)); 

//       this->driver->Publish(this->addr, resp_queue,
// 			    PLAYER_MSGTYPE_RESP_ACK, 
// 			    PLAYER_LASER_REQ_SET_CONFIG);
//       return(0);
//     }
//     else
//     {
//       PRINT_ERR2("config request len is invalid (%d != %d)", 
// 		 (int)hdr->size, (int)sizeof(player_laser_config_t));

//       return(-1);
//     }
//   }
//   else
//     {
//       PRINT_ERR2("config request len is invalid (%d != %d)", (int)hdr->size,0);
//       return(-1);
//     }
//   }

  // Don't know how to handle this message.
  PRINT_WARN2( "stage laser doesn't support message %d:%d.",
	       hdr->type, hdr->subtype);
  return(-1);
}


