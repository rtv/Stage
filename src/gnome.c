#include <libgnomecanvas/libgnomecanvas.h>

#include "config.h"
#include "stage_internal.h"
#include "stage.h"


#define STG_TO_GNOME_COLOR(COL) ((COL<<8)+255)
#define SENSOR_ALPHA 90

#define GRID_COLOR GNOME_CANVAS_COLOR_A(0,0,0,32)

// BEGIN MODEL

gint
acetate_event_callback(GnomeCanvasItem* item, 
		       GdkEvent* event, 
		       gpointer data)
{
  printf( "ACETATE EVENT TYPE %d\n", event->type );

  //gnome_canvas_item_raise_to_top( item );

  return FALSE;
}


void gc_model_highlight( stg_model_t* mod )
{
  GnomeCanvasGroup* grp = stg_model_get_property( mod, "_gc_basegroup" );
  assert(grp);

  GnomeCanvasItem* it = 
    gnome_canvas_item_new( grp,
			   gnome_canvas_ellipse_get_type(),
			   "x1", -mod->geom.size.x/1.0,
			   "y1", -mod->geom.size.y/1.0,
			   "x2", mod->geom.size.x/1.0,
			   "y2", mod->geom.size.y/1.0,
			   "outline_color", "red",
			   "width-pixels", 3,
			   "fill_color", 0,
			   NULL ); 

  stg_model_set_property( mod, "_gc_selected", it );
}

gc_model_highlight_remove( stg_model_t* mod )
{
  GnomeCanvasItem* it = stg_model_get_property( mod, "_gc_selected" );
  
  if( it )
    gtk_object_destroy( it );

  stg_model_unset_property( mod, "_gc_selected" );
}


void gc_model_select( stg_model_t* mod )
{
  printf( "Selecting model %s\n", mod->token );

  // if it's not there already, select it
  if( g_list_index( mod->world->selected_models, mod ) < 0 )
    {
      puts( "ADDING TO SELECTION LIST" );

      gc_model_highlight( mod );
      
      mod->world->selected_models = 
	g_list_append( mod->world->selected_models, mod );
    }
}

void gc_model_unselect( stg_model_t* mod )
{
  printf( "UNSelecting model %s\n", mod->token );

  // if it's selected, unselect it
  if( g_list_index( mod->world->selected_models, mod ) != -1 )
    {
      puts( "REMOVING FROM SELECTION LIST" );

      gc_model_highlight_remove( mod );
      
      mod->world->selected_models = 
	g_list_remove( mod->world->selected_models, mod );
    }
}


void gc_model_highlight_remove_cb( gpointer data, gpointer user )
{
  gc_model_highlight_remove( (stg_model_t*)data );
}

void gc_model_unselect_cb( gpointer data, gpointer user )
{
  gc_model_unselect( (stg_model_t*)data );
}


void select_if_in_rect( gpointer key,
		      gpointer value,
		      gpointer user_data )
{
  stg_model_t* mod = (stg_model_t*)value;
  double *bounds = (double*)user_data;
  
  if( mod->parent || (mod->gui_mask==0) )
    return;

  // normalize the bounds box
  if( bounds[2] < bounds[0] ) 
    {
      double tmp=bounds[0];
      bounds[0] = bounds[2];
      bounds[2] = tmp;
    }
  if( bounds[3] < bounds[1] ) 
    {
      double tmp=bounds[1];
      bounds[1] = bounds[3];
      bounds[3] = tmp;
    }

  printf( "testing %s pose (%f,%f)  bounds (%f, %f) to (%f, %f)\n",
	  mod->token,
	  mod->pose.x,
	  mod->pose.y,
	  bounds[0],
	  bounds[1],
	  bounds[2],
	  bounds[3] );

  if( mod->pose.x > bounds[0] && 
      mod->pose.y > bounds[1] && 
      mod->pose.x < bounds[2] && 
      mod->pose.y < bounds[3] )
    {
      printf( "model %s selected\n", mod->token );
    
      gc_model_select( mod );
    }
}


