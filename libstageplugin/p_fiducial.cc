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
 * CVS: $Id: p_fiducial.cc,v 1.2 2008-01-15 01:25:42 rtv Exp $
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
using namespace Stg;


InterfaceFiducial::InterfaceFiducial(  player_devaddr_t addr,
													StgDriver* driver,
													ConfigFile* cf,
													int section )
  : InterfaceModel( addr, driver, cf, section, "fiducial" )
{
}

void InterfaceFiducial::Publish( void )
{
  player_fiducial_data_t pdata;
  memset( &pdata, 0, sizeof(pdata) );
  
	std::vector<ModelFiducial::Fiducial>& fids = 
		((ModelFiducial*)mod)->GetFiducials();	
	
	pdata.fiducials_count = fids.size();

	if( pdata.fiducials_count > 0 )
    {
			pdata.fiducials = new player_fiducial_item_t[pdata.fiducials_count];
			
      for( unsigned int i=0; i<pdata.fiducials_count; i++ )
				{
					pdata.fiducials[i].id = fids[i].id;					

					// 2D x,y only
					double xpos = fids[i].range * cos(fids[i].bearing);
					double ypos = fids[i].range * sin(fids[i].bearing);
					
					pdata.fiducials[i].pose.px = xpos;
					pdata.fiducials[i].pose.py = ypos;
					pdata.fiducials[i].pose.pz = 0.0;
					pdata.fiducials[i].pose.proll = 0.0;
					pdata.fiducials[i].pose.ppitch = 0.0;
					pdata.fiducials[i].pose.pyaw = fids[i].geom.a;
				}
    }
	
  // publish this data
  this->driver->Publish( this->addr,
												 PLAYER_MSGTYPE_DATA,
												 PLAYER_FIDUCIAL_DATA_SCAN,
												 &pdata, sizeof(pdata), NULL);
	
  if ( pdata.fiducials )
		delete [] pdata.fiducials;
}

int InterfaceFiducial::ProcessMessage(QueuePointer& resp_queue,
												  player_msghdr_t* hdr,
												  void* data )
{
  //printf("got fiducial request\n");

  // Is it a request to get the geometry?
  if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ,
                           PLAYER_FIDUCIAL_REQ_GET_GEOM,
                           this->addr))
    {
      Geom geom = mod->GetGeom();
      Pose pose = mod->GetPose();

      // fill in the geometry data formatted player-like
      player_laser_geom_t pgeom;
      pgeom.pose.px = pose.x;
      pgeom.pose.py = pose.y;
      pgeom.pose.pz = pose.z;
      pgeom.pose.proll = 0.0;
      pgeom.pose.ppitch = 0.0;
      pgeom.pose.pyaw = pose.a;
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

			 mod->SetFiducialReturn( id );

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
		pid.id = mod->GetFiducialReturn();
		
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

