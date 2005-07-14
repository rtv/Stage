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

static GtkUIManager *ui_manager = NULL;


// movies can be saved at these multiples of real time
//const int STG_MOVIE_SPEEDS[] = {1, 2, 5, 10, 20, 50, 100};
//const int STG_MOVIE_SPEED_COUNT = 7; // must match the static array length

// regular actions
void gui_action_about( GtkAction *action, gpointer user_data);
void gui_action_save( GtkAction *action, gpointer user_data);
void gui_action_reset( GtkAction* action, gpointer userdata );
void gui_action_exit( GtkAction* action, gpointer userdata );
void gui_action_exportframe( GtkAction* action, gpointer userdata );

// toggle actions
void gui_action_exportsequence( GtkToggleAction* action, gpointer userdata );
void gui_action_pause( GtkToggleAction* action, gpointer userdata );
void gui_action_polygons( GtkToggleAction* action, gpointer userdata ); 
void gui_action_disable_polygons( GtkToggleAction* action, gpointer userdata ); 
void gui_action_grid( GtkToggleAction* action, gpointer userdata ); 
void gui_action_raytrace( GtkToggleAction* action, gpointer userdata ); 
void gui_action_geom( GtkToggleAction* action, gpointer userdata ); 
void gui_action_matrixtree( GtkToggleAction* action, gpointer userdata ); 
void gui_action_matrixocc( GtkToggleAction* action, gpointer userdata ); 

// radio actions
void gui_action_export_interval( GtkRadioAction* action, 
				 GtkRadioAction *current, 
				 gpointer userdata );
void gui_action_export_format( GtkRadioAction* action, 	 
			       GtkRadioAction *current, 
			       gpointer userdata );

/* Normal items */
static GtkActionEntry entries[] = {
  { "File", NULL, "_File" },
  { "View", NULL, "_View" },
  { "Debug", NULL, "_Debug" },
  { "Clock", NULL, "_Clock" },
  { "Help", NULL, "_Help" },
  { "Model", NULL, "_Models" },
  { "Export", NULL, "_Export" },
  { "ExportInterval", NULL, "Export interval" },
  { "ExportFormat", NULL, "Export bitmap format" },
  { "ExportFrame", NULL, "Single frame", "<ctrl>f", "Export a screenshot to disk", G_CALLBACK(gui_action_exportframe) },
  { "About", NULL, "About Stage", NULL, NULL, G_CALLBACK(gui_action_about) },
  { "Save", GTK_STOCK_SAVE, "_Save", "<control>S", "Save world state", G_CALLBACK(gui_action_save) },
  { "Reset", GTK_STOCK_OPEN, "_Reset", "<control>R", "Reset to last saved world state", G_CALLBACK(gui_action_reset) },
  { "Exit", GTK_STOCK_QUIT, "E_xit", "<control>Q", "Exit the program", G_CALLBACK(gui_action_exit) },
};

/* Toggle items */
static GtkToggleActionEntry toggle_entries[] = {
  { "Polygons", NULL, "Fill _polygons", "F", "Toggle drawing of filled or outlined polygons", G_CALLBACK(gui_action_polygons), 1 },
  { "DisablePolygons", NULL, "Show polygons", "D", NULL, G_CALLBACK(gui_action_disable_polygons), 1 },
  { "Grid", NULL, "_Grid", "G", "Toggle drawing of 1-metre grid", G_CALLBACK(gui_action_grid), 1 },
  { "Pause", NULL, "_Pause", "P", "Pause the simulation clock", G_CALLBACK(gui_action_pause), 0 },
  { "DebugRays", NULL, "_Raytrace", "<alt>R", "Draw sensor rays", G_CALLBACK(gui_action_raytrace), 0 },
  { "DebugGeom", NULL, "_Geometry", "<alt>G", "Draw model geometry", G_CALLBACK(gui_action_geom), 0 },
  { "DebugMatrixTree", NULL, "Matrix _Tree", "<alt>T", "Show occupancy quadtree", G_CALLBACK(gui_action_matrixtree), 0 },
  { "DebugMatrixOccupancy", NULL, "Matrix _Occupancy", "<alt>M", "Show occupancy grid", G_CALLBACK(gui_action_matrixocc), 0 },
  { "ExportSequence", NULL, "Sequence of frames", "<control>G", "Export a sequence of screenshots at regular intervals", G_CALLBACK(gui_action_exportsequence), 0 },
};

