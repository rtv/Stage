
#include "model.h"

// declare your registration function here

// new style registration funcs for derived models
int register_test(void);
int register_laser(void);

lib_entry_t library[STG_PROP_COUNT];

lib_entry_t derived[STG_MODEL_COUNT];

void library_create( void )
{
  memset( library, 0, sizeof(library[0]) * STG_PROP_COUNT );
  memset( derived, 0, sizeof(derived[0]) * STG_MODEL_COUNT );
  
  // call your registration function here
  register_test();
  register_laser();
}


