///////////////////////////////////////////////////////////////////////////
//
// File: omnipositiondevice.cc
// Author: Andrew Howard
// Date: 19 Oct 2001
// Desc: Simulates an omni robot
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/models/position.cc,v $
//  $Author: rtv $
//  $Revision: 1.1.2.1 $
//
// Usage:
//  (empty)
//
// Theory of operation:
//  (empty)
//
// Known bugs:
//  (empty)
//
// Possible enhancements:
//  (empty)
//
///////////////////////////////////////////////////////////////////////////

#define DEBUG

#include <math.h>
#include <assert.h>
#include "sio.h"
#include "raytrace.hh"
#include "position.hh"


#define THMAX(A,B) (A > B ? A : B)
#define THMIN(A,B) (A < B ? A : B)

///////////////////////////////////////////////////////////////////////////
// Constructor
CPositionModel::CPositionModel( int id, char* token, 
				char* color,  CEntity *parent)
  : CEntity( id, token, color, parent )
{    
  // set up our sensor response
  this->laser_return = LaserTransparent;
  this->sonar_return = true;
  this->obstacle_return = true;
  this->puck_return = true;
  this->idar_return = IDARReflect;

  // Set default shape and geometry
  this->size_x = 0.30;
  this->size_y = 0.30;

  // initial commands
  this->com_vx = this->com_vy = this->com_va = 0;
  this->com_px = this->com_py = this->com_pa = 0;
  this->odo_px = this->odo_py = this->odo_pa = 0;

  // update this device VERY frequently
  m_interval = 0.01; 
  
  // assume robot is 20kg
  this->mass = 20.0;

  // default to velocity mode - the most common way to control a robot
  this->control_mode = STG_POSITION_CONTROL_VELOCITY;
  this->steer_mode = STG_POSITION_STEER_DIFFERENTIAL;

  this->stall = false;
  this->power_on = true;
}



void CPositionModel::VelocityControl( void )
{
  switch( this->steer_mode )
    {
    case STG_POSITION_STEER_INDEPENDENT:
      // just set the speeds directly to their commanded values
      this->vx = this->com_vx;
      this->vy = this->com_vy;
      this->vth = this->com_va;
      break;
      
    case STG_POSITION_STEER_DIFFERENTIAL:
      // vy not allowed in differential mode
      this->vx = this->com_vx;
      this->vy = 0;
      this->vth = this->com_va;
      break;

    default:
      PRINT_WARN2( "position model %d unknown steer mode (%d)", 
		   this->stage_id, this->steer_mode );
      break;
  }   
}


///////////////////////////////////////////////////////////////////////////
// Update the device
int CPositionModel::Update()
{  
  // inherit the default processing
  CEntity::Update();
  
  // Dont do anything if its not time yet
  if ( CEntity::simtime - m_last_update <  m_interval)
    return 0;

  m_last_update = CEntity::simtime;
  
  //PRINT_WARN1( "updating position %d", this->stage_id );
  
  
  
  // update the robot's position
  if( this->power_on )
    {
      //PRINT_WARN1( "position %d is powered up and updating", this->stage_id );
      
      switch( this->control_mode )
	{
	case STG_POSITION_CONTROL_POSITION:
	  // compute some velocities that'll move us to the goal position
	  PositionControl();	
	  break;
	  
	case STG_POSITION_CONTROL_VELOCITY:
	  VelocityControl();
	  break;

	default:
	  PRINT_WARN2( "position model %d has unrecognized control mode %d",
		      this->stage_id, this->control_mode );
	  break;
	}
      
    }

  PRINT_DEBUG4( "position %d com_v %.2f,%.2f,%.2f",
		this->stage_id, com_vx, com_vy, com_va );
  
  PRINT_DEBUG4( "position %d v %.2f,%.2f,%.2f",
		this->stage_id, vx, vy, vth );
      
  
  stage_position_data_t posdata;
  posdata.x = this->odo_px;
  posdata.y = this->odo_py;
  posdata.a = this->odo_pa;
  posdata.xdot = this->vx;
  posdata.ydot = this->vy;
  posdata.adot = this->vth;
  
  //PRINT_DEBUG1( "position model %d exporting new estimated position", 
  //	this->stage_id );
  
  if( IsSubscribed( STG_PROP_ENTITY_DATA) ) 
    this->Property( -1, STG_PROP_ENTITY_DATA, 
		    &posdata, sizeof(posdata), NULL );
  else  
    {
      // the device is not subscribed,
      // so reset odometry to default settings.
      //this->odo_px = this->odo_py = this->odo_pa = 0;
    }
  
  return 0; //success
}

