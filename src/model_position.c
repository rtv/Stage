///////////////////////////////////////////////////////////////////////////
//
// File: model_laser.c
// Author: Richard Vaughan
// Date: 10 June 2004
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/model_position.c,v $
//  $Author: rtv $
//  $Revision: 1.41 $
//
///////////////////////////////////////////////////////////////////////////


#include <sys/time.h>
#include <math.h>

//#define DEBUG

#include "stage_internal.h"
#include "gui.h"

// move into header
void stg_model_position_odom_reset( stg_model_t* mod );
void stg_model_position_get_odom( stg_model_t* mod, stg_pose_t* odom );

//extern stg_rtk_fig_t* fig_debug_rays;

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
  drive "diff"
  odom [0 0 0]
)
@endverbatim

@par Details
- drive "diff" or "omni"
  - select differential-steer mode (like a Pioneer) or omnidirectional mode.
- odom [x y theta]
  - set the initial odometry value for this device. Tip: if you set this the same as the pose, the odometry will give you the true absolute pose of the position device. 

*/

const double STG_POSITION_WATTS_KGMS = 5.0; // cost per kg per meter per second
const double STG_POSITION_WATTS = 10.0; // base cost of position device

// simple odometry error model parameters. the error is selected at
// random in the interval -MAX/2 to +MAX/2 at startup
const double STG_POSITION_INTEGRATION_ERROR_MAX_X = 0.04;
const double STG_POSITION_INTEGRATION_ERROR_MAX_Y = 0.04;
const double STG_POSITION_INTEGRATION_ERROR_MAX_A = 0.06;

int position_startup( stg_model_t* mod );
int position_shutdown( stg_model_t* mod );
int position_update( stg_model_t* mod );
void position_load( stg_model_t* mod );
int position_render_data( stg_model_t* mod, char* name, void* data, size_t len, void* userp );
int position_unrender_data( stg_model_t* mod, char* name, void* data, size_t len, void* userp );

int position_init( stg_model_t* mod )
{
  PRINT_DEBUG( "created position model" );
  
  static int first_time = 1;

  if( first_time )
    {
      first_time = 0;
      // seed the RNG on startup
      srand48( time(NULL) );
    }

  // no power consumed until we're subscribed
  //mod->watts = 0.0; 

  // override the default methods
  mod->f_startup = position_startup;
  mod->f_shutdown = position_shutdown;
  mod->f_update = position_update;
  mod->f_load = position_load;

  // sensible position defaults

  stg_velocity_t vel;
  memset( &vel, 0, sizeof(vel));
  stg_model_set_property( mod, "velocity", &vel, sizeof(vel));
  
  stg_blob_return_t blb = 1;
  stg_model_set_property( mod, "blob_return", &blb, sizeof(blb));
  
  stg_position_drive_mode_t drive = STG_POSITION_DRIVE_DIFFERENTIAL;  
  stg_model_set_property( mod, "position_drive", &drive, sizeof(drive) );

  stg_position_stall_t stall = 0;
  stg_model_set_property( mod, "position_stall", &stall, sizeof(stall));
  
  stg_position_cmd_t cmd;
  memset( &cmd, 0, sizeof(cmd));
  stg_model_set_property( mod, "position_cmd", &cmd, sizeof(cmd));
  
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

  stg_model_set_property( mod, "position_data", &data, sizeof(data));
  
  stg_model_add_property_toggles( mod, "position_data",  
 				  position_render_data, // called when toggled on
 				  NULL, 
 				  position_unrender_data, // called when toggled off 
 				  NULL,  
 				  "position data", 
				  FALSE ); 
    
  
  return 0;
}

