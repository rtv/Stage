///////////////////////////////////////////////////////////////////////////
//
// File: model_laser.c
// Author: Richard Vaughan
// Date: 10 June 2004
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/model_position.c,v $
//  $Author: rtv $
//  $Revision: 1.21 $
//
///////////////////////////////////////////////////////////////////////////


#include <sys/time.h>
#include <math.h>

//#define DEBUG

#include "stage_internal.h"
#include "gui.h"

//extern rtk_fig_t* fig_debug_rays;

/** 
@defgroup model_position Position model 
The position model simulates a mobile robot base. It can drive in one
of two modes; either <i>differential</i> like a Pioneer robot, or
<i>omnidirectional</i>.

<h2>Worldfile properties</h2>

@par Summary and default values

@verbatim
position
(
  # position properties
  drive "diff"

  # model properties
)
@endverbatim

@par Details
- drive "diff" or "omni"
  - select differential-steer mode (like a Pioneer) or omnidirectional mode.

*/


void position_load( stg_model_t* mod )
{
  stg_position_cfg_t cfg;
  memset(&cfg,0,sizeof(cfg));
  
  const char* mode_str =  wf_read_string( mod->id, "drive", "diff" );
  
  if( strcmp( mode_str, "diff" ) == 0 )
    cfg.steer_mode = STG_POSITION_STEER_DIFFERENTIAL;
  else if( strcmp( mode_str, "omni" ) == 0 )
    cfg.steer_mode = STG_POSITION_STEER_INDEPENDENT;
  else
    {
      PRINT_ERR1( "invalid position drive mode specified: \"%s\" - should be one of: \"diff\", \"omni\". Using \"diff\" as default.", mode_str );
      
      cfg.steer_mode = STG_POSITION_STEER_DIFFERENTIAL;
    }
  stg_model_set_config( mod, &cfg, sizeof(cfg) ); 
  

  stg_pose_t odom;
  odom.x = wf_read_tuple_length(mod->id, "odom", 0, 0.0 );
  odom.y = wf_read_tuple_length(mod->id, "odom", 1, 0.0 );
  odom.a = wf_read_tuple_angle(mod->id, "odom", 2, 0.0 );
  stg_model_position_set_odom( mod, &odom );
}



int position_startup( stg_model_t* mod );
int position_shutdown( stg_model_t* mod );
int position_update( stg_model_t* mod );

stg_model_t* stg_position_create( stg_world_t* world, 
				  stg_model_t* parent, 
				  stg_id_t id, 
				  char* token )
{
  PRINT_DEBUG( "created position model" );
  
  stg_model_t* mod = stg_model_create_extended( world, 
						parent, 
						id,		
						STG_MODEL_POSITION, 
						token,
						sizeof(stg_model_position_t) );  
  // get the type-sepecific extension we added to the end of the model
  stg_model_position_t* pos = (stg_model_position_t*)mod->extend;
  
  // override the default methods
  mod->f_startup = position_startup;
  mod->f_shutdown = position_shutdown;
  mod->f_update = position_update;
  mod->f_load = position_load;

  // sensible position defaults
  stg_velocity_t vel;
  memset( &vel, 0, sizeof(vel));
  stg_model_set_velocity( mod, &vel );

  // init the command structure
  stg_position_cmd_t cmd;
  memset( &cmd, 0, sizeof(cmd));
  cmd.mode = STG_POSITION_CONTROL_VELOCITY; // vel control mode
  cmd.x = cmd.y = cmd.a = 0; // no speeds to start
  
  stg_model_set_command( mod, &cmd, sizeof(cmd));
  
  // set up a position-specific config structure
  stg_position_cfg_t cfg;
  memset(&cfg,0,sizeof(cfg));
  cfg.steer_mode =  STG_POSITION_STEER_DIFFERENTIAL;
  
  stg_model_set_config( mod, &cfg, sizeof(cfg) );

  stg_position_data_t data;
   memset( &data, 0, sizeof(data) );

  stg_model_set_data( mod, &data, sizeof(data) );

  return mod;
}

