/*
 *  Stage : a multi-robot simulator.
 *  Copyright (C) 2001, 2002 Richard Vaughan, Andrew Howard and Brian Gerkey.
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
 * Desc: GNOME2 visualization code for each device
 * Author: Richard Vaughan
 * Date: 26 Oct 2002
 * CVS info: $Id: gnomedevices.cc,v 1.1.2.1 2003-02-05 04:03:21 rtv Exp $
 */

#if HAVE_CONFIG_H
  #include <config.h>
#endif

// if stage was configured to use the experimental GNOME GUI
#ifdef USE_GNOME2


#include "gnomegui.hh"


// device data renderers
void GnomeEntityRenderDataLaser( CLaserDevice* ent )
{
  PRINT_DEBUG( "" );

  // get the device's data for rendering
  player_laser_data_t data;
  size_t len  =  ent->GetData( (void*)&data, sizeof(data) );

  assert( len > 0 );
  // rtv - took out last-time comparison for now...

  // we render the data as a colored polygon, if the range data is
  // different from last time. note that we look at the ranges
  // themselves, not just the timestamp, as we can get the same scans
  // over and over...
  //if( memcmp( &data->ranges, &last_data.ranges, 
  //      ntohs(data->range_count) * sizeof(data->ranges[0])
  //      ) != 0 )
  // {
      // convert the range data to a set of cartesian points
      short min_ang_deg = ntohs(data.min_angle);
      short max_ang_deg = ntohs(data.max_angle);
      unsigned short samples = ntohs(data.range_count); 
      
      GnomeCanvasPoints* points = gnome_canvas_points_new(samples+2);
      
      double min_ang_rad = DTOR( (double)min_ang_deg ) / 100.0;
      double max_ang_rad = DTOR( (double)max_ang_deg ) / 100.0;
      
      double incr = (double)(max_ang_rad - min_ang_rad) / (double)samples;
      
      // set the start and end points of the scan as the origin;
      memset( points->coords, 0, sizeof(points->coords[0]) * 2 * (samples+2) ); 
      
      double bearing = min_ang_rad;
      for( int i=0; i < (int)samples; i++ )
	{
	  // get range, converting from mm to m
	  unsigned short range_mm = ntohs(data.ranges[i]) & 0x1FFF;
	  double range_m = (double)range_mm / 1000.0;
	  
	  bearing += incr;
	  
	  double px = range_m * cos(bearing);
	  double py = range_m * sin(bearing);
	  
	  points->coords[2*i+2] = px;
	  points->coords[2*i+3] = py;
	  
	  // TODO
	  // add little boxes at high intensities (like in playerv)
	  //if(  (unsigned char)(ntohs(data.ranges[i]) >> 13) > 0 )
	  //{ rtk_fig_rectangle(this->scan_fig, px, py, 0, 0.05, 0.05, 1);
	  //rtk_fig_line( this->scan_fig, 0,0,px, py );
	  //      }
	}
      
      // kill any previous data rendering
      if( GetData(ent) ) 
	{
	  gtk_object_destroy(GTK_OBJECT(GetData(ent)));
	  SetData(ent, NULL);
	}
	  
      // construct a polygon matching the lasersweep
      SetData( ent,  
	       gnome_canvas_item_new ( GetGroup(ent),
				       gnome_canvas_polygon_get_type(),
				       "points", points,
				       "fill_color_rgba", RGBA(ent->color,16),
				       "outline_color", NULL, 
				       // TODO - optional laser scan outline
				       //"outline_color_rgba", RGBA(this->color,255),
				       //"width_pixels", 1,
				       NULL ) 
	       );
      
      gnome_canvas_points_free(points);
      //}
      
      // store the old data for next time
      //memcpy( &last_data, data, sizeof(player_laser_data_t) );
}

