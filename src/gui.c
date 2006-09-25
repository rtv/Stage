/*
CVS: $Id: gui.c,v 1.108.2.1 2006-09-25 17:49:49 gerkey Exp $
*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>

//#define DEBUG
//#undef DEBUG

// new canvas layer
#include "stage_internal.h"
#include "gui.h"

#if INCLUDE_GNOME
#include "gnome.h"
#endif

#define STG_DEFAULT_WINDOW_WIDTH 700
#define STG_DEFAULT_WINDOW_HEIGHT 740

// only models that have fewer rectangles than this get matrix
// rendered when dragged
#define STG_POLY_THRESHOLD 10

// single static application visible to all funcs in this file
static stg_rtk_app_t *app = NULL; 

// some global figures anyone can draw in for debug purposes
stg_rtk_fig_t* fig_debug_rays = NULL;
stg_rtk_fig_t* fig_debug_geom = NULL;
stg_rtk_fig_t* fig_debug_matrix = NULL;
stg_rtk_fig_t* fig_trails = NULL;

int _render_matrix_deltas = FALSE;

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
  
  // ask the canvas to comply
  gtk_window_resize( GTK_WINDOW(win->frame), window_width, window_height );
  stg_rtk_canvas_scale( win->canvas, window_scale, window_scale );
  stg_rtk_canvas_origin( win->canvas, window_center_x, window_center_y );

}

void gui_save( gui_window_t* win )
{
  int width, height;
  gtk_window_get_size(  GTK_WINDOW(win->frame), &width, &height );
  
  wf_write_tuple_float( win->wf_section, "size", 0, width );
  wf_write_tuple_float( win->wf_section, "size", 1, height );

  wf_write_tuple_float( win->wf_section, "center", 0, win->canvas->ox );
  wf_write_tuple_float( win->wf_section, "center", 1, win->canvas->oy );

  wf_write_float( win->wf_section, "scale", win->canvas->sx );

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

void gui_startup( int* argc, char** argv[] )
{
  PRINT_DEBUG( "gui startup" );

  stg_rtk_initxx(argc, argv);
  
  app = stg_rtk_app_create();
  stg_rtk_app_main_init( app );

  
}

void gui_poll( void )
{
  //PRINT_DEBUG( "gui poll" );
  if( stg_rtk_app_main_loop( app ) )
    stg_quit_request();
}

void gui_shutdown( void )
{
  PRINT_DEBUG( "gui shutdown" );
  stg_rtk_app_main_term( app );  
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
  gtk_widget_destroy(GTK_WIDGET(dlg));
  
  // return TRUE if use clicked YES
  return( result == GTK_RESPONSE_YES );
}


gboolean  signal_delete( GtkWidget *widget,
		     GdkEvent *event,
		     gpointer user_data )
{
  PRINT_MSG( "Request close window." );


  gboolean confirm = quit_dialog( GTK_WINDOW(widget) );

  if( confirm )
    {
      PRINT_MSG( "Confirmed" );
      stg_quit_request();
    }
  else
      PRINT_MSG( "Cancelled" );
  
  return TRUE;
}

gui_window_t* gui_window_create( stg_world_t* world, int xdim, int ydim )
{
  gui_window_t* win = calloc( sizeof(gui_window_t), 1 );

  // Create a top-level window
  win->frame = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_default_size( GTK_WINDOW(win->frame), xdim, ydim ); 

  win->layout = gtk_vbox_new(FALSE, 0);

  GtkHBox* hbox = GTK_HBOX(gtk_hbox_new( FALSE, 3 ));

  win->canvas = stg_rtk_canvas_create( app );

  win->status_bar = GTK_STATUSBAR(gtk_statusbar_new());
  gtk_statusbar_set_has_resize_grip( win->status_bar, FALSE );
  win->clock_label = GTK_LABEL(gtk_label_new( "clock" ));

  GtkWidget* contents = NULL;

#if INCLUDE_GNOME

  contents = gtk_scrolled_window_new( NULL, NULL );
  
  win->gcanvas = GNOME_CANVAS(gnome_canvas_new_aa());
  gtk_container_add( GTK_CONTAINER(contents), 
		     GTK_WIDGET(win->gcanvas) );

  GnomeCanvasItem* bg = 
    gnome_canvas_item_new(gnome_canvas_root(win->gcanvas),
			  gnome_canvas_rect_get_type(),
			  "x1",-world->width/2.0,
			  "y1",-world->height/2.0,
			  "x2", world->width/2.0,
			  "y2", world->height/2.0,
			  "fill_color", "white",
			  NULL);
  
  g_signal_connect( bg, 
		    "event", 
		    G_CALLBACK(background_event_callback), 
		    world );
  
  GnomeCanvasItem* grp = 
    gnome_canvas_item_new( gnome_canvas_root(win->gcanvas),
			   gnome_canvas_group_get_type(),
			   NULL );
  
  gnome_canvas_item_lower_to_bottom( grp );
  gnome_canvas_item_move( grp, world->width, 0 );


  //GnomeCanvasItem *it = 
    gnome_canvas_item_new( GNOME_CANVAS_GROUP(grp),
			   gnome_canvas_widget_get_type(),
			   "width", world->width,
			   "height", world->height,
			   "anchor", GTK_ANCHOR_CENTER,
			   "widget", win->canvas->canvas,
			   NULL);
  

  // flip the vertical axis for the root group
  double flip[6];  
  art_affine_identity( flip );
  art_affine_flip( flip, flip, FALSE, TRUE );
  gnome_canvas_item_affine_relative( GNOME_CANVAS_ITEM(gnome_canvas_root(win->gcanvas)),
				     flip );

  win->zoom = 50.0;

  gnome_canvas_set_pixels_per_unit( win->gcanvas, win->zoom  );
  gnome_canvas_set_center_scroll_region( win->gcanvas, TRUE ); 
  gnome_canvas_set_scroll_region ( win->gcanvas, 
				   1.1 * -world->width/2.0, 
				   1.1 * -world->height/2.0, 
				   1.1 * world->width + world->width/2.0, 
				   1.1 * world->height/2.0  );

#else
  // no GnomeCanvas, so add the RTK window directly into the frame
  //gtk_container_add( GTK_WINDOW(win->scrolled_win), 
  //				 GTK_WIDGET(win->canvas->canvas) );  

  contents = win->canvas->canvas;
#endif

  // Put it all together
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
  gtk_box_pack_end( GTK_BOX(win->layout),
		    contents, TRUE, TRUE, 0);
  

  win->world = world;
  
  // enable all objects on the canvas to find our window object
  win->canvas->userdata = (void*)win; 
  
  char txt[256];
  snprintf( txt, 256, " Stage v%s", VERSION );
  gtk_statusbar_push( win->status_bar, 0, txt ); 

  snprintf( txt, 256, " Stage: %s", world->token );
  gtk_window_set_title( GTK_WINDOW(win->frame), txt );
  
  win->bg = stg_rtk_fig_create( win->canvas, NULL, 0 );
  
  // draw a mark for the origin
  stg_rtk_fig_color_rgb32( win->bg, stg_lookup_color(STG_GRID_MAJOR_COLOR) );    
  double mark_sz = 0.4;
  stg_rtk_fig_ellipse( win->bg, 0,0,0, mark_sz/3, mark_sz/3, 0 );
  stg_rtk_fig_arrow( win->bg, -mark_sz/2, 0, 0, mark_sz, mark_sz/4 );
  stg_rtk_fig_arrow( win->bg, 0, -mark_sz/2, M_PI/2, mark_sz, mark_sz/4 );
  //stg_rtk_fig_grid( win->bg, 0,0, 100.0, 100.0, 1.0 );
  
  win->show_geom = TRUE;
  win->fill_polygons = TRUE;
  win->show_polygons = TRUE;
  win->frame_interval = 500; // ms
  win->frame_format = STK_IMAGE_FORMAT_PNG;

  win->poses = stg_rtk_fig_create( win->canvas, NULL, 0 );

  
  //gint canvas_w, canvas_h;
  //gtk_layout_get_size( GTK_LAYOUT(win->gcanvas), &canvas_w, &canvas_h );

  /* g_object_get( win->gcanvas,  */