void position_load( stg_model_t* mod )
{
  
  // load steering mode
  if( wf_property_exists( mod->id, "drive" ) )
    {
      stg_position_drive_mode_t* now = 
	stg_model_get_property_fixed( mod, "position_drive", 
				     sizeof(stg_position_drive_mode_t));
      
      stg_position_drive_mode_t drive =
	now ? *now : STG_POSITION_DRIVE_DIFFERENTIAL;
      
      const char* mode_str =  
	wf_read_string( mod->id, "drive", NULL );
      
      if( mode_str )
	{
	  if( strcmp( mode_str, "diff" ) == 0 )
	    drive = STG_POSITION_DRIVE_DIFFERENTIAL;
	  else if( strcmp( mode_str, "omni" ) == 0 )
	    drive = STG_POSITION_DRIVE_OMNI;
	  else
	    {
	      PRINT_ERR1( "invalid position drive mode specified: \"%s\" - should be one of: \"diff\", \"omni\". Using \"diff\" as default.", mode_str );	      
	    }	 
	  stg_model_set_property( mod, "position_drive", &drive, sizeof(drive)); 
	}
    }      
  
  stg_position_data_t* data = 
    stg_model_get_property_fixed( mod, "position_data", sizeof(stg_position_data_t));
  assert( data );
  
  // load odometry if specified
  if( wf_property_exists( mod->id, "odom" ) )
    {
      data->pose.x = wf_read_tuple_length(mod->id, "odom", 0, data->pose.x );
      data->pose.y = wf_read_tuple_length(mod->id, "odom", 1, data->pose.y );
      data->pose.a = wf_read_tuple_angle(mod->id, "odom", 2, data->pose.a );
      
      data->origin.x = data->pose.x;
      data->origin.y = data->pose.y;
      data->origin.a = data->pose.a;
      
      // zero position error: we have been told exactly where we are
      memset( &data->pose_error, 0, sizeof(data->pose_error));      
    }


  if( wf_property_exists( mod->id, "odom_error" ) )
    {
      data->integration_error.x = wf_read_tuple_length(mod->id, "odom_error", 0, data->integration_error.x );
      data->integration_error.y = wf_read_tuple_length(mod->id, "odom_error", 1, data->integration_error.y );
      data->integration_error.a = wf_read_tuple_angle(mod->id, "odom_error", 2, data->integration_error.a );
    }

  // set the starting pose as my initial odom position
  stg_model_get_global_pose( mod, &data->origin );

  stg_model_property_refresh( mod, "position_data" );


}


int position_update( stg_model_t* mod )
{ 
      
  PRINT_DEBUG1( "[%lu] position update", mod->world->sim_time );
  
  stg_position_data_t* data = 
    stg_model_get_property_fixed( mod, "position_data", 
				  sizeof(stg_position_data_t));
  assert(data);
  
  stg_velocity_t* vel = 
    stg_model_get_property_fixed( mod, "velocity", 
				  sizeof(stg_velocity_t));
  assert(vel);
  
  // stop by default
  memset( vel, 0, sizeof(stg_velocity_t) );
  
  if( mod->subs )   // no driving if noone is subscribed
    {            
      stg_position_cmd_t *cmd = 
	stg_model_get_property_fixed( mod, "position_cmd", sizeof(stg_position_cmd_t));      
      assert(cmd);
      
      stg_position_drive_mode_t *drive = 
	stg_model_get_property_fixed( mod, "position_drive", sizeof(stg_position_drive_mode_t));
      assert(drive);
      
      //printf( "cmd mode %d x %.2f y %.2f a %.2f\n",
      //      cmd.mode, cmd.x, cmd.y, cmd.a );

      
      switch( cmd->mode )
	{
	case STG_POSITION_CONTROL_VELOCITY :
	  {
	    PRINT_DEBUG( "velocity control mode" );
	    PRINT_DEBUG4( "model %s command(%.2f %.2f %.2f)",
			  mod->token, 
			  cmd->x, 
			  cmd->y, 
			  cmd->a );
	    
	    switch( *drive )
	      {
	      case STG_POSITION_DRIVE_DIFFERENTIAL:
		// differential-steering model, like a Pioneer
		vel->x = cmd->x;
		vel->y = 0;
		vel->a = cmd->a;
		break;
		
	      case STG_POSITION_DRIVE_OMNI:
		// direct steering model, like an omnidirectional robot
		vel->x = cmd->x;
		vel->y = cmd->y;
		vel->a = cmd->a;
		break;
		
	      default:
		PRINT_ERR1( "unknown steering mode %d", *drive );
	      }
	  } break;
	  
	case STG_POSITION_CONTROL_POSITION:
	  {
	    PRINT_DEBUG( "position control mode" );
	    
	    double x_error = cmd->x - data->pose.x;
	    double y_error = cmd->y - data->pose.y;
	    double a_error = NORMALIZE( cmd->a - data->pose.a );
	    
	    PRINT_DEBUG3( "errors: %.2f %.2f %.2f\n", x_error, y_error, a_error );
	    
	    // speed limits for controllers
	    // TODO - have these configurable
	    double max_speed_x = 0.4;
	    double max_speed_y = 0.4;
	    double max_speed_a = 1.0;	      
	    
	    switch( *drive )
	      {
	      case STG_POSITION_DRIVE_OMNI:
		{
		  // this is easy - we just reduce the errors in each axis
		  // independently with a proportional controller, speed
		  // limited
		  vel->x = MIN( x_error, max_speed_x );
		  vel->y = MIN( y_error, max_speed_y );
		  vel->a = MIN( a_error, max_speed_a );
		}
		break;
		
	      case STG_POSITION_DRIVE_DIFFERENTIAL:
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
		      
		      a_error = NORMALIZE( goal_angle - data->pose.a );
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
		  //vel->x = (calc.x * cos(mod->pose.a) - calc.y * sin(mod->pose.a));
		  //vel->y = (calc.x * sin(mod->pose.a) + calc.y * cos(mod->pose.a));
		  vel->x = calc.x;
		  vel->y = 0;
		  vel->a = calc.a;
		}
		break;
		
	      default:
		PRINT_ERR1( "unknown steering mode %d", (int)drive );
	      }
	  }
	  break;
	  
	default:
	  PRINT_ERR1( "unrecognized position command mode %d", cmd->mode );
	}
      
      // simple model of power consumption
      // mod->watts = STG_POSITION_WATTS + 
      //fabs(vel->x) * STG_POSITION_WATTS_KGMS * mod->mass + 
      //fabs(vel->y) * STG_POSITION_WATTS_KGMS * mod->mass + 
      //fabs(vel->a) * STG_POSITION_WATTS_KGMS * mod->mass; 

      //PRINT_DEBUG4( "model %s velocity (%.2f %.2f %.2f)",
      //	    mod->token, 
      //	    mod->velocity.x, 
      //	    mod->velocity.y,
      //	    mod->velocity.a );
      

      stg_position_stall_t stall = 0;
      stg_model_set_property( mod, "position_stall", &stall, sizeof(stall));


      // we've poked the velocity - muts refresh it so others notice
      // the change
      stg_model_property_refresh( mod, "velocity" );
    }
  
  // now  inherit the normal update - this does the actual moving
  _model_update( mod );
  
  double dt = mod->world->sim_interval/1e3;

  data->pose.a = NORMALIZE( data->pose.a + (vel->a * dt) * (1.0 +data->integration_error.a) );
  
  double cosa = cos(data->pose.a);
  double sina = sin(data->pose.a);
  double dx = (vel->x * dt) * (1.0 + data->integration_error.x );
  double dy = (vel->y * dt) * (1.0 + data->integration_error.y );

  data->pose.x += dx * cosa + dy * sina; 
  data->pose.y -= dy * cosa - dx * sina;
  
  // we've poked the position data - must refresh 
  stg_model_property_refresh( mod, "position_data" );
 
  return 0; //ok
}

