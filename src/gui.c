
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>

//#define DEBUG
//#undef DEBUG

#include "stage_internal.h"
#include "gui.h"

#define STG_DEFAULT_WINDOW_WIDTH 700
#define STG_DEFAULT_WINDOW_HEIGHT 740
#define STG_DEFAULT_PPM 40
#define STG_DEFAULT_SHOWGRID 1
#define STG_DEFAULT_MOVIE_SPEED 1


// models that have fewer rectangles than this get matrix rendered when dragged
#define STG_POLY_THRESHOLD 10
#define LASER_FILLED 1
#define BOUNDINGBOX 0

// single static application visible to all funcs in this file
static stg_rtk_app_t *app = NULL; 

// some global figures anyone can draw in for debug purposes
stg_rtk_fig_t* fig_debug_rays = NULL;
stg_rtk_fig_t* fig_debug_geom = NULL;


/** @defgroup window GUI Window

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
  //win->show_grid = wf_read_int(section, "show_grid", 1 );
  
  PRINT_DEBUG2( "window width %d height %d", window_width, window_height );
  PRINT_DEBUG2( "window center (%.2f,%.2f)", window_center_x, window_center_y );
  PRINT_DEBUG1( "window scale %.2f", window_scale );
  
  // ask the canvas to comply
  gtk_window_resize( GTK_WINDOW(win->canvas->frame), window_width, window_height );
  stg_rtk_canvas_scale( win->canvas, window_scale, window_scale );
  stg_rtk_canvas_origin( win->canvas, window_center_x, window_center_y );

  // load the flags that control showing of data and configs
  win->render_data_flag[STG_MODEL_LASER] = wf_read_int(section, "laser_data", 1 );
  win->render_cfg_flag[STG_MODEL_LASER] = wf_read_int(section, "laser_config", 0 );
  win->render_data_flag[STG_MODEL_GRIPPER] = wf_read_int(section, "gripper_data", 1 );
  win->render_cfg_flag[STG_MODEL_GRIPPER] = wf_read_int(section, "gripper_config", 0 );
  win->render_data_flag[STG_MODEL_RANGER] = wf_read_int(section, "ranger_data", 1 );
  win->render_cfg_flag[STG_MODEL_RANGER] = wf_read_int(section, "ranger_config", 0 );
  win->render_data_flag[STG_MODEL_FIDUCIAL] = wf_read_int(section, "fiducial_data", 1 );
  win->render_cfg_flag[STG_MODEL_FIDUCIAL] = wf_read_int(section, "fiducial_config", 0 );  
  win->render_data_flag[STG_MODEL_BLOB] = wf_read_int(section, "blobfinder_data", 1 );
  win->render_cfg_flag[STG_MODEL_BLOB] = wf_read_int(section, "blobfinder_config", 0 );
  win->render_data_flag[STG_MODEL_POSITION] = wf_read_int(section, "position_data", 0 );
  win->render_cfg_flag[STG_MODEL_POSITION] = wf_read_int(section, "position_config", 0 );
  win->render_data_flag[STG_MODEL_GRIPPER] = wf_read_int(section, "gripper_data", 1 );
  win->render_cfg_flag[STG_MODEL_GRIPPER] = wf_read_int(section, "gripper_config", 0 );
  win->render_data_flag[STG_MODEL_ENERGY] = wf_read_int(section, "energy_data", 0 );
  win->render_cfg_flag[STG_MODEL_ENERGY] = wf_read_int(section, "energy_config", 0 );
}

void gui_save( gui_window_t* win )
{
  int width, height;
  gtk_window_get_size(  GTK_WINDOW(win->canvas->frame), &width, &height );
  
  wf_write_tuple_float( win->wf_section, "size", 0, width );
  wf_write_tuple_float( win->wf_section, "size", 1, height );

  wf_write_tuple_float( win->wf_section, "center", 0, win->canvas->ox );
  wf_write_tuple_float( win->wf_section, "center", 1, win->canvas->oy );

  wf_write_float( win->wf_section, "scale", win->canvas->sx );

  wf_write_int( win->wf_section, "fill_polygons", win->fill_polygons );
  //wf_write_int( win->wf_section, "show_grid", win->show_grid );

  // save the flags that control visibility of data and configs
  wf_write_int(win->wf_section, "laser_data", win->render_data_flag[STG_MODEL_LASER]);
  wf_write_int(win->wf_section, "laser_config", win->render_cfg_flag[STG_MODEL_LASER]);
  wf_write_int(win->wf_section, "gripper_data", win->render_data_flag[STG_MODEL_GRIPPER]);
  wf_write_int(win->wf_section, "gripper_config", win->render_cfg_flag[STG_MODEL_GRIPPER]);
  wf_write_int(win->wf_section, "ranger_data", win->render_data_flag[STG_MODEL_RANGER]);
  wf_write_int(win->wf_section, "ranger_config", win->render_cfg_flag[STG_MODEL_RANGER]);
  wf_write_int(win->wf_section, "fiducial_data", win->render_data_flag[STG_MODEL_FIDUCIAL]);
  wf_write_int(win->wf_section, "fiducial_config", win->render_cfg_flag[STG_MODEL_FIDUCIAL]);
  wf_write_int(win->wf_section, "blobfinder_data", win->render_data_flag[STG_MODEL_BLOB]);
  wf_write_int(win->wf_section, "blobfinder_config", win->render_cfg_flag[STG_MODEL_BLOB]);
  wf_write_int(win->wf_section, "position_data", win->render_data_flag[STG_MODEL_POSITION]);
  wf_write_int(win->wf_section, "position_config", win->render_cfg_flag[STG_MODEL_POSITION]);
  wf_write_int(win->wf_section, "energy_data", win->render_data_flag[STG_MODEL_ENERGY]);
  wf_write_int(win->wf_section, "energy_config", win->render_cfg_flag[STG_MODEL_ENERGY]);
  wf_write_int(win->wf_section, "gripper_data", win->render_data_flag[STG_MODEL_GRIPPER]);
  wf_write_int(win->wf_section, "gripper_config", win->render_cfg_flag[STG_MODEL_GRIPPER]);
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

gui_window_t* gui_window_create( stg_world_t* world, int xdim, int ydim )
{
  gui_window_t* win = calloc( sizeof(gui_window_t), 1 );

  win->canvas = stg_rtk_canvas_create( app );
  
  win->world = world;
  
  // enable all objects on the canvas to find our window object
  win->canvas->userdata = (void*)win; 

  win->disable_data = FALSE;
  win->disable_polygons = FALSE;
  win->disable_config = FALSE;
  win->disable_commands = FALSE;

  gui_window_menus_create( win );

  char txt[256];
  snprintf( txt, 256, "Stage v%s", VERSION );
  gtk_statusbar_push( win->canvas->status_bar, 0, txt ); 

  stg_rtk_canvas_size( win->canvas, xdim, ydim );
  
  GString* titlestr = g_string_new( "Stage: " );
  g_string_append_printf( titlestr, "%s", world->token );
  
  stg_rtk_canvas_title( win->canvas, titlestr->str );
  g_string_free( titlestr, TRUE );
  
  double width = 10;//world->size.x;
  //double height = 10;//world->size.y;

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
  win->frame_interval = 500; // ms
  win->frame_format = STK_IMAGE_FORMAT_PNG;

  win->poses = stg_rtk_fig_create( win->canvas, NULL, 0 );

  // start in the center, fully zoomed out
  stg_rtk_canvas_scale( win->canvas, 1.1*width/xdim, 1.1*width/xdim );
  //stg_rtk_canvas_origin( win->canvas, width/2.0, height/2.0 );

  win->selection_active = NULL;
  
  // show all data by default
  int f;
  for( f=0; f<STG_MODEL_COUNT; f++ )
    {
      win->render_data_flag[f] = TRUE;
      win->render_cmd_flag[f] = FALSE;
      win->render_cfg_flag[f] = FALSE;
    }

  // except position
  win->render_data_flag[STG_MODEL_POSITION] = FALSE;
  
  // show the limits of the world
  stg_rtk_canvas_bgcolor( win->canvas, 0.9, 0.9, 0.9 );
  //stg_rtk_fig_color_rgb32( win->bg, 0xFEFEEFF );    
  //stg_rtk_fig_rectangle( win->bg, 0,0,0, world->width, world->height, 1 );
  stg_rtk_fig_color_rgb32( win->bg, 0xFFFFFF );    
  stg_rtk_fig_rectangle( win->bg, 0,0,0, world->width, world->height, 1 );
  
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
  
  gui_window_t* win = gui_window_create( world, 
					 STG_DEFAULT_WINDOW_WIDTH,
					 STG_DEFAULT_WINDOW_HEIGHT );
  
  // show the window
  gtk_widget_show_all(win->canvas->frame);

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
  
  // if this is a leaf node containing data, draw a little cross
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

/*
// useful debug function allows plotting the matrix
void gui_world_matrix( stg_world_t* world, gui_window_t* win )
{
 if( win->matrix )
//gui_world_render_cell( win->matrix, world->matrix->root );
 gui_world_render_cell_occupied( win->matrix, world->matrix->root );

  if( world->matrix->list_cells_added ) 
    {
      printf( "rendering %d cells\n", 
	      g_slist_length( world->matrix->list_cells_added ) ); 
      
      //g_slist_foreach( world->matrix->list_cells_added, 
      //	       gui_world_render_cell_cb,
      //	       win->matrix );

      stg_rtk_fig_color_rgb32( win->matrix, 0xEEEEEE );
      
      while( world->matrix->list_cells_removed )
	{
	  // get the cell from the head of the list
	  stg_cell_t* cell = 
	    (stg_cell_t*)world->matrix->list_cells_removed->data;
	  
	  //printf( "drawing cell at %p\n", cell );
	  
	  stg_rtk_fig_rectangle( win->matrix,
				 cell->x, cell->y, 0.0,
				 cell->size, cell->size, 0 );
	  
	  // if this is a leaf node containing data, draw a little dash
	  if( 0 )//cell->data && g_slist_length( (GSList*)cell->data ) )
	    {
	      stg_rtk_fig_line( win->matrix, 
				cell->x-0.006, cell->y,
				cell->x+0.006, cell->y );
	      
	      //stg_rtk_fig_line( win->matrix, 
	      //		cell->x, cell->y-0.004,
	      //		cell->x, cell->y+0.004 );
	      
	    }
	  
	  // remove the head of the list
	  world->matrix->list_cells_removed = 
	    g_slist_delete_link( world->matrix->list_cells_removed,
				 world->matrix->list_cells_removed );
	}  

      stg_rtk_fig_color_rgb32( win->matrix, 0x00FF00 );

      while( world->matrix->list_cells_added )
	{
	  // get the cell from the head of the list
	  stg_cell_t* cell = 
	    (stg_cell_t*)world->matrix->list_cells_added->data;
	  
	  //printf( "drawing cell at %p\n", cell );
	  
	  stg_rtk_fig_rectangle( win->matrix,
				 cell->x, cell->y, 0.0,
				 cell->size, cell->size, 0 );
	  
	  // if this is a leaf node containing data, draw a little dash
	  if( 0 )//cell->data && g_slist_length( (GSList*)cell->data ) )
	    {
	      stg_rtk_fig_line( win->matrix, 
				cell->x-0.006, cell->y,
				cell->x+0.006, cell->y );
	      
	      //stg_rtk_fig_line( win->matrix, 
	      //		cell->x, cell->y-0.004,
	      //		cell->x, cell->y+0.004 );
	      
	    }
	  
	  // remove the head of the list
	  world->matrix->list_cells_added = 
	    g_slist_delete_link( world->matrix->list_cells_added,
				 world->matrix->list_cells_added );
	}
    }

  
  //else
  //puts( "nothing to render" );
    }
*/

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
  stg_pose_t* pose = 
    stg_model_get_property_fixed( mod, "pose", sizeof(stg_pose_t));
  
  if( pose )
    stg_rtk_fig_arrow_ex( fig, 0,0, pose->x, pose->y, 0.05 );
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
  snprintf( clock, 255, "Time: %lu:%lu:%2lu:%2lu.%2lu\t(sim:%3d real:%3d ratio:%2.2f)",
	    world->sim_time / (24*3600000), // days
	    world->sim_time / 3600000, // hours
	    (world->sim_time % 3600000) / 60000, // minutes
	    (world->sim_time % 60000) / 1000, // seconds
	    world->sim_time % 1000, // milliseconds
	    (int)world->sim_interval,
	    (int)world->real_interval_measured,
	    (double)world->sim_interval / (double)world->real_interval_measured );

  //gtk_label_set_text( win->timelabel, clock );
  
  if( win->selection_active )
    gui_model_display_pose( win->selection_active, "Selection:" );
  else      
    {	  
      guint cid = gtk_statusbar_get_context_id( win->canvas->status_bar, "on_mouse" );
      gtk_statusbar_pop( win->canvas->status_bar, cid ); 
      gtk_statusbar_push( win->canvas->status_bar, cid, clock ); 
    }
  
  if( win->show_geom )
    gui_world_geom( world );
  
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


