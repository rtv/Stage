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
  
  stg_energy_t energy;

  GArray* ranger_config; // sonars, IRs, etc.
  GArray* ranger_data;
  int ranger_return;
 
  stg_geom_t laser_geom;
  stg_laser_config_t laser_config;
  GArray* laser_data;
  int laser_return;

  gboolean subs[STG_PROP_COUNT]; // flags used to control updates
  
  stg_blobfinder_config_t blob_cfg;
  GArray* blobs;
  
  stg_bool_t boundary;
  
  // store the time that each property was last calculated
  stg_msec_t update_times[STG_PROP_COUNT];
  
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

void model_blobfinder_init( model_t* mod );
void model_blobfinder_update( model_t* mod );

#endif