int position_startup( stg_model_t* mod )
{
  PRINT_DEBUG( "position startup" );

  //mod->watts = STG_POSITION_WATTS;

  
  //stg_model_position_odom_reset( mod );

  return 0; // ok
}

int position_shutdown( stg_model_t* mod )
{
  PRINT_DEBUG( "position shutdown" );
  
  // safety features!
  stg_position_cmd_t cmd;
  memset( &cmd, 0, sizeof(cmd) ); 
  stg_model_set_property( mod, "position_cmd", &cmd, sizeof(cmd));
   
  stg_velocity_t vel;
  memset( &vel, 0, sizeof(vel));
  stg_model_set_property( mod, "velocity", &vel, sizeof(vel) );
  
  return 0; // ok
}

/* // Set the origin of the odometric coordinate system in world coords */
/* void stg_model_position_set_odom_origin( stg_model_t* mod, stg_pose_t* org ) */
/* { */
/*   // get the position data */
/*   // get the position data */
/*   stg_model_position_t* pos = stg_model_get_position( mod ); */
/*   memcpy( &pos->odom_origin, org, sizeof(stg_pose_t)); */
  
/*   //printf( "Stage warning: the odom origin was set to (%.2f,%.2f,%.2f)\n", */
/*   //  pos->odom_origin.x, pos->odom_origin.y, pos->odom_origin.a ); */
/* } */

/* // set the current pose to be the origin of the odometry coordinate system */
/* void stg_model_position_odom_reset( stg_model_t* mod ) */
/* { */
/*   //printf( "Stage warning: odom was reset (zeroed)\n" ); */

/*   // set the current odom measurement to zero */
/*   // get the position data */
/*   stg_model_position_t* pos = stg_model_get_position( mod ); */
/*   memset( &pos->odom, 0, sizeof(pos->odom)); */
  
