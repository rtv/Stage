/*
 *  Stage : a multi-robot simulator.
 *  Copyright (C) 2001, 2002 Richard Vaughan, Andrew Howard and Brian Gerkey.
 *
 *  This program is free software; you can redistributing it and/or modify
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
 * Desc: The RTK gui implementation
 * Author: Richard Vaughan, Andrew Howard
 * Date: 7 Dec 2000
 * CVS info: $Id: rtkgui.cc,v 1.13 2003-08-27 02:07:05 rtv Exp $
 */

#if HAVE_CONFIG_H
#  include <config.h>
#endif

// this should go when I get the autoconf set up properly
#ifdef INCLUDE_RTK2

//#define DEBUG 
//#define VERBOSE
#undef DEBUG
#undef VERBOSE

#include <errno.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <signal.h>
#include <math.h>
#include <unistd.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pwd.h>
#include <stdio.h>
#include <netdb.h>

#include <fstream>
#include <iostream>

#include "rtk.h"
#include "matrix.hh"
#include "rtkgui.hh"
#include "colors.h"

extern int quit;
extern GArray* global_client_pids;

// defaults

#define STG_CANVAS_RENDER_INTERVAL 100

#define STG_DEFAULT_WINDOW_WIDTH 600
#define STG_DEFAULT_WINDOW_HEIGHT 600
#define STG_DEFAULT_WINDOW_XORIGIN (STG_DEFAULT_WINDOW_WIDTH/2)
#define STG_DEFAULT_WINDOW_YORIGIN (STG_DEFAULT_WINDOW_HEIGHT/2)
#define STG_DEFAULT_PPM 40
#define STG_DEFAULT_SHOWGRID 1

#define STG_MENUITEM_VIEW_MATRIX  1
#define STG_MENUITEM_VIEW_BODY  2
#define STG_MENUITEM_VIEW_DATA  3

#define STG_LAYER_BACKGROUND 40
#define STG_LAYER_BODY 50
#define STG_LAYER_DATA 60
#define STG_LAYER_LIGHTS 70
#define STG_LAYER_SENSORS 61

bool enable_matrix = FALSE;
bool enable_data = TRUE;

// single static application visible to all funcs in this file
static rtk_app_t *app = NULL; 

// these are internal and can only be called in rtkgui.cc
//static void RtkMenuHandling();
//static void RtkUpdateMovieMenu();


void RtkOnMouse(rtk_fig_t *fig, int event, int mode);



// allow the GUI to do any startup it needs to, including cmdline parsing
int stg_gui_init( int* argc, char*** argv )
{ 
  //PRINT_DEBUG( "init_func" );
  rtk_init(argc, argv);

  app = rtk_app_create();

  rtk_app_main_init( app );
  return 0; // OK
}

// useful debug function allows plotting the world externally
gboolean stg_gui_matrix_render( gpointer data )
{
  rtk_fig_t* fig = (rtk_fig_t*)data;
  CMatrix* matrix = (CMatrix*)fig->userdata;
  
  rtk_fig_clear( fig );
    
  double pixel_size = 1.0 / matrix->ppm;
  
  // render every pixel as an unfilled rectangle
  for( int y=0; y < matrix->height; y++ )
    for( int x=0; x < matrix->width; x++ )
      if( *(matrix->get_cell( x, y )) )
	rtk_fig_rectangle( fig, 
			   x*pixel_size, y*pixel_size, 0, 
			   pixel_size, pixel_size, 0 );

  return TRUE; // keep calling me
}

// shows or hides a figure stashed in the menu item's userdata
void stg_gui_menu_fig_toggle_show( rtk_menuitem_t *item )
{
  rtk_fig_show( (rtk_fig_t*)item->userdata, item->checked );
}

// inverts the show state of a figure - often used as a callback
gboolean stg_gui_fig_toggle_show( void* fig )
{
  rtk_fig_show( (rtk_fig_t*)fig, !((rtk_fig_t*)fig)->show );
  return TRUE;
}

// shows or hides all the figure bodies in a window
void stg_gui_menu_toggle_bodies( rtk_menuitem_t *item )
{
  stg_gui_window_t* win = (stg_gui_window_t*)item->userdata;  
  rtk_canvas_layer_show( win->canvas, STG_LAYER_BODY, 
			 !win->canvas->layer_show[STG_LAYER_BODY] );
}

