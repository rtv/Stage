#define DEBUG

#include <stdlib.h>
#include "stage_internal.h"
#include "gui.h"

extern stg_rtk_fig_t* fig_debug_geom;
extern stg_rtk_fig_t* fig_debug_rays;
extern int _stg_quit;

enum {
  STG_LASER_DATA,
  STG_LASER_CMD,
  STG_LASER_CFG
};

// movies can be saved at these multiples of real time
//const int STG_MOVIE_SPEEDS[] = {1, 2, 5, 10, 20, 50, 100};
//const int STG_MOVIE_SPEED_COUNT = 7; // must match the static array length

// declare callbacks for menu items
void gui_menu_polygons_cb( gpointer data, guint action, GtkWidget* mitem );
void gui_menu_debug_cb( gpointer data, guint action, GtkWidget* mitem );
void gui_menu_file_export_frame_cb( gpointer data, guint action, GtkWidget* mitem );
void gui_menu_file_export_sequence_cb( gpointer data, guint action, GtkWidget* mitem );
void gui_menu_file_save_cb( gpointer data, guint action, GtkWidget* mitem );
void gui_menu_file_load_cb( gpointer data, guint action, GtkWidget* mitem );
void gui_menu_file_exit_cb( void );
void gui_menu_layer_cb( gpointer data, guint action, GtkWidget* mitem );
void gui_menu_clock_pause_cb( gpointer data, guint action, GtkWidget* mitem );
//void gui_menu_file_about( void );
void gui_menu_file_export_interval( gpointer data, guint action, GtkWidget* mitem );
void gui_menu_file_export_format( gpointer data, guint action, GtkWidget* mitem );
void gui_menu_view_data( gpointer data, guint action, GtkWidget* mitem );
void gui_menu_view_cfg( gpointer data, guint action, GtkWidget* mitem );
void gui_menu_view_cmd( gpointer data, guint action, GtkWidget* mitem );


