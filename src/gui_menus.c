//#define DEBUG

#include "gui.h"
#include "stdlib.h"

extern rtk_fig_t* fig_debug;

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
  STG_MITEM_VIEW_DEBUG,
  STG_MITEM_VIEW_OBJECT,
  STG_MITEM_VIEW_DATA_NEIGHBORS,
  STG_MITEM_VIEW_DATA_LASER,
  STG_MITEM_VIEW_DATA_RANGER,
  STG_MITEM_VIEW_DATA_BLOB,
  STG_MITEM_VIEW_DATA_ENERGY, 
  STG_MITEM_VIEW_GEOM_NEIGHBORS,
  STG_MITEM_VIEW_GEOM_LASER,
  STG_MITEM_VIEW_GEOM_RANGER,
  STG_MITEM_VIEW_GEOM_BLOB,
  STG_MITEM_VIEW_GEOM_ENERGY,
  STG_MITEM_VIEW_CONFIG_NEIGHBORS,
  STG_MITEM_VIEW_CONFIG_LASER,
  STG_MITEM_VIEW_CONFIG_RANGER,
  STG_MITEM_VIEW_CONFIG_BLOB, 
  STG_MITEM_VIEW_CONFIG_ENERGY,
 STG_MITEM_COUNT
};


enum {
  STG_MENU_FILE,
  STG_MENU_FILE_IMAGE,
  STG_MENU_FILE_MOVIE,
  STG_MENU_VIEW,
  STG_MENU_VIEW_GEOM,
  STG_MENU_VIEW_DATA,
  STG_MENU_VIEW_CONFIG,
  STG_MENU_COUNT
};

// movies can be saved at these multiples of real time
const int STG_MOVIE_SPEEDS[] = {1, 2, 5, 10, 20, 50, 100};
const int STG_MOVIE_SPEED_COUNT = 7; // must match the static array length

// send a USR2 signal to the client process that created this menuitem
void gui_menu_save( rtk_menuitem_t *item )
{
  PRINT_DEBUG( "Save menu item" );

  world_t* world = (world_t*)item->userdata;
  stg_id_t wid = world->id;
  
  // tell the client to save the world with this server-side id
  stg_connection_write_msg( world->con, STG_MSG_CLIENT_SAVE, &wid, sizeof(wid) ); 
}

void gui_menu_exit( rtk_menuitem_t *item )
{
  //quit = TRUE;
  PRINT_DEBUG( "Exit menu item" );

  world_t* world = (world_t*)item->userdata;
  stg_id_t wid = world->id;
  
  // tell the client to save the world with this server-side id
  stg_connection_write_msg( world->con, STG_MSG_CLIENT_QUIT, &wid, sizeof(wid) ); 
}

void gui_menu_file_image_jpg( rtk_menuitem_t *item )
{
  static int index=0;

  PRINT_DEBUG( "File/Image/Jpg menu item" );
  
  //snprintf(filename, sizeof(filename), "stage-%03d-%04d.jpg",
  //   this->stills_series, this->stills_count++);
  
  char filename[128];
  snprintf(filename, sizeof(filename), "stage-%03d.jpg", index++);
  printf("saving [%s]\n", filename);
  
  rtk_canvas_export_image(item->menu->canvas, filename, RTK_IMAGE_FORMAT_JPEG);
}

