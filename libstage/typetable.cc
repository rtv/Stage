#include "stage.hh"
using namespace Stg;


static void Register( stg_model_type_t type,
					  const char* desc, 
					  stg_creator_t func )
{
  Model::name_map[ desc ] = func;  
  Model::type_map[ type ] = desc;
}

/** Map model names to named constructors for each model type */
void Stg::RegisterModels()
{
  // the generic model
  Register( MODEL_TYPE_PLAIN, "model", Model::Create );
  
  // and the rest (in alphabetical order)
  Register( MODEL_TYPE_ACTUATOR, "actuator", ModelActuator::Create );
  Register( MODEL_TYPE_BLINKENLIGHT, "blinkenlight", ModelBlinkenlight::Create );
  Register( MODEL_TYPE_BLOBFINDER, "blobfinder", ModelBlobfinder::Create );
  Register( MODEL_TYPE_CAMERA, "camera", ModelCamera::Create );
  Register( MODEL_TYPE_FIDUCIAL, "fiducial", ModelFiducial::Create );
  Register( MODEL_TYPE_GRIPPER, "gripper", ModelGripper::Create );
  Register( MODEL_TYPE_LASER, "laser", ModelLaser::Create );
  Register( MODEL_TYPE_LIGHTINDICATOR, "lightindicator", ModelLightIndicator::Create );
  Register( MODEL_TYPE_LOADCELL, "loadcel", ModelLoadCell::Create );
  Register( MODEL_TYPE_POSITION, "position", ModelPosition::Create );
  Register( MODEL_TYPE_RANGER, "ranger",  ModelRanger::Create );
}  

