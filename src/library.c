
#include "model.h"

// declare your registration function here

// new style registration funcs for derived models
int register_test(lib_entry_t* lib);
int register_laser(lib_entry_t* lib);
int register_position(lib_entry_t* lib);
int register_fiducial(lib_entry_t* lib);
int register_ranger(lib_entry_t* lib);
int register_blobfinder(lib_entry_t* lib);

lib_entry_t* stg_library_create( void )
{
  lib_entry_t* lib = calloc( sizeof(lib_entry_t), STG_MODEL_COUNT);
  
  // call your registration function here
  register_test(lib);
  register_laser(lib);
  register_position(lib);
  register_fiducial(lib);
  register_ranger(lib);
  register_blobfinder(lib);

  return lib;
}

void stg_library_destroy( lib_entry_t* lib )
{
  free( lib );
}

