#include "stage.hh"
using namespace Stg;


/* Template used to created named wrappers for model constructors */
template <class T>
Model* Creator( World* world, Model* parent, const std::string& type )
{
  return new T( world, parent, type );
}

static void Register( const std::string& type,
							 creator_t func )
{
  Model::name_map[ type ] = func;  
}

/** Map model names to named constructors for each model type */
void Stg::RegisterModels()
{
  // generic model
  Register( "model", Creator<Model> );
  
  Register( "actuator", Creator<ModelActuator> );
  Register( "blinkenlight", Creator<ModelBlinkenlight> );
  Register( "blobfinder", Creator<ModelBlobfinder> );
  Register( "camera", Creator<ModelCamera> );
  Register( "fiducial", Creator<ModelFiducial> );
  Register( "gripper", Creator<ModelGripper> );
  Register( "laser", Creator<ModelLaser> );
  Register( "lightindicator", Creator<ModelLightIndicator> );
  Register( "position", Creator<ModelPosition> );
  Register( "ranger",  Creator<ModelRanger> );
}  

