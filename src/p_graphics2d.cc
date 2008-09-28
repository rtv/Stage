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
 * CVS: $Id: p_graphics2d.cc,v 1.7 2007-11-01 22:17:18 gerkey Exp $
 */

#include "p_driver.h"

#include <iostream>
using namespace std;

// DOCUMENTATION ------------------------------------------------------------

/** @addtogroup player 
@par Graphics2d interface
- PLAYER_GRAPHICS2D_CMD_CLEAR
- PLAYER_GRAPHICS2D_CMD_POINTS
- PLAYER_GRAPHICS2D_CMD_POLYLINE
- PLAYER_GRAPHICS2D_CMD_POLYGON
*/

static unsigned int rgb32_pack( player_color_t *pcol )
{
  unsigned int col=0;
  col = pcol->red << 16;
  col += pcol->green << 8;
  col += pcol->blue;
  return col;
}

InterfaceGraphics2d::InterfaceGraphics2d( player_devaddr_t addr, 
			    StgDriver* driver,
			    ConfigFile* cf,
			    int section )
  : InterfaceModel( addr, driver, cf, section, NULL )
{ 
  for (int i = 0; i < INTERFACE_GRAPHICS_2D_MAX_CLIENTS; i++)
	  figs[i].queue = QueuePointer(); // Set to NULL
}

InterfaceGraphics2d::~InterfaceGraphics2d( void )
{ 
  for (int i = 0; i < INTERFACE_GRAPHICS_2D_MAX_CLIENTS; i++)
	if ( this->figs[i].queue != NULL ) // Find the client
		stg_rtk_fig_clear( this->figs[i].stageFig ); // Clear the figure
};


int InterfaceGraphics2d::ProcessMessage(QueuePointer &resp_queue,
				 player_msghdr_t* hdr,
				 void* data)
{
  // printf( "Stage graphics2d interface processing message\n" );

  // Find the index of the client.
  int index;
  for ( index = 0; index < INTERFACE_GRAPHICS_2D_MAX_CLIENTS; index++ )
	  if ( this->figs[index].queue == resp_queue )
		  break;
  if ( index >= INTERFACE_GRAPHICS_2D_MAX_CLIENTS )
  {
	  PLAYER_ERROR("Graphics 2d received message from client that was not subscribed");
	  return 0;
  }
  
  if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_CMD, 
                           PLAYER_GRAPHICS2D_CMD_CLEAR, 
                           this->addr))
    {
      stg_rtk_fig_clear( this->figs[index].stageFig );
      return 0; //ok
    }
  
  if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_CMD, 
                           PLAYER_GRAPHICS2D_CMD_POINTS, 
                           this->addr))
    {
      player_graphics2d_cmd_points_t* pcmd = 
	(player_graphics2d_cmd_points_t*)data;
      
      stg_rtk_fig_color_rgb32( this->figs[index].stageFig, rgb32_pack( &pcmd->color) );
      
      for(unsigned int p=0; p<pcmd->points_count; p++ )        
	{
	  //printf( "g2d: Drawing point %.2f %.2f\n", pcmd->points[p].px, pcmd->points[p].py );
	  stg_rtk_fig_point( this->figs[index].stageFig, 
			     pcmd->points[p].px, 
			     pcmd->points[p].py );      
	}
      return 0; //ok
    }
  
  if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_CMD, 
                           PLAYER_GRAPHICS2D_CMD_POLYLINE, 
                           this->addr))
    {
      player_graphics2d_cmd_polyline_t* pcmd = 
	(player_graphics2d_cmd_polyline_t*)data;
      
      stg_rtk_fig_color_rgb32( this->figs[index].stageFig, rgb32_pack( &pcmd->color) );
      
      if( pcmd->points_count > 1 )
       for(unsigned int p=0; p<pcmd->points_count-1; p++ )     
	  {
	    //printf( "g2d: Drawing polyline section from [%.2f,%.2f] to [%.2f,%.2f]\n", 
	    //    pcmd->points[p].px, pcmd->points[p].py, 
	    //    pcmd->points[p+1].px, pcmd->points[p+1].py );
	    
	    stg_rtk_fig_line( this->figs[index].stageFig, 
			      pcmd->points[p].px, 
			      pcmd->points[p].py,
			      pcmd->points[p+1].px, 
			      pcmd->points[p+1].py );      
	  }
      else
	PRINT_WARN( "refusing to draw a polyline with < 2 points" );
      
      return 0; //ok
    }

  if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_CMD, 
                           PLAYER_GRAPHICS2D_CMD_POLYGON, 
                           this->addr))
    {
      player_graphics2d_cmd_polygon_t* pcmd = 
	(player_graphics2d_cmd_polygon_t*)data;
      
      
      double (*pts)[2] = new double[pcmd->points_count][2];
      
      for(unsigned int p=0; p<pcmd->points_count; p++ )        
	{
	  pts[p][0] = pcmd->points[p].px;
	  pts[p][1] = pcmd->points[p].py;
	}
      
      //printf( "g2d: Drawing polygon of %d points\n", pcmd->count ); 
      
      if( pcmd->filled )
      {
          stg_rtk_fig_color_rgb32( this->figs[index].stageFig, rgb32_pack( &pcmd->fill_color));
          stg_rtk_fig_polygon( this->figs[index].stageFig, 
			       0,0,0,			   
                              pcmd->points_count,
			       pts,
			       TRUE );
      }
      
      stg_rtk_fig_color_rgb32( this->figs[index].stageFig, rgb32_pack( &pcmd->color) );
      stg_rtk_fig_polygon( this->figs[index].stageFig, 
			   0,0,0,			   
                          pcmd->points_count,
			   pts,
			   FALSE );
      delete [] pts;
      return 0; //ok
    }
  
  PLAYER_WARN2("stage graphics2d doesn't support message %d:%d.", 
	       hdr->type, hdr->subtype );
  return -1;
}

