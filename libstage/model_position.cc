///////////////////////////////////////////////////////////////////////////
//
// File: model_laser.c
// Author: Richard Vaughan
// Date: 10 June 2004
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/libstage/model_position.cc,v $
//  $Author: rtv $
//  $Revision: 1.5 $
//
///////////////////////////////////////////////////////////////////////////


#include <sys/time.h>

#include "stage.hh"
#include "worldfile.hh"
using namespace Stg;

/** 
    @ingroup model
    @defgroup model_position Position model 

    The position model simulates a
    mobile robot base. It can drive in one of two modes; either
    <i>differential</i>, i.e. able to control its speed and turn rate by
    driving left and roght wheels like a Pioneer robot, or
    <i>omnidirectional</i>, i.e. able to control each of its three axes
    independently.

    API: Stg::ModelPosition

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
    odom_error [0.03 0.03 0.00 0.05]

    # model properties
    )
    @endverbatim

    @par Note
    Since Stage-1.6.5 the odom property has been removed. Stage will generate a warning if odom is defined in your worldfile. See localization_origin instead.

    @par Details
 
    - drive "diff", "omni" or "car"\n
    select differential-steer model(like a Pioneer), omnidirectional mode or carlike (velocity and steering angle).
    - localization "gps" or "odom"\n
    if "gps" the position model reports its position with perfect accuracy. If "odom", a simple odometry model is used and position data drifts from the ground truth over time. The odometry model is parameterized by the odom_error property.
    - localization_origin [x y theta]
    - set the origin of the localization coordinate system. By default, this is copied from the model's initial pose, so the robot reports its position relative to the place it started out. Tip: If localization_origin is set to [0 0 0] and localization is "gps", the model will return its true global position. This is unrealistic, but useful if you want to abstract away the details of localization. Be prepared to justify the use of this mode in your research! 
    - odom_error [x y theta]
    - parameters for the odometry error model used when specifying localization "odom". Each value is the maximum proportion of error in intergrating x, y, and theta velocities to compute odometric position estimate. For each axis, if the the value specified here is E, the actual proportion is chosen at startup at random in the range -E/2 to +E/2. Note that due to rounding errors, setting these values to zero does NOT give you perfect localization - for that you need to choose localization "gps".
*/



const double STG_POSITION_WATTS_KGMS = 10.0; // current per kg per meter per second
const double STG_POSITION_WATTS = 1.0; // base cost of position device

// simple odometry error model parameters. the error is selected at
// random in the interval -MAX/2 to +MAX/2 at startup
const double STG_POSITION_INTEGRATION_ERROR_MAX_X = 0.03;
const double STG_POSITION_INTEGRATION_ERROR_MAX_Y = 0.03;
const double STG_POSITION_INTEGRATION_ERROR_MAX_A = 0.05;

const stg_position_control_mode_t POSITION_CONTROL_DEFAULT = STG_POSITION_CONTROL_VELOCITY;
const stg_position_localization_mode_t POSITION_LOCALIZATION_DEFAULT = STG_POSITION_LOCALIZATION_GPS;
const stg_position_drive_mode_t POSITION_DRIVE_DEFAULT  = STG_POSITION_DRIVE_DIFFERENTIAL;

ModelPosition::ModelPosition( World* world, 
			      Model* parent )
  : Model( world, parent, MODEL_TYPE_POSITION ),
    waypoints( NULL ),
    waypoint_count( 0 ),
	 wpvis(),
	 posevis()
{
  PRINT_DEBUG2( "Constructing ModelPosition %d (%s)\n", 
		id, typestr );
  
  // assert that Update() is reentrant for this derived model
  thread_safe = true;
	
  // no power consumed until we're subscribed
  this->SetWatts( 0 );
  
  // zero vel
  Velocity v; // initially zero
  this->SetVelocity( v );
  
  this->SetBlobReturn( TRUE );
  
  // configure the position-specific stuff
  
  // control
  memset( &goal, 0, sizeof(goal));
  drive_mode = POSITION_DRIVE_DEFAULT;
  control_mode = POSITION_CONTROL_DEFAULT;
  
  // localization
  localization_mode = POSITION_LOCALIZATION_DEFAULT;
  
  integration_error.x =  
    drand48() * STG_POSITION_INTEGRATION_ERROR_MAX_X - 
    STG_POSITION_INTEGRATION_ERROR_MAX_X/2.0;
  
  integration_error.y =  
    drand48() * STG_POSITION_INTEGRATION_ERROR_MAX_Y - 
    STG_POSITION_INTEGRATION_ERROR_MAX_Y/2.0;
  
  integration_error.a =  
    drand48() * STG_POSITION_INTEGRATION_ERROR_MAX_A - 
    STG_POSITION_INTEGRATION_ERROR_MAX_A/2.0;
  
  AddVisualizer( &wpvis, true );
  AddVisualizer( &posevis, false );
}


