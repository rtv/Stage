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
 * CVS: $Id: p_map.cc,v 1.5 2005-10-16 19:06:44 rtv Exp $
 */

#include "p_driver.h"

// DOCUMENTATION ------------------------------------------------------------

/** @addtogroup player 
@par Map interface

- Data 
 - (none)
- Configs

*/

//
// MAP INTERFACE
// 

typedef struct
{
  double ppm;
  unsigned int width, height;
  int8_t* data;
} stg_map_info_t;

InterfaceMap::InterfaceMap( player_devaddr_t addr, 
			    StgDriver* driver,
			    ConfigFile* cf,
			    int section )
  : InterfaceModel( addr, driver, cf, section, NULL )
{ 
  // nothing to do...
}

// Handle map info request - adapted from Player's mapfile driver by
// Brian Gerkey
int  InterfaceMap::HandleMsgReqInfo( MessageQueue* resp_queue,
				      player_msghdr_t* hdr,
				      void* data)
{
  printf( "Stage: device \"%s\" received map info request\n", 
	  this->mod->token );
  

//   size_t reqlen =  sizeof(info.subtype); // Expected length of request
  
//   // check if the config request is valid
//   if(len != reqlen)
//     {
//       PLAYER_ERROR2("config request len is invalid (%d != %d)", len, reqlen);
//       if (device->driver->PutReply(client, PLAYER_MSGTYPE_RESP_NACK,NULL) != 0)
// 	PLAYER_ERROR("PutReply() failed");
//       return;
//     }
  
  // create and render a map for this model
  stg_geom_t geom;
  stg_model_get_geom( this->mod, &geom );
  
  // if we already have a map info for this model, destroy it
  stg_map_info_t* minfo = (stg_map_info_t*)
    stg_model_get_property_fixed( this->mod, "_map", 
				  sizeof(stg_map_info_t) );
  
  if( minfo )
    {
      if( minfo->data ) delete [] minfo->data;
      delete minfo;
    }
  
  minfo = (stg_map_info_t*)calloc(sizeof(stg_map_info_t),1);

  double *mres = (double*)
    stg_model_get_property_fixed( this->mod, "map_resolution", sizeof(double));
  assert(mres);

  minfo->ppm = 1.0/(*mres);

  printf( "minfo ppm %.3f\n", minfo->ppm);

  minfo->width = (unsigned int)( geom.size.x * minfo->ppm );
  minfo->height = (unsigned int)( geom.size.y * minfo->ppm );
  
  // create an occupancy grid of the correct size, initially full of
  // unknown cells
  //int8_t* map = NULL; 
  assert( (minfo->data = new int8_t[minfo->width*minfo->height] ));
  memset( minfo->data, 0, minfo->width * minfo->height );
  
  printf( "Stage: creating map for model \"%s\" of %d by %d cells at %.2f ppm", 
	  this->mod->token, minfo->width, minfo->height, minfo->ppm );
  fflush(stdout);

  size_t sz=0;
  stg_polygon_t* polys = (stg_polygon_t*)
    stg_model_get_property( this->mod, "polygons", &sz);
  size_t count = sz / sizeof(stg_polygon_t);
  
  if( polys && count ) // if we have something to render
    {
      
      // we'll use a temporary matrix, as it knows how to render objects
      
      stg_matrix_t* matrix = stg_matrix_create( minfo->ppm, 
						this->mod->world->matrix->width,
						this->mod->world->matrix->height );    
      if( count > 0 ) 
	{
	  // render the model into the matrix
	  stg_matrix_polygons( matrix, 
			       geom.size.x/2.0,
			       geom.size.y/2.0,
			       0,
			       polys, count, 
			       this->mod );  
	  
	  stg_bool_t* boundary = (stg_bool_t*)
	    stg_model_get_property_fixed( this->mod, 
					  "boundary", sizeof(stg_bool_t));
	  if( *boundary )    
	    stg_matrix_rectangle( matrix,
				  geom.size.x/2.0,
				  geom.size.y/2.0,
				  0,
				  geom.size.x,
				  geom.size.y,
				  this->mod );
	}
      
      // Now convert the matrix to a Player occupancy grid. if the
      // matrix contains anything in a cell, we set that map cell to
      // be occupied, else unoccupied
      
      for( unsigned int p=0; p<minfo->width; p++ )
	for( unsigned int q=0; q<minfo->height; q++ )
        {	  
	  //printf( "{%d,%d}", p,q );

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
  stg_model_set_property( this->mod, "_map", (void*)minfo, sizeof(stg_map_info_t) );
  
  // minfo data was copied, so we can safely delete the original
  free(minfo);

  // prepare the map info for the client
  player_map_info_t info;  

  // pixels per kilometer
  info.scale = 1.0 / minfo->ppm;
  
  // size in pixels
  info.width = minfo->width;
  info.height = minfo->height;
  
  // origin of map center in global coords
  stg_pose_t global;
  memcpy( &global, &geom.pose, sizeof(global)); 
  stg_model_local_to_global( this->mod, &global );
 
  info.origin.px = geom.pose.x;
  info.origin.py = geom.pose.y;
  info.origin.pa = geom.pose.a;
     
  this->driver->Publish(this->addr, resp_queue,
			PLAYER_MSGTYPE_RESP_ACK, 
			PLAYER_MAP_REQ_GET_INFO,
			(void*)&info, sizeof(info), NULL);
  return 0;
}

// Handle map data request
int InterfaceMap::HandleMsgReqData( MessageQueue* resp_queue,
				     player_msghdr_t* hdr,
				     void* data)
{
  printf( "device %s received map data request\n", this->mod->token );

  
  stg_map_info_t* minfo = (stg_map_info_t*)
    stg_model_get_property_fixed( this->mod, "_map", sizeof(stg_map_info_t));

  assert( minfo );

  int8_t* map = NULL;  
  assert( (map = minfo->data ) );

  // request packet
  player_map_data_t* mapreq = (player_map_data_t*)data;
  

  size_t mapsize = (sizeof(player_map_data_t) - PLAYER_MAP_MAX_TILE_SIZE + 
		    (mapreq->width * mapreq->height));

  // response packet
  player_map_data_t* mapresp = (player_map_data_t*)calloc(1,mapsize);
  assert(mapresp);
  
  unsigned int oi, oj, si, sj;
  oi = mapresp->col = mapreq->col;
  oj = mapresp->row = mapreq->row;
  si = mapresp->width = mapreq->width;
  sj = mapresp->height =  mapreq->height;
  
  printf( "Stage map driver fetching tile from (%d,%d) of size (%d,%d) from map of (%d,%d) pixels.\n",
	  oi, oj, si, sj, minfo->width, minfo->height  );
    
   // Grab the pixels from the map
   for( unsigned int j = 0; j < sj; j++)
     {       
       for( unsigned int i = 0; i < si; i++)
	 {
	   if((i * j) <= PLAYER_MAP_MAX_TILE_SIZE)
	     {
	       if( i+oi >= 0 &&
		   i+oi < minfo->width &&
		   j+oj >= 0 &&
		   j+oj < minfo->height )
		 mapresp->data[i + j * si] = map[ i+oi + (minfo->width * (j+oj)) ];
	       else
		 {
		   PLAYER_WARN2("requested cell (%d,%d) is offmap", i+oi, j+oj);
		   mapresp->data[i + j * si] = 0;
		 }
	     }
	   else
	     {
	       PLAYER_WARN("requested tile is too large; truncating");
	       if(i == 0)
		 {
		   mapresp->width = si-1;
		   mapresp->height = j-1;
		 }
	       else
		 {
		   mapresp->width = i;
		   mapresp->height = j;
		 }
	     }
	 }
     }
         
   // recompute size, in case the tile got truncated
   mapsize = (sizeof(player_map_data_t) - PLAYER_MAP_MAX_TILE_SIZE + 
	      (mapresp->width * mapresp->height));
   mapresp->data_count = mapresp->width * mapresp->height;
   
   printf( "Stage publishing map data %d bytes\n",
	   (int)mapsize );

   this->driver->Publish(this->addr, resp_queue,
			 PLAYER_MSGTYPE_RESP_ACK,
			 PLAYER_MAP_REQ_GET_DATA,
			 (void*)mapresp, mapsize, NULL);
   free(mapresp);   
   return(0);
}

int InterfaceMap::ProcessMessage(MessageQueue* resp_queue,
				 player_msghdr_t* hdr,
				 void* data)
{
  printf( "Stage map interface processing message\n" );

  if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ, 
                           PLAYER_MAP_REQ_GET_DATA, 
                           this->addr))
    return( this->HandleMsgReqData( resp_queue, hdr, data ));
  
  if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ, 
			   PLAYER_MAP_REQ_GET_INFO, 
			   this->addr))
    return( this->HandleMsgReqInfo( resp_queue, hdr, data ));
  
  PLAYER_WARN2("stage map doesn't support message %d:%d.", 
	       hdr->type, hdr->subtype );
  return -1;
}


  // // print an activity spinner - now the tile sizes are so big we
  // // don't really need this.

//   const char* spin = "|/-\\";
//   static int spini = 0;

//   if( spini == 0 )
//     printf( "Stage: downloading map... " );
  
//   putchar( 8 ); // backspace
  
//   if( oi+si == minfo->width && oj+sj == minfo->height )
//     {
//       puts( " done." );
//       spini = 0;
//     }
//   else
//     {
//       putchar( spin[spini++%4] );
//       fflush(stdout);
//     }

//   fflush(stdout);  // // print an activity spinner - now the tile sizes are so big we
  // // don't really need this.

//   const char* spin = "|/-\\";
//   static int spini = 0;

//   if( spini == 0 )
//     printf( "Stage: downloading map... " );
  
//   putchar( 8 ); // backspace
  
//   if( oi+si == minfo->width && oj+sj == minfo->height )
//     {
//       puts( " done." );
//       spini = 0;
//     }
//   else
//     {
//       putchar( spin[spini++%4] );
//       fflush(stdout);
//     }

//   fflush(stdout);
