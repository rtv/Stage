
#include <stdlib.h>
#include <assert.h>

//#define DEBUG

#include "stage.h"

extern int _stg_quit; // quit flag is returned by stg_world_update()

stg_world_t* stg_world_create( stg_id_t id, 
			   char* token, 
			   int sim_interval, 
			   int real_interval,
			   double ppm )
{
  PRINT_DEBUG2( "alternate world creator %d (%s)", id, token );
  
  stg_world_t* world = calloc( sizeof(stg_world_t),1 );
  
  world->library = stg_library_create();
  assert(world->library);
  //world->con = NULL;
  world->id = id;
  world->token = strdup( token );
  world->models = g_hash_table_new_full( g_int_hash, g_int_equal,
					 NULL, model_destroy_cb );
  world->models_by_name = g_hash_table_new_full( g_str_hash, g_str_equal,
						 NULL, NULL );
  
  //world->server = NULL; // stash the server pointer
  
  world->sim_time = 0.0;
  world->sim_interval = sim_interval;
  world->wall_interval = real_interval;
  world->wall_last_update = 0;
  world->ppm = ppm;
  
  // todo - have the matrix resolutions fully configurable at startup
  world->matrix = stg_matrix_create( world->ppm, 5, 1 ); 
  
  world->paused = TRUE; // start paused.
  
  world->destroy = FALSE;
  
  world->win = gui_world_create( world );

  return world;
}

/// calculate the bounding rectangle of everything in the world
void stg_world_dimensions( stg_world_t* world, 
			   double* min_x, double * min_y,
			   double* max_x, double * max_y )
{
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

  //PRINT_WARN( "World update - not paused" );
 


#if 0 //DEBUG
  struct timeval tv1;
  gettimeofday( &tv1, NULL );
#endif
  
  if( world->win )
    {
      gui_poll();
      
      // if the window was closed, we request a quit
      if( gui_world_update( world ) )
	stg_quit_request();
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

  if( world->paused ) // only update if we're not paused
    return 0;

  
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
      
      g_hash_table_foreach( world->models, model_update_cb, world );
      
      
      world->wall_last_update = timenow;
      
      world->sim_time += world->sim_interval;
      
    }
  else
    if( sleepflag ) usleep( 10000 ); // sleep a little
  
  return _stg_quit; // may have been set TRUE by the GUI or someone else
}

//void world_update_cb( gpointer key, gpointer value, gpointer user )
//{
// stg_world_update( (stg_world_t*)value );
//}

stg_model_t* stg_world_get_model( stg_world_t* world, stg_id_t mid )
{
  return( world ? g_hash_table_lookup( (gpointer)world->models, &mid ) : NULL );
}


// add a model entry to the server & install its default properties
//stg_model_t* world_model_create( stg_world_t* world, stg_createstg_model_t* cm )

stg_model_t* stg_world_model_create( stg_world_t* world, 
			     stg_id_t id, 
			     stg_id_t parent_id, 
			     stg_model_type_t type, 
			     char* token )
{
  stg_model_t* parent = g_hash_table_lookup( world->models, &parent_id );
  
  if( parent_id && !parent )
    PRINT_WARN1( "model create requested with parent id %d, but parent not found", 
		 parent_id );
  
  PRINT_DEBUG4( "creating model %d:%d (%s) parent %d", world->id, id, token, parent_id  );
  

  stg_model_t* mod = stg_model_create( world, parent, id, type, token ); 
  
  g_hash_table_replace( world->models, &mod->id, mod );
  g_hash_table_replace( world->models_by_name, mod->token, mod );

  return mod; // the new model
}

int stg_world_model_destroy( stg_world_t* world, stg_id_t model )
{
  // delete the model
  g_hash_table_remove( world->models, &model );
  
  return 0; // ok
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
