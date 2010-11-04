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
@par Ranger interface
- PLAYER_RANGER_DATA_SCAN
- PLAYER_RANGER_REQ_SET_CONFIG
- PLAYER_RANGER_REQ_GET_CONFIG
- PLAYER_RANGER_REQ_GET_GEOM
*/

// CODE ----------------------------------------------------------------------

#include "p_driver.h"
using namespace Stg;

InterfaceRanger::InterfaceRanger( player_devaddr_t addr,
				StgDriver* driver,
				ConfigFile* cf,
				int section )
  : InterfaceModel( addr, driver, cf, section, "ranger" )
{
  this->scan_id = 0;
}

void InterfaceRanger::Publish( void )
{
  ModelRanger* rgr = dynamic_cast<ModelRanger*>(this->mod);
	
  const std::vector<ModelRanger::Sensor::Sample>& samples = rgr->GetSamples();
  
  uint32_t sample_count = samples.size();
  // don't publish anything until we have some real data
  if( sample_count == 0 )
    return;
	
  // Write range data
	player_ranger_data_range_t prange;
  memset( &prange, 0, sizeof(prange) );  
	prange.ranges = &samples[0];
	prange.ranges_count = samples.size();
	
  this->driver->Publish(this->addr,
												PLAYER_MSGTYPE_DATA,
												PLAYER_RANGER_DATA_RANGE,
												(void*)&prange, sizeof(prange), NULL);
	

  // Write intensity data
	player_ranger_data_intns_t pintens;
  memset( &pintensity, 0, sizeof(pintens) );	
	pntens.intensities = &intens[0];
	
	pintens.intensities_count = intens.size();
	this->driver->Publish(this->addr,
												PLAYER_MSGTYPE_DATA,
												PLAYER_RANGER_DATA_INTNS,
												(void*)&pintens, sizeof(pintens), NULL);

	// todo: shouldn't this be published?
	
  const ModelRanger::Config& cfg = rgr->cfg;
  pdata.min_angle = -cfg.fov/2.0;
  pdata.max_angle = +cfg.fov/2.0;
  pdata.resolution = cfg.fov / cfg.sample_count;
  pdata.max_range = cfg.range_bounds.max;
  pdata.ranges_count = pdata.intensity_count = cfg.sample_count;
  pdata.id = this->scan_id++;

  pdata.ranges = new float[pdata.ranges_count];
  pdata.intensity = new uint8_t[pdata.ranges_count];
}

int InterfaceRanger::ProcessMessage(QueuePointer & resp_queue,
				   player_msghdr_t* hdr,
				   void* data)
{
  ModelRanger* mod = (ModelRanger*)this->mod;

  // Is it a request to set the ranger's config?
  if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ,
                           PLAYER_RANGER_REQ_SET_CONFIG,
                           this->addr))
  {

    player_ranger_config_t* plc = (player_ranger_config_t*)data;

    if( hdr->size == sizeof(player_ranger_config_t) )
    {
      // TODO
      // int intensity = plc->intensity;

      ModelRanger::Config cfg = mod->GetConfig();

	  PRINT_DEBUG3( "ranger config was: resolution %d, fov %.6f, interval %d\n",
			  cfg.resolution, cfg.fov, cfg.interval );

	  cfg.fov = plc->max_angle - plc->min_angle;
	  cfg.resolution = (uint32_t) ( cfg.fov / ( cfg.sample_count * plc->resolution ) );
	  if ( cfg.resolution < 1 )
		  cfg.resolution = 1;
	  cfg.interval = (usec_t) ( 1.0E6 / plc->scanning_frequency );

	  PRINT_DEBUG3( "setting ranger config: resolution %d, fov %.6f, interval %d\n",
			  cfg.resolution, cfg.fov, cfg.interval );

	  // Range resolution is currently locked to the world setting
	  //  and intensity values are always read.  The relevant settings
	  //  are ignored.

      mod->SetConfig( cfg );

      this->driver->Publish(this->addr, resp_queue,
			    PLAYER_MSGTYPE_RESP_ACK,
			    PLAYER_RANGER_REQ_SET_CONFIG);
      return(0);
    }
    else
    {
      PRINT_ERR2("config request len is invalid (%d != %d)",
		 (int)hdr->size, (int)sizeof(player_ranger_config_t));

      return(-1);
    }
  }
  // Is it a request to get the ranger's config?
  else if (Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ,
                                 PLAYER_RANGER_REQ_GET_CONFIG,
                                 this->addr))
  {
    if( hdr->size == 0 )
    {
      ModelRanger::Config cfg = mod->GetConfig();

      player_ranger_config_t plc;
      memset(&plc,0,sizeof(plc));
      plc.min_angle = -cfg.fov/2.0;
      plc.max_angle = +cfg.fov/2.0;
      plc.resolution = cfg.fov / ( cfg.sample_count * cfg.resolution );
      plc.max_range = cfg.range_bounds.max;
	  plc.range_res = 1.0; // todo
      plc.intensity = 1; // todo
	  plc.scanning_frequency = 1.0E6 / cfg.interval;

      this->driver->Publish(this->addr, resp_queue,
			    PLAYER_MSGTYPE_RESP_ACK,
			    PLAYER_RANGER_REQ_GET_CONFIG,
			    (void*)&plc, sizeof(plc), NULL);
      return(0);
    }
    else
    {
      PRINT_ERR2("config request len is invalid (%d != %d)", (int)hdr->size,0);
      return(-1);
    }
  }
  // Is it a request to get the ranger's geom?
  else if (Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ,
                                 PLAYER_RANGER_REQ_GET_GEOM,
                                 this->addr))
  {
    if(hdr->size == 0)
    {
      Geom geom = this->mod->GetGeom();

      Pose pose = this->mod->GetPose();

      // fill in the geometry data formatted player-like
      player_ranger_geom_t pgeom;
      pgeom.pose.px = pose.x;
      pgeom.pose.py = pose.y;
      pgeom.pose.pyaw = pose.a;
      pgeom.size.sl = geom.size.x;
      pgeom.size.sw = geom.size.y;

      this->driver->Publish(this->addr, resp_queue,
			    PLAYER_MSGTYPE_RESP_ACK,
			    PLAYER_RANGER_REQ_GET_GEOM,
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
  PRINT_WARN2( "stage ranger doesn't support message %d:%d.",
	       hdr->type, hdr->subtype);
  return(-1);
}