ModelPosition::~ModelPosition( void )
{
  // nothing to do 
}

void ModelPosition::Load( void )
{
  Model::Load();

  // load steering mode
  if( wf->PropertyExists( wf_entity, "drive" ) )
    {
      const char* mode_str =  
		  wf->ReadString( wf_entity, "drive", NULL );
		
		if( mode_str )
		  {
			 if( strcmp( mode_str, "diff" ) == 0 )
				drive_mode = STG_POSITION_DRIVE_DIFFERENTIAL;
			 else if( strcmp( mode_str, "omni" ) == 0 )
				drive_mode = STG_POSITION_DRIVE_OMNI;
			 else if( strcmp( mode_str, "car" ) == 0 )
				drive_mode = STG_POSITION_DRIVE_CAR;
			 else
				{
				  PRINT_ERR1( "invalid position drive mode specified: \"%s\" - should be one of: \"diff\", \"omni\" or \"car\". Using \"diff\" as default.", mode_str );	      
				}
		  }
    }      
  
  // load odometry if specified
  if( wf->PropertyExists( wf_entity, "odom" ) )
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
  est_origin = this->GetGlobalPose();

	if( wf->PropertyExists( wf_entity, "localization_origin" ) )
	{  
		est_origin.x = wf->ReadTupleLength( wf_entity, "localization_origin", 0, est_origin.x );
		est_origin.y = wf->ReadTupleLength( wf_entity, "localization_origin", 1, est_origin.y );
		est_origin.z = wf->ReadTupleLength( wf_entity, "localization_origin", 2, est_origin.z );
		est_origin.a = wf->ReadTupleAngle( wf_entity, "localization_origin", 3, est_origin.a );

      // compute our localization pose based on the origin and true pose
      Pose gpose = this->GetGlobalPose();

      est_pose.a = normalize( gpose.a - est_origin.a );
      double cosa = cos(est_origin.a);
      double sina = sin(est_origin.a);
      double dx = gpose.x - est_origin.x;
      double dy = gpose.y - est_origin.y;
      est_pose.x = dx * cosa + dy * sina;
      est_pose.y = dy * cosa - dx * sina;

      // zero position error: assume we know exactly where we are on startup
      memset( &est_pose_error, 0, sizeof(est_pose_error));
    }

  // odometry model parameters
  if( wf->PropertyExists( wf_entity, "odom_error" ) )
    {
      integration_error.x = 
	wf->ReadTupleLength( wf_entity, "odom_error", 0, integration_error.x );
      integration_error.y = 
	wf->ReadTupleLength( wf_entity, "odom_error", 1, integration_error.y );
      integration_error.z = 
	wf->ReadTupleLength( wf_entity, "odom_error", 2, integration_error.z );
      integration_error.a 
	= wf->ReadTupleAngle( wf_entity, "odom_error", 3, integration_error.a );
    }

  // choose a localization model
  if( wf->PropertyExists( wf_entity, "localization" ) )
    {
      const char* loc_str =  
	wf->ReadString( wf_entity, "localization", NULL );

      if( loc_str )
	{
	  if( strcmp( loc_str, "gps" ) == 0 )
	    localization_mode = STG_POSITION_LOCALIZATION_GPS;
	  else if( strcmp( loc_str, "odom" ) == 0 )
	    localization_mode = STG_POSITION_LOCALIZATION_ODOM;
	  else
	    PRINT_ERR2( "unrecognized localization mode \"%s\" for model \"%s\"."
			" Valid choices are \"gps\" and \"odom\".", 
			loc_str, this->token );
	}
      else
	PRINT_ERR1( "no localization mode string specified for model \"%s\"", 
		    this->token );
    }
}

