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
 * CVS: $Id: p_laser.cc,v 1.1 2005-07-15 03:41:37 rtv Exp $
 */


// 
// LASER INTERFACE
//

#include "p_driver.h"

InterfaceLaser::InterfaceLaser( player_device_id_t id, 
				StgDriver* driver,
				ConfigFile* cf,
				int section )
  : InterfaceModel( id, driver, cf, section, STG_MODEL_LASER )
{
  this->data_len = sizeof(player_laser_data_t);
  this->cmd_len = 0;
}

void InterfaceLaser::Publish( void )
{
  size_t len = 0;
  stg_laser_sample_t* samples = (stg_laser_sample_t*)
    stg_model_get_property( this->mod, "laser_data", &len );
  
  int sample_count = len / sizeof( stg_laser_sample_t );
  
  //for( int i=0; i<sample_count; i++ )
  //  printf( "rrrange %d %d\n", i, samples[i].range);
  
  player_laser_data_t pdata;
  memset( &pdata, 0, sizeof(pdata) );
  
  stg_laser_config_t *cfg = (stg_laser_config_t*) 
    stg_model_get_property_fixed( this->mod, "laser_cfg", sizeof(stg_laser_config_t));
  assert(cfg);
  
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
      
      // Write laser data
      this->driver->PutData( this->id, &pdata, sizeof(pdata), NULL);
    }

  //return 0;
}

void InterfaceLaser::Configure( void* client, void* buffer, size_t len )
{
  printf("got laser request\n");
  
  // switch on the config type (first byte)
  uint8_t* buf = (uint8_t*)buffer;
  switch( buf[0] )
    {  
     case PLAYER_LASER_SET_CONFIG:
      {
	player_laser_config_t* plc = (player_laser_config_t*)buffer;
	
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
	    
	    stg_laser_config_t *current = (stg_laser_config_t*)
	      stg_model_get_property_fixed( this->mod, "laser_cfg", 
					    sizeof(stg_laser_config_t));
	    assert( current );
	    
	    stg_laser_config_t slc;
	    // copy the existing config
	    memcpy( &slc, current, sizeof(slc));
	    
	    // tweak the parts that player knows about
	    slc.fov = DTOR(max_a - min_a);
	    slc.samples = (int)(slc.fov / DTOR(ang_res));
	    
	    PRINT_DEBUG2( "setting laser config: fov %.2f samples %d", 
			  slc.fov, slc.samples );
	    
	    stg_model_set_property( this->mod, "laser_cfg", 
				  &slc, sizeof(slc)); 
	    
	    if(this->driver->PutReply( this->id, client, PLAYER_MSGTYPE_RESP_ACK, plc, len, NULL) != 0)
	      DRIVER_ERROR("PutReply() failed for PLAYER_LASER_SET_CONFIG");
	  }
	else
	  {
	    PRINT_ERR2("config request len is invalid (%d != %d)", 
	           (int)len, (int)sizeof(player_laser_config_t));

	    if( this->driver->PutReply( this->id, client, PLAYER_MSGTYPE_RESP_NACK, NULL) != 0)
	      DRIVER_ERROR("PutReply() failed for PLAYER_LASER_SET_CONFIG");
	  }
      }
      break;
      
    case PLAYER_LASER_GET_CONFIG:
      {   
	if( len == 1 )
	  {
	    stg_laser_config_t *slc = (stg_laser_config_t*) 
	      stg_model_get_property_fixed( this->mod, "laser_cfg", 
					    sizeof(stg_laser_config_t));
	    assert(slc);

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

	    if(this->driver->PutReply( this->id, client, PLAYER_MSGTYPE_RESP_ACK, &plc, 
					 sizeof(plc), NULL) != 0)
	      DRIVER_ERROR("PutReply() failed for PLAYER_LASER_GET_CONFIG");      
	  }
	else
	  {
	    PRINT_ERR2("config request len is invalid (%d != %d)", (int)len, 1);
	    if( this->driver->PutReply( this->id, client, PLAYER_MSGTYPE_RESP_NACK, NULL) != 0)
	      DRIVER_ERROR("PutReply() failed for PLAYER_LASER_GET_CONFIG");
	  }
      }
      break;

    case PLAYER_LASER_GET_GEOM:
      {	
	stg_geom_t geom;
	stg_model_get_geom( this->mod, &geom );
	
	PRINT_DEBUG5( "received laser geom: %.2f %.2f %.2f -  %.2f %.2f",
		      geom.pose.x, 
		      geom.pose.y, 
		      geom.pose.a, 
		      geom.size.x, 
		      geom.size.y ); 
	
	// fill in the geometry data formatted player-like
	player_laser_geom_t pgeom;
        pgeom.subtype = PLAYER_LASER_GET_GEOM;
	pgeom.pose[0] = htons((uint16_t)(1000.0 * geom.pose.x));
	pgeom.pose[1] = htons((uint16_t)(1000.0 * geom.pose.y));
	pgeom.pose[2] = htons((uint16_t)RTOD( geom.pose.a));
	
	pgeom.size[0] = htons((uint16_t)(1000.0 * geom.size.x)); 
	pgeom.size[1] = htons((uint16_t)(1000.0 * geom.size.y)); 

	if( this->driver->PutReply( this->id, client, PLAYER_MSGTYPE_RESP_ACK, 
				      &pgeom, sizeof(pgeom), NULL ) != 0 )
	  DRIVER_ERROR("PutReply() failed for PLAYER_LASER_GET_GEOM");      
      }
      break;
      
    default:
      {
	PRINT_WARN1( "stg_laser doesn't support config id %d", buf[0] );
        if (this->driver->PutReply( this->id, client, PLAYER_MSGTYPE_RESP_NACK, NULL, 0, NULL) != 0)
	  DRIVER_ERROR("PutReply() failed");
        break;
      }
      
    }
}

