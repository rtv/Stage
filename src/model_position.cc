///////////////////////////////////////////////////////////////////////////
//
// File: model_laser.c
// Author: Richard Vaughan
// Date: 10 June 2004
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/model_position.cc,v $
//  $Author: rtv $
//  $Revision: 1.1.2.1 $
//
///////////////////////////////////////////////////////////////////////////

//#define DEBUG

#include <sys/time.h>
#include <math.h>
#include <stdlib.h>

//#define DEBUG

#include "model.hh"
#include "gui.h"

/** 
@ingroup model
@defgroup model_position Position model 

The position model simulates a
mobile robot base. It can drive in one of two modes; either
<i>differential</i>, i.e. able to control its speed and turn rate by
driving left and roght wheels like a Pioneer robot, or
<i>omnidirectional</i>, i.e. able to control each of its three axes
independently.

<h2>Worldfile properties</h2>

@par Summary and default values

@verbatim
position
(
  # position properties
  drive "diff"
  localization "gps"
  localization_origin [ <defaults to model's start pose> ]
 
  # odometry error model parameters, 
  # only used if localization is set to "odom"
  odom_error [0.03 0.03 0.05]

  # model properties
)
@endverbatim

@par Note
Since Stage-1.6.5 the odom property has been removed. Stage will generate a warning if odom is defined in your worldfile. See localization_origin instead.

@par Details
- drive "diff", "omni" or "car"
  - select differential-steer mode (like a Pioneer), omnidirectional mode or carlike (velocity and steering angle).
- localization "gps" or "odom"
  - if "gps" the position model reports its position with perfect accuracy. If "odom", a simple odometry model is used and position data drifts from the ground truth over time. The odometry model is parameterized by the odom_error property.
- localization_origin [x y theta]
  - set the origin of the localization coordinate system. By default, this is copied from the model's initial pose, so the robot reports its position relative to the place it started out. Tip: If localization_origin is set to [0 0 0] and localization is "gps", the model will return its true global position. This is unrealistic, but useful if you want to abstract away the details of localization. Be prepared to justify the use of this mode in your research! 
- odom_error [x y theta]
  - parameters for the odometry error model used when specifying localization "odom". Each value is the maximum proportion of error in intergrating x, y, and theta velocities to compute odometric position estimate. For each axis, if the the value specified here is E, the actual proportion is chosen at startup at random in the range -E/2 to +E/2. Note that due to rounding errors, setting these values to zero does NOT give you perfect localization - for that you need to choose localization "gps".
*/


/* External interface */

/// set the current odometry estimate 
void StgModelPosition::SetOdom( stg_pose_t* odom )
{
  assert(this->data);
  stg_position_data_t* data = (stg_position_data_t*)this->data;
  memcpy( &data->pose, odom, sizeof(stg_pose_t));

  // figure out where the implied origin is in global coords
  stg_pose_t gp;
  this->GetGlobalPose( &gp );
			
  double da = -odom->a + gp.a;
  double dx = -odom->x * cos(da) + odom->y * sin(da);
  double dy = -odom->y * cos(da) - odom->x * sin(da);

  stg_pose_t origin;
  origin.x = gp.x + dx;
  origin.y = gp.y + dy;
  origin.a = da;

  memcpy( &data->origin, &origin, sizeof(stg_pose_t));
  
  //model_change( mod, &mod->data );
  this->SetData( this->data, this->data_len );
} 

/* Internal */

const double STG_POSITION_WATTS_KGMS = 5.0; // cost per kg per meter per second
const double STG_POSITION_WATTS = 2.0; // base cost of position device

// simple odometry error model parameters. the error is selected at
// random in the interval -MAX/2 to +MAX/2 at startup
const double STG_POSITION_INTEGRATION_ERROR_MAX_X = 0.03;
const double STG_POSITION_INTEGRATION_ERROR_MAX_Y = 0.03;
const double STG_POSITION_INTEGRATION_ERROR_MAX_A = 0.05;