const char* gui_model_describe(  stg_model_t* mod )
{
  static char txt[256];
  
  stg_pose_t* pose = 
    stg_model_get_property_fixed( mod, "pose", sizeof(stg_pose_t));
  
  snprintf(txt, sizeof(txt), "%s \"%s\" (%d:%d) pose: [%.2f,%.2f,%.2f]",  
	   stg_model_type_string(mod->type), 
	   mod->token, 
	   mod->world->id, 
	   mod->id,  
	   pose->x, pose->y, pose->a  );
  
  return txt;
}


void gui_model_display_pose( stg_model_t* mod, char* verb )
{
  char txt[256];
  gui_window_t* win = mod->world->win;

  // display the pose
  snprintf(txt, sizeof(txt), "%s %s", verb, gui_model_describe(mod)); 
  guint cid = gtk_statusbar_get_context_id( win->canvas->status_bar, "on_mouse" );
  gtk_statusbar_pop( win->canvas->status_bar, cid ); 
  gtk_statusbar_push( win->canvas->status_bar, cid, txt ); 
  //printf( "STATUSBAR: %s\n", txt );
}

// Process mouse events 
void gui_model_mouse(stg_rtk_fig_t *fig, int event, int mode)
{
  //PRINT_DEBUG2( "ON MOUSE CALLED BACK for %p with userdata %p", fig, fig->userdata );
    // each fig links back to the Entity that owns it
  stg_model_t* mod = (stg_model_t*)fig->userdata;
  assert( mod );

  gui_window_t* win = mod->world->win;
  assert(win);

  guint cid=0;
  
  static stg_velocity_t capture_vel;
  stg_pose_t pose;  
  stg_velocity_t zero_vel;
  memset( &zero_vel, 0, sizeof(zero_vel) );

  switch (event)
    {
    case STK_EVENT_MOUSE_OVER:
      win->selection_active = mod;      
      break;

    case STK_EVENT_MOUSE_NOT_OVER:
      win->selection_active = NULL;      
      
      // get rid of the selection info
      cid = gtk_statusbar_get_context_id( win->canvas->status_bar, "on_mouse" );
      gtk_statusbar_pop( win->canvas->status_bar, cid ); 
      break;
      
    case STK_EVENT_PRESS:
      // store the velocity at which we grabbed the model
      stg_model_get_velocity(mod, &capture_vel );
      stg_model_set_velocity( mod, &zero_vel );
      // DELIBERATE NO-BREAK      

    case STK_EVENT_MOTION:       
      // move the object to follow the mouse
      stg_rtk_fig_get_origin(fig, &pose.x, &pose.y, &pose.a );
      
      // TODO - if there are more motion events pending, do nothing.
      //if( !gtk_events_pending() )
	
      // only update simple objects on drag
      //if( mod->polygons->len < STG_POLY_THRESHOLD )
	//stg_model_set_pose( mod, &pose );
      stg_model_set_property( mod, "pose", &pose, sizeof(pose));
      
      // display the pose
      //gui_model_display_pose( mod, "Dragging:" );
      break;
      
    case STK_EVENT_RELEASE:
      // move the entity to its final position
      stg_rtk_fig_get_origin(fig, &pose.x, &pose.y, &pose.a );
      //stg_model_set_pose( mod, &pose );
      stg_model_set_property( mod, "pose", &pose, sizeof(pose));
      
      // and restore the velocity at which we grabbed it
      stg_model_set_velocity( mod, &capture_vel );
      break;      
      
    default:
      break;
    }

  return;
}


