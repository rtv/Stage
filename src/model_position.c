///////////////////////////////////////////////////////////////////////////
//
// File: model_laser.c
// Author: Richard Vaughan
// Date: 10 June 2004
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/model_position.c,v $
//  $Author: gerkey $
//  $Revision: 1.60 $
//
///////////////////////////////////////////////////////////////////////////


#include <sys/time.h>
#include <math.h>
#include <stdlib.h>

//#define DEBUG

#include "stage_internal.h"
#include "gui.h"

// move into header
void stg_model_position_odom_reset( stg_model_t* mod );
void stg_model_position_get_odom( stg_model_t* mod, stg_pose_t* odom );

// HACK - BPG
stg_model_t* stg_model_test_collision_at_pose( stg_model_t* mod, 
                                               stg_pose_t* pose, 
                                               double* hitx, double* hity );

//extern stg_rtk_fig_t* fig_debug_rays;

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
void stg_model_position_set_odom( stg_model_t* mod, stg_pose_t* odom )
{
  assert(mod->data);
  stg_position_data_t* data = (stg_position_data_t*)mod->data;
  memcpy( &data->pose, odom, sizeof(stg_pose_t));

  // figure out where the implied origin is in global coords
  stg_pose_t gp;
  stg_model_get_global_pose( mod, &gp );
			
  double da = -odom->a + gp.a;
  double dx = -odom->x * cos(da) + odom->y * sin(da);
  double dy = -odom->y * cos(da) - odom->x * sin(da);

  stg_pose_t origin;
  origin.x = gp.x + dx;
  origin.y = gp.y + dy;
  origin.a = da;

  memcpy( &data->origin, &origin, sizeof(stg_pose_t));
  
  model_change( mod, &mod->data );
} 

/* Internal */

const double STG_POSITION_WATTS_KGMS = 5.0; // cost per kg per meter per second
const double STG_POSITION_WATTS = 2.0; // base cost of position device

// simple odometry error model parameters. the error is selected at
// random in the interval -MAX/2 to +MAX/2 at startup
const double STG_POSITION_INTEGRATION_ERROR_MAX_X = 0.03;
const double STG_POSITION_INTEGRATION_ERROR_MAX_Y = 0.03;
const double STG_POSITION_INTEGRATION_ERROR_MAX_A = 0.05;

int position_startup( stg_model_t* mod );
int position_shutdown( stg_model_t* mod );
int position_update( stg_model_t* mod );
void position_load( stg_model_t* mod );
int position_render_data( stg_model_t* mod, void* userp );
int position_unrender_data( stg_model_t* mod, void* userp );
int position_render_text( stg_model_t* mod, void* userp );
int position_unrender_text( stg_model_t* mod, void* userp );

void position_init( stg_model_t* mod )
{
  static int first_time = 1;
  if( first_time )
    {
      first_time = 0;
      // seed the RNG on startup
      srand48( time(NULL) );
    }
 
  // no power consumed until we're subscribed
  mod->watts = 0.0; 

  // override the default methods
  mod->f_startup = position_startup;
  mod->f_shutdown = position_shutdown;
  mod->f_update = position_update;
  mod->f_load = position_load;

  // sensible position defaults

  stg_velocity_t vel;
  memset( &vel, 0, sizeof(vel));
  stg_model_set_velocity( mod, &vel );
  
  stg_model_set_blob_return( mod, TRUE );
  
  
  // configure the position-specific stuff

  stg_position_cfg_t cfg;
  memset( &cfg, 0, sizeof(cfg));
  cfg.drive_mode = STG_POSITION_DRIVE_DEFAULT;
  stg_model_set_cfg( mod, &cfg, sizeof(cfg));
  
  stg_position_cmd_t cmd;
  memset( &cmd, 0, sizeof(cmd));  
  cmd.mode = STG_POSITION_CONTROL_DEFAULT;
  stg_model_set_cmd( mod, &cmd, sizeof(cmd));
  
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
  
  stg_model_set_data( mod, &data, sizeof(data));
  
  stg_model_add_property_toggles( mod, 
				  &mod->data,
 				  position_render_data, // called when toggled on
 				  NULL,
 				  position_unrender_data, // called when toggled off
 				  NULL,
				  "positionlines",
 				  "position lines",
				  FALSE );

  stg_model_add_property_toggles( mod, 
				  &mod->data,
 				  position_render_text, // called when toggled on
 				  NULL,
 				  position_unrender_text, // called when toggled off
 				  NULL,
				  "positiontext",
 				  "position text",
				  FALSE );
}


