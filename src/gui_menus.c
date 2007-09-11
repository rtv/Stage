//#define DEBUG

#include <stdlib.h>
#include "stage_internal.h"
#include "gui.h"

#define TOGGLE_PATH "/Main/View"

extern stg_rtk_fig_t* fig_debug_geom;
extern stg_rtk_fig_t* fig_debug_rays;
extern stg_rtk_fig_t* fig_debug_matrix;
extern stg_rtk_fig_t* fig_trails;

extern int _render_matrix_deltas;
extern int _stg_quit;

static GtkUIManager *ui_manager = NULL;

// regular actions
void gui_action_about( GtkAction *action, gpointer user_data);
void gui_action_prefs( GtkAction *action, gpointer user_data);
void gui_action_save( GtkAction *action, gpointer user_data);
void gui_action_reset( GtkAction* action, gpointer userdata );
void gui_action_exit( GtkAction* action, gpointer userdata );
void gui_action_exportframe( GtkAction* action, gpointer userdata );

// toggle actions
void gui_action_exportsequence( GtkToggleAction* action, gpointer userdata );
void gui_action_pause( GtkToggleAction* action, gpointer userdata );
void gui_action_polygons( GtkToggleAction* action, gpointer userdata ); 
void gui_action_trails( GtkToggleAction* action, gpointer userdata ); 
void gui_action_disable_polygons( GtkToggleAction* action, gpointer userdata ); 
void gui_action_grid( GtkToggleAction* action, gpointer userdata ); 
void gui_action_raytrace( GtkToggleAction* action, gpointer userdata ); 
void gui_action_geom( GtkToggleAction* action, gpointer userdata ); 
void gui_action_matrixtree( GtkToggleAction* action, gpointer userdata ); 
void gui_action_matrixocc( GtkToggleAction* action, gpointer userdata ); 
void gui_action_matrixdelta( GtkToggleAction* action, gpointer userdata );

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
  { "Edit", NULL, "_Edit" },
  { "Clock", NULL, "_Clock" },
  { "Help", NULL, "_Help" },
  { "Model", NULL, "_Models" },
  { "Screenshot", NULL, "Scr_eenshot" },
  { "ExportInterval", NULL, "Screenshot interval" },
  { "ExportFormat", NULL, "Screenshot bitmap format" },
  { "ExportFrame", NULL, "Single frame", "<ctrl>f", "Export a screenshot to disk", G_CALLBACK(gui_action_exportframe) },
  { "About", NULL, "About Stage", NULL, NULL, G_CALLBACK(gui_action_about) },
  { "Preferences", NULL, "Preferences...", NULL, NULL, G_CALLBACK(gui_action_prefs) },
  { "Save", GTK_STOCK_SAVE, "_Save", "<control>S", "Save world state", G_CALLBACK(gui_action_save) },
  { "Reset", GTK_STOCK_OPEN, "_Reset", "<control>R", "Reset to last saved world state", G_CALLBACK(gui_action_reset) },
  { "Exit", GTK_STOCK_QUIT, "E_xit", "<control>Q", "Exit the program", G_CALLBACK(gui_action_exit) },
};