int InterfaceGraphics2d::Subscribe(QueuePointer &queue, player_devaddr_t addr)
{
	int result = InterfaceModel::Subscribe(queue, addr);
	if (result != 0)
		return result;
	
	if (queue == NULL) // Always on subscription.
		return 0;
	
	// Find an index for the new client.
	int index;
	for (index = 0; index < INTERFACE_GRAPHICS_2D_MAX_CLIENTS; index++)
	{
		if (figs[index].queue == queue)
		{
			PLAYER_ERROR("Client subscribed to graphics2d twice");
			return -1;
		}
		if (figs[index].queue == NULL) // Found an empty position.
			break;
	}
	
	if (index >= INTERFACE_GRAPHICS_2D_MAX_CLIENTS)
	{
		PLAYER_ERROR("Too many clients subscribed to graphics 2d");
		return -1;
	}
	
	// Take over position.
	figs[index].queue = queue;
	
	char figname[10];
	snprintf(figname, 10, "g2d_fig%d", index); // Name the figures g2d_fig0 through g2d_fign.
	
	// Get or create a nice clean figure for the client to draw on.
	figs[index].stageFig = stg_model_fig_get_or_create( mod, figname, "top", 99 );
	stg_rtk_fig_clear( figs[index].stageFig );
	
	return 0;
}

int InterfaceGraphics2d::Unsubscribe(QueuePointer &queue, player_devaddr_t addr)
{
	if (queue == NULL)
		return InterfaceModel::Unsubscribe(queue, addr);
	
	int index;
	for (index = 0; index < INTERFACE_GRAPHICS_2D_MAX_CLIENTS; index++)
	{
		if (figs[index].queue == queue)
			break;
	}
	
	if (index >= INTERFACE_GRAPHICS_2D_MAX_CLIENTS)
	{
		PLAYER_ERROR("Client unsubscribed without subscribing");
		return -1;
	}
	
	stg_rtk_fig_clear( figs[index].stageFig );
	figs[index].queue = QueuePointer(); // Set to NULL.
	
	return InterfaceModel::Unsubscribe(queue, addr);
}