// shows or hides all the figure bodies in a window
void stg_gui_menu_toggle_sensors( rtk_menuitem_t *item )
{
  stg_gui_window_t* win = (stg_gui_window_t*)item->userdata;  
  rtk_canvas_layer_show( win->canvas, STG_LAYER_SENSORS, 
			 !win->canvas->layer_show[STG_LAYER_SENSORS] );
}

// shows or hides all the lights in a window
void stg_gui_menu_toggle_lights( rtk_menuitem_t *item )
{
  stg_gui_window_t* win = (stg_gui_window_t*)item->userdata;  
  rtk_canvas_layer_show( win->canvas, STG_LAYER_LIGHTS, 
			 !win->canvas->layer_show[STG_LAYER_LIGHTS] );
}

// shows or hides all the figure bodies in a window
void stg_gui_menu_toggle_data( rtk_menuitem_t *item )
{
  enable_data = rtk_menuitem_ischecked(item);
}

// send a USR2 signal to all clients
void stg_gui_save( rtk_menuitem_t *item )
{
  for( int p=0; p<global_client_pids->len; p++ )
    kill( g_array_index( global_client_pids, pid_t, p ), SIGUSR2 );
}

void stg_gui_exit( rtk_menuitem_t *item )
{
  //quit = TRUE;
  puts( "Exit menu item. Destroying world" );

  stg_world_destroy( (stg_world_t*)item->userdata );
}

