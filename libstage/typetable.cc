#include "stage_internal.hh"


// define constructor wrapping functions for use in the type table only

static StgModel* CreateModel( StgWorld* world, StgModel* parent ) 
{  return new StgModel( world, parent ); }    

static StgModel* CreateModelBlinkenlight( StgWorld* world, StgModel* parent ) 
{  return new StgModelBlinkenlight( world, parent ); }    

static StgModel* CreateModelPosition( StgWorld* world, StgModel* parent ) 
{  return new StgModelPosition( world, parent ); }    

static StgModel* CreateModelLaser( StgWorld* world, StgModel* parent ) 
{  return new StgModelLaser( world, parent ); }    

static StgModel* CreateModelRanger( StgWorld* world, StgModel* parent ) 
{  return new StgModelRanger( world, parent ); }    

static StgModel* CreateModelCamera( StgWorld* world, StgModel* parent ) 
{  return new StgModelCamera( world, parent ); }    

static StgModel* CreateModelFiducial( StgWorld* world, StgModel* parent ) 
{  return new StgModelFiducial( world, parent ); }    

static StgModel* CreateModelBlobfinder( StgWorld* world, StgModel* parent ) 
{  return new StgModelBlobfinder( world, parent ); }    


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

