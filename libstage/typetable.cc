#include "stage.hh"
using namespace Stg;


// define constructor wrapping functions for use in the type table only

static Model* CreateModel( World* world, Model* parent ) 
{  return new Model( world, parent ); }    

static Model* CreateModelBlinkenlight( World* world, Model* parent ) 
{  return new ModelBlinkenlight( world, parent ); }    

static Model* CreateModelPosition( World* world, Model* parent ) 
{  return new ModelPosition( world, parent ); }    

static Model* CreateModelLaser( World* world, Model* parent ) 
{  return new ModelLaser( world, parent ); }    

static Model* CreateModelRanger( World* world, Model* parent ) 
{  return new ModelRanger( world, parent ); }    

static Model* CreateModelCamera( World* world, Model* parent ) 
{  return new ModelCamera( world, parent ); }    

static Model* CreateModelFiducial( World* world, Model* parent ) 
{  return new ModelFiducial( world, parent ); }    

static Model* CreateModelBlobfinder( World* world, Model* parent ) 
{  return new ModelBlobfinder( world, parent ); }    

static Model* CreateModelGripper( World* world, Model* parent ) 
{  return new ModelGripper( world, parent ); }    


void Stg::RegisterModels()
{
  RegisterModel( MODEL_TYPE_PLAIN, "model", CreateModel );
  RegisterModel( MODEL_TYPE_LASER,  "laser", CreateModelLaser );
  RegisterModel( MODEL_TYPE_FIDUCIAL,  "fiducial", CreateModelFiducial );
  RegisterModel( MODEL_TYPE_RANGER, "ranger", CreateModelRanger );
  RegisterModel( MODEL_TYPE_CAMERA, "camera", CreateModelCamera );
  RegisterModel( MODEL_TYPE_POSITION, "position", CreateModelPosition );
  RegisterModel( MODEL_TYPE_BLOBFINDER, "blobfinder", CreateModelBlobfinder );
  RegisterModel( MODEL_TYPE_BLINKENLIGHT, "blinkenlight", CreateModelBlinkenlight);
  RegisterModel( MODEL_TYPE_GRIPPER, "gripper", CreateModelGripper);

#if DEBUG // human-readable view of the table
  puts( "Stg::Typetable" );
  for( int i=0; i<MODEL_TYPE_COUNT; i++ )
	 printf( "  %d %s %p\n",
				i,
				typetable[i].token,
				typetable[i].creator );
  puts("");
#endif
}  

