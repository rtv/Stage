
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>

#define DEBUG

#include "rtk.h"
#include "gui.h"

// models that have fewer rectangles than this get matrix rendered when dragged
#define STG_LINE_THRESHOLD 40

#define LASER_FILLED 1

// single static application visible to all funcs in this file
static rtk_app_t *app = NULL; 

// table of world-to-window mappings
static GHashTable* wins;

#include "gui.h"

void gui_startup( int* argc, char** argv[] )
{
  PRINT_DEBUG( "gui startup" );

  rtk_init(argc, argv);
  
  app = rtk_app_create();
  
  wins = g_hash_table_new( g_int_hash, g_int_equal );

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
  
  g_hash_table_destroy( wins );
}

rtk_fig_t* gui_grid_create( rtk_canvas_t* canvas, rtk_fig_t* parent, 
			    double origin_x, double origin_y, double origin_a, 
			    double width, double height, 
			    double major, double minor )
{
  rtk_fig_t* grid = NULL;
  
  grid = rtk_fig_create( canvas, parent, STG_LAYER_GRID );
  
  if( minor > 0)
    {
      rtk_fig_color_rgb32( grid, stg_lookup_color(STG_GRID_MINOR_COLOR ) );
      rtk_fig_grid( grid, origin_x, origin_y, width, height, minor);
    }
  if( major > 0)
    {
      rtk_fig_color_rgb32( grid, stg_lookup_color(STG_GRID_MAJOR_COLOR ) );
      rtk_fig_grid( grid, origin_x, origin_y, width, height, major);
    }

  return grid;
}

gui_window_t* gui_window_create( stg_world_t* world, int xdim, int ydim )
{
  gui_window_t* win = calloc( sizeof(gui_window_t), 1 );

  win->canvas = rtk_canvas_create( app );
  
  // enable all objects on the canvas to find our window object
  win->canvas->userdata = (void*)win; 

  win->statusbar = GTK_STATUSBAR(gtk_statusbar_new());
  
  gtk_box_pack_start(GTK_BOX(win->canvas->layout), 
		     GTK_WIDGET(win->statusbar), FALSE, TRUE, 0);

  rtk_canvas_size( win->canvas, xdim, ydim );
  
  GString* titlestr = g_string_new( "Stage: " );
  g_string_append_printf( titlestr, "%s", world->token );
  
  rtk_canvas_title( win->canvas, titlestr->str );
  g_string_free( titlestr, TRUE );
  
  // todo - destructors for figures
  win->guimods = g_hash_table_new( g_int_hash, g_int_equal );
  
  win->bg = rtk_fig_create( win->canvas, NULL, 0 );
  
  double major = 1.0;
  double minor = 0.2;
  double origin_x = 0.0;
  double origin_y = 0.0;
  double origin_a = 0.0;
  double width = 10;//world->size.x;
  double height = 10;//world->size.y;

  // draw the axis origin lines
  //rtk_fig_color_rgb32( win->grid, stg_lookup_color(STG_GRID_AXIS_COLOR) );  
  //rtk_fig_line( win->grid, 0, 0, width, 0 );
  //rtk_fig_line( win->grid, 0, 0, 0, height );

  win->show_grid = TRUE;
  win->show_matrix = FALSE;
  win->show_rangers = TRUE;
  win->show_rangerdata = FALSE;
  win->show_geom = FALSE;
  win->show_rects = TRUE;
  win->show_laser = TRUE;
  win->show_laserdata = TRUE;

  win->movie_exporting = FALSE;
  win->movie_count = 0;
  win->movie_speed = STG_DEFAULT_MOVIE_SPEED;
  
  win->poses = rtk_fig_create( win->canvas, NULL, 0 );
  
  // start in the center, fully zoomed out
  rtk_canvas_scale( win->canvas, 1.1*width/xdim, 1.1*width/xdim );
  rtk_canvas_origin( win->canvas, width/2.0, height/2.0 );

  gui_window_create_menus( win );

  return win;
}

void gui_window_destroy( gui_window_t* win )
{
  PRINT_DEBUG( "gui window destroy" );

  g_hash_table_destroy( win->guimods );

  rtk_canvas_destroy( win->canvas );
  rtk_fig_destroy( win->bg );
  rtk_fig_destroy( win->poses );
}

