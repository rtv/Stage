

#include <stdlib.h>
#include <assert.h>

//#define DEBUG

#include "stage.h"
#include "gui.h"

/* world_t* world_create( server_t* server, connection_t* con,  */
/* 		       stg_id_t id, stg_createworld_t* cw ) */
/* { */
/*   PRINT_DEBUG3( "world creator %d (%s) on con %p", id, cw->token, con ); */
  
/*   world_t* world = calloc( sizeof(world_t),1 ); */
  
/*   // this is a little wierd, but we have to be compatible with the other constructor */
/*   world->library = server->library; */
  
/*   world->con = con; */
/*   world->id = id; */
/*   world->token = strdup( cw->token ); */
/*   world->models = g_hash_table_new_full( g_int_hash, g_int_equal, */
/* 					 NULL, model_destroy_cb ); */
/*   world->models_by_name = g_hash_table_new_full( g_str_hash, g_str_equal, */
/* 						 NULL, NULL ); */
  
/*   world->server = server; // stash the server pointer */
  
/*   world->sim_time = 0.0; */
/*   //world->sim_interval = cw->interval_sim;//STG_DEFAULT_WORLD_INTERVAL; */
/*   world->sim_interval = STG_DEFAULT_WORLD_INTERVAL_MS; */
/*   world->wall_interval = cw->interval_real;   */
/*   world->wall_last_update = 0;//stg_timenow(); */
/*   world->ppm = cw->ppm; */
  
/*   // todo - have the matrix resolutions fully configurable at startup */
/*   world->matrix = stg_matrix_create( world->ppm, 5, 1 );  */

/*   world->paused = TRUE; // start paused. */

/*   world->destroy = FALSE; */
  
/*   world->win = gui_world_create( world ); */

/*   return world; */
/* } */

world_t* stg_world_create( stg_id_t id, 
			   char* token, 
			   int sim_interval, 
			   int real_interval,
			   double ppm )
{
  PRINT_DEBUG2( "alternate world creator %d (%s)", id, token );
  
  world_t* world = calloc( sizeof(world_t),1 );
  
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



void world_destroy( world_t* world )
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
  world_destroy( (world_t*)world );
}


int world_update( world_t* world )
{
  //PRINT_WARN( "World update" );

#if 0 //DEBUG
  struct timeval tv1;
  gettimeofday( &tv1, NULL );
#endif
  
  gui_world_update( world );
  
#if 0// DEBUG
  struct timeval tv2;
  gettimeofday( &tv2, NULL );
  
  double guitime = (tv2.tv_sec + tv2.tv_usec / 1e6) - 
    (tv1.tv_sec + tv1.tv_usec / 1e6);
  
  printf( " guitime %.4f\n", guitime );
#endif

  if( world->paused ) // only update if we're not paused
    return 0;

  
  //PRINT_WARN( "World update - not paused" );
 

  //{
  stg_msec_t timenow = stg_timenow();
  
 
  //PRINT_DEBUG5( "timenow %lu last update %lu interval %lu diff %lu sim_time %lu", 
  //	timenow, world->wall_last_update, world->wall_interval,  
  //	timenow - world->wall_last_update, world->sim_time  );
  
  // if it's time for an update, update all the models
  if( timenow - world->wall_last_update > world->wall_interval )
    {
      stg_msec_t real_interval = timenow - world->wall_last_update;

      printf( " [%d %lu] sim:%lu real:%lu  ratio:%.2f\r",
	      world->id, 
	      world->sim_time,
	      world->sim_interval,
	      real_interval,
	      (double)world->sim_interval / (double)real_interval  );
      
      fflush(stdout);

      //fflush( stdout );
      
      g_hash_table_foreach( world->models, model_update_cb, world );
      
      
      world->wall_last_update = timenow;
      
      world->sim_time += world->sim_interval;
      
    }
  //}
 
  return 0;
}

void world_update_cb( gpointer key, gpointer value, gpointer user )
{
  world_update( (world_t*)value );
}

model_t* world_get_model( world_t* world, stg_id_t mid )
{
  return( world ? g_hash_table_lookup( (gpointer)world->models, &mid ) : NULL );
}


// add a model entry to the server & install its default properties
//model_t* world_model_create( world_t* world, stg_createmodel_t* cm )

model_t* world_model_create( world_t* world, 
			     stg_id_t id, 
			     stg_id_t parent_id, 
			     stg_model_type_t type, 
			     char* token )
{
  model_t* parent = g_hash_table_lookup( world->models, &parent_id );
  
  if( parent_id && !parent )
    PRINT_WARN1( "model create requested with parent id %d, but parent not found", 
		 parent_id );
  
  PRINT_DEBUG4( "creating model %d:%d (%s) parent %d", world->id, id, token, parent_id  );
  

  model_t* mod = model_create( world, parent, id, type, token ); 
  
  g_hash_table_replace( world->models, &mod->id, mod );
  g_hash_table_replace( world->models_by_name, mod->token, mod );

  return mod; // the new model
}

int world_model_destroy( world_t* world, stg_id_t model )
{
  // delete the model
  g_hash_table_remove( world->models, &model );
  
  return 0; // ok
}


void world_print( world_t* world )
{
  printf( " world %d:%s (%d models)\n", 
	  world->id, 
	  world->token,
	  g_hash_table_size( world->models ) );
  
   g_hash_table_foreach( world->models, model_print_cb, NULL );
}

void world_print_cb( gpointer key, gpointer value, gpointer user )
{
  world_print( (world_t*)value );
}

model_t* world_model_name_lookup( world_t* world, const char* name )
{
  return (model_t*)g_hash_table_lookup( world->models_by_name, name );
}