StgModelPosition::StgModelPosition( stg_world_t* world, 
				    StgModel* parent,
				    stg_id_t id,
				    CWorldFile* wf )
  : StgModel( world, parent, id, wf )
{
  PRINT_DEBUG2( "Constructing StgModelPosition %d (%s)\n", 
		id, wf->GetEntityType( id ) );
  
  // TODO - move this to stg_init()
  static int first_time = 1;
  if( first_time )
    {
      first_time = 0;
      // seed the RNG on startup
      srand48( time(NULL) );
    }
 
  // no power consumed until we're subscribed
  this->SetWatts( 0 ); 
  
  // sensible position defaults
  stg_velocity_t vel;
  memset( &vel, 0, sizeof(vel));
  this->SetVelocity( &vel );
  
  this->SetBlobReturn( TRUE );
    
  // configure the position-specific stuff

  stg_position_cfg_t cfg;
  memset( &cfg, 0, sizeof(cfg));
  cfg.drive_mode = STG_POSITION_DRIVE_DEFAULT;
  this->SetCfg( &cfg, sizeof(cfg));
  
  stg_position_cmd_t cmd;
  memset( &cmd, 0, sizeof(cmd));  
  cmd.mode = STG_POSITION_CONTROL_DEFAULT;
  this->SetCmd( &cmd, sizeof(cmd));
  
  stg_position_data_t data;
  memset( &data, 0, sizeof(data));
  
  data.integration_error.x =  
    drand48() * STG_POSITION_INTEGRATION_ERROR_MAX_X - 
    STG_POSITION_INTEGRATION_ERROR_MAX_X/2.0;
  
  data.integration_error.y =  
    drand48() * STG_POSITION_INTEGRATION_ERROR_MAX_Y - 
    STG_POSITION_INTEGRATION_ERROR_MAX_Y/2.0;
  
  data.integration_error.a =  
    drand48() * STG_POSITION_INTEGRATION_ERROR_MAX_A - 
    STG_POSITION_INTEGRATION_ERROR_MAX_A/2.0;
  
  data.localization = STG_POSITION_LOCALIZATION_DEFAULT;
  
  this->SetData( &data, sizeof(data));
  
  //gui_position_init( mod );  
}

 
 StgModelPosition::~StgModelPosition( void )
 {
   // nothing to do 
 }

