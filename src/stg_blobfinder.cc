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
 * $Id: stg_blobfinder.cc,v 1.1 2004-09-16 06:54:28 rtv Exp $
 */

#define PLAYER_ENABLE_TRACE 0
#define PLAYER_ENABLE_MSG 1

#include "playercommon.h"
#include "drivertable.h"
#include "player.h"
#include <stdlib.h>
#include "stg_driver.h"

// DRIVER FOR SONAR INTERFACE ///////////////////////////////////////////

class StgBlobfinder:public Stage1p4
{
public:
  StgBlobfinder( ConfigFile* cf, int section );

   /// Read data from the driver; @a id specifies the interface to be read.
  virtual size_t GetData(player_device_id_t id,
			 void* dest, size_t len,
			 struct timeval* timestamp);
 
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
  PLAYER_TRACE0( "constructing StgBlobfinder" );
}

Driver* StgBlobfinder_Init( ConfigFile* cf, int section)
{
  return((Driver*)(new StgBlobfinder( cf, section)));
}


void StgBlobfinder_Register(DriverTable* table)
{
  table->AddDriver("stg_blobfinder",  StgBlobfinder_Init);
}

// override GetData to get data from Stage on demand, rather than the
// standard model of the source filling a buffer periodically
size_t StgBlobfinder::GetData(player_device_id_t id,
			      void* dest, size_t len,
			      struct timeval* timestamp )
{  
  
  stg_property_t* prop = stg_model_get_prop_cached( model, STG_PROP_DATA);

  if( prop )
    {
      stg_blobfinder_blob_t *blobs = (stg_blobfinder_blob_t*)prop->data;
      size_t bcount = prop->len / sizeof(stg_blobfinder_blob_t);
      
      // ACTS has blobs sorted by channel (color), and by area within
      // channel. Player has inherited this awful mess, so we'll
      // bubble sort the blobs (This could be more efficient. Tony
      // Hoare would spank me).
      if( bcount > 1 )
	{
	  int change = true;
	  stg_blobfinder_blob_t tmp;
	  
	  while( change )
	    {
	      change = false;
	      
	      unsigned int b;
	      for( b=0; b<bcount-1; b++ )
		{
		  // if the channels are in the wrong order
		  if( (blobs[b].channel > blobs[b+1].channel)
		      // or they are the same channel but the areas are 
		      // in the wrong order
		      ||( (blobs[b].channel == blobs[b+1].channel) 
			  && blobs[b].area < blobs[b+1].area ) )
		    {		      
		      //switch the blobs
		      memcpy( &tmp, &(blobs[b]), sizeof(stg_blobfinder_blob_t) );
		      memcpy( &(blobs[b]), &(blobs[b+1]), sizeof(stg_blobfinder_blob_t) );
		      memcpy( &(blobs[b+1]), &tmp, sizeof(stg_blobfinder_blob_t) );
		      
		      change = true;
		    }
		}
	    }
	}      
      
      // limit the number of samples to Player's maximum
      if( bcount > PLAYER_BLOBFINDER_MAX_BLOBS )
	bcount = PLAYER_BLOBFINDER_MAX_BLOBS;      
     
      player_blobfinder_data_t bfd;
      memset( &bfd, 0, sizeof(bfd) );
      
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
	  
	  // increment the count for this channel
	  bfd.header[blobs[b].channel].num++;
	}

      // now we finish the header by setting the blob indexes and byte
      // swapping the counts.
      int pos = 0;
      for( int ch=0; ch<PLAYER_BLOBFINDER_MAX_CHANNELS; ch++ )
	{
	  bfd.header[ch].index = htons(pos);
	  pos += bfd.header[ch].num;
	  
	  // byte swap the blob count for each channel
	  PRINT_DEBUG2( "channel: %d blobs: %d\n", ch,  bfd.header[ch].num ); 
	  bfd.header[ch].num = htons(bfd.header[ch].num);
	}
      
      // get the configuration
      stg_property_t* cfg_prop = 
	stg_model_get_prop_cached( model, STG_PROP_CONFIG);
      
      assert( cfg_prop );
      assert( cfg_prop->data );
      assert( cfg_prop->len == sizeof(stg_blobfinder_config_t) );
      stg_blobfinder_config_t* cfg = (stg_blobfinder_config_t*)cfg_prop->data;
      
      // and set the image width * height
      bfd.width = htons((uint16_t)cfg->scan_width);
      bfd.height = htons((uint16_t)cfg->scan_height);
      
      Driver::PutData( id, &bfd, sizeof(bfd), NULL); // time gets filled in
    }
  
  // now inherit the standard data-getting behavior 
  return Driver::GetData(id,dest,len,timestamp);
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
	if (PutReply(id,client, PLAYER_MSGTYPE_RESP_NACK,NULL) != 0)
	  PLAYER_ERROR("PutReply() failed");
	break;
      }
      
    }
  
  return(0);
}
