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
using namespace Stg;

InterfaceLocalize::InterfaceLocalize( player_devaddr_t addr, 
				StgDriver* driver,
				ConfigFile* cf,
				int section )
  : InterfaceModel( addr, driver, cf, section, "" )
{ 
  // Nothing to do here
}

void InterfaceLocalize::Publish( void )
{  
  player_localize_data_t loc;
  memset( &loc, 0, sizeof(loc));

  Pose pose = mod->GetPose();

  // only 1 hypoth - it's the truth!
  loc.hypoths_count = 1;
  loc.hypoths = new player_localize_hypoth_t[1];
  loc.hypoths[0].mean.px = pose.x;
  loc.hypoths[0].mean.py = pose.y;
  loc.hypoths[0].mean.pa = pose.a;
  
  memset(loc.hypoths[0].cov, 0, sizeof loc.hypoths[0].cov);
  loc.hypoths[0].alpha = 1e6;

  // Write localize data
  this->driver->Publish(this->addr,
			PLAYER_MSGTYPE_DATA,
			PLAYER_LOCALIZE_DATA_HYPOTHS,
			(void*)&loc, sizeof(loc), NULL);
	delete[] loc.hypoths;
}


int InterfaceLocalize::ProcessMessage(QueuePointer &resp_queue,
				      player_msghdr_t* hdr,
				      void* data)
{
	if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ, PLAYER_LOCALIZE_REQ_GET_PARTICLES, this->addr))
	{
		Pose pose = mod->GetPose();
		
    player_localize_get_particles_t resp;
    resp.mean.px = pose.x;
    resp.mean.py = pose.y;
    resp.mean.pa = pose.a;
    resp.variance = 0;

    resp.particles_count = 1;
    resp.particles = new player_localize_particle[resp.particles_count];
    
    for(uint32_t i=0;i<resp.particles_count;i++)
    {
      resp.particles[i].pose.px = pose.x;
      resp.particles[i].pose.py = pose.y;
      resp.particles[i].pose.pa = pose.a;
      resp.particles[i].alpha = 1;
    }

    this->driver->Publish(this->addr, resp_queue, PLAYER_MSGTYPE_RESP_ACK,
	            PLAYER_LOCALIZE_REQ_GET_PARTICLES, (void*)&resp, sizeof(resp), NULL);
    delete[] resp.particles;
  }
  else
  {
  	// Don't know how to handle this message.
  	PRINT_WARN2( "stage localize doesn't support message %d:%d.",
	       hdr->type, hdr->subtype);
  	return(-1);
	}
	
	return (0);
}