void position_load( stg_model_t* mod )
{
//  return;

  char* keyword = NULL;
  
  stg_position_data_t* data = (stg_position_data_t*)mod->data;
  stg_position_cfg_t* cfg = (stg_position_cfg_t*)mod->cfg;

  // load steering mode
  if( wf_property_exists( mod->id, "drive" ) )
    {
      const char* mode_str =  
	wf_read_string( mod->id, "drive", NULL );
      
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

	  // let people know we changed the cfg
	  model_change( mod, &mod->cfg );
	}
    }      
  
  
  // load odometry if specified
  if( wf_property_exists( mod->id, "odom" ) )
    {
      PRINT_WARN1( "the odom property is specified for model \"%s\","
		   " but this property is no longer available."
		   " Use localization_origin instead. See the position"
		   " entry in the manual or src/model_position.c for details.", 
		   mod->token );
    }

  // set the starting pose as my initial odom position. This could be
  // overwritten below if the localization_origin property is
  // specified
  stg_model_get_global_pose( mod, &data->origin );

  keyword = "localization_origin"; 
  if( wf_property_exists( mod->id, keyword ) )
    {  
      data->origin.x = wf_read_tuple_length(mod->id, keyword, 0, data->pose.x );
      data->origin.y = wf_read_tuple_length(mod->id, keyword, 1, data->pose.y );
      data->origin.a = wf_read_tuple_angle(mod->id, keyword, 2, data->pose.a );

      // compute our localization pose based on the origin and true pose
      stg_pose_t gpose;
      stg_model_get_global_pose( mod, &gpose );
      
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
  if( wf_property_exists( mod->id, "odom_error" ) )
    {
      data->integration_error.x = 
	wf_read_tuple_length(mod->id, "odom_error", 0, data->integration_error.x );
      data->integration_error.y = 
	wf_read_tuple_length(mod->id, "odom_error", 1, data->integration_error.y );
      data->integration_error.a 
	= wf_read_tuple_angle(mod->id, "odom_error", 2, data->integration_error.a );
    }

  // choose a localization model
  if( wf_property_exists( mod->id, "localization" ) )
    {
      const char* loc_str =  
	wf_read_string( mod->id, "localization", NULL );
   
      if( loc_str )
	{
	  if( strcmp( loc_str, "gps" ) == 0 )
	    data->localization = STG_POSITION_LOCALIZATION_GPS;
	  else if( strcmp( loc_str, "odom" ) == 0 )
	    data->localization = STG_POSITION_LOCALIZATION_ODOM;
	  else
	    PRINT_ERR2( "unrecognized localization mode \"%s\" for model \"%s\"."
			" Valid choices are \"gps\" and \"odom\".", 
			loc_str, mod->token );
	}
      else
	PRINT_ERR1( "no localization mode string specified for model \"%s\"", 
		    mod->token );
    }
  
  // we've probably poked the localization data, so we must let people know
  model_change( mod, &mod->data );
}