/* 		"width", &canvas_w,  */
/* 		"height", &canvas_h,  */
/* 		NULL ); */

/*   stg_rtk_canvas_size( win->canvas, canvas_w, canvas_h ); */
  
/*   printf( "canvas_w %d xdim %d world->width %.2f\n",  */
/* 	  canvas_w, xdim, world->width ); */

  //gnome_canvas_set_pixels_per_unit( win->gcanvas, 1.0 );//world->width /  canvas_w   );
  
  // start in the center, fully zoomed out
  stg_rtk_canvas_scale( win->canvas, 
  		0.02,
  		0.02 );

  //stg_rtk_canvas_origin( win->canvas, width/2.0, height/2.0 );

  win->selection_active = NULL;
  
  // show the limits of the world
  stg_rtk_canvas_bgcolor( win->canvas, 0.9, 0.9, 0.9 );
  stg_rtk_fig_color_rgb32( win->bg, 0xFFFFFF );    
  stg_rtk_fig_rectangle( win->bg, 0,0,0, world->width, world->height, 1 );
 
  gui_window_menus_create( win );
 
  win->toggle_list = NULL;

  return win;
}

void gui_window_destroy( gui_window_t* win )
{
  PRINT_DEBUG( "gui window destroy" );

  //g_hash_table_destroy( win->guimods );

  stg_rtk_canvas_destroy( win->canvas );
  stg_rtk_fig_destroy( win->bg );
  stg_rtk_fig_destroy( win->poses );
}

