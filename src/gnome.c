#include "config.h"

/* we only have these functions if GnomeCanvas was installed */
#if INCLUDE_GNOME

#include <libgnomecanvas/libgnomecanvas.h>
#include "stage_internal.h"
#include "stage.h"

// BEGIN MODEL

gint
background_event_callback(GnomeCanvasItem* item, 
			  GdkEvent* event, 
			  gpointer data)
{
  static int dragging = 0;
  static int zoom = 0;

  static double xstart=0.0, ystart=0.0;

  static int pixel_start_x, pixel_start_y;

  GdkCursor* cursor = NULL;
  double item_x = event->button.x;
  double item_y = event->button.y;

  stg_world_t* world = (stg_world_t*)data;

  switch (event->type) {
    case GDK_BUTTON_PRESS:
      printf( "press\n" );
      
      gnome_canvas_item_grab_focus( item );
      
      xstart = item_x;
      ystart = item_y;
      
      switch(event->button.button) 
	{
	case 1:	  
	  cursor = gdk_cursor_new(GDK_FLEUR);
	  dragging = TRUE;
	  break;

	case 3:
	  cursor = gdk_cursor_new(GDK_SIZING);
	  zoom = TRUE;
	  gtk_widget_get_pointer( world->win->gcanvas, 
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
	  GnomeCanvasItem* root = gnome_canvas_root(world->win->gcanvas);
	  gnome_canvas_item_w2i( root, 
				 &item_x, &item_y );

	  static double x=50.0, y=0;
	  //gnome_canvas_set_scroll_region( world->win->gcanvas, 
	  //			  x, y, 10.0, 10.0 );
	  

	  x += 1;
	  
          //gnome_canvas_item_move(root, item_x, item_y);

	  //static double scale= 50;
	  //gnome_canvas_
        }

      if( zoom && (event->motion.state & GDK_BUTTON3_MASK)) 
	{
	  int px, py;
	  gtk_widget_get_pointer(  world->win->gcanvas, &px, &py );

	  printf( "px: %d %d  \n", px, py );

	  int dx = px - pixel_start_x;

	  pixel_start_x = px;

	  printf( "px: %d %d   dx %d\n", px, py, dx );
	  
	  // cx, cy is the dimension of the whole canvas in pixels
	  int cx, cy;
	  gtk_layout_get_size( world->win->gcanvas, &cx, &cy );
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


gint model_event_callback(GnomeCanvasItem* item, 
			  GdkEvent* event, 
			  gpointer data)
{
  stg_model_t* mod = (stg_model_t*)data;
  
  static int dragging = 0;
  static int spinning = 0;
  
  GdkCursor* cursor= NULL;
  
  double item_x = event->button.x;
  double item_y = event->button.y;
  
  double start_item_x, start_item_y;
  
  static GnomeCanvasItem* hilite = NULL;
  
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
	
	stg_geom_t* geom = 
	  stg_model_get_property_fixed( (stg_model_t*)data, 
					"geom", sizeof(stg_geom_t));
	
	hilite = gnome_canvas_item_new( mod->grp,
					gnome_canvas_ellipse_get_type(),
					"x1", -geom->size.x/1.0,
					"y1", -geom->size.y/1.0,
					"x2", geom->size.x/1.0,
					"y2", geom->size.y/1.0,
					"outline_color", "red",
					"width-pixels", 3,
					"fill_color", 0,
					NULL );
	mod->world->win->selection_active = mod;     
      }
      
      
      break;
      
    case GDK_LEAVE_NOTIFY:
      puts( "leave" );
      
      gtk_object_destroy( hilite );
      
      // get rid of the selection info
      mod->world->win->selection_active = NULL;     
      
      guint cid = 
	gtk_statusbar_get_context_id( mod->world->win->status_bar, "on_mouse" );
      gtk_statusbar_pop( mod->world->win->status_bar, cid ); 
      
      break;
      
      
    case GDK_MOTION_NOTIFY:
      {
	if( dragging || spinning )
	  {
	    stg_pose_t* pose = 
	      stg_model_get_property_fixed( mod, "pose", sizeof(stg_pose_t));
	    
	    if (dragging && (event->motion.state & GDK_BUTTON1_MASK)) 
	      {
		//gnome_canvas_item_w2i( item, &item_x, &item_y );
		//gnome_canvas_item_move(item, item_x, item_y);
		
		pose->x = item_x;
		pose->y = -item_y;
	      }
	    
	    if (spinning && (event->motion.state & GDK_BUTTON3_MASK)) 
	      {
		double dx = start_item_x - event->button.x;	    	    
		pose->a += dx / 40.0;	    
	      }
	    
	    stg_model_set_property( mod, "pose", pose, sizeof(stg_pose_t) );
	  }
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

int model_render_polygons_gc(  stg_model_t* mod, char* name, 
			       void* data, size_t len, void* userp )
{
  //printf( "render polygons for model %s\n", mod->token );

  stg_color_t *cp = 
    stg_model_get_property_fixed( mod, "color", sizeof(stg_color_t));
  
  stg_color_t col = cp ? *cp : 0; // black unless we were given a color
  
  size_t count = len / sizeof(stg_polygon_t);
  stg_polygon_t* polys = (stg_polygon_t*)data;
  
  if( ! polys ) PRINT_WARN1( "no polys for model %s", mod->token );

  stg_geom_t geom;
  stg_model_get_geom(mod, &geom);
  
  stg_pose_t* pose = 
    stg_model_get_property_fixed( mod, "pose", sizeof(stg_pose_t));
  
  
  // fetch our group of gnome canvas polygons
  GnomeCanvasGroup* pgrp = NULL;
  
  void** pp = (void**)
    stg_model_get_property_fixed( mod, "model_gpolygroup", sizeof(void**) );	       		  
  if( pp )
    pgrp = *(GnomeCanvasGroup**)pp;
  
  // and destroy it - deleting all the drawing items inside the group
  if( pgrp )
    gtk_object_destroy( GTK_OBJECT(pgrp) );
  
  if( polys && len > 0 )
    {
      //printf( "rendering %d polygons\n", count );

      // create a new group
      pgrp = gnome_canvas_item_new( mod->grp,
				    gnome_canvas_group_get_type(),
				    NULL );
      
      stg_movemask_t* mask = 
	stg_model_get_property_fixed( mod, "mask", sizeof(stg_movemask_t));
      assert( mask );

      // if this is a top-level item and not marked immovable, we need
      // mouse events from it
      if( (! mod->parent) && (*mask != 0))
	g_signal_connect( mod->grp, "event", G_CALLBACK(model_event_callback), mod );
      
      // store this new group
      stg_model_set_property( mod, "model_gpolygroup", &pgrp, sizeof(void*));
      
      for( int p=0; p<count; p++ )
	{	  
	  //printf( "working on poly %d\n", p );
	  
	  if(  polys[p].points->len  )
	    {
	      GnomeCanvasPoints* gp = 
		gnome_canvas_points_new( polys[p].points->len );  
	      
	      //printf( "count %d\n",polys[p].points->len  );
	      for(int i=0; i<polys[p].points->len; i++ )
		{
		  stg_point_t pt = 
		    g_array_index( polys[p].points, stg_point_t, i );
		  
		  assert( ! isnan(pt.x));
		  assert( ! isnan(pt.y));

		  gp->coords[i*2] = pt.x;
		  gp->coords[i*2+1] = pt.y;
		  
		  //printf( "x:%.2f y:%.3f\n", gp->coords[i*2], gp->coords[i*2+1] );
		}
	      //puts("" );

	      //GnomeCanvasItem* gcitem =
		gnome_canvas_item_new( pgrp,
				       gnome_canvas_polygon_get_type(),
				       "points", gp,
				       "fill_color_rgba", (col << 8) + 0xFF,
				       "outline_color", "black",
				       "width-pixels", 1,
				       NULL );	      
  	      
	      // allow for local pose
	      double local_trans[6], local_rot[6];
	      art_affine_translate( local_trans, geom.pose.x, geom.pose.y );
	      art_affine_rotate( local_rot, RTOD(geom.pose.a) );
	      art_affine_multiply( local_trans, local_trans, local_rot );
	      gnome_canvas_item_affine_relative( pgrp, local_trans );
	    }
	}
    }
  
  stg_bool_t* boundary = 
    stg_model_get_property_fixed( mod, "boundary", sizeof(stg_bool_t));  
  if( boundary && *boundary )
    {      
      gnome_canvas_item_new( pgrp,
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

  int* nose = 
    stg_model_get_property_fixed( mod, "nose", sizeof(int) );
  if( nose && *nose )
    {       
      // TODO
    }
  
  return 0;
}
// END MODEL

// BEGIN LASER

int laser_render_data_gc( stg_model_t* mod, char* name, 
			  void* data, size_t len, void* userp )
{  
  stg_laser_config_t *cfg = 
    stg_model_get_property_fixed( mod, "laser_cfg", sizeof(stg_laser_config_t));
  assert( cfg );
  
  
  stg_laser_sample_t* samples = (stg_laser_sample_t*)data; 
  size_t sample_count = len / sizeof(stg_laser_sample_t);
  
  GnomeCanvasPolygon* gp = NULL;
  void** gpp = stg_model_get_property_fixed( mod, "laser_data_gpolygon", sizeof(void**));
  if( gpp )
    gp = *(GnomeCanvasPolygon**)gpp;
  
  // create a poly and a points structure if they don't exist already
  if( ! gp )
    {
      gp = gnome_canvas_item_new( mod->grp,
				  gnome_canvas_polygon_get_type(),
				  "fill_color_rgba", GNOME_CANVAS_COLOR_A(0,0,255,20),
				  "outline_color", "blue",
				  "width-pixels", 1,
				  NULL );
      gnome_canvas_item_lower_to_bottom( gp );

      stg_model_set_property( mod, "laser_data_gpolygon", &gp, sizeof(gp));
    }
  
  
  GnomeCanvasPoints *gpts =  gnome_canvas_points_new( sample_count );
	    
  if( samples && sample_count )
    {
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
	  gpts->coords[2*s] = (samples[s].range/1000.0) * cos(bearing);
	  gpts->coords[2*s+1] = (samples[s].range/1000.0) * sin(bearing);
	  bearing += sample_incr;
	}      
    }
  
  // install the points structure (which may contain zero points)
  gnome_canvas_item_set( gp, "points", gpts, NULL );
  
  return 0; // callback runs until removed
}

int laser_unrender_data_gc( stg_model_t* mod, char* name, 
			 void* data, size_t len, void* userp )
{
  GnomeCanvasPolygon* gp = NULL;
  void** gpp = stg_model_get_property_fixed( mod, "laser_data_gpolygon", sizeof(void**));
  if( gpp )
    gp = *(GnomeCanvasPolygon**)gpp;

  gtk_object_destroy( gp );

  stg_model_set_property( mod, "laser_data_gpolygon", NULL, 0 );

  return 1; // callback just runs one time
}

// END LASER

#endif
