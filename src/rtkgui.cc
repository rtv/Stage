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
 * CVS info: $Id: rtkgui.cc,v 1.24 2003-09-09 21:44:39 rtv Exp $
 */

#if HAVE_CONFIG_H
#  include <config.h>
#endif

// determined by config.h, built by autoconf
#ifdef INCLUDE_RTK2

//#define DEBUG 
//#define VERBOSE
//#undef DEBUG
//#undef VERBOSE

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
extern GHashTable* global_client_table;
extern int global_num_clients;

// defaults

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
#define STG_LAYER_LIGHT 70
#define STG_LAYER_SENSOR 61
#define STG_LAYER_GRID 45
#define STG_LAYER_USER 99

#define STG_GUI_COUNTDOWN_INTERVAL_MS 200
#define STG_GUI_COUNTDOWN_TIME_DATA 500

// the menu-selectable window refresh rates
int intervals[] = {25, 50, 100, 200, 500, 1000};
int num_intervals = 6;  
int default_interval = 2; // xth entry in the above array

// single static application visible to all funcs in this file
static rtk_app_t *app = NULL; 


void RtkOnMouse(rtk_fig_t *fig, int event, int mode);
void StgClientDestroy( stg_client_data_t* cli ); // from main.cc


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
void stg_gui_matrix_render( stg_gui_window_t* win )
{
  if( win->fig_matrix == NULL )
    {
      win->fig_matrix = rtk_fig_create(win->canvas,NULL,STG_LAYER_BACKGROUND);
      rtk_fig_color_rgb32(win->fig_matrix, stg_lookup_color(STG_MATRIX_COLOR));
    }
  else
    rtk_fig_clear( win->fig_matrix );
  
  CMatrix* matrix = win->world->matrix;
  assert( matrix );
  
  double pixel_size = 1.0 / matrix->ppm;
  
  // render every pixel as an unfilled rectangle
  for( int y=0; y < matrix->height; y++ )
    for( int x=0; x < matrix->width; x++ )
      if( *(matrix->get_cell( x, y )) )
	rtk_fig_rectangle( win->fig_matrix, 
			   x*pixel_size, y*pixel_size, 0, 
			   pixel_size, pixel_size, 0 );
}


// shows or hides a layer
void stg_gui_menuitem_show_layer( rtk_menuitem_t *item )
{
  //printf( "%s layer %d\n", rtk_menuitem_ischecked(item) ? "showing" : "hiding", (int)item->userdata );

  rtk_canvas_layer_show( item->menu->canvas, (int)item->userdata, 
			 rtk_menuitem_ischecked(item) );
}

// send a USR2 signal to the client process that created this menuitem
void stg_gui_save( rtk_menuitem_t *item )
{
  stg_client_data_t* client = 
    ((stg_world_t*)item->menu->canvas->userdata)->client;
  if( client ) kill( client->pid, SIGUSR2 );
}

void stg_gui_exit( rtk_menuitem_t *item )
{
  //quit = TRUE;
  puts( "Exit menu item. Destroying world" );
  
  stg_world_t* world = (stg_world_t*)item->menu->canvas->userdata;
  //stg_client_data_t* client = world->client; 

  stg_world_destroy( world );

  // if the client has no worlds left, shut it down
  //if( g_list_length( client->worlds ) < 1 )
  //StgClientDestroy( client );
}


void stg_gui_menu_interval_callback( rtk_menuitem_t *item )
{
  assert(item);
  stg_gui_window_t* win = (stg_gui_window_t*)item->menu->canvas->userdata;
  int interval = (int)item->userdata;
  
  if( rtk_menuitem_ischecked( item ) )
    {
      if( win->tag_refresh > 0 )
	{      
	  PRINT_DEBUG1( "removing source %d",  win->tag_refresh );
	  g_source_remove( win->tag_refresh );
	}
      
      
      PRINT_DEBUG1( "refresh rate set to %d ms", interval );
      win->tag_refresh = g_timeout_add(interval,stg_gui_window_callback,win);
      PRINT_DEBUG2( "added source %d at interval %d",  
		    win->tag_refresh, intervals[i] );

      // uncheck all the other options
      if( win->mitems[STG_MITEM_VIEW_REFRESH_25] != item )
	rtk_menuitem_check( win->mitems[STG_MITEM_VIEW_REFRESH_25], 0 );
      if( win->mitems[STG_MITEM_VIEW_REFRESH_50] != item )
	rtk_menuitem_check( win->mitems[STG_MITEM_VIEW_REFRESH_50], 0 );
      if( win->mitems[STG_MITEM_VIEW_REFRESH_100] != item )
	rtk_menuitem_check( win->mitems[STG_MITEM_VIEW_REFRESH_100], 0 );
      if( win->mitems[STG_MITEM_VIEW_REFRESH_200] != item )
	rtk_menuitem_check( win->mitems[STG_MITEM_VIEW_REFRESH_200], 0 );
      if( win->mitems[STG_MITEM_VIEW_REFRESH_500] != item )
	rtk_menuitem_check( win->mitems[STG_MITEM_VIEW_REFRESH_500], 0 );
      if( win->mitems[STG_MITEM_VIEW_REFRESH_1000] != item )
	rtk_menuitem_check( win->mitems[STG_MITEM_VIEW_REFRESH_1000], 0 );
    }
}