/* Radio items */
static GtkRadioActionEntry export_format_entries[] = {
  { "ExportFormatPNG", NULL, "PNG", NULL, "Portable Network Graphics format", STK_IMAGE_FORMAT_PNG },
  { "ExportFormatJPEG", NULL, "JPEG", NULL, "JPEG format", STK_IMAGE_FORMAT_JPEG },
};
 
static GtkRadioActionEntry export_freq_entries[] = {
  { "ExportInterval1", NULL, "0.1 seconds", NULL, NULL, 100 },
  { "ExportInterval2", NULL, "0.2 seconds", NULL, NULL, 200 },
  { "ExportInterval5", NULL, "0.5 seconds", NULL, NULL, 500 },
  { "ExportInterval10", NULL, "1 second", NULL, NULL, 1000 },
  { "ExportInterval50", NULL, "5 seconds", NULL, NULL, 5000 },
  { "ExportInterval100", NULL, "10 seconds", NULL, NULL, 10000 },
};

static const char *ui_description =
"<ui>"
"  <menubar name='Main'>"
"    <menu action='File'>"
"      <menuitem action='Save'/>"
"      <menuitem action='Reset'/>"
"      <separator/>"
"        <menu action='Export'>"
"          <menuitem action='ExportFrame'/>" 
"          <menuitem action='ExportSequence'/>" 
"          <separator/>" 
"          <menuitem action='ExportFormat'/>"
"          <menuitem action='ExportFormatPNG'/>"
"          <menuitem action='ExportFormatJPEG'/>"
"          <separator/>"
"          <menuitem action='ExportInterval'/>"
"          <menuitem action='ExportInterval1'/>"
"          <menuitem action='ExportInterval2'/>"
"          <menuitem action='ExportInterval5'/>"
"          <menuitem action='ExportInterval10'/>"
"          <menuitem action='ExportInterval50'/>"
"          <menuitem action='ExportInterval100'/>"
"        </menu>"
"      <separator/>"
"      <menuitem action='Exit'/>"
"    </menu>"
"    <menu action='View'>"
"      <menuitem action='DisablePolygons'/>"
"      <menuitem action='Polygons'/>"
"      <menuitem action='Grid'/>"
"      <separator/>"
"        <menu action='Debug'>"
"          <menuitem action='DebugRays'/>"
"          <menuitem action='DebugGeom'/>"
"          <menuitem action='DebugMatrixTree'/>"
"          <menuitem action='DebugMatrixOccupancy'/>"
"          <separator/>"
"            <menu action='Model'>"
"              <separator/>"
"            </menu>"
"        </menu>"
"      <separator/>"
"    </menu>"
"    <menu action='Clock'>"
"      <menuitem action='Pause'/>"
"    </menu>"
"    <menu action='Help'>"
"      <menuitem action='About'/>"
"    </menu>"
"  </menubar>"
"</ui>";


void gui_action_about( GtkAction *action, gpointer user_data)
{
  // USE THIS WHEN WE CAN USED GTK+-2.6
  //GtkAboutDialog *about = gtk_about_dialog_new();
  // gtk_show_about_dialog( about );

  GtkMessageDialog* about = (GtkMessageDialog*)
    gtk_message_dialog_new( NULL, 0, 
			    GTK_MESSAGE_INFO,
			    GTK_BUTTONS_CLOSE, 
			    "Stage" );
  
  const char* str =  "Version %s\n"
    "Part of the Player/Stage Project\n"
    "[http://playerstage.sourceforge.net]\n\n"
    "Copyright Richard Vaughan, Andrew Howard, Brian Gerkey\n"
    " and contributors 2000-2005.\n\n"
    "Released under the GNU General Public License.";
  
  gtk_message_dialog_format_secondary_text( about, 
					    str,
					    PACKAGE_VERSION );
  
  g_signal_connect (about, "destroy",
		    G_CALLBACK(gtk_widget_destroyed), &about);
  
  /* Destroy the dialog when the user responds to it (e.g. clicks a button) */
  g_signal_connect_swapped( about, "response",
			    G_CALLBACK (gtk_widget_destroy),
			    about );

  gtk_widget_show(GTK_WIDGET(about));
}



void gui_action_export_interval( GtkRadioAction* action, 
				 GtkRadioAction *current, 
				 gpointer userdata )
{
  // set the frame interval in milliseconds
  ((gui_window_t*)userdata)->frame_interval = 
    gtk_radio_action_get_current_value(action);
  
  printf("frame export interval now %d ms\n", 
	 ((gui_window_t*)userdata)->frame_interval );
}

// set the graphics file format for window dumps
void gui_action_export_format( GtkRadioAction* action, 
			       GtkRadioAction *current, 
			       gpointer userdata )
{
  ((gui_window_t*)userdata)->frame_format = 
    gtk_radio_action_get_current_value(action);
}