int CPositionModel::Property( int con, stage_prop_id_t property, 
			      void* value, size_t len, stage_buffer_t* reply )
{  
  switch( property )
    {
    case STG_PROP_ENTITY_COMMAND:
      if( value )
	{
	  assert( len == sizeof(stage_position_cmd_t) );
	  
	  stage_position_cmd_t* cmd = (stage_position_cmd_t*)value;
	  
	  // commanded pose
	  this->com_px = cmd->x;
	  this->com_px = cmd->y;
	  this->com_px = cmd->a;
	  
	  // commmand velocity
	  this->com_vx = cmd->xdot;
	  this->com_vy = cmd->ydot;
	  this->com_va = cmd->adot;
	}
      
      // reply is handled automagically by CEntity
      break;
      
    case STG_PROP_ENTITY_POWER:      
      if( value )
	this->power_on  = *(int*)value;
      break;
      
    case STG_PROP_POSITION_ORIGIN:
      // reset position to 0,0,0
      this->odo_px = this->odo_py = this->odo_pa = 0.0;
      
      if( value )
	PRINT_WARN( "position model ignoring data in ORIGIN request" );
      
      if( reply ) // return no data, just ACK
	SIOBufferProperty( reply, this->stage_id, STG_PROP_POSITION_ORIGIN,
			   NULL, 0, STG_ISREPLY);
      break;
      
    case STG_PROP_POSITION_ODOM:
      if( value )
	{
	  assert( len == 3 * sizeof(double) );
	  this->odo_px = ((double*)value)[0];
	  this->odo_py = ((double*)value)[1];
	  this->odo_pa = ((double*)value)[2];
	}
      
      if( reply )
	{
	  stage_pose_t odopose;	  
	  odopose.x = this->odo_px;
	  odopose.y = this->odo_py;
	  odopose.a = this->odo_pa;
	  
	  SIOBufferProperty( reply, this->stage_id, STG_PROP_POSITION_ODOM,
			     &odopose, sizeof(odopose), STG_ISREPLY);	  
	}
      break;
      
    case STG_PROP_POSITION_MODE:
      if( value )   // switch between velocity and position control
	{	  
	  switch( *(stage_position_control_mode_t*)value )
	    {
	    case STG_POSITION_CONTROL_VELOCITY: 
	      this->control_mode = STG_POSITION_CONTROL_VELOCITY; 
	      PRINT_DEBUG1( "set position model %d to VELOCITY control mode",
			    this->stage_id );
	      break;
	      
	    case STG_POSITION_CONTROL_POSITION :
	      this->control_mode = STG_POSITION_CONTROL_POSITION; 
	      PRINT_DEBUG1( "set position model %d to POSITION control mode ",
			    this->stage_id );
	      break;
	      
	    default: 
	      PRINT_WARN2("Unrecognized mode in"
			  " STG_PROP_POSITION_MODE (%d)."
			  "Sticking with current mode (%d)", 
			  *(int*)value, this->control_mode );
	      break;
	    }
	}
      
      if( reply ) // send the current mode
	SIOBufferProperty( reply, this->stage_id, STG_PROP_POSITION_MODE,
			   &control_mode, sizeof(control_mode), STG_ISREPLY);	  
      
      break;
      
    case  STG_PROP_POSITION_STEER:
      // velocity control mode:
      //   0 = direct wheel velocity control (default)
      //   1 = separate translational and rotational control
      if( value )
	switch( *(stage_position_steer_mode_t*)value )
	  {
	  case STG_POSITION_STEER_DIFFERENTIAL: 
	    this->steer_mode = STG_POSITION_STEER_DIFFERENTIAL; 
	    PRINT_DEBUG1( "set position model %d to DIFFERENTIAL steer mode",
			  this->stage_id );
	      break;
	    
	  case STG_POSITION_STEER_INDEPENDENT: 
	      PRINT_DEBUG1( "set position model %d to INDEPENDENT steer mode",
			    this->stage_id );
	    this->steer_mode = STG_POSITION_STEER_INDEPENDENT; 
	    break;
	    
	  default:
	    PRINT_WARN2("Unrecognized mode in"
			" STG_PROP_POSITION_STEER (%d)."
			"Sticking with current mode (%d)", 
			*(int*)value, this->steer_mode );
	    break;
	    
	  }
      if( reply )
	SIOBufferProperty( reply, this->stage_id, STG_PROP_POSITION_STEER,
			   &steer_mode, sizeof(steer_mode), STG_ISREPLY);	  
      
      
    default:
      break;
    }

  return CEntity::Property( con, property, value, len, reply );
}


