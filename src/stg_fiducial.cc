/*
 *  Stage-1.4 driver for Player
 *  Copyright (C) 2003  Richard Vaughan (rtv) vaughan@hrl.com 
 *  for the Player/Stage Project http://playerstage.sourceforge.net
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
 * $Id: stg_fiducial.cc,v 1.2 2004-09-25 02:15:00 rtv Exp $
 */

#include <stdlib.h>
#include <math.h>

#define PLAYER_ENABLE_TRACE 0
#define PLAYER_ENABLE_MSG 1

//#include "playercommon.h"
//#include "drivertable.h"
//#include "player.h"
#include "stg_driver.h"

class StgFiducial:public Stage1p4
{
public:
  StgFiducial( ConfigFile* cf, int section );

  /// Read data from the driver; @a id specifies the interface to be read.
  virtual size_t GetData(player_device_id_t id,
			 void* dest, size_t len,
			 struct timeval* timestamp);
 
  /// override PutConfig to Write configuration request to driver.
  virtual int PutConfig(player_device_id_t id, void *client, 
			void* src, size_t len,
			struct timeval* timestamp);
};

StgFiducial::StgFiducial( ConfigFile* cf, int section ) 
  : Stage1p4(cf, section, PLAYER_FIDUCIAL_CODE, PLAYER_READ_MODE, 
	     sizeof(player_fiducial_data_t), 0, 1, 1 )
{
  PLAYER_TRACE1( "constructing StgFiducial with interface %s", interface );
}

Driver* StgFiducial_Init( ConfigFile* cf, int section)
{
    return((Driver*)(new StgFiducial( cf, section)));
}


void StgFiducial_Register(DriverTable* table)
{
  table->AddDriver("stg_fiducial",  StgFiducial_Init);
}

// override GetData to get data from Stage on demand, rather than the
// standard model of the source filling a buffer periodically
size_t StgFiducial::GetData(player_device_id_t id,
			    void* dest, size_t len,
			    struct timeval* timestamp )
{
  PLAYER_TRACE2(" STG_FIDUCIAL GETDATA section %d -> model %d",
		model->section, model->id_client );
  
  //stg_property_t* prop = stg_model_get_prop_cached( model, STG_PROP_DATA);

  player_fiducial_data_t pdata;
  memset( &pdata, 0, sizeof(pdata) );
  
  size_t datalen=0;
  stg_fiducial_t *fids = (stg_fiducial_t*)stg_model_get_data(this->model,&datalen);

  if( fids && datalen>0 )
    {
      size_t fcount = datalen / sizeof(stg_fiducial_t);      
      assert( fcount > 0 );
      
      pdata.count = htons((uint16_t)fcount);
      
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
  Driver::PutData(id,&pdata, sizeof(pdata),NULL); // time gets filled in
  
  // now inherit the standard data-getting behavior 
  return Driver::GetData(id,dest,len,timestamp);
}

int StgFiducial::PutConfig(player_device_id_t id, void *client, 
			   void* src, size_t len,
			   struct timeval* timestamp)
{
  // switch on the config type (first byte)
  uint8_t* buf = (uint8_t*)src;
  switch( buf[0] )
    {  
    case PLAYER_FIDUCIAL_GET_GEOM:
      {	
	PLAYER_TRACE0( "requesting fiducial geom" );

	// just get the model's geom - Stage doesn't have separate
	// fiducial geom (yet)
	stg_geom_t* geom = stg_model_get_geom(this->model);
	assert(geom);
	
	// fill in the geometry data formatted player-like
	player_fiducial_geom_t pgeom;
	pgeom.pose[0] = htons((uint16_t)(1000.0 * geom->pose.x));
	pgeom.pose[1] = htons((uint16_t)(1000.0 * geom->pose.y));
	pgeom.pose[2] = htons((uint16_t)RTOD( geom->pose.a));
	
	pgeom.size[0] = htons((uint16_t)(1000.0 * geom->size.x)); 
	pgeom.size[1] = htons((uint16_t)(1000.0 * geom->size.y)); 
	
	pgeom.fiducial_size[0] = ntohs((uint16_t)100); // TODO - get this info
	pgeom.fiducial_size[1] = ntohs((uint16_t)100);
	
	if( PutReply( id, client, PLAYER_MSGTYPE_RESP_ACK,  
		      &pgeom, sizeof(pgeom), NULL ) != 0 )
	  PLAYER_ERROR("PutReply() failed for PLAYER_LASER_GET_GEOM");      
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
	  
	  stg_model_set_config( this->model, &setcfg, sizeof(setcfg));
	}    
      else
	PLAYER_ERROR2("Incorrect packet size setting fiducial FOV (%d/%d)",
		      (int)len, (int)sizeof(player_fiducial_fov_t) );      
      
      // deliberate no-break - SET_FOV needs the current FOV as a reply
      
    case PLAYER_FIDUCIAL_GET_FOV:
      {
	PLAYER_TRACE0( "requesting fiducial FOV" );
	
	size_t cfglen=0;
	stg_fiducial_config_t* cfg = (stg_fiducial_config_t*)
	  stg_model_get_config( this->model, &cfglen );
	
	assert( cfglen == sizeof(stg_fiducial_config_t) );

	// fill in the geometry data formatted player-like
	player_fiducial_fov_t pfov;
	pfov.min_range = htons((uint16_t)(1000.0 * cfg->min_range));
	pfov.max_range = htons((uint16_t)(1000.0 * cfg->max_range_anon));
	pfov.view_angle = htons((uint16_t)RTOD(cfg->fov));
	
	if( PutReply( id, client, PLAYER_MSGTYPE_RESP_ACK, 
		      &pfov, sizeof(pfov), NULL ) != 0 )
	  PLAYER_ERROR("PutReply() failed for "
		       "PLAYER_FIDUCIAL_GET_FOV or PLAYER_FIDUCIAL_SET_FOV");      
      }
      break;
      
    case PLAYER_FIDUCIAL_SET_ID:
      
      if( len == sizeof(player_fiducial_id_t) )
	{
	  PLAYER_TRACE0( "setting fiducial id" );
	  
	  int id = ntohl(((player_fiducial_id_t*)src)->id);

	  stg_model_set_fiducialreturn( this->model, &id );
	}
      else
	PLAYER_ERROR2("Incorrect packet size setting fiducial ID (%d/%d)",
		      (int)len, (int)sizeof(player_fiducial_id_t) );      
      
      // deliberate no-break - SET_ID needs the current ID as a reply

  case PLAYER_FIDUCIAL_GET_ID:
      {
	PLAYER_TRACE0( "requesting fiducial ID" );
	//puts( "requesting fiducial ID" );
	
	stg_fiducial_return_t* ret = stg_model_get_fiducialreturn(this->model); 

	// fill in the data formatted player-like
	player_fiducial_id_t pid;
	pid.id = htonl((int)*ret);
	
	if( PutReply( client, PLAYER_MSGTYPE_RESP_ACK, 
		      &pid, sizeof(pid), NULL ) != 0 )
	  PLAYER_ERROR("PutReply() failed for "
		       "PLAYER_FIDUCIAL_GET_ID or PLAYER_FIDUCIAL_SET_ID");      
      }
      break;


    default:
      {
	printf( "Warning: stg_fiducial doesn't support config id %d\n", buf[0] );
        if (PutReply(id, client, PLAYER_MSGTYPE_RESP_NACK, NULL) != 0) 
          PLAYER_ERROR("PutReply() failed");
      }
    }
  
  return(0);
}