gui_window_t* gui_world_create( stg_world_t* world )
{
  PRINT_DEBUG( "gui world create" );
  
  assert(world);
  gui_window_t* win = gui_window_create( world, 
					 STG_DEFAULT_WINDOW_WIDTH,
					 STG_DEFAULT_WINDOW_HEIGHT );
  assert(win);

  // show the window
  gtk_widget_show_all(win->frame);

  return win;
}


void gui_world_render_cell( stg_rtk_fig_t* fig, stg_cell_t* cell )
{
  assert( fig );
  assert( cell );
  
  stg_rtk_fig_rectangle( fig,
			 cell->x, cell->y, 0.0,
			 cell->size, cell->size, 0 );
  
  // if this is a leaf node containing data, draw a little cross
  if( 0 )//cell->data && g_slist_length( (GSList*)cell->data ) )
    {
      stg_rtk_fig_line( fig, 
			cell->x-0.01, cell->y-0.01,
			cell->x+0.01, cell->y+0.01 );

      stg_rtk_fig_line( fig, 
			cell->x-0.01, cell->y+0.01,
			cell->x+0.01, cell->y-0.01 );
    }
  
  if( cell->children[0] )
    {
      int i;
      for( i=0; i<4; i++ )
	gui_world_render_cell( fig, cell->children[i] );  
    }
}

void gui_world_render_cell_occupied( stg_rtk_fig_t* fig, stg_cell_t* cell )
{
  assert( fig );
  assert( cell );
  
  // if this is a leaf node containing data, draw a 
  if( cell->data && g_slist_length( (GSList*)cell->data ) )
    stg_rtk_fig_rectangle( fig,
			   cell->x, cell->y, 0.0,
			   cell->size, cell->size, 0 );            
  
  if( cell->children[0] )
    {
      int i;
      for( i=0; i<4; i++ )
	gui_world_render_cell_occupied( fig, cell->children[i] );  
    }
}

void gui_world_render_cell_cb( gpointer cell, gpointer fig )
{
  gui_world_render_cell( (stg_rtk_fig_t*)fig, (stg_cell_t*)cell );
}