// build a simulation window
stg_gui_window_t* stg_gui_window_create( stg_world_t* world, int width, int height )
{
  stg_gui_window_t* win = (stg_gui_window_t*)calloc( 1, sizeof(stg_gui_window_t) );
 
  if( width < 1 ) width = STG_DEFAULT_WINDOW_WIDTH;
  if( height < 1 ) height = STG_DEFAULT_WINDOW_HEIGHT;
 
  win->canvas = rtk_canvas_create(app);
  win->canvas->userdata = (void*)win;

  win->fig_grid = stg_gui_grid_create( win->canvas, NULL, 
				       world->width/2.0,world->height/2.0,0, 
				       world->width, world->height, 1.0, 0.1 );
      
  win->countdowns = NULL; // a list of figures that should be cleared
			   // at some point in the future
  win->movie_count = 0;

  // hide all the important layers. they are reactivated by menu
  // options below.
  rtk_canvas_layer_show( win->canvas, STG_LAYER_BODY, 0 );
  rtk_canvas_layer_show( win->canvas, STG_LAYER_SENSOR, 0 );
  rtk_canvas_layer_show( win->canvas, STG_LAYER_LIGHT, 0 );
  rtk_canvas_layer_show( win->canvas, STG_LAYER_GRID, 0 );
  rtk_canvas_layer_show( win->canvas, STG_LAYER_USER, 0 );
  rtk_canvas_layer_show( win->canvas, STG_LAYER_BACKGROUND, 1 );

  memset( win->menus, 0, STG_MENU_COUNT * sizeof(rtk_menu_t*));
  memset( win->mitems, 0, STG_MITEM_COUNT * sizeof(rtk_menuitem_t*));
  
  // create the top-level menus
  win->menus[STG_MENU_FILE] = rtk_menu_create(win->canvas, "File");  
  win->menus[STG_MENU_VIEW] = rtk_menu_create(win->canvas, "View");

  // create the FILE sub-menus
  //win->menus[STG_MENU_FILE_STILLS] = 
  //rtk_menu_create_sub(win->menus[STG_MENU_FILE], "Export stills");
  //win->menus[STG_MENU_FILE_MOVIES] = 
  //rtk_menu_create_sub(win->menus[STG_MENU_FILE], "Export movies");

  // create the VIEW sub-menus
  win->menus[STG_MENU_VIEW_OBJECT] = 
    rtk_menu_create_sub(win->menus[STG_MENU_VIEW], "Objects");
  win->menus[STG_MENU_VIEW_DATA] = 
    rtk_menu_create_sub(win->menus[STG_MENU_VIEW], "Data");
  win->menus[STG_MENU_VIEW_REFRESH] = 
    rtk_menu_create_sub(win->menus[STG_MENU_VIEW], "Refresh");

  // create the FILE menu items
  win->mitems[STG_MITEM_FILE_SAVE] = 
    rtk_menuitem_create(win->menus[STG_MENU_FILE], "Save", 0);
  win->mitems[STG_MITEM_FILE_QUIT] = 
    rtk_menuitem_create(win->menus[STG_MENU_FILE], "Quit", 0);

  // create the VIEW menu items
  win->mitems[STG_MITEM_VIEW_GRID] = 
    rtk_menuitem_create(win->menus[STG_MENU_VIEW], "Grid", 1);
  win->mitems[STG_MITEM_VIEW_MATRIX] = 
    rtk_menuitem_create(win->menus[STG_MENU_VIEW], "Matrix", 1);


  // create the VIEW/OBJECTS menu items
  win->mitems[STG_MITEM_VIEW_OBJECT_BODY] 
    = rtk_menuitem_create(win->menus[STG_MENU_VIEW_OBJECT], "Body", 1);
  win->mitems[STG_MITEM_VIEW_OBJECT_SENSOR] 
    = rtk_menuitem_create(win->menus[STG_MENU_VIEW_OBJECT], "Sensor", 1);
  win->mitems[STG_MITEM_VIEW_OBJECT_LIGHT] 
    = rtk_menuitem_create(win->menus[STG_MENU_VIEW_OBJECT], "Light", 1);
  win->mitems[STG_MITEM_VIEW_OBJECT_USER] 
    = rtk_menuitem_create(win->menus[STG_MENU_VIEW_OBJECT], "User", 1);

  // create the VIEW/DATA menu items
  win->mitems[STG_MITEM_VIEW_DATA_LASER] 
    = rtk_menuitem_create(win->menus[STG_MENU_VIEW_DATA], "Laser", 1);
  win->mitems[STG_MITEM_VIEW_DATA_RANGER] 
    = rtk_menuitem_create(win->menus[STG_MENU_VIEW_DATA], "Ranger", 1);
  win->mitems[STG_MITEM_VIEW_DATA_NEIGHBORS] 
    = rtk_menuitem_create(win->menus[STG_MENU_VIEW_DATA], "Fiducial", 1);
  win->mitems[STG_MITEM_VIEW_DATA_BLOBFINDER] 
    = rtk_menuitem_create(win->menus[STG_MENU_VIEW_DATA], "Blobfinder", 1);
  
  // create the VIEW/REFRESH menu items
  win->mitems[STG_MITEM_VIEW_REFRESH_25] 
    = rtk_menuitem_create(win->menus[STG_MENU_VIEW_REFRESH], "25 ms", 1);
  win->mitems[STG_MITEM_VIEW_REFRESH_50] 
    = rtk_menuitem_create(win->menus[STG_MENU_VIEW_REFRESH], "50 ms", 1);
  win->mitems[STG_MITEM_VIEW_REFRESH_100] 
    = rtk_menuitem_create(win->menus[STG_MENU_VIEW_REFRESH], "100 ms", 1);
  win->mitems[STG_MITEM_VIEW_REFRESH_200] 
    = rtk_menuitem_create(win->menus[STG_MENU_VIEW_REFRESH], "200 ms", 1);
  win->mitems[STG_MITEM_VIEW_REFRESH_500] 
    = rtk_menuitem_create(win->menus[STG_MENU_VIEW_REFRESH], "500 ms", 1);
  win->mitems[STG_MITEM_VIEW_REFRESH_1000] 
    = rtk_menuitem_create(win->menus[STG_MENU_VIEW_REFRESH], "1000 ms", 1);

  
  /*
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


  // every menu item gets a pointer to this window for use in callbacks
  for( int i=0; i<STG_MITEM_COUNT; i++ )
    if( win->mitems[i] ) win->mitems[i]->userdata =  (void*)win;
    

  // add the userdata for callbacks that need it
  win->mitems[STG_MITEM_VIEW_OBJECT_BODY]->userdata = (void*)STG_LAYER_BODY;
  win->mitems[STG_MITEM_VIEW_OBJECT_SENSOR]->userdata =(void*)STG_LAYER_SENSOR;
  win->mitems[STG_MITEM_VIEW_OBJECT_LIGHT]->userdata = (void*)STG_LAYER_LIGHT;
  win->mitems[STG_MITEM_VIEW_OBJECT_USER]->userdata = (void*)STG_LAYER_USER;
  win->mitems[STG_MITEM_VIEW_GRID]->userdata = (void*)STG_LAYER_GRID;
  
  win->mitems[STG_MITEM_VIEW_REFRESH_25]->userdata = (void*)25;
  win->mitems[STG_MITEM_VIEW_REFRESH_50]->userdata = (void*)50;
  win->mitems[STG_MITEM_VIEW_REFRESH_100]->userdata = (void*)100;
  win->mitems[STG_MITEM_VIEW_REFRESH_200]->userdata = (void*)200;
  win->mitems[STG_MITEM_VIEW_REFRESH_500]->userdata = (void*)500;
  win->mitems[STG_MITEM_VIEW_REFRESH_1000]->userdata = (void*)1000;
  
  // add the callbacks
  rtk_menuitem_set_callback( win->mitems[STG_MITEM_FILE_SAVE], stg_gui_save );
  rtk_menuitem_set_callback( win->mitems[STG_MITEM_FILE_QUIT], stg_gui_exit );
			     
  rtk_menuitem_set_callback( win->mitems[STG_MITEM_VIEW_OBJECT_BODY], 
			     stg_gui_menuitem_show_layer );
  rtk_menuitem_set_callback( win->mitems[STG_MITEM_VIEW_OBJECT_SENSOR], 
			     stg_gui_menuitem_show_layer );
  rtk_menuitem_set_callback( win->mitems[STG_MITEM_VIEW_OBJECT_LIGHT], 
			     stg_gui_menuitem_show_layer );
  rtk_menuitem_set_callback( win->mitems[STG_MITEM_VIEW_OBJECT_USER], 
			     stg_gui_menuitem_show_layer );
  rtk_menuitem_set_callback( win->mitems[STG_MITEM_VIEW_GRID], 
			     stg_gui_menuitem_show_layer );
    
  // refresh rate items
  rtk_menuitem_set_callback( win->mitems[STG_MITEM_VIEW_REFRESH_25], 
			     stg_gui_menu_interval_callback );
  rtk_menuitem_set_callback( win->mitems[STG_MITEM_VIEW_REFRESH_50], 
			     stg_gui_menu_interval_callback );
  rtk_menuitem_set_callback( win->mitems[STG_MITEM_VIEW_REFRESH_100], 
			     stg_gui_menu_interval_callback );
  rtk_menuitem_set_callback( win->mitems[STG_MITEM_VIEW_REFRESH_200], 
			     stg_gui_menu_interval_callback );
  rtk_menuitem_set_callback( win->mitems[STG_MITEM_VIEW_REFRESH_500], 
			     stg_gui_menu_interval_callback );
  rtk_menuitem_set_callback( win->mitems[STG_MITEM_VIEW_REFRESH_1000], 
			     stg_gui_menu_interval_callback );


  // set the default checks - the callback functions will set things
  // up properly
  rtk_menuitem_check(win->mitems[STG_MITEM_VIEW_GRID], 0);
  rtk_menuitem_check(win->mitems[STG_MITEM_VIEW_REFRESH_100], 1);
  rtk_menuitem_check(win->mitems[STG_MITEM_VIEW_OBJECT_BODY], 1);
  rtk_menuitem_check(win->mitems[STG_MITEM_VIEW_OBJECT_LIGHT], 1);
  rtk_menuitem_check(win->mitems[STG_MITEM_VIEW_OBJECT_SENSOR], 0);
  rtk_menuitem_check(win->mitems[STG_MITEM_VIEW_OBJECT_LIGHT], 1);
  rtk_menuitem_check(win->mitems[STG_MITEM_VIEW_OBJECT_USER], 1);
  rtk_menuitem_check(win->mitems[STG_MITEM_VIEW_DATA_LASER], 1);
  rtk_menuitem_check(win->mitems[STG_MITEM_VIEW_DATA_RANGER], 1);
  rtk_menuitem_check(win->mitems[STG_MITEM_VIEW_DATA_NEIGHBORS], 1);
  rtk_menuitem_check(win->mitems[STG_MITEM_VIEW_DATA_BLOBFINDER], 1);  

  win->statusbar = GTK_STATUSBAR(gtk_statusbar_new());
  
  gtk_box_pack_start(GTK_BOX(win->canvas->layout), 
		     GTK_WIDGET(win->statusbar), FALSE, TRUE, 0);
  

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

  // start a callback that deletes expired data figures
  win->tag_countdown = g_timeout_add( STG_GUI_COUNTDOWN_INTERVAL_MS, 
				      stg_gui_window_clear_countdowns, win );
  
  // return the completed window
  return win;
}

const char* stg_gui_model_describe(  stg_gui_model_t* mod )
{
  static char txt[256];
  
  double x, y, a;
  rtk_fig_get_origin( *mod->fig, &x, &y, &a );
  
  // display the pose
  snprintf(txt, sizeof(txt), "\"%s\" (%d) pose: [%.2f,%.2f,%.2f]",  
	   mod->ent->name->str, mod->ent->id, x,y,a  );
  
  return txt;
}

// this gets called periodically from a callback installed in
// stg_gui_window_create() or stg_gui_menu_interval_callback()
gboolean stg_gui_window_callback( gpointer data )
{
  //putchar( '.' ); fflush(stdout);

  stg_gui_window_t* win = (stg_gui_window_t*)data;
  
  // if the menu says draw the matrix, we do it.
  if( rtk_menuitem_ischecked(win->mitems[STG_MITEM_VIEW_MATRIX]) )
    stg_gui_matrix_render( win );
  else if( win->fig_matrix ) // if there's a left over matrix fig, we zap it
    {
      rtk_fig_destroy( win->fig_matrix );
      win->fig_matrix = NULL;
    }
	
  rtk_canvas_flash_update( win->canvas );

  // redraw anything that needs it
  rtk_canvas_render( win->canvas );

  // put a description of the selected model in the status bar
  static rtk_fig_t* fig = NULL;
  if( win->canvas->mouse_over_fig != fig ) //if the mouseover fig has changed
    {
      fig = win->canvas->mouse_over_fig;
      
      // remove the old status bar entry
      guint cid = 
	gtk_statusbar_get_context_id( win->statusbar, "window_callback" );
      gtk_statusbar_pop( win->statusbar, cid ); 
      
      if( fig ) // if there is a figure under the mouse, describe it
	{
	  CEntity* ent = (CEntity*)win->canvas->mouse_over_fig->userdata;
	  char txt[256];
	  snprintf( txt, sizeof(txt), "Selection: %s",
		    stg_gui_model_describe(ent->guimod) ); 
	  gtk_statusbar_push( win->statusbar, cid, txt ); 
	}
    }
  
  return TRUE;
}

void stg_gui_window_destroy( stg_gui_window_t* win )
{
  PRINT_DEBUG1( "destroying window %p", win );

  if( win == NULL )
    {
      PRINT_WARN( "requested destruction of NULL window" );
      return;
    }
  
  PRINT_DEBUG1( "removing source %d",  win->tag_refresh );
  // cancel the callback for this window
  g_source_remove( win->tag_refresh );
  g_source_remove( win->tag_countdown );
  
  if( win->fig_grid ) rtk_fig_destroy( win->fig_grid );
  if( win->fig_matrix ) rtk_fig_destroy( win->fig_matrix );

  for( int i=0; i<STG_MITEM_COUNT; i++ )
    if( win->mitems[i] ) rtk_menuitem_destroy( win->mitems[i] );
  
  for( int i=0; i<STG_MENU_COUNT; i++ )
    if( win->menus[i] ) rtk_menu_destroy( win->menus[i] );

  // must delete the canvas after the figures
  rtk_canvas_destroy( win->canvas );
  free( win );
}

gboolean stg_gui_window_clear_countdowns( gpointer ptr )
{
  stg_gui_window_t* win = (stg_gui_window_t*)ptr;
  
  // operate on the head of the list of figures we should kill

  GList* lel = win->countdowns;
  while( lel )
    {
      stg_gui_countdown_t* cd = (stg_gui_countdown_t*)lel->data;
      
      if( cd->timeleft < 1 )
	{ 
	  rtk_fig_clear( cd->fig );
	  
	  GList* doomed = lel;
	  lel = lel->next; // next element before we delete this element
	  
	  win->countdowns =  g_list_delete_link( win->countdowns, doomed );
	}
      else
	{
	  // decrement the timeleft by the interval we were called at
	  cd->timeleft -= STG_GUI_COUNTDOWN_INTERVAL_MS;
	  lel = lel->next;
	}
	  
    }
  
  return TRUE;
}


rtk_fig_t* stg_gui_grid_create( rtk_canvas_t* canvas, rtk_fig_t* parent, 
				double origin_x, double origin_y, double origin_a, 
				double width, double height, double major, double minor )
{
  rtk_fig_t* grid = NULL;

  assert( (grid = rtk_fig_create( canvas, parent, 
				     STG_LAYER_GRID )) );
  
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


void stg_gui_model_blinkenlight( CEntity* ent )
{
  g_assert(ent);
  g_assert(ent->guimod);
  
  stg_gui_model_t* mod = ent->guimod;
  
  if( mod->fig[STG_GUI_OBJECT_LIGHT] == NULL )
    {
      mod->fig[STG_GUI_OBJECT_LIGHT] = 
	rtk_fig_create( mod->win->canvas, *mod->fig, STG_LAYER_LIGHT);
      
      rtk_fig_color_rgb32( mod->fig[STG_GUI_OBJECT_LIGHT], 
			   mod->ent->color );
    }
  else
    rtk_fig_clear( mod->fig[STG_GUI_OBJECT_LIGHT] );
  
  // nothing to see here
  if( !ent->blinkenlight.enable )
    return;
  
  rtk_fig_ellipse( mod->fig[STG_GUI_OBJECT_LIGHT], 
		   0,0,0,  ent->size.x, ent->size.y, 1 );
  
  // start the light blinking if it's not stuck on (ie. period 0)
  if( ent->blinkenlight.period_ms != 0 )    
    rtk_fig_blink( mod->fig[STG_GUI_OBJECT_LIGHT], 
		   ent->blinkenlight.period_ms/2, 1 );  
}

void stg_gui_model_rects( CEntity* ent )
{
  g_assert(ent);
  g_assert(ent->guimod);
  
  stg_gui_model_t* mod = ent->guimod;

  // the rectangle figure already exists
  rtk_fig_clear( mod->fig[STG_GUI_OBJECT_RECT] );
  rtk_fig_color_rgb32( mod->fig[STG_GUI_OBJECT_RECT], 
		       ent->color );
  
  int r;
  int rmax = ent->GetNumRects();
  
  PRINT_DEBUG1( "rendering %d rectangles", rmax);
  
  for( r=0; r<rmax; r++ )
    {
      stg_rotrect_t* src = ent->GetRect(r);
      double x,y,a,w,h;
      
      x = ((src->x + src->w/2.0) * ent->size.x) 
	- ent->size.x/2.0 + ent->pose_origin.x;
      y = ((src->y + src->h/2.0) * ent->size.y) 
	- ent->size.y/2.0 + ent->pose_origin.y;
      a = src->a;
      w = src->w * ent->size.x;
      h = src->h * ent->size.y;
      
      rtk_fig_rectangle( mod->fig[STG_GUI_OBJECT_RECT], 
			 x,y,a,w,h, 0 ); 
    }
}

void stg_gui_model_rangers( CEntity* ent )
{
  g_assert(ent);
  g_assert(ent->guimod);

  stg_gui_model_t* mod = ent->guimod;
  
  if( mod->fig[STG_GUI_OBJECT_SENSOR] == NULL )
    {
      mod->fig[STG_GUI_OBJECT_SENSOR] = 
	rtk_fig_create( mod->win->canvas, *mod->fig, STG_LAYER_SENSOR);
      
      rtk_fig_color_rgb32( mod->fig[STG_GUI_OBJECT_SENSOR], 
			   stg_lookup_color(STG_RANGER_COLOR) );
     
      //rtk_fig_show(  mod->fig[STG_GUI_OBJECT_SENSOR], 
      //	     rtk_menuitem_ischecked( mod->win->mitems[STG_MITEM_VIEW_OBJECT_SENSOR] ) 
      ///	     );
    }
  else
    rtk_fig_clear( mod->fig[STG_GUI_OBJECT_SENSOR] );
  
  // add rects showing ranger positions
  if( ent->rangers )
    for( int s=0; s< (int)ent->rangers->len; s++ )
      {
	stg_ranger_t* rngr = &g_array_index( ent->rangers, 
					     stg_ranger_t, s );
	
	//printf( "drawing a ranger rect (%.2f,%.2f,%.2f)[%.2f %.2f]\n",
	//  rngr->pose.x, rngr->pose.y, rngr->pose.a,
	//  rngr->size.x, rngr->size.y );
	
	rtk_fig_rectangle( mod->fig[STG_GUI_OBJECT_SENSOR], 
			   rngr->pose.x, rngr->pose.y, rngr->pose.a,
			   rngr->size.x, rngr->size.y, 0 ); 
	
	// show the FOV too
	double sidelen = rngr->size.x/2.0;
	
	double x1= rngr->pose.x + sidelen*cos(rngr->pose.a - rngr->fov/2.0 );
	double y1= rngr->pose.y + sidelen*sin(rngr->pose.a - rngr->fov/2.0 );
	double x2= rngr->pose.x + sidelen*cos(rngr->pose.a + rngr->fov/2.0 );
	double y2= rngr->pose.y + sidelen*sin(rngr->pose.a + rngr->fov/2.0 );
	
	rtk_fig_line( mod->fig[STG_GUI_OBJECT_SENSOR],
		      rngr->pose.x, rngr->pose.y, x1, y1 );
	rtk_fig_line( mod->fig[STG_GUI_OBJECT_SENSOR],
		      rngr->pose.x, rngr->pose.y, x2, y2 );
      }
}

void stg_gui_model_nose( CEntity* ent )
{
  g_assert(ent);
  g_assert(ent->guimod);
 
  stg_gui_model_t* mod = ent->guimod;
  
  if( mod->fig[STG_GUI_OBJECT_NOSE] == NULL )
    {
      mod->fig[STG_GUI_OBJECT_NOSE] = 
	rtk_fig_create( mod->win->canvas, *mod->fig, STG_LAYER_BODY);
      rtk_fig_color_rgb32( mod->fig[STG_GUI_OBJECT_NOSE], ent->color );
    }
  else
    rtk_fig_clear( mod->fig[STG_GUI_OBJECT_NOSE] );
  
  // add a nose-line to position models
  if( ent->draw_nose )
    {
      stg_pose_t origin;
      ent->GetOrigin( &origin );
      stg_size_t size;
      ent->GetSize( &size );
      rtk_fig_line( mod->fig[STG_GUI_OBJECT_NOSE],
		    origin.x, origin.y, size.x/2.0, origin.y );
    }
}

void stg_gui_model_mouse_mode( CEntity* ent )
{
  g_assert(ent);
  g_assert(ent->guimod);

  stg_gui_model_t* mod = ent->guimod;
  
  // we can only manipulate top-level figures
  if( g_node_depth( ent->node ) == 2 )
    {
      if( ent->mouseable )
	mod->movemask = RTK_MOVE_TRANS | RTK_MOVE_ROT;
      else
	mod->movemask = 0;
    }
  else if( g_node_depth( ent->node ) > 2 )
    {
      mod->movemask = 0;
    }

  if( mod->movemask )
    {
      // figure gets the same movemask as the model
      rtk_fig_movemask( *mod->fig, mod->movemask);  
      
      // Set the mouse handler
      rtk_fig_add_mouse_handler( *mod->fig, RtkOnMouse);
    }
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
 
  // calloc zeros the structure
  stg_gui_model_t* mod = (stg_gui_model_t*)calloc( 1,sizeof(stg_gui_model_t));
  
  // associate the window and entity with this figure
  mod->ent = ent; 
  mod->win = win; 
  
  rtk_fig_t* parent_fig = NULL;
  
  // children of top-level entities are attached to their parent's figure
  if( g_node_depth( ent->node ) > 2 )
    parent_fig = *stg_ent_parent(ent)->guimod->fig;      
  
  // Create a figure representing this entity
  assert( (*mod->fig = 
	   rtk_fig_create( mod->win->canvas, parent_fig, STG_LAYER_BODY )));
  
  // associate the figure with the entity, too.
  (*mod->fig)->userdata = (void*)ent;
  
  // move the figure to the entity's pose
  stg_pose_t pose;
  ent->GetPose( &pose );
  rtk_fig_origin( *mod->fig, 
		  pose.x, pose.y, pose.a );
  
  //rtk_fig_show( mod->fig[STG_GUI_OBJECT_RECT], true ); 
  rtk_fig_color_rgb32( *mod->fig, ent->color);   

  // zero the pointers and counters of our data figures
  memset( mod->datafigs, 0, sizeof(rtk_fig_t*) * STG_GUI_DATA_COUNT );

  return mod;
}

void stg_gui_model_destroy( stg_gui_model_t* mod )
{
  //printf( "destroying model %p\n", mod );


  //printf( "destroying data figs\n" );
  // clean up the figures
  for( int i=0; i<STG_GUI_DATA_COUNT; i++ )
    if( mod->datafigs[i].fig ) rtk_fig_destroy( mod->datafigs[i].fig );

  //printf( "destroying object figs\n" );
  for( int i=1; i<STG_GUI_OBJECT_COUNT; i++ )
    if( mod->fig[i] ) rtk_fig_destroy( mod->fig[i] );  
  
  //printf( "destroying head fig\n" );
  // destroy the parent fig last
  rtk_fig_destroy( mod->fig[0] );
  //printf( "destroying figs for model %p done\n", mod );

  free( mod );
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
  char txt[256];

  switch (event)
    {
    case RTK_EVENT_PRESS:
      
      // this is how to get some sensor data, in case we need to reinstate the 
      // showing of data when handling an object. how best to do this?
      //{
      //stg_property_t* prop = entity->GetProperty( STG_PROP_LASER_DATA );
      //free(prop);
      //} 

      // DELIBERATE NO-BREAK      
    case RTK_EVENT_MOTION:

      // if there are more motion events pending, do nothing.

      // move the object to follow the mouse
      rtk_fig_get_origin(fig, &pose.x, &pose.y, &pose.a );
      entity->SetProperty( STG_PROP_POSE, &pose, sizeof(pose) );
      
      // display the pose
      snprintf(txt, sizeof(txt), "Dragging: %s", stg_gui_model_describe(mod)); 
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
  stg_gui_model_t* model = ent->guimod;

  stg_gui_countdown_t* cd = &model->datafigs[STG_GUI_DATA_NEIGHBORS];
  model->win->countdowns = g_list_remove( model->win->countdowns, cd );

  if( cd->fig == NULL )
    {
      cd->fig = rtk_fig_create( model->win->canvas, NULL, STG_LAYER_DATA ); 
      rtk_fig_color_rgb32( cd->fig, stg_lookup_color( STG_FIDUCIAL_COLOR ) );
    }
  else
    rtk_fig_clear( cd->fig );
  
  
  if( !rtk_menuitem_ischecked( model->win->mitems[STG_MITEM_VIEW_DATA_NEIGHBORS]))
    return;
  
  stg_pose_t pz;
  ent->GetGlobalPose( &pz );
  
  rtk_fig_origin( cd->fig, pz.x, pz.y, pz.a );

  char text[65];
  
  for( int n=0; n < (int)neighbors->len; n++ )
    {
      stg_neighbor_t* nbor = &g_array_index( neighbors, stg_neighbor_t, n);
      
      double px = nbor->range * cos(nbor->bearing);
      double py = nbor->range * sin(nbor->bearing);
      double pa = nbor->orientation;
      
      rtk_fig_line( cd->fig, 0, 0, px, py );	
      
      // double wx = nbor->size.x;
      //double wy = nbor->size.y;
      //rtk_fig_rectangle( cd->fig, px, py, pa, wx, wy, 0);
      //rtk_fig_line( cd->fig, px+wx/2.0, py+wy/2.0, px-wx/2.0, py-wy/2.0 );
      //rtk_fig_line( cd->fig, px+wx/2.0, py-wy/2.0, px-wx/2.0, py+wy/2.0 );
      //rtk_fig_arrow( cd->fig, px, py, pa, wy, 0.10);
      
      snprintf(text, 64, "  %d", nbor->id );
      rtk_fig_text( cd->fig, px, py, pa, text);
    }

  // reset the timer on our countdown
  cd->timeleft = STG_GUI_COUNTDOWN_TIME_DATA;
  
  // if our countdown does not appear in the window's list, add it.
  if( g_list_find( model->win->countdowns, cd ) == NULL  )
    model->win->countdowns = g_list_append( model->win->countdowns, cd );
}

// render the entity's laser data
void stg_gui_laser_render( CEntity* ent )
{
  stg_gui_model_t* model = ent->guimod;
  stg_laser_data_t* laser = &ent->laser_data;
  
  // reset the timer on our countdown
  stg_gui_countdown_t* cd = &model->datafigs[STG_GUI_DATA_LASER];
  model->win->countdowns = g_list_remove( model->win->countdowns, cd );
  
  if( cd->fig == NULL )
    {
      cd->fig = rtk_fig_create( model->win->canvas, NULL, STG_LAYER_DATA ); 
      rtk_fig_color_rgb32( cd->fig, stg_lookup_color( STG_LASER_COLOR ) );
    }
  else
    rtk_fig_clear( cd->fig );
  
  if( !rtk_menuitem_ischecked( model->win->mitems[STG_MITEM_VIEW_DATA_LASER]))
    return;
  
  stg_pose_t pz;
  memcpy( &pz, &laser->pose, sizeof(stg_pose_t) );
  ent->LocalToGlobal( &pz );
  rtk_fig_origin( cd->fig, pz.x, pz.y, pz.a );
  
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
      
      rtk_fig_line( cd->fig, lx, ly, px, py );
      lx = px;
      ly = py;
      
      // add little boxes at high intensities (like in playerv)
      if(  laser->samples[i].reflectance > 0 )
	rtk_fig_rectangle( cd->fig, px, py, 0, 0.02, 0.02, 1);
      
    }
  
  rtk_fig_line( cd->fig, 0.0, 0.0, lx, ly );

  cd->timeleft = STG_GUI_COUNTDOWN_TIME_DATA;
  model->win->countdowns = g_list_append( model->win->countdowns, cd );
}

// render the entity's rangers
void stg_gui_rangers_render( CEntity* ent )
{
  stg_gui_model_t* model = ent->guimod;
  
  // the world will clear this fig in a few ms
  // so we reset the timer
  stg_gui_countdown_t* cd = &model->datafigs[STG_GUI_DATA_RANGER];
  model->win->countdowns = g_list_remove( model->win->countdowns, cd );

  if( cd->fig == NULL )
    {
      cd->fig = rtk_fig_create( model->win->canvas, NULL, STG_LAYER_DATA ); 
      rtk_fig_color_rgb32( cd->fig, stg_lookup_color(STG_RANGER_COLOR) );
    }
  else
    rtk_fig_clear( cd->fig );
  
  
  if( !rtk_menuitem_ischecked( model->win->mitems[STG_MITEM_VIEW_DATA_RANGER] ) )
    return;

  for( int t=0; t < (int)ent->rangers->len; t++ )
    {
      stg_ranger_t* tran = &g_array_index( ent->rangers, 
					   stg_ranger_t, t );
      stg_pose_t pz;
      memcpy( &pz, &tran->pose, sizeof(stg_pose_t) );
      ent->LocalToGlobal( &pz );       
      
      rtk_fig_arrow( cd->fig, pz.x, pz.y, pz.a, tran->range, 0.05 );
      
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
  
  
  // if our countdown does not appear in the window's list, add it.
   cd->timeleft = STG_GUI_COUNTDOWN_TIME_DATA;
  model->win->countdowns = g_list_append( model->win->countdowns, cd );
}

int stg_gui_los_msg_send( CEntity* ent, stg_los_msg_t* msg )
{
  rtk_canvas_t* canvas =  ent->guimod->fig[0]->canvas;
  rtk_fig_t* fig = 
    rtk_fig_create( canvas, ent->guimod->fig[0], STG_LAYER_DATA );
  
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
  rtk_canvas_t* canvas =  sender->guimod->fig[0]->canvas;
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
  rtk_canvas_flash( canvas, fig, 8, 1 );

  free( buf );
  return 0; //ok
}

int stg_gui_model_update( CEntity* ent, stg_prop_id_t prop )
{
  //PRINT_DEBUG3( "gui update for %d:%s prop %s", 
  //	ent->id, ent->name->str, stg_property_string(prop) );
  
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
	rtk_fig_origin( *model->fig, pose.x, pose.y, pose.a );
      }
      break;

    case STG_PROP_BORDER:      
    case STG_PROP_SIZE:
    case STG_PROP_COLOR:
    case STG_PROP_ORIGIN: // could this one be faster?
      stg_gui_model_rects( ent );
      stg_gui_model_nose( ent );
      break;

    case STG_PROP_RECTS:
      stg_gui_model_rects( ent );
      break;

    case STG_PROP_RANGERS:
      stg_gui_model_rangers( ent );
      break;

    case STG_PROP_NOSE:
      stg_gui_model_nose( ent );
      break;

    case STG_PROP_BLINKENLIGHT:
      stg_gui_model_blinkenlight( ent );
      break;

    case STG_PROP_MOUSE_MODE:
      stg_gui_model_mouse_mode( ent );
      break;


      // do nothing for these things
    case STG_PROP_LASERRETURN:
    case STG_PROP_SONARRETURN:
    case STG_PROP_IDARRETURN:
    case STG_PROP_OBSTACLERETURN:
    case STG_PROP_VISIONRETURN:
    case STG_PROP_PUCKRETURN:
    case STG_PROP_VELOCITY:
    case STG_PROP_NEIGHBORBOUNDS:
    case STG_PROP_NEIGHBORRETURN:
    case STG_PROP_LOS_MSG:
    case STG_PROP_LOS_MSG_CONSUME:
    case STG_PROP_NAME:
    case STG_PROP_MATRIX_RENDER:
      break;

      // these aren't yet exposed to the outside
      //case STG_PROP_PARENT:
      //case STG_PROP_CIRCLES:
      //case STG_PROP_PLAYERID:
      
    default:
      PRINT_WARN1( "property change %d unhandled by gui", prop ); 
      break;
    } 
  
  return 0;
}
 


#endif // INCLUDE_RTK2


