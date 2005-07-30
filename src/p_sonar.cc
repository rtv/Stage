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
 * CVS: $Id: p_sonar.cc,v 1.3 2005-07-30 00:04:41 rtv Exp $
 */


#include "p_driver.h"

//
// SONAR INTERFACE
//
extern "C" { 
int ranger_init( stg_model_t* mod );
}

InterfaceSonar::InterfaceSonar( player_device_id_t id, 
				StgDriver* driver,
				ConfigFile* cf,
				int section )
  : InterfaceModel( id, driver, cf, section, ranger_init )
{
  this->data_len = sizeof(player_sonar_data_t);
  this->cmd_len = 0;
}

void InterfaceSonar::Publish( void )
{
  player_sonar_data_t sonar;
  memset( &sonar, 0, sizeof(sonar) );
  
  size_t len=0;
  stg_ranger_sample_t* rangers = (stg_ranger_sample_t*)
    stg_model_get_property( mod, "ranger_data", &len );

  if( len > 0 )
    {      
      size_t rcount = len / sizeof(stg_ranger_sample_t);
      
      // limit the number of samples to Player's maximum
      if( rcount > PLAYER_SONAR_MAX_SAMPLES )
	rcount = PLAYER_SONAR_MAX_SAMPLES;
      
      //if( son->power_on ) // set with a sonar config
      {
	sonar.range_count = htons((uint16_t)rcount);
	
	for( int i=0; i<(int)rcount; i++ )
	  sonar.ranges[i] = htons((uint16_t)(1000.0*rangers[i].range));
      }
    }
  
  this->driver->PutData( this->id, &sonar, sizeof(sonar), NULL); 
}


void InterfaceSonar::Configure( void* client, void* src, size_t len )
{
  //printf("got sonar request\n");
  
 // switch on the config type (first byte)
  uint8_t* buf = (uint8_t*)src;
  switch( buf[0] )
    {  
    case PLAYER_SONAR_GET_GEOM_REQ:
      { 
	size_t cfglen = 0;
	stg_ranger_config_t* cfgs = (stg_ranger_config_t*)
	  stg_model_get_property( this->mod, "ranger_cfg", &cfglen );
	assert( cfgs );
	
	size_t rcount = cfglen / sizeof(stg_ranger_config_t);
	
	// convert the ranger data into Player-format sonar poses	
	player_sonar_geom_t pgeom;
	memset( &pgeom, 0, sizeof(pgeom) );
	
	pgeom.subtype = PLAYER_SONAR_GET_GEOM_REQ;
	
	// limit the number of samples to Player's maximum
	if( rcount > PLAYER_SONAR_MAX_SAMPLES )
	  rcount = PLAYER_SONAR_MAX_SAMPLES;

	pgeom.pose_count = htons((uint16_t)rcount);

	for( int i=0; i<(int)rcount; i++ )
	  {
	    // fill in the geometry data formatted player-like
	    pgeom.poses[i][0] = 
	      ntohs((uint16_t)(1000.0 * cfgs[i].pose.x));
	    
	    pgeom.poses[i][1] = 
	      ntohs((uint16_t)(1000.0 * cfgs[i].pose.y));
	    
	    pgeom.poses[i][2] 
	      = ntohs((uint16_t)RTOD( cfgs[i].pose.a));	    
	  }

	  if(this->driver->PutReply( this->id, client, PLAYER_MSGTYPE_RESP_ACK, 
	  &pgeom, sizeof(pgeom), NULL ) )
	  DRIVER_ERROR("failed to PutReply");
      }
      break;
      
      /* DON'T SUPPORT SONAR POWER REQUEST: IT'S A STRANGE
	 PIONEER-SPECIFIC THING THAT DOESN'T BELONG HERE */
    // case PLAYER_SONAR_POWER_REQ:
//       /*
//        * 1 = enable sonars
//        * 0 = disable sonar
//        */
//       if( len != sizeof(player_sonar_power_config_t))
// 	{
// 	  PRINT_WARN2( "stg_sonar: arg to sonar state change "
// 		       "request wrong size (%d/%d bytes); ignoring",
// 		       (int)len,(int)sizeof(player_sonar_power_config_t) );
	  
// 	  if(this->driver->PutReply( this->id, client, PLAYER_MSGTYPE_RESP_NACK, NULL ))
// 	    DRIVER_ERROR("failed to PutReply");
// 	}
      
//       //int power_save =  ! ((player_sonar_power_config_t*)src)->value;
//       //stg_model_set_property( this->mod, "power_save", &power_save, sizeof(power_save));
      
//       PRINT_WARN1( "Stage is ignoring a sonar power request (received formodel %s sonar power save: %d\n", 
// 	      this->mod->token, this->mod->power_save );

//       if(this->driver->PutReply( this->id, client, PLAYER_MSGTYPE_RESP_ACK, NULL )) 
// 	DRIVER_ERROR("failed to PutReply");
	
// 	break;
	
    default:
      {
	PRINT_WARN1( "stage sonar model doesn't support config id %d\n", buf[0] );
	if (this->driver->PutReply( this->id, client, PLAYER_MSGTYPE_RESP_NACK, NULL) != 0)
	  DRIVER_ERROR("PutReply() failed");
	break;
      }      
    }
}

