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
 * CVS: $Id: p_fiducial.cc,v 1.8 2006-01-22 04:16:57 rtv Exp $
 */

// DOCUMENTATION

/** 
@addtogroup player
@par Fiducial interface
- PLAYER_FIDUCIAL_DATA_SCAN
- PLAYER_FIDUCIAL_REQ_GET_GEOM
- PLAYER_FIDUCIAL_REQ_SET_ID
- PLAYER_FIDUCIAL_REQ_GET_ID
*/

/* TODO
- PLAYER_FIDUCIAL_REQ_SET_FOV
- PLAYER_FIDUCIAL_REQ_GET_FOV
*/

// CODE

#include "p_driver.h"

extern "C" { 
int fiducial_init( stg_model_t* mod );
}

InterfaceFiducial::InterfaceFiducial(  player_devaddr_t addr, 
				       StgDriver* driver, 
				       ConfigFile* cf, 
				       int section )
  : InterfaceModel( addr, driver, cf, section, fiducial_init )
{
  // nothing to do...
}

void InterfaceFiducial::Publish( void )
{
  size_t len = mod->data_len;
  stg_fiducial_t* fids = (stg_fiducial_t*)mod->data;
  
  player_fiducial_data_t pdata;
  memset( &pdata, 0, sizeof(pdata) );
  
  if( len > 0 )
    {
      size_t fcount = len / sizeof(stg_fiducial_t);      
      assert( fcount > 0 );
      
      pdata.fiducials_count = fcount;
      
      //printf( "reporting %d fiducials\n",
      //      fcount );

      if( fcount > PLAYER_FIDUCIAL_MAX_SAMPLES )
	{
	  PRINT_WARN2( "A Stage model has detected more fiducials than"
		       " will fit in Player's buffer (%d/%d)\n",
		      fcount, PLAYER_FIDUCIAL_MAX_SAMPLES );
	  fcount = PLAYER_FIDUCIAL_MAX_SAMPLES;
	}

      for( int i=0; i<(int)fcount; i++ )
	{
	  pdata.fiducials[i].id = fids[i].id;
	  
	  // 2D x,y only	  
	  double xpos = fids[i].range * cos(fids[i].bearing);
	  double ypos = fids[i].range * sin(fids[i].bearing);
	  
          /*
	  pdata.fiducials[i].pos[0] = xpos;
	  pdata.fiducials[i].pos[1] = ypos;
	  
	  // yaw only
	  pdata.fiducials[i].rot[2] = fids[i].geom.a;	      	  
	  // player can't handle per-fiducial size.
	  // we leave uncertainty (upose) at zero
          */

          // I'm guessing at this, but the above doesn't compile, because
          // there's no 'pos' or 'rot' fields - BPG
	  pdata.fiducials[i].pose.px = xpos;
	  pdata.fiducials[i].pose.py = ypos;
	  pdata.fiducials[i].pose.pz = 0.0;
	  pdata.fiducials[i].pose.proll = 0.0;
	  pdata.fiducials[i].pose.ppitch = 0.0;
	  pdata.fiducials[i].pose.pyaw = fids[i].geom.a;
	}
    }
  
  // publish this data
  this->driver->Publish( this->addr, NULL,
			 PLAYER_MSGTYPE_DATA,
			 PLAYER_FIDUCIAL_DATA_SCAN,
			 &pdata, sizeof(pdata), NULL);
}