static GtkItemFactoryEntry menu_table[] = {
  { "/_File",         NULL,      NULL, 0, "<Branch>" },
  { "/File/tear1",    NULL,      NULL, 0, "<Tearoff>" },
  //  { "/File/About", NULL,  gui_menu_file_about, 0, "<Item>" },
  { "/File/_Save", "<CTRL>S", gui_menu_file_save_cb,  1, "<Item>", GTK_STOCK_SAVE },
  { "/File/_Reset", "<CTRL>R", gui_menu_file_load_cb,  1, "<Item>", GTK_STOCK_OPEN },
  { "/File/Export", NULL,   NULL, 1, "<Branch>" },
  { "/File/Export/Single frame", "<CTRL>J", gui_menu_file_export_frame_cb, 1, "<Item>" },
  { "/File/Export/Sequence of frames", "<CTRL>K",  gui_menu_file_export_sequence_cb, 1, "<CheckItem>" },
  { "/File/Export/sep1",     NULL,      NULL, 0, "<Separator>" },
  { "/File/Export/Frame export interval",     NULL,      NULL, 0, "<Title>" },
  { "/File/Export/0.1 seconds", NULL,  gui_menu_file_export_interval, 1, "<RadioItem>" },
  { "/File/Export/0.2 seconds", NULL, gui_menu_file_export_interval,  2, "/File/Export/0.1 seconds" },
  { "/File/Export/0.5 seconds", NULL, gui_menu_file_export_interval,  5, "/File/Export/0.1 seconds" },
  { "/File/Export/1.0 seconds", NULL, gui_menu_file_export_interval, 10, "/File/Export/0.1 seconds" },
  { "/File/Export/2.0 seconds", NULL, gui_menu_file_export_interval, 20, "/File/Export/0.1 seconds" },
  { "/File/Export/5.0 seconds", NULL, gui_menu_file_export_interval, 50, "/File/Export/0.1 seconds" },
  { "/File/Export/10.0 seconds", NULL, gui_menu_file_export_interval, 100, "/File/Export/0.1 seconds" },
  { "/File/Export/sep2",     NULL,      NULL, 0, "<Separator>" },
  { "/File/Export/Frame export format",     NULL,      NULL, 0, "<Title>" },
  { "/File/Export/PNG", NULL, gui_menu_file_export_format, STK_IMAGE_FORMAT_PNG,  "<RadioItem>" },
  { "/File/Export/JPEG", NULL,  gui_menu_file_export_format, STK_IMAGE_FORMAT_JPEG, "/File/Export/PNG"},
  // PPM and PNM don't seem to work. why?
  // { "/File/Export/PPM", NULL, gui_menu_file_export_format,  STK_IMAGE_FORMAT_PPM, "/File/Export/JPEG" },
  // { "/File/Export/PNM", NULL, gui_menu_file_export_format, STK_IMAGE_FORMAT_PNM, "/File/Export/JPEG" },
  { "/File/sep1",     NULL,      NULL, 0, "<Separator>" },
  { "/File/_Quit",    "<CTRL>Q", gui_menu_file_exit_cb, 0, "<StockItem>", GTK_STOCK_QUIT },
  { "/_View",         NULL,      NULL, 0, "<Branch>" },
  { "/View/tear1",    NULL,      NULL, 0, "<Tearoff>" },
  { "/View/Fill polygons", "<CTRL>P", gui_menu_polygons_cb, 1, "<CheckItem>" },
  { "/View/Grid", "<CTRL>G",   gui_menu_layer_cb, STG_LAYER_GRID, "<CheckItem>" },
  { "/View/sep0",     NULL,      NULL, 0, "<Separator>" },
  { "/View/Data",       NULL,   NULL, 0, "<Title>" },
  { "/View/Blobfinder data", NULL,   gui_menu_view_data, STG_MODEL_BLOB,     "<CheckItem>" },
  { "/View/Energy data",   NULL,   gui_menu_view_data, STG_MODEL_ENERGY, "<CheckItem>" },
  { "/View/Fiducial data",   NULL,   gui_menu_view_data, STG_MODEL_FIDUCIAL, "<CheckItem>" },
  { "/View/Laser data",      NULL,   gui_menu_view_data, STG_MODEL_LASER,    "<CheckItem>" },
  { "/View/Position data",   NULL,   gui_menu_view_data, STG_MODEL_POSITION, "<CheckItem>" },
  { "/View/Ranger data",     NULL,   gui_menu_view_data, STG_MODEL_RANGER,   "<CheckItem>" },
  { "/View/sep1",       NULL,   NULL, 0, "<Separator>" },
  { "/View/Config",          NULL,    NULL, 0, "<Title>" },
  { "/View/Blobfinder config",     NULL,   gui_menu_view_cfg, STG_MODEL_BLOB,     "<CheckItem>" },
  { "/View/Energy config", NULL,   gui_menu_view_cfg, STG_MODEL_ENERGY, "<CheckItem>" },
  { "/View/Fiducial config", NULL,   gui_menu_view_cfg, STG_MODEL_FIDUCIAL, "<CheckItem>" },
  { "/View/Laser config",    NULL,   gui_menu_view_cfg, STG_MODEL_LASER,    "<CheckItem>" },
  { "/View/Ranger config",   NULL,   gui_menu_view_cfg, STG_MODEL_RANGER ,  "<CheckItem>" },  

  { "/View/sep2",     NULL,      NULL, 0, "<Separator>" },
  { "/View/Debug", NULL, NULL, 1, "<Branch>" },
  { "/View/Debug/Raytrace", NULL, gui_menu_debug_cb, 1, "<CheckItem>" },
  { "/View/Debug/Geometry", NULL, gui_menu_debug_cb, 2, "<CheckItem>" },
  { "/View/Debug/Matrix", "<CTRL>M",   gui_menu_debug_cb, 3, "<CheckItem>" },
  { "/_Clock",         NULL,      NULL, 0, "<Branch>" },
  { "/Clock/tear1",    NULL,      NULL, 0, "<Tearoff>" },
  { "/Clock/Pause", NULL, gui_menu_clock_pause_cb, 1, "<CheckItem>" }
};

// SET THIS TO THE NUMBER OF MENU ITEMS IN THE ARRAY ABOVE
static const int menu_table_count = 49;

