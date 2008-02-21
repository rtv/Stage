#include "stage.hh"
using namespace Stg;
  
typedef struct
{
  StgModelPosition* pos;
  StgModelLaser* laser;
} robot_t;

int LaserUpdate( StgModel* mod, robot_t* robot );
int PositionUpdate( StgModel* mod, robot_t* robot );

// Stage calls this when the model starts up
extern "C" int Init( StgModel* mod )
{
  puts( "Init controller" );
  
  robot_t* robot = new robot_t;
 
  robot->pos = (StgModelPosition*)mod;
  robot->laser = (StgModelLaser*)mod->GetModel( "laser:0" );
  
  assert( robot->laser );
  robot->laser->Subscribe();
  
  robot->laser->AddUpdateCallback( (stg_model_callback_t)LaserUpdate, robot );
  robot->pos->AddUpdateCallback( (stg_model_callback_t)PositionUpdate, robot );

  return 0; //ok
}

int LaserUpdate( StgModel* mod, robot_t* robot )
{
  robot->pos->SetSpeed( 0.1, 0, 0.1 );  
  return FALSE; // run again
}

int PositionUpdate( StgModel* mod, robot_t* robot )
{
  stg_pose_t pose = robot->pos->GetPose();

  printf( "Pose: [%.2f %.2f %.2f %.2f]\n",
	  pose.x, pose.y, pose.z, pose.a );

  return FALSE; // run again
}

