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
 * CVS: $Id: p_map.cc,v 1.2 2008-01-15 01:25:42 rtv Exp $
 */

#include "p_driver.h"

#include <iostream>
using namespace std;

// DOCUMENTATION ------------------------------------------------------------

/** @addtogroup player 
@par Map interface
- PLAYER_MAP_REQ_GET_INFO
- PLAYER_MAP_REQ_GET_DATA
*/


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
  Geom geom;
  stg_model_get_geom( this->mod, &geom );
  
  
  double mres = this->mod->map_resolution;

  // size_t sz=0;
//   stg_polygon_t* polys = (stg_polygon_t*)
//     stg_model_get_property( this->mod, "polygons", &sz);
//   size_t count = sz / sizeof(stg_polygon_t);
  
  
  // prepare the map info for the client
  player_map_info_t info;  

  info.scale = mres;;
  
  // size in pixels
  info.width = (uint32_t)(geom.size.x / mres);
  info.height = (uint32_t)(geom.size.y / mres);
  
  // origin of map center in global coords
  Pose global;
  memcpy( &global, &geom.pose, sizeof(global)); 
  stg_model_local_to_global( this->mod, &global );
 
  // get real-world pose of lower-left corner of map (this is what Player
  // calls the 'origin'
  info.origin.px = geom.pose.x - geom.size.x / 2.0;
  info.origin.py = geom.pose.y - geom.size.y / 2.0;
  info.origin.pa = geom.pose.a;

  printf( "Stage: creating map for model \"%s\" of %d by %d cells at res %.2f\n", 
	  this->mod->token, 
	  info.width, info.height, info.scale );
       
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
  // if both ends of the line are out of bounds, there's nothing to do
  if( x1<0 && x1>w && y1<0 && y1>h &&
      x2<0 && x2>w && y2<0 && y2>h ) 
    return;
    
  // if the two ends are adjacent, fill in the pixels
  if( abs(x2-x1) <= 1 && abs(y2-y1) <= 1 )
    {
      if( x1 >= 0 && x1 < w && y1 >=0 && y1 < h )
	{
	  if( x1 == w ) x1 = w-1;
	  if( y1 == h ) y1 = h-1;

	  buf[ y1 * w + x1] = val; 
	}

      
      if( x2 >= 0 && x2 <= w && y2 >=0 && y2 <= h )
	{
	  if( x2 == w ) x2 = w-1;
	  if( y2 == h ) y2 = h-1;

	  buf[ y2 * w + x2] = val; 
	}
    }
  else // recursively draw two half-lines
    {
      int xm = x1+(x2-x1)/2;
      int ym = y1+(y2-y1)/2;
      render_line( buf, w, h, x1, y1, xm, ym, val );
      render_line( buf, w, h, xm, ym, x2, y2, val );
    }
}


// Handle map data request
int InterfaceMap::HandleMsgReqData( MessageQueue* resp_queue,
				     player_msghdr_t* hdr,
				     void* data)
{
  // printf( "device %s received map data request\n", this->mod->token );
  
  Geom geom;
  stg_model_get_geom( this->mod, &geom );
  
  double mres = this->mod->map_resolution;
  
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

  printf( "Stage computing map tile (%d,%d)(%d,%d)...",
   	  oi, oj, si, sj);
  fflush(stdout);
  
  // render the polygons directly into the outgoing message buffer. fast! outrageous!
  
  GList* plist;
  for( plist = stg_model_get_polygons( mod ); 
       plist; 
       plist = plist->next )
    {       
      stg_polygon_t* p = (stg_polygon_t*)plist->data;

      // draw each line in the poly
      int line_count = (int)p->points->len;
      for( int l=0; l<line_count; l++ )
	{
	  stg_point_t* pt1 = &g_array_index( p->points, stg_point_t, l ); 
	  stg_point_t* pt2 = &g_array_index( p->points, stg_point_t, (l+1) % line_count );	   
	  
	  render_line( mapresp->data, 
		       mapresp->width, mapresp->height,
		       (int)((pt1->x+geom.size.x/2.0) / mres) -oi, 
		       (int)((pt1->y+geom.size.y/2.0) / mres) -oj,
		       (int)((pt2->x+geom.size.x/2.0) / mres) -oi, 
		       (int)((pt2->y+geom.size.y/2.0) / mres) -oj,
		       1 ); 	  	  
	}	  
    }
  
  // if the model has a boundary, that's in the map too

  // TODO: need to think about this a little. not vital for now....

//   stg_bool_t* boundary = (stg_bool_t*)
//     stg_model_get_property_fixed( mod, "boundary", sizeof(stg_bool_t));
  
//   if( boundary && *boundary )
//     {      
//       int right =  (int)(geom.size.x/mres - oi);
//       int top   =  (int)(geom.size.y/mres - oj);
      
//       render_line( mapresp->data, mapresp->width, mapresp->height,
// 		   0,0, 0, top, 1 );
//       render_line( mapresp->data, mapresp->width, mapresp->height,
// 		   0, top, right, top, 1 );
//       render_line( mapresp->data, mapresp->width, mapresp->height,
// 		   right, top, right, 0, 1 );
//       render_line( mapresp->data, mapresp->width, mapresp->height,
// 		   right, 0, 0,0, 1 );
//     }

   mapresp->data_count = mapresp->width * mapresp->height;
   
   //printf( "Stage publishing map data %d bytes\n",
   //   (int)mapsize );

   this->driver->Publish(this->addr, resp_queue,
			 PLAYER_MSGTYPE_RESP_ACK,
			 PLAYER_MAP_REQ_GET_DATA,
			 (void*)mapresp, mapsize, NULL);
   free(mapresp);   

   puts( " done." );
   
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