// USE THIS WHEN WE FIX ON A RECENT GTK VERSION 
/* void gui_menu_file_about( void ) */
/* { */
/*   //gtk_show_about_dialog( NULL, NULL ); */
  
/*   GtkDialog *about = gtk_dialog_new(); */
/*   gtk_window_set_title( about, "About Stage" ); */
/*   gtk_window_resize( about, 200, 200 ); */
/*   gtk_window_set_resizable( about, FALSE ); */
/*   gtk_dialog_run( about ); */
/* } */


// TO-DOUBLE-DO - remove
// TODO - move this somewhere sensible - so far it's only used here
//#define STG_DATA_MAX 32767
//#define STG_CFG_MAX 32767
//#define STG_CMD_MAX 32767


void gui_model_render_data_cb( gpointer key, gpointer value, gpointer user )
{
  gui_model_render_data( (stg_model_t*)value );
}

void gui_model_render_command_cb( gpointer key, gpointer value, gpointer user )
{
  gui_model_render_command( (stg_model_t*)value );
}

void gui_model_render_config_cb( gpointer key, gpointer value, gpointer user )
{
  gui_model_render_config( (stg_model_t*)value );
}


void gui_world_render_data( stg_world_t* world )
{
  g_hash_table_foreach( world->models, gui_model_render_data_cb, NULL );   
}

void gui_world_render_cfg( stg_world_t* world )
{
  g_hash_table_foreach( world->models, gui_model_render_config_cb, NULL );   
}

void gui_world_render_cmd( stg_world_t* world )
{
  g_hash_table_foreach( world->models, gui_model_render_config_cb, NULL );   
}

void gui_menu_view_data( gpointer data, guint action, GtkWidget* mitem )
{
  ((gui_window_t*)data)->render_data_flag[action] = 
    GTK_CHECK_MENU_ITEM(mitem)->active;

  gui_world_render_data( ((gui_window_t*)data)->world );
}

void gui_menu_view_cfg( gpointer data, guint action, GtkWidget* mitem )
{
  ((gui_window_t*)data)->render_cfg_flag[action] = 
    GTK_CHECK_MENU_ITEM(mitem)->active;
  
  gui_world_render_cfg( ((gui_window_t*)data)->world );
}

void gui_menu_view_cmd( gpointer data, guint action, GtkWidget* mitem )
{
  ((gui_window_t*)data)->render_cmd_flag[action] = 
    GTK_CHECK_MENU_ITEM(mitem)->active;

  gui_world_render_cmd( ((gui_window_t*)data)->world );
}


void gui_menu_file_export_interval( gpointer data, guint action, GtkWidget* mitem )
{
  ((gui_window_t*)data)->frame_interval = action * 100;
}

// set the graphics file format for window dumps
void gui_menu_file_export_format( gpointer data, guint action, GtkWidget* mitem )
{
  ((gui_window_t*)data)->frame_format = action;
}

void gui_menu_file_save_cb( gpointer data, 
			    guint action, 
			    GtkWidget* mitem )
{
  stg_world_t* world = ((gui_window_t*)data)->world;
  printf( "Saving world \"%s\"\n",  world->token );
  stg_world_save( world );
}

void gui_menu_file_load_cb( gpointer data, 
			    guint action, 
			    GtkWidget* mitem )
{
  stg_world_t* world = ((gui_window_t*)data)->world;
  printf( "Resetting to saved state \"%s\"\n",  world->token );
  stg_world_reload( world );
}

void gui_menu_clock_pause_cb( gpointer data, 
			      guint action, 
			      GtkWidget* mitem )
{
  ((gui_window_t*)data)->world->paused = GTK_CHECK_MENU_ITEM(mitem)->active;
}


void gui_menu_file_exit_cb( void )
{
  PRINT_DEBUG( "Exit menu item" );
  _stg_quit = TRUE;
}

