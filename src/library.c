
#include "model.h"


void model_ranger_init( model_t* mod );
void model_ranger_update( model_t* mod );
void model_ranger_render( model_t* mod );

void model_laser_init( model_t* mod );
void model_laser_update( model_t* mod );
void model_laser_render( model_t* mod );

void model_blobfinder_init( model_t* mod );
void model_blobfinder_update( model_t* mod );
void model_blobfinder_render( model_t* mod );

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
      NULL, 
      NULL, 
      model_blobfinder_update, 
      model_blobfinder_render 
    },
    
    { STG_PROP_LASERDATA, 
      "laser",
      model_laser_init, 
      NULL, 
      NULL, 
      model_laser_update,
      model_laser_render
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
  
  printf( "[Models: " );
  
  while( item->id  )
    {
      memcpy( &library[item->id], item, sizeof(libitem_t) );
      //inits[ item->id ] = item->init;
      printf( "%s ", item->name ); 
      item++;
    }

  printf( "\b] " );
  fflush( stdout );
  
}
