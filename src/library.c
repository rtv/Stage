
#include "model.h"

lib_entry_t library[STG_PROP_COUNT];

void library_create( void )
{
  memset( library, 0, sizeof(library[0]) * STG_PROP_COUNT );
  
  // add your registration function here
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


