
#include "model.h"

// declare your registration function here
void  model_pose_register();
void model_geom_register();
void model_mass_register(); 
void model_lines_register();
void model_color_register();
void model_velocity_register();
void model_laser_register();
void model_guifeatures_register();
void model_ranger_register();
void model_fiducial_register();
void model_blobfinder_register();
void model_energy_register();

lib_entry_t library[STG_PROP_COUNT];

void library_create( void )
{
  memset( library, 0, sizeof(library[0]) * STG_PROP_COUNT );
  
  // call your registration function here
  model_pose_register(); 
  model_geom_register();
  model_mass_register(); 
  model_lines_register();
  model_color_register();
  model_velocity_register();
  model_laser_register();
  model_guifeatures_register();
  model_ranger_register();
  model_fiducial_register();
  model_blobfinder_register();
  model_energy_register();
}


