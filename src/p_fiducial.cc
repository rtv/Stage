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
 * CVS: $Id: p_fiducial.cc,v 1.2 2005-07-23 07:20:39 rtv Exp $
 */

#include "p_driver.h"


// 
// FIDUCIAL INTERFACE
//

extern "C" { 
int fiducial_init( stg_model_t* mod );
}

InterfaceFiducial::InterfaceFiducial(  player_device_id_t id, StgDriver* driver, ConfigFile* cf, int section )
  : InterfaceModel( id, driver, cf, section, fiducial_init )
{
  this->data_len = sizeof(player_fiducial_data_t);
  this->cmd_len = 0; // no commands
}

void InterfaceFiducial::Publish( void )
{
  size_t len = 0;
  stg_fiducial_t* fids = (stg_fiducial_t*)
    stg_model_get_property( this->mod, "fiducial_data", &len );
  
  player_fiducial_data_t pdata;
  memset( &pdata, 0, sizeof(pdata) );
  
  if( len > 0 )
    {
      size_t fcount = len / sizeof(stg_fiducial_t);      
      assert( fcount > 0 );
      
      pdata.count = htons((uint16_t)fcount);
      
      //printf( "reporting %d fiducials\n",
      //      fcount );

      if( fcount > PLAYER_FIDUCIAL_MAX_SAMPLES )
	{
	  PRINT_WARN2( "A Stage model has detected more fiducials than will fit in Player's buffer (%d/%d)\n",
		      fcount, PLAYER_FIDUCIAL_MAX_SAMPLES );
	  fcount = PLAYER_FIDUCIAL_MAX_SAMPLES;
	}

      for( int i=0; i<(int)fcount; i++ )
	{
	  pdata.fiducials[i].id = htons((int16_t)fids[i].id);
	  
	  // 2D x,y only
	  
	  double xpos = fids[i].range * cos(fids[i].bearing);
	  double ypos = fids[i].range * sin(fids[i].bearing);
	  
	  pdata.fiducials[i].pos[0] = htonl((int32_t)(xpos*1000.0));
	  pdata.fiducials[i].pos[1] = htonl((int32_t)(ypos*1000.0));
	  
	  // yaw only
	  pdata.fiducials[i].rot[2] = htonl((int32_t)(fids[i].geom.a*1000.0));	      	  
	  // player can't handle per-fiducial size.
	  // we leave uncertainty (upose) at zero
	}
    }
  
  // publish this data
  this->driver->PutData( this->id, &pdata, sizeof(pdata), NULL);
}

void InterfaceFiducial::Configure( void* client, void* src, size_t len  )
{
  //printf("got fiducial request\n");

  // switch on the config type (first byte)
  uint8_t* buf = (uint8_t*)src;
  switch( buf[0] )
    {  
    case PLAYER_FIDUCIAL_GET_GEOM:
      {	
	// just get the model's geom - Stage doesn't have separate
	// fiducial geom (yet)
	stg_geom_t geom;
	stg_model_get_geom(this->mod,&geom);
	
	// fill in the geometry data formatted player-like
	player_fiducial_geom_t pgeom;
	pgeom.pose[0] = htons((uint16_t)(1000.0 * geom.pose.x));
	pgeom.pose[1] = htons((uint16_t)(1000.0 * geom.pose.y));
	pgeom.pose[2] = htons((uint16_t)RTOD( geom.pose.a));
	
	pgeom.size[0] = htons((uint16_t)(1000.0 * geom.size.x)); 
	pgeom.size[1] = htons((uint16_t)(1000.0 * geom.size.y)); 
	
	pgeom.fiducial_size[0] = ntohs((uint16_t)100); // TODO - get this info
	pgeom.fiducial_size[1] = ntohs((uint16_t)100);
	
	if( this->driver->PutReply(  this->id, client, PLAYER_MSGTYPE_RESP_ACK,  
				       &pgeom, sizeof(pgeom), NULL ) != 0 )
	  DRIVER_ERROR("PutReply() failed for PLAYER_LASER_GET_GEOM");      
      }
      break;
      
    case PLAYER_FIDUCIAL_SET_FOV:
      
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
      
    case PLAYER_FIDUCIAL_SET_ID:
      
      if( len == sizeof(player_fiducial_id_t) )
	{
	  int id = ntohl(((player_fiducial_id_t*)src)->id);
	  
	  stg_model_set_property( this->mod, "fidicial_return", &id, sizeof(id));
	}
      else
	PRINT_ERR2("Incorrect packet size setting fiducial ID (%d/%d)",
		     (int)len, (int)sizeof(player_fiducial_id_t) );      
      
      // deliberate no-break - SET_ID needs the current ID as a reply

  case PLAYER_FIDUCIAL_GET_ID:
      {
	stg_fiducial_return_t* ret = (stg_fiducial_return_t*) 
	  stg_model_get_property_fixed( this->mod, 
					"fidicial_return",
					sizeof(stg_fiducial_return_t));

	// fill in the data formatted player-like
	player_fiducial_id_t pid;
	pid.id = htonl((int)*ret);
	
	if( this->driver->PutReply(  this->id, client, PLAYER_MSGTYPE_RESP_ACK, 
				       &pid, sizeof(pid), NULL ) != 0 )
	  DRIVER_ERROR("PutReply() failed for "
		       "PLAYER_FIDUCIAL_GET_ID or PLAYER_FIDUCIAL_SET_ID");      
      }
      break;
      
      
    default:
      {
	PRINT_WARN1( "Warning: stg_fiducial doesn't support config id %d\n", buf[0] );
        if ( this->driver->PutReply( this->id, client, PLAYER_MSGTYPE_RESP_NACK, NULL) != 0) 
          DRIVER_ERROR("PutReply() failed");
      }
    }  
}

