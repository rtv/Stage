
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>

//#define DEBUG
//#undef DEBUG

#include "config.h"
#include "rtk.h"
#include "gui.h"

// models that have fewer rectangles than this get matrix rendered when dragged
#define STG_LINE_THRESHOLD 40
#define LASER_FILLED 1
#define BOUNDINGBOX 0

// single static application visible to all funcs in this file
static rtk_app_t *app = NULL; 

// here's a figure anyone can draw in for debug purposes
rtk_fig_t* fig_debug = NULL;

void gui_startup( int* argc, char** argv[] )
{
  PRINT_DEBUG( "gui startup" );

  rtk_init(argc, argv);
  
  app = rtk_app_create();
  rtk_app_main_init( app );
}

void gui_poll( void )
{
  //PRINT_DEBUG( "gui poll" );
  rtk_app_main_loop( app );
}

void gui_shutdown( void )
{
  PRINT_DEBUG( "gui shutdown" );
  rtk_app_main_term( app );  
}



gui_window_t* gui_window_create( world_t* world, int xdim, int ydim )
{
  gui_window_t* win = calloc( sizeof(gui_window_t), 1 );

  win->canvas = rtk_canvas_create( app );
  
  win->world = world;
  
  // enable all objects on the canvas to find our window object
  win->canvas->userdata = (void*)win; 

  GtkHBox* hbox = GTK_HBOX(gtk_hbox_new( TRUE, 10 ));
  
  win->statusbar = GTK_STATUSBAR(gtk_statusbar_new());
  gtk_statusbar_set_has_resize_grip( win->statusbar, FALSE );
  gtk_box_pack_start(GTK_BOX(hbox), 
		     GTK_WIDGET(win->statusbar), FALSE, TRUE, 0);
  
  win->timelabel = GTK_LABEL(gtk_label_new("0:00:00"));
  gtk_box_pack_end(GTK_BOX(hbox), 
		   GTK_WIDGET(win->timelabel), FALSE, FALSE, 0);

  gtk_box_pack_start(GTK_BOX(win->canvas->layout), 
		     GTK_WIDGET(hbox), FALSE, TRUE, 0);


  char txt[256];
  snprintf( txt, 256, "Stage v%s", VERSION );
  gtk_statusbar_push( win->statusbar, 0, txt ); 

  rtk_canvas_size( win->canvas, xdim, ydim );
  
  GString* titlestr = g_string_new( "Stage: " );
  g_string_append_printf( titlestr, "%s", world->token );
  
  rtk_canvas_title( win->canvas, titlestr->str );
  g_string_free( titlestr, TRUE );
  
  // todo - destructors for figures
  //win->guimods = g_hash_table_new( g_int_hash, g_int_equal );
  
  win->bg = rtk_fig_create( win->canvas, NULL, 0 );
  
  double width = 10;//world->size.x;
  double height = 10;//world->size.y;

  // draw the axis origin lines
  //rtk_fig_color_rgb32( win->grid, stg_lookup_color(STG_GRID_AXIS_COLOR) );  
  //rtk_fig_line( win->grid, 0, 0, width, 0 );
  //rtk_fig_line( win->grid, 0, 0, 0, height );

  win->show_matrix = FALSE;

  win->movie_exporting = FALSE;
  win->movie_count = 0;
  win->movie_speed = STG_DEFAULT_MOVIE_SPEED;
  
  win->poses = rtk_fig_create( win->canvas, NULL, 0 );

  // start in the center, fully zoomed out
  rtk_canvas_scale( win->canvas, 1.1*width/xdim, 1.1*width/xdim );
  rtk_canvas_origin( win->canvas, width/2.0, height/2.0 );

  gui_window_menus_create( win );

  return win;
}

void gui_window_destroy( gui_window_t* win )
{
  PRINT_DEBUG( "gui window destroy" );

  //g_hash_table_destroy( win->guimods );

  rtk_canvas_destroy( win->canvas );
  rtk_fig_destroy( win->bg );
  rtk_fig_destroy( win->poses );
}

gui_window_t* gui_world_create( world_t* world )
{
  PRINT_DEBUG( "gui world create" );
  
  gui_window_t* win = gui_window_create( world, 
					 STG_DEFAULT_WINDOW_WIDTH,
					 STG_DEFAULT_WINDOW_HEIGHT );
  
  // show the window
  gtk_widget_show_all(win->canvas->frame);

  return win;
}


// render every occupied matrix pixel as an unfilled rectangle
typedef struct
{
  double ppm;
  rtk_fig_t* fig;
} gui_mf_t;


void render_matrix_cell( gui_mf_t*mf, stg_matrix_coord_t* coord )
{
  double pixel_size = 1.0 / mf->ppm;  
  rtk_fig_rectangle( mf->fig, 
		     (double)coord->x * pixel_size + pixel_size/2.0, 
		     (double)coord->y * pixel_size + pixel_size/2.0, 0, 
		     pixel_size, pixel_size, 0 );
}

