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
 * CVS: $Id: p_laser.cc,v 1.7 2005-08-25 18:11:45 gerkey Exp $
 */

// DOCUMENTATION ------------------------------------------------------------

/** @addtogroup player 
@par Laser interface

- Data
- Configs
 - PLAYER_LASER_SET_CONFIG
 - PLAYER_LASER_SET_CONFIG
 - PLAYER_LASER_GET_GEOM
*/

// CODE ----------------------------------------------------------------------

#include "p_driver.h" 

extern "C" { 
int laser_init( stg_model_t* mod );
}


InterfaceLaser::InterfaceLaser( player_devaddr_t addr, 
				StgDriver* driver,
				ConfigFile* cf,
				int section )
  : InterfaceModel( addr, driver, cf, section, laser_init )
{
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
      pdata.min_angle = -cfg->fov/2.0;
      pdata.max_angle = +cfg->fov/2.0;
      pdata.resolution = cfg->fov / cfg->samples;
      pdata.ranges_count = pdata.intensity_count = cfg->samples;
      
      for( int i=0; i<cfg->samples; i++ )
	{
	  //printf( "range %d %d\n", i, samples[i].range);
	  
	  pdata.ranges[i] = samples[i].range;
	  pdata.intensity[i] = (uint8_t)samples[i].reflectance;
	}
      
      // Write laser data
      this->driver->Publish(this->addr, NULL,
                            PLAYER_MSGTYPE_DATA,
                            PLAYER_LASER_DATA_SCAN,
                            (void*)&pdata, sizeof(pdata), NULL);
    }

  //return 0;
}

int InterfaceLaser::ProcessMessage(MessageQueue* resp_queue,
				   player_msghdr_t* hdr,
				   void* data)
{
  // Is it a request to set the laser's config?
  if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ, 
                           PLAYER_LASER_REQ_SET_CONFIG, 
                           this->addr))
  {

    player_laser_config_t* plc = (player_laser_config_t*)data;

    if( hdr->size == sizeof(player_laser_config_t) )
    {
      // TODO
      // int intensity = plc->intensity;

      PRINT_DEBUG3( "requested laser config:\n %f %f %f",
		    RTOD(plc->min_angle), RTOD(plc->max_angle), 
		    plc->resolution/1e2);

      stg_laser_config_t *current = (stg_laser_config_t*)
	      stg_model_get_property_fixed( this->mod, "laser_cfg", 
					    sizeof(stg_laser_config_t));
      assert( current );

      stg_laser_config_t slc;
      // copy the existing config
      memcpy( &slc, current, sizeof(slc));

      // tweak the parts that player knows about
      slc.fov = plc->max_angle - plc->min_angle;
      slc.samples = (int)(slc.fov / DTOR(plc->resolution/1e2));

      PRINT_DEBUG2( "setting laser config: fov %.2f samples %d", 
		    slc.fov, slc.samples );

      stg_model_set_property( this->mod, "laser_cfg", 
			      &slc, sizeof(slc)); 

      this->driver->Publish(this->addr, resp_queue,
			    PLAYER_MSGTYPE_RESP_ACK, 
			    PLAYER_LASER_REQ_SET_CONFIG);
      return(0);
    }
    else
    {
      PRINT_ERR2("config request len is invalid (%d != %d)", 
		 (int)hdr->size, (int)sizeof(player_laser_config_t));

      return(-1);
    }
  }
  // Is it a request to get the laser's config?
  else if (Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ,
                                 PLAYER_LASER_REQ_GET_CONFIG,
                                 this->addr))
  {   
    if( hdr->size == 0 )
    {
      stg_laser_config_t *slc = (stg_laser_config_t*) 
	      stg_model_get_property_fixed( this->mod, "laser_cfg", 
					    sizeof(stg_laser_config_t));
      assert(slc);

      uint8_t angular_resolution = (uint8_t)(RTOD(slc->fov / (slc->samples-1)) * 100);
      int intensity = 1; // todo

      //printf( "laser config:\n %d %d %d %d\n",
      //	    min_angle, max_angle, angular_resolution, intensity );

      player_laser_config_t plc;
      memset(&plc,0,sizeof(plc));
      plc.min_angle = -slc->fov/2.0;
      plc.max_angle = +slc->fov/2.0;
      plc.resolution = angular_resolution;
      plc.intensity = intensity;

      this->driver->Publish(this->addr, resp_queue,
			    PLAYER_MSGTYPE_RESP_ACK, 
			    PLAYER_LASER_REQ_GET_CONFIG,
			    (void*)&plc, sizeof(plc), NULL);
      return(0);
    }
    else
    {
      PRINT_ERR2("config request len is invalid (%d != %d)", (int)hdr->size,0);
      return(-1);
    }
  }
  // Is it a request to get the laser's geom?
  else if (Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ,
                                 PLAYER_LASER_REQ_GET_GEOM,
                                 this->addr))
  {	
    if(hdr->size == 0)
    {
      stg_geom_t geom;
      stg_model_get_geom( this->mod, &geom );

      // BPG - I think the laser should report its pose, not its origin,
      // when asked for geometry.
      stg_pose_t* pose = 
	      (stg_pose_t*)stg_model_get_property_fixed( this->mod, "pose", 
	  						 sizeof(stg_pose_t));

      PRINT_DEBUG5( "received laser geom: %.2f %.2f %.2f -  %.2f %.2f",
  		    pose->x, 
  		    pose->y, 
  		    pose->a, 
  		    geom.size.x, 
  		    geom.size.y ); 

      // fill in the geometry data formatted player-like
      player_laser_geom_t pgeom;
      pgeom.pose.px = pose->x;
      pgeom.pose.py = pose->y;
      pgeom.pose.pa = pose->a;

      pgeom.size.sl = geom.size.x;
      pgeom.size.sw = geom.size.y;

      this->driver->Publish(this->addr, resp_queue,
			    PLAYER_MSGTYPE_RESP_ACK, 
			    PLAYER_LASER_REQ_GET_GEOM,
			    (void*)&pgeom, sizeof(pgeom), NULL);
      return(0);
    }
    else
    {
      PRINT_ERR2("config request len is invalid (%d != %d)", (int)hdr->size,0);
      return(-1);
    }
  }
  else
  {
    // Don't know how to handle this message.
    PRINT_WARN2( "stg_laser doesn't support laser msg with type/subtype %d/%d",
		 hdr->type, hdr->subtype);
    return(-1);
  }
}