void render_matrix_object( gpointer key, gpointer value, gpointer user )
{
  // value is a list of cells
  GSList* list = (GSList*)value;
  
  while( list )
    {
      stg_cell_t* cell = (stg_cell_t*)list->data;
      stg_rtk_fig_rectangle( (stg_rtk_fig_t*)user,
			     cell->x, cell->y, 0.0,
			     cell->size, cell->size, 0 );
      
      list = list->next;
    }
}

// useful debug function allows plotting the matrix
void gui_world_matrix_table( stg_world_t* world, gui_window_t* win )
{
  if( win->matrix )
    {
      if( world->matrix->ptable )
	g_hash_table_foreach( world->matrix->ptable, 
			  render_matrix_object, 
			      win->matrix );   
    }
}


void gui_pose( stg_rtk_fig_t* fig, stg_model_t* mod )
{
  stg_rtk_fig_arrow_ex( fig, 0,0, mod->pose.x, mod->pose.y, 0.05 );
}


void gui_pose_cb( gpointer key, gpointer value, gpointer user )
{
  gui_pose( (stg_rtk_fig_t*)user, (stg_model_t*)value );
}


// returns 1 if the world should be destroyed
int gui_world_update( stg_world_t* world )
{
  //PRINT_DEBUG( "gui world update" );
  
  gui_window_t* win = world->win;
  
  if( stg_rtk_canvas_isclosed( win->canvas ) )
    {
      puts( "<window closed>" );
      //stg_world_destroy( world );
      return 1;
    }

  if( win->matrix )
    {
      stg_rtk_fig_clear( win->matrix );
      gui_world_render_cell_occupied( win->matrix, world->matrix->root );
      //gui_world_matrix_table( world, win );
    }  
  
  if( win->matrix_tree )
    {
      stg_rtk_fig_clear( win->matrix_tree );
      gui_world_render_cell( win->matrix_tree, world->matrix->root );
    }  

  char clock[256];
#ifdef DEBUG
  snprintf( clock, 255, "Time: %lu:%lu:%02lu:%02lu.%03lu (sim:%3d real:%3d ratio:%2.2f)\tsubs: %d  %s",
	    world->sim_time / (24*3600000), // days
	    world->sim_time / 3600000, // hours
	    (world->sim_time % 3600000) / 60000, // minutes
	    (world->sim_time % 60000) / 1000, // seconds
	    world->sim_time % 1000, // milliseconds
	    (int)world->sim_interval,
	    (int)world->real_interval_measured,
	    (double)world->sim_interval / (double)world->real_interval_measured,
	    world->subs,
	    world->paused ? "--PAUSED--" : "" );
#else

  snprintf( clock, 255, "Time: %lu:%lu:%02lu:%02lu.%03lu\t(sim/real:%2.2f)\tsubs: %d  %s",
	    world->sim_time / (24*3600000), // days
	    world->sim_time / 3600000, // hours
	    (world->sim_time % 3600000) / 60000, // minutes
	    (world->sim_time % 60000) / 1000, // seconds
	    world->sim_time % 1000, // milliseconds
	    (double)world->sim_interval / (double)world->real_interval_measured,
	    world->subs,
	    world->paused ? "--PAUSED--" : "" );
#endif
  

  // smooth the performance avg a little
/*   static double fraction_avg = 1.0; */
/*   fraction_avg = fraction_avg * 0.9 + */
/*     (double)world->wall_interval / (double)world->real_interval_measured * 0.1; */
/*   gtk_progress_bar_set_fraction( win->canvas->perf_bar, fraction_avg ); */
  
/*   static double rt_avg = 1.0; */
/*   rt_avg = rt_avg * 0.9 + */
/*     (double)world->real_interval_measured / */
/*     (double)world->sim_interval * 0.1; */
/*   gtk_progress_bar_set_fraction( win->canvas->rt_bar,  */
/* 				 rt_avg ); */
  
  if( win->show_geom )
    gui_world_geom( world );

  static int trail_interval = 5;
  
  if( fig_trails )
    if( trail_interval++ > 4 )
      {
    void gui_world_trails(stg_world_t *); // forward declaration
	gui_world_trails( world );
	trail_interval = 0;
      }
  
  if( win->selection_active )
    gui_model_display_pose( win->selection_active, "Selection:" );
  
  gtk_label_set_text( win->clock_label, clock );

  stg_rtk_canvas_render( win->canvas );      

  return 0;
}