void gui_model_create( stg_model_t* mod )
{
  PRINT_DEBUG( "gui model create" );
  
  gui_window_t* win = mod->world->win;  
  stg_rtk_fig_t* parent_fig = win->bg; // default parent
  
 
  // attach instead to our parent's fig if there is one
  if( mod->parent )
    {
      gui_model_t* parent_gui = 
	stg_model_get_property_fixed( mod->parent, "gui", sizeof(gui_model_t));
      
      if( parent_gui )
	parent_fig = parent_gui->top;
    }

  // clean the structure
  gui_model_t gui;
  memset( &gui, 0, sizeof(gui_model_t) );
  
  gui.top = 
    stg_rtk_fig_create( mod->world->win->canvas, parent_fig, STG_LAYER_BODY );

  gui.geom = 
    stg_rtk_fig_create( mod->world->win->canvas, gui.top, STG_LAYER_GEOM );

  gui.top->userdata = mod;
  
  stg_model_set_property( mod, "gui", &gui, sizeof(gui));

  gui_model_features( mod );
}

gui_model_t* gui_model_figs( stg_model_t* model )
{ 
  return( (gui_model_t*)stg_model_get_property_fixed( model, "gui", sizeof(gui_model_t)));
}

void gui_model_destroy( stg_model_t* model )
{
  PRINT_DEBUG( "gui model destroy" );

  // TODO - It's too late to kill the figs - the canvas is gone! fix this?
  
  gui_model_t* gui = 
    stg_model_get_property_fixed( model, "gui", sizeof(gui_model_t));

  if( gui )
    {
      if( gui->top ) stg_rtk_fig_destroy( gui->top );
      if( gui->grid ) stg_rtk_fig_destroy( gui->grid );
      if( gui->geom ) stg_rtk_fig_destroy( gui->geom );
    }
  // todo - erase the property figs
}