gint background_event_callback(GnomeCanvasItem* item, 
			       GdkEvent* event, 
			       gpointer data)
{
  //puts( "background callback" );

  static int dragging = 0;
  static int zoom = 0;

  static double start_x=0.0, start_y=0.0;

  static int pixel_start_x, pixel_start_y;

  GdkCursor* cursor = NULL;
  double item_x = event->button.x;
  double item_y = event->button.y;

  stg_world_t* world = (stg_world_t*)data;

  static GnomeCanvasItem* selector = NULL;

  switch (event->type) {
    case GDK_BUTTON_PRESS:
      printf( "press\n" );
      
      gnome_canvas_item_grab_focus( item );
      
      start_x = item_x;
      start_y = item_y;
      
      switch(event->button.button) 
	{
	case 1:	  
	  
	  // if nothing is selected, we're dragging rects
	  if( g_list_length( world->selected_models ) == 0 )
	    {
	  cursor = gdk_cursor_new(GDK_FLEUR);
	  dragging = TRUE;

	  selector = gnome_canvas_item_new( gnome_canvas_root(world->win->gcanvas),
					    gnome_canvas_rect_get_type(),
					    "x1", start_x,
					    "y1", -start_y,
					    "x2", start_x,
					    "y2", -start_y,
					    "fill_color_rgba", GNOME_CANVAS_COLOR_A(255,0,255,20),
					    "outline_color_rgba", GNOME_CANVAS_COLOR_A(0,0,0,200),	    
					    "width-pixels", 1,
					    NULL);
	    }

	  break;

	case 3:
	  cursor = gdk_cursor_new(GDK_SIZING);
	  zoom = TRUE;
	  gtk_widget_get_pointer( GTK_WIDGET(world->win->gcanvas), 
				  &pixel_start_x,
				  &pixel_start_y );
	  
	  printf( "START x %d  y %d\n", pixel_start_x, pixel_start_y );

	  break;
	    
	default:
	  printf( "button: %d\n",event->button.button); 
	}

      gnome_canvas_item_grab(item,
			     GDK_POINTER_MOTION_MASK | 
			     GDK_BUTTON_RELEASE_MASK,
			     cursor,
			     event->button.time);
      gdk_cursor_destroy(cursor);

      break;
      
    case GDK_MOTION_NOTIFY:
     if (dragging && (event->motion.state & GDK_BUTTON1_MASK)) 
        {
	  // this works to move the item, but doesn't change the scroll region
	  //gnome_canvas_item_w2i( item,  &item_x, &item_y );
          //gnome_canvas_item_move( item, item_x, item_y);

	  // TODO
	  //gnome_canvas_set_scroll_region( world->win->gcanvas, 
	  //			  x, y, 10.0, 10.0 );

	  printf( "button 1 & motion [%f,%f]\n", item_x, -item_y );
	  gnome_canvas_item_set( selector, "x2", item_x, "y2", -item_y, NULL );
        }
      else if( zoom && (event->motion.state & GDK_BUTTON3_MASK)) 
	{
	  int px, py;
	  gtk_widget_get_pointer( GTK_WIDGET(world->win->gcanvas), 
				  &px, &py );

	  printf( "px: %d %d  \n", px, py );

	  int dx = px - pixel_start_x;

	  pixel_start_x = px;

	  printf( "px: %d %d   dx %d\n", px, py, dx );
	  
	  // cx, cy is the dimension of the whole canvas in pixels
	  int cx, cy;
	  gtk_layout_get_size( GTK_LAYOUT(world->win->gcanvas), &cx, &cy );
	  printf( "layout cx: %d cy: %d\n", cx, cy );

	  

	  world->win->zoom += dx;

	  if( world->win->zoom < 1.0 )
	    world->win->zoom = 1.0;

	  printf( "world->win->zoom %.6f\n", world->win->zoom );
	  //gnome_canvas_set_pixels_per_unit( world->win->gcanvas, world->win->zoom  );

	  gnome_canvas_set_pixels_per_unit( world->win->gcanvas,  
					    world->win->zoom );	  
	}

      break;
                    
    case GDK_BUTTON_RELEASE:
      printf( "release\n" );
      dragging = FALSE;
      zoom = FALSE;

/*       if( multimove ) */
/* 	{ */
/* 	  printf( "multimove ends" ); */
/* 	  multimove = FALSE; */

/* 	  // remove the selection figures */
/* 	  g_list_foreach(  world->selected_models, gc_model_unselect_cb, NULL ); */
/* 	} */
      if( selector )
	{

	  // TODO - find all the models under the selector and select them
	  printf( "RUBBERBOX bounds   [%f,%f] to [%f,%f]\n", start_x, -start_y, item_x, -item_y );
	  
	  double bounds[4];
	  bounds[0] = start_x;
	  bounds[1] = -start_y;
	  bounds[2] = item_x;
	  bounds[3] = -item_y;
	  

	  g_hash_table_foreach( world->models, select_if_in_rect, bounds );

	  gtk_object_destroy( selector );
	  selector = NULL;
	}
      else // unselect everything
	g_list_foreach(  world->selected_models, gc_model_unselect_cb, NULL ); 
      
      gnome_canvas_item_ungrab(item, event->button.time);

      break;

    default:
      break;
  }

  /* Returning FALSE propagates the event to parent items;
   * returning TRUE ends event propagation. 
   */
  return FALSE;
}