/// save a frame as a jpeg, with incremental index numbers. if series
/// is greater than 0, its value is incorporated into the filename
void export_window( gui_window_t* win  ) //stg_rtk_canvas_t* canvas, int series )
{
  char filename[128];

  win->frame_index++;

  char* suffix;
  switch( win->frame_format )
    {
    case STK_IMAGE_FORMAT_JPEG: suffix = "jpg"; break;
    case STK_IMAGE_FORMAT_PNM: suffix = "pnm"; break;
    case STK_IMAGE_FORMAT_PNG: suffix = "png"; break;
    case STK_IMAGE_FORMAT_PPM: suffix = "ppm"; break;
    default:
      suffix = ".png";
    }

  if( win->frame_series > 0 )
    snprintf(filename, sizeof(filename), "stage-%03d-%04d.%s",
	     win->frame_series, win->frame_index, suffix);
  else
    snprintf(filename, sizeof(filename), "stage-%03d.%s", win->frame_index, suffix);

  printf("Stage: saving [%s]\n", filename);
  
  stg_rtk_canvas_export_image( win->canvas, filename, win->frame_format );
}

void gui_menu_file_export_frame_cb( gpointer data, 
				    guint action, 
				    GtkWidget* mitem )
{
  PRINT_DEBUG( "File/Image/Save frame menu item");
  export_window( (gui_window_t*)data );
}

gboolean frame_callback( gpointer data )
{
  export_window( (gui_window_t*)data );
  return TRUE;
}

void gui_menu_file_export_sequence_cb( gpointer data, 
				       guint action, 
				       GtkWidget* mitem )
{
  PRINT_DEBUG( "File/Export/Sequence menu item" );
  
  gui_window_t* win = (gui_window_t*)data;

  if(GTK_CHECK_MENU_ITEM(mitem)->active)
    {
      PRINT_DEBUG( "sequence start" );
      // advance the sequence counter - the first sequence is 1.
      win->frame_series++;
      win->frame_index = 0;
      
      printf( "callback with interval %d\n", win->frame_interval );

      win->frame_callback_tag =
	g_timeout_add( win->frame_interval,
		       frame_callback,
		       (gpointer)win );
    }
  else
    {
      PRINT_DEBUG( "sequence stop" );
      // stop the frame callback
      g_source_remove(win->frame_callback_tag); 
    }
}


void gui_menu_layer_cb( gpointer data, 
			guint action, 
			GtkWidget* mitem )    
{
  // show or hide the layer depending on the state of the menu item
  stg_rtk_canvas_layer_show( ((gui_window_t*)data)->canvas, 
			 action, // action is the layer number
			 GTK_CHECK_MENU_ITEM(mitem)->active );
}

void model_render_polygons_cb( gpointer key, gpointer data, gpointer user )
{
  stg_model_render_polygons( (stg_model_t*)data );
}


void gui_menu_polygons_cb( gpointer data, guint action, GtkWidget* mitem )
{
  gui_window_t* win = (gui_window_t*)data;
  win->fill_polygons = GTK_CHECK_MENU_ITEM(mitem)->active;

  // redraw everything to see the polygons change 
  //g_hash_table_foreach( win->world->models, refresh_cb, NULL ); 
  g_hash_table_foreach( win->world->models, model_render_polygons_cb, NULL ); 
}
  
void gui_menu_debug_cb( gpointer data, guint action, GtkWidget* mitem )
{
  gui_window_t* win = (gui_window_t*)data;

  switch( action )
    {
    case 1: // raytrace
      if(GTK_CHECK_MENU_ITEM(mitem)->active)
	{
	  fig_debug_rays = stg_rtk_fig_create( win->canvas, NULL, STG_LAYER_DEBUG );
	  stg_rtk_fig_color_rgb32( fig_debug_rays, stg_lookup_color(STG_DEBUG_COLOR) );
	}
      else if( fig_debug_rays )
	{ 
	  stg_rtk_fig_destroy( fig_debug_rays );
	  fig_debug_rays = NULL;
	}
      break;
    case 2: // geometry
      if(GTK_CHECK_MENU_ITEM(mitem)->active)
	{
	  fig_debug_geom = stg_rtk_fig_create( win->canvas, NULL, STG_LAYER_DEBUG );
	  stg_rtk_fig_color_rgb32( fig_debug_geom, stg_lookup_color(STG_DEBUG_COLOR) );
	}
      else if( fig_debug_geom )
	{ 
	  stg_rtk_fig_destroy( fig_debug_geom );
	  fig_debug_geom = NULL;
	}
      break;

    case 3: // matrix
      win->show_matrix = GTK_CHECK_MENU_ITEM(mitem)->active;
      if( win->matrix ) stg_rtk_fig_clear( win->matrix );     
      break;
      
    default:
      PRINT_WARN1( "unknown debug menu item %d", action );
    }
}


