/*
CVS: $Id: gui_gtk.c,v 1.1.2.3 2007-06-22 01:36:24 rtv Exp $
*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>

//#define DEBUG
//#undef DEBUG


//#include "config.h" // need to know which GUI canvas to use
#include "stage_internal.h"
#include "gui.h"

#include <gtk/gtkgl.h>

// only models that have fewer rectangles than this get matrix
// rendered when dragged
#define STG_POLY_THRESHOLD 10


/** @addtogroup stage 
    @{
*/

/** @defgroup window Window

The Stage window consists of a menu bar, a view of the simulated
world, and a status bar.

The world view shows part of the simulated world. You can zoom the
view in and out, and scroll it to see more of the world. Simulated
robot devices, obstacles, etc., are rendered as colored polygons. The
world view can also show visualizations of the data and configuration
of various sensor and actuator models. The View menu has options to
control which data and configurations are rendered.

<h2>Worldfile Properties</h2>

@par Summary and default values

@verbatim
window
(
  # gui properties
  center [0 0]
  size [700 740]
  scale 1.0

  # model properties do not apply to the gui window
)
@endverbatim

@par Details
- size [width:int width:int]
  - size of the window in pixels
- center [x:float y:float]
  - location of the center of the window in world coordinates (meters)
  - scale [?:double]
  - ratio of world to pixel coordinates (window zoom)


<h2>Using the Stage window</h2>


<h3>Scrolling the view</h3>

<p>Left-click and drag on the background to move your view of the world.

<h3>Zooming the view</h3>

<p>Right-click and drag on the background to zoom your view of the
world. When you press the right mouse button, a circle appears. Moving
the mouse adjusts the size of the circle; the current view is scaled
with the circle.

<h3>Saving the world</h3>

<P>You can save the current pose of everything in the world, using the
File/Save menu item. <b>Warning: the saved poses overwrite the current
world file.</b> Make a copy of your world file before saving if you
want to keep the old poses.


<h3>Saving a screenshot</h3>

<p> The File/Export menu allows you to export a screenshot of the
current world view in JPEG or PNG format. The frame is saved in the
current directory with filename in the format "stage-(frame
number).(jpg/png)". 

 You can also save sequences of screen shots. To start saving a
sequence, select the desired time interval from the same menu, then
select File/Export/Sequence of frames. The frames are saved in the
current directory with filenames in the format "stage-(sequence
number)-(frame number).(jpg/png)".

The frame and sequence numbers are reset to zero every time you run
Stage, so be careful to rename important frames before they are
overwritten.

<h3>Pausing and resuming the clock</h3>

<p>The Clock/Pause menu item allows you to stop the simulation clock,
freezing the world. Selecting this item again re-starts the clock.


<h3>View options</h3>

<p>The View menu allows you to toggle rendering of a 1m grid, to help
you line up objects (View/Grid). You can control whether polygons are
filled (View/Fill polygons); turning this off slightly improves
graphics performance. The rest of the view menu contains options for
rendering of data and configuration for each type of model, and a
debug menu that enables visualization of some of the innards of Stage.

*/

/** @} */

#define TOGGLE_PATH "/Main/View"

extern int _stg_quit;

static GtkUIManager *ui_manager = NULL;

// regular actions
void gui_action_about( GtkAction *action, gpointer user_data);
void gui_action_prefs( GtkAction *action, gpointer user_data);
void gui_action_save( GtkAction *action, gpointer user_data);
void gui_action_reset( GtkAction* action, gpointer userdata );
void gui_action_exit( GtkAction* action, gpointer userdata );
void gui_action_exportframe( GtkAction* action, stg_world_t* world );
void gui_action_derotate( GtkAction* action, stg_world_t* world );