// add a nose  indicating heading  
void gui_model_features( stg_model_t* mod )
{
  stg_guifeatures_t *gf = 
    stg_model_get_property_fixed( mod, "gui_features",
				  sizeof(stg_guifeatures_t) );
  if( gf == NULL )
    return;
  
  PRINT_DEBUG3( "model %d gui features grid %d nose %d mask %d",
		(int)gf->grid, (int)gf->nose, (int)gf->movemask );

  stg_geom_t geom;
  stg_model_get_geom(mod, &geom);
    
  gui_window_t* win = mod->world->win;
  
  // if we need a nose, draw one
  if( gf->nose )
    { 
      stg_rtk_fig_t* fig = gui_model_figs(mod)->top;      
      
      stg_color_t *col = 
	stg_model_get_property_fixed( mod, "color", sizeof(stg_color_t));

      if( col )
	stg_rtk_fig_color_rgb32( fig, *col );
      
      // draw an arrow from the center to the front of the model
      stg_rtk_fig_arrow( fig, geom.pose.x, geom.pose.y, geom.pose.a, 
		     geom.size.x/2.0, 0.05 );
    }

  stg_rtk_fig_movemask( gui_model_figs(mod)->top, gf->movemask);  
  
  // only install a mouse handler if the object needs one
  //(TODO can we remove mouse handlers dynamically?)
  if( gf->movemask )    
    stg_rtk_fig_add_mouse_handler( gui_model_figs(mod)->top, gui_model_mouse );
  
  // if we need a grid and don't have one, make one
  if( gf->grid && gui_model_figs(mod)->grid == NULL )
    {
      gui_model_figs(mod)->grid = 
	stg_rtk_fig_create( win->canvas, gui_model_figs(mod)->top, STG_LAYER_GRID);
      
      stg_rtk_fig_color_rgb32( gui_model_figs(mod)->grid, 
			   stg_lookup_color(STG_GRID_MAJOR_COLOR ) );
      
      stg_rtk_fig_grid( gui_model_figs(mod)->grid, 
		    geom.pose.x, geom.pose.y, 
		    geom.size.x, geom.size.y, 1.0  ) ;
    }

  
  // if we have a grid and don't need one, destroy it
  if( !gf->grid && gui_model_figs(mod)->grid )
    {
      stg_rtk_fig_destroy( gui_model_figs(mod)->grid );
      gui_model_figs(mod)->grid == NULL;
    }  
}

