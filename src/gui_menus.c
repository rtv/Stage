//#define DEBUG

#include "gui.h"
#include "stdlib.h"

enum {
  STG_MITEM_FILE_SAVE,
  STG_MITEM_FILE_LOAD,
  STG_MITEM_FILE_QUIT,
  STG_MITEM_FILE_IMAGE_JPG,
  STG_MITEM_FILE_IMAGE_PPM,
  STG_MITEM_FILE_MOVIE_START,
  STG_MITEM_FILE_MOVIE_STOP,
  STG_MITEM_VIEW_MATRIX,
  STG_MITEM_VIEW_GRID,
  STG_MITEM_VIEW_OBJECT_BODY,
  STG_MITEM_VIEW_OBJECT_LIGHT,
  STG_MITEM_VIEW_OBJECT_SENSOR,
  STG_MITEM_VIEW_OBJECT_USER,
  STG_MITEM_VIEW_DATA_NEIGHBORS,
  STG_MITEM_VIEW_DATA_LASER,
  STG_MITEM_VIEW_DATA_RANGER,
  STG_MITEM_VIEW_DATA_BLOBFINDER,
  STG_MITEM_VIEW_REFRESH_25,
  STG_MITEM_VIEW_REFRESH_50,
  STG_MITEM_VIEW_REFRESH_100,
  STG_MITEM_VIEW_REFRESH_200,
  STG_MITEM_VIEW_REFRESH_500,
  STG_MITEM_VIEW_REFRESH_1000,
  STG_MITEM_COUNT
};


enum {
  STG_MENU_FILE,
  STG_MENU_FILE_IMAGE,
  STG_MENU_FILE_MOVIE,
  STG_MENU_VIEW,
  STG_MENU_VIEW_REFRESH,
  STG_MENU_VIEW_OBJECT,
  STG_MENU_VIEW_DATA,
  STG_MENU_COUNT
};

enum {
  STG_FIGS_LASER,
  STG_FIGS_RANGER
};

// movies can be saved at these multiples of real time
const int STG_MOVIE_SPEEDS[] = {1, 2, 5, 10, 20, 50, 100};
const int STG_MOVIE_SPEED_COUNT = 7; // must match the static array length

// send a USR2 signal to the client process that created this menuitem
void gui_menu_save( rtk_menuitem_t *item )
{
  PRINT_DEBUG( "Save menu item" );

  world_t* world = (world_t*)item->userdata;
  stg_connection_write_msg( world->con, STG_MSG_CLIENT_SAVE, NULL, 0 ); 
}

void gui_menu_exit( rtk_menuitem_t *item )
{
  //quit = TRUE;
  PRINT_DEBUG( "Exit menu item" );
}


// toggle movie exports
void gui_menu_movie_speed( rtk_menuitem_t *item )
{
  PRINT_DEBUG( "Movie speed menuitem" );



  if( rtk_menuitem_ischecked( item ) )
    {
      gui_window_t* win = (gui_window_t*)item->menu->canvas->userdata;

      int speed_index = (int)item->userdata;  
      
      win->movie_speed = STG_MOVIE_SPEEDS[speed_index];
      
      // uncheck all other speeds
      
      // GTK has menu groups that make this easier. could add this to
      // rtk...
      int i;
      for( i=0; i<win->mitems_mspeed_count; i++ )
	if( win->mitems_mspeed[i] != item )
	  rtk_menuitem_check( win->mitems_mspeed[i], 0);
    }
}



// toggle movie exports
void gui_menu_movie_stop( rtk_menuitem_t *item )
{
  PRINT_DEBUG( "Movie startstop menuitem" );
  
  gui_window_t* win = (gui_window_t*)item->menu->canvas->userdata;
  
  // stop the frame callback
  //g_source_remove(win->movie_tag);
  
  rtk_canvas_movie_stop(win->canvas);
  win->movie_exporting = FALSE;
  
  // disable the speed menu items
  // careful: this assumes the movie speed mitems are contiguous in the enum.
  int i;
  for( i=0; i<win->mitems_mspeed_count; i++ )
    rtk_menuitem_enable( win->mitems_mspeed[i], TRUE );
  
  rtk_menuitem_enable( win->mitems[STG_MITEM_FILE_MOVIE_STOP], 0 );
  rtk_menuitem_enable( win->mitems[STG_MITEM_FILE_MOVIE_START], 1 );
}