void gui_world_create( stg_world_t* world )
{
  PRINT_DEBUG( "gui world create" );
  
  gui_window_t* win = gui_window_create( world, 
					 STG_DEFAULT_WINDOW_WIDTH,
					 STG_DEFAULT_WINDOW_HEIGHT );
  
  // associate this canvas with the world id
  g_hash_table_replace( wins, &world->id, win );
  
  // show the window
  gtk_widget_show_all(win->canvas->frame);
}



  
// render every pixel as an unfilled rectangle

typedef struct
{
  stg_matrix_t* matrix;
  rtk_fig_t* fig;
} gui_mf_t;


void render_matrix_cell( gui_mf_t*mf, stg_matrix_coord_t* coord )
{
  double pixel_size = 1.0 / mf->matrix->ppm;
  
  rtk_fig_rectangle( mf->fig, 
		     coord->x * pixel_size, coord->y*pixel_size, 0, 
		     pixel_size, pixel_size, 0 );
}

void render_matrix_cell_cb( gpointer key, gpointer value, gpointer user )
{
  stg_matrix_coord_t* coord = (stg_matrix_coord_t*)key;
  gui_mf_t* mf = (gui_mf_t*)user;
  
  render_matrix_cell( mf, coord );
}


// useful debug function allows plotting the matrix
void gui_world_matrix( stg_world_t* world, gui_window_t* win )
{
  if( win->matrix == NULL )
    {
      win->matrix = rtk_fig_create(win->canvas,win->bg,STG_LAYER_MATRIX);
      rtk_fig_color_rgb32(win->matrix, stg_lookup_color(STG_MATRIX_COLOR));
      
    }
  else
    rtk_fig_clear( win->matrix );

  //printf( "now %d cells in the matrix\n", 
  //  g_hash_table_size( world->matrix->table ) );
  
  gui_mf_t mf;
  mf.matrix = world->matrix;
  mf.fig = win->matrix;

  if( win->show_matrix )
    g_hash_table_foreach( world->matrix->table, render_matrix_cell_cb, &mf );
}

void gui_pose( rtk_fig_t* fig, stg_model_t* mod )
{
  rtk_fig_arrow_ex( fig, 0,0, mod->pose.x, mod->pose.y, 0.05 );
}


void gui_pose_cb( gpointer key, gpointer value, gpointer user )
{
  gui_pose( (rtk_fig_t*)user, (stg_model_t*)value );
}


void gui_world_update( stg_world_t* world )
{
  //PRINT_DEBUG( "gui world update" );
  
  gui_window_t* win = g_hash_table_lookup( wins, &world->id );
  
  gui_world_matrix( world, win );
  //rtk_fig_clear( win->poses );
  //g_hash_table_foreach( world->models, gui_pose_cb, win->poses );    
  rtk_canvas_render( win->canvas );
}

void gui_world_destroy( stg_world_t* world )
{
  PRINT_DEBUG( "gui world destroy" );

  gui_window_t* win = g_hash_table_lookup( wins, &world->id );
    
  if( win && win->canvas ) 
    rtk_canvas_destroy( win->canvas );
  else
    PRINT_WARN1( "can't find a window for world %d", world->id );
}


const char* gui_model_describe(  stg_model_t* mod )
{
  static char txt[256];
  
  snprintf(txt, sizeof(txt), "\"%s\" (%d:%d) pose: [%.2f,%.2f,%.2f]",  
	   mod->token, mod->world->id, mod->id,  
	   mod->pose.x, mod->pose.y, mod->pose.a  );
  
  return txt;
}