void gui_menu_file_image_ppm( rtk_menuitem_t *item )
{
  static int index=0;

  PRINT_DEBUG( "File/Image/Jpg menu item" );
  
  //snprintf(filename, sizeof(filename), "stage-%03d-%04d.jpg",
  //   this->stills_series, this->stills_count++);
  
  char filename[128];
  snprintf(filename, sizeof(filename), "stage-%03d.ppm", index++);
  printf("saving [%s]\n", filename);
  
  rtk_canvas_export_image(item->menu->canvas, filename, RTK_IMAGE_FORMAT_PPM);
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

gboolean gui_movie_frame_callback( gpointer data )
{
  rtk_canvas_movie_frame( (rtk_canvas_t*)data );
  //gui_poll();
  return TRUE;
}

// toggle movie exports
void gui_menu_movie_stop( rtk_menuitem_t *item )
{
  PRINT_DEBUG( "Movie startstop menuitem" );
  
  gui_window_t* win = (gui_window_t*)item->menu->canvas->userdata;
  
  // stop the frame callback
  g_source_remove(win->movie_tag);
  
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
  
  int interval_ms = win->movie_speed * win->world->wall_interval;

  printf( "starting movie with frame interval %d\n", interval_ms );

  // TODO!
  win->movie_tag = 
    g_timeout_add( interval_ms,
		   gui_movie_frame_callback, 
		   win->canvas );

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

  // invalidate the whole canvas - how?
}

void gui_menu_matrix( rtk_menuitem_t *item )
{
  PRINT_DEBUG1( "menu selected %s.",
	       rtk_menuitem_ischecked( item ) ? "<checked>" : "<unchecked>" );

  gui_window_t* win = (gui_window_t*)item->menu->canvas->userdata;
 
  win->show_matrix = rtk_menuitem_ischecked( item );

  if( win->matrix ) rtk_fig_clear( win->matrix );
}

void gui_menu_debug( rtk_menuitem_t *item )
{
  PRINT_DEBUG1( "menu selected %s.",
	       rtk_menuitem_ischecked( item ) ? "<checked>" : "<unchecked>" );
 
  if(rtk_menuitem_ischecked( item ) )
    {
      fig_debug = rtk_fig_create(item->menu->canvas, NULL, STG_LAYER_DEBUG );
      rtk_fig_color_rgb32( fig_debug, stg_lookup_color(STG_DEBUG_COLOR) );
    }
  else if( fig_debug )
    { 
      rtk_fig_destroy( fig_debug );
      fig_debug = NULL;
    }
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
  
  rtk_menuitem_set_callback( win->mitems[STG_MITEM_FILE_IMAGE_JPG], 
			     gui_menu_file_image_jpg );
  rtk_menuitem_set_callback( win->mitems[STG_MITEM_FILE_IMAGE_PPM], 
			     gui_menu_file_image_ppm );

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


  // create the FILE menu items
  win->mitems[STG_MITEM_FILE_SAVE] = 
    rtk_menuitem_create(win->menus[STG_MENU_FILE], "Save world", 0);
  win->mitems[STG_MITEM_FILE_QUIT] = 
    rtk_menuitem_create(win->menus[STG_MENU_FILE], "Quit world", 0);
  
  // attach objects to the FILE menu items
  win->mitems[STG_MITEM_FILE_SAVE]->userdata = (void*)win->world;
  win->mitems[STG_MITEM_FILE_QUIT]->userdata = (void*)win->world;

  // create the VIEW menu items
  win->mitems[STG_MITEM_VIEW_OBJECT] = 
    rtk_menuitem_create(win->menus[STG_MENU_VIEW], "Models", 1);

  // create the VIEW sub-menus
  win->menus[STG_MENU_VIEW_DATA] = 
    rtk_menu_create_sub(win->menus[STG_MENU_VIEW], "Data");
  win->menus[STG_MENU_VIEW_GEOM] = 
    rtk_menu_create_sub(win->menus[STG_MENU_VIEW], "Geometry");
  win->menus[STG_MENU_VIEW_CONFIG] = 
    rtk_menu_create_sub(win->menus[STG_MENU_VIEW], "Configuration");

  // create the rest of the VIEW menu items
  win->mitems[STG_MITEM_VIEW_GRID] = 
    rtk_menuitem_create(win->menus[STG_MENU_VIEW], "Grid", 1);
  win->mitems[STG_MITEM_VIEW_MATRIX] = 
    rtk_menuitem_create(win->menus[STG_MENU_VIEW], "Matrix", 1);
  win->mitems[STG_MITEM_VIEW_DEBUG] = 
    rtk_menuitem_create(win->menus[STG_MENU_VIEW], "Debug", 1);


  // create the VIEW/DATA menu items
  win->mitems[STG_MITEM_VIEW_DATA_LASER] 
    = rtk_menuitem_create(win->menus[STG_MENU_VIEW_DATA], "Laser", 1);
  win->mitems[STG_MITEM_VIEW_DATA_RANGER] 
    = rtk_menuitem_create(win->menus[STG_MENU_VIEW_DATA], "Ranger", 1);
  win->mitems[STG_MITEM_VIEW_DATA_NEIGHBORS] 
    = rtk_menuitem_create(win->menus[STG_MENU_VIEW_DATA], "Fiducial", 1);
  win->mitems[STG_MITEM_VIEW_DATA_BLOB] 
    = rtk_menuitem_create(win->menus[STG_MENU_VIEW_DATA], "Blobfinder", 1);
  win->mitems[STG_MITEM_VIEW_DATA_ENERGY] 
    = rtk_menuitem_create(win->menus[STG_MENU_VIEW_DATA], "Energy", 1);

  win->mitems[STG_MITEM_VIEW_CONFIG_LASER] 
    = rtk_menuitem_create(win->menus[STG_MENU_VIEW_CONFIG], "Laser", 1);
  win->mitems[STG_MITEM_VIEW_CONFIG_RANGER] 
    = rtk_menuitem_create(win->menus[STG_MENU_VIEW_CONFIG], "Ranger", 1);
  win->mitems[STG_MITEM_VIEW_CONFIG_NEIGHBORS] 
    = rtk_menuitem_create(win->menus[STG_MENU_VIEW_CONFIG], "Fiducial", 1);
  win->mitems[STG_MITEM_VIEW_CONFIG_BLOB] 
    = rtk_menuitem_create(win->menus[STG_MENU_VIEW_CONFIG], "Blobfinder", 1);
  win->mitems[STG_MITEM_VIEW_CONFIG_ENERGY] 
    = rtk_menuitem_create(win->menus[STG_MENU_VIEW_CONFIG], "Energy", 1);

  win->mitems[STG_MITEM_VIEW_GEOM_LASER] 
    = rtk_menuitem_create(win->menus[STG_MENU_VIEW_GEOM], "Laser", 1);
  win->mitems[STG_MITEM_VIEW_GEOM_RANGER] 
    = rtk_menuitem_create(win->menus[STG_MENU_VIEW_GEOM], "Ranger", 1);
  win->mitems[STG_MITEM_VIEW_GEOM_NEIGHBORS] 
    = rtk_menuitem_create(win->menus[STG_MENU_VIEW_GEOM], "Fiducial", 1);
  win->mitems[STG_MITEM_VIEW_GEOM_BLOB] 
    = rtk_menuitem_create(win->menus[STG_MENU_VIEW_GEOM], "Blobfinder", 1);
  win->mitems[STG_MITEM_VIEW_GEOM_ENERGY] 
    = rtk_menuitem_create(win->menus[STG_MENU_VIEW_GEOM], "Energy", 1);

  
  win->mitems[STG_MITEM_VIEW_OBJECT]->userdata = (void*)STG_LAYER_BODY;
  win->mitems[STG_MITEM_VIEW_GRID]->userdata = (void*)STG_LAYER_GRID;

  win->mitems[STG_MITEM_VIEW_DATA_LASER]->userdata =(void*)STG_LAYER_LASERDATA;
  win->mitems[STG_MITEM_VIEW_DATA_RANGER]->userdata =(void*)STG_LAYER_RANGERDATA;
  win->mitems[STG_MITEM_VIEW_DATA_BLOB]->userdata =(void*)STG_LAYER_BLOBDATA;
  win->mitems[STG_MITEM_VIEW_DATA_NEIGHBORS]->userdata =(void*)STG_LAYER_NEIGHBORDATA;
  win->mitems[STG_MITEM_VIEW_DATA_ENERGY]->userdata =(void*)STG_LAYER_ENERGYDATA;

  win->mitems[STG_MITEM_VIEW_CONFIG_LASER]->userdata =(void*)STG_LAYER_LASERCONFIG;
  win->mitems[STG_MITEM_VIEW_CONFIG_RANGER]->userdata =(void*)STG_LAYER_RANGERCONFIG;
  win->mitems[STG_MITEM_VIEW_CONFIG_BLOB]->userdata =(void*)STG_LAYER_BLOBCONFIG;
  win->mitems[STG_MITEM_VIEW_CONFIG_NEIGHBORS]->userdata =(void*)STG_LAYER_NEIGHBORCONFIG;
  win->mitems[STG_MITEM_VIEW_CONFIG_ENERGY]->userdata =(void*)STG_LAYER_ENERGYCONFIG;

  win->mitems[STG_MITEM_VIEW_GEOM_LASER]->userdata =(void*)STG_LAYER_LASERGEOM;
  win->mitems[STG_MITEM_VIEW_GEOM_RANGER]->userdata =(void*)STG_LAYER_RANGERGEOM;
  win->mitems[STG_MITEM_VIEW_GEOM_BLOB]->userdata =(void*)STG_LAYER_BLOBGEOM;
  win->mitems[STG_MITEM_VIEW_GEOM_NEIGHBORS]->userdata =(void*)STG_LAYER_NEIGHBORGEOM;
  win->mitems[STG_MITEM_VIEW_GEOM_ENERGY]->userdata =(void*)STG_LAYER_ENERGYGEOM;

  
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
  
  rtk_menuitem_set_callback( win->mitems[STG_MITEM_VIEW_OBJECT], 
			     gui_menu_layer );

  rtk_menuitem_set_callback( win->mitems[STG_MITEM_VIEW_GRID], 
			     gui_menu_layer );
  
  rtk_menuitem_set_callback( win->mitems[STG_MITEM_VIEW_DEBUG], 
			     gui_menu_debug );
    
  rtk_menuitem_set_callback( win->mitems[STG_MITEM_VIEW_DATA_LASER], 
			     gui_menu_layer );
  rtk_menuitem_set_callback( win->mitems[STG_MITEM_VIEW_DATA_RANGER], 
			     gui_menu_layer );
  rtk_menuitem_set_callback( win->mitems[STG_MITEM_VIEW_DATA_BLOB], 
			     gui_menu_layer );
  rtk_menuitem_set_callback( win->mitems[STG_MITEM_VIEW_DATA_NEIGHBORS], 
			     gui_menu_layer );
  rtk_menuitem_set_callback( win->mitems[STG_MITEM_VIEW_DATA_ENERGY], 
			     gui_menu_layer );

  rtk_menuitem_set_callback( win->mitems[STG_MITEM_VIEW_GEOM_LASER], 
			     gui_menu_layer );
  rtk_menuitem_set_callback( win->mitems[STG_MITEM_VIEW_GEOM_RANGER], 
			     gui_menu_layer );
  rtk_menuitem_set_callback( win->mitems[STG_MITEM_VIEW_GEOM_BLOB], 
			     gui_menu_layer );
  rtk_menuitem_set_callback( win->mitems[STG_MITEM_VIEW_GEOM_NEIGHBORS], 
			     gui_menu_layer );
  rtk_menuitem_set_callback( win->mitems[STG_MITEM_VIEW_GEOM_ENERGY], 
			     gui_menu_layer );

  rtk_menuitem_set_callback( win->mitems[STG_MITEM_VIEW_CONFIG_LASER], 
			     gui_menu_layer );
  rtk_menuitem_set_callback( win->mitems[STG_MITEM_VIEW_CONFIG_RANGER], 
			     gui_menu_layer );
  rtk_menuitem_set_callback( win->mitems[STG_MITEM_VIEW_CONFIG_BLOB], 
			     gui_menu_layer );
  rtk_menuitem_set_callback( win->mitems[STG_MITEM_VIEW_CONFIG_NEIGHBORS], 
			     gui_menu_layer );
  rtk_menuitem_set_callback( win->mitems[STG_MITEM_VIEW_CONFIG_ENERGY], 
			     gui_menu_layer );

  // set the default checks - the callback functions will set things
  // up properly

  // NOTE: work around a curiosity of rtk - we have to check THEN uncheck
  // menu items to get the callback called for non-checked state

  rtk_menuitem_check(win->mitems[STG_MITEM_VIEW_GRID], 1);
  rtk_menuitem_check(win->mitems[STG_MITEM_VIEW_DEBUG], 1);
  rtk_menuitem_check(win->mitems[STG_MITEM_VIEW_DEBUG], 0);
  rtk_menuitem_check(win->mitems[STG_MITEM_VIEW_OBJECT], 1);

  rtk_menuitem_check(win->mitems[STG_MITEM_VIEW_DATA_LASER], 1);
  rtk_menuitem_check(win->mitems[STG_MITEM_VIEW_DATA_RANGER], 1);
  rtk_menuitem_check(win->mitems[STG_MITEM_VIEW_DATA_NEIGHBORS], 1);
  rtk_menuitem_check(win->mitems[STG_MITEM_VIEW_DATA_BLOB], 1);    
  rtk_menuitem_check(win->mitems[STG_MITEM_VIEW_DATA_ENERGY], 1);    

  rtk_menuitem_check(win->mitems[STG_MITEM_VIEW_CONFIG_LASER], 1);
  rtk_menuitem_check(win->mitems[STG_MITEM_VIEW_CONFIG_RANGER], 1);
  rtk_menuitem_check(win->mitems[STG_MITEM_VIEW_CONFIG_NEIGHBORS], 1);
  rtk_menuitem_check(win->mitems[STG_MITEM_VIEW_CONFIG_BLOB], 1);    
  rtk_menuitem_check(win->mitems[STG_MITEM_VIEW_CONFIG_ENERGY], 1);    
  rtk_menuitem_check(win->mitems[STG_MITEM_VIEW_CONFIG_LASER], 0);
  rtk_menuitem_check(win->mitems[STG_MITEM_VIEW_CONFIG_RANGER], 0);
  rtk_menuitem_check(win->mitems[STG_MITEM_VIEW_CONFIG_NEIGHBORS], 0);
  rtk_menuitem_check(win->mitems[STG_MITEM_VIEW_CONFIG_BLOB], 0);    
  rtk_menuitem_check(win->mitems[STG_MITEM_VIEW_CONFIG_ENERGY], 0);    

  rtk_menuitem_check(win->mitems[STG_MITEM_VIEW_GEOM_LASER], 1);
  rtk_menuitem_check(win->mitems[STG_MITEM_VIEW_GEOM_RANGER], 1);
  rtk_menuitem_check(win->mitems[STG_MITEM_VIEW_GEOM_NEIGHBORS], 1);
  rtk_menuitem_check(win->mitems[STG_MITEM_VIEW_GEOM_BLOB], 1);    
  rtk_menuitem_check(win->mitems[STG_MITEM_VIEW_GEOM_ENERGY], 1);    
}

void gui_window_menu_destroy( gui_window_t* win )
{
  if( win && win->mitems )
    free( win->mitems );

  if( win && win->menus )
    free( win->menus );
}
