///////////////////////////////////////////////////////////////////////////
//
// File: omnipositiondevice.cc
// Author: Andrew Howard
// Date: 19 Oct 2001
// Desc: Simulates an omni robot
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/models/omnipositiondevice.cc,v $
//  $Author: rtv $
//  $Revision: 1.5 $
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

#define ENABLE_RTK_TRACE 1

#include <math.h>
#include "world.hh"
#include "raytrace.hh"
#include "omnipositiondevice.hh"

///////////////////////////////////////////////////////////////////////////
// Constructor
COmniPositionDevice::COmniPositionDevice(LibraryItem* libit,CWorld *world, CEntity *parent)
  : CPlayerEntity( libit,world, parent )
{    
  // setup our Player interface for a POSITION device interface
  PositionInit();
  
  // set up our sensor response
  this->laser_return = LaserTransparent;
  this->sonar_return = true;
  this->obstacle_return = true;
  this->puck_return = true;

  // Set default shape and geometry
  this->shape = ShapeRect;
  this->size_x = 0.30;
  this->size_y = 0.30;

  this->com_vx = this->com_vy = this->com_va = 0;
  this->com_px = this->com_py = this->com_pa = 0;
  this->odo_px = this->odo_py = this->odo_pa = 0;

  // update this device VERY frequently
  m_interval = 0.01; 
  
  // assume robot is 20kg
  this->mass = 20.0;

  // default to velocity mode - the most common way to control a robot
  this->move_mode = VELOCITY_MODE;

  // x y th axes are independent
  this->enable_omnidrive = true;

  this->stall = false;
}

///////////////////////////////////////////////////////////////////////////
// Startup routine
//
bool COmniPositionDevice::Startup()
{
  if (!CPlayerEntity::Startup())
    return false;

  return true;
}

///////////////////////////////////////////////////////////////////////////
// Update the device
void COmniPositionDevice::Update( double sim_time )
{  
  // Do some default processing
  CPlayerEntity::Update(sim_time);

  // Dont do anything if its not time yet
  if (sim_time - m_last_update <  m_interval)
    return;
  m_last_update = sim_time;

  if (Subscribed() > 0)
  {
    // Process configuration requests
    UpdateConfig();

    PositionGetCommand( &this->com_px, &this->com_py, &this->com_pa, 
			&this->com_vx, &this->com_vy, &this->com_va );
    
    // update the robot's position
    if( this->motors_enabled )
      {
	if( this->move_mode == POSITION_MODE )
	  // compute some velocities that'll move us to the goal position
	  PositionControl();
	
	// and update our position based on the current velocities
	Move();
      }
    
    PositionPutData( this->odo_px, this->odo_py, this->odo_pa,
		     this->com_vx, this->com_vy,  this->com_va, 
		     this->stall );
  }
  else  
  {
    // the device is not subscribed,
    // so reset odometry to default settings.
    this->odo_px = this->odo_py = this->odo_pa = 0;
  }

  // Get our new pose
  double px, py, pa;
  GetGlobalPose(px, py, pa);

  // Remap ourselves if we have moved
  ReMap(px, py, pa);
}

  ///////////////////////////////////////////////////////////////////////////
  // Process configuration requests.
  void COmniPositionDevice::UpdateConfig()
  {
    int len;
    void *client;
    char buffer[PLAYER_MAX_REQREP_SIZE];

    while (true)
    {
      len = GetConfig(&client, buffer, sizeof(buffer));
      if (len <= 0)
        break;

      switch (buffer[0])
        {
        case PLAYER_POSITION_MOTOR_POWER_REQ:
          // motor state change request 
          // 1 = enable motors (default)
          // 0 = disable motors
	  switch( buffer[1] )
	    {
	    case 0: this->motors_enabled = false;
	      break;
	    case 1: this->motors_enabled = true;
	      break;
	    default: 
	      this->motors_enabled = false;
	      PRINT_WARN1("Unrecognized motor state %d in"
			  " PLAYER_POSITION_MOTOR_POWER_REQ "
			  "(expecting 1(on) or 0(off). "
			  "Switching off motors for safety.", 
			  buffer[1] ); 
	    }
	  if( !this->motors_enabled ) SetGlobalVel( 0,0,0 );
	  
          PutReply(client, PLAYER_MSGTYPE_RESP_ACK);
          break;
	  
        case PLAYER_POSITION_RESET_ODOM_REQ:
          // reset position to 0,0,0
          this->odo_px = this->odo_py = this->odo_pa = 0.0;
          PutReply(client, PLAYER_MSGTYPE_RESP_ACK);
          break;
	  
	case PLAYER_POSITION_SET_ODOM_REQ:
	  // set my odometry estimate to the values in the packet
	  PositionSetOdomReqUnpack( (player_position_set_odom_req_t*)buffer,
				    &this->odo_px, 
				    &this->odo_py, 
				    &this->odo_pa );
	     
	  PutReply(client, PLAYER_MSGTYPE_RESP_ACK);
          break;
	  
        case PLAYER_POSITION_GET_GEOM_REQ:
	  {
	    // Return the robot geometry (rotation offsets and size)
	    player_position_geom_t geom;	  
	    PositionGeomPack( &geom, 
			      this->origin_x, this->origin_y, 0,
			      this->size_x, this->size_y );
	    
  	  PutReply(client, PLAYER_MSGTYPE_RESP_ACK, NULL, &geom, sizeof(geom));
  	}
          break;

        case PLAYER_POSITION_POSITION_MODE_REQ:
  	// switch between velocity and position control
  	switch( buffer[1] )
  	  {
  	  case 0: this->move_mode = VELOCITY_MODE; 
  	    break;
  	  case 1: this->move_mode = POSITION_MODE; 
  	    break;
  	  default: 
  	    PRINT_WARN2("Unrecognized mode in"
  			" PLAYER_POSITION_POSITION_MODE_REQ (%d)."
			"Sticking with current mode (%s)", 
  			buffer[0], 
			this->move_mode ? "POSITION_MODE" : "VELOCITY_MODE" ); 
  	    break;
  	  }
          PutReply(client, PLAYER_MSGTYPE_RESP_ACK);
          break;

          // CONFIGS NOT IMPLEMENTED
	
        case PLAYER_POSITION_POSITION_PID_REQ:
  	PRINT_WARN( "PLAYER_POSITION_POSITION_PID_REQ has no effect" );
          PutReply(client, PLAYER_MSGTYPE_RESP_ACK);
          break;

        case PLAYER_POSITION_SPEED_PID_REQ:
  	PRINT_WARN( "PLAYER_POSITION_SPEED_PID_REQ has no effect" );
          PutReply(client, PLAYER_MSGTYPE_RESP_ACK);
          break;

        case PLAYER_POSITION_SPEED_PROF_REQ:
  	PRINT_WARN( "PLAYER_POSITION_SPEED_PROF_REQ has no effect" );
          PutReply(client, PLAYER_MSGTYPE_RESP_ACK);
          break;

        case PLAYER_POSITION_VELOCITY_MODE_REQ:
          // velocity control mode:
          //   0 = direct wheel velocity control (default)
          //   1 = separate translational and rotational control
  	PRINT_WARN( "PLAYER_POSITION_VELOCITY_MODE_REQ has no effect" );
          PutReply(client, PLAYER_MSGTYPE_RESP_ACK);
          break;

	    default:
	      PRINT_WARN1("got unknown config request \"%c\"\n", buffer[0]);
	      PutReply(client, PLAYER_MSGTYPE_RESP_NACK);
	      break;
	    }
    }
  }

