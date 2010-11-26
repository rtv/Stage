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

// DOCUMENTATION

/** @addtogroup player 
@par Ptz interface 
- PLAYER_PTZ_DATA_STATE
- PLAYER_PTZ_REQ_GEOM
*/

// CODE

#include "p_driver.h"

extern "C" { 
int ptz_init( model_t* mod );
}


InterfacePtz::InterfacePtz( player_devaddr_t addr, 
				StgDriver* driver,
				ConfigFile* cf,
				int section )
  : InterfaceModel( addr, driver, cf, section, ptz_init )
{
  // nothing to do for now
}


void InterfacePtz::Publish( void )
{
  assert( this->mod->cfg );
  assert( this->mod->cfg_len == sizeof(ptz_config_t) );
  
  ptz_config_t *scfg = (ptz_config_t*)this->mod->cfg;
  ptz_data_t *sdata = (ptz_data_t*)this->mod->data;

  player_ptz_data_t pdata;
  pdata.pan = sdata->pan;
  pdata.tilt = sdata->tilt;
  pdata.zoom = sdata->zoom;
  pdata.panspeed = scfg->speed.pan;
  pdata.tiltspeed = scfg->speed.tilt;
  //pdata.zoomspeed = scfg->zoomspeed; // Player doesn't have this field
  
  this->driver->Publish( this->addr, NULL, 
			 PLAYER_MSGTYPE_DATA,
			 PLAYER_PTZ_DATA_STATE,
			 &pdata, sizeof(pdata), NULL);
}

int InterfacePtz::ProcessMessage( MessageQueue* resp_queue,
					 player_msghdr_t* hdr,
					 void* data )
{
  assert( this->mod->cfg );
  assert( this->mod->cfg_len == sizeof(ptz_config_t) );

  //ptz_cmd_t* cmd = (ptz_cmd_t*)this->mod->cmd;

  ptz_config_t scfg;  
  memcpy( &scfg, this->mod->cfg, sizeof(scfg));

  // Is it a new motor command?
  if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_CMD, 
                           PLAYER_PTZ_CMD_STATE, 
                           this->addr))
    {
      if( hdr->size == sizeof(player_ptz_cmd_t) )
	{
	  // convert from Player to Stage format
	  player_ptz_cmd_t* pcmd = (player_ptz_cmd_t*)data;
	  
	  scfg.goal.pan = pcmd->pan;
	  scfg.goal.tilt = pcmd->tilt;
	  scfg.goal.zoom = pcmd->zoom;
	  scfg.speed.pan = pcmd->panspeed;
	  scfg.speed.tilt = pcmd->tiltspeed;
	  //scfg.zoomgoal = pcmd->zoomspeed; // not in player

	  //printf( "setting goals: p%.2f t%.2f z %.2f\n",
	  //  scfg.pangoal,
	  //  scfg.tiltgoal,
	  //  scfg.zoomgoal );		  

	  model_set_cfg( this->mod, &scfg, sizeof(scfg));
	}
      else
	PRINT_ERR2( "wrong size ptz command packet (%d/%d bytes)",
		    (int)hdr->size, (int)sizeof(player_ptz_cmd_t) );
      return(0);
    }
  // Is it a request for ptz geometry?
  else if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ, 
                                PLAYER_PTZ_REQ_GEOM, 
                                this->addr))
    {
      if(hdr->size == 0)
	{
	  Geom geom;
	  model_get_geom( this->mod,&geom );
	  
	  player_ptz_geom_t pgeom;
	  pgeom.pos.px = geom.pose.x;
	  pgeom.pos.py = geom.pose.y;
	  pgeom.pos.pz = 0;
	  pgeom.pos.proll = 0;
	  pgeom.pos.ppitch = 0;
	  pgeom.pos.pyaw   = geom.pose.a;
	  
	  pgeom.size.sl = geom.size.x; 
	  pgeom.size.sw = geom.size.y; 
	  pgeom.size.sh = geom.size.x; // same as sl.  
	  
	  this->driver->Publish( this->addr, resp_queue,
				 PLAYER_MSGTYPE_RESP_ACK, 
				 PLAYER_PTZ_REQ_GEOM,
				 (void*)&pgeom, sizeof(pgeom), NULL );
	  return(0);
	}
    }
  
  // Don't know how to handle this message.
  PRINT_WARN2( "Stage PTZ interface doesn't support msg with type/subtype %d/%d",
	       hdr->type, hdr->subtype);
  return(-1);
}