// build a simulation window
stg_gui_window_t* stg_gui_window_create( stg_world_t* world, int width, int height )
{
  stg_gui_window_t* win = (stg_gui_window_t*)calloc( 1, sizeof(stg_gui_window_t) );
 
  if( width < 1 ) width = STG_DEFAULT_WINDOW_WIDTH;
  if( height < 1 ) height = STG_DEFAULT_WINDOW_HEIGHT;
 
  win->canvas = rtk_canvas_create(app);
      
  // Add some menu items 
  win->file_menu = rtk_menu_create(win->canvas, "File");
  win->save_menuitem = rtk_menuitem_create(win->file_menu, "Save", 0);
  rtk_menuitem_set_callback( win->save_menuitem, stg_gui_save );

  win->exit_menuitem = rtk_menuitem_create(win->file_menu, "Exit", 0);
  win->exit_menuitem->userdata = (void*)world;
  rtk_menuitem_set_callback( win->exit_menuitem, stg_gui_exit );

  /*
  win->stills_menu = rtk_menu_create_sub(win->file_menu, "Capture stills");
  win->movie_menu = rtk_menu_create_sub(win->file_menu, "Capture movie");
  
  win->stills_jpeg_menuitem = rtk_menuitem_create(win->stills_menu, "JPEG format", 1);
  win->stills_ppm_menuitem = rtk_menuitem_create(win->stills_menu, "PPM format", 1);
  win->stills_series = 0;
  win->stills_count = 0;
  
  win->movie_options[0].menuitem = rtk_menuitem_create(win->movie_menu, "Speed x1", 1);
  win->movie_options[0].speed = 1;
  win->movie_options[1].menuitem = rtk_menuitem_create(win->movie_menu, "Speed x2", 1);
  win->movie_options[1].speed = 2;
  win->movie_options[2].menuitem = rtk_menuitem_create(win->movie_menu, "Speed x5", 1);
  win->movie_options[2].speed = 5;
  win->movie_options[3].menuitem = rtk_menuitem_create(win->movie_menu, "Speed x10", 1);
  win->movie_options[3].speed = 10;
  win->movie_option_count = 4;
  */

  win->movie_count = 0;
  
  // Create the view menu
  win->view_menu = rtk_menu_create(win->canvas, "View");

  // grid item
  win->grid_item = rtk_menuitem_create(win->view_menu, "Grid", 1);
  win->fig_grid = stg_gui_grid_create( win->canvas, NULL, 
				       world->width/2.0,world->height/2.0,0, 
				       world->width, world->height, 1.0, 0.1 );
  win->grid_item->userdata = (void*)win->fig_grid;
  rtk_fig_show( win->fig_grid, 1 );
  rtk_menuitem_set_callback( win->grid_item, stg_gui_menu_fig_toggle_show );
  rtk_menuitem_check(win->grid_item, 1);
  

  // matrix item
  win->fig_matrix = rtk_fig_create(win->canvas, NULL, 60);
  win->fig_matrix->userdata = (void*)world->matrix;
  win->matrix_item = rtk_menuitem_create(win->view_menu, "Matrix", 1);
  win->matrix_item->userdata = (void*)win->fig_matrix;
  rtk_fig_color_rgb32(win->fig_matrix, stg_lookup_color(STG_MATRIX_COLOR) );
  rtk_fig_show( win->fig_matrix, 0 );
  rtk_menuitem_set_callback( win->matrix_item, stg_gui_menu_fig_toggle_show );
  rtk_menuitem_check(win->matrix_item, 0);


  // objects item
  win->objects_item = rtk_menuitem_create(win->view_menu, "Objects", 1);
  rtk_menuitem_check(win->objects_item, 1);
  win->objects_item->userdata = (void*)win;
  // the callback shows/hides all entity body figs in the window
  rtk_menuitem_set_callback( win->objects_item, stg_gui_menu_toggle_bodies );
  
  // data item
  win->data_item = rtk_menuitem_create(win->view_menu, "Data", 1);  
  rtk_menuitem_check(win->data_item, 1);
  rtk_menuitem_set_callback( win->data_item, stg_gui_menu_toggle_data );
 
  // lights item
  win->lights_item = rtk_menuitem_create(win->view_menu, "Lights", 1);  
  win->lights_item->userdata = (void*)win;
  rtk_menuitem_check(win->lights_item, 1);
  // the callback shows/hides all light figs in the window
  rtk_menuitem_set_callback( win->lights_item, stg_gui_menu_toggle_lights );

  // sensors item
  win->sensors_item = rtk_menuitem_create(win->view_menu, "Sensors", 1);  
  win->sensors_item->userdata = (void*)win;
  rtk_menuitem_check(win->sensors_item, 0);
  rtk_menuitem_set_callback( win->sensors_item, stg_gui_menu_toggle_sensors );
  
  // sensors start out hidden - they'ye visual clutter
  rtk_canvas_layer_show( win->canvas, STG_LAYER_SENSORS, 0 ); 

  // create the action menu
  /*
    win->action_menu = rtk_menu_create(win->canvas, "Action");
  */

  win->statusbar = GTK_STATUSBAR(gtk_statusbar_new());
  
  gtk_box_pack_start(GTK_BOX(win->canvas->layout), 
		     GTK_WIDGET(win->statusbar), FALSE, TRUE, 0);
  
  /* BROKEN
  //zero the view menus
  memset( &device_menu,0,sizeof(stg_menu_t));
  memset( &data_menu,0,sizeof(stg_menu_t));
  
  // Create the view/device sub menu
  assert( data_menu.menu = 
  rtk_menu_create_sub(view_menu, "Data"));
  
  // Create the view/data sub menu
  assert( device_menu.menu = 
  rtk_menu_create_sub(view_menu, "Object"));
  
  // each device adds itself to the correct view menus in its rtkstartup()
  */

  // update this window every few ms
  win->source_tag = g_timeout_add( STG_CANVAS_RENDER_INTERVAL, 
				   stg_gui_window_callback, win );
  
  // TODO - fix this behavior in rtk - I have to fall back on GTK here.
  PRINT_DEBUG2( "resizing window to %d %d", width, height );
  //rtk_canvas_size( mod->win->canvas, 700, 700 );   
  gtk_window_resize( GTK_WINDOW(win->canvas->frame), width, height ); 
  
  rtk_canvas_origin( win->canvas, world->width/2.0, world->height/2.0 );
  rtk_canvas_scale( win->canvas, 1.1 * world->width/width, 1.1 * world->width / width );

  GString* titlestr = g_string_new( "Stage: " );
  g_string_append_printf( titlestr, "%s", 
			  world->name->str );

  rtk_canvas_title( win->canvas, titlestr->str );
  g_string_free( titlestr, TRUE );

  win->world = world;

  // show the window
  gtk_widget_show_all(win->canvas->frame);
  
  // return the completed window
  return win;
}


// this gets called periodically from a callback set up in
// stg_gui_window_create
gboolean stg_gui_window_callback( gpointer data )
{
  stg_gui_window_t* win = (stg_gui_window_t*)data;

  // handle menus and all that guff
  
  if( win->fig_matrix->show )
    stg_gui_matrix_render( win->fig_matrix );

  rtk_canvas_flash_update( win->canvas );

  // redraw anything that needs it
  rtk_canvas_render( win->canvas );

  return TRUE;
}

int stg_gui_window_update( stg_world_t* world, stg_prop_id_t prop )
{
  PRINT_WARN( "" );  
  return 0;
}

