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
 * CVS: $Id: p_blobfinder.cc,v 1.2 2008-01-15 01:25:42 rtv Exp $
 */

// DOCUMENTATION

/** @addtogroup player
@par Blobfinder interface
- PLAYER_BLOBFINDER_DATA_BLOBS
*/

// CODE

#include "p_driver.h"
using namespace Stg;

InterfaceBlobfinder::InterfaceBlobfinder( player_devaddr_t addr,
				StgDriver* driver,
				ConfigFile* cf,
				int section )
  : InterfaceModel( addr, driver, cf, section, "blobfinder" )
{
  // nothing to do for now
}


void InterfaceBlobfinder::Publish( void )
{
  player_blobfinder_data_t bfd;
  bzero( &bfd, sizeof(bfd) );

  ModelBlobfinder* blobmod = (ModelBlobfinder*)this->mod;
  
  uint32_t bcount = 0;
  const ModelBlobfinder::Blob* blobs = blobmod->GetBlobs( &bcount );
  
  if ( bcount > 0 )
  {
	  // and set the image width * height
	  bfd.width = blobmod->scan_width;
	  bfd.height = blobmod->scan_height;
	  bfd.blobs_count = bcount;

	  bfd.blobs = new player_blobfinder_blob_t[ bcount ];

	  // now run through the blobs, packing them into the player buffer
	  // counting the number of blobs in each channel and making entries
	  // in the acts header
	  unsigned int b;
	  for( b=0; b<bcount; b++ )
		{
		  // useful debug - leave in
		/*
		cout << "blob "
		<< " left: " <<  blobs[b].left
		<< " right: " <<  blobs[b].right
		<< " top: " <<  blobs[b].top
		<< " bottom: " <<  blobs[b].bottom
		<< " color: " << hex << blobs[b].color << dec
		<< endl;
		  */

			int dx = blobs[b].right - blobs[b].left;
			int dy = blobs[b].top - blobs[b].bottom;

		  bfd.blobs[b].x      = blobs[b].left + dx/2;
		  bfd.blobs[b].y      = blobs[b].bottom + dy/2;

		  bfd.blobs[b].left   = blobs[b].left;
		  bfd.blobs[b].right  = blobs[b].right;
		  bfd.blobs[b].top    = blobs[b].top;
		  bfd.blobs[b].bottom = blobs[b].bottom;

		  bfd.blobs[b].color = 
			 ((uint8_t)(blobs[b].color.r*255.0) << 16) +
			 ((uint8_t)(blobs[b].color.g*255.0) << 8) +
			 ((uint8_t)(blobs[b].color.b*255.0));
			 
		  bfd.blobs[b].area  = dx * dy;

		  bfd.blobs[b].range = blobs[b].range;
		}
  }

  // should change player interface to support variable-lenght blob data
  // size_t size = sizeof(bfd) - sizeof(bfd.blobs) + bcount * sizeof(bfd.blobs[0]);

  this->driver->Publish( this->addr,
								 PLAYER_MSGTYPE_DATA,
								 PLAYER_BLOBFINDER_DATA_BLOBS,
								 &bfd, sizeof(bfd), NULL);
  if ( bfd.blobs )
	  delete [] bfd.blobs;
}

int InterfaceBlobfinder::ProcessMessage( QueuePointer& resp_queue,
													  player_msghdr_t* hdr,
													  void* data )
{
  // todo: handle configuration requests

  //else
  {
    // Don't know how to handle this message.
    PRINT_WARN2( "stg_blobfinder doesn't support msg with type/subtype %d/%d",
		 hdr->type, hdr->subtype);
    return(-1);
  }
}

