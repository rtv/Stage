
#define _GNU_SOURCE
#include <stdlib.h>
#include <assert.h>
#include <string.h> // for strdup(3)

//#define DEBUG 0


const double STG_DEFAULT_RESOLUTION = 0.02;  // 2cm pixels
const double STG_DEFAULT_INTERVAL_REAL = 100.0; // msec between updates
const double STG_DEFAULT_INTERVAL_SIM = 100.0;  // duration of a simulation timestep in msec
const double STG_DEFAULT_INTERVAL_GUI = 100.0; // msec between GUI updates
const double STG_DEFAULT_INTERVAL_MENU = 20.0; // msec between GUI updates
const double STG_DEFAULT_WORLD_WIDTH = 20.0; // meters
const double STG_DEFAULT_WORLD_HEIGHT = 20.0; // meters

static int init_occurred = 0;

#include "model.hh"

extern int _stg_quit; // quit flag is returned by stg_world_update()
extern int _stg_disable_gui;

#include <GL/gl.h>
extern int dl_debug;// debugging displaylist

extern GHashTable* global_typetable;

void gui_render_clock( stg_world_t* world );

// HACK
stg_world_t* global_world = NULL;

/** @addtogroup stage
    @{ */

/** @defgroup world Worlds

Stage simulates a 'world' composed of `models', defined in a `world
file'. 

<h2>Worldfile properties</h2>

@par Summary and default values

@verbatim
world
(
   name            "[filename of worldfile]"
   interval_real   100
   interval_sim    100
   gui_interval    100
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
- gui_interval [milliseconds]
  - the amount of real-world time between GUI updates
- resolution [meters]
  - specifies the resolution of the underlying bitmap model. Larger values speed up raytracing at the expense of fidelity in collision detection and sensing. 

@par More examples

The Stage source distribution contains several example world files in
<tt>(stage src)/worlds</tt> along with the worldfile properties
described on the manual page for each model type.

*/

/**@}*/

typedef	StgModel* (*stg_creator_t)(stg_world_t*, StgModel*, 
				   stg_id_t id, CWorldFile* );
 
void stg_world_clockstring( stg_world_t* world, char* str, size_t maxlen )
{
#if DEBUG
  snprintf( str, maxlen, "Ticks %lu Time: %lu:%lu:%02lu:%02lu.%03lu (sim:%3d real:%3d ratio:%2.2f)\tsubs: %d  %s",
	    world->updates,
	    world->sim_time_ms / (24*3600000), // days
	    world->sim_time_ms / 3600000, // hours
	    (world->sim_time_ms % 3600000) / 60000, // minutes
	    (world->sim_time_ms % 60000) / 1000, // seconds
	    world->sim_time_ms % 1000, // milliseconds
	    (int)world->sim_interval_ms,
	    (int)world->real_interval_measured,
    (double)world->sim_interval_ms / (double)world->real_interval_measured,
	    world->subs,
	    world->paused ? "--PAUSED--" : "" );
#else

  snprintf( str, maxlen, "Ticks %lu Time: %lu:%lu:%02lu:%02lu.%03lu\t(sim/real:%2.2f)\tsubs: %d  %s",
	    world->updates,
	    world->sim_time_ms / (24*3600000), // days
	    world->sim_time_ms / 3600000, // hours
	    (world->sim_time_ms % 3600000) / 60000, // minutes
	    (world->sim_time_ms % 60000) / 1000, // seconds
	    world->sim_time_ms % 1000, // milliseconds
	    (double)world->sim_interval_ms / (double)world->real_interval_measured,
	    world->subs,
	    world->paused ? "--PAUSED--" : "" );
#endif
}


void stg_world_set_interval_real( stg_world_t* world, unsigned int val )
{
  world->wall_interval = val;
}

void stg_world_set_interval_sim( stg_world_t* world, unsigned int val )
{
  world->sim_interval_ms = val;
}




