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
 * CVS: $Id: p_blobfinder.cc,v 1.8 2006-06-14 17:51:22 gerkey Exp $
 */

// DOCUMENTATION

/** @addtogroup player 
@par Blobfinder interface
- PLAYER_BLOBFINDER_DATA_BLOBS
*/

// CODE

#include "p_driver.h"

extern "C" { 
int blobfinder_init( stg_model_t* mod );
}


InterfaceBlobfinder::InterfaceBlobfinder( player_devaddr_t addr, 
				StgDriver* driver,
				ConfigFile* cf,
				int section )
  : InterfaceModel( addr, driver, cf, section, blobfinder_init )
{
  // nothing to do for now
}


void InterfaceBlobfinder::Publish( void )
{
  size_t len=0;
  stg_blobfinder_blob_t* blobs = (stg_blobfinder_blob_t*)mod->data;
  
  size_t bcount = mod->data_len / sizeof(stg_blobfinder_blob_t);
  
  // limit the number of samples to Player's maximum
  if( bcount > PLAYER_BLOBFINDER_MAX_BLOBS )
    bcount = PLAYER_BLOBFINDER_MAX_BLOBS;      
  
  player_blobfinder_data_t bfd;
  memset( &bfd, 0, sizeof(bfd) );
  
  // get the configuration
  stg_blobfinder_config_t *cfg = (stg_blobfinder_config_t*)mod->cfg;
  assert(cfg);
  
  // and set the image width * height
  bfd.width = cfg->scan_width;
  bfd.height = cfg->scan_height;
  bfd.blobs_count = bcount;
  
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
      
      bfd.blobs[b].id     = blobs[b].channel;
      bfd.blobs[b].x      = blobs[b].xpos;
      bfd.blobs[b].y      = blobs[b].ypos;
      bfd.blobs[b].left   = blobs[b].left;
      bfd.blobs[b].right  = blobs[b].right;
      bfd.blobs[b].top    = blobs[b].top;
      bfd.blobs[b].bottom = blobs[b].bottom;
      
      bfd.blobs[b].color = blobs[b].color;
      bfd.blobs[b].area  = blobs[b].area;          
      
      bfd.blobs[b].range = blobs[b].range;          
    }
  
  // should change player interface to support variable-lenght blob data
  // size_t size = sizeof(bfd) - sizeof(bfd.blobs) + bcount * sizeof(bfd.blobs[0]);   

  this->driver->Publish( this->addr, NULL, 
			 PLAYER_MSGTYPE_DATA,
			 PLAYER_BLOBFINDER_DATA_BLOBS,
			 &bfd, sizeof(bfd), NULL);
}

int InterfaceBlobfinder::ProcessMessage( MessageQueue* resp_queue,
					 player_msghdr_t* hdr,
					 void* data )
{
  // todo: handle configuration requests
  
  //else
  {
    // Don't know how to handle this message.
    PRINT_WARN2( "stg_blobfindeer doesn't support msg with type/subtype %d/%d",
		 hdr->type, hdr->subtype);
    return(-1);
  }
}

