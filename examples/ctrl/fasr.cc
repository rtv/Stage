#include "stage.hh"
using namespace Stg;

const double cruisespeed = 0.4; 
const double avoidspeed = 0.05; 
const double avoidturn = 0.5;
const double minfrontdistance = 0.7;  
const double stopdist = 0.5;
const bool verbose = false;
const int avoidduration = 10;
const int workduration = 20;
const int payload = 4;


double have[4][4] = { 
  { 90, 180, 180, 180 },
  { 90, -90, 0, -90 },
  { 90, 90, 180, 90 },
  { 0, 45, 0, 45} 
};

double need[4][4] = {
  { -120, -180, 180, 180 },
  { -90, -120, 180, 180 },
  { -90, -90, 180, 180 },
  { -90, -180, -90, -90 }
};
  


typedef struct
{
  StgModelPosition* pos;
  StgModelLaser* laser;
  StgModelBlobfinder* blobfinder;

  StgModel *source, *sink;

  int avoidcount, randcount;

  int work_get, work_put;

} robot_t;

int LaserUpdate( StgModel* mod, robot_t* robot );
int PositionUpdate( StgModel* mod, robot_t* robot );

// Stage calls this when the model starts up
extern "C" int Init( StgModel* mod )
{
  robot_t* robot = new robot_t;
  robot->work_get = 0;
  robot->work_put = 0;
  
  robot->pos = (StgModelPosition*)mod;

  robot->laser = (StgModelLaser*)mod->GetModel( "laser:0" );
  assert( robot->laser );
  robot->laser->Subscribe();

  robot->avoidcount = 0;
  robot->randcount = 0;
  
  robot->laser->AddUpdateCallback( (stg_model_callback_t)LaserUpdate, robot );
  robot->pos->AddUpdateCallback( (stg_model_callback_t)PositionUpdate, robot );

  robot->source = mod->GetWorld()->GetModel( "source" );
  assert(robot->source);

  robot->sink = mod->GetWorld()->GetModel( "sink" );
  assert(robot->sink);

  return 0; //ok
}

// inspect the laser data and decide what to do
int LaserUpdate( StgModel* mod, robot_t* robot )
{
  // get the data
  uint32_t sample_count=0;
  stg_laser_sample_t* scan = robot->laser->GetSamples( &sample_count );
  assert(scan);
  
  bool obstruction = false;
  bool stop = false;

  // find the closest distance to the left and right and check if
  // there's anything in front
  double minleft = 1e6;
  double minright = 1e6;
  
  for (uint32_t i = 0; i < sample_count; i++)
    {
      if( (i > (sample_count/4)) 
	  && (i < (sample_count - (sample_count/4))) 
	  && scan[i].range < minfrontdistance)
	obstruction = true;
      
      if( scan[i].range < stopdist )
	stop = true;
      
      if( i > sample_count/2 )
	minleft = MIN( minleft, scan[i].range );
      else      
	minright = MIN( minright, scan[i].range );
    }
  
  if( obstruction || stop || (robot->avoidcount>0) )
    {
      if( verbose ) puts( "Avoid" );
      robot->pos->SetXSpeed( stop ? 0.0 : avoidspeed );      
      
      /* once we start avoiding, select a turn direction and stick
	 with it for a few iterations */
      if( robot->avoidcount < 1 )
        {
	  if( verbose ) puts( "Avoid START" );
          robot->avoidcount = random() % avoidduration + avoidduration;

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
      
      stg_pose_t pose = robot->pos->GetPose();

      int x = (pose.x + 8) / 4;
      int y = (pose.y + 8) / 4;
      
      double a_goal = 
	dtor( robot->pos->GetFlagCount() ? have[y][x] : need[y][x] );
      
      double a_error = normalize( a_goal - pose.a );
 
      robot->pos->SetTurnSpeed(  a_error );
    }
  
  return 0;
}

int PositionUpdate( StgModel* mod, robot_t* robot )
{
  stg_pose_t pose = robot->pos->GetPose();
  
  //printf( "Pose: [%.2f %.2f %.2f %.2f]\n",
  //  pose.x, pose.y, pose.z, pose.a );
  
  if( robot->pos->GetFlagCount() < payload && 
      hypot( -7-pose.x, -7-pose.y ) < 2.0 )
    {
      if( ++robot->work_get > workduration )
	{
	  robot->pos->PushFlag( robot->source->PopFlag() );
	  robot->work_get = 0;
	}	  
    }
  
  if( hypot( 7-pose.x, 7-pose.y ) < 1.0 )
    {
      if( ++robot->work_put > workduration )
	{
	  //puts( "dropping" );
	  // transfer a chunk between robot and goal
	  robot->sink->PushFlag( robot->pos->PopFlag() );
	  robot->work_put = 0;
	}
    }
  
  return 0; // run again
}