void render_matrix_cell_cb( gpointer key, gpointer value, gpointer user )
{
  GPtrArray* arr = (GPtrArray*)value;

  if( arr->len > 0 )
    {  
      stg_matrix_coord_t* coord = (stg_matrix_coord_t*)key;
      gui_mf_t* mf = (gui_mf_t*)user;
      
      render_matrix_cell( mf, coord );
    }
}


// useful debug function allows plotting the matrix
void gui_world_matrix( world_t* world, gui_window_t* win )
{
  if( win->matrix == NULL )
    {
      win->matrix = rtk_fig_create(win->canvas,win->bg,STG_LAYER_MATRIX);
      rtk_fig_color_rgb32(win->matrix, stg_lookup_color(STG_MATRIX_COLOR));
      
    }
  else
    rtk_fig_clear( win->matrix );
  
  gui_mf_t mf;
  mf.fig = win->matrix;

  mf.ppm = world->matrix->ppm;
  g_hash_table_foreach( world->matrix->table, render_matrix_cell_cb, &mf );
  
  mf.ppm = world->matrix->medppm;
  g_hash_table_foreach( world->matrix->medtable, render_matrix_cell_cb, &mf );
  
  mf.ppm = world->matrix->bigppm;
  g_hash_table_foreach( world->matrix->bigtable, render_matrix_cell_cb, &mf );
}

void gui_pose( rtk_fig_t* fig, model_t* mod )
{
  stg_pose_t* pose = model_get_pose( mod );
  rtk_fig_arrow_ex( fig, 0,0, pose->x, pose->y, 0.05 );
}


void gui_pose_cb( gpointer key, gpointer value, gpointer user )
{
  gui_pose( (rtk_fig_t*)user, (model_t*)value );
}


void gui_world_update( world_t* world )
{
  //PRINT_DEBUG( "gui world update" );
  
  gui_window_t* win = world->win;
  
  if( rtk_canvas_isclosed( win->canvas ) )
    {
      //PRINT_WARN( "marking world for destruction" );
      //world->destroy = TRUE;
    }
  else
    {
      if( win->show_matrix ) gui_world_matrix( world, win );
      
      char clock[256];
      snprintf( clock, 255, "%lu:%2lu:%2lu.%3lu\n",
		world->sim_time / 3600000, // hours
		(world->sim_time % 3600000) / 60000, // minutes
		(world->sim_time % 60000) / 1000, // seconds
		world->sim_time % 1000 ); // milliseconds
      
      gtk_label_set_text( win->timelabel, clock );
      
      rtk_canvas_render( win->canvas );      
    }
}

void gui_world_destroy( world_t* world )
{
  PRINT_DEBUG( "gui world destroy" );
  
  if( world->win && world->win->canvas ) 
    rtk_canvas_destroy( world->win->canvas );
  else
    PRINT_WARN1( "can't find a window for world %d", world->id );
}


const char* gui_model_describe(  model_t* mod )
{
  static char txt[256];
  
  stg_pose_t* pose = model_get_pose( mod );

  snprintf(txt, sizeof(txt), "\"%s\" (%d:%d) pose: [%.2f,%.2f,%.2f]",  
	   mod->token, mod->world->id, mod->id,  
	   pose->x, pose->y, pose->a  );
  
  return txt;
}


// Process mouse events 
void gui_model_mouse(rtk_fig_t *fig, int event, int mode)
{
  //PRINT_DEBUG2( "ON MOUSE CALLED BACK for %p with userdata %p", fig, fig->userdata );
    // each fig links back to the Entity that owns it
  model_t* mod = (model_t*)fig->userdata;
  assert( mod );

  gui_window_t* win = mod->world->win;
  assert(win);

  guint cid; // statusbar context id
  char txt[256];
    
  static stg_velocity_t capture_vel;
  stg_pose_t pose;  
  stg_velocity_t zero_vel;
  memset( &zero_vel, 0, sizeof(zero_vel) );

  switch (event)
    {
    case RTK_EVENT_PRESS:
      // store the velocity at which we grabbed the model
      memcpy( &capture_vel, model_get_velocity(mod), sizeof(capture_vel) );
      model_set_velocity( mod, &zero_vel );

      // DELIBERATE NO-BREAK      

    case RTK_EVENT_MOTION:       
      // move the object to follow the mouse
      rtk_fig_get_origin(fig, &pose.x, &pose.y, &pose.a );
      
      // TODO - if there are more motion events pending, do nothing.
      //if( !gtk_events_pending() )
	
      // only update simple objects on drag
      if( mod->lines_count < STG_LINE_THRESHOLD )
	model_set_pose( mod, &pose );
      
      // display the pose
      snprintf(txt, sizeof(txt), "Dragging: %s", gui_model_describe(mod)); 
      cid = gtk_statusbar_get_context_id( win->statusbar, "on_mouse" );
      gtk_statusbar_pop( win->statusbar, cid ); 
      gtk_statusbar_push( win->statusbar, cid, txt ); 
      //printf( "STATUSBAR: %s\n", txt );
      break;
      
    case RTK_EVENT_RELEASE:
      // move the entity to its final position
      rtk_fig_get_origin(fig, &pose.x, &pose.y, &pose.a );
      model_set_pose( mod, &pose );
      
      // and restore the velocity at which we grabbed it
      model_set_velocity( mod, &capture_vel );

      // take the pose message from the status bar
      cid = gtk_statusbar_get_context_id( win->statusbar, "on_mouse" );
      gtk_statusbar_pop( win->statusbar, cid ); 
      break;      
      
    default:
      break;
    }

  return;
}