void gc_move_model( stg_model_t* mod, GdkEvent* event )
{
  printf( "moving selected model %s\n", mod->token );
}


gint model_event_callback(GnomeCanvasItem* item, 
			  GdkEvent* event, 
			  gpointer data)
{
  puts( "event callback" );
  
  stg_model_t* mod = (stg_model_t*)data;
  

  // pass this event on to top-level objects
  if( mod->parent )//||  mod->gui_mask )
    return model_event_callback( item, event, mod->parent );

  // if we get here, this is a top-level model
  if( mod->gui_mask == 0 )
    return FALSE; // don't handle the event

  static int dragging = 0;
  static int spinning = 0;
  
  GdkCursor* cursor= NULL;
  
  double item_x = event->button.x;
  double item_y = event->button.y;
  
  static double start_item_x=0.0, start_item_y=0.0;
  
  switch (event->type) 
    {
    case GDK_BUTTON_PRESS:
      printf( "press\n" );
      switch(event->button.button) 
	{
	case 1:
	  cursor = gdk_cursor_new(GDK_FLEUR);
	  dragging = TRUE;
	  break;
	  
	case 3:
	  start_item_x = event->button.x;
	  start_item_y = event->button.y;
	  //cursor = gdk_cursor_new(GDK_SB_H_DOUBLE_ARROW);
	  cursor = gdk_cursor_new(GDK_EXCHANGE);
	  spinning = TRUE;	  
	  break;
	}
      
      gnome_canvas_item_grab_focus( item );      
      gnome_canvas_item_grab(item,
			     GDK_POINTER_MOTION_MASK |
			     GDK_BUTTON_RELEASE_MASK,
			     cursor,
			     event->button.time);
      gdk_cursor_destroy(cursor);
      
      break;
      
    case GDK_ENTER_NOTIFY:
      {
	puts( "enter" );	
	gc_model_select( mod );
      }
      
      
      break;
      
    case GDK_LEAVE_NOTIFY:
      puts( "leave" );
      
      // if this model is the only one selected, we unselect it
      if( g_list_length( mod->world->selected_models ) == 1 )
	gc_model_unselect( mod );

      guint cid = 
	gtk_statusbar_get_context_id( mod->world->win->status_bar, "on_mouse" );
      gtk_statusbar_pop( mod->world->win->status_bar, cid ); 
      
      break;
      
      
    case GDK_MOTION_NOTIFY:
      if( (!(event->button.x == start_item_x  && event->button.y == start_item_y )) 
	  &&  (dragging||spinning))
	{
	  stg_pose_t pose;
	  stg_model_get_pose( mod, &pose );
	  
	  if (dragging && (event->motion.state & GDK_BUTTON1_MASK)) 
	    {
	      //gnome_canvas_item_w2i( item, &item_x, &item_y );
	      //gnome_canvas_item_move(item, item_x, item_y);
	      
	      pose.x = item_x;
	      pose.y = -item_y;
	    }
	  
	  if (spinning && (event->motion.state & GDK_BUTTON3_MASK)) 
	    {
	      if( event->button.x != start_item_x )
		{
		  if( event->button.x > start_item_x )
		    pose.a += M_PI/48.0;
		  else
		    pose.a -= M_PI/48.0;
		}
	    }
	  
	  start_item_x = event->button.x;
	  start_item_y = event->button.y;
	  
	  // for all the selected models
	  g_list_foreach( mod->world->selected_models, gc_move_model, event );

	  stg_model_set_pose( mod, &pose );
	}
      break;
      
    case GDK_BUTTON_RELEASE:
      printf( "release\n" );
      dragging = FALSE;
      spinning = FALSE;
      gnome_canvas_item_ungrab(item, event->button.time);
      break;

    default:
      break;
  }

  /* Returning FALSE propagates the event to parent items;
   * returning TRUE ends event propagation. 
   */
  return TRUE;
}


