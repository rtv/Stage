#ifndef _WORLD_H
#define _WORLD_H

#include "stage.h"
#include "server.h"
#include "matrix.h"

//struct stg_model_t;
//struct stg_server_t;

typedef struct _stg_world
 {
   stg_id_t id; // Stage's identifier for this world
   
   GHashTable* models; // the models that make up the world
   
   stg_matrix_t* matrix;

   char* token;

   stg_server_t* server;

   stg_time_t sim_time; // the current time in this world
   stg_time_t sim_interval; // this much simulated time elapses each step.
   
   double wall_interval; // real-time interval between updates -
			      // set this to zero for 'as fast as possible'
   stg_time_t wall_last_update; // the wall-clock time of the last update

   double ppm; // the resolution of the world model in pixels per meter

 } stg_world_t;



stg_world_t* world_create( stg_server_t* server, 
			   stg_id_t id, 
			   stg_createworld_t* cw );

void world_destroy( stg_world_t* world );
void world_destroy_cb( gpointer world );
void world_update( stg_world_t* world );
void world_update_cb( gpointer key, gpointer value, gpointer user );
void world_handle_msg( stg_world_t* world, int fd, stg_msg_t* msg );
void world_print( stg_world_t* world );
void world_print_cb( gpointer key, gpointer value, gpointer user );
struct _stg_model* world_get_model( stg_world_t* world, stg_id_t mid );

// get a property of the indicated type, returns NULL on failure
stg_property_t* stg_world_property_get(stg_world_t* world, stg_prop_type_t type);

// set a property of a world
void stg_world_property_set( stg_world_t* world, stg_property_t* prop );

#endif _WORLD_H