void stg_gui_window_destroy( stg_gui_window_t* win )
{
  if( win == NULL )
    {
      PRINT_WARN( "requested destruction of NULL window" );
      return;
    }
  
  // cancel the callback for this window
  g_source_remove( win->source_tag );
  
  if( win->fig_grid ) rtk_fig_destroy( win->fig_grid );
  if( win->fig_matrix ) rtk_fig_destroy( win->fig_matrix );
  
  // must delete the canvas after the figures
  rtk_canvas_destroy( win->canvas );
  free( win );
}

rtk_fig_t* stg_gui_grid_create( rtk_canvas_t* canvas, rtk_fig_t* parent, 
				double origin_x, double origin_y, double origin_a, 
				double width, double height, double major, double minor )
{
  rtk_fig_t* grid = NULL;

  assert( (grid = rtk_fig_create( canvas, parent, 
				     STG_LAYER_BACKGROUND )) );
  
  if( minor > 0)
    {
      rtk_fig_color_rgb32( grid, stg_lookup_color(STG_GRID_MINOR_COLOR ) );
      rtk_fig_grid( grid, origin_x, origin_y, width, height, minor);
    }
  if( major > 0)
    {
      rtk_fig_color_rgb32( grid, stg_lookup_color(STG_GRID_MAJOR_COLOR ) );
      rtk_fig_grid( grid, origin_x, origin_y, width, height, major);
    }
  
  return grid;
}


void stg_gui_model_blinkenlight( stg_gui_model_t* mod )
{
  g_assert( mod );
  g_assert( mod->fig );
  g_assert( mod->fig->canvas );
  g_assert( mod->ent );

  rtk_canvas_t* canvas = mod->fig->canvas;
  
  if( mod->fig_light )
    {
      rtk_fig_destroy( mod->fig_light );
      mod->fig_light = NULL;
    }
  
  // nothing to see here
  if( mod->ent->blinkenlight == STG_LIGHT_OFF )
    return;
  
  mod->fig_light = rtk_fig_create( canvas, mod->fig, STG_LAYER_LIGHTS);  
  rtk_fig_color_rgb32( mod->fig_light, mod->ent->color ); 
  rtk_fig_ellipse( mod->fig_light, 0,0,0, 
		   mod->ent->size.x,  
		   mod->ent->size.y, 1 );
  
  // start the light blinking if it's not stuck on
  if( mod->ent->blinkenlight != STG_LIGHT_ON )    
    rtk_fig_blink( mod->fig_light, mod->ent->blinkenlight, 1 );  
}