GnomeCanvasItem* gc_model_group_create( stg_model_t* mod, 
					const char* name, 
					const char* parent_name )
{
  GnomeCanvasItem* parent_grp = 
    stg_model_get_property( mod, parent_name ? parent_name : "_gc_basegroup" );
  
  GnomeCanvasItem* grp = NULL;
  
  assert( grp = gnome_canvas_item_new( parent_grp,
				       gnome_canvas_group_get_type(),
				       "x", 0,
				       "y", 0,
				       NULL ) );
  gnome_canvas_item_raise_to_top( grp );

  // allow for local pose */
  if( mod->geom.pose.x || mod->geom.pose.y || mod->geom.pose.a )
    {
      double local_trans[6], local_rot[6]; 
      art_affine_translate( local_trans, mod->geom.pose.x, mod->geom.pose.y ); 
      art_affine_rotate( local_rot, RTOD(mod->geom.pose.a) ); 
      art_affine_multiply( local_trans, local_trans, local_rot ); 
      gnome_canvas_item_affine_relative( grp, local_trans ); 
    }

  stg_model_set_property( mod, name, grp );

  return grp;
} 

void gc_model_group_destroy( stg_model_t* mod, const char* name )
{
  GnomeCanvasItem* grp = stg_model_get_property( mod, name );
  
  if( grp )
    {
      stg_model_set_property( mod, name, NULL );
      gtk_object_destroy( grp );
    }
}

int gc_model_polygons(  stg_model_t* mod, void* userp )
{

  stg_color_t col = mod->color;

  size_t count = mod->polygons_count;
  stg_polygon_t* polys = mod->polygons;

  stg_geom_t geom;
  stg_model_get_geom(mod, &geom);
  
  gc_model_group_destroy( mod, "_gc_polys" );
  GnomeCanvasItem* grp = gc_model_group_create( mod, "_gc_polys", NULL );
  
  if( polys && mod->polygons_count  > 0 )
    {      
      
      gc_draw_polygons( grp,  
			mod->polygons, 
			mod->polygons_count,
			mod->color, 
			mod->world->win->fill_polygons );
      
      // if this is a top-level item and not marked immovable, we need
      // mouse events from it
      //if( (! mod->parent) && mod->gui_mask )

      g_signal_connect_after( grp, "event", G_CALLBACK(model_event_callback), mod );
    }
  
  if( mod->boundary )
    {
      gnome_canvas_item_new( grp,
			     gnome_canvas_rect_get_type(),
			     "x1", geom.pose.x - geom.size.x/2.0,
			     "y1", geom.pose.y - geom.size.y/2.0,
			     "x2", geom.pose.x + geom.size.x/2.0,
			     "y2", geom.pose.y + geom.size.y/2.0,
			     "outline_color", "black",
			     "width-pixels", 1,
			     "fill_color", 0,
			     NULL );
    }

/*   if( mod->gui_nose ) */
/*     {        */
/*       // TODO */
/*     } */
  
  return 0;
}



/// move a model's figure to the model's current location
int gc_model_move( stg_model_t* mod, void* userp )
{ 
  //puts( "model move" );

  GnomeCanvasItem* grp = 
    stg_model_get_property( mod, "_gc_basegroup" );
  assert(grp);

  double r[6], t[6], f[6];
  art_affine_rotate(r,RTOD(mod->pose.a));
  //art_affine_translate( t, mod->pose.x, mod->pose.y ); 
  //art_affine_multiply( f, t, r ); 
  gnome_canvas_item_affine_absolute( grp, r );

  gnome_canvas_item_set( grp,
			 "x", mod->pose.x,
			 "y", mod->pose.y,
			 NULL );
  return 0;
}