// Process mouse events 
void gui_model_mouse(rtk_fig_t *fig, int event, int mode)
{
  //PRINT_DEBUG2( "ON MOUSE CALLED BACK for %p with userdata %p", fig, fig->userdata );
  
  // each fig links back to the Entity that owns it
  stg_model_t* mod = (stg_model_t*)fig->userdata;
  assert( mod );

  gui_window_t* win = g_hash_table_lookup( wins, &mod->world->id );

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
      memcpy( &capture_vel, &mod->velocity, sizeof(capture_vel) );
      model_set_prop( mod, STG_PROP_VELOCITY, &zero_vel, sizeof(zero_vel) );

      // DELIBERATE NO-BREAK      

    case RTK_EVENT_MOTION:
	

      // move the object to follow the mouse
      rtk_fig_get_origin(fig, &pose.x, &pose.y, &pose.a );
      
      // TODO - if there are more motion events pending, do nothing.
      //if( !gtk_events_pending() )
	
      if( mod->lines->len < STG_LINE_THRESHOLD )
	model_set_prop( mod, STG_PROP_POSE, &pose, sizeof(pose) );
      
      // display the pose
      snprintf(txt, sizeof(txt), "Dragging: %s", gui_model_describe(mod)); 
      cid = gtk_statusbar_get_context_id( win->statusbar, "on_mouse" );
      gtk_statusbar_pop( win->statusbar, cid ); 
      gtk_statusbar_push( win->statusbar, cid, txt ); 
      
      break;
      
    case RTK_EVENT_RELEASE:
      // move the entity to its final position
      rtk_fig_get_origin(fig, &pose.x, &pose.y, &pose.a );
      model_set_prop( mod, STG_PROP_POSE, &pose, sizeof(pose) );
      
      // and restore the velocity at which we grabbed it
      model_set_prop( mod, STG_PROP_VELOCITY, &capture_vel, sizeof(capture_vel) );

      // take the pose message from the status bar
      cid = gtk_statusbar_get_context_id( win->statusbar, "on_mouse" );
      gtk_statusbar_pop( win->statusbar, cid ); 
      break;
      
      
    default:
      break;
    }

  return;
}

void gui_model_parent( stg_model_t* model )
{
  
}

void gui_model_grid( stg_model_t* model )
{  
  gui_window_t* win = g_hash_table_lookup( wins, &model->world->id );
  gui_model_t* gmod = gui_model_figs(model);
  
  if( gmod->grid )
    rtk_fig_destroy( gmod->grid );
  
  gmod->grid = gui_grid_create( win->canvas, gmod->top, 
				0, 0, 0, 
				model->size.x, model->size.y, 1.0, 0 );
}
  
void gui_model_create( stg_model_t* model )
{
  PRINT_DEBUG( "gui model create" );
  
  gui_window_t* win = g_hash_table_lookup( wins, &model->world->id );
  
  rtk_fig_t* parent_fig = win->bg;
  
  // attach to our parent's fig if there is one
  if( model->parent )
    parent_fig = gui_model_figs(model->parent)->top;
  

  gui_model_t* gmod = calloc( sizeof(gui_model_t),1 );
  g_hash_table_replace( win->guimods, &model->id, gmod );
  
  gmod->top = rtk_fig_create( win->canvas, parent_fig, STG_LAYER_BODY );
  gmod->geom =  rtk_fig_create( win->canvas, gmod->top, STG_LAYER_GEOM );
  gmod->rangers = rtk_fig_create( win->canvas, gmod->top, STG_LAYER_SENSOR );
  gmod->ranger_data = rtk_fig_create( win->canvas, gmod->top, STG_LAYER_DATA);
  gmod->laser = rtk_fig_create( win->canvas, gmod->top, STG_LAYER_SENSOR );
#ifdef LASER_FILLED
  gmod->laser_data = rtk_fig_create( win->canvas, gmod->top, STG_LAYER_BACKGROUND);
#else
  gmod->laser_data = rtk_fig_create( win->canvas, gmod->top, STG_LAYER_DATA);
#endif
  gmod->grid = NULL;
  
  gmod->top->userdata = model;
  rtk_fig_movemask( gmod->top, model->movemask );
  rtk_fig_add_mouse_handler( gmod->top, gui_model_mouse );
  
  gui_model_render( model );
}

gui_model_t* gui_model_figs( stg_model_t* model )
{
  gui_window_t* win = g_hash_table_lookup( wins, &model->world->id );  
  return (gui_model_t*)g_hash_table_lookup( win->guimods, &model->id );
}

// draw a model from scratch
void gui_model_render( stg_model_t* model )
{
  PRINT_DEBUG( "gui model render" );
  
  rtk_fig_t* fig = gui_model_figs(model)->top;
  rtk_fig_clear( fig );
  
  rtk_fig_origin( fig, model->pose.x, model->pose.y, model->pose.a );

#ifdef DEBUG 
  //rtk_fig_color_rgb32( fig, stg_lookup_color(STG_BOUNDINGBOX_COLOR) );
  
  // draw the bounding box
  //rtk_fig_rectangle( fig, 0,0,0, model->size.x, model->size.y, 0 );
#endif
 
  gui_window_t* win = g_hash_table_lookup( wins, &model->world->id );  

  if( win->show_rects )
    gui_model_lines( model );

  if( win->show_nose && model->nose )
    gui_model_nose( model );
  
  if( win->show_geom ) 
    gui_model_geom( model );

  if( win->show_rangers ) 
    gui_model_rangers( model );
  
  if( win->show_laser ) 
    gui_model_laser( model );

  if( win->show_grid && model->grid )
    gui_model_grid( model );
}