// Initialise the GUI
stg_gui_model_t* stg_gui_model_create(  CEntity* ent )
{
  g_assert( ent );

  PRINT_DEBUG2( "creating a stg_gui_model_t for ent %d (%s)", 
		ent->id, ent->name->str );
  
  stg_gui_window_t* win = ent->GetWorld()->win;

  g_assert( win );
  g_assert( win->canvas );

  stg_gui_model_t* mod = (stg_gui_model_t*)calloc( 1,sizeof(stg_gui_model_t));
  
  mod->ent = ent;
  mod->win = win; // use the world window

  
  rtk_fig_t* parent_fig = NULL;
  
  // we can only manipulate top-level figures
  if( g_node_depth( ent->node ) == 2 )
    {
      if( ent->mouseable )
	mod->movemask = RTK_MOVE_TRANS | RTK_MOVE_ROT;
      else
	mod->movemask = 0;
      
      parent_fig = NULL;
    }
  else if( g_node_depth( ent->node ) > 2 )
    {
      parent_fig = stg_ent_parent(ent)->guimod->fig;      
      mod->movemask = 0;
    }

  mod->grid_enable = false;
  mod->grid_major = 1.0;
  mod->grid_minor = 0.2;
  
  // Create a figure representing this entity
  // rtk head
  //mod->fig = rtk_fig_create_ex( mod->win->canvas, parent_fig, 
  //			STG_LAYER_BODY, ent);
  
  // rtk 2.2
  mod->fig = rtk_fig_create( mod->win->canvas, parent_fig, 
			      STG_LAYER_BODY );
  mod->fig->userdata = (void*)ent;
  // end rtk 2.2

  assert( mod->fig );
  
  // put the figure's origin at the entity's position
  stg_pose_t pose;
  ent->GetPose( &pose );
  rtk_fig_origin( mod->fig, pose.x, pose.y, pose.a );

  // figure gets the same movemask as the model
  rtk_fig_movemask(mod->fig, mod->movemask);  

  // Set the mouse handler
  rtk_fig_add_mouse_handler(mod->fig, RtkOnMouse);

  // visible by default
  rtk_fig_show( mod->fig, true );

  // Set the color
  rtk_fig_color_rgb32( mod->fig, ent->color);
  
#ifdef RENDER_INITIAL_BOUNDING_BOXES
  double xmin, ymin, xmax, ymax;
  xmin = ymin = 999999.9;
  xmax = ymax = 0.0;
  ent->GetBoundingBox( xmin, ymin, xmax, ymax );
  
  //rtk_fig_t* boundaries = rtk_fig_create_ex( mod->win->canvas, NULL, 99, ent);
  rtk_fig_t* boundaries = rtk_fig_create( mod->win->canvas, NULL, 99 );
  boundaries->userdata = (void*)ent;

  double width = xmax - xmin;
  double height = ymax - ymin;
  double xcenter = xmin + width/2.0;
  double ycenter = ymin + height/2.0;

  rtk_fig_rectangle( boundaries, xcenter, ycenter, 0, width, height, 0 ); 

   
#endif
   
          
  if( mod->grid_enable )
    {
      mod->fig_grid = stg_gui_grid_create( mod->win->canvas, mod->fig, 0,0,0, 
					   ent->size.x, ent->size.y, 1.0, 0.1 );
      rtk_fig_show( mod->fig_grid, 1);
    }
  else
    mod->fig_grid = NULL;
  

  PRINT_DEBUG1( "rendering %d rectangles", ent->GetNumRects() );
  
  // now create GUI rects
  int r;
  int rmax = ent->GetNumRects();
  for( r=0; r<rmax; r++ )
    {
      stg_rotrect_t* src = ent->GetRect(r);
      double x,y,a,w,h;
      
      x = ((src->x + src->w/2.0) * ent->size.x) - ent->size.x/2.0 + ent->pose_origin.x;
      y = ((src->y + src->h/2.0) * ent->size.y) - ent->size.y/2.0 + ent->pose_origin.y;
      a = src->a;
      w = src->w * ent->size.x;
      h = src->h * ent->size.y;
      
      //if( ent->m_parent_entity == NULL )
      //rtk_fig_rectangle(mod->fig, x,y,a,w,h, 1 ); 
      //else
      rtk_fig_rectangle(mod->fig, x,y,a,w,h, 0 ); 
    }
  


  // add a nose-line to position models
  if( ent->draw_nose )
    {
      stg_pose_t origin;
      ent->GetOrigin( &origin );
      stg_size_t size;
      ent->GetSize( &size );
      rtk_fig_line( mod->fig, origin.x, origin.y, size.x/2.0, origin.y );
    }

  // add rects showing ranger positions
  if( ent->rangers )
    {
      mod->fig_sensors = 
	rtk_fig_create( mod->win->canvas, mod->fig, STG_LAYER_SENSORS );
      
      for( int s=0; s< (int)ent->rangers->len; s++ )
	{
	  stg_ranger_t* rngr = &g_array_index( ent->rangers, 
					       stg_ranger_t, s );
	  
	  //printf( "drawing a ranger rect (%.2f,%.2f,%.2f)[%.2f %.2f]\n",
	  //  rngr->pose.x, rngr->pose.y, rngr->pose.a,
	  //  rngr->size.x, rngr->size.y );
	  
	  rtk_fig_rectangle( mod->fig_sensors, 
			     rngr->pose.x, rngr->pose.y, rngr->pose.a,
			     rngr->size.x, rngr->size.y, 0 ); 
	  
	  // show the FOV too
	  double sidelen = rngr->size.x/2.0;
	  
	  double x1= rngr->pose.x + sidelen*cos(rngr->pose.a - rngr->fov/2.0 );
	  double y1= rngr->pose.y + sidelen*sin(rngr->pose.a - rngr->fov/2.0 );
	  double x2= rngr->pose.x + sidelen*cos(rngr->pose.a + rngr->fov/2.0 );
	  double y2= rngr->pose.y + sidelen*sin(rngr->pose.a + rngr->fov/2.0 );
	  
	  rtk_fig_line(mod->fig_sensors, rngr->pose.x, rngr->pose.y, x1, y1 );
	  rtk_fig_line(mod->fig_sensors, rngr->pose.x, rngr->pose.y, x2, y2 );
	}
    }
  
  // conditionally add blinking bits
  stg_gui_model_blinkenlight( mod );

  // if we created a window for this model, scale the window to fit
  // nicely around the figure and shift the origin to the bottom left
  // corner

  return mod;
}

