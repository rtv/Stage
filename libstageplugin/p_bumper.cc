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
 * CVS: $Id: p_bumper.cc,v 1.2 2008-01-15 01:25:42 rtv Exp $
 */

// DOCUMENTATION ------------------------------------------------------------

/** @addtogroup player 
@par Bumper interface
- PLAYER_BUMPER_DATA_STATE
- PLAYER_BUMPER_GET_GEOM
*/

// CODE ----------------------------------------------------------------------

#include "p_driver.h"

//
// BUMPER INTERFACE
//
extern "C" { 
int bumper_init( stg_model_t* mod );
}

InterfaceBumper::InterfaceBumper( player_devaddr_t id, 
				StgDriver* driver,
				ConfigFile* cf,
				int section )
  : InterfaceModel( id, driver, cf, section, bumper_init )
{
}

void InterfaceBumper::Publish( void )
{
  
  size_t len = mod->data_len;
  stg_bumper_sample_t* sdata = (stg_bumper_sample_t*)mod->data;

  player_bumper_data_t pdata;
  memset( &pdata, 0, sizeof(pdata) );
  
  if( len > 0 )
    {      
      size_t bcount = len / sizeof(stg_bumper_sample_t);
      
      // limit the number of samples to Player's maximum
      if( bcount > PLAYER_BUMPER_MAX_SAMPLES )
	bcount = PLAYER_BUMPER_MAX_SAMPLES;
      
      pdata.bumpers_count = bcount;
      
      for( int i=0; i<(int)bcount; i++ )
	pdata.bumpers[i] = sdata[i].hit ? 1 : 0;
    } 
  
  this->driver->Publish( this->addr, NULL,
			 PLAYER_MSGTYPE_DATA,
			 PLAYER_BUMPER_DATA_STATE,
			 &pdata, sizeof(pdata), NULL); 
}


int InterfaceBumper::ProcessMessage( MessageQueue* resp_queue,
				     player_msghdr_t* hdr,
				     void* data )
{  
  if( Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ, 
			    PLAYER_BUMPER_GET_GEOM, 
			    this->addr) )
    {
      size_t cfglen = mod->cfg_len;
      stg_bumper_config_t* cfgs = (stg_bumper_config_t*)mod->cfg;
      assert( cfgs );
      
      size_t bcount = cfglen / sizeof(stg_bumper_config_t);
      
      // convert the bumper data into Player-format bumper poses	
      player_bumper_geom_t pgeom;
      memset( &pgeom, 0, sizeof(pgeom) );
      
      // limit the number of samples to Player's maximum
      if( bcount > PLAYER_BUMPER_MAX_SAMPLES )
	bcount = PLAYER_BUMPER_MAX_SAMPLES;
      
      pgeom.bumper_def_count = bcount;
      
      for( int i=0; i<(int)bcount; i++ )
	{
	  // fill in the geometry data formatted player-like
	  pgeom.bumper_def[i].pose.px = cfgs[i].pose.x;	  
	  pgeom.bumper_def[i].pose.py = cfgs[i].pose.y;	  
	  pgeom.bumper_def[i].pose.pa = cfgs[i].pose.a;	    
	  pgeom.bumper_def[i].length = cfgs[i].length;	    
	  pgeom.bumper_def[i].radius = 0; 
	}
      
      this->driver->Publish( this->addr, resp_queue, 
			     PLAYER_MSGTYPE_RESP_ACK, 
			     PLAYER_BUMPER_GET_GEOM,
			     (void*)&pgeom, sizeof(pgeom), NULL );
      
      return 0; // ok
    }
  else
    {
      // Don't know how to handle this message.
      PRINT_WARN2( "stg_bumper doesn't support msg with type/subtype %d/%d",
		   hdr->type, hdr->subtype);
      return(-1);
    }    
}