// toggle actions
void gui_action_exportsequence( GtkToggleAction* action, stg_world_t* world );
void gui_action_pause( GtkToggleAction* action, stg_world_t* world );
void gui_action_polygons( GtkToggleAction* action, stg_world_t* world ); 
void gui_action_fill( GtkToggleAction* action, stg_world_t* world ); 
void gui_action_grid( GtkToggleAction* action, stg_world_t* world ); 
void gui_action_alpha( GtkToggleAction* action, stg_world_t* world ); 
void gui_action_thumbnail( GtkToggleAction* action, stg_world_t* world );
void gui_action_bboxes( GtkToggleAction* action, stg_world_t* world );
void gui_action_trails( GtkToggleAction* action, stg_world_t* world ); 
void gui_action_follow( GtkToggleAction* action, stg_world_t* world ); 

// TODO 
/* void gui_action_raytrace( GtkToggleAction* action, stg_world_t* world );  */
/* void gui_action_geom( GtkToggleAction* action, stg_world_t* world );  */
/* void gui_action_matrixtree( GtkToggleAction* action, stg_world_t* world );  */
/* void gui_action_matrixocc( GtkToggleAction* action, stg_world_t* world );  */
/* void gui_action_matrixdelta( GtkToggleAction* action, stg_world_t* world ); */

// radio actions
void gui_action_export_interval( GtkRadioAction* action, 
				 GtkRadioAction *current, 
				 gpointer userdata );
void gui_action_export_format( GtkRadioAction* action, 	 
			       GtkRadioAction *current, 
			       gpointer userdata );

gboolean quit_dialog( GtkWindow* parent );


/* Normal items */
static GtkActionEntry entries[] = {
  { "File", NULL, "_File" },
  { "View", NULL, "_View" },
  { "Debug", NULL, "_Debug" },
  { "Edit", NULL, "_Edit" },
  { "Clock", NULL, "_Clock" },
  { "Help", NULL, "_Help" },
  { "Model", NULL, "_Models" },
  { "Screenshot", NULL, "Screenshot" },
  { "ExportInterval", NULL, "Screenshot interval" },
  { "ExportFormat", NULL, "Screenshot bitmap format" },
  { "ExportFrame", NULL, "Single frame", "<ctrl>f", "Export a screenshot to disk", G_CALLBACK(gui_action_exportframe) },
  { "About", NULL, "About Stage", NULL, NULL, G_CALLBACK(gui_action_about) },
  { "Preferences", NULL, "Preferences...", NULL, NULL, G_CALLBACK(gui_action_prefs) },
  { "Save", GTK_STOCK_SAVE, "_Save", "<control>S", "Save world state", G_CALLBACK(gui_action_save) },
  { "Reset", GTK_STOCK_OPEN, "_Reset", "<control>R", "Reset to last saved world state", G_CALLBACK(gui_action_reset) },
  { "Derotate", NULL, "Zero rotation", NULL, "Reset the 3D view rotation to zero", G_CALLBACK(gui_action_derotate) },
  { "Exit", GTK_STOCK_QUIT, "E_xit", "<control>Q", "Exit the program", G_CALLBACK(gui_action_exit) },
};

/* Toggle items */
static GtkToggleActionEntry toggle_entries[] = {
  { "Pause", NULL, "_Pause", "P", "Pause the simulation clock", G_CALLBACK(gui_action_pause), 0 },
  { "Fill", NULL, "Fill polygons", NULL, "Toggle drawing of filled or outlined polygons", G_CALLBACK(gui_action_fill), 1 },
  { "ExportSequence", NULL, "Sequence of frames", "<control>G", "Export a sequence of screenshots at regular intervals", G_CALLBACK(gui_action_exportsequence), 0 },

  { "Polygons", NULL, "Show polygons", "D", NULL, G_CALLBACK(gui_action_polygons), 1 },
  { "Alpha", NULL, "Alpha channel", "A", NULL, G_CALLBACK(gui_action_alpha), 1 },
  { "Thumbnail", NULL, "Thumbnail", "T", NULL, G_CALLBACK(gui_action_thumbnail), 0 },
  { "Bboxes", NULL, "Bounding _boxes", "B", NULL, G_CALLBACK(gui_action_bboxes), 1 },
    { "Trails", NULL, "Trails", NULL, NULL, G_CALLBACK(gui_action_trails), 0 },
    { "Follow", NULL, "Follow selection", NULL, NULL, G_CALLBACK(gui_action_follow), 0 },
  { "Grid", NULL, "_Grid", "G", "Toggle drawing of 1-metre grid", G_CALLBACK(gui_action_grid), 1 },

/*   { "DebugRays", NULL, "_Raytrace", "<alt>R", "Draw sensor rays", G_CALLBACK(gui_action_raytrace), 0 }, */
/*   { "DebugGeom", NULL, "_Geometry", "<alt>G", "Draw model geometry", G_CALLBACK(gui_action_geom), 0 }, */
/*   { "DebugMatrixTree", NULL, "Matrix _Tree", "<alt>T", "Show occupancy quadtree", G_CALLBACK(gui_action_matrixtree), 0 }, */
/*   { "DebugMatrixOccupancy", NULL, "Matrix _Occupancy", "<alt>M", "Show occupancy grid", G_CALLBACK(gui_action_matrixocc), 0 }, */
/*   { "DebugMatrixDelta", NULL, "Matrix _Delta", "<alt>D", "Show changes to quadtree", G_CALLBACK(gui_action_matrixdelta), 0 }, */
};

