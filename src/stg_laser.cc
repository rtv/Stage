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
 * $Id: stg_laser.cc,v 1.7 2004-12-03 01:32:57 rtv Exp $
 */

#define PLAYER_ENABLE_MSG 1

#include <stdlib.h>

//#define DEBUG

#include "stg_driver.h"

// DRIVER FOR LASER INTERACE ////////////////////////////////////////////////////////

class StgLaser:public Stage1p4
{
public:
  StgLaser( ConfigFile* cf, int section );

  virtual int Setup();
  virtual int Shutdown(); 

  static void PublishData( void* ptr );


  /// override PutConfig to Write configuration request to driver.
  virtual int PutConfig(player_device_id_t id, void *client, 
			void* src, size_t len,
			struct timeval* timestamp);
};


StgLaser::StgLaser( ConfigFile* cf, int section ) 
  : Stage1p4( cf, section, PLAYER_LASER_CODE, PLAYER_READ_MODE,
	      sizeof(player_laser_data_t), 0, 1, 1 )
{
  PRINT_DEBUG( "created Stage laser device" );
  
  this->model->data_notify = StgLaser::PublishData;
  this->model->data_notify_arg = this;
}

Driver* StgLaser_Init( ConfigFile* cf, int section)
{
  return((Driver*)(new StgLaser( cf, section)));
}


void StgLaser_Register(DriverTable* table)
{
  table->AddDriver("stg_laser",  StgLaser_Init);
}


int StgLaser::Setup()
{
  // subscribe to laser config as well as our regular data
  //stg_model_subscribe( this->model, STG_PROP_CONFIG, 100 ); // 100ms    
  
  return Stage1p4::Setup();
}

int StgLaser::Shutdown()
{
  // unsubscribe to laser config
  //stg_model_unsubscribe( this->model, STG_PROP_CONFIG );  
  
  return Stage1p4::Shutdown();
}

// this is called in the simulation thread to stick our data into
// Player
void StgLaser::PublishData( void* ptr )
{
  //puts( "publishing laser data" );

  StgLaser* laser = (StgLaser*)ptr;
  
  size_t dlen=0;
  stg_laser_sample_t* samples = 
    (stg_laser_sample_t*)stg_model_get_data( laser->model, &dlen );
  
  int sample_count = dlen / sizeof( stg_laser_sample_t );

  //for( int i=0; i<sample_count; i++ )
  //  printf( "rrrange %d %d\n", i, samples[i].range);

  player_laser_data_t pdata;
  memset( &pdata, 0, sizeof(pdata) );

  size_t clen=0;
  stg_laser_config_t* cfg = 
    (stg_laser_config_t*)stg_model_get_config( laser->model, &clen );
  
  assert(clen == sizeof(stg_laser_config_t));
  
  if( sample_count != cfg->samples )
    {
      //PRINT_ERR2( "bad laser data: got %d/%d samples",
      //	  sample_count, cfg->samples );
    }
  else
    {      
      double min_angle = -RTOD(cfg->fov/2.0);
      double max_angle = +RTOD(cfg->fov/2.0);
      double angular_resolution = RTOD(cfg->fov / cfg->samples);
      double range_multiplier = 1; // TODO - support multipliers
      
      pdata.min_angle = htons( (int16_t)(min_angle * 100 )); // degrees / 100
      pdata.max_angle = htons( (int16_t)(max_angle * 100 )); // degrees / 100
      pdata.resolution = htons( (uint16_t)(angular_resolution * 100)); // degrees / 100
      pdata.range_res = htons( (uint16_t)range_multiplier);
      pdata.range_count = htons( (uint16_t)cfg->samples );
      
      for( int i=0; i<cfg->samples; i++ )
	{
	  //printf( "range %d %d\n", i, samples[i].range);
	  
	  pdata.ranges[i] = htons( (uint16_t)(samples[i].range) );
	  pdata.intensity[i] = (uint8_t)samples[i].reflectance;
	}
      
      //PRINT_WARN1( "laser putting %d bytes of data", sizeof(pdata) );

      // publish this data
      laser->PutData( &pdata, sizeof(pdata), NULL ); // time gets filled in
    }
}