/*
///////////////////////////////////////////////////////////////////////////
// Compute the robots new pose
void CPositionModel::Move()
{
  double step = CEntity::timestep;

  // Get the current robot pose
  //
  double px, py, pa;
  GetPose(px, py, pa);
  
  //fprintf( stderr, "%.2f %.2f %.2f   %.2f %.2f %.2f\n", 
  //   px -1, py -1, pa, odo_px, odo_py, odo_pa );
  
  // Compute the new velocity
  // For now, just set to commanded velocity
  double vx = this->com_vx;
  double vy = this->com_vy;
  double va = this->com_va;
  
  // Compute movement deltas
  // This is a zero-th order approximation
  double dx=0, dy=0, da=0;

  switch( this->steer_mode )
  {
    case STG_POSITION_STEER_INDEPENDENT: // omnidirectional - axes independent
      dx = step * vx * cos(pa) - step * vy * sin(pa);
      dy = step * vx * sin(pa) + step * vy * cos(pa);
      da = step * va;
      break;
      
    case STG_POSITION_STEER_DIFFERENTIAL: //differential steering - no vy allowed
      dx = step * vx * cos(pa);
      dy = step * vx * sin(pa);
      da = step * va;
      break;
      
    default:
      PRINT_WARN1( "unknown steer mode (%d)", this->steer_mode );
      break;
  }
  
  // compute a new pose
  double qx = px + dx;
  double qy = py + dy;
  double qa = pa + da;

  // Check for collisions
  // and accept the new pose if ok
  if (TestCollision(qx, qy, qa ) != NULL)
  {
    SetGlobalVel(0, 0, 0);
    this->stall = true;
  }
  else
  {
    // set pose now takes care of marking us dirty
    SetPose(qx, qy, qa);
    SetGlobalVel(vx, vy, va);
        
    // Compute the new odometric pose
    // we currently have PERFECT odometry. yum!
    this->odo_px += step * vx * cos(this->odo_pa) + step * vy * sin(this->odo_pa);
    this->odo_py += step * vx * sin(this->odo_pa) + step * vy * cos(this->odo_pa);
    this->odo_pa += step * va;
    
    this->stall = false;
  }

}
*/

///////////////////////////////////////////////////////////////////////////
// Generate sensible velocities to achieve the goal pose
void CPositionModel::PositionControl()
{
  PRINT_WARN( "doing position control" );

  const double DISTANCE_THRESHOLD = 0.02;
  const double ANGLE_THRESHOLD = 0.1;

  // normalize odo_pa to +-PI
  this->odo_pa = NORMALIZE(this->odo_pa);

  // Get the pose error
  double error_x = this->com_px - this->odo_px;
  double error_y = this->com_py - this->odo_py;
  double error_a = this->com_pa - this->odo_pa;

  // handle the zero-crossing
  error_a = NORMALIZE(error_a);
  
  //printf( "Odo: %.2f %.2f %.2f\n", odo_px, odo_py, odo_pa );
  //printf( "Errors: %.2f %.2f %.2f\n", error_x, error_y, error_a );


  // set the translation speeds
  switch( this->steer_mode )
    {
    case STG_POSITION_STEER_INDEPENDENT:
      // set speeds proportional to error
      this->com_vx = error_x;
      this->com_vy = error_y;
      this->com_va = error_a;
      break;
      
    case STG_POSITION_STEER_DIFFERENTIAL:
      // a little more complex with a diff steer robot - this
      // controller seems to work ok
      {
	// find the angle to the goal
	double error_a_goal = atan2( error_y, error_x ) - this->odo_pa;
	// find the distance from the goal
	double error_d_goal = hypot( error_y, error_x );
	
	// if the goal is behind us, we'll go backwards!
	if( error_a_goal > M_PI/2.0 || error_a_goal < -M_PI/2 )
	  {
	    //puts( "BACKWARDS" );
	    error_d_goal = -error_d_goal; // negative speed
	    error_a_goal = NORMALIZE( error_a_goal + M_PI );// invert heading
	  }
	//else
	//puts( "FORWARDS" );
	
	// if we're at the goal, turn to face the right direction
	if( fabs(error_d_goal) < DISTANCE_THRESHOLD )
	  {
	 //puts( "turning to the right heading" );
	    this->com_vx = error_d_goal;
	    this->com_vy = 0.0;
	    this->com_va = error_a;
	  }
	else // if we're pointing towards the goal, move towards it 
	  if( fabs(error_a_goal) < ANGLE_THRESHOLD )
	    {
	      //puts( "moving towards goal point" );
	      this->com_vx = error_d_goal;
	      this->com_vy = 0.0;
	      this->com_va = error_a_goal; // keep turning towards the goal
	    }
	  else // turn towards the goal
	    {
	      //puts( "turning towards goal point" );
	      this->com_va = error_a_goal;
	      this->com_vx = 0.0;
	      this->com_vy = 0.0;
	    }
      }
      break;
      
    default:
      PRINT_ERR2( "position model %d has unknown steer mode %d", 
		  this->stage_id, this->steer_mode );
      
    }
  
  // threshold speeds
  this->com_vx = THMIN(this->com_vx,  0.5);
  this->com_vx = THMAX(this->com_vx, -0.5);
  this->com_vy = THMIN(this->com_vy,  0.5);
  this->com_vy = THMAX(this->com_vy, -0.5);
  this->com_va = THMIN(this->com_va,  1.0);
  this->com_va = THMAX(this->com_va, -1.0);
}

#ifdef INCLUDE_RTK2

///////////////////////////////////////////////////////////////////////////
// Initialise the rtk gui
int CPositionModel::RtkStartup()
{
  if( CEntity::RtkStartup() == -1 )
    return -1;
  
  // add a 'nose line' indicating forward to the entity's normal
  // rectangle or circle. draw from the center of rotation to the front.
  rtk_fig_line( this->fig, 
		this->origin_x, this->origin_y, 
		this->size_x/2.0, this->origin_y );

  return 0;
}


#endif


