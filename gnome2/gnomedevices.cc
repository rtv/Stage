
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

#endif