int StgLaser::PutConfig(player_device_id_t device, void* client, 
			void* data, size_t len, struct timeval* timestamp )
{
  assert( data );

  //PRINT_WARN1( "putconfig with %d bytes", (int)len );


  // switch on the config type (first byte)
  uint8_t* buf = (uint8_t*)data;
  switch( buf[0] )
    {  
     case PLAYER_LASER_SET_CONFIG:
      {
	player_laser_config_t* plc = (player_laser_config_t*)data;
	
        if( len == sizeof(player_laser_config_t) )
	  {
	    int min_a = (int16_t)ntohs(plc->min_angle);
	    int max_a = (int16_t)ntohs(plc->max_angle);
	    int ang_res = (int16_t)ntohs(plc->resolution);
	    // TODO
	    // int intensity = plc->intensity;
	    
	    //PRINT_DEBUG4( "requested laser config:\n %d %d %d %d",
	    //	  min_a, max_a, ang_res, intensity );
	    
	    min_a /= 100;
	    max_a /= 100;
	    ang_res /= 100;

	    size_t cfglen=0;
	    stg_laser_config_t* current = (stg_laser_config_t*)
	      stg_model_get_config( this->model, &cfglen );
	    assert( cfglen == sizeof(stg_laser_config_t));
	    
	    stg_laser_config_t slc;
	    // copy the existing config
	    memcpy( &slc, current, sizeof(slc));
	    
	    // tweak the parts that player knows about
	    slc.fov = DTOR(max_a - min_a);
	    slc.samples = (int)(slc.fov / DTOR(ang_res));
	    
	    PRINT_DEBUG2( "setting laser config: fov %.2f samples %d", 
			  slc.fov, slc.samples );
	    
	    stg_model_set_config( this->model, &slc, sizeof(slc));
	    
	    if(PutReply(client, PLAYER_MSGTYPE_RESP_ACK, plc, len, NULL) != 0)
	      DRIVER_ERROR("PutReply() failed for PLAYER_LASER_SET_CONFIG");
	  }
	else
	  {
	    PRINT_ERR2("config request len is invalid (%d != %d)", 
		       (int)len, (int)sizeof(player_laser_config_t));

	    if(PutReply( client, PLAYER_MSGTYPE_RESP_NACK, NULL) != 0)
	      DRIVER_ERROR("PutReply() failed for PLAYER_LASER_SET_CONFIG");
	  }
      }
      break;
      
    case PLAYER_LASER_GET_CONFIG:
      {   
	if( len == 1 )
	  {
	    size_t clen=0;
	    stg_laser_config_t* slc =
	      (stg_laser_config_t*)stg_model_get_config( this->model, &clen);
	    
	    assert( slc );
	    assert( clen == sizeof(stg_laser_config_t));

	    // integer angles in degrees/100
	    int16_t  min_angle = (int16_t)(-RTOD(slc->fov/2.0) * 100);
	    int16_t  max_angle = (int16_t)(+RTOD(slc->fov/2.0) * 100);
	    //uint16_t angular_resolution = RTOD(slc->fov / slc->samples) * 100;
	    uint16_t angular_resolution =(uint16_t)(RTOD(slc->fov / (slc->samples-1)) * 100);
	    uint16_t range_multiplier = 1; // TODO - support multipliers
	    
	    int intensity = 1; // todo
	    
	    //printf( "laser config:\n %d %d %d %d\n",
	    //	    min_angle, max_angle, angular_resolution, intensity );
	    
	    player_laser_config_t plc;
	    memset(&plc,0,sizeof(plc));
	    plc.min_angle = htons(min_angle); 
	    plc.max_angle = htons(max_angle); 
	    plc.resolution = htons(angular_resolution);
	    plc.range_res = htons(range_multiplier);
	    plc.intensity = intensity;

	    if(PutReply(client, PLAYER_MSGTYPE_RESP_ACK, &plc, 
			sizeof(plc), NULL) != 0)
	      DRIVER_ERROR("PutReply() failed for PLAYER_LASER_GET_CONFIG");      
	  }
	else
	  {
	    PRINT_ERR2("config request len is invalid (%d != %d)", (int)len, 1);
	    if(PutReply(client, PLAYER_MSGTYPE_RESP_NACK, NULL) != 0)
	      DRIVER_ERROR("PutReply() failed for PLAYER_LASER_GET_CONFIG");
	  }
      }
      break;

    case PLAYER_LASER_GET_GEOM:
      {	
	stg_geom_t geom;
	memcpy( &geom, stg_model_get_geom( this->model ), sizeof(stg_geom_t));
	
	PRINT_DEBUG5( "received laser geom: %.2f %.2f %.2f -  %.2f %.2f",
		      geom.pose.x, 
		      geom.pose.y, 
		      geom.pose.a, 
		      geom.size.x, 
		      geom.size.y ); 
	
	// fill in the geometry data formatted player-like
	player_laser_geom_t pgeom;
	pgeom.pose[0] = htons((uint16_t)(1000.0 * geom.pose.x));
	pgeom.pose[1] = htons((uint16_t)(1000.0 * geom.pose.y));
	pgeom.pose[2] = htons((uint16_t)RTOD( geom.pose.a));
	
	pgeom.size[0] = htons((uint16_t)(1000.0 * geom.size.x)); 
	pgeom.size[1] = htons((uint16_t)(1000.0 * geom.size.y)); 
	
	if( PutReply(client, PLAYER_MSGTYPE_RESP_ACK, 
		     &pgeom, sizeof(pgeom), NULL ) != 0 )
	  DRIVER_ERROR("PutReply() failed for PLAYER_LASER_GET_GEOM");      
      }
      break;
      
    default:
      {
	PRINT_WARN1( "stg_laser doesn't support config id %d", buf[0] );
        if (PutReply(client, PLAYER_MSGTYPE_RESP_NACK, NULL) != 0)
          DRIVER_ERROR("PutReply() failed");
        break;
      }
      
    }
  
  return(0);
}