void gui_model_parent( model_t* model )
{
  
}

void gui_model_create( model_t* model )
{
  PRINT_DEBUG( "gui model create" );
  
  gui_window_t* win = model->world->win;  
  rtk_fig_t* parent_fig = win->bg; // default parent
  
  // attach instead to our parent's fig if there is one
  if( model->parent )
    parent_fig = model->parent->gui.top;
  
  // clean the structure
  memset( &model->gui, 0, sizeof(gui_model_t) );
  
  model->gui.top = 
    rtk_fig_create( model->world->win->canvas, parent_fig, STG_LAYER_BODY );

  model->gui.geom = 
    rtk_fig_create( model->world->win->canvas, parent_fig, STG_LAYER_GEOM );

  model->gui.top->userdata = model;

  gui_model_features( model );
}

gui_model_t* gui_model_figs( model_t* model )
{
  return &model->gui;
}

void gui_model_destroy( model_t* model )
{
  PRINT_DEBUG( "gui model destroy" );

  // TODO - It's too late to kill the figs - the canvas is gone! fix this?

  if( model->gui.top ) rtk_fig_destroy( model->gui.top );
  if( model->gui.grid ) rtk_fig_destroy( model->gui.grid );
  if( model->gui.geom ) rtk_fig_destroy( model->gui.geom );
  // todo - erase the property figs
}


// add a nose  indicating heading  
void gui_model_features( model_t* mod )
{
  stg_guifeatures_t* gf = model_get_guifeatures( mod );

  
  PRINT_DEBUG4( "model %d gui features grid %d nose %d boundary mask %d",
		(int)gf->grid, (int)gf->nose, (int)gf->boundary, (int)gf->movemask );

  
  gui_window_t* win = mod->world->win;
  
  // if we need a nose, draw one
  if( gf->nose )
    { 
      rtk_fig_t* fig = gui_model_figs(mod)->top;      
      rtk_fig_color_rgb32( fig, model_get_color(mod) );
      
      stg_geom_t* geom = model_get_geom(mod);
      
      // draw a line from the center to the front of the model
      rtk_fig_line( fig, 
		    geom->pose.x, 
		    geom->pose.y, 
		    geom->size.x/2, 0 );
    }

  rtk_fig_movemask( gui_model_figs(mod)->top, gf->movemask);  
  
  // only install a mouse handler if the object needs one
  //(TODO can we remove mouse handlers dynamically?)
  if( gf->movemask )    
    rtk_fig_add_mouse_handler( gui_model_figs(mod)->top, gui_model_mouse );
  
  // if we need a grid and don't have one, make one
  if( gf->grid && gui_model_figs(mod)->grid == NULL )
    {
      gui_model_figs(mod)->grid = 
	rtk_fig_create( win->canvas, gui_model_figs(mod)->top, STG_LAYER_GRID);
      
      rtk_fig_color_rgb32( gui_model_figs(mod)->grid, 
			   stg_lookup_color(STG_GRID_MAJOR_COLOR ) );
      
      stg_geom_t* geom = model_get_geom(mod);
      
      rtk_fig_grid( gui_model_figs(mod)->grid, 
		    geom->pose.x, geom->pose.y, 
		    geom->size.x, geom->size.y, 1.0  ) ;
    }

  
  // if we have a grid and don't need one, destroy it
  if( !gf->grid && gui_model_figs(mod)->grid )
    {
      rtk_fig_destroy( gui_model_figs(mod)->grid );
      gui_model_figs(mod)->grid == NULL;
    }

  
  if( gf->boundary )
    {
      stg_geom_t* geom = model_get_geom(mod);
      
      rtk_fig_rectangle( gui_model_figs(mod)->grid, 
			 geom->pose.x, geom->pose.y, geom->pose.a, 
			 geom->size.x, geom->size.y, 0 ); 
    }
}



