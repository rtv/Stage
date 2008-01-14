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
 * CVS: $Id: p_laser.cc,v 1.1.2.5 2008-01-14 22:35:46 rtv Exp $
 */

// DOCUMENTATION ------------------------------------------------------------

/** @addtogroup player 
@par Laser interface
- PLAYER_LASER_DATA_SCAN
- PLAYER_LASER_REQ_SET_CONFIG
- PLAYER_LASER_REQ_GET_CONFIG
- PLAYER_LASER_REQ_GET_GEOM
*/

// CODE ----------------------------------------------------------------------

#include "p_driver.h" 

InterfaceLaser::InterfaceLaser( player_devaddr_t addr, 
				StgDriver* driver,
				ConfigFile* cf,
				int section )
  : InterfaceModel( addr, driver, cf, section, "laser" )
{ 
  this->scan_id = 0;
}

void InterfaceLaser::Publish( void )
{
  StgModelLaser* mod = (StgModelLaser*)this->mod;
  stg_laser_sample_t* samples = mod->GetSamples();

  // don't publish anything until we have some real data
  if( samples == NULL )
    return;
  
  player_laser_data_t pdata;
  memset( &pdata, 0, sizeof(pdata) );

  stg_laser_cfg_t cfg;
  mod->GetConfig( &cfg );

  pdata.min_angle = -cfg.fov/2.0;
  pdata.max_angle = +cfg.fov/2.0;
  pdata.max_range = cfg.range_bounds.max;
  pdata.resolution = cfg.fov / (double)(cfg.sample_count-1);
  pdata.ranges_count = pdata.intensity_count = cfg.sample_count;
  pdata.id = this->scan_id++;
  
  pdata.ranges = new float[pdata.ranges_count];
  pdata.intensity = new uint8_t[pdata.ranges_count];
  
  for( int i=0; i<cfg.sample_count; i++ )
    {
      //printf( "range %d %d\n", i, samples[i].range);
      pdata.ranges[i] = samples[i].range;
      pdata.intensity[i] = (uint8_t)samples[i].reflectance;
    }
  
  // Write laser data
  this->driver->Publish(this->addr,
			PLAYER_MSGTYPE_DATA,
			PLAYER_LASER_DATA_SCAN,
			(void*)&pdata, sizeof(pdata), NULL);

  delete [] pdata.ranges;
  delete [] pdata.intensity;
}

int InterfaceLaser::ProcessMessage(QueuePointer & resp_queue,
				   player_msghdr_t* hdr,
				   void* data)
{
  StgModelLaser* mod = (StgModelLaser*)this->mod;

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

      stg_laser_cfg_t cfg;
      mod->GetConfig( &cfg );

      // tweak the parts that player knows about
      cfg.fov = plc->max_angle - plc->min_angle;
      
      // todo - test this
      cfg.sample_count = (int)(cfg.fov / DTOR(plc->resolution/1e2));

      PRINT_DEBUG2( "setting laser config: fov %.2f samples %d", 
		    slc.fov, slc.samples );
      
      mod->SetConfig( &cfg );
      

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
      stg_laser_cfg_t cfg;
      mod->GetConfig( &cfg );

      player_laser_config_t plc;
      memset(&plc,0,sizeof(plc));
      plc.min_angle = -cfg.fov/2.0;
      plc.max_angle = +cfg.fov/2.0;
      plc.max_range = cfg.range_bounds.max;
      plc.resolution = (uint8_t)(RTOD(cfg.fov / (cfg.sample_count-1)) * 100);
      plc.intensity = 1; // todo

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
      stg_geom_t geom = this->mod->GetGeom();

      stg_pose_t pose = this->mod->GetPose();

      // fill in the geometry data formatted player-like
      player_laser_geom_t pgeom;
      pgeom.pose.px = pose.x;
      pgeom.pose.py = pose.y;
      pgeom.pose.pyaw = pose.a;
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

  // Don't know how to handle this message.
  PRINT_WARN2( "stage laser doesn't support message %d:%d.",
	       hdr->type, hdr->subtype);
  return(-1);
}


