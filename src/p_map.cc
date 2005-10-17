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
 * CVS: $Id: p_map.cc,v 1.7 2005-10-17 04:58:17 rtv Exp $
 */

#include "p_driver.h"

#include <iostream>
using namespace std;

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
  
  // create and render a map for this model
  stg_geom_t geom;
  stg_model_get_geom( this->mod, &geom );
  
  // if we already have a map for this model, destroy it
  stg_matrix_t* old_matrix = (stg_matrix_t*)
    stg_model_get_property_fixed( this->mod, "map_matrix", 
				  sizeof(stg_matrix_t) );  
  if( old_matrix )
    stg_matrix_destroy( old_matrix );

  double *mres = (double*)
    stg_model_get_property_fixed( this->mod, "map_resolution", sizeof(double));
  assert(mres);

  size_t sz=0;
  stg_polygon_t* polys = (stg_polygon_t*)
    stg_model_get_property( this->mod, "polygons", &sz);
  size_t count = sz / sizeof(stg_polygon_t);
  
  stg_matrix_t* matrix = stg_matrix_create( 1.0/(*mres), 
					    geom.size.x,
					    geom.size.y );
  
  printf( "Stage: creating map for model \"%s\" of %f by %f cells at %.2f ppm\n", 
	  this->mod->token, 
	  matrix->width * matrix->ppm, 
	  matrix->height * matrix->ppm, 
	  matrix->ppm );
  
  if( polys && count ) // if we have something to render
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
      
      // add this matrix as a property
      stg_model_set_property( this->mod, "map_matrix", 
			      (void*) matrix, sizeof(stg_matrix_t));
      
      // TODO: fix leak of  matrix structure. no big deal.
      // we're done with the matrix
      //stg_matrix_destroy( matrix );
    }

  puts( "done." );
  
  // prepare the map info for the client
  player_map_info_t info;  

  // pixels per kilometer
  info.scale = 1.0 / matrix->ppm;
  
  // size in pixels
  //info.width = (uint32_t)(matrix->width * matrix->ppm);
  //info.height = (uint32_t)(matrix->height * matrix->ppm);
  info.width = (uint32_t)(geom.size.x * matrix->ppm);
  info.height = (uint32_t)(geom.size.y * matrix->ppm);
  
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


void render_line( int8_t* buf, 
		  int w, int h, 
		  int x1, int y1, 
		  int x2, int y2, 
		  int8_t val )
{
  //printf( "render line: from %d,%d to %d,%d in image size %d,%d\n",
  //  x1, y1, x2, y2, w, h );

  double length = hypot( x2-x1, y2-y1 );
  double angle = atan2( y2-y1, x2-x1 );
  double cosa = cos(angle);
  double sina = sin(angle);
  
  int p,q;
  for( double i=0; i<=length; i++ )
    {
      p = x1 + (int)(i*cosa);
      q = y1 + (int)(i*sina);
      
      //printf( "(%d,%d)", p,q );

      if( p >= 0 && p < w && q >=0 && q < h )
	buf[ q * w + p ] = val;      
    }
  //puts("");
}


// Handle map data request
int InterfaceMap::HandleMsgReqData( MessageQueue* resp_queue,
				     player_msghdr_t* hdr,
				     void* data)
{
  // printf( "device %s received map data request\n", this->mod->token );
  
  stg_geom_t geom;
  stg_model_get_geom( this->mod, &geom );
  
  double mres = *(double*)
    stg_model_get_property_fixed( this->mod, 
				  "map_resolution", sizeof(double));
  
  // request packet
  player_map_data_t* mapreq = (player_map_data_t*)data;  
  size_t mapsize = sizeof(player_map_data_t);

  // response packet
  player_map_data_t* mapresp = (player_map_data_t*)calloc(1,mapsize);
  assert(mapresp);
  
  unsigned int oi, oj, si, sj;
  oi = mapresp->col = mapreq->col;
  oj = mapresp->row = mapreq->row;
  si = mapresp->width = mapreq->width;
  sj = mapresp->height =  mapreq->height;
  
  // initiall all cells are 'empty'
  memset( mapresp->data, -1, sizeof(uint8_t) * PLAYER_MAP_MAX_TILE_SIZE );

  // printf( "Stage map driver fetching tile from (%d,%d) of size
  //(%d,%d) from map of (%d,%d) pixels.\n",
  // 	  oi, oj, si, sj, 
  // 	  (int)(geom.size.x*mres), 
  // 	  (int)(geom.size.y*mres)  );
  
  
  // render the polygons directly into the outgoing message buffer. fast! outrageous!
  size_t sz=0;
  stg_polygon_t* polys = (stg_polygon_t*)
    stg_model_get_property( this->mod, "polygons", &sz);
  size_t polycount = sz / sizeof(stg_polygon_t);
  
  for( int p=0; p<polycount; p++ )
    {       
      // draw each line in the poly
      for( int l=0; l<polys[p].points->len; l++ )
	{
	  stg_point_t* pt1 = &g_array_index( polys[p].points, stg_point_t, l );	  
	  stg_point_t* pt2 = &g_array_index( polys[p].points, stg_point_t, (l+1) % l);	   
	  
	  render_line( mapresp->data, 
		       mapresp->width, mapresp->height,
		       (int)((pt1->x+geom.size.x/2.0) / mres), 
		       (int)((pt1->y+geom.size.y/2.0) / mres),
		       (int)((pt2->x+geom.size.x/2.0) / mres), 
		       (int)((pt2->y+geom.size.y/2.0) / mres),
		       1 ); 	  	  
	}	  
    }
  
  // if the model has a boundary, that's in the map too
  stg_bool_t* boundary = (stg_bool_t*)
    stg_model_get_property_fixed( mod, "boundary", sizeof(stg_bool_t));
  
  if( boundary && *boundary )
    {      
      render_line( mapresp->data, 
		   mapresp->width, mapresp->height,
		   0,0, mapresp->width-1,0, 1 );

      render_line( mapresp->data, 
		   mapresp->width, mapresp->height,
		   mapresp->width-1,0, mapresp->width-1,mapresp->height-1, 1 );
      
      render_line( mapresp->data, 
		   mapresp->width, mapresp->height,
		   mapresp->width-1,mapresp->height-1,0, mapresp->height-1, 1 );
      
      render_line( mapresp->data, 
		   mapresp->width, mapresp->height,
		   0, mapresp->height-1, 0,0, 1 );
    }

   mapresp->data_count = mapresp->width * mapresp->height;
   
   //printf( "Stage publishing map data %d bytes\n",
   //   (int)mapsize );

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
  // printf( "Stage map interface processing message\n" );

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