void gui_window_menus_create( gui_window_t* win )
{
  GtkAccelGroup* ag = gtk_accel_group_new();
  
  GtkItemFactory* fac = gtk_item_factory_new( GTK_TYPE_MENU_BAR,
						  "<main>", ag );
  
  gtk_item_factory_create_items( fac, 
				 menu_table_count, 
				 menu_table, 
				 (gpointer)win );

  /* Attach the new accelerator group to the window. */
  gtk_window_add_accel_group (GTK_WINDOW(win->canvas->frame), ag );

  win->canvas->menu_bar = gtk_item_factory_get_widget( fac, "<main>" );
  
  GtkCheckMenuItem* mitem;

  // View
  mitem = GTK_CHECK_MENU_ITEM(gtk_item_factory_get_item(fac, "/View/Grid"));
  gtk_check_menu_item_set_active( mitem, TRUE );

  //mitem = GTK_CHECK_MENU_ITEM(gtk_item_factory_get_item(fac, "/View/Debug"));
  //gtk_check_menu_item_set_active( mitem, FALSE );
  //mitem = GTK_CHECK_MENU_ITEM(gtk_item_factory_get_item(fac, "/View/Matrix"));
  //gtk_check_menu_item_set_active( mitem, FALSE );

  mitem = GTK_CHECK_MENU_ITEM(gtk_item_factory_get_item(fac, "/View/Fill polygons"));
  gtk_check_menu_item_set_active( mitem, TRUE );

  // View/Data
  mitem = GTK_CHECK_MENU_ITEM(gtk_item_factory_get_item(fac, "/View/Laser data"));
  gtk_check_menu_item_set_active( mitem, TRUE );
  mitem = GTK_CHECK_MENU_ITEM(gtk_item_factory_get_item(fac, "/View/Ranger data"));
  gtk_check_menu_item_set_active( mitem, TRUE );
  mitem = GTK_CHECK_MENU_ITEM(gtk_item_factory_get_item(fac, "/View/Fiducial data"));
  gtk_check_menu_item_set_active( mitem, TRUE );
  mitem = GTK_CHECK_MENU_ITEM(gtk_item_factory_get_item(fac, "/View/Blobfinder data"));
  gtk_check_menu_item_set_active( mitem, TRUE );
  mitem = GTK_CHECK_MENU_ITEM(gtk_item_factory_get_item(fac, "/View/Energy data"));
  gtk_check_menu_item_set_active( mitem, FALSE );
  mitem = GTK_CHECK_MENU_ITEM(gtk_item_factory_get_item(fac, "/View/Position data"));
  gtk_check_menu_item_set_active( mitem, FALSE );

  // View/Config
  mitem = GTK_CHECK_MENU_ITEM(gtk_item_factory_get_item(fac, "/View/Laser config"));
  gtk_check_menu_item_set_active( mitem, FALSE );
  mitem = GTK_CHECK_MENU_ITEM(gtk_item_factory_get_item(fac, "/View/Ranger config"));
  gtk_check_menu_item_set_active( mitem, FALSE );
  mitem = GTK_CHECK_MENU_ITEM(gtk_item_factory_get_item(fac, "/View/Fiducial config"));
  gtk_check_menu_item_set_active( mitem, FALSE );
  mitem = GTK_CHECK_MENU_ITEM(gtk_item_factory_get_item(fac, "/View/Blobfinder config"));
  gtk_check_menu_item_set_active( mitem, FALSE );
  mitem = GTK_CHECK_MENU_ITEM(gtk_item_factory_get_item(fac, "/View/Energy config"));
  gtk_check_menu_item_set_active( mitem, FALSE );
  
  gtk_box_pack_start(GTK_BOX(win->canvas->layout), 
		     win->canvas->menu_bar, FALSE, TRUE, 0);
}


void gui_window_menu_destroy( gui_window_t* win )
{
  // TODO - destroy everything
}