void StgModelPosition::Load( void )
{
  StgModel::Load();
  
  char* keyword = NULL;
  
  stg_position_data_t* data = (stg_position_data_t*)this->data;
  stg_position_cfg_t* cfg = (stg_position_cfg_t*)this->cfg;

  // load steering mode
  if( wf->PropertyExists( this->id, "drive" ) )
    {
      const char* mode_str =  
	wf->ReadString( this->id, "drive", NULL );
      
      if( mode_str )
	{
	  if( strcmp( mode_str, "diff" ) == 0 )
	    cfg->drive_mode = STG_POSITION_DRIVE_DIFFERENTIAL;
	  else if( strcmp( mode_str, "omni" ) == 0 )
	    cfg->drive_mode = STG_POSITION_DRIVE_OMNI;
	  else if( strcmp( mode_str, "car" ) == 0 )
	    cfg->drive_mode = STG_POSITION_DRIVE_CAR;
	  else
	    {
	      PRINT_ERR1( "invalid position drive mode specified: \"%s\" - should be one of: \"diff\", \"omni\" or \"car\". Using \"diff\" as default.", mode_str );	      
	    }

	  // TODO
	  // let people know we changed the cfg
	  //model_change( mod, &mod->cfg );
	}
    }      
  
  
  // load odometry if specified
  if( wf->PropertyExists( this->id, "odom" ) )
    {
      PRINT_WARN1( "the odom property is specified for model \"%s\","
		   " but this property is no longer available."
		   " Use localization_origin instead. See the position"
		   " entry in the manual or src/model_position.c for details.", 
		   this->token );
    }

  // set the starting pose as my initial odom position. This could be
  // overwritten below if the localization_origin property is
  // specified
  this->GetGlobalPose( &data->origin );

  keyword = "localization_origin"; 
  if( wf->PropertyExists( this->id, keyword ) )
    {  
      data->origin.x = wf->ReadTupleLength( id, keyword, 0, data->pose.x );
      data->origin.y = wf->ReadTupleLength( id, keyword, 1, data->pose.y );
      data->origin.a = wf->ReadTupleAngle( id,keyword, 2, data->pose.a );

      // compute our localization pose based on the origin and true pose
      stg_pose_t gpose;
      this->GetGlobalPose( &gpose );
      
      data->pose.a = NORMALIZE( gpose.a - data->origin.a );
      //double cosa = cos(data->pose.a);
      //double sina = sin(data->pose.a);
      double cosa = cos(data->origin.a);
      double sina = sin(data->origin.a);
      double dx = gpose.x - data->origin.x;
      double dy = gpose.y - data->origin.y; 
      data->pose.x = dx * cosa + dy * sina; 
      data->pose.y = dy * cosa - dx * sina;

      // zero position error: assume we know exactly where we are on startup
      memset( &data->pose_error, 0, sizeof(data->pose_error));      
    }

  // odometry model parameters
  if( wf->PropertyExists( id, "odom_error" ) )
    {
      data->integration_error.x = 
	wf->ReadTupleLength( id, "odom_error", 0, data->integration_error.x );
      data->integration_error.y = 
	wf->ReadTupleLength( id, "odom_error", 1, data->integration_error.y );
      data->integration_error.a 
	= wf->ReadTupleAngle( id, "odom_error", 2, data->integration_error.a );
    }

  // choose a localization model
  if( wf->PropertyExists( this->id, "localization" ) )
    {
      const char* loc_str =  
	wf->ReadString( id, "localization", NULL );
   
      if( loc_str )
	{
	  if( strcmp( loc_str, "gps" ) == 0 )
	    data->localization = STG_POSITION_LOCALIZATION_GPS;
	  else if( strcmp( loc_str, "odom" ) == 0 )
	    data->localization = STG_POSITION_LOCALIZATION_ODOM;
	  else
	    PRINT_ERR2( "unrecognized localization mode \"%s\" for model \"%s\"."
			" Valid choices are \"gps\" and \"odom\".", 
			loc_str, this->token );
	}
      else
	PRINT_ERR1( "no localization mode string specified for model \"%s\"", 
		    this->token );
    }
  
  // TODO
  // we've probably poked the localization data, so we must let people know
  //model_change( mod, &mod->data );
}

 void StgModelPosition::Update( void  )
{ 
  StgModel::Update();

//   PRINT_DEBUG1( "[%lu] position update", mod->world->sim_time );
  
//   stg_position_data_t* data = (stg_position_data_t*)mod->data;
//   stg_position_cmd_t* cmd = (stg_position_cmd_t*)mod->cmd;
//   stg_position_cfg_t* cfg = (stg_position_cfg_t*)mod->cfg;
  
//   stg_velocity_t* vel = &mod->velocity;

//   // stop by default
//   memset( vel, 0, sizeof(stg_velocity_t) );
  
//   if( mod->subs )   // no driving if noone is subscribed
//     {            
//       switch( cmd->mode )
// 	{
// 	case STG_POSITION_CONTROL_VELOCITY :
// 	  {
// 	    PRINT_DEBUG( "velocity control mode" );
// 	    PRINT_DEBUG4( "model %s command(%.2f %.2f %.2f)",
// 			  mod->token, 
// 			  mod->cmd.x, 
// 			  mod->cmd.y, 
// 			  mod->cmd.a );
	    

// 	    switch( cfg->drive_mode )
// 	      {
// 	      case STG_POSITION_DRIVE_DIFFERENTIAL:
// 		// differential-steering model, like a Pioneer
// 		vel->x = cmd->x;
// 		vel->y = 0;
// 		vel->a = cmd->a;
// 		break;
		
// 	      case STG_POSITION_DRIVE_OMNI:
// 		// direct steering model, like an omnidirectional robot
// 		vel->x = cmd->x;
// 		vel->y = cmd->y;
// 		vel->a = cmd->a;
// 		break;

// 	      case STG_POSITION_DRIVE_CAR:
// 		// car like steering model based on speed and turning angle
// 		vel->x = cmd->x * cos(cmd->a);
// 		vel->y = 0;
// 		vel->a = cmd->x * sin(cmd->a)/1.0; // here 1.0 is the wheel base, this should be a config option
// 		break;
		
// 	      default:
// 		PRINT_ERR1( "unknown steering mode %d", cfg->drive_mode );
// 	      }
// 	  } break;
	  
// 	case STG_POSITION_CONTROL_POSITION:
// 	  {
// 	    PRINT_DEBUG( "position control mode" );
	    
// 	    double x_error = cmd->x - data->pose.x;
// 	    double y_error = cmd->y - data->pose.y;
// 	    double a_error = NORMALIZE( cmd->a - data->pose.a );
	    
// 	    PRINT_DEBUG3( "errors: %.2f %.2f %.2f\n", x_error, y_error, a_error );
	    
// 	    // speed limits for controllers
// 	    // TODO - have these configurable
// 	    double max_speed_x = 0.4;
// 	    double max_speed_y = 0.4;
// 	    double max_speed_a = 1.0;	      
	    
// 	    switch( cfg->drive_mode )
// 	      {
// 	      case STG_POSITION_DRIVE_OMNI:
// 		{
// 		  // this is easy - we just reduce the errors in each axis
// 		  // independently with a proportional controller, speed
// 		  // limited
// 		  vel->x = MIN( x_error, max_speed_x );
// 		  vel->y = MIN( y_error, max_speed_y );
// 		  vel->a = MIN( a_error, max_speed_a );
// 		}
// 		break;
		
// 	      case STG_POSITION_DRIVE_DIFFERENTIAL:
// 		{
// 		  // axes can not be controlled independently. We have to
// 		  // turn towards the desired x,y position, drive there,
// 		  // then turn to face the desired angle.  this is a
// 		  // simple controller that works ok. Could easily be
// 		  // improved if anyone needs it better. Who really does
// 		  // position control anyhoo?
		  
// 		  // start out with no velocity
// 		  stg_velocity_t calc;
// 		  memset( &calc, 0, sizeof(calc));
		  
// 		  double close_enough = 0.02; // fudge factor
		  
// 		  // if we're at the right spot
// 		  if( fabs(x_error) < close_enough && fabs(y_error) < close_enough )
// 		    {
// 		      PRINT_DEBUG( "TURNING ON THE SPOT" );
// 		      // turn on the spot to minimize the error
// 		      calc.a = MIN( a_error, max_speed_a );
// 		      calc.a = MAX( a_error, -max_speed_a );
// 		    }
// 		  else
// 		    {
// 		      PRINT_DEBUG( "TURNING TO FACE THE GOAL POINT" );
// 		      // turn to face the goal point
// 		      double goal_angle = atan2( y_error, x_error );
// 		      double goal_distance = hypot( y_error, x_error );
		      
// 		      a_error = NORMALIZE( goal_angle - data->pose.a );
// 		      calc.a = MIN( a_error, max_speed_a );
// 		      calc.a = MAX( a_error, -max_speed_a );
		      
// 		      PRINT_DEBUG2( "steer errors: %.2f %.2f \n", a_error, goal_distance );
		      
// 		      // if we're pointing about the right direction, move
// 		      // forward
// 		      if( fabs(a_error) < M_PI/16 )
// 			{
// 			  PRINT_DEBUG( "DRIVING TOWARDS THE GOAL" );
// 			  calc.x = MIN( goal_distance, max_speed_x );
// 			}
// 		    }
		  
// 		  // now set the underlying velocities using the normal
// 		  // diff-steer model
// 		  vel->x = calc.x;
// 		  vel->y = 0;
// 		  vel->a = calc.a;
// 		}
// 		break;
		
// 	      default:
// 		PRINT_ERR1( "unknown steering mode %d", (int)cfg->drive_mode );
// 	      }
// 	  }
// 	  break;
	  
// 	default:
// 	  PRINT_ERR1( "unrecognized position command mode %d", cmd->mode );
// 	}
      
//       // simple model of power consumption
//       // mod->watts = STG_POSITION_WATTS + 
//       //fabs(vel->x) * STG_POSITION_WATTS_KGMS * mod->mass + 
//       //fabs(vel->y) * STG_POSITION_WATTS_KGMS * mod->mass + 
//       //fabs(vel->a) * STG_POSITION_WATTS_KGMS * mod->mass; 

//       //PRINT_DEBUG4( "model %s velocity (%.2f %.2f %.2f)",
//       //	    mod->token, 
//       //	    mod->velocity.x, 
//       //	    mod->velocity.y,
//       //	    mod->velocity.a );
      

//       // we've poked the velocity 
//       model_change( mod, &mod->velocity );
//     }
  
//   switch( data->localization )
//     {
//     case STG_POSITION_LOCALIZATION_GPS:
//       {
// 	// compute our localization pose based on the origin and true pose
// 	stg_pose_t gpose;
// 	stg_model_get_global_pose( mod, &gpose );
	
// 	data->pose.a = NORMALIZE( gpose.a - data->origin.a );
// 	//data->pose.a =0;// NORMALIZE( gpose.a - data->origin.a );
// 	double cosa = cos(data->origin.a);
// 	double sina = sin(data->origin.a);
// 	double dx = gpose.x - data->origin.x;
// 	double dy = gpose.y - data->origin.y; 
// 	data->pose.x = dx * cosa + dy * sina; 
// 	data->pose.y = dy * cosa - dx * sina;
// 	model_change( mod, &mod->data );
//       }
//       break;
      
//     case STG_POSITION_LOCALIZATION_ODOM:
//       {
// 	// integrate our velocities to get an 'odometry' position estimate.
// 	double dt = mod->world->sim_interval/1e3;
	
// 	data->pose.a = NORMALIZE( data->pose.a + (vel->a * dt) * (1.0 +data->integration_error.a) );
	
// 	double cosa = cos(data->pose.a);
// 	double sina = sin(data->pose.a);
// 	double dx = (vel->x * dt) * (1.0 + data->integration_error.x );
// 	double dy = (vel->y * dt) * (1.0 + data->integration_error.y );
	
// 	data->pose.x += dx * cosa + dy * sina; 
// 	data->pose.y -= dy * cosa - dx * sina;
// 	model_change( mod, &mod->data );
//       }
//       break;
      
//     default:
//       PRINT_ERR2( "unknown localization mode %d for model %s\n",
// 		  data->localization, mod->token );
//       break;
//     }
  
//   model_move_due_to_velocity( mod );
 
//   // we've probably poked the position data
//   //model_change( mod, &mod->pose );
 
}

 void StgModelPosition::Startup( void )
 {
   PRINT_DEBUG( "position startup" );
   
   this->SetWatts( STG_POSITION_WATTS );
   
   //stg_model_position_odom_reset( mod );   
 }

 void StgModelPosition::Shutdown( void )
 {
   PRINT_DEBUG( "position shutdown" );
   
   // safety features!
   stg_position_cmd_t cmd;
   memset( &cmd, 0, sizeof(cmd) ); 
   this->SetCmd( &cmd, sizeof(cmd) );
   
   stg_velocity_t vel;
   memset( &vel, 0, sizeof(vel));
   this->SetVelocity( &vel );
   
   this->SetWatts( 0 );
}

