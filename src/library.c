
#include "model.h"

// declare your registration function here

// new style registration funcs for derived models
int register_test(void);
int register_laser(void);
int register_position(void);
int register_fiducial(void);
int register_ranger(void);
int register_blobfinder(void);

lib_entry_t derived[STG_MODEL_COUNT];

void library_create( void )
{
  memset( derived, 0, sizeof(derived[0]) * STG_MODEL_COUNT );
  
  // call your registration function here
  register_test();
  register_laser();
  register_position();
  register_fiducial();
  register_ranger();
  register_blobfinder();
}

// registration funcs
void register_init( stg_model_type_t type, func_init_t func )
{
  derived[type].init = func;
}

void register_startup( stg_model_type_t type, func_startup_t func )
{
  derived[type].startup = func;
}

void register_shutdown( stg_model_type_t type, func_shutdown_t func )
{
  derived[type].shutdown = func;
}

void register_update( stg_model_type_t type, func_update_t func )
{
  derived[type].update = func;
}

void register_get_data( stg_model_type_t type, func_get_data_t func )
{
  derived[type].get_data = func;
}

void register_set_data( stg_model_type_t type, func_set_data_t func )
{
  derived[type].set_data = func;
}


void register_get_command( stg_model_type_t type, func_get_command_t func )
{
  derived[type].get_command = func;
}

void register_set_command( stg_model_type_t type, func_set_command_t func )
{
  derived[type].set_command = func;
}


void register_set_config( stg_model_type_t type, func_set_config_t func )
{
  derived[type].set_config = func;
}

void register_get_config( stg_model_type_t type, func_get_config_t func )
{
  derived[type].get_config = func;
}

