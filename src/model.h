#ifndef STG_MODEL_H
#define STG_MODEL_H

#include "world.h"
#include "rtk.h"

typedef struct
{
  rtk_fig_t* top;
  rtk_fig_t* geom;
  rtk_fig_t* grid;
  
  // a hash table would use less space , but this is fast and easy
  rtk_fig_t* propdata[ STG_PROP_COUNT ];
  rtk_fig_t* propgeom[ STG_PROP_COUNT ];
  
} gui_model_t;

typedef struct _model
{
  stg_id_t id; // used as hash table key
  
  world_t* world;
  
  char* token;
    
  stg_pose_t pose;
  stg_geom_t geom; // origin and size in one
  stg_color_t color;
  stg_velocity_t velocity;
  
  GArray* lines; // array of point-pairs specifying lines in our body
  // GArray* arcs; // TODO?

  // GUI features
  gboolean nose;
  gboolean grid;
  stg_movemask_t movemask;

  stg_bool_t boundary;

  struct _model *parent;
  
  int ranger_return;
  int laser_return;
  int fiducial_return;
  int obstacle_return;
  
  stg_kg_t mass;

  stg_joules_t energy_consumed;

  //stg_watts_t watts; // energy consumption this timestep. Devices may
  //	     // add their energy consumption in Watts to this
  ///	     // and it will be reported by the energy model

  gboolean subs[STG_PROP_COUNT]; // flags used to control updates
    
  // store the time that each property was last calculated
  stg_msec_t update_times[STG_PROP_COUNT];
  
  GHashTable* props; // table of stg_property_t's indexed by property id
  
  gui_model_t gui; // all the gui stuff
  
} model_t;  

typedef void(*init_func_t)(model_t*);

typedef struct
{
  stg_id_t id;
  const char* name;
  init_func_t init;
  init_func_t startup;
  init_func_t shutdown;
  init_func_t update;
  init_func_t render;
} libitem_t;


// MODEL
model_t* model_create(  world_t* world, stg_id_t id, char* token );
void model_destroy( model_t* mod );
void model_destroy_cb( gpointer mod );
void model_handle_msg( model_t* model, int fd, stg_msg_t* msg );

int model_set_prop( model_t* mod, stg_id_t propid, void* data, size_t len );
int model_get_prop( model_t* model, stg_id_t propid, 
		    void** data, size_t* size );

void model_set_prop_generic( model_t* mod, stg_id_t propid, void* data, size_t len );
stg_property_t* model_get_prop_generic( model_t* mod, stg_id_t propid );
void model_remove_prop_generic( model_t* mod, stg_id_t propid );
void* model_get_prop_data_generic( model_t* mod, stg_id_t propid );

int model_update_prop( model_t* mod, stg_id_t propid );
void model_ranger_update( model_t* mod );
void model_laser_update( model_t* mod );

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

void model_fiducial_init( model_t* mod );
void model_fiducial_update( model_t* mod );

void model_energy_consume( model_t* mod, stg_watts_t rate );

#endif