// toggle movie exports
void gui_menu_movie_start( rtk_menuitem_t *item )
{
  PRINT_DEBUG( "Movie startstop menuitem" );

  gui_window_t* win = (gui_window_t*)item->menu->canvas->userdata;
  
  
  // start the movie
  win->movie_exporting = TRUE;
  char filename[256];
  snprintf(filename, sizeof(filename), "stage-%03d-sp%02d.mpg",
	   win->movie_count++, win->movie_speed );
  
  rtk_canvas_movie_start( win->canvas, filename, 10, win->movie_speed );
  
  // TODO!
  //win->movie_tag = 
  //g_timeout_add( 100, gui_movie_frame_callback, win->canvas );
  // disable the speed menu items
  // careful: this assumes the movie speed mitems are contiguous in the enum.

  int i;
  for( i=0; i< win->mitems_mspeed_count; i++ )
    rtk_menuitem_enable( win->mitems_mspeed[i], FALSE );
  
  rtk_menuitem_enable( win->mitems[STG_MITEM_FILE_MOVIE_STOP], 1 );
  rtk_menuitem_enable( win->mitems[STG_MITEM_FILE_MOVIE_START], 0 );
}


// shows or hides a layer
void gui_menu_layer( rtk_menuitem_t *item )
{
  PRINT_DEBUG2( "%s layer %d\n", 
		rtk_menuitem_ischecked(item) ? "showing" : "hiding", 
		(int)item->userdata );
  
  rtk_canvas_layer_show( item->menu->canvas, 
			 (int)item->userdata, 
			 rtk_menuitem_ischecked(item) );
}

void gui_menu_matrix( rtk_menuitem_t *item )
{
  PRINT_DEBUG1( "menu selected %s.",
	       rtk_menuitem_ischecked( item ) ? "<checked>" : "<unchecked>" );

  gui_window_t* win = (gui_window_t*)item->menu->canvas->userdata;
 
  win->show_matrix = rtk_menuitem_ischecked( item );
}


void clear_figs( gui_model_t* mod, int type )
{
  switch( type )
    {
    case STG_FIGS_LASER: rtk_fig_clear( mod->laser_data ); break;
    case STG_FIGS_RANGER: rtk_fig_clear( mod->ranger_data ); break;
    default: PRINT_WARN1( "uknown figure type %d", type ); break;
    }
}

void clear_figs_cb( gpointer key, gpointer value, gpointer user )
{
  clear_figs( (gui_model_t*)value, (int)user );
}

void gui_menu_view_data_laser( rtk_menuitem_t *item )
{
  PRINT_DEBUG1( "menu selected %s.",
	       rtk_menuitem_ischecked( item ) ? "<checked>" : "<unchecked>" );

  gui_window_t* win = (gui_window_t*)item->menu->canvas->userdata;
  win->show_laserdata = rtk_menuitem_ischecked( item );
  
  // clear all laserdata figs in this window
  g_hash_table_foreach( win->guimods, clear_figs_cb, STG_FIGS_LASER );
}

void gui_menu_view_data_ranger( rtk_menuitem_t *item )
{
  PRINT_DEBUG1( "menu selected %s.",
	       rtk_menuitem_ischecked( item ) ? "<checked>" : "<unchecked>" );

  gui_window_t* win = (gui_window_t*)item->menu->canvas->userdata;
  win->show_rangerdata = rtk_menuitem_ischecked( item );
  
  // clear all rangerdata figs in this window
  g_hash_table_foreach( win->guimods, clear_figs_cb, (gpointer)STG_FIGS_RANGER );
}
 
