

#include "stage_internal.h"

// declare the initialization functions for specialized models
int blobfinder_init( stg_model_t* mod );
int fiducial_init( stg_model_t* mod );
int gripper_init( stg_model_t* mod );
int laser_init( stg_model_t* mod );
int position_init( stg_model_t* mod );
int ranger_init( stg_model_t* mod );
int energy_init( stg_model_t* mod );

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
    { "power", energy_init },       
    { NULL, 0, NULL } // this must be the last entry
  };

/* // map worldfile keywords onto initialization functions */
/* stg_type_record_t typetable[] =  */
/*   {     */
/*     { "model", STG_MODEL_TYPE_GENERIC, NULL }, */
/*     { "generic", STG_MODEL_TYPE_GENERIC, NULL }, */
/*     { "position", STG_MODEL_TYPE_POSITION, position_init }, */
/*     { "ranger",  STG_MODEL_TYPE_RANGER, ranger_init }, */
/*     { "laser", STG_MODEL_TYPE_LASER, laser_init }, */
/*     { "blobfinder",STG_MODEL_TYPE_BLOBFINDER, blobfinder_init }, */
/*     { "fiducialfinder", STG_MODEL_TYPE_FIDUCIAL, fiducial_init }, */
/*     { "gripper", STG_MODEL_TYPE_GRIPPER, gripper_init },        */
/*     { "power",STG_MODEL_TYPE_ENERGY, energy_init },        */
/*     { NULL, 0, NULL } // this must be the last entry */
/*   }; */

