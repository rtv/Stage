
#include "stage.h"


void test_init( stg_model_t* mod )
{
  PRINT_WARN2( "TEST_INIT model %d %s", mod->id, mod->token );
  // todo - return 0; //ok


  mod->data = realloc( mod->data, 256 );
}

int test_startup( stg_model_t* mod )
{
  PRINT_WARN2( "TEST_STARTUP model %d %s", mod->id, mod->token );
  return 0; //ok
}

int test_shutdown( stg_model_t* mod )
{
  PRINT_WARN2( "TEST_SHUTDOWN model %d %s", mod->id, mod->token );
  return 0; //ok
}

int test_update( stg_model_t* mod )
{
  static int count = 0;

  PRINT_WARN2( "TEST_UPDATE model %d %s", mod->id, mod->token );
 
  snprintf( mod->data, 255, "hello %d", count++ );
  
  return 0; //ok
}

int test_getdata( stg_model_t* mod, void** data, size_t* len )
{
  PRINT_WARN2( "TEST_GETDATA model %d %s", mod->id, mod->token );

  *data = mod->data;
  *len = mod->data_len;

  //printf( "data is %s\n", *data );

  return 0; //ok
}

int test_putcommand( stg_model_t* mod, void* data, size_t len )
{
  PRINT_WARN2( "TEST_PUTCOMMAND model %d %s", mod->id, mod->token );

  mod->cmd = realloc( mod->cmd, len );
  memcpy( mod->cmd, data, len );
  mod->cmd_len  = len;

  return 0; //ok
}

int register_test( lib_entry_t* lib )
{ 
  assert(lib);
  
  lib[STG_MODEL_TEST].init = test_init;
  lib[STG_MODEL_TEST].startup = test_startup;
  lib[STG_MODEL_TEST].shutdown = test_shutdown;
  lib[STG_MODEL_TEST].update = test_update;
  lib[STG_MODEL_TEST].set_command = test_putcommand;


  return 0; //ok
} 