void gui_world_destroy( stg_world_t* world )
{
  PRINT_DEBUG( "gui world destroy" );
  
  if( world->win && world->win->canvas ) 
    stg_rtk_canvas_destroy( world->win->canvas );
  else
    PRINT_WARN1( "can't find a window for world %d", world->id );
}


// draw trails behing moving models
void gui_model_trail( stg_model_t* mod )
{ 
  assert( fig_trails );

  if( mod->parent ) // only trail top-level objects
    return; 

  stg_rtk_fig_t* spot = stg_rtk_fig_create( mod->world->win->canvas,
					    fig_trails, STG_LAYER_BODY-1 );      
  
  stg_rtk_fig_color_rgb32( spot, mod->color );
  
  
  // draw the bounding box
  stg_pose_t bbox_pose;
  memcpy( &bbox_pose, &mod->geom.pose, sizeof(bbox_pose));
  stg_model_local_to_global( mod, &bbox_pose );
  stg_rtk_fig_rectangle( spot, 
			 bbox_pose.x, bbox_pose.y, bbox_pose.a, 
			 mod->geom.size.x,
			 mod->geom.size.y, 0 );  
}


// wrapper 
void gui_model_trail_cb( gpointer key, gpointer value, gpointer user )
{
  gui_model_trail( (stg_model_t*)value );
}