void stg_model_render_polygons( stg_model_t* mod )
{
  //printf( "render polygons for model %s\n",
  //  mod->token );

  if( mod->world->win->disable_polygons )
    {
      //puts( "polys disabled" );
      return;
    }

  stg_rtk_fig_t* fig = gui_model_figs(mod)->top;
  
  if( fig == NULL )
    return;
  //else

  stg_rtk_fig_clear( fig );
  
  // don't draw objects with no size 
  //if( mod->geom.size.x == 0 && mod->geom.size.y == 0 )
  //return;
  
  stg_color_t *col = 
    stg_model_get_property_fixed( mod, "color", sizeof(stg_color_t));
  
  size_t count=0;
  stg_polygon_t* polys = stg_model_get_polygons(mod,&count);

  stg_geom_t geom;
  stg_model_get_geom(mod, &geom);


  if( polys )
    {
      if( ! mod->world->win->fill_polygons )
	{
	  if( col )
	    stg_rtk_fig_color_rgb32( fig, *col );
	  
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
	  if( col )
	    stg_rtk_fig_color_rgb32( fig, *col );
	  
	  int p;
	  for( p=0; p<count; p++ )
	    stg_rtk_fig_polygon( fig,
			     geom.pose.x,
			     geom.pose.y,
			     geom.pose.a,
			     polys[p].points->len,
			     polys[p].points->data,
			     1 );
	  
	  stg_guifeatures_t* gf = 
	    stg_model_get_property_fixed( mod, "gui_features", 
					       sizeof(stg_guifeatures_t) );
	  if( gf && gf->outline )
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
	    }
	}
    }
  
  stg_bool_t* boundary = 
    stg_model_get_property_fixed( mod, "boundary", sizeof(stg_bool_t));

  if( boundary && *boundary )
    {      
      stg_rtk_fig_rectangle( gui_model_figs(mod)->top, 
			     geom.pose.x, geom.pose.y, geom.pose.a, 
			     geom.size.x, geom.size.y, 0 ); 
    }  
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
void gui_model_move( stg_model_t* mod )
{ 
  stg_pose_t* pose = 
    stg_model_get_property_fixed( mod, "pose", sizeof(stg_pose_t));
  
  if( pose )
    stg_rtk_fig_origin( gui_model_figs(mod)->top, 
			pose->x, pose->y, pose->a );   
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

void gui_model_render_data( stg_model_t* mod )
{
  gui_model_t* gui = 
    stg_model_get_property_fixed( mod, "gui", sizeof(gui_model_t));

  if( gui )
    {
      // if a rendering callback was registered, and the gui wants to
      // render this type of data, call it
      if( mod->f_render_data && 
	  mod->world->win->render_data_flag[mod->type] )
	(*mod->f_render_data)(mod);
      else
	{
	  // remove any graphics that linger
	  if( gui->data )
	    stg_rtk_fig_clear( gui->data );
	  
	  if( gui->data_bg )
	    stg_rtk_fig_clear( gui->data_bg );
	}
    }
} 

void gui_model_render_command( stg_model_t* mod )
{
  gui_model_t* gui = 
    stg_model_get_property_fixed( mod, "gui", sizeof(gui_model_t));

  if( gui )
    {
      // if a rendering callback was registered, and the gui wants to
      // render this type of command, call it
      if( mod->f_render_cmd && 
	  mod->world->win->render_cmd_flag[mod->type] )
	(*mod->f_render_cmd)(mod);
      else
	{
	  // remove any graphics that linger
	  if( gui->cmd )
	    stg_rtk_fig_clear( gui->cmd );
	  
	  if( gui->cmd_bg )
	    stg_rtk_fig_clear( gui->cmd_bg );
	}
    }
}

  void gui_model_render_config( stg_model_t* mod )
{
  gui_model_t* gui = 
    stg_model_get_property_fixed( mod, "gui", sizeof(gui_model_t));

  if( gui )
    {
      // if a rendering callback was registered, and the gui wants to
      // render this type of cfg, call it
      if( mod->f_render_cfg && 
	  mod->world->win->render_cfg_flag[mod->type] )
	(*mod->f_render_cfg)(mod);
      else
	{
	  // remove any graphics that linger
	  if( gui->cfg )
	    stg_rtk_fig_clear( gui->cfg );
	  
	  if( gui->cfg_bg )
	    stg_rtk_fig_clear( gui->cfg_bg );
	} 
    }
}