// create a world containing a passel of Stage models based on the
// worldfile
stg_world_t* stg_world_create_from_file( stg_id_t id, const char* worldfile_path )
{
  printf( " [Loading %s]", worldfile_path );      
  fflush(stdout);

  CWorldFile* wf = new CWorldFile();
  wf->Load( worldfile_path );

  // end the output line of worldfile components
  puts("");

  int entity = 0;
  
  const char* world_name =
    wf->ReadString( entity, "name", (char*)worldfile_path );
  
  stg_msec_t interval_real = 
    wf->ReadInt( entity, "interval_real", STG_DEFAULT_INTERVAL_REAL );

  stg_msec_t interval_sim = 
    wf->ReadInt( entity, "interval_sim", STG_DEFAULT_INTERVAL_SIM );
      
  stg_msec_t interval_gui = 
    wf->ReadInt( entity, "gui_interval", STG_DEFAULT_INTERVAL_GUI );

  double ppm = 
    1.0 / wf->ReadFloat( entity, "resolution", STG_DEFAULT_RESOLUTION ); 
  
  double width = 
    wf->ReadTupleFloat( entity, "size", 0, STG_DEFAULT_WORLD_WIDTH ); 

  double height = 
    wf->ReadTupleFloat( entity, "size", 1, STG_DEFAULT_WORLD_HEIGHT ); 
  
  //stg_msec_t gui_menu_interval = 
  //wf_read_int( entity, "gui_menu_interval", STG_DEFAULT_INTERVAL_MENU );

  _stg_disable_gui = wf->ReadInt( entity, "gui_disable", _stg_disable_gui );

  // create a single world
  stg_world_t* world = 
    stg_world_create( id, 
		      world_name, 
		      interval_sim, 
		      interval_real,
		      interval_gui,
		      ppm,
		      width,
		      height );

  if( world == NULL )
    return NULL; // failure
  
  world->wf = wf;
    
  PRINT_DEBUG1( "wf has %d entitys", wf->GetEntityCount() );
  
  // Iterate through entitys and create client-side models
  for( entity = 1; entity < wf->GetEntityCount(); entity++ )
    {
      if( strcmp( wf->GetEntityType(entity), "window") == 0 )
	{
	  // configure the GUI
	  if( world->win )
	    gui_load( world, entity ); 
	}
      else
	{
	  char *typestr = (char*)wf->GetEntityType(entity);      	  
	  int parent_entity = wf->GetEntityParent( entity );
	  
	  //PRINT_DEBUG2( "wf entity %d parent entity %d\n", 
	  //	entity, parent_entity );

	  StgModel *mod, *parent;
	  
	  parent = (StgModel*)
	    g_hash_table_lookup( world->models, &parent_entity );

	  // find the creator function pointer in the hash table
	  stg_creator_t creator = (stg_creator_t) 
	    g_hash_table_lookup( global_typetable, typestr );
	  
	  // if we found a creator function, call it
	  if( creator )
	    {
	      //printf( "creator fn: %p\n", creator );
	      mod = (*creator)( world, parent, entity, wf );
	    }
	  else
	    {
	      PRINT_ERR1( "Unknown model type %s in world file.", 
			  typestr ); 
	      exit( 1 );
	    }
	  
	  // configure the model with properties from the world file
	  mod->Load();
	  
	  // add the new model to the world
	  stg_world_add_model( world, mod );

	  // increment the count of how many models of this type we've
	  // seen before
	  //int counter = (int)g_hash_table_lookup( world->child_type_count, typestr) + 1;
	  //g_hash_table_insert( world->child_type_count, typestr, (gpointer)counter );	  
	}
    }
  
  // warn about unused WF linesa
  wf->WarnUnused();
  
  global_world = world;
  
  return world;
}


