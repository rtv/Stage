#ifndef STG_MODEL_H
#define STG_MODEL_H

#include "world.h"

typedef struct _model
{
  stg_id_t id; // used as hash table key
  
  world_t* world;
  
  char* token;
    
  stg_pose_t pose;
  //stg_pose_t origin;

  stg_pose_t local_pose; // local offset from our pose

  stg_size_t size;
  stg_color_t color;
  stg_velocity_t velocity;
  
  GArray* lines; // array of point-pairs specifying lines in our body
  // GArray* arcs; // todo?

  // GUI features
  gboolean nose;
  gboolean grid;
  stg_movemask_t movemask;

  struct _model *parent;
  
  
  GArray* ranger_config; // sonars, IRs, etc.
  GArray* ranger_data;
  int ranger_return;
 
  stg_geom_t laser_geom;
  stg_laser_config_t laser_config;
  GArray* laser_data;
  int laser_return;

  gboolean subs[STG_PROP_COUNT]; // flags used to control updates
  
  stg_bool_t boundary;
  
  // store the time that each property was last calculated
  double update_times[STG_PROP_COUNT];
  
  // todo - random user properties
  //GHashTable* props;  
} model_t;  


// MODEL
model_t* model_create(  world_t* world, stg_id_t id, char* token );
void model_destroy( model_t* mod );
void model_destroy_cb( gpointer mod );
void model_handle_msg( model_t* model, int fd, stg_msg_t* msg );

int model_set_prop( model_t* mod, stg_id_t propid, void* data, size_t len );
int model_get_prop( model_t* model, stg_id_t propid, 
		    void** data, size_t* size );

int model_update_prop( model_t* mod, stg_id_t propid );
void model_update_rangers( model_t* mod );
void model_update_laser( model_t* mod );

void model_subscribe( model_t* mod, stg_id_t pid );
void model_unsubscribe( model_t* mod, stg_id_t pid );

void model_set_pose( model_t* mod, stg_pose_t* pose );
void model_set_velocity( model_t* mod, stg_velocity_t* vel );
void model_set_size( model_t* mod, stg_size_t* sz );
void model_set_color( model_t* mod, stg_color_t* col );

void model_update( model_t* model );
void model_update_cb( gpointer key, gpointer value, gpointer user );
void model_update_velocity( model_t* model );
void model_update_pose( model_t* model );

void model_print( model_t* mod );
void model_print_cb( gpointer key, gpointer value, gpointer user );

void model_local_to_global( model_t* mod, stg_pose_t* pose );

// ARE ANY OF THESE USED?

// apparently not! delete em!

// returns a pointer to the property + payload
//stg_property_t* stg_model_property( model_t* model, stg_prop_type_t prop );

// returns the payload of the property without the property header stuff.
//void* stg_model_property_data( model_t* model, stg_prop_type_t prop );

// loops on stg_client_read() until data is available for the
// indicated property
//void stg_model_property_wait( model_t* model, stg_prop_type_t datatype );

// requests a model property from the server, to be sent
// asynchonously. Returns immediately after sending the request. To
// get the data, either (i) call stg_client_read() repeatedly until
// the data shows up. To guarantee getting new data this way, either
// delete the local cached version before making the request, or
// inspect the property timestamps; or (ii) call stg_client_msg_read()
// until you see the data you want.
//void stg_model_property_req( model_t* model,  stg_prop_type_t type );

// ask the server for a property, then attempt to read it from the
// incoming socket. Assumes there is no asyncronous traffic on the
// socket (so don't use this after a starting a subscription, for
// example).
//stg_property_t* stg_model_property_get_sync( model_t* model, 
//				     stg_prop_type_t type );

// set a property of the model, composing a stg_property_t from the
// arguments. read and return an immediate reply, assuming there is no
// asyncronhous data on the same channel. returns the reply, possibly
// NULL.
//stg_property_t*
//stg_model_property_set_ex_sync( model_t* model,
//			stg_time_t timestamp,
//			stg_prop_type_t type, // the property 
//			void* data, // the new contents
//			size_t len ); // the size of the new contents

// Set a property of a model
//int stg_model_property_set( model_t* model, 
//		    stg_property_t* prop );

//int stg_model_property_set_from_hdr( model_t* model, 
//			     stg_property_hdr_t* hdr );
   
//int stg_model_property_set_from_data( model_t* model, 
//			      stg_prop_type_t type, 
//			      void* data, 
//			      size_t len ); 
   
// Functions for accessing a model's cache of properties.  it's good
// idea to use these interfaces to get a model's data rather than
// accessng the model_t structure directly, as the structure may
// change in future.

// frees and zeros the model's property
//void stg_model_property_delete( model_t* model, stg_prop_type_t prop);

// returns a pointer to the property + payload
//stg_property_t* stg_model_property( model_t* model, stg_prop_type_t prop);

// returns the payload of the property without the property header stuff.
//void* stg_model_property_data( model_t* model, stg_prop_type_t prop );

// deletes the indicated property, requests a new copy, and read until
// it arrives
//void stg_model_property_refresh( model_t* model, stg_prop_type_t prop );

// print a human-readable description of the tree on stdout
//void stg_model_print_tree( int depth, model_t* m );

#endif
