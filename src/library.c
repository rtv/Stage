
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