void gui_action_save( GtkAction* action, gpointer userdata )
{
  stg_world_t* world = (stg_world_t*)userdata;
  printf( "Saving world \"%s\"\n",  world->token );
  stg_world_save( world );
}

void gui_action_reset( GtkAction* action, gpointer userdata )
{
  stg_world_t* world = (stg_world_t*)userdata;
  printf( "Resetting to saved state \"%s\"\n",  world->token );
  stg_world_reload( world );
}


void gui_action_pause( GtkToggleAction* action, void* userdata )
{
  PRINT_DEBUG( "Pause menu item" );
  ((stg_world_t*)userdata)->paused = gtk_toggle_action_get_active( action );

}

void gui_action_raytrace( GtkToggleAction* action, gpointer userdata )
{
  PRINT_DEBUG( "Raytrace menu item" );  
  stg_world_t* world = (stg_world_t*)userdata;

  if( gtk_toggle_action_get_active( action ) )
    {
      fig_debug_rays = stg_rtk_fig_create( world->win->canvas, NULL, STG_LAYER_DEBUG );
      stg_rtk_fig_color_rgb32( fig_debug_rays, stg_lookup_color(STG_DEBUG_COLOR) );
    }
  else if( fig_debug_rays )
    { 
      stg_rtk_fig_destroy( fig_debug_rays );
      fig_debug_rays = NULL;
    }
}

void gui_action_geom( GtkToggleAction* action, gpointer userdata )
{
  PRINT_DEBUG( "Geom menu item" );
  
  stg_world_t* world = (stg_world_t*)userdata;
  
  if( gtk_toggle_action_get_active( action ) )
    {
      fig_debug_geom = stg_rtk_fig_create( world->win->canvas, NULL, STG_LAYER_GEOM );
      stg_rtk_fig_color_rgb32( fig_debug_geom, 0xFF0000 );
    }
  else if( fig_debug_geom )
    { 
      stg_rtk_fig_destroy( fig_debug_geom );
      fig_debug_geom = NULL;
    }
}

void gui_action_matrixtree( GtkToggleAction* action, gpointer userdata )
{
  PRINT_DEBUG( "Matrix tree menu item" );

  stg_world_t* world = (stg_world_t*)userdata;  
  if( world->win->matrix_tree ) 
    {
      stg_rtk_fig_destroy( world->win->matrix_tree );     
      world->win->matrix_tree = NULL;
    }
  else
    {
      world->win->matrix_tree = 
	stg_rtk_fig_create(world->win->canvas,world->win->bg,STG_LAYER_MATRIX_TREE);  
      stg_rtk_fig_color_rgb32( world->win->matrix_tree, 0x00CC00 );
    } 
}

void gui_action_matrixocc( GtkToggleAction* action, gpointer userdata ) 
{
  PRINT_DEBUG( "Matrix occupancy menu item" );
  stg_world_t* world = (stg_world_t*)userdata;
  
  if( world->win->matrix ) 
    {
      stg_rtk_fig_destroy( world->win->matrix );     
      world->win->matrix = NULL;
    }
  else
    {
      world->win->matrix = 
	stg_rtk_fig_create(world->win->canvas,world->win->bg,STG_LAYER_MATRIX);	  
      
      stg_rtk_fig_color_rgb32( world->win->matrix, 0x008800 );
    }
}


void gui_add_view_item( const gchar *name,
			const gchar *label,
			const gchar *tooltip,
			GCallback callback,
			gboolean  is_active,
			void* userdata )
{
  static GtkActionGroup* grp = NULL;  
  if( grp == NULL )
    {
      grp = gtk_action_group_new( "DynamicDataActions" );
    }
  
  GtkToggleActionEntry entry;
  memset( &entry, 0, sizeof(entry));
  entry.name = name;
  entry.label = label;
  entry.tooltip = tooltip;
  entry.callback = callback;
  entry.is_active = is_active;

  gtk_action_group_add_toggle_actions( grp, &entry, 1, userdata   );  
  gtk_ui_manager_insert_action_group(ui_manager, grp, 0);

  guint merge = gtk_ui_manager_new_merge_id( ui_manager );
  gtk_ui_manager_add_ui( ui_manager, 
			 merge,
			 "/Main/View", 
			 name, 
			 name, 
			 GTK_UI_MANAGER_AUTO, 
			 FALSE );
  
  // send the toggle signal to get the item set up
  GtkToggleAction *act = (GtkToggleAction*)gtk_action_group_get_action( grp, name );
  gtk_toggle_action_set_active( act, FALSE );
  
  // seems like this ought to work, but it doesn't. :(
  gtk_toggle_action_toggled( act );
}


