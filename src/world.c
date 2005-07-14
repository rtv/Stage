
#define _GNU_SOURCE
#include <stdlib.h>
#include <assert.h>
#include <string.h> // for strdup(3)

//#define DEBUG

#define STG_TOKEN_MAX 64

const double STG_DEFAULT_RESOLUTION = 0.02;  // 2cm pixels
const double STG_DEFAULT_INTERVAL_REAL = 100.0; // msec between updates
const double STG_DEFAULT_INTERVAL_SIM = 100.0;  // duration of a simulation timestep in msec
const double STG_DEFAULT_WORLD_WIDTH = 20.0; // meters
const double STG_DEFAULT_WORLD_HEIGHT = 20.0; // meters

static int init_occurred = 0;

#include "stage_internal.h"

extern int _stg_quit; // quit flag is returned by stg_world_update()
extern int _stg_disable_gui;

/** @defgroup world The World

Stage simulates a 'world' composed of models, defined in a 'world
file'. 

<h2>Worldfile properties</h2>

@par Summary and default values

@verbatim
world
(
   name            "[filename of worldfile]"
   interval_real   100
   interval_sim    100
   resolution      0.01
)
@endverbatim

@par Details
- name [string]
  - the name of the world, as displayed in the window title bar. Defaults to the worldfile file name.
- interval_sim [milliseconds]
  - the length of each simulation update cycle in milliseconds.
- interval_real [milliseconds]
  - the amount of real-world (wall-clock) time the siulator will attempt to spend on each simulation cycle.
- resolution [meters]
  - specifies the resolution of the underlying bitmap model. Larger values speed up raytracing at the expense of fidelity in collision detection and sensing. 

@par More examples

The Stage source distribution contains several example world files in
<tt>(stage src)/worlds</tt> along with the worldfile properties
described on the manual page for each model type.

*/

/* UNDOCUMENTED - don't want to confuse people.
- resolution_med [meters]
  - resolution of the medium-level raytrace bitmap.
- resolution_med [meters]
  - resolution of the top-level raytrace bitmap.
*/


// create a world containing a passel of Stage models based on the
// worldfile