void ModelPosition::Update( void  )
{ 
  PRINT_DEBUG1( "[%lu] position update", this->world->sim_time );

  // stop by default
  Velocity vel;
  memset( &vel, 0, sizeof(Velocity) );

  if( this->subs ) // no driving if noone is subscribed
    {            
      switch( control_mode )
	{
	case STG_POSITION_CONTROL_VELOCITY :
	  {
	    PRINT_DEBUG( "velocity control mode" );
	    PRINT_DEBUG4( "model %s command(%.2f %.2f %.2f)",
			  this->token, 
			  this->goal.x, 
			  this->goal.y, 
			  this->goal.a );

	    switch( drive_mode )
	      {
	      case STG_POSITION_DRIVE_DIFFERENTIAL:
		// differential-steering model, like a Pioneer
		vel.x = goal.x;
		vel.y = 0;
		vel.a = goal.a;
		break;

	      case STG_POSITION_DRIVE_OMNI:
		// direct steering model, like an omnidirectional robot
		vel.x = goal.x;
		vel.y = goal.y;
		vel.a = goal.a;
		break;

	      case STG_POSITION_DRIVE_CAR:
		// car like steering model based on speed and turning angle
		vel.x = goal.x * cos(goal.a);
		vel.y = 0;
		vel.a = goal.x * sin(goal.a)/1.0; // here 1.0 is the wheel base, this should be a config option
		break;

	      default:
		PRINT_ERR1( "unknown steering mode %d", drive_mode );
	      }
	  } break;

	case STG_POSITION_CONTROL_POSITION:
	  {
	    PRINT_DEBUG( "position control mode" );

	    double x_error = goal.x - est_pose.x;
	    double y_error = goal.y - est_pose.y;
	    double a_error = normalize( goal.a - est_pose.a );

	    PRINT_DEBUG3( "errors: %.2f %.2f %.2f\n", x_error, y_error, a_error );

	    // speed limits for controllers
	    // TODO - have these configurable
	    double max_speed_x = 0.4;
	    double max_speed_y = 0.4;
	    double max_speed_a = 1.0;	      

	    switch( drive_mode )
	      {
	      case STG_POSITION_DRIVE_OMNI:
		{
		  // this is easy - we just reduce the errors in each axis
		  // independently with a proportional controller, speed
		  // limited
		  vel.x = MIN( x_error, max_speed_x );
		  vel.y = MIN( y_error, max_speed_y );
		  vel.a = MIN( a_error, max_speed_a );
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
		  Velocity calc;
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

		      a_error = normalize( goal_angle - est_pose.a );
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
		  vel.x = calc.x;
		  vel.y = 0;
		  vel.a = calc.a;
		}
		break;

	      default:
		PRINT_ERR1( "unknown steering mode %d", (int)drive_mode );
	      }
	  }
	  break;

	default:
	  PRINT_ERR1( "unrecognized position command mode %d", control_mode );
	}
		
      // simple model of power consumption
      watts = STG_POSITION_WATTS + 
		  fabs(vel.x) * STG_POSITION_WATTS_KGMS * mass + 
		  fabs(vel.y) * STG_POSITION_WATTS_KGMS * mass + 
		  fabs(vel.a) * STG_POSITION_WATTS_KGMS * mass;
		
      //PRINT_DEBUG4( "model %s velocity (%.2f %.2f %.2f)",
      //	    this->token, 
      //	    this->velocity.x, 
      //	    this->velocity.y,
      //	    this->velocity.a );
		
      this->SetVelocity( vel );
    }

  switch( localization_mode )
    {
    case STG_POSITION_LOCALIZATION_GPS:
      {
	// compute our localization pose based on the origin and true pose
	Pose gpose = this->GetGlobalPose();

	est_pose.a = normalize( gpose.a - est_origin.a );
	double cosa = cos(est_origin.a);
	double sina = sin(est_origin.a);
	double dx = gpose.x - est_origin.x;
	double dy = gpose.y - est_origin.y;
	est_pose.x = dx * cosa + dy * sina;
	est_pose.y = dy * cosa - dx * sina;

      }
      break;

    case STG_POSITION_LOCALIZATION_ODOM:
      {
		  // integrate our velocities to get an 'odometry' position estimate.
		  double dt = this->world->GetSimInterval()/1e6;
		  
		  est_pose.a = normalize( est_pose.a + (vel.a * dt) * (1.0 +integration_error.a) );
		  
		  double cosa = cos(est_pose.a);
		  double sina = sin(est_pose.a);
		  double dx = (vel.x * dt) * (1.0 + integration_error.x );
		  double dy = (vel.y * dt) * (1.0 + integration_error.y );
		  
		  est_pose.x += dx * cosa + dy * sina;
		  est_pose.y -= dy * cosa - dx * sina;
      }
      break;
		
    default:
      PRINT_ERR2( "unknown localization mode %d for model %s\n",
		  localization_mode, this->token );
      break;
    }

  PRINT_DEBUG3( " READING POSITION: [ %.4f %.4f %.4f ]\n",
		est_pose.x, est_pose.y, est_pose.a );

  Model::Update();
}