void test( GtkMenuItem* item, void* userdata )
{
  stg_model_t* mod = (stg_model_t*)userdata;
  printf( "TEST model %s\n", mod->token );

  // TODO - tree text view of models
/*   GtkDialog* win = gtk_dialog_new_with_buttons ("Model inspector", */
/* 						NULL, */
/* 						0, */
/* 						GTK_STOCK_CLOSE, */
/* 						GTK_RESPONSE_CLOSE, */
/* 						NULL); */

/*   gtk_widget_show( win ); */
}

void gui_add_tree_item( stg_model_t* mod )
{
  GtkWidget* m = 
    gtk_ui_manager_get_widget( ui_manager, "/Main/View/Debug/Model" );
  assert(m);
  
  GtkWidget* menu = GTK_WIDGET(gtk_menu_item_get_submenu(GTK_MENU_ITEM(m)));

  GtkWidget* item = gtk_menu_item_new_with_label(  mod->token );
  assert(item);

  gtk_widget_set_sensitive( item, FALSE );

  //GtkWidget* submenu = gtk_menu_new();
  //gtk_menu_item_set_submenu( item, submenu );
  
  //g_signal_connect( item, "activate", test, (void*)mod );
 
  gtk_menu_shell_append( (GtkMenuShell*)menu, item );

  gtk_widget_show(item);
}

void gui_action_exit( GtkAction* action, void* userdata )
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


gboolean frame_callback( gpointer data )
{
  export_window( (gui_window_t*)data );
  return TRUE;
}