stg_world_t* stg_world_create_from_file( const char* worldfile_path )
{
  wf_load( (char*)worldfile_path );
  
  int section = 0;
  
  const char* world_name =
    wf_read_string( section, "name", (char*)worldfile_path );
  
  stg_msec_t interval_real = 
    wf_read_int( section, "interval_real", STG_DEFAULT_INTERVAL_REAL );

  stg_msec_t interval_sim = 
    wf_read_int( section, "interval_sim", STG_DEFAULT_INTERVAL_SIM );
      
  double ppm = 
    1.0 / wf_read_float( section, "resolution", STG_DEFAULT_RESOLUTION ); 
  
  double width = 
    wf_read_tuple_float( section, "size", 0, STG_DEFAULT_WORLD_WIDTH ); 

  double height = 
    wf_read_tuple_float( section, "size", 1, STG_DEFAULT_WORLD_HEIGHT ); 

  _stg_disable_gui = wf_read_int( section, "gui_disable", _stg_disable_gui );

  // create a single world
  stg_world_t* world = 
    stg_world_create( 0, 
		      world_name, 
		      interval_sim, 
		      interval_real,
		      ppm,
		      width,
		      height );

  if( world == NULL )
    return NULL; // failure
  
  // configure the GUI
  

  int section_count = wf_section_count();
  
  // Iterate through sections and create client-side models
  for( section = 1; section < section_count; section++ )
    {
      if( strcmp( wf_get_section_type(section), "window") == 0 )
	{
	  if( world->win )
	    gui_load( world->win, section ); 
	}
      else
	{
	  char *typestr = (char*)wf_get_section_type(section);      
	  
	  int parent_section = wf_get_parent_section( section );
	  
	  PRINT_DEBUG2( "section %d parent section %d\n", 
			section, parent_section );
	  
	  stg_model_t* parent = NULL;
	  
	  parent = (stg_model_t*)
	    g_hash_table_lookup( world->models, &parent_section );
	  
	  // select model type based on the worldfile token
	  stg_model_type_t type;
	  
	  if( strcmp( typestr, "model" ) == 0 ) // basic model
	    type = STG_MODEL_BASIC;
	  else if( strcmp( typestr, "test" ) == 0 ) // specialized models
	    type = STG_MODEL_TEST;
	  else if( strcmp( typestr, "laser" ) == 0 )
	    type = STG_MODEL_LASER;
	  else if( strcmp( typestr, "gripper" ) == 0 )
	    type = STG_MODEL_GRIPPER;
	  else if( strcmp( typestr, "ranger" ) == 0 )
	    type = STG_MODEL_RANGER;
	  else if( strcmp( typestr, "position" ) == 0 )
	    type = STG_MODEL_POSITION;
	  else if( strcmp( typestr, "blobfinder" ) == 0 )
	    type = STG_MODEL_BLOB;
	  else if( strcmp( typestr, "fiducialfinder" ) == 0 )
	    type = STG_MODEL_FIDUCIAL;
	  else if( strcmp( typestr, "energy" ) == 0 )
	    type = STG_MODEL_ENERGY;
	  else 
	    {
	      PRINT_ERR1( "unknown model type \"%s\". Model has not been created.",
			  typestr ); 
	      continue;
	    }
	  
	  //PRINT_WARN3( "creating model token %s type %d instance %d", 
	  ///       typestr, 
	  //       type,
	  //       parent ? parent->child_type_count[type] : world->child_type_count[type] );
	  
	  // generate a name and count this type in its parent (or world,
	  // if it's a top-level object)
	  char namebuf[STG_TOKEN_MAX];  
	  if( parent == NULL )
	    snprintf( namebuf, STG_TOKEN_MAX, "%s:%d", 
		      typestr, 
		      world->child_type_count[type]++);
	  else
	    snprintf( namebuf, STG_TOKEN_MAX, "%s.%s:%d", 
		      parent->token,
		      typestr, 
		      parent->child_type_count[type]++ );
	  
	  //PRINT_WARN1( "generated name %s", namebuf );
	  
	  // having done all that, allow the user to specify a name instead
	  char *namestr = (char*)wf_read_string(section, "name", namebuf );
	  
	  //PRINT_WARN2( "loading model name %s for type %s", namebuf, typestr );
	  
	  
	  PRINT_DEBUG2( "creating model from section %d parent section %d",
			section, parent_section );
	  
	  stg_model_t* mod = NULL;
	  stg_model_t* parent_mod = stg_world_get_model( world, parent_section );
	  
	  switch( type )
	    {
	    case STG_MODEL_BASIC:
	      mod = stg_model_create( world, parent_mod, section, STG_MODEL_BASIC, namestr );
	      break;
	      
	    case STG_MODEL_LASER:
	      mod = stg_laser_create( world,  parent_mod, section, namestr );
	      break;

	    case STG_MODEL_POSITION:
	      mod = stg_position_create( world,  parent_mod, section, namestr );
	      break;

	    case STG_MODEL_FIDUCIAL:
	      mod = stg_fiducial_create( world,  parent_mod, section, namestr );
	      break;
	      
	    case STG_MODEL_BLOB:
	      mod = stg_blobfinder_create( world, parent_mod, section, namestr );
	      break;	      	      
	    
	    case STG_MODEL_RANGER:
	      mod = stg_ranger_create( world,  parent_mod, section, namestr );
	      break;
	      
	      /*	      

	      //case STG_MODEL_GRIPPER:
	      //mod = stg_gripper_create( world,parent_mod,section,namestr);
	      //break;
	      
	      //case STG_MODEL_ENERGY:
	      //mod = stg_energy_create( world,  parent_mod, section, namestr );
	      break;

	      */
	    default:
	      PRINT_ERR1( "don't know how to configure type %d", type );
	    }

	  assert( mod );
	  
	  // configure the model with properties from the world file
	  stg_model_load( mod );
	  
	  // add the new model to the world
	  stg_world_add_model( world, mod );
	}
    }
  return world;
}