stg_world_t* stg_world_create( stg_id_t id, 
			       const char* token, 
			       int sim_interval_ms, 
			       int real_interval,
			       int gui_interval,
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

  stg_world_t* world = (stg_world_t*)calloc( sizeof(stg_world_t),1 );
  
  world->id = id;
  world->token = strdup( token );
  world->wf = NULL;
  world->models = g_hash_table_new( g_int_hash, g_int_equal );
  world->models_by_name = g_hash_table_new( g_str_hash, g_str_equal );
  world->sim_time_ms = 0.0;
  world->sim_interval_ms = sim_interval_ms;
  world->wall_interval = real_interval;
  world->gui_interval = gui_interval;
  world->wall_last_update = 0;

  world->update_list = NULL;
  world->velocity_list = NULL;

  world->width = width;
  world->height = height;
  world->ppm = ppm; // this is the finest resolution of the matrix
  world->matrix = stg_matrix_create( ppm, width, height ); 
    
  world->paused = TRUE; // start paused.
  world->destroy = FALSE;
  
  world->child_type_count = g_hash_table_new( g_str_hash, g_str_equal );

  if( _stg_disable_gui )
    world->win = NULL;
  else    
    world->win = (gui_window_t*)gui_world_create( world );

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

void stg_world_set_title( stg_world_t* world, char* txt )
{
  gui_world_set_title( world, txt );
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

  delete world->wf;

  free( world->token );
  g_hash_table_destroy( world->models );

  //gui_world_destroy( world );

  free( world );
}

void world_destroy_cb( gpointer world )
{
  stg_world_destroy( (stg_world_t*)world );
}



int stg_world_update( stg_world_t* world, int sleepflag )
{
  //PRINT_WARN( "World update" );

  stg_msec_t timenow = stg_realtime_since_start();

#if DEBUG  
  double t = stg_realtime_since_start() / 1e3;

  static double calls = 0;
  calls++;  
  printf( "STG_WORLD_UPDATE: time %.2f calls: %.2f  %.2f/sec\n", t, calls, calls/t );
#endif

   // if it's time for an update, update all the models
  stg_msec_t elapsed =  timenow - world->wall_last_update;
  
  if( (!world->paused) && (world->wall_interval <= elapsed) )
    {
      world->real_interval_measured = timenow - world->wall_last_update;
      
      gui_render_clock( world );

      world->updates++;

#if DEBUG     
      printf( " [%d %lu %f] sim:%lu real:%lu  ratio:%.2f freq:%.2f \n",
	      world->id, 
	      world->sim_time_ms,
	      world->updates,
	      world->sim_interval_ms,
	      world->real_interval_measured,
	      (double)world->sim_interval_ms / (double)world->real_interval_measured,
	      world->updates/t );
      
      fflush(stdout);
#endif
      
      // update any models that are due to be updated
      for( GList* it=world->update_list; it; it=it->next )
	((StgModel*)it->data)->UpdateIfDue();
      
      // update any models with non-zero velocity
      for( GList* it=world->velocity_list; it; it=it->next )
	((StgModel*)it->data)->UpdatePose();
            
      world->wall_last_update = timenow;	  
      world->sim_time_ms += world->sim_interval_ms;
    
    }

  if( gui_world_update( world ) )
    stg_quit_request();
  

/*   // is it time to redraw all the action? */
/*   if( world->win && world->gui_interval <= (timenow - world->gui_last_update) ) */
/*     { */
      
      
/*       world->gui_last_update = timenow; */
/*     } */
    

  //else
  //if( sleepflag )
  //  {
  ////puts( "sleeping" );
  //usleep( 10000 ); // sleep a little
  //  }

  return _stg_quit; // may have been set TRUE by the GUI or someone else
}

StgModel* stg_world_get_model( stg_world_t* world, stg_id_t mid )
{
  return( world ? (StgModel*)g_hash_table_lookup( world->models, &mid ) : NULL );
}



void stg_world_add_model( stg_world_t* world, 
			  StgModel*  mod  )
{
  //printf( "World adding model %d %s to hash tables ", mod->id, mod->Token() );  
  g_hash_table_replace( world->models, &mod->id, mod );
  g_hash_table_replace( world->models_by_name, mod->Token(), mod );
  
  // if this is a top level model, it's a child of the world
  if( mod->Parent() == NULL )
    {
      //printf( "and child list", mod->id, mod->Token() );  
      world->children = g_list_append( world->children, mod );  
    }
  //puts("");
}


int stg_world_model_destroy( stg_world_t* world, stg_id_t model )
{
  // delete the model
  g_hash_table_remove( world->models, &model );
  
  return 0; // ok
}

// void stg_world_print( stg_world_t* world )
// {
//   printf( " world %d:%s (%d models)\n", 
// 	  world->id, 
// 	  world->token,
// 	  g_hash_table_size( world->models ) );
  
//    g_hash_table_foreach( world->models, model_print_cb, NULL );
// }

// void world_print_cb( gpointer key, gpointer value, gpointer user )
// {
//   stg_world_print( (stg_world_t*)value );
// }

StgModel* stg_world_model_name_lookup( stg_world_t* world, const char* name )
{
   //printf( "looking up model name %s in models_by_name\n", name );
   return (StgModel*)g_hash_table_lookup( world->models_by_name, name );
 }


void stg_model_save_cb( gpointer key, gpointer data, gpointer user )
{
  ((StgModel*)data)->Save();
}

void stg_model_reload_cb( gpointer key, gpointer data, gpointer user )
{
  ((StgModel*)data)->Load();
}

void stg_world_save( stg_world_t* world )
{
  // ask every model to save itself
  g_hash_table_foreach( world->models, stg_model_save_cb, NULL );
  
  if( world->win )
    gui_save( world );
  
  world->wf->Save(NULL);
}

// reload the current worldfile
void stg_world_reload( stg_world_t* world )
{
  // can't reload the file yet - need to hack on the worldfile class. 
  //wf->load( NULL ); 

  // ask every model to load itself from the file database
  g_hash_table_foreach( world->models, stg_model_reload_cb, NULL );
}

void stg_world_start_updating_model( stg_world_t* world, StgModel* mod )
{
  if( g_list_find( world->update_list, mod ) == NULL )
    world->update_list = 
      g_list_append( world->update_list, mod ); 
}

void stg_world_stop_updating_model( stg_world_t* world, StgModel* mod )
{
  world->update_list = g_list_remove( world->update_list, mod ); 
}
