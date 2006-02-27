

#include "stage_internal.h"

// declare the initialization functions for specialized models
int blobfinder_init( stg_model_t* mod );
int fiducial_init( stg_model_t* mod );
int gripper_init( stg_model_t* mod );
int laser_init( stg_model_t* mod );
int position_init( stg_model_t* mod );
int ranger_init( stg_model_t* mod );
//int energy_init( stg_model_t* mod );
int ptz_init( stg_model_t* mod );

// map worldfile keywords onto initialization functions
stg_type_record_t typetable[] = 
  {    
    { "model", NULL },
    { "position", position_init },
    { "ranger",  ranger_init },
    { "laser", laser_init },
    { "blobfinder", blobfinder_init },
    { "fiducialfinder", fiducial_init },
    { "gripper", gripper_init },       
    //{ "power", energy_init },      
    { "ptz", ptz_init },       
    { NULL, 0, NULL } // this must be the last entry
  };

