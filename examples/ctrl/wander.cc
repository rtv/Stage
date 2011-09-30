#include "stage.hh"
using namespace Stg;

static const double cruisespeed = 0.4; 
static const double avoidspeed = 0.05; 
static const double avoidturn = 0.5;
static const double minfrontdistance = 1.0; // 0.6  
static const bool verbose = false;
static const double stopdist = 0.3;
static const int avoidduration = 10;

typedef struct
{
  ModelPosition* pos;
  ModelRanger* laser;
  int avoidcount, randcount;
} robot_t;

int LaserUpdate( Model* mod, robot_t* robot );
int PositionUpdate( Model* mod, robot_t* robot );

// Stage calls this when the model starts up
extern "C" int Init( Model* mod, CtrlArgs* args )
{
  // local arguments
  /*  printf( "\nWander controller initialised with:\n"
      "\tworldfile string \"%s\"\n" 
      "\tcmdline string \"%s\"",
      args->worldfile.c_str(),
      args->cmdline.c_str() );
  */

  robot_t* robot = new robot_t;
 
  robot->avoidcount = 0;
  robot->randcount = 0;
  
  robot->pos = (ModelPosition*)mod;
  
  if( verbose )
    robot->pos->AddCallback( Model::CB_UPDATE, (model_callback_t)PositionUpdate, robot );
  
  robot->pos->Subscribe(); // starts the position updates
  
  robot->laser = (ModelRanger*)mod->GetChild( "ranger:1" );
  robot->laser->AddCallback( Model::CB_UPDATE, (model_callback_t)LaserUpdate, robot );
  robot->laser->Subscribe(); // starts the ranger updates
  
  return 0; //ok
}


// inspect the ranger data and decide what to do
int LaserUpdate( Model* mod, robot_t* robot )
{
  // get the data
  const std::vector<meters_t>& scan = robot->laser->GetRanges();
  uint32_t sample_count = scan.size();
  if( sample_count < 1 )
    return 0;
  
  bool obstruction = false;
  bool stop = false;
	
  // find the closest distance to the left and right and check if
  // there's anything in front
  double minleft = 1e6;
  double minright = 1e6;

  for (uint32_t i = 0; i < sample_count; i++)
    {

      if( verbose ) printf( "%.3f ", scan[i] );

      if( (i > (sample_count/3)) 
	  && (i < (sample_count - (sample_count/3))) 
	  && scan[i] < minfrontdistance)
	{
	  if( verbose ) puts( "  obstruction!" );
	  obstruction = true;
	}
		
      if( scan[i] < stopdist )
	{
	  if( verbose ) puts( "  stopping!" );
	  stop = true;
	}
      
      if( i > sample_count/2 )
	minleft = std::min( minleft, scan[i] );
      else      
	minright = std::min( minright, scan[i] );
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
      robot->pos->SetTurnSpeed(  0 );
    }

  //  if( robot->pos->Stalled() )
  // 	 {
  // 		robot->pos->SetSpeed( 0,0,0 );
  // 		robot->pos->SetTurnSpeed( 0 );
  // }
			
  return 0; // run again
}

int PositionUpdate( Model* mod, robot_t* robot )
{
  Pose pose = robot->pos->GetPose();

  printf( "Pose: [%.2f %.2f %.2f %.2f]\n",
	  pose.x, pose.y, pose.z, pose.a );

  return 0; // run again
}

