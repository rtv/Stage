#include <assert.h>
#include <math.h>

#define DEBUG
//#undef DEBUG

#include "model.h"

extern lib_entry_t derived[];

///////////////////////////////////////////////////////////////////////////



int model_getdata( model_t* mod, void** data, size_t* len )
{
  // if this type of model has a getdata function, call it.
  if( derived[ mod->type ].getdata )
    {
      derived[ mod->type ].getdata(mod, data, len);
      PRINT_WARN1( "used special getdata, returned %d bytes", (int)*len );
    }
  else
    { // we do a generic data copy
      *data = mod->data;
      *len = mod->data_len; 
      PRINT_WARN3( "model %d(%s) generic getdata, returned %d bytes", 
		   mod->id, mod->token, (int)*len );
    }
  return 0; //ok
}

int model_putdata( model_t* mod, void* data, size_t len )
{
  // if this type of model has a putdata function, call it.
  if( derived[ mod->type ].putdata )
    {
      derived[ mod->type ].putdata(mod, data, len);
      PRINT_WARN1( "used special putdata returned %d bytes", (int)len );
    }
  else
    { // we do a generic data copy
      stg_copybuf( &mod->data, &mod->data_len, data, len );
      PRINT_WARN3( "model %d(%s) put data of %d bytes",
		   mod->id, mod->token, (int)mod->data_len);
    }

  return 0; //ok
}

int model_putcommand( model_t* mod, void* cmd, size_t len )
{
  // if this type of model has a putcommand function, call it.
  if( derived[ mod->type ].putcommand )
    {
      derived[ mod->type ].putcommand(mod, cmd, len);
      PRINT_WARN1( "used special putcommand, put %d bytes", (int)len );
    }
  else
    {
      stg_copybuf( &mod->cmd, &mod->cmd_len, cmd, len );
      PRINT_DEBUG1( "put a command of %d bytes", (int)len);
    }
  return 0; //ok
}

int model_getcommand( model_t* mod, void** command, size_t* len )
{
  // if this type of model has a getcommand function, call it.
  if( derived[ mod->type ].getcommand )
    {
      derived[ mod->type ].getcommand(mod, command, len);
      PRINT_WARN1( "used special getcommand, returned %d bytes", (int)*len );
    }
  else
    { // we do a generic command copy
      *command = mod->cmd;
      *len = mod->cmd_len; 
      PRINT_WARN1( "used generic getcommand, returned %d bytes", (int)*len );
    }
  return 0; //ok
}

int model_putconfig( model_t* mod, void* config, size_t len )
{
  // if this type of model has a putconfig function, call it.
  if( derived[ mod->type ].putconfig )
    {
      derived[ mod->type ].putconfig(mod, config, len);
      PRINT_WARN1( "used special putconfig returned %d bytes", (int)len );
    }
  else
    { // we do a generic data copy
      stg_copybuf( &mod->cfg, &mod->cfg_len, config, len );
      PRINT_WARN3( "model %d(%s) generic putconfig of %d bytes",
		   mod->id, mod->token, (int)mod->cfg_len);
    }
  
  return 0; //ok
}


int model_getconfig( model_t* mod, void** config, size_t* len )
{
  // if this type of model has an getconfig function, call it.
  if( derived[ mod->type ].getconfig )
    {
      derived[ mod->type ].getconfig(mod, config, len);
      PRINT_WARN1( "used special getconfig returned %d bytes", (int)*len );
    }
  else
    { // we do a generic data copy
      *config = mod->cfg;
      *len = mod->cfg_len; 
      PRINT_WARN3( "model %d(%s) generic getconfig, returned %d bytes", 
		   mod->id, mod->token, (int)*len );
    }
  return 0; //ok
}

// new registration funcs
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