void gui_window_menus_create( gui_window_t* win )
{
  win->menu_count = STG_MENU_COUNT;
  win->menus = calloc( sizeof(rtk_menu_t*), win->menu_count  );

  win->mitem_count = STG_MITEM_COUNT;
  win->mitems = calloc( sizeof(rtk_menuitem_t*), win->mitem_count  );
  
  win->mitems_mspeed_count = STG_MOVIE_SPEED_COUNT;
  win->mitems_mspeed = calloc( sizeof(rtk_menuitem_t*), 
			       win->mitems_mspeed_count  );
  
  // create the top-level menus
  win->menus[STG_MENU_FILE] = rtk_menu_create(win->canvas, "File");  
  win->menus[STG_MENU_VIEW] = rtk_menu_create(win->canvas, "View");
  
  // create the FILE sub-menus
  win->menus[STG_MENU_FILE_IMAGE] = 
  rtk_menu_create_sub(win->menus[STG_MENU_FILE], "Export image");
  win->menus[STG_MENU_FILE_MOVIE] = 
  rtk_menu_create_sub(win->menus[STG_MENU_FILE], "Export movie");

  // create the FILE/STILLS menu items 
  win->mitems[STG_MITEM_FILE_IMAGE_JPG] = 
    rtk_menuitem_create(win->menus[STG_MENU_FILE_IMAGE], "JPEG format", 1);
  win->mitems[STG_MITEM_FILE_IMAGE_PPM] = 
    rtk_menuitem_create(win->menus[STG_MENU_FILE_IMAGE], "PPM format", 1);

  // init the stills data
  //win->stills_series = 0;
  //win->stills_count = 0;
  
  // create the FILE/MOVIE menu items
  win->mitems[STG_MITEM_FILE_MOVIE_START] = 
    rtk_menuitem_create(win->menus[STG_MENU_FILE_MOVIE], "Start generating movie",0);
  win->mitems[STG_MITEM_FILE_MOVIE_STOP] = 
    rtk_menuitem_create(win->menus[STG_MENU_FILE_MOVIE], "Stop generating movie",0);

  int i;
  for( i=0; i<win->mitems_mspeed_count; i++ )
    {
      char txt[64];
      snprintf( txt, 63, "Speed x%d", STG_MOVIE_SPEEDS[i] );
      
      win->mitems_mspeed[i] = 
	rtk_menuitem_create(win->menus[STG_MENU_FILE_MOVIE], txt, 1);
      
      rtk_menuitem_set_callback( win->mitems_mspeed[i], gui_menu_movie_speed );
      
      win->mitems_mspeed[i]->userdata = (void*)i;

    }
  
  rtk_menuitem_check(win->mitems_mspeed[0], 1);


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

  // attach objects to the FILE menu items
  win->mitems[STG_MITEM_FILE_SAVE]->userdata = (void*)win->world;
  //win->mitems[STG_MITEM_FILE_QUIT]->userdata = (void*)0;

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
  
  win->mitems[STG_MITEM_VIEW_OBJECT_BODY]->userdata = (void*)STG_LAYER_BODY;
  win->mitems[STG_MITEM_VIEW_OBJECT_SENSOR]->userdata =(void*)STG_LAYER_SENSOR;
  win->mitems[STG_MITEM_VIEW_OBJECT_LIGHT]->userdata = (void*)STG_LAYER_LIGHT;
  win->mitems[STG_MITEM_VIEW_OBJECT_USER]->userdata = (void*)STG_LAYER_USER;
  win->mitems[STG_MITEM_VIEW_GRID]->userdata = (void*)STG_LAYER_GRID;
  
  // add the callbacks   
  rtk_menuitem_set_callback( win->mitems[STG_MITEM_FILE_MOVIE_START], 
			     gui_menu_movie_start );
  rtk_menuitem_set_callback( win->mitems[STG_MITEM_FILE_MOVIE_STOP], 
			     gui_menu_movie_stop );
  rtk_menuitem_enable( win->mitems[STG_MITEM_FILE_MOVIE_STOP], 0 );
  

  rtk_menuitem_set_callback( win->mitems[STG_MITEM_FILE_SAVE], gui_menu_save );
  rtk_menuitem_set_callback( win->mitems[STG_MITEM_FILE_QUIT], gui_menu_exit );
  
  rtk_menuitem_set_callback( win->mitems[STG_MITEM_VIEW_MATRIX], 
			     gui_menu_matrix );
  
  rtk_menuitem_set_callback( win->mitems[STG_MITEM_VIEW_OBJECT_BODY], 
			     gui_menu_layer );
  rtk_menuitem_set_callback( win->mitems[STG_MITEM_VIEW_OBJECT_SENSOR], 
			     gui_menu_layer );
  rtk_menuitem_set_callback( win->mitems[STG_MITEM_VIEW_OBJECT_LIGHT], 
			     gui_menu_layer );
  rtk_menuitem_set_callback( win->mitems[STG_MITEM_VIEW_OBJECT_USER], 
			     gui_menu_layer );
  rtk_menuitem_set_callback( win->mitems[STG_MITEM_VIEW_GRID], 
			     gui_menu_layer );
    

  rtk_menuitem_set_callback( win->mitems[STG_MITEM_VIEW_DATA_LASER], 
			     gui_menu_view_data_laser );
  rtk_menuitem_set_callback( win->mitems[STG_MITEM_VIEW_DATA_RANGER], 
			     gui_menu_view_data_ranger );

  // set the default checks - the callback functions will set things
  // up properly
  rtk_menuitem_check(win->mitems[STG_MITEM_VIEW_GRID], 0);
  rtk_menuitem_check(win->mitems[STG_MITEM_VIEW_OBJECT_BODY], 1);
  rtk_menuitem_check(win->mitems[STG_MITEM_VIEW_OBJECT_LIGHT], 1);
  rtk_menuitem_check(win->mitems[STG_MITEM_VIEW_OBJECT_SENSOR], 0);
  rtk_menuitem_check(win->mitems[STG_MITEM_VIEW_OBJECT_LIGHT], 1);
  rtk_menuitem_check(win->mitems[STG_MITEM_VIEW_OBJECT_USER], 1);
  rtk_menuitem_check(win->mitems[STG_MITEM_VIEW_DATA_LASER], 1);
  rtk_menuitem_check(win->mitems[STG_MITEM_VIEW_DATA_RANGER], 1);
  rtk_menuitem_check(win->mitems[STG_MITEM_VIEW_DATA_NEIGHBORS], 1);
  rtk_menuitem_check(win->mitems[STG_MITEM_VIEW_DATA_BLOBFINDER], 1);    
}

void gui_window_menu_destroy( gui_window_t* win )
{
  if( win && win->mitems )
    free( win->mitems );

  if( win && win->menus )
    free( win->menus );
}