void stg_gui_model_destroy( stg_gui_model_t* mod )
{
  if( mod->fig_rects ) rtk_fig_destroy( mod->fig_rects );
  if( mod->fig_user ) rtk_fig_destroy( mod->fig_user );
  if( mod->fig_light ) rtk_fig_destroy( mod->fig_light );
  if( mod->fig_sensors ) rtk_fig_destroy( mod->fig_sensors );
  if( mod->fig ) rtk_fig_destroy( mod->fig );
}

rtk_fig_t* stg_gui_label_create( rtk_canvas_t* canvas, CEntity* ent )
{
  //rtk_fig_t* label = rtk_fig_create_ex( canvas, NULL, 51, ent);
  rtk_fig_t* label = rtk_fig_create( canvas, NULL, 51 );
  label->userdata = (void*)ent;
  rtk_fig_show(label, 1); 
  rtk_fig_movemask(label, 0);
      
  char labelstr[1024];
  
  snprintf(labelstr, sizeof(labelstr), "(%d) %s", 
	     ent->id, ent->name->str );
    
  //rtk_fig_color_rgb32(mod->fig, ent->color);
  //rtk_fig_origin( mod->fig_label, 0.05, 1.0, 0 );
  rtk_fig_text(label, 0,0,0, labelstr);
  
  // attach the label to the main figure
  // rtk will draw the label when the mouse goes over the figure
  // TODO: FIX
  //mod->fig->mouseover_fig = fig_label;
  
  return label;
}

///////////////////////////////////////////////////////////////////////////
// Process mouse events 
void RtkOnMouse(rtk_fig_t *fig, int event, int mode)
{
  //PRINT_DEBUG2( "ON MOUSE CALLED BACK for %p with userdata %p", fig, fig->userdata );

  // each fig links back to the Entity that owns it
  CEntity *entity = (CEntity*) fig->userdata;
  assert( entity );

  // the entity stores a set of figures and display options here
  stg_gui_model_t* mod = entity->guimod;
  assert( mod );
  
  //static rtk_fig_t* fig_pose = NULL;

  //double px, py, pth;
  stg_pose_t pose;
  guint cid; // statusbar context id
  char txt[100];

  switch (event)
    {
    case RTK_EVENT_PRESS:
      // DELIBERATE NO-BREAK      
    case RTK_EVENT_MOTION:

      // if there are more motion events pending, do nothing.

      // move the object to follow the mouse
      rtk_fig_get_origin(fig, &pose.x, &pose.y, &pose.a );
      entity->SetProperty( STG_PROP_POSE, &pose, sizeof(pose) );
      
      // display the pose
      snprintf(txt, sizeof(txt), "Selection: %d:%s pose: [%.2f,%.2f,%.2f]",  
	       entity->id, entity->name->str,
	       pose.x,pose.y,pose.a  );

      cid = gtk_statusbar_get_context_id( mod->win->statusbar, "on_mouse" );
      gtk_statusbar_pop( mod->win->statusbar, cid ); 
      gtk_statusbar_push( mod->win->statusbar, cid, txt ); 
      
      break;
      
    case RTK_EVENT_RELEASE:
      // move the entity to its final position
      rtk_fig_get_origin(fig, &pose.x, &pose.y, &pose.a );
      entity->SetProperty( STG_PROP_POSE, &pose, sizeof(pose) );

      // take the pose message from the status bar
      cid = gtk_statusbar_get_context_id( mod->win->statusbar, "on_mouse" );
      gtk_statusbar_pop( mod->win->statusbar, cid ); 
      break;
      
      
    default:
      break;
    }

  return;
}
void stg_gui_neighbor_render( CEntity* ent, GArray* neighbors )
{
  // if data drawing is disabled, do nothing
  if( !enable_data )
    return;

  stg_gui_model_t* model = ent->guimod;
  rtk_canvas_t* canvas = model->fig->canvas;

  rtk_fig_t* fig = rtk_fig_create( canvas, NULL, STG_LAYER_DATA );
  rtk_fig_color_rgb32( fig, stg_lookup_color( STG_FIDUCIAL_COLOR ) );

  stg_pose_t pz;
  ent->GetGlobalPose( &pz );
  
  rtk_fig_origin( fig, pz.x, pz.y, pz.a );
  
  char text[65];
  
  for( int n=0; n < (int)neighbors->len; n++ )
    {
      stg_neighbor_t* nbor = &g_array_index( neighbors, stg_neighbor_t, n);
      
      double px = nbor->range * cos(nbor->bearing);
      double py = nbor->range * sin(nbor->bearing);
      double pa = nbor->orientation;
      
      rtk_fig_line( fig, 0, 0, px, py );	
      
      double wx = nbor->size.x;
      double wy = nbor->size.y;
      
      //rtk_fig_rectangle( fig, px, py, pa, wx, wy, 0);
      rtk_fig_line( fig, px+wx/2.0, py+wy/2.0, px-wx/2.0, py-wy/2.0 );
      rtk_fig_line( fig, px+wx/2.0, py-wy/2.0, px-wx/2.0, py+wy/2.0 );
      rtk_fig_arrow( fig, px, py, pa, wy, 0.10);
      
      snprintf(text, 64, "  %d", nbor->id );
      rtk_fig_text( fig, px, py, pa, text);
    }

  rtk_canvas_flash( canvas, fig, 2, 1 );
}

