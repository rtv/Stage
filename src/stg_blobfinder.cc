/*
 *  Stage-1.4 driver for Player
 *  Copyright (C) 2003  Richard Vaughan (rtv) vaughan@hrl.com 
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
 * $Id: stg_blobfinder.cc,v 1.6 2004-12-03 01:32:57 rtv Exp $
 */

#define PLAYER_ENABLE_TRACE 0
#define PLAYER_ENABLE_MSG 1

#include <stdlib.h>
#include "stg_driver.h"

// DRIVER FOR SONAR INTERFACE ///////////////////////////////////////////

class StgBlobfinder:public Stage1p4
{
public:
  StgBlobfinder( ConfigFile* cf, int section );
  
  static void PublishData( void* ptr );
  
  /// override PutConfig to Write configuration request to driver.
  virtual int PutConfig(player_device_id_t id, void *client, 
			void* src, size_t len,
			struct timeval* timestamp);
private:


};

StgBlobfinder::StgBlobfinder( ConfigFile* cf, int section ) 
  : Stage1p4( cf, section, PLAYER_BLOBFINDER_CODE, PLAYER_READ_MODE,
	      sizeof(player_blobfinder_data_t), 0, 1, 1 )
{
  //PLAYER_TRACE0( "constructing StgBlobfinder" ); 
  
  this->model->data_notify = StgBlobfinder::PublishData;
  this->model->data_notify_arg = (void*)this;
}

Driver* StgBlobfinder_Init( ConfigFile* cf, int section)
{
  return((Driver*)(new StgBlobfinder( cf, section)));
}


void StgBlobfinder_Register(DriverTable* table)
{
  table->AddDriver("stg_blobfinder",  StgBlobfinder_Init);
}

void StgBlobfinder::PublishData( void* ptr )
{
  StgBlobfinder* bf = (StgBlobfinder*)ptr;
  
  size_t datalen = 0;
  stg_blobfinder_blob_t *blobs = (stg_blobfinder_blob_t*)
    stg_model_get_data( bf->model, &datalen );

  if( blobs == NULL )
    {
      bf->PutData( NULL, 0, NULL);
    }
  else
    {
      size_t bcount = datalen / sizeof(stg_blobfinder_blob_t);
      

      // limit the number of samples to Player's maximum
      if( bcount > PLAYER_BLOBFINDER_MAX_BLOBS )
	bcount = PLAYER_BLOBFINDER_MAX_BLOBS;      
     
      player_blobfinder_data_t bfd;
      memset( &bfd, 0, sizeof(bfd) );
      
      // get the configuration
      size_t cfglen=0;
      stg_blobfinder_config_t* cfg = (stg_blobfinder_config_t*)
	stg_model_get_config( bf->model, &cfglen );
      assert( cfglen == sizeof(stg_blobfinder_config_t) );
      
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

      if( bf->ready )
	{
	  size_t size = sizeof(bfd) - sizeof(bfd.blobs) + bcount * sizeof(bfd.blobs[0]);
	  
	  //PRINT_WARN1( "blobfinder putting %d bytes of data", size );
	  bf->PutData( &bfd, size, NULL); // time gets filled in
	}
    }
}


int StgBlobfinder::PutConfig(player_device_id_t id, void *client, 
			     void* src, size_t len,
			     struct timeval* timestamp )
{
  // switch on the config type (first byte)
  uint8_t* buf = (uint8_t*)src;
  switch( buf[0] )
    {  
    default:
      {
	printf( "Warning: stg_blobfinder doesn't support config id %d\n", buf[0] );
	if (PutReply(client, PLAYER_MSGTYPE_RESP_NACK,NULL) != 0)
	  DRIVER_ERROR( "PutReply() failed" );
	break;
      }
      
    }
  
  return(0);
}