/*   // and set the odom origin is the current pose */
/*   stg_model_position_set_odom_origin( mod,  */
/* 				      stg_model_get_property_fixed( mod, "pose", sizeof(stg_pose_t)) ); */
/* } */


/* void stg_model_position_get_odom( stg_model_t* mod, stg_pose_t* odom ) */
/* { */
/*   stg_model_position_t* pos = stg_model_get_position( mod ); */
/*   // copy the odom pose straight in */
/*   memcpy( odom, &pos->odom, sizeof(stg_pose_t)); */
/* } */

/* void stg_model_position_set_odom( stg_model_t* mod, stg_pose_t* odom ) */
/* { */
/*   stg_model_position_t* pos = stg_model_get_position( mod ); */

/*   // copy the odom pose straight in */
/*   memcpy( &pos->odom, odom, sizeof(stg_pose_t)); */
  
/*   // calculate what the origin of this coord system must be  */
  
/*   stg_pose_t* pose =  */
/*     stg_model_get_property_fixed( mod, "pose", sizeof(stg_pose_t)); */
  
/*   double da = odom->a - pose->a; */
/*   double cosa = cos(da); */
/*   double sina = sin(da); */
  
/*   double xx = odom->x * cosa + odom->y * sina; */
/*   double yy = odom->y * cosa - odom->x * sina; */

/*   stg_pose_t o; */
/*   o.x = pose->x - xx; */
/*   o.y = pose->y - yy; */
/*   o.a = NORMALIZE( pose->a - odom->a );   */
/*   stg_model_position_set_odom_origin( mod, &o );  \ */
/* } */

int position_unrender_data( stg_model_t* mod, char* name, 
			    void* data, size_t len, void* userp )
{
  stg_model_fig_clear( mod, "position_data_fig" );
  return 1;
}

int position_render_data( stg_model_t* mod, char* name, 
			  void* data, size_t len, void* userp )
{
  stg_rtk_fig_t* fig = stg_model_get_fig( mod, "position_data_fig" );
  
  if( !fig )
    {
      fig = stg_model_fig_create( mod, "position_data_fig", NULL, STG_LAYER_POSITIONDATA );
      //stg_rtk_fig_color_rgb32( fig, 0x9999FF ); // pale blue
      
      stg_color_t* col = stg_model_get_property_fixed( mod, "color", sizeof(stg_color_t));
      assert(col);

      stg_rtk_fig_color_rgb32( fig, *col ); 
    }

  stg_rtk_fig_clear(fig);
	  
  if( mod->subs )
    {  
      stg_position_data_t* odom = (stg_position_data_t*)data;
      
      stg_velocity_t* vel = 
	stg_model_get_property_fixed( mod, "velocity", sizeof(stg_velocity_t));
      
      //stg_rtk_fig_origin( fig,  odom->pose.x, odom->pose.y, odom->.a );
      stg_rtk_fig_origin( fig,  odom->origin.x, odom->origin.y, odom->origin.a );
      
      stg_rtk_fig_rectangle(  fig, 0,0,0, 0.06, 0.06, 0 );
      stg_rtk_fig_line( fig, 0,0, odom->pose.x, 0);
      stg_rtk_fig_line( fig, odom->pose.x, 0, odom->pose.x, odom->pose.y );
      
      char buf[256];
      snprintf( buf, 255, "vel(%.3f,%.3f,%.1f)\npos(%.3f,%.3f,%.1f)", 
		vel->x, vel->y, vel->a,
		odom->pose.x, odom->pose.y, odom->pose.a ); 
      
      stg_rtk_fig_text( fig, odom->pose.x + 0.4, odom->pose.y + 0.2, 0, buf );

      // draw an outline of the position model
      stg_geom_t *geom = 
	stg_model_get_property_fixed( mod, "geom", sizeof(stg_geom_t));
      assert(geom);

      stg_rtk_fig_rectangle(  fig, 
			      odom->pose.x, odom->pose.y, odom->pose.a,
			      0.1, 0.1, 0 );

      stg_rtk_fig_arrow( fig, 
			 odom->pose.x, odom->pose.y, odom->pose.a, 
			 geom->size.x/2.0, geom->size.y/2.0 );

      //stg_pose_t gpose;
      //stg_model_get_global_pose( mod, &gpose );
      //stg_rtk_fig_line( fig, gpose.x, gpose.y, odom->pose.x, odom->pose.y );
      
    }

  return 0;
}
