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
 * CVS: $Id: p_map.cc,v 1.1 2005-07-15 03:41:37 rtv Exp $
 */

#include "p_driver.h"

// DOCUMENTATION ------------------------------------------------------------

//
// MAP INTERFACE
// 

typedef struct
{
  double ppm;
  unsigned int width, height;
  int8_t* data;
} stg_map_info_t;

void MapData( Interface* device, void* data, size_t len )
{
  //PRINT_WARN( "someone subscribed to the map interface, but we don't generate any data" );
  device->driver->PutData( device->id, NULL, 0, NULL); 
}

// Handle map info request - adapted from Player's mapfile driver by
// Brian Gerkey
void  MapConfigInfo( Interface* device,  
		     void *client, 
		     void *request, 
		     size_t len)
{
  printf( "Stage: device \"%s\" received map info request\n", device->mod->token );
  
  // prepare the map info for the client
  player_map_info_t info;  
  size_t reqlen =  sizeof(info.subtype); // Expected length of request
  
  // check if the config request is valid
  if(len != reqlen)
    {
      PLAYER_ERROR2("config request len is invalid (%d != %d)", len, reqlen);
      if (device->driver->PutReply(client, PLAYER_MSGTYPE_RESP_NACK,NULL) != 0)
	PLAYER_ERROR("PutReply() failed");
      return;
    }
  
  // create and render a map for this model
  stg_geom_t geom;
  stg_model_get_geom( device->mod, &geom );
  
  // if we already have a map info for this model, destroy it
  stg_map_info_t* minfo = (stg_map_info_t*)
    stg_model_get_property_fixed( device->mod, "_map", 
				       sizeof(stg_map_info_t) );
  
  if( minfo )
    {
      if( minfo->data ) delete [] minfo->data;
      delete minfo;
    }
  
  minfo = new stg_map_info_t();
  minfo->ppm = device->mod->world->ppm;  
  minfo->width = (unsigned int)( geom.size.x * minfo->ppm );
  minfo->height = (unsigned int)( geom.size.y * minfo->ppm );
  
  // create an occupancy grid of the correct size, initially full of
  // unknown cells
  //int8_t* map = NULL; 
  assert( (minfo->data = new int8_t[minfo->width*minfo->height] ));
  memset( minfo->data, 0, minfo->width * minfo->height );
  
  printf( "Stage: creating map for model \"%s\" of %d by %d cells... ", 
	  device->mod->token, minfo->width, minfo->height );
  fflush(stdout);

  size_t sz=0;
  stg_polygon_t* polys = (stg_polygon_t*)
    stg_model_get_property( device->mod, "polygons", &sz);
  size_t count = sz / sizeof(stg_polygon_t);
  
  if( polys && count ) // if we have something to render
    {
      
      // we'll use a temporary matrix, as it knows how to render objects
      
      stg_matrix_t* matrix = stg_matrix_create( minfo->ppm, 
						device->mod->world->matrix->width,
						device->mod->world->matrix->height );    
      if( count > 0 ) 
	{
	  // render the model into the matrix
	  stg_matrix_polygons( matrix, 
			       geom.size.x/2.0,
			       geom.size.y/2.0,
			       0,
			       polys, count, 
			       device->mod );  
	  
	  stg_bool_t* boundary = (stg_bool_t*)
	    stg_model_get_property_fixed( device->mod, 
					  "boundary", sizeof(stg_bool_t));
	  if( *boundary )    
	    stg_matrix_rectangle( matrix,
				  geom.size.x/2.0,
				  geom.size.y/2.0,
				  0,
				  geom.size.x,
				  geom.size.y,
				  device->mod );
	}
      
      // Now convert the matrix to a Player occupancy grid. if the
      // matrix contains anything in a cell, we set that map cell to
      // be occupied, else unoccupied
      
      for( unsigned int p=0; p<minfo->width; p++ )
	for( unsigned int q=0; q<minfo->height; q++ )
        {	  
	  //printf( "%d,%d \n", p,q );

	  stg_cell_t* cell = 
	    stg_cell_locate( matrix->root, p/minfo->ppm, q/minfo->ppm );

	  if( cell && cell->data )
	    minfo->data[ q * minfo->width + p ] =  1;
	  else
	    minfo->data[ q * minfo->width + p ] =  0;
	}
      
      // we're done with the matrix
      stg_matrix_destroy( matrix );
    }

  puts( "done." );

  // store the map as a model property named "_map"
  stg_model_set_property( device->mod, "_map", (void*)minfo, sizeof(minfo) );

  //if(this->mapdata == NULL)
  //{
  //PLAYER_ERROR("NULL map data");
  //if(PutReply(client, PLAYER_MSGTYPE_RESP_NACK,NULL) != 0)
  //  PLAYER_ERROR("PutReply() failed");
  // return;
  
  // pixels per kilometer
  info.scale = htonl((uint32_t)rint(minfo->ppm * 1e3 ));
  
  // size in pixels
  info.width = htonl((uint32_t)minfo->width);
  info.height = htonl((uint32_t)minfo->height);
  
  // Send map info to the client
  if (device->driver->PutReply(client, PLAYER_MSGTYPE_RESP_ACK, &info, sizeof(info), NULL) != 0)
    PLAYER_ERROR("PutReply() failed");
  
  return;
}

