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
  StgModelRanger* ranger;
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

  robot->ranger = (StgModelRanger*)mod->GetModel( "ranger:0" );
  assert( robot->ranger );
  robot->ranger->Subscribe();


  robot->avoidcount = 0;
  robot->randcount = 0;
  
  robot->laser->AddUpdateCallback( (stg_model_callback_t)LaserUpdate, robot );
  robot->pos->AddUpdateCallback( (stg_model_callback_t)PositionUpdate, robot );

  robot->source = mod->GetWorld()->GetModel( "source" );
  assert(robot->source);

  robot->sink = mod->GetWorld()->GetModel( "sink" );
  assert(robot->sink);
    
  
//   const int waypoint_count = 100;
//   Waypoint* waypoints = new Waypoint[waypoint_count];
  
//   for( int i=0; i<waypoint_count; i++ )
// 	 {
// 		waypoints[i].pose.x = i* 0.1;
// 		waypoints[i].pose.y = drand48() * 4.0;
// 		waypoints[i].pose.z = 0;
// 		waypoints[i].pose.a = normalize( i/10.0 );
// 		waypoints[i].color = stg_color_pack( 0,0,1,0 );
// 	 }
  
//   robot->pos->SetWaypoints( waypoints, waypoint_count );

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

  //return 0;
  
  for (uint32_t i = 0; i < sample_count; i++)
    {

		if( verbose ) printf( "%.3f ", scan[i].range );

      if( (i > (sample_count/4)) 
			 && (i < (sample_count - (sample_count/4))) 
			 && scan[i].range < minfrontdistance)
		  {
			 if( verbose ) puts( "  obstruction!" );
			 obstruction = true;
		  }
		
      if( scan[i].range < stopdist )
		  {
			 if( verbose ) puts( "  stopping!" );
			 stop = true;
		  }
      
      if( i > sample_count/2 )
		  minleft = MIN( minleft, scan[i].range );
      else      
		  minright = MIN( minright, scan[i].range );
    }
  
  if( verbose ) 
	 {
		puts( "" );
		printf( "minleft %.3f \n", minleft );
		printf( "minright %.3f\n ", minright );
	 }

  if( obstruction || stop || (robot->avoidcount>0) )
    {
      if( verbose ) printf( "Avoid %d\n", robot->avoidcount );
	  		
      robot->pos->SetXSpeed( stop ? 0.0 : avoidspeed );      
      
      /* once we start avoiding, select a turn direction and stick
	 with it for a few iterations */
      if( robot->avoidcount < 1 )
        {
			 if( verbose ) puts( "Avoid START" );
          robot->avoidcount = random() % avoidduration + avoidduration;
			 
			 if( minleft < minright  )
				{
				  robot->pos->SetTurnSpeed( -avoidturn );
				  if( verbose ) printf( "turning right %.2f\n", -avoidturn );
				}
			 else
				{
				  robot->pos->SetTurnSpeed( +avoidturn );
				  if( verbose ) printf( "turning left %2f\n", +avoidturn );
				}
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
  
  //pose.z += 0.0001;
  //robot->pos->SetPose( pose );
  
  if( robot->pos->GetFlagCount() < payload && 
      hypot( -7-pose.x, -7-pose.y ) < 2.0 )
    {
      if( ++robot->work_get > workduration )
		  {
			 // protect source from concurrent access
			 robot->source->Lock();

			 // transfer a chunk from source to robot
			 robot->pos->PushFlag( robot->source->PopFlag() );
			 robot->source->Unlock();

			 robot->work_get = 0;
		  }	  
    }
  
  if( hypot( 7-pose.x, 7-pose.y ) < 1.0 )
    {
      if( ++robot->work_put > workduration )
		  {
			 // protect sink from concurrent access
			 robot->sink->Lock();

			 //puts( "dropping" );
			 // transfer a chunk between robot and goal
			 robot->sink->PushFlag( robot->pos->PopFlag() );
			 robot->sink->Unlock();

			 robot->work_put = 0;
		  }
    }
  
  
  return 0; // run again
}