// render the entity's laser data
void stg_gui_laser_render( CEntity* ent )
{
  // if data drawing is disabled, do nothing
  if( !enable_data  )
    return;

  stg_gui_model_t* model = ent->guimod;
  rtk_canvas_t* canvas = model->fig->canvas;
  stg_laser_data_t* laser = &ent->laser_data;
  
  rtk_fig_t *fig = rtk_fig_create( canvas, NULL, STG_LAYER_DATA );
  
  rtk_fig_color_rgb32( fig, stg_lookup_color( "light blue" ) ); 

  stg_pose_t pz;
  memcpy( &pz, &laser->pose, sizeof(stg_pose_t) );
  ent->LocalToGlobal( &pz );
  rtk_fig_origin( fig, pz.x, pz.y, pz.a );
  
  double bearing = laser->angle_min;
  double incr = (laser->angle_max-laser->angle_min)/(double)laser->sample_count;
  double lx = 0.0;
  double ly = 0.0;
    
  for( int i=0; i < laser->sample_count; i++ )
    {
      // get range, converting from mm to m
      double range = laser->samples[i].range;
      
      bearing += incr;

      double px = range * cos(bearing);
      double py = range * sin(bearing);
      
      //rtk_fig_line(this->scan_fig, 0.0, 0.0, px, py);
      
      rtk_fig_line( fig, lx, ly, px, py );
      lx = px;
      ly = py;
      
      // add little boxes at high intensities (like in playerv)
      if(  laser->samples[i].reflectance > 0 )
	rtk_fig_rectangle( fig, px, py, 0, 0.02, 0.02, 1);
      
    }
  
  rtk_fig_line( fig, 0.0, 0.0, lx, ly );
  rtk_canvas_flash( canvas, fig, 2, 1 );
}

// render the entity's rangers
void stg_gui_rangers_render( CEntity* ent )
{
   stg_gui_model_t* model = ent->guimod;
   rtk_canvas_t* canvas = model->fig->canvas;

   rtk_fig_t* fig = rtk_fig_create( canvas, NULL, STG_LAYER_DATA );
   
   rtk_fig_color_rgb32( fig, stg_lookup_color(STG_SONAR_COLOR) );

   for( int t=0; t < (int)ent->rangers->len; t++ )
     {
       stg_ranger_t* tran = &g_array_index( ent->rangers, 
						stg_ranger_t, t );
       stg_pose_t pz;
       memcpy( &pz, &tran->pose, sizeof(stg_pose_t) );
       ent->LocalToGlobal( &pz );       
       
       rtk_fig_arrow( fig, pz.x, pz.y, pz.a, tran->range, 0.05 );
       
       // this code renders triangles indicating the beam width
       /*
	 double hitx =  pz.x + tran->range * cos(pz.a); 
       double hity =  pz.y + tran->range * sin(pz.a);       
       double sidelen = tran->range / cos(tran->fov/2.0);
       
       double x1 = pz.x + sidelen * cos( pz.a - tran->fov/2.0 );
       double y1 = pz.y + sidelen * sin( pz.a - tran->fov/2.0 );
       double x2 = pz.x + sidelen * cos( pz.a + tran->fov/2.0 );
       double y2 = pz.y + sidelen * sin( pz.a + tran->fov/2.0 );

       rtk_fig_line( fig, pz.x, pz.y, x1, y1 );
       rtk_fig_line( fig, pz.x, pz.y, x2, y2 );
       rtk_fig_line( fig, x1, y1, x2, y2 );
       */

       // this code renders the ranger's body
       //rtk_fig_rectangle( fig, pz.x, pz.y, pz.a, 
       //	  tran->size.x, tran->size.y, 1 );
     }

   rtk_canvas_flash( canvas, fig, 2, 1 );
}