// Handle map data request
void MapConfigData( Interface* device,  
		    void *client, 
		    void *request, 
		    size_t len)
{
  //printf( "device %s received map data request\n", device->mod->token );

  //int i, j;
  unsigned int oi, oj, si, sj;
  size_t reqlen;
  player_map_data_t data;
  
  // Expected length of request
  reqlen = sizeof(data) - sizeof(data.data);
  
  // check if the config request is valid
  if(len != reqlen)
    {
      PLAYER_ERROR2("config request len is invalid (%d != %d)", len, reqlen);
      if( device->driver->PutReply(client, PLAYER_MSGTYPE_RESP_NACK,NULL) != 0)
      PLAYER_ERROR("PutReply() failed");
      return;
    }
  
  stg_map_info_t* minfo = (stg_map_info_t*)
    stg_model_get_property_fixed( device->mod, "_map", sizeof(minfo));

  assert( minfo );

  int8_t* map = NULL;  
  assert( (map = minfo->data ) );

  // Construct reply
  memcpy(&data, request, len);
  
   oi = ntohl(data.col);
   oj = ntohl(data.row);
   si = ntohl(data.width);
   sj = ntohl(data.height);
      
  const char* spin = "|/-\\";
  static int spini = 0;

  if( spini == 0 )
    printf( "Stage: downloading map... " );
  
  putchar( 8 ); // backspace
  
  if( oi+si == minfo->width && oj+sj == minfo->height )
    {
      puts( " done." );
      spini = 0;
    }
  else
    {
      putchar( spin[spini++%4] );
      fflush(stdout);
    }

  fflush(stdout);
  
   //   // Grab the pixels from the map
   for( unsigned int j = 0; j < sj; j++)
     {
       for( unsigned int i = 0; i < si; i++)
	 {
	   if((i * j) <= PLAYER_MAP_MAX_CELLS_PER_TILE)
	     {
	       if( i+oi >= 0 &&
		   i+oi < minfo->width &&
		   j+oj >= 0 &&
		   j+oj < minfo->height )
		 data.data[i + j * si] = map[ i+oi + (minfo->width * (j+oj)) ];
	       else
		 {
		   PLAYER_WARN2("requested cell (%d,%d) is offmap", i+oi, j+oj);
		   data.data[i + j * si] = 0;
		 }
	     }
	   else
	     {
	       PLAYER_WARN("requested tile is too large; truncating");
	       if(i == 0)
		 {
		   data.width = htonl(si-1);
		   data.height = htonl(j-1);
		 }
	       else
		 {
		   data.width = htonl(i);
		   data.height = htonl(j);
		 }
	     }
	 }
     }
      
   
   //printf( "Stage driver: providing map data %d/%d %d/%d\n",
   // oi+si, minfo->width, oj+si, minfo->height );


   //fflush(stdout);
     

  // Send map info to the client
  if(device->driver->PutReply(client, PLAYER_MSGTYPE_RESP_ACK, &data, 
              sizeof(data) - sizeof(data.data) + 
              ntohl(data.width) * ntohl(data.height),NULL) != 0)
    PLAYER_ERROR("PutReply() failed");
  return;
}


void MapConfig( Interface* device, void* client, void* buffer, size_t len )
{
  switch( ((unsigned char*)buffer)[0])
    {
    case PLAYER_MAP_GET_INFO_REQ:
      MapConfigInfo( device, client, buffer, len);
      break;
    case PLAYER_MAP_GET_DATA_REQ:
      MapConfigData( device, client, buffer, len);
      break;
    default:
      PLAYER_ERROR("got unknown map config request; ignoring");
      if(device->driver->PutReply(client, PLAYER_MSGTYPE_RESP_NACK,NULL) != 0)
        PLAYER_ERROR("PutReply() failed");
      break;
    }
}

