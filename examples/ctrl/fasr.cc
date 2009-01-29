#include "stage.hh"
using namespace Stg;

const bool verbose = false;

// navigation control params
const double cruisespeed = 0.4; 
const double avoidspeed = 0.05; 
const double avoidturn = 0.5;
const double minfrontdistance = 0.7;  
const double stopdist = 0.5;
const int avoidduration = 10;
const int workduration = 20;
const int payload = 4;

double have[4][4] = { 
  { 90, 180, 180, 180 },
  { 90, -90, 0, -90 },
  { 90, 90, 180, 90 },
  { 0, 45, 0, 0} 
};

double need[4][4] = {
  { -120, -180, 180, 180 },
  { -90, -120, 180, 180 },
  { -90, -90, 180, 180 },
  { -90, -180, -90, -90 }
};


class Robot
{
private:
  ModelPosition* pos;
  ModelLaser* laser;
  ModelRanger* ranger;
  //ModelBlobfinder* blobfinder;
  ModelFiducial* fiducial;
  Model *source, *sink;
  int avoidcount, randcount;
  int work_get, work_put;
  
  static int LaserUpdate( ModelLaser* mod, Robot* robot );
  static int PositionUpdate( ModelPosition* mod, Robot* robot );
  static int FiducialUpdate( ModelFiducial* mod, Robot* robot );

public:
  Robot( ModelPosition* pos, 
			Model* source,
			Model* sink ) 
	 : pos(pos), 
		laser( (ModelLaser*)pos->GetUnusedModelOfType( MODEL_TYPE_LASER )),
		ranger( (ModelRanger*)pos->GetUnusedModelOfType( MODEL_TYPE_RANGER )),
		fiducial( (ModelFiducial*)pos->GetUnusedModelOfType( MODEL_TYPE_FIDUCIAL )),
		source(source), 
		sink(sink), 
		avoidcount(0), 
		randcount(0), 
		work_get(0), 
		work_put(0)
  {
	 // need at least these models to get any work done
	 // (pos must be good, as we used it in the initialization list)
	 assert( laser );
	 assert( source );
	 assert( sink );

	 pos->AddUpdateCallback( (stg_model_callback_t)PositionUpdate, this );
	 laser->AddUpdateCallback( (stg_model_callback_t)LaserUpdate, this );

	 if( fiducial ) // optional
		fiducial->AddUpdateCallback( (stg_model_callback_t)FiducialUpdate, this );
  }
};



// Stage calls this when the model starts up
extern "C" int Init( Model* mod )
{  
  Robot* robot = new Robot( (ModelPosition*)mod,
									 mod->GetWorld()->GetModel( "source" ),
									 mod->GetWorld()->GetModel( "sink" ) );
    
  return 0; //ok
}

// inspect the laser data and decide what to do
int Robot::LaserUpdate( ModelLaser* laser, Robot* robot )
{
  if( laser->power_pack && laser->power_pack->charging )
	 printf( "model %s power pack @%p is charging\n",
				laser->Token(), laser->power_pack );
  
  // Get the data
  uint32_t sample_count=0;
  stg_laser_sample_t* scan = laser->GetSamples( &sample_count );

  if( scan == NULL )
	 return 0;
  
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
      
      Pose pose = robot->pos->GetPose();

      int x = (pose.x + 8) / 4;
      int y = (pose.y + 8) / 4;

		// oh what an awful bug - 5 hours to track this down. When using
		// this controller in a world larger than 8*8 meters, a_goal can
		// sometimes be NAN. Causing trouble WAY upstream. 
 		if( x > 3 ) x = 3;
 		if( y > 3 ) y = 3;
 		if( x < 0 ) x = 0;
 		if( y < 0 ) y = 0;
      
      double a_goal = 
		  dtor( robot->pos->GetFlagCount() ? have[y][x] : need[y][x] );
      
		assert( ! isnan(a_goal ) );
		assert( ! isnan(pose.a ) );

      double a_error = normalize( a_goal - pose.a );

		assert( ! isnan(a_error) );

      robot->pos->SetTurnSpeed(  a_error );
    }
 
 
  return 0;
}

int Robot::PositionUpdate( ModelPosition* pos, Robot* robot )
{  
  Pose pose = pos->GetPose();
  
  //printf( "Pose: [%.2f %.2f %.2f %.2f]\n",
  //  pose.x, pose.y, pose.z, pose.a );
  
  //pose.z += 0.0001;
  //robot->pos->SetPose( pose );
  
  if( pos->GetFlagCount() < payload && 
      hypot( -7-pose.x, -7-pose.y ) < 2.0 )
    {
      if( ++robot->work_get > workduration )
		  {
			 // protect source from concurrent access
			 robot->source->Lock();

			 // transfer a chunk from source to robot
			 pos->PushFlag( robot->source->PopFlag() );
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
			 robot->sink->PushFlag( pos->PopFlag() );
			 robot->sink->Unlock();

			 robot->work_put = 0;
		  }
    }
  
  
  return 0; // run again
}



int Robot::FiducialUpdate( ModelFiducial* mod, Robot* robot )
{    
  for( unsigned int i = 0; i < mod->fiducial_count; i++ )
	 {
		stg_fiducial_t* f = &mod->fiducials[i];
		
		//printf( "fiducial %d is %d at %.2f m %.2f radians\n",
		//	  i, f->id, f->range, f->bearing );
		
		//		if( 0 )
		if( f->range < 1 )
		  {
			 printf( "attempt to grab model @%p %s\n",
						f->mod, f->mod->Token() );
			 	
			 // working on picking up models
			 robot->pos->BecomeParentOf( f->mod );
			 f->mod->SetPose( Pose(0,0,0,0) );
			 f->mod->Disable();
		  }
		
	 }						  
  
  return 0; // run again
}