// Draw a grid.  Grid is centered on (ox, oy) with size (dx, dy) with
// spacing (sp).
void gc_grid( GnomeCanvasItem* grp, 
	      double ox, double oy,
	      double dx, double dy, double sp,
	      uint32_t color )
{
  int i, nx, ny;

  nx = (int) ceil((dx/2.0) / sp);
  ny = (int) ceil((dy/2.0) / sp);

  if( nx == 0 ) nx = 1.0;
  if( ny == 0 ) ny = 1.0;


  // draw the bounding box first
  //stg_rtk_fig_rectangle( fig, ox,oy,0, dx, dy, 0 );

  GnomeCanvasPoints* xpts = gnome_canvas_points_new( 4 * nx); 
  GnomeCanvasPoints* ypts = gnome_canvas_points_new( 4 * nx); 

  int index=0;
  for (i = -nx; i < nx; i+=2)
  {
    xpts->coords[index++] =  ox + i * sp;
    xpts->coords[index++] =  oy - dy/2;
    xpts->coords[index++] =  ox + i * sp;
    xpts->coords[index++] =  oy + dy/2;
    xpts->coords[index++] =  ox + i+1 * sp;
    xpts->coords[index++] =  oy + dy/2;
    xpts->coords[index++] =  ox + i+1 * sp;
    xpts->coords[index++] =  oy - dy/2;
 	       
    //snprintf( str, 64, "%d", (int)i );
    //stg_rtk_fig_text( fig, -0.2 + (ox + i * sp), -0.2 , 0, str );
  }

  index=0;
  for (i = -ny; i < ny; i+=2)
    {
      ypts->coords[index++] =  ox - dx/2;
      ypts->coords[index++] =  oy + i * sp;
      ypts->coords[index++] =  ox + dx/2;
      ypts->coords[index++] =  oy + i * sp;
      ypts->coords[index++] =  ox + dx/2;
      ypts->coords[index++] =  oy + i+1 * sp;
      ypts->coords[index++] =  ox - dx/2;
      ypts->coords[index++] =  oy + i+1 * sp;
      
      //snprintf( str, 64, "%d", (int)i );
      //stg_rtk_fig_text( fig, -0.2, -0.2 + (oy + i * sp) , 0, str );
    }
  
  gnome_canvas_item_new( grp,
			 gnome_canvas_line_get_type(),
			 "fill_color_rgba", color,
			 "width-pixels", 1,
			 "points", xpts,
			 NULL );
  
  gnome_canvas_item_new( grp,
			 gnome_canvas_line_get_type(),
			 "fill_color_rgba", color,
			 "width-pixels", 1,
			 "points", ypts,
			 NULL );
  
  gnome_canvas_points_free( xpts );
  gnome_canvas_points_free( ypts );

  // draw the axis origin lines
  //stg_rtk_fig_color_rgb32( fig, 0 );
  //stg_rtk_fig_line( fig, ox-dx/2, 0, ox+dx/2, 0 );
  //stg_rtk_fig_line( fig, 0, oy-dy/2, 0, oy+dy/2 );
}


int gc_model_grid( stg_model_t* mod, void* userp )
{  
  gc_model_group_destroy( mod, "_gc_grid" );

  if( mod->gui_grid  )
    {    
      GnomeCanvasItem* grid = gc_model_group_create( mod, "_gc_grid", NULL );
      
      gc_grid( grid, 
	       mod->geom.pose.x, mod->geom.pose.y, 
	       mod->geom.size.x, mod->geom.size.y, 1.0, GRID_COLOR );
    }
  
  return 0;
}

int gc_model_grid_clear( stg_model_t* mod, void* userp )
{
  puts( "GRID CLEAR" );

  gc_model_group_destroy( mod, "_gc_grid" );
  return 1; // don't call me again
}

// END MODEL

// BEGIN LASER

int gc_laser_data( stg_model_t* mod, void* userp )
{
  stg_laser_config_t *cfg = (stg_laser_config_t*)mod->cfg;
  
  stg_laser_sample_t* samples = (stg_laser_sample_t*)mod->data;
  size_t sample_count = mod->data_len / sizeof(stg_laser_sample_t);

  // zap the old one
  GnomeCanvasItem* gp = stg_model_get_property( mod, "_gc_laser_data" );
  gtk_object_destroy( gp );

  if( samples && sample_count )
    {
      GnomeCanvasPoints *gpts =  gnome_canvas_points_new( sample_count );
      
      // now get on with rendering the laser data
      stg_pose_t pose;
      stg_model_get_global_pose( mod, &pose );
      
      stg_geom_t geom;
      stg_model_get_geom( mod, &geom );
      
      double sample_incr = cfg->fov / (sample_count-1.0);
      double bearing = geom.pose.a - cfg->fov/2.0;
      
      int s;
      for( s=0; s<sample_count; s++ )
	{
	  gpts->coords[2*s] = (samples[s].range) * cos(bearing);
	  gpts->coords[2*s+1] = (samples[s].range) * sin(bearing);
	  bearing += sample_incr;
	}
      
      GnomeCanvasItem* gp = 
	gnome_canvas_item_new( stg_model_get_property( mod, "_gc_basegroup" ),
			       gnome_canvas_polygon_get_type(),
			       "fill_color_rgba",   mod->world->win->fill_polygons ? GNOME_CANVAS_COLOR_A(0,0,255,20) : 0,
			       "outline_color_rgba",  GNOME_CANVAS_COLOR_A(0,0,255,SENSOR_ALPHA),
			       "width-pixels", 1,
			       "points", gpts,
			       NULL );      

      gnome_canvas_points_free( gpts );
      gnome_canvas_item_lower_to_bottom( gp );
      stg_model_set_property( mod, "_gc_laser_data", gp );
    }
  
  return 0; // callback runs until removed
}