void ModelPosition::Startup( void )
{
  Model::Startup();

  PRINT_DEBUG( "position startup" );

  this->SetWatts( STG_POSITION_WATTS );
}

void ModelPosition::Shutdown( void )
{
  PRINT_DEBUG( "position shutdown" );

  // safety features!
  bzero( &goal, sizeof(goal) );
  bzero( &velocity, sizeof(velocity) );

  this->SetWatts( 0 );

  Model::Shutdown();
}

void ModelPosition::Stop()
{
  SetSpeed( 0,0,0 );
}

void ModelPosition::SetSpeed( double x, double y, double a ) 
{ 
  //assert( ! isnan(x) );
  //assert( ! isnan(y) );
  //assert( ! isnan(a) );
  
  control_mode = STG_POSITION_CONTROL_VELOCITY;
  goal.x = x;
  goal.y = y;
  goal.z = 0;
  goal.a = a;
}  

void ModelPosition::SetXSpeed( double x )
{ 
  //assert( ! isnan(x) );
  control_mode = STG_POSITION_CONTROL_VELOCITY;
  goal.x = x;
}  


void ModelPosition::SetYSpeed( double y )
{ 
  //assert( ! isnan(y) );
  control_mode = STG_POSITION_CONTROL_VELOCITY;
  goal.y = y;
}  

void ModelPosition::SetZSpeed( double z )
{ 
  //assert( ! isnan(z) );
  control_mode = STG_POSITION_CONTROL_VELOCITY;
  goal.z = z;
}  

void ModelPosition::SetTurnSpeed( double a )
{ 
  //assert( ! isnan(a) );
  control_mode = STG_POSITION_CONTROL_VELOCITY;
  goal.a = a;
}  


void ModelPosition::SetSpeed( Velocity vel ) 
{ 
  //assert( ! isnan(vel.x) );
  //assert( ! isnan(vel.y) );
  //assert( ! isnan(vel.z) );
  //assert( ! isnan(vel.a) );

  control_mode = STG_POSITION_CONTROL_VELOCITY;
  goal.x = vel.x;
  goal.y = vel.y;
  goal.z = vel.z;
  goal.a = vel.a;
}

void ModelPosition::GoTo( double x, double y, double a ) 
{
  //assert( ! isnan(x) );
  //assert( ! isnan(y) );
  //assert( ! isnan(a) );

  control_mode = STG_POSITION_CONTROL_POSITION;
  goal.x = x;
  goal.y = y;
  goal.z = 0;
  goal.a = a;
}  

void ModelPosition::GoTo( Pose pose ) 
{
  control_mode = STG_POSITION_CONTROL_POSITION;
  goal = pose;
}  

/** 
    Set the current odometry estimate 
*/
void ModelPosition::SetOdom( Pose odom )
{
  est_pose = odom;

  // figure out where the implied origin is in global coords
  Pose gp = GetGlobalPose();

  double da = normalize( -odom.a + gp.a );
  double dx = -odom.x * cos(da) + odom.y * sin(da);
  double dy = -odom.y * cos(da) - odom.x * sin(da);

  // origin of our estimated pose
  est_origin.x = gp.x + dx;
  est_origin.y = gp.y + dy;
  est_origin.a = da;
} 