void gui_action_exportsequence( GtkToggleAction* action, void* userdata )
{
  gui_window_t* win = ((stg_world_t*)userdata)->win;
  
  if( gtk_toggle_action_get_active(action) )
    {
      PRINT_DEBUG1( "Saving frame sequence with interval %d", 
		    win->frame_interval );

      // advance the sequence counter - the first sequence is 1.
      win->frame_series++;
      win->frame_index = 0;
      
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


void model_render_polygons_cb( gpointer key, gpointer data, gpointer user )
{
  stg_model_render_polygons( (stg_model_t*)data );
}


void gui_action_exportframe( GtkAction* action, void* userdata ) 
{ 
  stg_world_t* world = (stg_world_t*)userdata;
  export_window( world->win );
}


void gui_action_polygons( GtkToggleAction* action, void* userdata )
{
  PRINT_DEBUG( "Polygons menu item" );
  stg_world_t* world = (stg_world_t*)userdata;
  
  world->win->fill_polygons = gtk_toggle_action_get_active( action );
  g_hash_table_foreach( world->models, model_render_polygons_cb, NULL ); 
}

void gui_action_disable_polygons( GtkToggleAction* action, void* userdata )
{
  PRINT_DEBUG( "Disable polygons menu item" );
  stg_world_t* world = (stg_world_t*)userdata;
  
  // show or hide the layer depending on the state of the menu item
  //world->win->show_polygons = ! gtk_toggle_action_get_active( action );
  // show or hide the layer depending on the state of the menu item
  stg_rtk_canvas_layer_show( world->win->canvas, 
			     STG_LAYER_BODY,
			     gtk_toggle_action_get_active( action ));
}

void gui_action_grid( GtkToggleAction* action, void* userdata )
{
  PRINT_DEBUG( "Grid menu item" );
  stg_world_t* world = (stg_world_t*)userdata;
  
  // show or hide the layer depending on the state of the menu item
  stg_rtk_canvas_layer_show( world->win->canvas, 
			     STG_LAYER_GRID,
			     gtk_toggle_action_get_active( action ));
}

void gui_window_menus_create( gui_window_t* win )
{
  ui_manager = gtk_ui_manager_new ();
  
  // actions
  GtkActionGroup *group = gtk_action_group_new ("MenuActions");
  
  gtk_action_group_add_actions( group, entries, 
				G_N_ELEMENTS (entries),
				win->world );
  
  gtk_action_group_add_toggle_actions( group,toggle_entries, 
				       G_N_ELEMENTS(toggle_entries), 
				       win->world );
  
  gtk_action_group_add_radio_actions( group, export_format_entries, 
				      G_N_ELEMENTS (export_format_entries), 
				      win->frame_format, 
				      gui_action_export_format,
				      win );
  
  gtk_action_group_add_radio_actions( group, export_freq_entries, 
				      G_N_ELEMENTS (export_freq_entries),
				      win->frame_interval, 
				      gui_action_export_interval, 
				      win );
  
  gtk_ui_manager_insert_action_group (ui_manager, group, 0);

  // grey-out the radio list labels
  gtk_action_set_sensitive(gtk_action_group_get_action( group, "ExportFormat" ), FALSE );
  gtk_action_set_sensitive(gtk_action_group_get_action( group, "ExportInterval" ), FALSE );

  // accels
  GtkAccelGroup *accel_group = gtk_ui_manager_get_accel_group (ui_manager);
  gtk_window_add_accel_group(GTK_WINDOW (win->canvas->frame), accel_group);
  
  // menus
  gtk_ui_manager_set_add_tearoffs( ui_manager, TRUE );
  
  GError* error = NULL;
  if (!gtk_ui_manager_add_ui_from_string (ui_manager, ui_description, -1, &error))
    {
      g_message ("building menus failed: %s", error->message);
      g_error_free (error);
      exit (EXIT_FAILURE);
    }
  
  // install the widget
  GtkWidget *menubar = gtk_ui_manager_get_widget (ui_manager, "/Main");
  gtk_box_pack_start (GTK_BOX(win->canvas->layout), menubar, FALSE, FALSE, 0);
  
}


void toggle_property_callback( GtkToggleAction* action, void* userdata )
{
  stg_property_toggle_args_t* args = 
    (stg_property_toggle_args_t*)userdata;  
  
  if( gtk_toggle_action_get_active( action ) )
    {
      if( args->callback_off )
	{
	  //printf( "removing OFF callback\n" );
	  stg_model_remove_property_callback( args->mod, 
					    args->propname, 
					    args->callback_off );
	}
      
      if( args->callback_on )
	{
	  //printf( "adding ON callback with args %s\n", (char*)args->arg_on );
	  stg_model_add_property_callback( args->mod, 
					 args->propname, 
					 args->callback_on, 
					 args->arg_on );
	}
    }
  else
    {
      if( args->callback_on )
	{
	  //printf( "removing ON callback\n" );
	  stg_model_remove_property_callback( args->mod, 
					    args->propname, 
					    args->callback_on );
	}
      
      if( args->callback_off )
	{
	  //printf( "adding OFF callback with args %s\n", (char*)args->arg_off );
	  stg_model_add_property_callback( args->mod, 
					   args->propname, 
					   args->callback_off, 
					   args->arg_off );
	}
      
    }
}

void stg_model_add_property_toggles( stg_model_t* mod, 
				     const char* propname, 
				     stg_property_callback_t callback_on,
				     void* arg_on,
				     stg_property_callback_t callback_off,
				     void* arg_off,
				     const char* label,
				     int enabled )
{
  stg_property_toggle_args_t* args = 
    calloc(sizeof(stg_property_toggle_args_t),1);
  
  args->mod = mod;
  args->propname = propname;
  args->callback_on = callback_on;
  args->callback_off = callback_off;
  args->arg_on = arg_on;
  args->arg_off = arg_off;
  
  static GtkActionGroup* grp = NULL;  
  if( ! grp )
    grp = gtk_action_group_new( "DynamicDataActions" );
  
  // find the action associated with this propname
  GtkAction* act = gtk_action_group_get_action( grp, propname );
  
  if( act == NULL )
    {
      //printf( "creating new action/item for prop %s\n", propname );
      
      GtkToggleActionEntry entry;
      memset( &entry, 0, sizeof(entry));
      entry.name = propname;
      entry.label = label;
      entry.tooltip = NULL;
      entry.callback = G_CALLBACK(toggle_property_callback);
      entry.is_active = !enabled; // invert the starting setting - see below
      
      gtk_action_group_add_toggle_actions( grp, &entry, 1, args  );  

      gtk_ui_manager_insert_action_group(ui_manager, grp, 0);
      
      guint merge = gtk_ui_manager_new_merge_id( ui_manager );
      gtk_ui_manager_add_ui( ui_manager, 
			     merge,
			     "/Main/View", 
			     propname, 
			     propname, 
			     GTK_UI_MANAGER_AUTO, 
			     FALSE );
      
      act = gtk_action_group_get_action( grp, propname );
    }
  else
    {
      //printf( "connecting to signal for model %s prop %s\n",
      //      mod->token, propname );

      g_signal_connect( act, "activate",  G_CALLBACK(toggle_property_callback), args );
    }
  
  // causes the callbacks to be called - un-inverts the starting setting!
  gtk_action_activate( act ); 
}