int InterfaceFiducial::ProcessMessage(MessageQueue* resp_queue,
				      player_msghdr_t* hdr,
				      void* data )
{
  //printf("got fiducial request\n");
  
  // Is it a request to get the geometry?
  if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ, 
                           PLAYER_FIDUCIAL_REQ_GET_GEOM, 
                           this->addr))
    {
      stg_geom_t geom;
      stg_model_get_geom( this->mod, &geom );
      
      stg_pose_t pose;
      stg_model_get_pose( this->mod, &pose );
      
      // fill in the geometry data formatted player-like
      player_laser_geom_t pgeom;
      pgeom.pose.px = pose.x;
      pgeom.pose.py = pose.y;
      pgeom.pose.pa = pose.a;      
      pgeom.size.sl = geom.size.x;
      pgeom.size.sw = geom.size.y;
      
      this->driver->Publish(this->addr, resp_queue,
			    PLAYER_MSGTYPE_RESP_ACK, 
			    PLAYER_FIDUCIAL_REQ_GET_GEOM,
			    (void*)&pgeom, sizeof(pgeom), NULL);
      return(0);
    }
  else if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ, 
				PLAYER_FIDUCIAL_REQ_SET_ID, 
				this->addr))
    {  
      if( hdr->size == sizeof(player_fiducial_id_t) )
	{
	  // copy the new ID 
	  player_fiducial_id_t* incoming = (player_fiducial_id_t*)data;
	  
	  // Stage uses a simple int for IDs.
	  int id = incoming->id;
	  
	  stg_model_set_fiducial_return( this->mod, id );
	  
	  player_fiducial_id_t pid;
	  pid.id = id;

	  // acknowledge, including the new ID
	  this->driver->Publish(this->addr, resp_queue,
				PLAYER_MSGTYPE_RESP_ACK, 
				PLAYER_FIDUCIAL_REQ_SET_ID,
				(void*)&pid, sizeof(pid) );
	}
      else
	{
	  PRINT_ERR2("Incorrect packet size setting fiducial ID (%d/%d)",
		     (int)hdr->size, (int)sizeof(player_fiducial_id_t) );      
	  return(-1); // error - NACK is sent automatically
	}
    }  
  else if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ, 
				PLAYER_FIDUCIAL_REQ_GET_ID, 
				this->addr))
    {
      // fill in the data formatted player-like
      player_fiducial_id_t pid;
      pid.id = mod->fiducial_return;
      
      // acknowledge, including the new ID
      this->driver->Publish(this->addr, resp_queue,
			    PLAYER_MSGTYPE_RESP_ACK, 
			    PLAYER_FIDUCIAL_REQ_GET_ID,
			    (void*)&pid, sizeof(pid) );      
    }      
  /*    case PLAYER_FIDUCIAL_SET_FOV:
      
      if( len == sizeof(player_fiducial_fov_t) )
	{
	  player_fiducial_fov_t* pfov = (player_fiducial_fov_t*)src;
	  
	  // convert from player to stage FOV packets
	  stg_fiducial_config_t setcfg;
	  memset( &setcfg, 0, sizeof(setcfg) );
	  setcfg.min_range = (uint16_t)ntohs(pfov->min_range) / 1000.0;
	  setcfg.max_range_id = (uint16_t)ntohs(pfov->max_range) / 1000.0;
	  setcfg.max_range_anon = setcfg.max_range_id;
	  setcfg.fov = DTOR((uint16_t)ntohs(pfov->view_angle));
	  
	  //printf( "setting fiducial FOV to min %f max %f fov %f\n",
	  //  setcfg.min_range, setcfg.max_range_anon, setcfg.fov );
	  
	  //stg_model_set_config( this->mod, &setcfg, sizeof(setcfg));
	  stg_model_set_property( this->mod, "fiducial_cfg", 
				  &setcfg, sizeof(setcfg)); 
	}    
      else
	PRINT_ERR2("Incorrect packet size setting fiducial FOV (%d/%d)",
		      (int)len, (int)sizeof(player_fiducial_fov_t) );      
      
      // deliberate no-break - SET_FOV needs the current FOV as a reply
      
    case PLAYER_FIDUCIAL_GET_FOV:
      {
	stg_fiducial_config_t *cfg = (stg_fiducial_config_t*)
	  stg_model_get_property_fixed( this->mod, "fiducial_cfg", sizeof(stg_fiducial_config_t));
	assert(cfg);

	// fill in the geometry data formatted player-like
	player_fiducial_fov_t pfov;
	pfov.min_range = htons((uint16_t)(1000.0 * cfg->min_range));
	pfov.max_range = htons((uint16_t)(1000.0 * cfg->max_range_anon));
	pfov.view_angle = htons((uint16_t)RTOD(cfg->fov));
	
	if( this->driver->PutReply(  this->id, client, PLAYER_MSGTYPE_RESP_ACK, 
				       &pfov, sizeof(pfov), NULL ) != 0 )
	  DRIVER_ERROR("PutReply() failed for "
		       "PLAYER_FIDUCIAL_GET_FOV or PLAYER_FIDUCIAL_SET_FOV");      
      }
      break;
  */
 
  else
    {
      // Don't know how to handle this message.
      PRINT_WARN2( "stg_fiducial doesn't support msg with type/subtype %d/%d",
		   hdr->type, hdr->subtype);
      return(-1);
    }  

  return 0;
}