int position_update( stg_model_t* mod )
{ 

  //return 0;
      
  PRINT_DEBUG1( "[%lu] position update", mod->world->sim_time );
  
  stg_position_data_t* data = (stg_position_data_t*)mod->data;
  stg_position_cmd_t* cmd = (stg_position_cmd_t*)mod->cmd;
  stg_position_cfg_t* cfg = (stg_position_cfg_t*)mod->cfg;
  
  stg_velocity_t* vel = &mod->velocity;

  // stop by default
  memset( vel, 0, sizeof(stg_velocity_t) );
  
  if( mod->subs )   // no driving if noone is subscribed
    {            
      switch( cmd->mode )
	{
	case STG_POSITION_CONTROL_VELOCITY :
	  {
	    PRINT_DEBUG( "velocity control mode" );
	    PRINT_DEBUG4( "model %s command(%.2f %.2f %.2f)",
			  mod->token, 
			  mod->cmd.x, 
			  mod->cmd.y, 
			  mod->cmd.a );
	    

	    switch( cfg->drive_mode )
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

	      case STG_POSITION_DRIVE_CAR:
		// car like steering model based on speed and turning angle
		vel->x = cmd->x * cos(cmd->a);
		vel->y = 0;
		vel->a = cmd->x * sin(cmd->a)/1.0; // here 1.0 is the wheel base, this should be a config option
		break;
		
	      default:
		PRINT_ERR1( "unknown steering mode %d", cfg->drive_mode );
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
	    
	    switch( cfg->drive_mode )
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
		  vel->x = calc.x;
		  vel->y = 0;
		  vel->a = calc.a;
		}
		break;
		
	      default:
		PRINT_ERR1( "unknown steering mode %d", (int)cfg->drive_mode );
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
      

      // we've poked the velocity 
      model_change( mod, &mod->velocity );
    }
  
  // now  inherit the normal update - this does the actual moving
  _model_update( mod );
  
  switch( data->localization )
    {
    case STG_POSITION_LOCALIZATION_GPS:
      {
	// compute our localization pose based on the origin and true pose
	stg_pose_t gpose;
	stg_model_get_global_pose( mod, &gpose );
	
	data->pose.a = NORMALIZE( gpose.a - data->origin.a );
	//data->pose.a =0;// NORMALIZE( gpose.a - data->origin.a );
	double cosa = cos(data->origin.a);
	double sina = sin(data->origin.a);
	double dx = gpose.x - data->origin.x;
	double dy = gpose.y - data->origin.y; 
	data->pose.x = dx * cosa + dy * sina; 
	data->pose.y = dy * cosa - dx * sina;
	model_change( mod, &mod->data );
      }
      break;
      
    case STG_POSITION_LOCALIZATION_ODOM:
      {
	// integrate our velocities to get an 'odometry' position estimate.
	double dt = mod->world->sim_interval/1e3;
	
	data->pose.a = NORMALIZE( data->pose.a + (vel->a * dt) * (1.0 +data->integration_error.a) );
	
	double cosa = cos(data->pose.a);
	double sina = sin(data->pose.a);
	double dx = (vel->x * dt) * (1.0 + data->integration_error.x );
	double dy = (vel->y * dt) * (1.0 + data->integration_error.y );
	
	data->pose.x += dx * cosa + dy * sina; 
	data->pose.y -= dy * cosa - dx * sina;
	model_change( mod, &mod->data );
      }
      break;
      
    default:
      PRINT_ERR2( "unknown localization mode %d for model %s\n",
		  data->localization, mod->token );
      break;
    }
  
  // we've probably poked the position data
  model_change( mod, &mod->pose );
 
  return 0; //ok
}

int position_startup( stg_model_t* mod )
{
  PRINT_DEBUG( "position startup" );

  stg_model_set_watts( mod, STG_POSITION_WATTS );

  //stg_model_position_odom_reset( mod );

  return 0; // ok
}

int position_shutdown( stg_model_t* mod )
{
  PRINT_DEBUG( "position shutdown" );
  
  // safety features!
  stg_position_cmd_t cmd;
  memset( &cmd, 0, sizeof(cmd) ); 
  stg_model_set_cmd( mod, &cmd, sizeof(cmd) );
  
  stg_velocity_t vel;
  memset( &vel, 0, sizeof(vel));
  stg_model_set_velocity( mod, &vel );
  
  stg_model_set_watts( mod, 0 );

  return 0; // ok
}

int position_unrender_data( stg_model_t* mod, void* userp )
{
  stg_model_fig_clear( mod, "position_data_fig" );
  return 1;
}

int position_render_data( stg_model_t* mod, void* enabled )
{
  stg_rtk_fig_t* fig = stg_model_get_fig( mod, "position_data_fig" );
  
  if( !fig )
    {
      fig = stg_model_fig_create( mod, "position_data_fig", 
				  NULL, STG_LAYER_POSITIONDATA );      
      stg_rtk_fig_color_rgb32( fig, mod->color ); 
    }

  stg_rtk_fig_clear(fig);
	  
  if( mod->subs )
    {  
      stg_position_data_t* odom = (stg_position_data_t*)mod->data;      
      stg_velocity_t* vel = &mod->velocity;
      stg_geom_t *geom = &mod->geom;

      //printf( "odom pose [%.2f %.2f %.2f] origin [%.2f %.2f %.2f]\n",
      //      odom->pose.x,
      //      odom->pose.y,
      //      odom->pose.a,
      //      odom->origin.x,
      //      odom->origin.y,
      //      odom->origin.a );

      // draw the coordinate system origin
      stg_rtk_fig_origin( fig,  odom->origin.x, odom->origin.y, odom->origin.a );      

      stg_rtk_fig_rectangle(  fig, 0,0,0, geom->size.x,geom->size.y, 0 );
      stg_rtk_fig_arrow( fig, 0,0,0, 
			 geom->size.x/2.0, geom->size.y/2.0 );      

      // connect the origin to the estimated location
      stg_rtk_fig_line( fig, 0,0, odom->pose.x, 0);
      stg_rtk_fig_line( fig, odom->pose.x, 0, odom->pose.x, odom->pose.y );
      
      // draw an outline of the position model at it's estimated location
      stg_rtk_fig_rectangle(  fig, 
			      odom->pose.x, odom->pose.y, odom->pose.a,
			      geom->size.x, geom->size.y, 0 );
      stg_rtk_fig_arrow( fig, 
			 odom->pose.x, odom->pose.y, odom->pose.a, 
			 geom->size.x/2.0, geom->size.y/2.0 );        

/*       char buf[256]; */
/*       snprintf( buf, 255, "%s[%.2f, %.2f, %.1f]\nv[%.2f,%.2f,%.1f] %s\n",  */
/* 		odom->localization == STG_POSITION_LOCALIZATION_GPS ? "GPS" : "odom", */
/* 		odom->pose.x, odom->pose.y, odom->pose.a, */
/* 		vel->x, vel->y, vel->a, */
/* 		mod->stall ? "STALL" : "" ); */
      
/*       stg_rtk_fig_text( fig, odom->pose.x + 0.4, odom->pose.y + 0.2, 0, buf ); */

    }

  return 0;
}

int position_unrender_text( stg_model_t* mod, void* userp )
{
  stg_model_fig_clear( mod, "position_text_fig" );
  return 1;
}

int position_render_text( stg_model_t* mod, void* enabled )
{
  stg_rtk_fig_t* fig = stg_model_get_fig( mod, "position_text_fig" );
  
  if( !fig )
    {
      fig = stg_model_fig_create( mod, "position_text_fig", 
				  NULL, STG_LAYER_POSITIONDATA );      
      stg_rtk_fig_color_rgb32( fig, mod->color ); 
    }

  stg_rtk_fig_clear(fig);
	  
  if( mod->subs )
    {  
      stg_position_data_t* odom = (stg_position_data_t*)mod->data;      
      stg_velocity_t* vel = &mod->velocity;

      char buf[256];
      snprintf( buf, 255, "%s[%.2f, %.2f, %.1f]\nv[%.2f,%.2f,%.1f] %s\n", 
		odom->localization == STG_POSITION_LOCALIZATION_GPS ? "GPS" : "odom",
		odom->pose.x, odom->pose.y, odom->pose.a,
		vel->x, vel->y, vel->a,
		mod->stall ? "STALL" : "" );
      
      stg_rtk_fig_text( fig, odom->pose.x + 0.4, odom->pose.y + 0.2, 0, buf );

    }

  return 0;
}