///////////////////////////////////////////////////////////////////////////
// Compute the robots new pose
void COmniPositionDevice::Move()
{
  double step = m_world->m_sim_timestep;

  // Get the current robot pose
  //
  double px, py, pa;
  GetPose(px, py, pa);

  // Compute the new velocity
  // For now, just set to commanded velocity
  double vx = this->com_vx;
  double vy = this->com_vy;
  double va = this->com_va;
    
  // Compute movement deltas
  // This is a zero-th order approximation
  double dx = step * vx * cos(pa) - step * vy * sin(pa);
  double dy = step * vx * sin(pa) + step * vy * cos(pa);
  double da = step * va;
  
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
    // Uses a zero-th order approximation
    this->odo_px += step * vx;// * cos(pa) - step * vy * sin(pa);
    this->odo_py += step * vy;//step * vx * sin(pa) + step * vy * cos(pa);
    this->odo_pa += step * va;//step * va;
    this->odo_pa = fmod(this->odo_pa + TWOPI, TWOPI);

    this->stall = false;
  }
}

///////////////////////////////////////////////////////////////////////////
// Compute the robots new pose
void COmniPositionDevice::PositionControl()
{
  const double DISTANCE_THRESHOLD = 0.05;
  const double ANGLE_THRESHOLD = 0.3;

  // TODO - position control servoing
  // this doesn't work, but it shows the idea

  double step = m_world->m_sim_timestep;

  // Get the pose error
  //
  double error_x = this->com_px - this->odo_px;
  double error_y = this->com_py - this->odo_py;
  double error_a = this->com_pa - this->odo_pa;

  // handle the zero-crossing
  if( error_a > M_PI ) 
    error_a -= 2.0 * M_PI; 
  
  if( error_a < -M_PI )
    error_a += 2.0 * M_PI;
  
  
  printf( "Odo: %.2f %.2f %.2f\n", odo_px, odo_py, odo_pa );
  printf( "Errors: %.2f %.2f %.2f\n", error_x, error_y, error_a );

  // set the translation speeds
  if( this->enable_omnidrive )
    {
      // set speeds proportional to error
      this->com_vx = error_x;
      this->com_vy = error_y;
      this->com_va = error_a;
    }
  else // a little more complex with a diff steer robot
    {
      // find the angle to the goal
      double error_a_goal = atan2( error_y, error_x );

      // handle the zero-crossing
      if( error_a_goal > M_PI ) 
	error_a_goal -= 2.0 * M_PI; 
      
      if( error_a_goal < -M_PI )
	error_a_goal += 2.0 * M_PI;

      // find the distance from the goal
      double error_d_goal = hypot( error_y, error_x );

      // if we're at the goal, turn to face the right direction
      if( fabs(error_d_goal) < DISTANCE_THRESHOLD )
	{
	  this->com_va = 0.5 * error_a;
	  this->com_vx = 0.0;
	  this->com_vy = 0.0;
	}
      else // if we're pointing towards the goal, move towards it 
	if( fabs(error_a_goal) < ANGLE_THRESHOLD )
	  {
	    this->com_vx = 0.5 * error_d_goal;
	    this->com_vx = 0.0;
	    this->com_vy = 0.0;
	  }
	else // turn towards the goal
	  {
	    this->com_va = 0.5 * error_a_goal;
	    this->com_vx = 0.0;
	    this->com_vy = 0.0;
	  }
    }
}

#ifdef INCLUDE_RTK2

///////////////////////////////////////////////////////////////////////////
// Initialise the rtk gui
void COmniPositionDevice::RtkStartup()
{
  CPlayerEntity::RtkStartup();
  
  // add a 'nose line' indicating forward to the entity's normal
  // rectangle or circle. draw from the center of rotation to the front.
  rtk_fig_line( this->fig, 
		this->origin_x, this->origin_y, 
		this->size_x/2.0, this->origin_y );
}


#endif