void gui_model_destroy( stg_model_t* model )
{
  //PRINT_DEBUG( "gui model destroy" );
  
  gui_window_t* win = g_hash_table_lookup( wins, &model->world->id );
  
  
  /*
  gui_model_t* gmod = gui_model_figs( model );

  if( gmod->grid )        rtk_fig_destroy(  gmod->grid );
  if( gmod->laser_data )  rtk_fig_destroy(  gmod->laser_data );
  if( gmod->laser )       rtk_fig_destroy(  gmod->laser );
  if( gmod->ranger_data ) rtk_fig_destroy(  gmod->ranger_data );
  if( gmod->rangers )     rtk_fig_destroy(  gmod->rangers );
  if( gmod->geom )        rtk_fig_destroy(  gmod->geom );
  if( gmod->top )         rtk_fig_destroy(  gmod->top );
  */

  g_hash_table_remove( win->guimods, &model->id );
}

void gui_model_pose( stg_model_t* mod )
{
  //PRINT_DEBUG( "gui model pose" );
  rtk_fig_origin( gui_model_figs(mod)->top, 
		  mod->pose.x, mod->pose.y, mod->pose.a );
}

void gui_model_rangers( stg_model_t* mod )
{
  PRINT_DEBUG( "drawing rangers" );

  rtk_fig_t* fig = gui_model_figs(mod)->rangers;
  
  rtk_fig_color_rgb32(fig, stg_lookup_color("orange") );
  //rtk_fig_color_rgb32( fig, stg_lookup_color(STG_RANGER_COLOR) );
  
  rtk_fig_origin( fig, mod->local_pose.x, mod->local_pose.y, mod->local_pose.a );  
  rtk_fig_clear( fig ); 
  
  // add rects showing ranger positions
  int s;
  for( s=0; s<(int)mod->ranger_config->len; s++ )
    {
      stg_ranger_config_t* rngr 
	= &g_array_index( mod->ranger_config, stg_ranger_config_t, s );
      
      //printf( "drawing a ranger rect (%.2f,%.2f,%.2f)[%.2f %.2f]\n",
      //  rngr->pose.x, rngr->pose.y, rngr->pose.a,
      //  rngr->size.x, rngr->size.y );
      
      rtk_fig_rectangle( fig, 
			 rngr->pose.x, rngr->pose.y, rngr->pose.a,
			 rngr->size.x, rngr->size.y, 0 ); 
      
	// show the FOV too
	double sidelen = rngr->size.x/2.0;
	
	double x1= rngr->pose.x + sidelen*cos(rngr->pose.a - rngr->fov/2.0 );
	double y1= rngr->pose.y + sidelen*sin(rngr->pose.a - rngr->fov/2.0 );
	double x2= rngr->pose.x + sidelen*cos(rngr->pose.a + rngr->fov/2.0 );
	double y2= rngr->pose.y + sidelen*sin(rngr->pose.a + rngr->fov/2.0 );
	
	rtk_fig_line( fig, rngr->pose.x, rngr->pose.y, x1, y1 );
	rtk_fig_line( fig, rngr->pose.x, rngr->pose.y, x2, y2 );	
    }
}

void gui_model_rangers_data( stg_model_t* mod )
{ 
  gui_window_t* win = g_hash_table_lookup( wins, &mod->world->id );  
  
  if( win->show_rangerdata )
    {
      rtk_fig_t* fig = gui_model_figs(mod)->ranger_data;  
      rtk_fig_clear( fig );       
      rtk_fig_color_rgb32(fig, stg_lookup_color(STG_RANGER_COLOR) );
      rtk_fig_origin( fig, 
		      mod->local_pose.x, mod->local_pose.y, mod->local_pose.a );
      
      // draw the range  beams
      int s;
      for( s=0; s< mod->ranger_config->len; s++ )
	{
	  stg_ranger_config_t* rngr 
	    = &g_array_index( mod->ranger_config, stg_ranger_config_t, s );
	  
	  stg_ranger_sample_t* sample 
	    = &g_array_index(mod->ranger_data, stg_ranger_sample_t, s );
	  
	  rtk_fig_arrow( fig, rngr->pose.x, rngr->pose.y, rngr->pose.a, 
			 sample->range, 0.02 );
	}
    }
}