int position_update( stg_model_t* mod )
{ 
  // get the type-sepecific extension we added to the end of the model
  stg_model_position_t* pos = (stg_model_position_t*)mod->extend;
  
  //stg_model_position_t pp;
  //stg_model_position_t* pos = &pp;

  PRINT_DEBUG1( "[%lu] position update", mod->world->sim_time );

  if( mod->subs )   // no driving if noone is subscribed
    {      
      // prevent anyone changing this model while we work
      // pthread_mutex_lock( mod->mutex );

      assert( mod->cfg ); // position_init should have filled these
      assert( mod->cmd );
      
      // set the model's velocity here according to the position command
      
      stg_position_cmd_t cmd;
      assert( stg_model_get_command( mod, &cmd, sizeof(cmd)) == sizeof(cmd));
      
      stg_position_cfg_t cfg;
      assert( stg_model_get_config( mod, &cfg, sizeof(cfg)) == sizeof(cfg));
      
      //stg_position_cfg_t* cfg = (stg_position_cfg_t*)mod->cfg;
      
      //printf( "cmd mode %d x %.2f y %.2f a %.2f\n",
      //      cmd.mode, cmd.x, cmd.y, cmd.a );

      stg_velocity_t vel;
      memset( &vel, 0, sizeof(vel) );
      
      switch( cmd.mode )
	{
	case STG_POSITION_CONTROL_VELOCITY :
	  {
	    PRINT_DEBUG( "velocity control mode" );
	    PRINT_DEBUG4( "model %s command(%.2f %.2f %.2f)",
			  mod->token, 
			  cmd.x, 
			  cmd.y, 
			  cmd.a );
	    
	    switch( cfg.steer_mode )
	      {
	      case STG_POSITION_STEER_DIFFERENTIAL:
		// differential-steering model, like a Pioneer
		//vel.x = (cmd.x * cos(mod->pose.a) - cmd.y * sin(mod->pose.a));
		//vel.y = (cmd.x * sin(mod->pose.a) + cmd.y * cos(mod->pose.a));
		vel.x = cmd.x;
		vel.y = 0;
		vel.a = cmd.a;
		break;
		
	      case STG_POSITION_STEER_INDEPENDENT:
		// direct steering model, like an omnidirectional robot
		vel.x = cmd.x;
		vel.y = cmd.y;
		vel.a = cmd.a;
		break;
		
	      default:
		PRINT_ERR1( "unknown steering mode %d", cfg.steer_mode );
	      }
	  } break;
	  
	case STG_POSITION_CONTROL_POSITION:
	  {
	    PRINT_DEBUG( "position control mode" );
	    
	    double x_error = cmd.x - pos->odom.x;
	    double y_error = cmd.y - pos->odom.y;
	    double a_error = NORMALIZE( cmd.a - pos->odom.a );
	    
	    PRINT_DEBUG3( "errors: %.2f %.2f %.2f\n", x_error, y_error, a_error );
	    
	    // speed limits for controllers
	    // TODO - have these configurable
	    double max_speed_x = 0.5;
	    double max_speed_y = 0.5;
	    double max_speed_a = 2.0;	      
	    
	    switch( cfg.steer_mode )
	      {
	      case STG_POSITION_STEER_INDEPENDENT:
		{
		  // this is easy - we just reduce the errors in each axis
		  // independently with a proportional controller, speed
		  // limited
		  vel.x = MIN( x_error, max_speed_x );
		  vel.y = MIN( y_error, max_speed_y );
		  vel.a = MIN( a_error, max_speed_a );
		}
		break;
		
	      case STG_POSITION_STEER_DIFFERENTIAL:
		{
		  // axes can not be controlled independently. We have to
		  // turn towards the desired x,y position, drive there,
		  // then turn to face the desired angle.  this is a
		  // simple controller that works ok. Could easily be
		  // improved if anyone needs it better. Who really does
		  // position control anyhoo?
		  
		  // start out with no velocity
		  stg_velocity_t calc;
		  memset( &calc, 0, sizeof(calc));
		  
		  double close_enough = 0.02; // fudge factor
		  
		  // if we're at the right spot
		  if( fabs(x_error) < close_enough && fabs(y_error) < close_enough )
		    {
		      PRINT_DEBUG( "TURNING ON THE SPOT" );
		      // turn on the spot to minimize the error
		      calc.a = MIN( a_error, max_speed_a );
		      calc.a = MAX( a_error, -max_speed_a );
		    }
		  else
		    {
		      PRINT_DEBUG( "TURNING TO FACE THE GOAL POINT" );
		      // turn to face the goal point
		      double goal_angle = atan2( y_error, x_error );
		      double goal_distance = hypot( y_error, x_error );
		      
		      a_error = NORMALIZE( goal_angle - pos->odom.a );
		      calc.a = MIN( a_error, max_speed_a );
		      calc.a = MAX( a_error, -max_speed_a );
		      
		      PRINT_DEBUG2( "steer errors: %.2f %.2f \n", a_error, goal_distance );
		      
		      // if we're pointing about the right direction, move
		      // forward
		      if( fabs(a_error) < M_PI/16 )
			{
			  PRINT_DEBUG( "DRIVING TOWARDS THE GOAL" );
			  calc.x = MIN( goal_distance, max_speed_x );
			}
		    }
		  
		  // now set the underlying velocities using the normal
		  // diff-steer model
		  //vel.x = (calc.x * cos(mod->pose.a) - calc.y * sin(mod->pose.a));
		  //vel.y = (calc.x * sin(mod->pose.a) + calc.y * cos(mod->pose.a));
		  vel.x = calc.x;
		  vel.y = 0;
		  vel.a = calc.a;
		}
		break;
		
	      default:
		PRINT_ERR1( "unknown steering mode %d", cfg.steer_mode );
	      }
	  }
	  break;
	  
	default:
	  PRINT_ERR1( "unhandled position command mode %d", cmd.mode );
	}
      
      stg_model_set_velocity( mod, &vel );
      
      PRINT_DEBUG4( "model %s velocity (%.2f %.2f %.2f)",
		    mod->token, 
		    mod->velocity.x, 
		    mod->velocity.y,
		    mod->velocity.a );
      
      double interval = (double)mod->world->sim_interval / 1000.0;
      
      // set the data here
      stg_position_data_t data;
      memcpy( &data.pose, &pos->odom, sizeof(stg_pose_t));  
      memcpy( &data.velocity, &mod->velocity, sizeof(stg_velocity_t));
      
      data.stall = mod->stall;
      
      //PRINT_WARN3( "data pos %.2f %.2f %.2f",
      //       data.pose.x, data.pose.y, data.pose.a );
      
      // publish the data
      stg_model_set_data( mod, &data, sizeof(data));    

      //pthread_mutex_unlock( mod->mutex );
    }
  

  // store our postion before moving
  stg_pose_t before;
  stg_model_get_pose( mod, &before );

  // now  inherit the normal update - this does the actual moving
  _model_update( mod );

  // record the movement as odometry
  stg_pose_t after;
  stg_model_get_pose( mod, &after );
  
  double a = pos->odom.a;
  
  double dx = after.x - before.x;;
  double dy = after.y - before.y;
  double da = after.a - before.a;
  
  pos->odom.x -= dx * cos(pos->odom_origin.a) - dy * sin(pos->odom_origin.a);
  pos->odom.y -= dx * sin(pos->odom_origin.a) + dy * cos(pos->odom_origin.a);
  pos->odom.a += da;
  pos->odom.a = NORMALIZE(pos->odom.a);
  
  return 0; //ok
}

int position_startup( stg_model_t* mod )
{
  PRINT_DEBUG( "position startup" );

  // set the starting pose as my initial odom position
  stg_model_position_t* pos = (stg_model_position_t*)mod->extend;
  stg_model_get_pose( mod, &pos->odom_origin );

  return 0; // ok
}

int position_shutdown( stg_model_t* mod )
{
  PRINT_DEBUG( "position shutdown" );
  
  // safety feature!
  //position_init(mod);

  stg_velocity_t vel;
  memset( &vel, 0, sizeof(vel));
  stg_model_set_velocity( mod, &vel );


  return 0; // ok
}

void stg_model_position_set_odom( stg_model_t* mod, stg_pose_t* odom )
{
  // get the type-sepecific extension we added to the end of the model
  stg_model_position_t* pos = (stg_model_position_t*)mod->extend;
  memcpy( &pos->odom, odom, sizeof(stg_pose_t));
}


