
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>

#undef DEBUG

#include "config.h"
#include "rtk.h"
#include "gui.h"

// models that have fewer rectangles than this get matrix rendered when dragged
#define STG_LINE_THRESHOLD 40

#define LASER_FILLED 1

// single static application visible to all funcs in this file
static rtk_app_t *app = NULL; 

// table of world-to-window mappings
static GHashTable* wins;

rtk_fig_t* fig_debug = NULL;

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
  win->guimods = g_hash_table_new( g_int_hash, g_int_equal );
  
  win->bg = rtk_fig_create( win->canvas, NULL, 0 );
  
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
  win->show_blobdata = TRUE;
  win->show_fiducialdata = TRUE;

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

  g_hash_table_destroy( win->guimods );

  rtk_canvas_destroy( win->canvas );
  rtk_fig_destroy( win->bg );
  rtk_fig_destroy( win->poses );
}

void gui_world_create( world_t* world )
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
  //stg_matrix_t* matrix;
  double ppm;
  rtk_fig_t* fig;
} gui_mf_t;


void render_matrix_cell( gui_mf_t*mf, stg_matrix_coord_t* coord )
{
  //double pixel_size = 1.0 / mf->matrix->ppm;
  double pixel_size = 1.0 / mf->ppm;
  
  rtk_fig_rectangle( mf->fig, 
		     coord->x * pixel_size + pixel_size/2.0, 
		     coord->y*pixel_size + pixel_size/2.0, 0, 
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

  //printf( "now %d cells in the matrix\n", 
  //  g_hash_table_size( world->matrix->table ) );
  
  gui_mf_t mf;
  //mf.matrix = world->matrix;
  mf.fig = win->matrix;

  if( win->show_matrix )
    {
      mf.ppm = world->matrix->ppm;
      g_hash_table_foreach( world->matrix->table, render_matrix_cell_cb, &mf );
      
      mf.ppm = world->matrix->medppm;
      g_hash_table_foreach( world->matrix->table, render_matrix_cell_cb, &mf );

      mf.ppm = world->matrix->bigppm;
      g_hash_table_foreach( world->matrix->bigtable, render_matrix_cell_cb, &mf );
    }
}

void gui_pose( rtk_fig_t* fig, model_t* mod )
{
  rtk_fig_arrow_ex( fig, 0,0, mod->pose.x, mod->pose.y, 0.05 );
}


void gui_pose_cb( gpointer key, gpointer value, gpointer user )
{
  gui_pose( (rtk_fig_t*)user, (model_t*)value );
}


void gui_world_update( world_t* world )
{
  //PRINT_DEBUG( "gui world update" );
  
  gui_window_t* win = g_hash_table_lookup( wins, &world->id );
  
  gui_world_matrix( world, win );
  //rtk_fig_clear( win->poses );
  //g_hash_table_foreach( world->models, gui_pose_cb, win->poses );    

  rtk_canvas_render( win->canvas );


  char clock[256];
  snprintf( clock, 255, "%lu:%2lu:%2lu.%3lu\n",
	    world->sim_time / 3600000, // hours
	    (world->sim_time % 3600000) / 60000, // minutes
	    (world->sim_time % 60000) / 1000, // seconds
	    world->sim_time % 1000 ); // milliseconds

  //puts( clock );
  gtk_label_set_text( win->timelabel, clock );

}

void gui_world_destroy( world_t* world )
{
  PRINT_DEBUG( "gui world destroy" );

  gui_window_t* win = g_hash_table_lookup( wins, &world->id );
    
  if( win && win->canvas ) 
    rtk_canvas_destroy( win->canvas );
  else
    PRINT_WARN1( "can't find a window for world %d", world->id );
}


const char* gui_model_describe(  model_t* mod )
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
  model_t* mod = (model_t*)fig->userdata;
  assert( mod );

  gui_window_t* win = g_hash_table_lookup( wins, &mod->world->id );
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
      memcpy( &capture_vel, &mod->velocity, sizeof(capture_vel) );
      model_set_prop( mod, STG_PROP_VELOCITY, &zero_vel, sizeof(zero_vel) );

      // DELIBERATE NO-BREAK      

    case RTK_EVENT_MOTION:
	

      // move the object to follow the mouse
      rtk_fig_get_origin(fig, &pose.x, &pose.y, &pose.a );
      
      // TODO - if there are more motion events pending, do nothing.
      //if( !gtk_events_pending() )
	
      // only update simple objects on drag
      //if( mod->lines->len < STG_LINE_THRESHOLD )
      model_set_prop( mod, STG_PROP_POSE, &pose, sizeof(pose) );
      

      // TODO BUG
      // -- win->statusbar points to the wrong place here. why?
      // printf( "\n\nwin: %p\nsb: %p\n", win, win->statusbar );

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

void gui_model_parent( model_t* model )
{
  
}

void gui_model_grid( model_t* model )
{  
  gui_window_t* win = g_hash_table_lookup( wins, &model->world->id );
  gui_model_t* gmod = gui_model_figs(model);

  assert( gmod );

  if( gmod->grid )
    rtk_fig_destroy( gmod->grid );
  
  if( win->show_grid && model->grid )
    gmod->grid = gui_grid_create( win->canvas, gmod->top, 
				  0, 0, 0, 
				  model->size.x, model->size.y, 1.0, 0 );
}
  
void gui_model_create( model_t* model )
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

  gmod->blob_data = rtk_fig_create( win->canvas, parent_fig, STG_LAYER_DATA);
  gmod->fiducial_data = rtk_fig_create( win->canvas, parent_fig, STG_LAYER_BACKGROUND);
  
  gmod->grid = NULL;
  
  gmod->top->userdata = model;
  rtk_fig_movemask( gmod->top, model->movemask );
  rtk_fig_add_mouse_handler( gmod->top, gui_model_mouse );
  
  gui_model_render( model );
}