/* Toggle items */
static GtkToggleActionEntry toggle_entries[] = {
  { "Pause", NULL, "_Pause", "P", "Pause the simulation clock", G_CALLBACK(gui_action_pause), 0 },
  { "Polygons", NULL, "Fill polygons", NULL, "Toggle drawing of filled or outlined polygons", G_CALLBACK(gui_action_polygons), 1 },
  { "ExportSequence", NULL, "Sequence of frames", "<control>G", "Export a sequence of screenshots at regular intervals", G_CALLBACK(gui_action_exportsequence), 0 },

#if ! INCLUDE_GNOME
  { "DisablePolygons", NULL, "Show polygons", "D", NULL, G_CALLBACK(gui_action_disable_polygons), 1 },
  { "Trails", NULL, "Show trails", "T", NULL, G_CALLBACK(gui_action_trails), 0 },
  { "Grid", NULL, "_Grid", "G", "Toggle drawing of 1-metre grid", G_CALLBACK(gui_action_grid), 1 },
  { "DebugRays", NULL, "_Raytrace", "<alt>R", "Draw sensor rays", G_CALLBACK(gui_action_raytrace), 0 },
  { "DebugGeom", NULL, "_Geometry", "<alt>G", "Draw model geometry", G_CALLBACK(gui_action_geom), 0 },
  { "DebugMatrixTree", NULL, "Matrix _Tree", "<alt>T", "Show occupancy quadtree", G_CALLBACK(gui_action_matrixtree), 0 },
  { "DebugMatrixOccupancy", NULL, "Matrix _Occupancy", "<alt>M", "Show occupancy grid", G_CALLBACK(gui_action_matrixocc), 0 },
  { "DebugMatrixDelta", NULL, "Matrix _Delta", "<alt>D", "Show changes to quadtree", G_CALLBACK(gui_action_matrixdelta), 0 },
#endif
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
"        <menu action='Screenshot'>"
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
"    <menu action='Edit'>"
"      <menuitem action='Preferences'/>"
"    </menu>"
"    <menu action='View'>"
"      <menuitem action='Polygons'/>"
#if ! INCLUDE_GNOME
"      <menuitem action='DisablePolygons'/>"
"      <menuitem action='Grid'/>"
"      <menuitem action='Trails'/>"
"      <separator/>"
"        <menu action='Debug'>"
"          <menuitem action='DebugRays'/>"
"          <menuitem action='DebugGeom'/>"
"          <menuitem action='DebugMatrixTree'/>"
"          <menuitem action='DebugMatrixOccupancy'/>"
"          <menuitem action='DebugMatrixDelta'/>"
"          <separator/>"
"            <menu action='Model'>"
"              <separator/>"
"            </menu>"
"        </menu>"
#endif
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
  // USE THIS WHEN WE CAN USE GTK+-2.6
  //GtkAboutDialog *about = gtk_about_dialog_new();
  // gtk_show_about_dialog( about );
  
  const char* str =  "Stage\nVersion %s\n"
    "Part of the Player/Stage Project\n"
    "[http://playerstage.sourceforge.net]\n\n"
    "Copyright Richard Vaughan, Andrew Howard, Brian Gerkey\n"
    " and contributors 2000-2006.\n\n"
    "Released under the GNU General Public License v2.";
  
  GtkMessageDialog* about = (GtkMessageDialog*)
    gtk_message_dialog_new( NULL, 0, 
			    GTK_MESSAGE_INFO,
			    GTK_BUTTONS_CLOSE, 
			    str,
			    PACKAGE_VERSION);
    
  g_signal_connect (about, "destroy",
		    G_CALLBACK(gtk_widget_destroyed), &about);
  
  /* Destroy the dialog when the user responds to it (e.g. clicks a button) */
  g_signal_connect_swapped( about, "response",
			    G_CALLBACK (gtk_widget_destroy),
			    about );

  gtk_widget_show(GTK_WIDGET(about));
}


// defined in gui_prefs.c
GtkWidget* create_prefsdialog( void );

