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

  struct _model *parent;

  GPtrArray* children;

  // the number of subscriptions to each property
  int subs[STG_PROP_COUNT]; 
  
  // the time that each property was last calculated
  stg_msec_t update_times[STG_PROP_COUNT];
  
  gui_model_t gui; // all the gui stuff

  // todo - add this as a property?
  stg_joules_t energy_consumed;

  stg_msec_t interval; // time between updates in ms
  stg_msec_t interval_elapsed; // time since last update in ms
  
  void* data;
  size_t data_len;

  void* cmd;
  size_t cmd_len;
  
  void* cfg;
  size_t cfg_len;

  stg_bool_t stall;

  stg_laser_return_t laser_return;
  stg_bool_t obstacle_return;

  stg_pose_t pose;
  stg_velocity_t velocity;
  stg_geom_t geom;
  stg_color_t color;
  stg_kg_t mass;
 
  stg_line_t* lines; // this buffer is lines_count * sizeof(stg_line_t) big
  int lines_count; // the number of lines

  stg_guifeatures_t guifeatures;

  // these are a little strange
  stg_energy_config_t energy_config;
  stg_energy_data_t energy_data;


} model_t;  


typedef void(*func_init_t)(model_t*);
typedef int(*func_set_t)(model_t*,void*,size_t);
typedef void*(*func_get_t)(model_t*,size_t*);
typedef int(*func_update_t)(model_t*);
typedef int(*func_service_t)(model_t*);
typedef int(*func_startup_t)(model_t*);
typedef int(*func_shutdown_t)(model_t*);

typedef int(*func_putcommand_t)(model_t*,void*,size_t);
typedef int(*func_putdata_t)(model_t*,void*,size_t);
typedef int(*func_putconfig_t)(model_t*,void*,size_t);

typedef int(*func_getcommand_t)(model_t*,void**,size_t*);
typedef int(*func_getdata_t)(model_t*,void**,size_t*);
typedef int(*func_getconfig_t)(model_t*,void**,size_t*);

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
  func_putdata_t putdata;
  func_putcommand_t putcommand;
  func_getcommand_t getcommand;
  func_putconfig_t putconfig;
  func_getconfig_t getconfig;

  func_request_t request;
} lib_entry_t;


// MODEL
model_t* model_create(  world_t* world, model_t* parent, stg_id_t id, stg_model_type_t type, char* token );
void model_destroy( model_t* mod );
void model_destroy_cb( gpointer mod );
void model_handle_msg( model_t* model, int fd, stg_msg_t* msg );
void model_global_pose( model_t* mod, stg_pose_t* pose );
void model_subscribe( model_t* mod, stg_id_t pid );
void model_unsubscribe( model_t* mod, stg_id_t pid );

// this calls one of the set property functions, according to the propid value
int model_set_prop( model_t* mod, stg_id_t propid, void* data, size_t len );
int model_get_prop( model_t* mod, stg_id_t pid, void** data, size_t* len );

// SET properties - use these to set props, don't set them directly
int model_set_pose( model_t* mod, stg_pose_t* pose );
int model_set_velocity( model_t* mod, stg_velocity_t* vel );
int model_set_size( model_t* mod, stg_size_t* sz );
int model_set_color( model_t* mod, stg_color_t* col );
int model_set_geom( model_t* mod, stg_geom_t* geom );
int model_set_mass( model_t* mod, stg_kg_t* mass );
int model_set_guifeatures( model_t* mod, stg_guifeatures_t* gf );
int model_set_energy_config( model_t* mod, stg_energy_config_t* gf );
int model_set_energy_data( model_t* mod, stg_energy_data_t* gf );
int model_set_lines( model_t* mod, stg_line_t* lines, size_t lines_count );
int model_set_obstaclereturn( model_t* mod, stg_bool_t ret );
int model_set_laserreturn( model_t* mod, stg_laser_return_t val );

// GET properties - use these to get props - don't get them directly
stg_velocity_t*      model_get_velocity( model_t* mod );
stg_geom_t*          model_get_geom( model_t* mod );
stg_color_t          model_get_color( model_t* mod );
stg_pose_t*          model_get_pose( model_t* mod );
stg_kg_t*            model_get_mass( model_t* mod );
stg_line_t*          model_get_lines( model_t* mod, size_t* count );
stg_guifeatures_t*   model_get_guifeaturess( model_t* mod );
stg_energy_data_t*   model_get_energy_data( model_t* mod );
stg_energy_config_t* model_get_energy_config( model_t* mod );
stg_bool_t           model_get_obstaclereturn( model_t* mod );
stg_laser_return_t   model_get_laserreturn( model_t* mod );

// special
int model_set_command( model_t* mod, void* cmd, size_t len );
int model_set_data( model_t* mod, void* data, size_t len );
int model_set_config( model_t* mod, void* cmd, size_t len );

int model_get_command( model_t* mod, void** cmd, size_t* len );
int model_get_data( model_t* mod, void** data, size_t* len );
int model_get_config( model_t* mod, void** cmd, size_t* len );


int model_update( model_t* model );
void model_update_cb( gpointer key, gpointer value, gpointer user );
void model_update_velocity( model_t* model );
int model_update_pose( model_t* model );


void model_print( model_t* mod );
void model_print_cb( gpointer key, gpointer value, gpointer user );

void model_local_to_global( model_t* mod, stg_pose_t* pose );


void model_energy_consume( model_t* mod, stg_watts_t rate );


void model_lines_render( model_t* mod );

void register_init( stg_model_type_t type, func_init_t func );
void register_startup( stg_model_type_t type, func_startup_t func );
void register_shutdown( stg_model_type_t type, func_shutdown_t func );
void register_update( stg_model_type_t type, func_update_t func );
void register_service( stg_model_type_t type, func_service_t func );

void register_putdata( stg_model_type_t type, func_putdata_t func );
void register_putcommand( stg_model_type_t type, func_putcommand_t func );
void register_putconfig( stg_model_type_t type, func_putconfig_t func );

void register_getdata( stg_model_type_t type, func_getdata_t func );
void register_getcommand( stg_model_type_t type, func_getcommand_t func );
void register_getconfig( stg_model_type_t type, func_getconfig_t func );


void model_map( model_t* mod, gboolean render );
void model_map_with_children( model_t* mod, gboolean render );



rtk_fig_t* model_prop_fig_create( model_t* mod, 
				  rtk_fig_t* array[],
				  stg_id_t propid, 
				  rtk_fig_t* parent,
				  int layer );

int model_is_antecedent( model_t* mod, model_t* testmod );
int model_is_descendent( model_t* mod, model_t* testmod );

void model_render_lines( model_t* mod );
void model_render_geom( model_t* mod );
void model_render_pose( model_t* mod );
//void model_render_( model_t* mod );


#endif