void gui_model_laser( stg_model_t* mod )
{
  // only draw the laser gadget if it has a non-zero size
  if( mod->laser_config.size.x || mod->laser_config.size.y )
    {
      
      rtk_fig_t* fig = gui_model_figs(mod)->laser;  
      
      rtk_fig_clear(fig);
      rtk_fig_color_rgb32(fig, stg_lookup_color(STG_LASER_COLOR) );
      rtk_fig_origin( fig, mod->local_pose.x, mod->local_pose.y, mod->local_pose.a );   
      rtk_fig_rectangle( fig, 
			 mod->laser_config.pose.x, 
			 mod->laser_config.pose.y, 
			 mod->laser_config.pose.a,
			 mod->laser_config.size.x,
			 mod->laser_config.size.y, 0 );
    }
}  

void gui_model_laser_data( stg_model_t* mod )
{
  gui_window_t* win = g_hash_table_lookup( wins, &mod->world->id );  

  if( win->show_laserdata )
    {
      rtk_fig_t* fig = gui_model_figs(mod)->laser_data;  
      rtk_fig_clear( fig ); 
  
      rtk_fig_color_rgb32(fig, stg_lookup_color(STG_LASER_DATA_COLOR) );
      rtk_fig_origin( fig, mod->local_pose.x, mod->local_pose.y, mod->local_pose.a );  
      stg_laser_config_t* cfg = &mod->laser_config;
  
      double sample_incr = cfg->fov / cfg->samples;
      double bearing = cfg->pose.a - cfg->fov/2.0;
      double px, py;
      stg_laser_sample_t* sample = NULL;
      
      double* points = calloc( 1 + sizeof(double) * 2 * cfg->samples, 1 );
      //points[0] = points[1] = 0.0;  // this is implied by the calloc
      
      int s;
      for( s=0; s<cfg->samples; s++ )
	{
	  sample = &g_array_index( mod->laser_data, stg_laser_sample_t, s );
	  
	  // useful debug
	  //rtk_fig_arrow( fig, 0, 0, bearing, (sample->range/1000.0), 0.01 );
	  
	  points[2+2*s] = (sample->range/1000.0) * cos(bearing);
	  points[2+2*s+1] = (sample->range/1000.0) * sin(bearing);
	  
	  bearing += sample_incr;
	}

      rtk_fig_polygon( fig, 0,0,0, cfg->samples+1, points, LASER_FILLED );      

      free( points );
    }
}

void gui_model_lines( stg_model_t* mod )
{
  rtk_fig_t* fig = gui_model_figs(mod)->top;
  
  rtk_fig_clear( fig );

  rtk_fig_color_rgb32( fig, mod->color );

  PRINT_DEBUG1( "rendering %d lines", mod->lines->len );

  double localx = mod->local_pose.x;
  double localy = mod->local_pose.y;
  double locala = mod->local_pose.a;
  
  double cosla = cos(locala);
  double sinla = sin(locala);
  
  // draw lines too
  int l;
  for( l=0; l<mod->lines->len; l++ )
    {
      stg_line_t* line = &g_array_index( mod->lines, stg_line_t, l );
      
      double x1 = localx + line->x1 * cosla - line->y1 * sinla;
      double y1 = localy + line->x1 * sinla + line->y1 * cosla;
      double x2 = localx + line->x2 * cosla - line->y2 * sinla;
      double y2 = localy + line->x2 * sinla + line->y2 * cosla;
      
      rtk_fig_line( fig, x1,y1, x2,y2 );
    }
}



void gui_model_geom( stg_model_t* mod )
{
  printf( "drawing geometry" );

  rtk_fig_t* fig = gui_model_figs(mod)->geom;
  
  rtk_fig_clear( fig );

  rtk_fig_color_rgb32( fig, 0 );
  
  double localx = mod->local_pose.x;
  double localy = mod->local_pose.y;
  double locala = mod->local_pose.a;
  
  if( mod->boundary )
    rtk_fig_rectangle( fig, localx, localy, locala, 
		       mod->size.x, mod->size.y, 0 ); 
  
  // draw the origin and the offset arrow
  double orgx = 0.05;
  double orgy = 0.03;
  rtk_fig_arrow_ex( fig, -orgx, 0, orgx, 0, 0.02 );
  rtk_fig_line( fig, 0,-orgy, 0, orgy );
  rtk_fig_line( fig, 0, 0, localx, localy );
  //rtk_fig_line( fig, localx-orgx, localy, localx+orgx, localy );
  rtk_fig_arrow( fig, localx, localy, locala, orgx, 0.02 );  
  rtk_fig_arrow( fig, localx, localy, locala-M_PI/2.0, orgy, 0.0 );
  rtk_fig_arrow( fig, localx, localy, locala+M_PI/2.0, orgy, 0.0 );
  rtk_fig_arrow( fig, localx, localy, locala+M_PI, orgy, 0.0 );
  //rtk_fig_arrow( fig, localx, localy, 0.0, orgx, 0.0 );  
}