/** Set the waypoint array pointer. Returns the old pointer, in case you need to free/delete[] it */
Waypoint* ModelPosition::SetWaypoints( Waypoint* wps, uint32_t count )
{
  Waypoint* replaced = waypoints;
  
  waypoints = wps;
  waypoint_count = count;
  
  return replaced;
}

ModelPosition::PoseVis::PoseVis()
  : Visualizer( "Position coordinates", "show_position_coords" )
{}

void ModelPosition::PoseVis::Visualize( Model* mod, Camera* cam )
{
  ModelPosition* pos = dynamic_cast<ModelPosition*>(mod);
  
  // vizualize my estimated pose 
  glPushMatrix();
  
  // back into global coords
  Gl::pose_inverse_shift( pos->GetGlobalPose() );
 
  Gl::pose_shift( pos->est_origin );
  pos->PushColor( 1,0,0,1 ); // origin in red
  Gl::draw_origin( 0.5 );
  
  glEnable (GL_LINE_STIPPLE);
  glLineStipple (3, 0xAAAA);
  
  pos->PushColor( 1,0,0,0.5 ); 
  glBegin( GL_LINE_STRIP );
  glVertex2f( 0,0 );
  glVertex2f( pos->est_pose.x, 0 );
  glVertex2f( pos->est_pose.x, pos->est_pose.y );  
  glEnd();
  
  glDisable(GL_LINE_STIPPLE);
  
  char label[64];
  snprintf( label, 64, "x:%.3f", pos->est_pose.x );
  Gl::draw_string( pos->est_pose.x / 2.0, -0.5, 0, label );
  
  snprintf( label, 64, "y:%.3f", pos->est_pose.y );
  Gl::draw_string( pos->est_pose.x + 0.5 , pos->est_pose.y / 2.0, 0, (const char*)label );
  
  pos->PopColor();
  
  Gl::pose_shift( pos->est_pose );
  pos->PushColor( 0,1,0,1 ); // pose in green
  Gl::draw_origin( 0.5 );
  pos->PopColor();
  
  Gl::pose_shift( pos->geom.pose );
  pos->PushColor( 0,0,1,1 ); // offset in blue
  Gl::draw_origin( 0.5 );
  pos->PopColor();
  
  double r,g,b,a;
  stg_color_unpack( pos->color, &r, &g, &b, &a );
  pos->PushColor( r, g, b, 0.5 );
  
  glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
  pos->blockgroup.DrawFootPrint( pos->geom );
  pos->PopColor();
  
  glPopMatrix(); 
}


ModelPosition::WaypointVis::WaypointVis()
  : Visualizer( "Position waypoints", "show_position_waypoints" )
{}

void ModelPosition::WaypointVis::Visualize( Model* mod, Camera* cam )
{
  ModelPosition* pos = dynamic_cast<ModelPosition*>(mod);

  Waypoint* waypoints = pos->waypoints;
  unsigned int waypoint_count = pos->waypoint_count;

  if( (waypoints == NULL) || (waypoint_count < 1) )
	 return;

  glPointSize( 5 );
  glPushMatrix();
  pos->PushColor( pos->color );
  
  Gl::pose_inverse_shift( pos->pose );
  Gl::pose_shift( pos->est_origin );
  
  glTranslatef( 0,0,0.02 );
  
  // draw waypoints
  glLineWidth( 3 );
  for( unsigned int i=0; i < waypoint_count; i++ )
	 waypoints[i].Draw();
  glLineWidth( 1 );

  // draw lines connecting the waypoints
  if( waypoint_count > 1 )
    {
		pos->PushColor( 1,0,0,0.3 );
		glBegin( GL_LINES );
      
      for( unsigned int i=1; i < waypoint_count; i++ )
		  {
			 glVertex2f( waypoints[i].pose.x,  waypoints[i].pose.y );
			 glVertex2f( waypoints[i-1].pose.x,  waypoints[i-1].pose.y );
		  }
		
      glEnd();

		pos->PopColor();
    }
  
  pos->PopColor();
  glPopMatrix();
}