// render a trail cell for all models
void gui_world_trails( stg_world_t* world )
{
  assert( fig_trails );
  g_hash_table_foreach( world->models, gui_model_trail_cb, NULL ); 
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

#define DEBUG 1

// Process mouse events 
void gui_model_mouse(stg_rtk_fig_t *fig, int event, int mode)
{  
  // each fig links back to the model that owns it
  stg_model_t* mod = (stg_model_t*)fig->userdata;
  assert( mod );

  //printf( "ON MOUSE CALLED BACK for model %s fig %p with userdata %p mask %d\n", 
  //	mod->token, fig, fig->userdata, mod->gui_mask );

  gui_window_t* win = mod->world->win;
  assert(win);

  guint cid=0;
  
  static stg_velocity_t capture_vel;
  stg_pose_t pose;  
  stg_velocity_t zero_vel;
  memset( &capture_vel, 0, sizeof(capture_vel) );
  memset( &zero_vel, 0, sizeof(zero_vel) );

  switch (event)
    {
    case STK_EVENT_MOUSE_OVER:
      win->selection_active = mod;      
      break;

    case STK_EVENT_MOUSE_NOT_OVER:
      win->selection_active = NULL;      
      
      // get rid of the selection info
      cid = gtk_statusbar_get_context_id( win->status_bar, "on_mouse" );
      gtk_statusbar_pop( win->status_bar, cid ); 
      break;
      
    case STK_EVENT_PRESS:
      {
	// store the velocity at which we grabbed the model (if it has a velocity)
	stg_model_get_velocity( mod, &capture_vel );

	stg_velocity_t zv;
	memset( &zv, 0, sizeof(zv));
	stg_model_set_velocity( mod, &zv );     
      }
      // DELIBERATE NO-BREAK      

    case STK_EVENT_MOTION:       
      // move the object to follow the mouse
      stg_rtk_fig_get_origin(fig, &pose.x, &pose.y, &pose.a );
      
      // TODO - if there are more motion events pending, do nothing.
      //if( !gtk_events_pending() )
      
      // only update simple objects on drag
      //if( mod->polygons->len < STG_POLY_THRESHOLD )
      //stg_model_set_pose( mod, &pose );

      stg_model_set_pose( mod, &pose );
      
      // display the pose
      //gui_model_display_pose( mod, "Dragging:" );
      break;
      
    case STK_EVENT_RELEASE:
      // move the entity to its final position
      stg_rtk_fig_get_origin(fig, &pose.x, &pose.y, &pose.a );
      stg_model_set_pose( mod, &pose );
      
      // and restore the velocity at which we grabbed it
      stg_model_set_velocity( mod, &capture_vel );
      break;      
      
    default:
      break;
    }

  return;
}

int stg_fig_clear_cb(  stg_model_t* mod, char* name, 
		       void* data, size_t len, void* userp )
{
  printf( "clear CB %p\n", userp );
  
  if( userp )
    stg_rtk_fig_clear((stg_rtk_fig_t*)userp);
  
  return 1; // cancels callback
}



void gui_model_create( stg_model_t* mod )
{
  PRINT_DEBUG( "gui model create" );
  
  //gui_window_t* win = mod->world->win;  

  stg_rtk_fig_t* parent = mod->world->win->bg;
  
  if( mod->parent )
    parent = stg_model_get_fig( mod->parent, "top" );
  
  stg_rtk_fig_t* top = stg_rtk_fig_create(  mod->world->win->canvas, parent, STG_LAYER_BODY );
  
  top->userdata = mod;
		     
  // install the figure 
  g_datalist_set_data( &mod->figs, "top", top ); 

  int ch;
  for(ch=0; ch < mod->children->len; ch++ )
    {
      stg_model_t* child = g_ptr_array_index( mod->children, ch );
      gui_model_create( child );
    }

  // XX
  //stg_model_property_refresh( mod, "polygons" );
}

void gui_model_destroy( stg_model_t* mod )
{
  PRINT_DEBUG( "gui model destroy" );
  
  // TODO - It's too late to kill the figs - the canvas is gone! fix this?
  
  stg_rtk_fig_t* fig = stg_model_get_fig( mod, "top" );
  
  if( fig ) 
    stg_rtk_fig_and_descendents_destroy( fig );

  fig = NULL;
}


/// render a model's global pose vector
void gui_model_render_geom_global( stg_model_t* mod, stg_rtk_fig_t* fig )
{
  stg_pose_t glob;
  stg_model_get_global_pose( mod, &glob );
  
  stg_rtk_fig_color_rgb32( fig, 0x0000FF ); // blue
  //stg_rtk_fig_line( fig, 0, 0, glob.x, glob.y );
  stg_rtk_fig_arrow( fig, glob.x, glob.y, glob.a, 0.15, 0.05 );  
  
  if( mod->parent )
    {
      stg_pose_t parentpose;
      stg_model_get_global_pose( mod->parent, &parentpose );
      stg_rtk_fig_line( fig, parentpose.x, parentpose.y, glob.x, parentpose.y );
      stg_rtk_fig_line( fig, glob.x, parentpose.y, glob.x, glob.y );
    }
  else
    {
      stg_rtk_fig_line( fig, 0, 0, glob.x, 0 );
      stg_rtk_fig_line( fig, glob.x, 0, glob.x, glob.y );
    }
  
  stg_geom_t geom;
  stg_model_get_geom( mod, &geom );
  
  stg_pose_t localpose;
  memcpy( &localpose, &geom.pose, sizeof(localpose));
  stg_model_local_to_global( mod, &localpose );
  
  // draw the local offset
  stg_rtk_fig_line( fig, 
		glob.x, glob.y, 
		localpose.x, glob.y );

  stg_rtk_fig_line( fig, 
		localpose.x, glob.y, 
		localpose.x, localpose.y );
  
  // draw the bounding box
  stg_pose_t bbox_pose;
  memcpy( &bbox_pose, &geom.pose, sizeof(bbox_pose));
  stg_model_local_to_global( mod, &bbox_pose );
  stg_rtk_fig_rectangle( fig, 
		     bbox_pose.x, bbox_pose.y, bbox_pose.a, 
		     geom.size.x,
		     geom.size.y, 0 );  
}

/// move a model's figure to the model's current location
int gui_model_move( stg_model_t* mod, void* userp )
{ 
  
  stg_rtk_fig_origin( stg_model_get_fig(mod,"top"), 
		      mod->pose.x, mod->pose.y, mod->pose.a );   
  
#if INCLUDE_GNOME
  double r[6], t[6];
  art_affine_rotate(r,RTOD(pose->a));
  gnome_canvas_item_affine_absolute( GNOME_CANVAS_ITEM(mod->grp), r );
  gnome_canvas_item_set( GNOME_CANVAS_ITEM(mod->grp),
			 "x", mod->pose.x,
			 "y", mod->pose.y,
			 NULL );
#endif			

  return 0;
}

///  render a model's geometry if geom viewing is enabled
void gui_model_render_geom( stg_model_t* mod )
{
  if( fig_debug_geom )
    gui_model_render_geom_global( mod, fig_debug_geom ); 
}

/// wrapper for gui_model_render_geom for use in callbacks
void gui_model_render_geom_cb( gpointer key, gpointer value, gpointer user )
{
  gui_model_render_geom( (stg_model_t*)value );
}

/// render the geometry of all models
void gui_world_geom( stg_world_t* world )
{
  if( fig_debug_geom )
    {
      stg_rtk_fig_clear( fig_debug_geom );
      g_hash_table_foreach( world->models, gui_model_render_geom_cb, NULL ); 
    }
}


void stg_model_fig_clear( stg_model_t* mod, const char* figname )
{
  stg_rtk_fig_t* fig = stg_model_get_fig( mod, figname );
  if( fig )
    stg_rtk_fig_clear( fig );
}

int stg_model_fig_clear_cb( stg_model_t* mod, void* data, size_t len, 
			    void* userp )
{
  if( userp )
    stg_model_fig_clear( mod, (char*)userp );
  return 1;  // remove callback
}


stg_rtk_fig_t* stg_model_get_fig( stg_model_t* mod, const char* figname ) 
{
  return( (stg_rtk_fig_t*)g_datalist_get_data( &mod->figs, figname ));  
}


stg_rtk_fig_t* stg_model_fig_get_or_create( stg_model_t* mod, 
					    const char* figname, 
					    const char* parentname, 
					    int layer )
{
  stg_rtk_fig_t* fig = stg_model_get_fig( mod, figname );
  
  if( fig ) 
    return fig;
  else
    return stg_model_fig_create( mod, figname, parentname, layer );
}

stg_rtk_fig_t* stg_model_fig_create( stg_model_t* mod, 
				     const char* figname, 
				     const char* parentname, 
				     int layer )
{
  stg_rtk_fig_t* parent = NULL;
  
  if( parentname )
    parent = stg_model_get_fig( mod, parentname );
  
  stg_rtk_fig_t* fig = stg_rtk_fig_create( mod->world->win->canvas, parent, layer );  
  g_datalist_set_data( &mod->figs, figname, (void*)fig );

  return fig;
}

int gui_model_lines( stg_model_t* mod, void* userp )
{
 /*  puts( "GUI MODEL LINES" ); */

/*   printf( "mod %s rendering %d lines at %p\n", */
/* 	  mod->token, mod->lines_count, mod->lines); */

  stg_rtk_fig_t* fig = stg_model_get_fig( mod, "model_lines_fig" );  
  
  if( ! fig )
    {
      fig = stg_model_fig_create( mod, "model_lines_fig", "top", 40 );  
      stg_rtk_fig_color_rgb32( fig, mod->color );
    }
  
  stg_rtk_fig_clear( fig );
  
  int p,q;
  if( mod->lines && mod->lines_count > 0 )
    for( p=0; p<mod->lines_count; p++ )
      {
	stg_polyline_t* polyline = &mod->lines[p];
	
	if( polyline->points_count > 1 )
	  for( q=0; q<polyline->points_count-1; q++ )
	    {
	      /* printf( "drawing line from %.2f,%.2f to %.2f,%.2f\n", */
/* 				polyline->points[q].x, */
/* 				polyline->points[q].y, */
/* 				polyline->points[q+1].x, */
/* 				polyline->points[q+1].y ); */
		      
	      stg_rtk_fig_line( fig, 
				polyline->points[q].x,
				polyline->points[q].y,
				polyline->points[q+1].x,
				polyline->points[q+1].y );
	    }
      }
  return 0;
}

int gui_model_polygons( stg_model_t* mod, void* userp )
{
  //puts( "model render polygons" );

  stg_rtk_fig_t* fig = stg_model_get_fig( mod, "top" );
  assert( fig );
  
  stg_rtk_fig_clear( fig );
  
  size_t count = mod->polygons_count;
  stg_polygon_t* polys = mod->polygons;
  
  stg_geom_t geom;
  stg_model_get_geom(mod, &geom);
  
  stg_rtk_fig_color_rgb32( fig, mod->color );

  if( polys && count > 0 )
    {
      

	if( ! mod->world->win->fill_polygons )
	{
	  
	  int p;
	  for( p=0; p<count; p++ )
	    stg_rtk_fig_polygon( fig,
				 geom.pose.x,
				 geom.pose.y,
				 geom.pose.a,
				 polys[p].points->len,
				 polys[p].points->data,
				 0 );
	}
      else
	{
	  int p;
	  for( p=0; p<count; p++ )
	    stg_rtk_fig_polygon( fig,
				 geom.pose.x,
				 geom.pose.y,
				 geom.pose.a,
				 polys[p].points->len,
				 polys[p].points->data,
				 ! polys[p].unfilled );
	  
	  if( mod->gui_outline )
	    {
	      stg_rtk_fig_color_rgb32( fig, 0 ); // black
	      
	      for( p=0; p<count; p++ )
		stg_rtk_fig_polygon( fig,
				     geom.pose.x,
				     geom.pose.y,
				     geom.pose.a,
				     polys[p].points->len,
				     polys[p].points->data,
				     0 );

	      stg_rtk_fig_color_rgb32( fig, mod->color ); // change back
	    }
	  
	}
    }
  
  if( mod->boundary )
    {      
      stg_rtk_fig_rectangle( fig, 
			     geom.pose.x, geom.pose.y, geom.pose.a, 
			     geom.size.x, geom.size.y, 0 ); 
    }

  if( mod->gui_nose )
    {       
      // draw an arrow from the center to the front of the model
      stg_rtk_fig_arrow( fig, geom.pose.x, geom.pose.y, geom.pose.a, 
			 geom.size.x/2.0, 0.05 );
    }
    
  return 0;
}


int gui_model_mask( stg_model_t* mod, void* userp )
{
  
  stg_rtk_fig_t* fig = stg_model_get_fig(mod, "top" );
  assert(fig);
  
  stg_rtk_fig_movemask( fig, mod->gui_mask );  
  
  // only install a mouse handler if the object needs one
  //(TODO can we remove mouse handlers dynamically?)
  if( mod->gui_mask )    
    {
      //printf( "adding mouse handler for model %s\n", mod->token );
      stg_rtk_fig_add_mouse_handler( fig, gui_model_mouse );
    }
  return 0;
}
  
int gui_model_grid( stg_model_t* mod, void* userp )
{
  stg_geom_t geom;
  stg_model_get_geom(mod, &geom);
  
  stg_rtk_fig_t* grid = stg_model_get_fig(mod,"grid"); 
  
  // if we need a grid and don't have one, make one
  if( mod->gui_grid  )
    {    
      if( ! grid ) 
	grid = stg_model_fig_create( mod, "grid", "top", STG_LAYER_GRID); 
      
      stg_rtk_fig_clear( grid );
      stg_rtk_fig_color_rgb32( grid, stg_lookup_color(STG_GRID_MAJOR_COLOR ) );      
      stg_rtk_fig_grid( grid, 
			geom.pose.x, geom.pose.y, 
			geom.size.x, geom.size.y, 1.0  ) ;
    }
  else
    if( grid ) // if we have a grid and don't need one, clear it but keep the fig around      
      stg_rtk_fig_clear( grid );
  
  return 0;
}