int gc_laser_data_clear( stg_model_t* mod, void* userp )
{
  gc_model_group_destroy( mod, "_gc_laser_data" );
  return 1; // callback just runs once
}


// END LASER

// RANGER

int gc_ranger_data( stg_model_t* mod, void* userp )
{
  PRINT_DEBUG( "ranger render data" );

  // erase prior drawings
  gc_model_group_destroy( mod, "_gc_ranger" );
  
  size_t clen = mod->cfg_len;
  stg_ranger_config_t* cfg = (stg_ranger_config_t*)mod->cfg;
  
  // any samples at all?
  if( clen < sizeof(stg_ranger_config_t) )
    return 0;
  
  int rcount = clen / sizeof( stg_ranger_config_t );
  
  if( rcount < 1 ) // no samples
    return 0;
  
  size_t dlen = mod->data_len;
  stg_ranger_sample_t *samples = (stg_ranger_sample_t*)mod->data;
  
  // iff we have the right amount of data
  if( dlen == rcount * sizeof(stg_ranger_sample_t) )
    {
      // create a new drawing group 
      GnomeCanvasItem* gp = gc_model_group_create( mod, "_gc_ranger", NULL );
      
      stg_geom_t geom;
      stg_model_get_geom(mod,&geom);
      
      // draw the range  beams
      int s;
      for( s=0; s<rcount; s++ )
	{
	  if( samples[s].range > 0.0 )
	    {
	      stg_ranger_config_t* rngr = &cfg[s];
	      
	      //stg_rtk_fig_arrow( fig,
	      //		 rngr->pose.x, rngr->pose.y, rngr->pose.a,
	      //		 samples[s].range, 0.02 );
	      	      
	      // sensor FOV
	      double sidelen = samples[s].range;
	      double da = rngr->fov/2.0;

	      GnomeCanvasPoints *gpts =  gnome_canvas_points_new( 3 );

	      gpts->coords[0] = rngr->pose.x; 
	      gpts->coords[1] = rngr->pose.y; 
	      gpts->coords[2] = rngr->pose.x + sidelen*cos(rngr->pose.a - da ); 
	      gpts->coords[3] = rngr->pose.y + sidelen*sin(rngr->pose.a - da ); 
	      gpts->coords[4] = rngr->pose.x + sidelen*cos(rngr->pose.a + da ); 
	      gpts->coords[5] = rngr->pose.y + sidelen*sin(rngr->pose.a + da ); 
	      
	      GnomeCanvasItem* it = 
		gnome_canvas_item_new( gp,
				       gnome_canvas_polygon_get_type(),
				       "fill_color_rgba", GNOME_CANVAS_COLOR_A(0,255,0,20),
				       "outline_color_rgba", GNOME_CANVAS_COLOR_A(0,255,0,SENSOR_ALPHA),
				       "width-pixels", 1, 
				       "points", gpts,
				       NULL );

	      gnome_canvas_points_free( gpts );
	    }
	}
    }
  else
    if( dlen > 0 )
      PRINT_WARN2( "data size doesn't match configuation (%d/%d bytes)",
		   (int)dlen,  (int)rcount * sizeof(stg_ranger_sample_t) );
  
  return 0; // keep running
}

int gc_ranger_data_clear( stg_model_t* mod, void* userp )
{
  gc_model_group_destroy( mod, "_gc_ranger" );
  return 1;
}

// END RANGER


void stg_model_draw_clear(  stg_model_t* mod, const char* group )
{
  if( group == NULL )
    group = "_gc_basegroup";
  
  GnomeCanvasItem*  grp = stg_model_get_property( mod, group );
  
  if( grp )
    gtk_object_destroy( grp );
}