stg_world_t* stg_world_create( stg_id_t id, 
			       const char* token, 
			       int sim_interval, 
			       int real_interval,
			       double ppm,
			       double width,
			       double height )
{
  if( ! init_occurred )
    {
      //puts( "STG_INIT" );
      stg_init( 0, NULL );
      init_occurred = 1;
    }

  stg_world_t* world = calloc( sizeof(stg_world_t),1 );
  
  world->id = id;
  world->token = strdup( token );
  world->models = g_hash_table_new_full( g_int_hash, g_int_equal,
					 NULL, model_destroy_cb );
  world->models_by_name = g_hash_table_new_full( g_str_hash, g_str_equal,
						 NULL, NULL );  
  world->sim_time = 0.0;
  world->sim_interval = sim_interval;
  world->wall_interval = real_interval;
  world->wall_last_update = 0;
  
  world->width = width;
  world->height = height;
  world->ppm = ppm; // this is the finest resolution of the matrix
  world->matrix = stg_matrix_create( ppm, width, height ); 
  
  
  world->paused = TRUE; // start paused.
  
  world->destroy = FALSE;
  
  if( _stg_disable_gui )
    world->win = NULL;
  else    
    world->win = gui_world_create( world );
  
  return world;
}

void stg_world_stop( stg_world_t* world )
{
  world->paused = TRUE;
}

void stg_world_start( stg_world_t* world )
{
  world->paused = FALSE;
}

// calculate the bounding rectangle of everything in the world
void stg_world_dimensions( stg_world_t* world, 
			   double* min_x, double * min_y,
			   double* max_x, double * max_y )
{
  // TODO
  *min_x = *min_y =  MILLION;
  *max_x = *max_y = -MILLION;
}


void stg_world_destroy( stg_world_t* world )
{
  assert( world );
		   
  PRINT_DEBUG1( "destroying world %d", world->id );
  
  
  stg_matrix_destroy( world->matrix );

  free( world->token );
  g_hash_table_destroy( world->models );

  gui_world_destroy( world );

  free( world );
}

void world_destroy_cb( gpointer world )
{
  stg_world_destroy( (stg_world_t*)world );
}


int stg_world_update( stg_world_t* world, int sleepflag )
{
  //PRINT_WARN( "World update" );

#if 0 //DEBUG
  struct timeval tv1;
  gettimeofday( &tv1, NULL );
#endif
  
  if( world->win )
    {
      gui_poll();      
    }

#if 0// DEBUG
  struct timeval tv2;
  gettimeofday( &tv2, NULL );
  
  double guitime = (tv2.tv_sec + tv2.tv_usec / 1e6) - 
    (tv1.tv_sec + tv1.tv_usec / 1e6);
  
  printf( " guitime %.4f\n", guitime );
#endif

  //putchar( '.' );
  //fflush( stdout );
  
  stg_msec_t timenow = stg_timenow();
   
  //PRINT_DEBUG5( "timenow %lu last update %lu interval %lu diff %lu sim_time %lu", 
  //	timenow, world->wall_last_update, world->wall_interval,  
  //	timenow - world->wall_last_update, world->sim_time  );
  
  // if it's time for an update, update all the models
  stg_msec_t elapsed =  timenow - world->wall_last_update;

  if( world->wall_interval < elapsed )
    {
      stg_msec_t real_interval = timenow - world->wall_last_update;

#if 0      
      printf( " [%d %lu] sim:%lu real:%lu  ratio:%.2f\n",
	      world->id, 
	      world->sim_time,
	      world->sim_interval,
	      real_interval,
	      (double)world->sim_interval / (double)real_interval  );
      
      fflush(stdout);
#endif
      
      if( ! world->paused ) // only update if we're not paused
	{
	  world->real_interval_measured = real_interval;	  
	  g_hash_table_foreach( world->models, model_update_cb, world );	  	  
	  world->wall_last_update = timenow;	  
	  world->sim_time += world->sim_interval;
	}

      // redraw all the action
      if( world->win )
	if( gui_world_update( world ) )
	  stg_quit_request();      
    }
  else
    if( sleepflag )
      {
	//puts( "sleeping" );
	usleep( 10000 ); // sleep a little
      }

  return _stg_quit; // may have been set TRUE by the GUI or someone else
}

stg_model_t* stg_world_get_model( stg_world_t* world, stg_id_t mid )
{
  return( world ? g_hash_table_lookup( (gpointer)world->models, &mid ) : NULL );
}

void stg_world_add_model( stg_world_t* world, 
			  stg_model_t* mod  )
{
  g_hash_table_replace( world->models, &mod->id, mod );
  g_hash_table_replace( world->models_by_name, mod->token, mod );
}

