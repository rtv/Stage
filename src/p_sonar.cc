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
 * CVS: $Id: p_sonar.cc,v 1.8 2006-01-22 04:16:57 rtv Exp $
 */

// DOCUMENTATION ------------------------------------------------------------

/** @addtogroup player 
@par Sonar interface
- PLAYER_SONAR_DATA_RANGES
- PLAYER_SONAR_REQ_GET_GEOM
*/

// CODE ----------------------------------------------------------------------

#include "p_driver.h"

//
// SONAR INTERFACE
//
extern "C" { 
int ranger_init( stg_model_t* mod );
}

InterfaceSonar::InterfaceSonar( player_devaddr_t id, 
				StgDriver* driver,
				ConfigFile* cf,
				int section )
  : InterfaceModel( id, driver, cf, section, ranger_init )
{
  //this->data_len = sizeof(player_sonar_data_t);
  //this->cmd_len = 0;
}

void InterfaceSonar::Publish( void )
{
  
  size_t len = mod->data_len;
  stg_ranger_sample_t* rangers = (stg_ranger_sample_t*)mod->data;

  player_sonar_data_t sonar;
  memset( &sonar, 0, sizeof(sonar) );
  
  if( len > 0 )
    {      
      size_t rcount = len / sizeof(stg_ranger_sample_t);
      
      // limit the number of samples to Player's maximum
      if( rcount > PLAYER_SONAR_MAX_SAMPLES )
	rcount = PLAYER_SONAR_MAX_SAMPLES;
      
      //if( son->power_on ) // set with a sonar config
      {
	sonar.ranges_count = rcount;
	
	for( int i=0; i<(int)rcount; i++ )
	  sonar.ranges[i] = rangers[i].range;
      } 
    }
  
  this->driver->Publish( this->addr, NULL,
			 PLAYER_MSGTYPE_DATA,
			 PLAYER_SONAR_DATA_RANGES,
			 &sonar, sizeof(sonar), NULL); 
}


int InterfaceSonar::ProcessMessage( MessageQueue* resp_queue,
				     player_msghdr_t* hdr,
				     void* data )
{  
  if( Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ, 
			    PLAYER_SONAR_REQ_GET_GEOM, 
			    this->addr) )
    {
      size_t cfglen = mod->cfg_len;
      stg_ranger_config_t* cfgs = (stg_ranger_config_t*)mod->cfg;
      assert( cfgs );
      
      size_t rcount = cfglen / sizeof(stg_ranger_config_t);
      
      // convert the ranger data into Player-format sonar poses	
      player_sonar_geom_t pgeom;
      memset( &pgeom, 0, sizeof(pgeom) );
      
      // limit the number of samples to Player's maximum
      if( rcount > PLAYER_SONAR_MAX_SAMPLES )
	rcount = PLAYER_SONAR_MAX_SAMPLES;
      
      pgeom.poses_count = rcount;
      
      for( int i=0; i<(int)rcount; i++ )
	{
	  // fill in the geometry data formatted player-like
	  pgeom.poses[i].px = cfgs[i].pose.x;	  
	  pgeom.poses[i].py = cfgs[i].pose.y;	  
	  pgeom.poses[i].pa = cfgs[i].pose.a;	    
	}
      
      this->driver->Publish( this->addr, resp_queue, 
			     PLAYER_MSGTYPE_RESP_ACK, 
			     PLAYER_SONAR_REQ_GET_GEOM,
			     (void*)&pgeom, sizeof(pgeom), NULL );
      
      return 0; // ok
    }
  else
    {
      // Don't know how to handle this message.
      PRINT_WARN2( "stg_sonar doesn't support msg with type/subtype %d/%d",
		   hdr->type, hdr->subtype);
      return(-1);
    }    
}

