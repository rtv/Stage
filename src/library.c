
#include "model.h"

#define VERBOSE 1

void model_ranger_init( model_t* mod );
void model_ranger_update( model_t* mod );
void model_ranger_render( model_t* mod );

void model_laser_init( model_t* mod );
void model_laser_update( model_t* mod );
void model_laser_render( model_t* mod );
void model_laser_config_render( model_t* mod );

void model_blobfinder_init( model_t* mod );
void model_blobfinder_startup( model_t* mod );
void model_blobfinder_shutdown( model_t* mod );
void model_blobfinder_update( model_t* mod );
void model_blobfinder_render( model_t* mod );

void model_blobfinder_config_render( model_t* mod );

void model_fiducial_init( model_t* mod );
void model_fiducial_update( model_t* mod );
void model_fiducial_render( model_t* mod );

libitem_t items[] = 
  {
    { STG_PROP_RANGERDATA, 
      "ranger",
      model_ranger_init, 
      NULL, 
      NULL, 
      model_ranger_update, 
      model_ranger_render
    },
    
    { STG_PROP_BLOBDATA, 
      "blob",
      model_blobfinder_init, 
      model_blobfinder_startup, 
      model_blobfinder_shutdown, 
      model_blobfinder_update, 
      model_blobfinder_render 
    },

    { STG_PROP_BLOBCONFIG, 
      "blobconfig",
      model_blobfinder_init, // same as above
      NULL,
      NULL,
      NULL,
      model_blobfinder_config_render 
    },
    
    { STG_PROP_LASERDATA, 
      "laser",
      model_laser_init, 
      NULL, 
      NULL, 
      model_laser_update,
      model_laser_render
    },
    
    { STG_PROP_LASERCONFIG, 
      "laserconfig",
      model_laser_init, // same as above
      NULL,
      NULL,
      NULL,
      model_laser_config_render 
    },

    { STG_PROP_FIDUCIALDATA, 
      "fiducial",
      model_fiducial_init, 
      NULL, 
      NULL, 
      model_fiducial_update,
      model_fiducial_render
    },
    
    
    { 0,NULL,NULL,NULL,NULL,NULL}
  };

// the table above gets sorted by ID and stashed in here
libitem_t library[STG_PROP_COUNT];

int library_create( void )
{
  memset( library, 0, sizeof(libitem_t) * STG_PROP_COUNT );
  
  libitem_t* item = items;
  
#if VERBOSE
  printf( "[Models: " );
#endif  

  while( item->id  )
    {
#if VERBOSE
      printf( "%s ", item->name ); 
#endif

      memcpy( &library[item->id], item, sizeof(libitem_t) );
      item++;
    }
  
#if VERBSOSE
  printf( "\b] " );
  fflush( stdout );
#endif
  
}