// add a nose  indicating heading  
void gui_model_nose( stg_model_t* mod )
{
  if( mod->nose )
    { 
      rtk_fig_t* fig = gui_model_figs(mod)->top;      
      rtk_fig_color_rgb32( fig, 0 ); // black
      
      double dx = mod->size.x/2.0;
      double dy = mod->size.y/2.0;
      double cosa = cos(mod->local_pose.a);
      double sina = sin(mod->local_pose.a);
      double dxcosa = dx * cosa;
      double dycosa = dy * cosa;
      double dxsina = dx * sina;
      double dysina = dy * sina;
      
      double x = mod->local_pose.x + dxcosa - dysina;
      double y = mod->local_pose.y + dxsina + dycosa;
      double yy = mod->local_pose.y + dxsina -dycosa;
      
      rtk_fig_line( fig, mod->local_pose.x, mod->local_pose.y, 
		    x, y );
      rtk_fig_line( fig, mod->local_pose.x, mod->local_pose.y, 
		    x, yy );
      
      rtk_fig_color_rgb32( fig, mod->color );
    }
}

void gui_model_movemask( stg_model_t* mod )
{
  // we can only manipulate top-level figures
  //if( ent->parent == NULL )
  
  // figure gets the same movemask as the model, ONLY if it is a
  // top-level object
  if( mod->parent == NULL )
    rtk_fig_movemask( gui_model_figs(mod)->top, mod->movemask);  
      

  if( mod->movemask )
    // Set the mouse handler
    rtk_fig_add_mouse_handler( gui_model_figs(mod)->top, gui_model_mouse );
}


void gui_model_update( stg_model_t* mod, stg_prop_type_t prop )
{
  //PRINT_DEBUG3( "gui update for %d:%s prop %s", 
  //	ent->id, ent->name->str, stg_property_string(prop) );
  
  assert( mod );
  //assert( prop >= 0 );
  //assert( prop < STG_PROP_PROP_COUNT );
  
  switch( prop )
    {
    case 0: // for these we basically redraw everything
    case STG_PROP_BOUNDARY:      
    case STG_PROP_SIZE:
    case STG_PROP_COLOR:
    case STG_PROP_NOSE:
    case STG_PROP_ORIGIN: // could this one be faster?
    case STG_PROP_MOVEMASK:
    case STG_PROP_RANGERCONFIG:
    case STG_PROP_GRID:
      gui_model_render( mod );
      gui_model_movemask( mod );
      break;
      
    case STG_PROP_POSE:
      gui_model_pose( mod );
      break;

      //case STG_PROP_RECTS:
      //gui_model_rects( mod );
      //break;
      
    case STG_PROP_LINES:
      gui_model_lines( mod );
      break;

    case STG_PROP_PARENT: 
      gui_model_parent( mod );
      break;
      
    case STG_PROP_BLINKENLIGHT:
      //gui_model_blinkenlight( mod );
      break;
      
      // do nothing for these things
    case STG_PROP_LASERRETURN:
    case STG_PROP_SONARRETURN:
    case STG_PROP_OBSTACLERETURN:
    case STG_PROP_VISIONRETURN:
    case STG_PROP_PUCKRETURN:
    case STG_PROP_VELOCITY:
    case STG_PROP_NEIGHBORBOUNDS:
    case STG_PROP_NEIGHBORRETURN:
    case STG_PROP_LOSMSG:
    case STG_PROP_LOSMSGCONSUME:
    case STG_PROP_NAME:
    case STG_PROP_INTERVAL:
    case STG_PROP_MATRIXRENDER:
      break;

    default:
      PRINT_WARN1( "property change %d unhandled by gui", prop ); 
      break;
    } 
}
 
