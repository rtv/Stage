#include "stage.hh"
using namespace Stg;

const double cruisespeed = 0.4; 
const double avoidspeed = 0.05; 
const double avoidturn = 1.0;
const double minfrontdistance = 0.8;  
const bool verbose = false;

typedef struct
{
  StgModelPosition* pos;
  StgModelLaser* laser;
  int avoidcount, randcount;
} robot_t;

int LaserUpdate( StgModel* mod, robot_t* robot );
int PositionUpdate( StgModel* mod, robot_t* robot );

// Stage calls this when the model starts up
extern "C" int Init( StgModel* mod )
{
  robot_t* robot = new robot_t;
 
  robot->pos = (StgModelPosition*)mod;
  robot->laser = (StgModelLaser*)mod->GetModel( "laser:0" );
  robot->avoidcount = 0;
  robot->randcount = 0;

  assert( robot->laser );
  robot->laser->Subscribe();
  
  robot->laser->AddUpdateCallback( (stg_model_callback_t)LaserUpdate, robot );
  //robot->pos->AddUpdateCallback( (stg_model_callback_t)PositionUpdate, robot );
  return 0; //ok
}

// inspect the laser data and decide what to do
int LaserUpdate( StgModel* mod, robot_t* robot )
{
  // get the data
  uint32_t sample_count=0;
  stg_laser_sample_t* scan = robot->laser->GetSamples( &sample_count );
  assert(scan);
  
  double newturnrate=0.0, newspeed=0.0;  
  bool obstruction = false;
  
  // find the closest distance to the left and right and check if
  // there's anything in front
  double minleft = 1e6;
  double minright = 1e6;
  
  for (uint32_t i = 0; i < sample_count; i++)
    {
      if( scan[i].range < minfrontdistance)
	obstruction = true;
      
      if( i > sample_count/2 )
	minleft = MIN( minleft, scan[i].range );
      else      
	minright = MIN( minright, scan[i].range );
    }
  
  if( obstruction || robot->avoidcount )
    {
      if( verbose ) puts( "Avoid" );

      robot->pos->SetXSpeed( avoidspeed );
      
      /* once we start avoiding, select a turn direction and stick
	 with it for a few iterations */
      if( robot->avoidcount == 0 )
        {
	  if( verbose ) puts( "Avoid START" );
          robot->avoidcount = 5;
	  
	  if( minleft < minright  )
	    robot->pos->SetTurnSpeed( -avoidturn );
	  else
	    robot->pos->SetTurnSpeed( +avoidturn );
        }

      robot->avoidcount--;
    }
  else
    {
      if( verbose ) puts( "Cruise" );

      robot->avoidcount = 0;
      robot->pos->SetXSpeed( cruisespeed );	  
      
      /* update turnrate every few updates */
      if( robot->randcount == 0 )
	{
	  if( verbose )puts( "Random turn" );
	  
	  /* make random int tween -30 and 30 */
	  //newturnrate = dtor( rand() % 61 - 30 );
	  
	  robot->randcount = 20;
	  
	  robot->pos->SetTurnSpeed(  dtor( rand() % 11 - 5 ) );
	}
      
      robot->randcount--;
    }
  
  return 0;
}

int PositionUpdate( StgModel* mod, robot_t* robot )
{
  stg_pose_t pose = robot->pos->GetPose();

  printf( "Pose: [%.2f %.2f %.2f %.2f]\n",
	  pose.x, pose.y, pose.z, pose.a );

  return 0; // run again
}

