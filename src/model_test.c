
#include "model.h"


char * mesg = "hello";

void test_init( model_t* mod )
{
  PRINT_WARN2( "TEST_INIT model %d %s", mod->id, mod->token );
}

int test_startup( model_t* mod )
{
  PRINT_WARN2( "TEST_STARTUP model %d %s", mod->id, mod->token );
}

int test_shutdown( model_t* mod )
{
  PRINT_WARN2( "TEST_SHUTDOWN model %d %s", mod->id, mod->token );
}

int test_update( model_t* mod )
{
  PRINT_WARN2( "TEST_UPDATE model %d %s", mod->id, mod->token );
}

int test_getdata( model_t* mod, void** data, size_t* len )
{
  PRINT_WARN2( "TEST_GETDATA model %d %s", mod->id, mod->token );

  *data = (void*)mesg;
  *len = strlen(mesg);

  return 0; //ok
}

int test_putcommand( model_t* mod, void* data, size_t len )
{
  PRINT_WARN2( "TEST_GETDATA model %d %s", mod->id, mod->token );

  mod->cmd = realloc( mod->cmd, len );
  memcpy( mod->cmd, data, len );
  mod->cmd_len  = len;

  return 0; //ok
}

int register_test( lib_entry_t lib[] )
{
  register_init( STG_MODEL_TEST, test_init );
  register_startup( STG_MODEL_TEST, test_startup );
  register_shutdown( STG_MODEL_TEST, test_shutdown );
  register_getdata( STG_MODEL_TEST, test_getdata );
  register_putcommand( STG_MODEL_TEST, test_putcommand );
  register_update( STG_MODEL_TEST, test_update );


} 
