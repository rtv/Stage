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

  int type; // what kind of a model am I?


  // todo
  struct _model *parent;
  
  // the number of subscriptions to each property
  int subs[STG_PROP_COUNT]; 
  
  // the time that each property was last calculated
  stg_msec_t update_times[STG_PROP_COUNT];
  
  GHashTable* props; // table of stg_property_t's indexed by property id
  
  gui_model_t gui; // all the gui stuff - this should be a property too!

  // todo - add this as a property?
  stg_joules_t energy_consumed;

  stg_msec_t interval; // time between updates in ms
  stg_msec_t interval_elapsed; // time since last update in ms
  
  void* data;
  size_t data_len;

  void* cmd;
  size_t cmd_len;

} model_t;  


typedef void(*func_init_t)(model_t*);
typedef int(*func_set_t)(model_t*,void*,size_t);
typedef void*(*func_get_t)(model_t*,size_t*);
typedef int(*func_update_t)(model_t*);
typedef int(*func_service_t)(model_t*);
typedef int(*func_startup_t)(model_t*);
typedef int(*func_shutdown_t)(model_t*);

typedef int(*func_putcommand_t)(model_t*,void*,size_t);
typedef int(*func_getdata_t)(model_t*,void**,size_t*);
typedef int(*func_request_t)(model_t*);

typedef struct
{
  func_init_t init;
  func_startup_t startup;
  func_shutdown_t shutdown;
  func_update_t update;
  func_service_t service;
  func_get_t get;
  func_set_t set;
  func_getdata_t getdata;
  func_putcommand_t putcommand;
  func_request_t request;
} lib_entry_t;



// MODEL
model_t* model_create(  world_t* world, model_t* parent, stg_id_t id, stg_model_type_t type, char* token );
void model_destroy( model_t* mod );
void model_destroy_cb( gpointer mod );
void model_handle_msg( model_t* model, int fd, stg_msg_t* msg );

int model_set_prop( model_t* mod, stg_id_t propid, void* data, size_t len );
int model_get_prop( model_t* model, stg_id_t propid, 
		    void** data, size_t* size );

int model_getdata( model_t* mod, void** data, size_t* len );
int model_putcommand( model_t* mod, void* cmd, size_t len );

void model_global_pose( model_t* mod, stg_pose_t* pose );

int model_set_prop_generic( model_t* mod, stg_id_t propid, void* data, size_t len );
stg_property_t* model_get_prop_generic( model_t* mod, stg_id_t propid );
int model_remove_prop_generic( model_t* mod, stg_id_t propid );
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

int model_update( model_t* model );
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

stg_velocity_t* model_velocity_get( model_t* mod );
stg_geom_t* model_geom_get( model_t* mod );
stg_color_t model_color_get( model_t* mod );
stg_pose_t* model_pose_get( model_t* mod );
stg_kg_t* model_mass_get( model_t* mod );
stg_line_t* model_lines_get( model_t* mod, size_t* count );

stg_energy_data_t* model_energy_get( model_t* mod );
stg_energy_config_t* model_energy_config_get( model_t* mod );

void model_lines_render( model_t* mod );

void model_register_init( stg_id_t pid, func_init_t func );
void model_register_startup( stg_id_t pid, func_startup_t func );
void model_register_shutdown( stg_id_t pid, func_shutdown_t func );
void model_register_update( stg_id_t pid, func_update_t func );
void model_register_service( stg_id_t pid, func_service_t func );
void model_register_set( stg_id_t pid, func_set_t func );
void model_register_get( stg_id_t pid, func_get_t func );

void register_init( stg_model_type_t type, func_init_t func );
void register_startup( stg_model_type_t type, func_startup_t func );
void register_shutdown( stg_model_type_t type, func_shutdown_t func );
void register_update( stg_model_type_t type, func_update_t func );
void register_service( stg_model_type_t type, func_service_t func );
void register_set( stg_model_type_t type, func_set_t func );
void register_get( stg_model_type_t type, func_get_t func );

/*int model_register_type( stg_model_type_t type, 
			 func_init_t initf,
			 func_startup_t startupf,
			 func_shutdown_t shutdownf,
			 func_service_t servicef,
			 func_putcommand_t putcommandf,
			 func_getdata_t getdataf,
			 func_request_t requestf );
*/



stg_bool_t model_obstacle_get( model_t* model );

void model_map( model_t* mod, gboolean render );



rtk_fig_t* model_prop_fig_create( model_t* mod, 
				  rtk_fig_t* array[],
				  stg_id_t propid, 
				  rtk_fig_t* parent,
				  int layer );


#endif