gui_model_t* gui_model_figs( model_t* model )
{
  gui_window_t* win = g_hash_table_lookup( wins, &model->world->id );  
  return (gui_model_t*)g_hash_table_lookup( win->guimods, &model->id );
}

// draw a model from scratch
void gui_model_render( model_t* model )
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
  
  // TODO - could i just look this up once, rather than in each of
  //the calls below?
  
  //gui_window_t* win = g_hash_table_lookup( wins, &model->world->id );

  gui_model_lines( model );
  gui_model_nose( model );
  gui_model_geom( model );
  gui_model_rangers( model );
  gui_model_rangers_data( model );
  gui_model_laser( model );
  gui_model_laser_data( model );
  gui_model_grid( model );
  gui_model_blobfinder_data( model );
  gui_model_fiducial_data( model );

}

void gui_model_destroy( model_t* model )
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

void gui_model_pose( model_t* mod )
{
  //PRINT_DEBUG( "gui model pose" );
  rtk_fig_origin( gui_model_figs(mod)->top, 
		  mod->pose.x, mod->pose.y, mod->pose.a );
}

void gui_model_rangers( model_t* mod )
{
  //PRINT_DEBUG( "drawing rangers" );

  rtk_fig_t* fig = gui_model_figs(mod)->rangers;
  
  rtk_fig_color_rgb32(fig, stg_lookup_color("orange") );
  //rtk_fig_color_rgb32( fig, stg_lookup_color(STG_RANGER_COLOR) );
  
  rtk_fig_origin( fig, mod->local_pose.x, mod->local_pose.y, mod->local_pose.a );  
  rtk_fig_clear( fig ); 
  

  stg_property_t* prop = model_get_prop_generic( mod, STG_PROP_RANGERCONFIG );
  
  if( prop == NULL ) // no rangers to update!
    return;
  //else
  
  stg_ranger_config_t* cfg = (stg_ranger_config_t*)prop->data;
  assert( cfg );

  int rcount = prop->len / sizeof( stg_ranger_config_t );
  
  if( rcount < 1 )
    return;


  // add rects showing ranger positions
  int s;
  for( s=0; s<rcount; s++ )
    {
      stg_ranger_config_t* rngr = &cfg[s];
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

void gui_model_rangers_data( model_t* mod )
{ 
  gui_window_t* win = g_hash_table_lookup( wins, &mod->world->id );  
  
  rtk_fig_t* fig = gui_model_figs(mod)->ranger_data;  
  if( fig ) rtk_fig_clear( fig );   
  
  if( win->show_rangerdata && mod->subs[STG_PROP_RANGERDATA] )
    {
      
      stg_property_t* prop = model_get_prop_generic( mod, STG_PROP_RANGERCONFIG );
      
      if( prop == NULL ) // no rangers to update!
	return;
      
      stg_ranger_config_t* cfg = (stg_ranger_config_t*)prop->data;  
      int rcount = prop->len / sizeof( stg_ranger_config_t );
      
      stg_ranger_sample_t* samples = (stg_ranger_sample_t*)
	model_get_prop_data_generic( mod, STG_PROP_RANGERDATA );
      
      if( rcount > 0 && cfg && samples )
	{
	  rtk_fig_color_rgb32(fig, stg_lookup_color(STG_RANGER_COLOR) );
	  rtk_fig_origin( fig, 
			  mod->local_pose.x, mod->local_pose.y, mod->local_pose.a );	  
	  // draw the range  beams
	  int s;
	  for( s=0; s<rcount; s++ )
	    {
	      if( samples[s].range > 0.0 )
		{
		  stg_ranger_config_t* rngr = &cfg[s];
		  
		  rtk_fig_arrow( fig, rngr->pose.x, rngr->pose.y, rngr->pose.a, 			       samples[s].range, 0.02 );
		}
	    }
	}
    }
}

void gui_model_laser( model_t* mod )
{
  stg_geom_t* lgeom = (stg_geom_t*)
    model_get_prop_data_generic( mod, STG_PROP_LASERGEOM );
  
  // only draw the laser gadget if it has a non-zero size
  if( lgeom->size.x || lgeom->size.y )
    {      
      rtk_fig_t* fig = gui_model_figs(mod)->laser;  
      
      rtk_fig_clear(fig);
      rtk_fig_color_rgb32(fig, stg_lookup_color(STG_LASER_TRANSDUCER_COLOR) );
      rtk_fig_origin( fig, mod->local_pose.x, mod->local_pose.y, mod->local_pose.a );   
      rtk_fig_rectangle( fig, 
			 lgeom->pose.x, 
			 lgeom->pose.y, 
			 lgeom->pose.a,
			 lgeom->size.x,
			 lgeom->size.y, 0 );
    }
}  

void gui_model_blobfinder_data( model_t* mod )
{ 
  gui_window_t* win = g_hash_table_lookup( wins, &mod->world->id );  
  rtk_fig_t* fig = gui_model_figs(mod)->blob_data;  
  rtk_fig_clear(fig);

  if( win->show_blobdata && mod->subs[STG_PROP_BLOBDATA] )
    {

      // The vision figure is attached to the entity, but we dont want
      // it to rotate.  Fix the rotation here.
      //double gx, gy, gth;
      //double nx, ny, nth;
      //GetGlobalPose(gx, gy, gth);
      //rtk_fig_get_origin(this->vision_fig, &nx, &ny, &nth);
      //rtk_fig_origin(this->vision_fig, nx, ny, -gth);

      // place the visualization a little away from the device
      stg_pose_t pose;
      pose.x = pose.y = pose.a = 0.0;
  
      model_local_to_global( mod, &pose );
  
      pose.x += 1.0;
      pose.y += 1.0;
      pose.a = 0.0;
      rtk_fig_origin( fig, pose.x, pose.y, pose.a );

      double scale = 0.007; // shrink from pixels to meters for display
  
      stg_blobfinder_config_t* cfg = (stg_blobfinder_config_t*) 
	model_get_prop_data_generic( mod, STG_PROP_BLOBCONFIG );
      assert(cfg);
  
      stg_property_t* prop = model_get_prop_generic( mod, STG_PROP_BLOBDATA );
  
      if( prop == NULL )
	return; // no data to render yet
  
      stg_blobfinder_blob_t* blobs = (stg_blobfinder_blob_t*)prop->data;
      if( blobs == NULL )
	return; // no data to render yet
  
      int num_blobs = prop->len / sizeof(stg_blobfinder_blob_t);
      if( num_blobs < 1 )
	return; // no data to render yet
  
      short width = cfg->scan_width;
      short height = cfg->scan_height;
      double mwidth = width * scale;
      double mheight = height * scale;
  
      // the view outline rectangle
      rtk_fig_color_rgb32(fig, 0xFFFFFF);
      rtk_fig_rectangle(fig, 0.0, 0.0, 0.0, mwidth,  mheight, 1 ); 
      rtk_fig_color_rgb32(fig, 0x000000);
      rtk_fig_rectangle(fig, 0.0, 0.0, 0.0, mwidth,  mheight, 0); 
  
      int c;
      for( c=0; c<num_blobs; c++)
	{
	  stg_blobfinder_blob_t* blob = &blobs[c];
      
	  // set the color from the blob data
	  rtk_fig_color_rgb32( fig, blob->color ); 
      
	  short top =   blob->top;
	  short bot =   blob->bottom;
	  short left =   blob->left;
	  short right =   blob->right;
      
	  double mtop = top * scale;
	  double mbot = bot * scale;
	  double mleft = left * scale;
	  double mright = right * scale;
      
	  // get the range in meters
	  //double range = (double)ntohs(data.blobs[index+b].range) / 1000.0; 
      
	  rtk_fig_rectangle(fig, 
			    -mwidth/2.0 + (mleft+mright)/2.0, 
			    -mheight/2.0 +  (mtop+mbot)/2.0,
			    0.0, 
			    mright-mleft, 
			    mbot-mtop, 
			    1 );
	}
    }
}



void gui_model_laser_data( model_t* mod )
{
  gui_window_t* win = g_hash_table_lookup( wins, &mod->world->id );  

  rtk_fig_t* fig = gui_model_figs(mod)->laser_data;  
  if( fig ) rtk_fig_clear( fig ); 
  
  // if we're drawing laser data and this model has a laser subscription
  // and some data, we'll draw the data
  if( win->show_laserdata && mod->subs[STG_PROP_LASERDATA] )
    {
      rtk_fig_color_rgb32(fig, stg_lookup_color(STG_LASER_COLOR) );
      rtk_fig_origin( fig, mod->local_pose.x, mod->local_pose.y, mod->local_pose.a );  
      stg_laser_config_t* cfg = (stg_laser_config_t*)
	model_get_prop_data_generic( mod, STG_PROP_LASERCONFIG );
      
      stg_geom_t* geom = (stg_geom_t*)
	model_get_prop_data_generic( mod, STG_PROP_LASERGEOM );
      
      stg_laser_sample_t* samples = (stg_laser_sample_t*)
	model_get_prop_data_generic( mod, STG_PROP_LASERDATA );
      
      double sample_incr = cfg->fov / cfg->samples;
      double bearing = geom->pose.a - cfg->fov/2.0;
      stg_laser_sample_t* sample = NULL;
      
      stg_point_t* points = calloc( sizeof(stg_point_t), cfg->samples + 1 );

      int s;
      for( s=0; s<cfg->samples; s++ )
	{
	  // useful debug
	  //rtk_fig_arrow( fig, 0, 0, bearing, (sample->range/1000.0), 0.01 );
	  
	  points[1+s].x = (samples[s].range/1000.0) * cos(bearing);
	  points[1+s].y = (samples[s].range/1000.0) * sin(bearing);
	  bearing += sample_incr;
	}

      // hmm, what's the right cast to get rid of the compiler warning
      // for the points argument?
      rtk_fig_polygon( fig, 0,0,0, cfg->samples+1, points, LASER_FILLED );      

      free( points );
    }
}


void gui_model_fiducial_data( model_t* mod )
{ 
  char text[32];

  gui_window_t* win = g_hash_table_lookup( wins, &mod->world->id );  
  
  rtk_fig_t* fig = gui_model_figs(mod)->fiducial_data;  
  if( fig ) rtk_fig_clear( fig ); 
  
  if( win->show_fiducialdata && mod->subs[STG_PROP_FIDUCIALDATA] )
    {      
      rtk_fig_color_rgb32( fig, stg_lookup_color( STG_FIDUCIAL_COLOR ) );
      
      stg_pose_t pose;
      model_global_pose( mod, &pose );  
      rtk_fig_origin( fig, pose.x, pose.y, pose.a ); 
      
      stg_property_t* prop = model_get_prop_generic( mod, STG_PROP_FIDUCIALDATA );
      
      if( prop )
	{
	  int bcount = prop->len / sizeof(stg_fiducial_t);
	  
	  stg_fiducial_t* fids = (stg_fiducial_t*)prop->data;
	  
	  int b;
	  for( b=0; b < bcount; b++ )
	    {
	      double pa = fids[b].bearing;
	      double px = fids[b].range * cos(pa); 
	      double py = fids[b].range * sin(pa);
	      
	      rtk_fig_line( fig, 0, 0, px, py );	
	      
	      // TODO: use the configuration info to determine beacon size
	      // for now we use these canned values
	      double wx = fids[b].geom.x;
	      double wy = fids[b].geom.y;
	      double wa = fids[b].geom.a;
	      
	      rtk_fig_rectangle(fig, px, py, wa, wx, wy, 0);
	      rtk_fig_arrow(fig, px, py, wa, wy, 0.10);
	      
	      if( fids[b].id > 0 )
		{
		  snprintf(text, sizeof(text), "  %d", fids[b].id);
		  rtk_fig_text(fig, px, py, pa, text);
		}
	    }  
	}
    }
}


void gui_model_lines( model_t* mod )
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



void gui_model_geom( model_t* mod )
{
  rtk_fig_t* fig = gui_model_figs(mod)->geom;
  gui_window_t* win = g_hash_table_lookup( wins, &mod->world->id );  

  rtk_fig_clear( fig );

  if( win->show_geom )
    {
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
}

// add a nose  indicating heading  
void gui_model_nose( model_t* mod )
{
  if( mod->nose )
    { 
      rtk_fig_t* fig = gui_model_figs(mod)->top;      
      rtk_fig_color_rgb32( fig, mod->color );
      
      // draw a line from the center to the front of the model
      rtk_fig_line( fig, mod->local_pose.x, mod->local_pose.y, mod->size.x/2, 0 );

      /*
	// this is the old-style nose-triangle. It doesn't work too
	// well with the new arbitrary shapes so I've taken it out.

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
      */
    }
}

void gui_model_movemask( model_t* mod )
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


void gui_model_update( model_t* mod, stg_prop_type_t prop )
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
    case STG_PROP_LASERCONFIG:
    case STG_PROP_LASERGEOM:
      gui_model_render( mod );
      gui_model_movemask( mod );
      break;
      
    case STG_PROP_POSE:
      gui_model_pose( mod );
      break;

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
      PRINT_WARN2( "property change %d(%s) unhandled by gui", 
		   prop, stg_property_string(prop) ); 
      break;
    } 
}
 