void gui_action_prefs( GtkAction *action, gpointer user_data)
{
  GtkWidget* dlg = create_prefsdialog();

    
    //g_signal_connect (about, "destroy",
    //	    G_CALLBACK(gtk_widget_destroyed), &about);
  
  /* Destroy the dialog when the user responds to it (e.g. clicks a button) */
  //g_signal_connect_swapped( about, "response",
    //		    G_CALLBACK (gtk_widget_destroy),
    //		    about );

  gtk_widget_show(dlg);
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

#if ! INCLUDE_GNOME

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

void gui_action_matrixdelta( GtkToggleAction* action, gpointer userdata )
{
  PRINT_DEBUG( "Matrix delta menu item" );

  stg_world_t* world = (stg_world_t*)userdata;  
  if( gtk_toggle_action_get_active( action ) ) 
    {
      if( ! fig_debug_matrix )
	fig_debug_matrix = stg_rtk_fig_create(world->win->canvas,world->win->bg,STG_LAYER_MATRIX_TREE);  

      _render_matrix_deltas = TRUE;

      // doesn't work: too many figs?
      //stg_cell_render_tree( world->matrix->root );
    }
  else
    {
      _render_matrix_deltas = FALSE;
      stg_cell_unrender_tree( world->matrix->root );
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

#endif

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

  if( m == NULL )
    {
      puts( "Warning: debug model menu not found" );
      return;
    }
  
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
  PRINT_MSG( "Exit menu item selected" );
  
  gboolean confirm = quit_dialog( NULL );
  
  if( confirm )
    {
      PRINT_MSG( "Confirmed" );
      stg_quit_request();
    }
  else
      PRINT_MSG( "Cancelled" );
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
    case STK_IMAGE_FORMAT_PNG: suffix = "png"; break;
      //case STK_IMAGE_FORMAT_PNM: suffix = "pnm"; break;
      //case STK_IMAGE_FORMAT_PPM: suffix = "ppm"; break;
    default:
      suffix = ".png";
    }

  if( win->frame_series > 0 )
    snprintf(filename, sizeof(filename), "stage-%03d-%04d.%s",
	     win->frame_series, win->frame_index, suffix);
  else
    snprintf(filename, sizeof(filename), "stage-%03d.%s", win->frame_index, suffix);

  printf("Stage: saving [%s]\n", filename);
  
#if INCLUDE_GNOME
  // TODO
#else
  stg_rtk_canvas_export_image( win->canvas, filename, win->frame_format );
#endif

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
  stg_model_t* mod = (stg_model_t*)data;
  model_change( mod, &mod->polygons ); 
}

void gui_action_exportframe( GtkAction* action, void* userdata ) 
{ 
  stg_world_t* world = (stg_world_t*)userdata;
  export_window( world->win );
}


void gui_action_trails( GtkToggleAction* action, void* userdata )
{
  PRINT_DEBUG( "Trails menu item" );
  stg_world_t* world = (stg_world_t*)userdata;
  
  if( gtk_toggle_action_get_active( action ) )
    {

#if ! INCLUDE_GNOME
      fig_trails = stg_rtk_fig_create( world->win->canvas,
				       world->win->bg, STG_LAYER_BODY-1 );      
#endif
    }
  else
    {
#if ! INCLUDE_GNOME
      stg_rtk_fig_and_descendents_destroy( fig_trails );
      fig_trails = NULL;
#endif
    }
}


void gui_action_polygons( GtkToggleAction* action, void* userdata )
{
  PRINT_DEBUG( "Polygons menu item" );
  stg_world_t* world = (stg_world_t*)userdata;
  
  world->win->fill_polygons = gtk_toggle_action_get_active( action );
  g_hash_table_foreach( world->models, model_render_polygons_cb, NULL ); 
}


#if ! INCLUDE_GNOME

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
#endif

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
				      G_N_ELEMENTS(export_format_entries), 
				      win->frame_format, 
				      G_CALLBACK(gui_action_export_format),
				      win );
  
  gtk_action_group_add_radio_actions( group, export_freq_entries, 
				      G_N_ELEMENTS(export_freq_entries),
				      win->frame_interval, 
				      G_CALLBACK(gui_action_export_interval), 
				      win );
  
  gtk_ui_manager_insert_action_group (ui_manager, group, 0);

  // grey-out the radio list labels
  
  // gtk+-2.6 code
  //gtk_action_set_sensitive(gtk_action_group_get_action( group, "ExportFormat" ), FALSE );
  //gtk_action_set_sensitive(gtk_action_group_get_action( group, "ExportInterval" ), FALSE );

  // gtk+-2.4 code
  int sensi = FALSE;
  g_object_set( gtk_action_group_get_action( group, "ExportFormat" ), "sensitive", sensi, NULL );
  g_object_set( gtk_action_group_get_action( group, "ExportInterval" ), "sensitive", sensi, NULL);

  // accels
  GtkAccelGroup *accel_group = gtk_ui_manager_get_accel_group (ui_manager);
  //gtk_window_add_accel_group(GTK_WINDOW (win->canvas->frame), accel_group);
  
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
  gtk_box_pack_start (GTK_BOX(win->layout), menubar, FALSE, FALSE, 0);
  //gtk_layout_put(GTK_LAYOUT(win->canvas->layout), menubar, 0,300 );
  
}