/* Radio items */
static GtkRadioActionEntry export_format_entries[] = {
  { "ExportFormatPNG", NULL, "PNG", NULL, "Portable Network Graphics format", STG_IMAGE_FORMAT_PNG },
  { "ExportFormatJPEG", NULL, "JPEG", NULL, "JPEG format", STG_IMAGE_FORMAT_JPEG },
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
"      <menuitem action='Fill'/>"
"      <menuitem action='Polygons'/>"
"      <menuitem action='Grid'/>"
"      <menuitem action='Bboxes'/>"
"      <menuitem action='Follow'/>"
"      <menuitem action='Alpha'/>"
"      <menuitem action='Trails'/>"
"      <menuitem action='Thumbnail'/>"
"      <separator/>"
"      <menuitem action='Derotate'/>"
/* "      <separator/>" */
/* "        <menu action='Debug'>" */
/* "          <menuitem action='DebugRays'/>" */
/* "          <menuitem action='DebugGeom'/>" */
/* "          <menuitem action='DebugMatrixTree'/>" */
/* "          <menuitem action='DebugMatrixOccupancy'/>" */
/* "          <menuitem action='DebugMatrixDelta'/>" */
/* "          <separator/>" */
/* "            <menu action='Model'>" */
/* "              <separator/>" */
/* "            </menu>" */
/* "        </menu>" */
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



void gui_startup( int* argc, char** argv[] )
{
  PRINT_DEBUG( "gui startup" );

  // Initialise the gtk lib
  gtk_init(argc, argv);
}

// TODO
void gui_shutdown( void )
{
  PRINT_DEBUG( "gui shutdown" );

  // Process remaining events (might catch something important, such
  // as a "save" menu click)
  while (gtk_events_pending())
    gtk_main_iteration();

  // do something?
}


void gui_action_about( GtkAction *action, gpointer user_data)
{
  // USE THIS WHEN WE CAN USE GTK+-2.6
  //GtkAboutDialog *about = gtk_about_dialog_new();
  // gtk_show_about_dialog( about );
  
  const char* str =  "Stage\nVersion %s\n"
    "Part of the Player/Stage Project\n"
    "[http://playerstage.sourceforge.net]\n\n"
    "Copyright Richard Vaughan, Brian Gerkey, Andrew Howard\n"
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

void gui_action_derotate( GtkAction* action, stg_world_t* world )
{
  puts( "Resetting view rotation." );
  world->win->stheta = 0.0;
  world->win->sphi = 0.0;
}

void gui_action_pause( GtkToggleAction* action, stg_world_t* world )
{
  PRINT_DEBUG( "Pause menu item" );
  world->paused = gtk_toggle_action_get_active( action );
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



void gui_window_menus_create( gui_window_t* win )
{
  ui_manager = gtk_ui_manager_new ();
  
  // actions
  win->action_group = gtk_action_group_new ("MenuActions");
  
  gtk_action_group_add_actions( win->action_group, entries, 
				G_N_ELEMENTS (entries),
				win->world );
  
  gtk_action_group_add_toggle_actions( win->action_group,toggle_entries, 
				       G_N_ELEMENTS(toggle_entries), 
				       win->world );
  
  gtk_action_group_add_radio_actions( win->action_group, export_format_entries, 
				      G_N_ELEMENTS(export_format_entries), 
				      win->frame_format, 
				      G_CALLBACK(gui_action_export_format),
				      win );
  
  gtk_action_group_add_radio_actions( win->action_group, export_freq_entries, 
				      G_N_ELEMENTS(export_freq_entries),
				      win->frame_interval, 
				      G_CALLBACK(gui_action_export_interval), 
				      win );
  
  gtk_ui_manager_insert_action_group (ui_manager, win->action_group, 0);

  // grey-out the radio list labels
  
  // gtk+-2.6 code
  //gtk_action_set_sensitive(gtk_action_group_get_action( win->action_group, "ExportFormat" ), FALSE );
  //gtk_action_set_sensitive(gtk_action_group_get_action( win->action_group, "ExportInterval" ), FALSE );

  // gtk+-2.4 code
  int sensi = FALSE;
  g_object_set( gtk_action_group_get_action( win->action_group, "ExportFormat" ), "sensitive", sensi, NULL );
  g_object_set( gtk_action_group_get_action( win->action_group, "ExportInterval" ), "sensitive", sensi, NULL);

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
  gui_window_t* win = (gui_window_t*)mod->world->win;

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
      
      //override setting with value in worldfile, if one exists
      if( wf_property_exists(  win->wf_section, name ))
	enabled = wf_read_int( win->wf_section, 
			       name,
			       enabled );
      
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
      win->toggle_list = 
	g_list_append( win->toggle_list, args );
      
    }
  else
    g_signal_connect( act, "activate",  G_CALLBACK(toggle_property_callback), args );
  
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

/* gboolean timeout( stg_world_t* world ) */
/* { */
/*   puts( "TIMEOUT" ); */
  
/*   char* clock = stg_world_clockstring( world ); */
/*   gtk_label_set_text( world->win->clock_label, clock ); */
/*   free(clock); */
  
/*   if( world->win->dirty ) */
/*     { */
/*       /\* Invalidate the whole window to trigger a redraw *\/ */
/*       gdk_window_invalidate_rect( world->win->canvas->window, &world->win->canvas->allocation, FALSE); */
/*       world->win->dirty = FALSE; */
/*     } */
  
/*   return TRUE; */
/* } */


void gui_load_toggle( gui_window_t* win, 
		      int section,
		      int* var, 
		      const char* action_name, 
		      const char* keyword )
{
  if( wf_property_exists( section, keyword ) )
    {
      *var = wf_read_int(section, keyword, *var );
      
      GtkToggleAction* action = 
	gtk_action_group_get_action( win->action_group, action_name );
      
      if( action )      
	gtk_toggle_action_set_active( action, *var );
      else
	PRINT_ERR1( "Failed to get menu action name \"%s\"", action_name );
    }
}


void gui_load( stg_world_t* world, int section )
{
  gui_window_t* win = (gui_window_t*)world->win;

  // remember the section for subsequent gui_save()s
  win->wf_section = section;
  
/*   win->redraw_interval =  */
/*     wf_read_int( section, "redraw_interval", win->redraw_interval ); */
/*   // cancel our old timeout callback */
/*   g_source_remove( win->timeout_id ); */
/*   // and install a new one with the (possibly) new interval */
/*   win->timeout_id = g_timeout_add( win->redraw_interval, timeout, world ); */
  
/*   printf( "** REDRAW interval %d ms\n", win->redraw_interval ); */

  win->panx = wf_read_tuple_float(section, "center", 0, win->panx );
  win->pany = wf_read_tuple_float(section, "center", 1, win->pany );
  win->stheta = wf_read_tuple_float( section, "rotate", 0, win->stheta );
  win->sphi = wf_read_tuple_float( section, "rotate", 1, win->sphi );
  win->scale = wf_read_float(section, "scale", win->scale );

 /*  win->fill_polygons = wf_read_int(section, "fill_polygons", win->fill_polygons ); */
/*   win->show_grid = wf_read_int(section, "show_grid", win->show_grid ); */
/*   win->show_alpha = wf_read_int(section, "show_alpha", win->show_alpha ); */
/*   win->show_data = wf_read_int(section, "show_data", win->show_data ); */
/*   win->show_thumbnail = wf_read_int(section, "show_thumbnail", win ->show_thumbnail ); */

  gui_load_toggle( win, section, &win->show_bboxes, "Bboxes", "show_bboxes" );
  gui_load_toggle( win, section, &win->show_alpha, "Alpha", "show_alpha" );
  gui_load_toggle( win, section, &win->show_grid, "Grid", "show_grid" );
  gui_load_toggle( win, section, &win->show_bboxes, "Thumbnail", "show_thumbnail" );
  gui_load_toggle( win, section, &win->show_data, "Data", "show_data" );
  gui_load_toggle( win, section, &win->follow_selection, "Follow", "show_follow" );

/*   if( wf_property_exists( section, "show_bboxes" ) ) */
/*     { */
/*       win->show_bboxes = wf_read_int(section, "show_bboxes", win->show_bboxes ); */
      
/*       GtkToggleAction* action = gtk_action_group_get_action( win->action_group, "Bboxes" ); */
/*       gtk_toggle_action_set_active( action, win->show_bboxes ); */
/*     } */

  int width, height;
  gtk_window_get_size(  GTK_WINDOW(win->frame), &width, &height );
  width = (int)wf_read_tuple_float(section, "size", 0, width );
  height = (int)wf_read_tuple_float(section, "size", 1, height );
  gtk_window_resize( GTK_WINDOW(win->frame), width, height );
}

void gui_world_set_title( stg_world_t* world, char* txt )
{
  gui_window_t* win = (gui_window_t*)world->win;
  
  // poke the string
  if( win && win->frame )
    gtk_window_set_title( GTK_WINDOW(win->frame), txt );
}

void gui_save( stg_world_t* world )
{
  gui_window_t* win = (gui_window_t*)world->win;

  int width, height;
  gtk_window_get_size(  GTK_WINDOW(win->frame), &width, &height );
  
  wf_write_tuple_float( win->wf_section, "size", 0, width );
  wf_write_tuple_float( win->wf_section, "size", 1, height );

  wf_write_float( win->wf_section, "scale", win->scale );

  wf_write_tuple_float( win->wf_section, "center", 0, win->panx );
  wf_write_tuple_float( win->wf_section, "center", 1, win->pany );

  wf_write_tuple_float( win->wf_section, "rotate", 0, win->stheta  );
  wf_write_tuple_float( win->wf_section, "rotate", 1, win->sphi  );

  wf_write_int( win->wf_section, "fill_polygons", win->fill_polygons );
  wf_write_int( win->wf_section, "show_grid", win->show_grid );
  wf_write_int( win->wf_section, "show_bboxes", win->show_bboxes );

  // save the state of the property toggles
  for( GList* list = win->toggle_list; list; list = g_list_next( list ) )
    {
      stg_property_toggle_args_t* tog = 
	(stg_property_toggle_args_t*)list->data;
      assert( tog );
      printf( "toggle item path: %s\n", tog->path );

      wf_write_int( win->wf_section, tog->path, 
		   gtk_toggle_action_get_active(  GTK_TOGGLE_ACTION(tog->action) ));
    }
}


void  signal_destroy( GtkObject *object,
		      gpointer user_data )
{
  PRINT_MSG( "Window destroyed." );
  stg_quit_request();
}

gboolean quit_dialog( GtkWindow* parent )
{
  GtkMessageDialog *dlg = 
    (GtkMessageDialog*)gtk_message_dialog_new( parent,
					       GTK_DIALOG_DESTROY_WITH_PARENT,
					       GTK_MESSAGE_QUESTION,
					       GTK_BUTTONS_YES_NO,
					       "Are you sure you want to quit Stage?" );
  
  
  gint result = gtk_dialog_run( GTK_DIALOG(dlg));
  gtk_widget_destroy(dlg);
  
  // return TRUE if use clicked YES
  return( result == GTK_RESPONSE_YES );
}

gboolean  signal_delete( GtkWidget *widget,
		     GdkEvent *event,
		     gpointer user_data )
{
  PRINT_MSG( "Request close window." );


  gboolean confirm = quit_dialog( widget );

  if( confirm )
    {
      PRINT_MSG( "Confirmed" );
      stg_quit_request();
    }
  else
      PRINT_MSG( "Cancelled" );
  
  return TRUE;
}


typedef struct 
{
  double x,y;
  stg_model_t* closest_model;
  stg_meters_t closest_range;
} find_close_t;

void find_close_func( gpointer key,
		      gpointer value,
		      gpointer user_data )
{
  stg_model_t* mod = (stg_model_t*)value;
  find_close_t* f = (find_close_t*)user_data;

  if( mod->parent == NULL && mod->gui_mask )
    {
      double range = hypot( f->y - mod->pose.y, f->x - mod->pose.x );
      
      if( range < f->closest_range )
	{
	  f->closest_range = range;
	  f->closest_model = mod;
	}
    }
}

stg_model_t* stg_world_nearest_root_model( stg_world_t* world, double wx, double wy )
{
  find_close_t f;
  f.closest_range = 0.5;
  f.closest_model = NULL;
  f.x = wx;
  f.y = wy;

  g_hash_table_foreach( world->models, find_close_func, &f );

  //printf( "closest model %s\n", f.closest_model ? f.closest_model->token : "none found" );

  return f.closest_model;
}


const char* gui_model_describe(  stg_model_t* mod )
{
  static char txt[256];
  
  snprintf(txt, sizeof(txt), "model \"%s\" pose: [%.2f,%.2f,%.2f]",  
	   //stg_model_type_string(mod->type), 
	   mod->token, 
	   //mod->world->id, 
	   //mod->id,  
	   mod->pose.x, mod->pose.y, mod->pose.a  );
  
  return txt;
}


void gui_model_display_pose( stg_model_t* mod, char* verb )
{
  char txt[256];
  gui_window_t* win = mod->world->win;

  // display the pose
  snprintf(txt, sizeof(txt), "%s %s", 
	   verb, gui_model_describe(mod)); 
  guint cid = gtk_statusbar_get_context_id( win->status_bar, "on_mouse" );
  gtk_statusbar_pop( win->status_bar, cid ); 
  gtk_statusbar_push( win->status_bar, cid, txt ); 
  //printf( "STATUSBAR: %s\n", txt );
}


void* gui_world_create( stg_world_t* world )
{
  gui_window_t* win = calloc( sizeof(gui_window_t), 1 );
  world->win = win;

  // Create a top-level window
  win->frame = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_default_size( GTK_WINDOW(win->frame), 
			       STG_DEFAULT_WINDOW_WIDTH,
			       STG_DEFAULT_WINDOW_HEIGHT  );

  win->layout = gtk_vbox_new(FALSE, 0);

  GtkHBox* hbox = GTK_HBOX(gtk_hbox_new( FALSE, 3 ));

  win->status_bar = GTK_STATUSBAR(gtk_statusbar_new());
  gtk_statusbar_set_has_resize_grip( win->status_bar, FALSE );
  win->clock_label = GTK_LABEL(gtk_label_new( "clock" ));

  GtkWidget* contents = NULL;

  gtk_container_add(GTK_CONTAINER(win->frame), win->layout);
  
  g_signal_connect(GTK_OBJECT(win->frame),
		   "destroy",
		   GTK_SIGNAL_FUNC(signal_destroy),
		   NULL);

  g_signal_connect(GTK_OBJECT(win->frame),
		   "delete-event",
		   GTK_SIGNAL_FUNC(signal_delete),
		   NULL);

  gtk_box_pack_start(GTK_BOX(hbox), GTK_WIDGET(win->clock_label), FALSE, FALSE, 5);

  gtk_box_pack_start(GTK_BOX(hbox), 
  	   GTK_WIDGET(win->status_bar), TRUE, TRUE, 5);

 
  // we'll add these backwards so we can stick the menu in later
  gtk_box_pack_end(GTK_BOX(win->layout), 
		   GTK_WIDGET(hbox), FALSE, TRUE, 0);

  // create the GL drawing widget
  win->canvas = gui_create_canvas( world );

  gtk_box_pack_end( GTK_BOX(win->layout),
		    win->canvas, TRUE, TRUE, 0);

  gtk_widget_show( win->canvas );

  win->dirty = TRUE; // redraw requested

  puts( "CREATED GL WIDGET" );

  win->world = world;
    
  char txt[256];
  snprintf( txt, 256, " Stage v%s", VERSION );
  gtk_statusbar_push( win->status_bar, 0, txt ); 

  snprintf( txt, 256, " Stage: %s", world->token );
  gtk_window_set_title( GTK_WINDOW(win->frame), txt );

  win->show_cfg = TRUE;
  win->show_data = TRUE;
  win->show_geom = TRUE;
  win->show_alpha = TRUE;
  win->fill_polygons = TRUE;
  win->show_polygons = TRUE;
  win->show_thumbnail = FALSE;
  win->show_bboxes = TRUE;
  win->frame_interval = 500; // ms
  win->frame_format = STG_IMAGE_FORMAT_PNG;
  //win->selection_active = NULL;

  win->sphi = 0.0;
  win->stheta = 0.0;
  //win->scale = 1.0; // fix this up in a second

  // enable all objects on the canvas to find our window object
  //win->canvas->userdata = (void*)win; 
  
  //win->bg = stg_rtk_fig_create( win->canvas, NULL, 0 );
  
  // draw a mark for the origin
  /* stg_rtk_fig_color_rgb32( win->bg, stg_lookup_color(STG_GRID_MAJOR_COLOR) );     */
/*   double mark_sz = 0.4; */
/*   stg_rtk_fig_ellipse( win->bg, 0,0,0, mark_sz/3, mark_sz/3, 0 ); */
/*   stg_rtk_fig_arrow( win->bg, -mark_sz/2, 0, 0, mark_sz, mark_sz/4 ); */
/*   stg_rtk_fig_arrow( win->bg, 0, -mark_sz/2, M_PI/2, mark_sz, mark_sz/4 ); */
  //stg_rtk_fig_grid( win->bg, 0,0, 100.0, 100.0, 1.0 );


  //win->poses = stg_rtk_fig_create( win->canvas, NULL, 0 );

  
   // start in the center, fully zoomed out
/*   stg_rtk_canvas_scale( win->canvas,  */
/*   		0.02, */
/*   		0.02 ); */

/*   //stg_rtk_canvas_origin( win->canvas, width/2.0, height/2.0 ); */
  
/*   // show the limits of the world */
/*   stg_rtk_canvas_bgcolor( win->canvas, 0.9, 0.9, 0.9 ); */
/*   stg_rtk_fig_color_rgb32( win->bg, 0xFFFFFF );     */
/*   stg_rtk_fig_rectangle( win->bg, 0,0,0, world->width, world->height, 1 ); */

  gui_window_menus_create( win );
 
  win->toggle_list = NULL;

  // show the window
  gtk_widget_show_all(win->frame);

/*   // install the timeout callback that renders the window */
/*   win->redraw_interval = STG_DEFAULT_REDRAW_INTERVAL; */
/*   win->timeout_id = g_timeout_add( win->redraw_interval, timeout, world ); */

  return (void*)win;
}

void gui_action_exportframe( GtkAction* action, stg_world_t* world ) 
{ 
  export_window( world->win );
}

void gui_action_grid( GtkToggleAction* action, stg_world_t* world )
{
  PRINT_DEBUG( "Grid menu item" );  
  world->win->show_grid = gtk_toggle_action_get_active( action );
}

void gui_action_follow( GtkToggleAction* action, stg_world_t* world )
{
  PRINT_DEBUG( "Follow menu item" );  
  world->win->follow_selection = gtk_toggle_action_get_active( action );
}

void gui_action_alpha( GtkToggleAction* action,  stg_world_t* world )
{
  PRINT_DEBUG( "Toggle alpha menu item" );
  gui_enable_alpha( world, gtk_toggle_action_get_active( action ) );
}

void gui_action_trails( GtkToggleAction* action, stg_world_t* world )
{
  PRINT_DEBUG( "Trails menu item" );
  
  if( gtk_toggle_action_get_active( action ) )
    {
      puts( "TRAILS NOT IMLEMENTED" );
    }
  else
    {
    }
}


//void stg_model_blocks_update

void gui_action_fill( GtkToggleAction* action, stg_world_t* world )
{
  PRINT_DEBUG( "Polygons menu item" );
  world->win->fill_polygons = gtk_toggle_action_get_active( action );

  // TODO regenerate all block displaylists to notice this change
  
  //g_hash_table_foreach( world->models, model_render_polygons_cb, NULL ); 
}

void gui_action_thumbnail( GtkToggleAction* action, stg_world_t* world )
{
  PRINT_DEBUG( "Thumbnail menu item" );
  world->win->show_thumbnail = gtk_toggle_action_get_active( action );
}

void gui_action_bboxes( GtkToggleAction* action, stg_world_t* world )
{
  PRINT_DEBUG( "BBox menu item" );
  world->win->show_bboxes = gtk_toggle_action_get_active( action );
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
    case STG_IMAGE_FORMAT_JPEG: suffix = "jpg"; break;
    case STG_IMAGE_FORMAT_PNG: suffix = "png"; break;
    default:
      suffix = ".png";
    }

  if( win->frame_series > 0 )
    snprintf(filename, sizeof(filename), "stage-%03d-%04d.%s",
	     win->frame_series, win->frame_index, suffix);
  else
    snprintf(filename, sizeof(filename), "stage-%03d.%s", win->frame_index, suffix);

  printf("Stage: saving [%s]\n", filename);
  
  
  // TODO - look into why this doesn't work - it just makes empty
  // bitmaps right now

  GdkWindow* drawable = gdk_gl_window_get_window(gtk_widget_get_gl_window( win->canvas ));

  GdkPixbuf* buf = gdk_pixbuf_get_from_drawable( NULL, 
						 drawable,
						 NULL,
						 0,0,0,0,
						 200, 200 );//win->canvas->sizex,
  //win->canvas->sizey );
  if( buf )
    { 
      switch( win->frame_format )
	{
	case STK_IMAGE_FORMAT_JPEG:
	  gdk_pixbuf_save( buf, filename, "jpeg", NULL,
			   "quality", "100", NULL );
	  break;
	  
	case STK_IMAGE_FORMAT_PPM:
	  gdk_pixbuf_save( buf, filename, "ppm", NULL, NULL );
	  break;
	  
	case STK_IMAGE_FORMAT_PNG:
	  gdk_pixbuf_save( buf, filename, "png", NULL, NULL );
	  break;
	case STK_IMAGE_FORMAT_PNM:
	  gdk_pixbuf_save( buf, filename, "pnm", NULL, NULL );
	  break;
	  
	default: 
	  puts( "unrecognized image format" );
	  break;
	}

      gdk_pixbuf_unref( buf );
    }
}


gboolean frame_callback( gpointer data )
{
  export_window( (gui_window_t*)data );
  return TRUE;
}

void gui_action_exportsequence( GtkToggleAction* action, stg_world_t* world )
{
  gui_window_t* win = world->win;
  
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


void gui_action_polygons( GtkToggleAction* action, stg_world_t* world )
{
  PRINT_DEBUG( "Disable polygons menu item" );
  world->win->show_polygons = gtk_toggle_action_get_active( action );
}

// returns 1 if the world should be destroyed
int gui_world_update( stg_world_t* world )
{
  PRINT_DEBUG( "gui world update" );
  
  char* clock = stg_world_clockstring( world );
  gtk_label_set_text( world->win->clock_label, clock );
  free(clock);
  
  if( world->win->dirty )
    {
      /* Invalidate the whole window to trigger a redraw */
      gdk_window_invalidate_rect( world->win->canvas->window, &world->win->canvas->allocation, FALSE);
      world->win->dirty = FALSE;
    }
  
  while( gtk_events_pending() )
    gtk_main_iteration(); 

  return 0;
}