int stg_gui_los_msg_send( CEntity* ent, stg_los_msg_t* msg )
{
  rtk_canvas_t* canvas =  ent->guimod->fig->canvas;
  rtk_fig_t* fig = rtk_fig_create( canvas, ent->guimod->fig, STG_LAYER_DATA );
  
  // array is zeroed with space for a null terminator
  char* buf = (char*)calloc(msg->len+1, 1 );
  memcpy( buf, &msg->bytes, msg->len );
  
  rtk_fig_color_rgb32( fig, stg_lookup_color( "magenta" ) );

  // indicate broadcast with a circle around the sender
  if( msg->id == -1 )
      rtk_fig_ellipse( fig, 0,0,0, 0.5, 0.5, 0 );
  
  // draw the message at the sender
  rtk_fig_text( fig, 0,0,0, buf );
  rtk_canvas_flash( canvas, fig, 4, 1 );

  free( buf );
  return 0; // ok
}

int stg_gui_los_msg_recv( CEntity* receiver, CEntity* sender, 
			  stg_los_msg_t* msg )
{
  rtk_canvas_t* canvas =  sender->guimod->fig->canvas;
  rtk_fig_t* fig = rtk_fig_create( canvas, NULL, STG_LAYER_DATA );

  stg_pose_t ps, pr;
  sender->GetGlobalPose( &ps );
  receiver->GetGlobalPose( &pr );
  
  rtk_fig_color_rgb32( fig, stg_lookup_color( "magenta" ) );

  rtk_fig_arrow_ex( fig, ps.x, ps.y, pr.x, pr.y, 0.1 );

  // array is zeroed with space for a null terminator
  char* buf = (char*)calloc(msg->len+1, 1 );
  memcpy( buf, &msg->bytes, msg->len );
  
  rtk_fig_text( fig, pr.x, pr.y,0, buf );
  rtk_canvas_flash( canvas, fig, 4, 1 );

  free( buf );
  return 0; //ok
}

int stg_gui_model_update( CEntity* ent, stg_prop_id_t prop )
{
  PRINT_DEBUG3( "gui update for %d:%s prop %s", 
		ent->id, ent->name->str, stg_property_string(prop) );
  
  assert( ent );
  assert( prop > 0 );
  assert( prop < STG_PROPERTY_COUNT );
    
  stg_gui_model_t* model = ent->guimod;

  switch( prop )
    {
      // these require just moving the figure
    case STG_PROP_POSE:
      {
	stg_pose_t pose;
	ent->GetPose( &pose );
	//PRINT_DEBUG3( "moving figure to %.2f %.2f %.2f", px,py,pa );
	rtk_fig_origin( model->fig, pose.x, pose.y, pose.a );

	// if things get too laggy you can remove this render and the
	// canvas will be redrawn in a few ms. mousing feels MUCH more
	// smooth with this here, though, at least with a small
	// population.

	// if the matrix is not visible, and we're haven't got a lot
	// of rectangles, we can probably afford to redraw things here	
	if( !ent->GetWorld()->win->fig_matrix->show )
	  //&& ent->GetNumRects() < 30 )
	  rtk_canvas_render( model->fig->canvas );
      }
      break;

      // these all need us to totally redraw the object 
      // (for now - this could be optimized quite a bit)
    case STG_PROP_ORIGIN:
    case STG_PROP_SIZE:
    case STG_PROP_PARENT:
    case STG_PROP_NAME:
    case STG_PROP_COLOR:
    case STG_PROP_RECTS:
    case STG_PROP_CIRCLES:
    case STG_PROP_RANGERS:
    case STG_PROP_NOSE:
    case STG_PROP_BLINKENLIGHT:
    case STG_PROP_MOUSE_MODE:
    case STG_PROP_BORDER:
      
      stg_gui_model_destroy( ent->guimod );
      ent->guimod = stg_gui_model_create( ent );
      break;

      // do nothing for these things
    case STG_PROP_LASERRETURN:
    case STG_PROP_SONARRETURN:
    case STG_PROP_IDARRETURN:
    case STG_PROP_OBSTACLERETURN:
    case STG_PROP_VISIONRETURN:
    case STG_PROP_PUCKRETURN:
    case STG_PROP_PLAYERID:
    case STG_PROP_VELOCITY:
    case STG_PROP_NEIGHBORBOUNDS:
    case STG_PROP_NEIGHBORRETURN:
    case STG_PROP_LOS_MSG:
      break;

    default:
      PRINT_WARN1( "property change %d unhandled by gui", prop ); 
      break;
    } 
  
  return 0;
}
 


#endif // INCLUDE_RTK2


