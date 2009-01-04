

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
int wifi_init( stg_model_t* mod );
int speech_init( stg_model_t* mod );
int audio_init( stg_model_t* mod );
int bumper_init( stg_model_t* mod );
int indicator_init( stg_model_t* mod );

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
    { "wifi", wifi_init },
    { "speech", speech_init },
    { "audio", audio_init },
    { "indicator", indicator_init },
    { "bumper", bumper_init },
    { NULL, NULL } // this must be the last entry
  };

