/*
 *  Stage-1.4 driver for Player
 *  Copyright (C) 2003  Richard Vaughan (rtv) vaughan@hrl.com 
 *
 * This program is free software; you can redistribute it and/or modify
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
 * $Id: stg_sonar.cc,v 1.3 2004-09-26 02:00:45 rtv Exp $
 */

#define PLAYER_ENABLE_TRACE 0
#define PLAYER_ENABLE_MSG 1

#include <stdlib.h>
#include "stg_driver.h"

// DRIVER FOR SONAR INTERFACE ///////////////////////////////////////////

class StgSonar:public Stage1p4
{
public:
  StgSonar( ConfigFile* cf, int section );

  static void PublishData( void* ptr );

  /// override PutConfig to Write configuration request to driver.
  virtual int PutConfig(player_device_id_t id, void *client, 
			void* src, size_t len,
			struct timeval* timestamp);
private:
  // stage has no concept of sonar off/on, so we implement that here
  // to support the sonar power config.
  int power_on;
};

StgSonar::StgSonar( ConfigFile* cf, int section ) 
  : Stage1p4( cf, section, PLAYER_SONAR_CODE, PLAYER_READ_MODE, 
	      sizeof(player_sonar_data_t), 0, 1, 1 )
{
  PLAYER_TRACE1( "constructing StgSonar with interface %s", interface );
  
  power_on = 1; // enabled by default

 this->model->data_notify = StgSonar::PublishData;
 this->model->data_notify_arg = this;
}

Driver* StgSonar_Init( ConfigFile* cf, int section)
{
  return((Driver*)(new StgSonar( cf, section)));
}


void StgSonar_Register(DriverTable* table)
{
  table->AddDriver("stg_sonar",  StgSonar_Init);
}

void StgSonar::PublishData( void* ptr )
{
  //puts( "sonar publish data" );

  StgSonar* son = (StgSonar*)ptr;

  size_t datalen;
  stg_ranger_sample_t *rangers = (stg_ranger_sample_t*)
    stg_model_get_data( son->model, &datalen );
  
  if( rangers && datalen > 0 )
    {
      size_t rcount = datalen / sizeof(stg_ranger_sample_t);
      
      PLAYER_TRACE2( "i see %d bytes of ranger data: %d ranger readings", 
		     (int)prop->len, (int)rcount );
      
      // limit the number of samples to Player's maximum
      if( rcount > PLAYER_SONAR_MAX_SAMPLES )
	rcount = PLAYER_SONAR_MAX_SAMPLES;
      
      player_sonar_data_t sonar;
      memset( &sonar, 0, sizeof(sonar) );
      
      if( son->power_on ) // set with a sonar config
	{
	  sonar.range_count = htons((uint16_t)rcount);
	  
	  for( int i=0; i<(int)rcount; i++ )
	    sonar.ranges[i] = htons((uint16_t)(1000.0*rangers[i].range));
	}
      
      son->PutData( &sonar, sizeof(sonar), NULL); // time gets filled in
    }
}


int StgSonar::PutConfig(player_device_id_t id, void *client, 
			void* src, size_t len,
			struct timeval* timestamp )
{
  // switch on the config type (first byte)
  uint8_t* buf = (uint8_t*)src;
  switch( buf[0] )
    {  
    case PLAYER_SONAR_GET_GEOM_REQ:
      { 
	size_t cfglen = 0;	
	stg_ranger_config_t* cfgs = (stg_ranger_config_t*)
	  stg_model_get_config( this->model, &cfglen );
	
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

	  if(PutReply( id, client, PLAYER_MSGTYPE_RESP_ACK, 
	  &pgeom, sizeof(pgeom), NULL ) )
	  PLAYER_ERROR("failed to PutReply");
      }
      break;
      
    case PLAYER_SONAR_POWER_REQ:
      /*
	 * 1 = enable sonars
	 * 0 = disable sonar
	 */
	if( len != sizeof(player_sonar_power_config_t))
	  {
	    PLAYER_WARN2( "stg_sonar: arg to sonar state change "
			  "request wrong size (%d/%d bytes); ignoring",
			  (int)len,(int)sizeof(player_sonar_power_config_t) );
	    
	    if(PutReply( id, client, PLAYER_MSGTYPE_RESP_NACK, NULL ))
	      PLAYER_ERROR("failed to PutReply");
	  }
	
	this->power_on = ((player_sonar_power_config_t*)src)->value;
	
	if(PutReply( id, client, PLAYER_MSGTYPE_RESP_ACK, NULL ))
	  PLAYER_ERROR("failed to PutReply");
	
	break;
	
    default:
      {
	printf( "Warning: stg_sonar doesn't support config id %d\n", buf[0] );
	if (PutReply( id, client, PLAYER_MSGTYPE_RESP_NACK, NULL) != 0)
	  PLAYER_ERROR("PutReply() failed");
	break;
      }
      
    }
  
  return(0);
}