int stg_world_model_destroy( stg_world_t* world, stg_id_t model )
{
  // delete the model
  g_hash_table_remove( world->models, &model );
  
  return 0; // ok
}


struct cb_package
{
  const char* propname;
  stg_property_callback_t callback;
  void* userdata;
};

void add_callback_wrapper( gpointer key, gpointer value, gpointer user )
{
  struct cb_package *pkg = (struct cb_package*)user;  
  stg_model_t* mod = (stg_model_t*)value;
  
  size_t dummy;
  if( stg_model_get_property( mod, pkg->propname, &dummy ) )
    stg_model_add_property_callback( mod,
				     pkg->propname,
				     pkg->callback,
				     pkg->userdata );
}			   

void remove_callback_wrapper( gpointer key, gpointer value, gpointer user )
{
  struct cb_package *pkg = (struct cb_package*)user;  
  stg_model_t* mod = (stg_model_t*)value;
  
  size_t dummy;
  if( stg_model_get_property( mod, pkg->propname, &dummy ) )
    stg_model_remove_property_callback( (stg_model_t*)value,
					pkg->propname,
					pkg->callback );
}			   
  
void stg_world_add_property_callback( stg_world_t* world,
				      char* propname,
				      stg_property_callback_t callback,
				      void* userdata )     
{  
  assert( world );
  assert( propname );
  assert( callback );

  struct cb_package pkg;
  pkg.propname = propname;
  pkg.callback = callback;
  pkg.userdata = userdata;

  g_hash_table_foreach( world->models, add_callback_wrapper, &pkg );
}

void stg_world_remove_property_callback( stg_world_t* world,
					 char* propname,
					 stg_property_callback_t callback )
{  
  assert( world );
  assert( propname );
  assert( callback );
  
  struct cb_package pkg;
  pkg.propname = propname;
  pkg.callback = callback;
  pkg.userdata = NULL;

  g_hash_table_foreach( world->models, remove_callback_wrapper, &pkg );
}


void stg_world_print( stg_world_t* world )
{
  printf( " world %d:%s (%d models)\n", 
	  world->id, 
	  world->token,
	  g_hash_table_size( world->models ) );
  
   g_hash_table_foreach( world->models, model_print_cb, NULL );
}

void world_print_cb( gpointer key, gpointer value, gpointer user )
{
  stg_world_print( (stg_world_t*)value );
}

stg_model_t* stg_world_model_name_lookup( stg_world_t* world, const char* name )
{
  return (stg_model_t*)g_hash_table_lookup( world->models_by_name, name );
}


void stg_model_save_cb( gpointer key, gpointer data, gpointer user )
{
  stg_model_save( (stg_model_t*)data );
}

void stg_model_reload_cb( gpointer key, gpointer data, gpointer user )
{
  stg_model_load( (stg_model_t*)data );
}

void stg_world_save( stg_world_t* world )
{
  // ask every model to save itself
  g_hash_table_foreach( world->models, stg_model_save_cb, NULL );
  
  if( world->win )
    gui_save( world->win );
  
  wf_save();
}

// reload the current worldfile
void stg_world_reload( stg_world_t* world )
{
  // can't reload the file yet - need to hack on the worldfile class. 
  //wf_load( NULL ); 

  // ask every model to save itself
  g_hash_table_foreach( world->models, stg_model_reload_cb, NULL );
}


/* void stg_world_add_property_toggles( stg_world_t* world,  */
/* 				     const char* propname,  */
/* 				     stg_property_callback_t callback_on, */
/* 				     void* arg_on, */
/* 				     stg_property_callback_t callback_off, */
/* 				     void* arg_off, */
/* 				     const char* label, */
/* 				     int enabled ) */
/* { */
/*   stg_world_property_callback_args_t* args =  */
/*     calloc(sizeof(stg_world_property_callback_args_t),1); */
  
/*   args->world = world; */
/*   strncpy(args->propname, propname, STG_PROPNAME_MAX ); */
/*   args->callback_on = callback_on; */
/*   args->callback_off = callback_off; */
/*   args->arg_on = arg_on; */
/*   args->arg_off = arg_off; */

/*   gui_add_view_item( propname, label, NULL,  */
/* 		     toggle_property_callback, enabled, args ); */
/* } */