void GnomeEntityRenderDataSonar( CSonarDevice* ent )
{  

  PRINT_DEBUG( "" );

  // get the device's data for rendering
  player_sonar_data_t data;
  size_t len = ent->GetData( &data, sizeof(data) );

  assert( len > 0 );

  // TODO - have this conditional on the view data menu
  //if( m_world->ShowDeviceData( this->stage_type) )
  
  // we render the data as a colored polygon, if the range data is
  // different from last time. note that we look at the ranges
  // themselves, not just the timestamp, as we can get the same scans
  // over and over...

  //if( memcmp( &data.ranges, &last_data.ranges, 
  //      ntohs(data.range_count) * sizeof(data.ranges[0])
  //      ) != 0 )
  // {
      
      // kill any previous data rendering
  if( GetData(ent) ) 
    gtk_object_destroy(GTK_OBJECT(GetData(ent)));
      
      // construct a group to contain several sonar triangles
   SetData( ent,
	    gnome_canvas_item_new( GetGroup(ent),
				   gnome_canvas_group_get_type(),
				   "x", 0,
				   "y", 0,
				   NULL) 
	    );
      
   GnomeCanvasPoints* points = gnome_canvas_points_new(3);
   
   for (int s=0; s < ntohs(data.range_count); s++)
     {	 
       // convert from integer mm to double m
       double range = (double)ntohs(data.ranges[s]) / 1000.0;      
       double originx = ent->sonars[s][0];
       double originy = ent->sonars[s][1];
       double angle = ent->sonars[s][2];
       
       double hitx =  originx + range * cos(angle); 
       double hity =  originy + range * sin(angle);
       
       double diverge = M_PI/40.0;
       double sidelen = range / cos(diverge);
       
       points->coords[0] = ent->sonars[s][0]; // transducer origin
       points->coords[1] = ent->sonars[s][1];
       points->coords[2] = originx + sidelen * cos( angle - diverge );
       points->coords[3] = originy + sidelen * sin( angle - diverge );
       points->coords[4] = originx + sidelen * cos( angle + diverge );
       points->coords[5] = originy + sidelen * sin( angle + diverge );
       
       // construct a polygon matching the lasersweep
       assert( gnome_canvas_item_new ( GNOME_CANVAS_GROUP(GetData(ent)),
				       gnome_canvas_polygon_get_type(),
				       "points", points,
				       "fill_color_rgba", RGBA(ent->color,32),
				       "outline_color", NULL,
				       // TODO - optional sonar scan outline
				       //"outline_color_rgba", RGBA(::LookupColor("green"),255),
				       //"width_pixels", 1,
				       NULL ) );
       
     }
   
   gnome_canvas_points_free(points);
   // }
   // store the old data for next time
   //memcpy( &last_data, data, len );
}

void GnomeEntityRenderDataBlobfinder( CVisionDevice* ent )
{  
  uint8_t transparency = 200; 

  player_blobfinder_data_t data;
  
  // attempt to get the right size chunk of data from the mmapped buffer
  if( ent->GetData( &data, sizeof(data) ) != sizeof(data) )
    {
      puts( "got wrong data size" );
      return;
    }
 
  // kill any previous data rendering
  if( GetData(ent) ) 
    gtk_object_destroy(GTK_OBJECT(GetData(ent)));
  
  // construct a group under the entity to contain a screen and color
  // blobs
  SetData( ent, gnome_canvas_item_new( GetOrigin(ent),
				       gnome_canvas_group_get_type(),
				       "x", -1.0,
				       "y", -0.5, NULL));
     
  double scale = 0.007; // shrink from pixels to meters for display
  
  short width = ntohs(data.width);
  short height = ntohs(data.height);
  double mwidth = width * scale;
  double mheight = height * scale;
  
  double offsetx = -mwidth/2.0;//-1.5;
  double offsety = -mheight/2.0;//-1.5;

  // draw a white TV screen 
  gnome_canvas_item_new (  GNOME_CANVAS_GROUP(GetData(ent)), 
			  gnome_canvas_rect_get_type(),
			  "x1", offsetx,
			  "y1", offsety,
			  "x2", offsetx + mwidth,
			  "y2", offsety + mheight,
			   //"fill_color", "white",
			   //"outline_color", "black",
			   "fill_color_rgba", 0xFFFFFF00 + transparency ,
			  "outline_color_rgba", 0x0 + transparency,
			  "width_pixels", 1,
			  NULL );

  
  // The vision figure is attached to the entity, but we dont want
  // it to rotate.  Fix the rotation here.
  double gx, gy, gth, rotate[6];
  ent->GetGlobalPose(gx, gy, gth);
  art_affine_rotate( rotate, RTOD(-gth) );
  gnome_canvas_item_affine_relative( GNOME_CANVAS_ITEM(GetData(ent)), rotate);

  // for each color channel
  for( int c=0; c<PLAYER_BLOBFINDER_MAX_CHANNELS;c++)
    {
      short numblobs = ntohs(data.header[c].num);
      short index = ntohs(data.header[c].index);	    
  
      // draw a color blob 
      for( int b=0; b<numblobs; b++ )
	{
	  uint32_t color = RGBA(ntohl(data.blobs[index+b].color),transparency); 
	  
	  short top =  ntohs(data.blobs[index+b].top);
	  short bot =  ntohs(data.blobs[index+b].bottom);
	  short left =  ntohs(data.blobs[index+b].left);
	  short right =  ntohs(data.blobs[index+b].right);
	  
	  double mtop = top * scale;
	  double mbot = bot * scale;
	  double mleft = left * scale;
	  double mright = right * scale;
	  
	  gnome_canvas_item_new (  GNOME_CANVAS_GROUP(GetData(ent)), 
				   gnome_canvas_rect_get_type(),
				   "x1", offsetx + mleft,
				   "y1", offsety + mtop,
				   "x2", offsetx + mright,
				   "y2", offsety + mbot,
				   "fill_color_rgba", color,
				   NULL );
	}
    }
}
#endif
