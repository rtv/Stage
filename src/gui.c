/*
CVS: $Id: gui.c,v 1.109.2.1 2006-09-14 07:03:25 rtv Exp $
*/

// NOTE: this code is a mess right now as Stage moves to a new GUI
// model. The RTK-based code works fine, but this file will be untidy
// for a while.

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>

//#define DEBUG
//#undef DEBUG

// new canvas layer
#include "stage_internal.h"
#include "gui.h"


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

void gui_load( gui_window_t* win, int section )
{
  // remember the section for saving later
  win->wf_section = section;

  int window_width = 
    (int)wf_read_tuple_float(section, "size", 0, STG_DEFAULT_WINDOW_WIDTH);
  int window_height = 
    (int)wf_read_tuple_float(section, "size", 1, STG_DEFAULT_WINDOW_HEIGHT);
  
  double window_center_x = 
    wf_read_tuple_float(section, "center", 0, 0.0 );
  double window_center_y = 
    wf_read_tuple_float(section, "center", 1, 0.0 );
  
  double window_scale = 
    wf_read_float(section, "scale", 1.0 );
  
  win->fill_polygons = wf_read_int(section, "fill_polygons", 1 );
  win->show_grid = wf_read_int(section, "show_grid", 1 );
  
  PRINT_DEBUG2( "window width %d height %d", window_width, window_height );
  PRINT_DEBUG2( "window center (%.2f,%.2f)", window_center_x, window_center_y );
  PRINT_DEBUG1( "window scale %.2f", window_scale );
  
  gui_configure( win->world, 
		 window_width, window_height, 
		 window_scale, window_scale, 
		 window_center_x, window_center_y );
}


void gui_save( gui_window_t* win )
{
  int width, height;
  gtk_window_get_size(  GTK_WINDOW(win->frame), &width, &height );
  
  wf_write_tuple_float( win->wf_section, "size", 0, width );
  wf_write_tuple_float( win->wf_section, "size", 1, height );


#if INCLUDE_GNOME
  // TODO
#else
  wf_write_tuple_float( win->wf_section, "center", 0, win->canvas->ox );
  wf_write_tuple_float( win->wf_section, "center", 1, win->canvas->oy );
  wf_write_float( win->wf_section, "scale", win->canvas->sx );
#endif

  wf_write_int( win->wf_section, "fill_polygons", win->fill_polygons );
  wf_write_int( win->wf_section, "show_grid", win->show_grid );

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

  double range = hypot( f->y - mod->pose.y, f->x - mod->pose.x );

  if( range < f->closest_range )
    {
      f->closest_range = range;
      f->closest_model = mod;
    }
}

stg_model_t* stg_world_nearest_model( stg_world_t* world, double wx, double wy )
{
  find_close_t f;
  f.closest_range = 3.0;
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