void toggle_property_callback( GtkToggleAction* action, void* userdata )
{
  stg_property_toggle_args_t* args = 
    (stg_property_toggle_args_t*)userdata;  
  
  //puts( "TOGGLE!" );

  int active = gtk_toggle_action_get_active( action );
  if( active )
    {
      if( args->callback_off )
	{
	  //printf( "removing OFF callback\n" );
	  stg_model_remove_callback( args->mod, 
				     args->member, 
				     args->callback_off );
	}
      
      if( args->callback_on )
	{
	  //printf( "adding ON callback with args %s\n", (char*)args->arg_on );
	  stg_model_add_callback( args->mod, 
				  args->member,
				  args->callback_on, 
				  args->arg_on );

	  // trigger the callback
	  model_change( args->mod, args->member );
	}
    }
  else
    {
      if( args->callback_on )
	{
	  //printf( "removing ON callback\n" );
	  stg_model_remove_callback( args->mod, 
				     args->member, 
				     args->callback_on );
	}
      
      if( args->callback_off )
	{
	  //printf( "adding OFF callback with args %s\n", (char*)args->arg_off );
	  stg_model_add_callback( args->mod, 
				  args->member, 
				  args->callback_off, 
				  args->arg_off );

	  // trigger the callback
	  model_change( args->mod, args->member );
	}
      
    }
}



void stg_model_add_property_toggles( stg_model_t* mod, 
				     void* member, 
				     stg_model_callback_t callback_on,
				     void* arg_on,
				     stg_model_callback_t callback_off,
				     void* arg_off,
				     const char* name,
				     const char* label,
				     gboolean enabled )
{
  stg_property_toggle_args_t* args = 
    calloc(sizeof(stg_property_toggle_args_t),1);
  
  args->mod = mod;
  args->name = name;
  args->member = member; 
  args->callback_on = callback_on;
  args->callback_off = callback_off;
  args->arg_on = arg_on;
  args->arg_off = arg_off;

  static GtkActionGroup* grp = NULL;  
  GtkAction* act = NULL;
  
  if( ! grp )
    {
      grp = gtk_action_group_new( "DynamicDataActions" );
      gtk_ui_manager_insert_action_group(ui_manager, grp, 0);
    }      
  else
    // find the action associated with this label
    act = gtk_action_group_get_action( grp, name );
  
  if( act == NULL )
    {
      //printf( "creating new action/item for prop %s\n", propname );
      
      GtkToggleActionEntry entry;
      memset( &entry, 0, sizeof(entry));
      entry.name = name;
      entry.label = label;
      entry.tooltip = NULL;
      entry.callback = G_CALLBACK(toggle_property_callback);
      
      
      entry.is_active = enabled;
            
      gtk_action_group_add_toggle_actions( grp, &entry, 1, args  );        
      guint merge = gtk_ui_manager_new_merge_id( ui_manager );
      gtk_ui_manager_add_ui( ui_manager, 
			     merge,
			     TOGGLE_PATH, 
			     name, 
			     name, 
			     GTK_UI_MANAGER_AUTO, 
			     FALSE );
      
      act = gtk_action_group_get_action( grp, name );
      
      // store the action in the toggle structure for recall
      args->action = act;
      args->path = strdup(name);
      
      // stash this structure in the window's pointer list
      mod->world->win->toggle_list = 
	g_list_append( mod->world->win->toggle_list, args );
      
    }
  else
    g_signal_connect( act, "activate",  G_CALLBACK(toggle_property_callback), args );

  //override setting with value in worldfile, if one exists
  if( wf_property_exists(  mod->world->win->wf_section, name ))
    enabled = wf_read_int( mod->world->win->wf_section, 
			   name,
			   enabled );
  
  
  // if we start enabled, attach the 'on' callback
  if( enabled )
    {
      //printf( "adding ON callback with args %s\n", (char*)args->arg_on );
      stg_model_add_callback( args->mod, 
			      args->member,
			      args->callback_on, 
			      args->arg_on );
      
      // trigger the callback
      model_change( args->mod, args->member );
    }  
}
