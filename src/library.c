
#include "model.h"

// declare your registration function here

// new style registration funcs for derived models
int register_test(void);
int register_laser(void);
int register_position(void);

lib_entry_t derived[STG_MODEL_COUNT];

void library_create( void )
{
  memset( derived, 0, sizeof(derived[0]) * STG_MODEL_COUNT );
  
  // call your registration function here
  register_test();
  register_laser();
  register_position();
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

void register_getdata( stg_model_type_t type, func_getdata_t func )
{
  derived[type].getdata = func;
}

void register_putdata( stg_model_type_t type, func_putdata_t func )
{
  derived[type].putdata = func;
}


void register_getcommand( stg_model_type_t type, func_getcommand_t func )
{
  derived[type].getcommand = func;
}

void register_putcommand( stg_model_type_t type, func_putcommand_t func )
{
  derived[type].putcommand = func;
}


void register_putconfig( stg_model_type_t type, func_putconfig_t func )
{
  derived[type].putconfig = func;
}

void register_getconfig( stg_model_type_t type, func_getconfig_t func )
{
  derived[type].getconfig = func;
}

