
#include "model.h"

#define VERBOSE 1

void model_ranger_shutdown( model_t* mod );
void model_ranger_update( model_t* mod );
void model_ranger_render( model_t* mod );
void model_ranger_config_render( model_t* mod );

void model_laser_shutdown( model_t* mod );
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
void model_fiducial_shutdown( model_t* mod );
void model_fiducial_update( model_t* mod );
void model_fiducial_render( model_t* mod );
void model_fiducial_config_render( model_t* mod );

libitem_t items[] = 
  {
    { STG_PROP_RANGERDATA,
      "ranger",
      NULL,
      NULL,
      model_ranger_shutdown,
      model_ranger_update,
      model_ranger_render
    },

    { STG_PROP_RANGERCONFIG,
      "rangerconfig",
      NULL,
      NULL,
      NULL,
      NULL,      
      model_ranger_config_render
    },
    
/*     { STG_PROP_BLOBDATA, */
/*       "blob", */
/*       NULL, */
/*       model_blobfinder_startup, */
/*       model_blobfinder_shutdown, */
/*       model_blobfinder_update, */
/*       model_blobfinder_render */
/*     }, */

/*     { STG_PROP_BLOBCONFIG,  */
/*       "blobconfig", */
/*       NULL, */
/*       NULL, */
/*       NULL, */
/*       NULL, */
/*       model_blobfinder_config_render  */
/*     }, */
    
    { STG_PROP_LASERDATA, 
      "laser",
      NULL, 
      NULL,
      model_laser_shutdown, 
      model_laser_update,
      model_laser_render
    },
    
    { STG_PROP_LASERCONFIG, 
      "laserconfig",
      NULL,
      NULL,
      NULL,
      NULL,
      model_laser_config_render 
    },

 /*    { STG_PROP_FIDUCIALDATA,  */
/*       "fiducial", */
/*       NULL, */
/*       NULL,  */
/*       model_fiducial_shutdown, */
/*       model_fiducial_update, */
/*       model_fiducial_render */
/*     }, */

    
/*     { STG_PROP_FIDUCIALCONFIG,  */
/*       "fiducialconfig", */
/*       NULL, */
/*       NULL, */
/*       NULL,  */
/*       NULL, */
/*       model_fiducial_config_render */
/*     }, */
    
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
