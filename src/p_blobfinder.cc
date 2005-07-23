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
 * CVS: $Id: p_blobfinder.cc,v 1.2 2005-07-23 07:20:39 rtv Exp $
 */


#include "p_driver.h"

extern "C" { 
int blobfinder_init( stg_model_t* mod );
}


InterfaceBlobfinder::InterfaceBlobfinder( player_device_id_t id, 
				StgDriver* driver,
				ConfigFile* cf,
				int section )
  : InterfaceModel( id, driver, cf, section, blobfinder_init )
{
  this->data_len = sizeof(player_blobfinder_data_t);
  this->cmd_len = 0;
}


void InterfaceBlobfinder::Publish( void )
{
  size_t len=0;
  stg_blobfinder_blob_t* blobs = (stg_blobfinder_blob_t*)
    stg_model_get_property( this->mod, "blob_data", &len );
  
  size_t bcount = len / sizeof(stg_blobfinder_blob_t);
  
  // limit the number of samples to Player's maximum
  if( bcount > PLAYER_BLOBFINDER_MAX_BLOBS )
    bcount = PLAYER_BLOBFINDER_MAX_BLOBS;      
  
  player_blobfinder_data_t bfd;
  memset( &bfd, 0, sizeof(bfd) );
  
  // get the configuration
  stg_blobfinder_config_t *cfg = (stg_blobfinder_config_t*)
    stg_model_get_property_fixed( this->mod, "blob_cfg", sizeof(stg_blobfinder_config_t));
  assert(cfg);
  
  // and set the image width * height
  bfd.width = htons((uint16_t)cfg->scan_width);
  bfd.height = htons((uint16_t)cfg->scan_height);
  bfd.blob_count = htons((uint16_t)bcount);
  
  // now run through the blobs, packing them into the player buffer
  // counting the number of blobs in each channel and making entries
  // in the acts header 
  unsigned int b;
  for( b=0; b<bcount; b++ )
    {
      // I'm not sure the ACTS-area is really just the area of the
      // bounding box, or if it is in fact the pixel count of the
      // actual blob. Here it's just the rectangular area.
      
      // useful debug - leave in
      /*
	cout << "blob "
	<< " channel: " <<  (int)blobs[b].channel
	<< " area: " <<  blobs[b].area
	<< " left: " <<  blobs[b].left
	<< " right: " <<  blobs[b].right
	<< " top: " <<  blobs[b].top
	<< " bottom: " <<  blobs[b].bottom
	<< endl;
      */
      
      bfd.blobs[b].x      = htons((uint16_t)blobs[b].xpos);
      bfd.blobs[b].y      = htons((uint16_t)blobs[b].ypos);
      bfd.blobs[b].left   = htons((uint16_t)blobs[b].left);
      bfd.blobs[b].right  = htons((uint16_t)blobs[b].right);
      bfd.blobs[b].top    = htons((uint16_t)blobs[b].top);
      bfd.blobs[b].bottom = htons((uint16_t)blobs[b].bottom);
      
      bfd.blobs[b].color = htonl(blobs[b].color);
      bfd.blobs[b].area  = htonl(blobs[b].area);          
    }
  
  size_t size = sizeof(bfd) - sizeof(bfd.blobs) + bcount * sizeof(bfd.blobs[0]);   
  this->driver->PutData( this->id, &bfd, size, NULL);
}

void InterfaceBlobfinder::Configure( void* client, void* buffer, size_t len )
{
  printf("got blobfinder request\n");
  
  if (this->driver->PutReply( this->id, client, PLAYER_MSGTYPE_RESP_NACK, NULL, 0, NULL) != 0)
    DRIVER_ERROR("PutReply() failed");  
}

