#ifndef _WORLD_H
#define _WORLD_H

#ifdef __cplusplus
 extern "C" {
#endif 

#include "stage.h"
#include "matrix.h"

   //#include "connection.h"
   //#include "server.h"

struct _gui_window;

/// defines a simulated world
typedef struct _world
 {
   stg_id_t id; // Stage's identifier for this world
   
   GHashTable* models; // the models that make up the world, indexed by id
   GHashTable* models_by_name; // the models that make up the world, indexed by name
  
   // the number of models of each type is counted so we can
   // automatically generate names for them
   int child_type_count[ STG_MODEL_COUNT ];
   
   stg_matrix_t* matrix;

   char* token;


   stg_msec_t sim_time; // the current time in this world
   stg_msec_t sim_interval; // this much simulated time elapses each step.
   
   stg_msec_t wall_interval; // real-time interval between updates -
			      // set this to zero for 'as fast as possible'

   stg_msec_t wall_last_update; // the wall-clock time of the last update

   double ppm; // the resolution of the world model in pixels per meter

   int paused; // the world only updates when this is zero
   
   gboolean destroy;

   lib_entry_t* library;

   struct _gui_window* win; // the gui window associated with this world
   
   ///  a hooks for the user to store things in the world
   void* user;
   size_t user_len;

 } world_t;



   //world_t* world_create( server_t* server, 
   //	       connection_t* con,
   //	       stg_id_t id, 
   //	       stg_createworld_t* cw );
   
world_t* stg_world_create( stg_id_t id, 
			   char* token, 
			   int sim_interval, 
			   int real_interval,
			   double ppm );
   
   world_t* world_create_from_file( char* worldfile_path );


void world_destroy( world_t* world );
int world_update( world_t* world );
   //void world_handle_msg( world_t* world, int fd, stg_msg_t* msg );
void world_print( world_t* world );


/// create a new model  
struct _model*  world_model_create( world_t* world, 
				    stg_id_t id, 
				    stg_id_t parent_id, 
				    stg_model_type_t type, 
				    char* token );

/// get a model pointer from its ID
struct _model* world_get_model( world_t* world, stg_id_t mid );

/// get a model pointer from its name
struct _model* world_model_name_lookup( world_t* world, const char* name );

#ifdef __cplusplus
  }
#endif 

//void world_print_cb( gpointer key, gpointer value, gpointer user );
//struct _model* world_model_create( world_t* world, stg_createmodel_t* cm );
// get a property of the indicated type, returns NULL on failure
//stg_property_t* stg_world_property_get(world_t* world, stg_prop_type_t type);

// set a property of a world
//void stg_world_property_set( world_t* world, stg_property_t* prop );

//void world_update_cb( gpointer key, gpointer value, gpointer user );
//void world_destroy_cb( gpointer world );


#endif // _WORLD_H