void gc_draw_polygons( GnomeCanvasGroup* grp,
		       stg_polygon_t* polys, size_t polycount,
		       stg_color_t color, int filled )
{

  int p;
  for( p=0; p<polycount; p++ )
    {
      int num_points = polys[p].points->len;
      
      if( num_points > 0 )
	{
	  GnomeCanvasPoints *gpts =  gnome_canvas_points_new( num_points );
	  
	  int q;
	  for( q=0; q<num_points; q++ )
	    {
	      stg_point_t* pt = &g_array_index( polys[p].points, stg_point_t, q );
	      gpts->coords[2*q] = pt->x;
	      gpts->coords[2*q+1] = pt->y;
	    }
	  
	  GnomeCanvasItem* it = 
	    gnome_canvas_item_new( grp,
				   gnome_canvas_polygon_get_type(),
				   "fill_color_rgba", filled ?  STG_TO_GNOME_COLOR(color) : 0,
				   "outline_color_rgba", filled ? 255 : STG_TO_GNOME_COLOR(color),
				   "width-pixels", 1,
				   "points", gpts,
				   NULL );

	  gnome_canvas_item_raise_to_top( it );
	  gnome_canvas_points_free( gpts );
	}
    }
}

/* void stg_model_draw_polylines( stg_model_t* mod, const char* figure,  */
/* 			       double x, double y, double a,  */
/* 			       stg_polyline_t* lines, size_t linecount, */
/* 			       double thickness, stg_color_t colot ); */

/* void stg_model_draw_points( stg_model_t* mod, const char* figure, */
/* 			    double x, double y, double a,  */
/* 			    stg_point_t* points, size_t point,  */
/* 			    double size, stg_color_t color ); */


int gc_model_init( stg_model_t* mod )
{
  GnomeCanvasGroup* parent_grp = mod->parent ? 
    stg_model_get_property( mod->parent, "_gc_basegroup" ) 
    : gnome_canvas_root( mod->world->win->gcanvas );
    
  GnomeCanvasItem* base = 
    gnome_canvas_item_new( parent_grp,
			   gnome_canvas_group_get_type(),
			   "x", mod->pose.x,
			   "y", mod->pose.y,
			   NULL );

  stg_model_set_property( mod, "_gc_basegroup", base );

  gnome_canvas_item_raise_to_top( base );

  stg_model_add_callback( mod, &mod->pose, gc_model_move, NULL );
  stg_model_add_callback( mod, &mod->polygons, gc_model_polygons, NULL );
  stg_model_add_callback( mod, &mod->color, gc_model_polygons, NULL );
  stg_model_add_callback( mod, &mod->gui_nose, gc_model_polygons, NULL );
  stg_model_add_callback( mod, &mod->gui_outline, gc_model_polygons, NULL );
  stg_model_add_callback( mod, &mod->gui_mask, gc_model_polygons, NULL );
  stg_model_add_callback( mod, &mod->parent, gc_model_polygons, NULL );
  stg_model_add_callback( mod, &mod->boundary, gc_model_polygons, NULL );

  //stg_model_add_callback( mod, &mod->gui_grid, gc_model_grid, NULL );

  stg_model_add_property_toggles( mod, 
				  &mod->gui_grid,
				  gc_model_grid, // called when toggled on
				  NULL,
				  gc_model_grid_clear, // called when toggled off
				  NULL,
				  "grid",
				  "grids",
				  TRUE );  


  return 0; //ok
}

int gc_laser_init( stg_model_t* mod )
{
  // adds a menu item and associated on-and-off callbacks
  stg_model_add_property_toggles( mod, 
				  &mod->data,
				  gc_laser_data, // called when toggled on
				  NULL,
				  gc_laser_data_clear, // called when toggled off
				  NULL,
				  "laserdata",
				  "laser scans",
				  TRUE );  

  return 0; // ok
}

int gc_ranger_init( stg_model_t* mod )
{
  // adds a menu item and associated on-and-off callbacks
  stg_model_add_property_toggles( mod, 
				  &mod->data,
				  gc_ranger_data, // called when toggled on
				  NULL,
				  gc_ranger_data_clear, // called when toggled off
				  NULL,
				  "rangerdata",
				  "ranger beams",
				  TRUE );  

  return 0; // ok
}
