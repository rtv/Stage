#ifndef _WORLD_H
#define _WORLD_H

#include "stage.h"
#include "server.h"
#include "matrix.h"
#include "connection.h"

typedef struct _world
 {
   stg_id_t id; // Stage's identifier for this world
   
   GHashTable* models; // the models that make up the world
   
   stg_matrix_t* matrix;

   char* token;

   server_t* server;

   stg_msec_t sim_time; // the current time in this world
   stg_msec_t sim_interval; // this much simulated time elapses each step.
   
   stg_msec_t wall_interval; // real-time interval between updates -
			      // set this to zero for 'as fast as possible'

   stg_msec_t wall_last_update; // the wall-clock time of the last update

   double ppm; // the resolution of the world model in pixels per meter

   int paused; // the world only updates when this is zero
   
   connection_t* con; // the connection that created this world

 } world_t;



world_t* world_create( server_t* server, 
		       connection_t* con,
			   stg_id_t id, 
			   stg_createworld_t* cw );

void world_destroy( world_t* world );
void world_destroy_cb( gpointer world );
void world_update( world_t* world );
void world_update_cb( gpointer key, gpointer value, gpointer user );
void world_handle_msg( world_t* world, int fd, stg_msg_t* msg );
void world_print( world_t* world );
void world_print_cb( gpointer key, gpointer value, gpointer user );
struct _model* world_get_model( world_t* world, stg_id_t mid );

// get a property of the indicated type, returns NULL on failure
stg_property_t* stg_world_property_get(world_t* world, stg_prop_type_t type);

// set a property of a world
void stg_world_property_set( world_t* world, stg_property_t* prop );

#endif // _WORLD_H
