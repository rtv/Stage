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
 * CVS: $Id: p_sonar.cc,v 1.1.2.3 2007-11-29 07:59:00 rtv Exp $
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

InterfaceSonar::InterfaceSonar( player_devaddr_t id, 
				StgDriver* driver,
				ConfigFile* cf,
				int section )
  : InterfaceModel( id, driver, cf, section, "ranger" )
{
  //this->data_len = sizeof(player_sonar_data_t);
  //this->cmd_len = 0;
}

void InterfaceSonar::Publish( void )
{
  StgModelRanger* mod = (StgModelRanger*)this->mod;

  if( mod->samples == NULL )
    return;

  player_sonar_data_t sonar;
  memset( &sonar, 0, sizeof(sonar) );
  
  size_t sensor_count = mod->sensor_count;
  
  if( sensor_count > 0 )
    {      
      // limit the number of samples to Player's maximum
      //if( sensor_count > PLAYER_SONAR_MAX_SAMPLES )
      //sensor_count = PLAYER_SONAR_MAX_SAMPLES;
      
      //if( son->power_on ) // set with a sonar config
      {
	sonar.ranges_count = sensor_count;
	sonar.ranges = new float[sensor_count];

	for( unsigned int i=0; i<sensor_count; i++ )
	  sonar.ranges[i] = mod->samples[i];
      } 
    }
  
  this->driver->Publish( this->addr,
			 PLAYER_MSGTYPE_DATA,
			 PLAYER_SONAR_DATA_RANGES,
			 &sonar, sizeof(sonar), NULL); 

  if( sonar.ranges )
    delete[] sonar.ranges;
}


int InterfaceSonar::ProcessMessage( QueuePointer & resp_queue,
				     player_msghdr_t* hdr,
				     void* data )
{  

  if( Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ, 
			    PLAYER_SONAR_REQ_GET_GEOM, 
			    this->addr) )
    {
      StgModelRanger* mod = (StgModelRanger*)this->mod;

      size_t rcount = mod->sensor_count;
      
      // limit the number of samples to Player's maximum
      //if( rcount > PLAYER_SONAR_MAX_SAMPLES )
      //rcount = PLAYER_SONAR_MAX_SAMPLES;

      // convert the ranger data into Player-format sonar poses	
      player_sonar_geom_t pgeom;
      memset( &pgeom, 0, sizeof(pgeom) );
            
      pgeom.poses_count = rcount;
      pgeom.poses = new player_pose3d_t[rcount];

      for( unsigned int i=0; i<rcount; i++ )
	{
	  // fill in the geometry data formatted player-like
	  pgeom.poses[i].px = mod->sensors[i].pose.x;	  
	  pgeom.poses[i].py = mod->sensors[i].pose.y;	  
	  pgeom.poses[i].pyaw = mod->sensors[i].pose.a;	    
	}
      
      this->driver->Publish( this->addr, resp_queue, 
			     PLAYER_MSGTYPE_RESP_ACK, 
			     PLAYER_SONAR_REQ_GET_GEOM,
			     (void*)&pgeom, sizeof(pgeom), NULL );

      delete[] pgeom.poses;      
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

