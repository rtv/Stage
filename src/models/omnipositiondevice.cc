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
//  $Revision: 1.4 $
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
  // set the Player IO sizes correctly for this type of Entity
  m_data_len = sizeof( player_position_data_t );
  m_command_len = sizeof( player_position_cmd_t );
  m_config_len = 1;
  m_reply_len = 1;
  
  m_player.code = PLAYER_POSITION_CODE;

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
    
    // Get the latest command
    if (GetCommand(&this->command, sizeof(this->command)) == sizeof(this->command))
      ParseCommandBuffer();
    
    // update the robot's position
    if( this->motors_enabled )
      {
	if( this->move_mode == POSITION_MODE )
	  // compute some velocities that'll move us to the goal position
	  PositionControl();
	
	// and update our position based on the current velocities
	Move();
      }
    
    // Prepare data packet
    ComposeData();     
    PutData( &this->data, sizeof(this->data)  );
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
	this->motors_enabled = buffer[1];
	if( !this->motors_enabled ) SetGlobalVel( 0,0,0 );

        PutReply(client, PLAYER_MSGTYPE_RESP_ACK);
        break;

      case PLAYER_POSITION_RESET_ODOM_REQ:
        // reset position to 0,0,0
        this->odo_px = this->odo_py = this->odo_pa = 0.0;
        PutReply(client, PLAYER_MSGTYPE_RESP_ACK);
        break;

      case PLAYER_POSITION_SET_ODOM_REQ:
	{
	  player_position_set_odom_req_t* set = 
	    (player_position_set_odom_req_t*)buffer;
	  
	  // set odometry position to the requested values
	  this->odo_px = ntohl(set->x) / 1000.0;
	  this->odo_py = ntohl(set->y) / 1000.0;
	  this->odo_pa = DTOR(ntohs(set->y));
	  
	  PutReply(client, PLAYER_MSGTYPE_RESP_ACK);
	}
        break;
	
      case PLAYER_POSITION_GET_GEOM_REQ:
	{
	  // Return the robot geometry (rotation offsets and size)
	  player_position_geom_t geom;
	  
	  geom.pose[0] = htons((short) (this->origin_x * 1000));
	  geom.pose[1] = htons((short) (this->origin_y * 1000));
	  geom.pose[2] = 0;
	  geom.size[0] = htons((short) (this->size_x * 1000));
	  geom.size[1] = htons((short) (this->size_y * 1000));
	  
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
	    PRINT_WARN1("Unrecognized mode in"
			" PLAYER_POSITION_POSITION_MODE_REQ (%d)", 
			buffer[0] ); 
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
    
  // Compute a new pose
  // This is a zero-th order approximation
  double qx = px + step * vx * cos(pa) - step * vy * sin(pa);
  double qy = py + step * vx * sin(pa) + step * vy * cos(pa);
  double qa = pa + step * va;

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
    this->odo_px += step * vx * cos(pa) - step * vy * sin(pa);
    this->odo_py += step * vx * sin(pa) + step * vy * cos(pa);
    this->odo_pa += pa + step * va;
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
  
  
  printf( "Errors: %.2f %.2f %.2f\n", error_x, error_y, error_a );

  // set the translation speeds
  if( this->enable_omnidrive )
    {
      // set speeds proportional to error
      this->com_vx = error_x;
      this->com_vy = error_y;
      //this->com_va = 0.1 * error_a;
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

///////////////////////////////////////////////////////////////////////////
// Parse the command buffer to extract commands
void COmniPositionDevice::ParseCommandBuffer()
{
  // commands for velocity control
  double vx = (int)ntohl(this->command.xspeed);
  double vy = (int)ntohl(this->command.yspeed);
  double va = (int)ntohl(this->command.yawspeed);
    
  // commands for position control
  double px = (int)ntohl(this->command.xpos);
  double py = (int)ntohl(this->command.ypos);
  double pa = (int)ntohl(this->command.yaw);

  // Store commanded speed
  // Linear is in m/s
  // Angular is in radians/sec
  this->com_vx = vx / 1000;
  this->com_vy = vy / 1000;
  this->com_va = DTOR(va);

  // Store commanded position
  // Linear is in m
  // Angular is in radians
  this->com_px = px / 1000;
  this->com_py = py / 1000;
  this->com_pa = DTOR(pa);

  //printf( "command: (%.2f %.2f %.2f) [%.2f %.2f %.2f]\n", 
  //  com_vx, com_vy, com_va,
  //  com_px, com_py, com_pa );
}


///////////////////////////////////////////////////////////////////////////
// Compose data to send back to client
void COmniPositionDevice::ComposeData()
{
  // Compute odometric pose
  // Convert mm and degrees (0 - 360)
  double px = this->odo_px * 1000.0;
  double py = this->odo_py * 1000.0;
  double pa = RTOD(this->odo_pa);

  // Construct the data packet
  // Basically just changes byte orders and some units
  this->data.xpos = htonl((int) px);
  this->data.ypos = htonl((int) py);
  this->data.yaw = htonl((unsigned short) pa);

  this->data.xspeed = htonl((unsigned short) (this->com_vx * 1000.0));
  this->data.yspeed = htonl((unsigned short) (this->com_vy * 1000.0));
  this->data.yawspeed = htonl((short) RTOD(this->com_va));  

  this->data.stall = (uint8_t)( this->stall ? 1 : 0 );
}



