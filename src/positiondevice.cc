///////////////////////////////////////////////////////////////////////////
//
// File: pioneermobiledevice.cc
// Author: Richard Vaughan, Andrew Howard
// Date: 5 Dec 2000
// Desc: Simulates the Pioneer robot base
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/positiondevice.cc,v $
//  $Author: gerkey $
//  $Revision: 1.21.2.1 $
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

//#define DEBUG

#include <math.h>
#include "world.hh"
#include "positiondevice.hh"


///////////////////////////////////////////////////////////////////////////
// Constructor
CPositionDevice::CPositionDevice(CWorld *world, CEntity *parent )
  : CEntity( world, parent )
{    
  // set the Player IO sizes correctly for this type of Entity
  m_data_len = sizeof( player_position_data_t );
  m_command_len = sizeof( player_position_cmd_t );
  //m_config_len = sizeof( player_position_config_t ); // configurable!
  m_config_len = 1; 
  m_reply_len = 1; 
  
  m_player_type = PLAYER_POSITION_CODE; // from player's messages.h
  m_stage_type = RectRobotType;

  SetColor(POSITION_COLOR);

  // set up our sensor response
  laser_return = LaserTransparent;
  sonar_return = true;
  obstacle_return = true;
  puck_return = true;
  idar_return = IDARTransparent;

  m_com_vr = m_com_vth = 0;
  m_odo_px = m_odo_py = m_odo_pth = 0;
  stall = 0;
  
  m_interval = 0.01; // update this device VERY frequently
  
  // assume robot is 20kg
  m_mass = 20.0;
  
  // Set the default shape and geometry
  // to correspond to a Pioneer2 DX
  this->shape = ShapeRect;
  this->size_x = 0.440;
  this->size_y = 0.380;
  this->origin_x = -0.04;
  this->origin_y = 0;
  
#ifdef INCLUDE_RTK
    m_mouse_radius = 0.400;
    m_draggable = true;
#endif
}


///////////////////////////////////////////////////////////////////////////
// Update the position of the robot base
void CPositionDevice::Update( double sim_time )
{
  CEntity::Update(sim_time);

  ASSERT(m_world != NULL);

  // if its time to recalculate state
  if( sim_time - m_last_update >  m_interval )
  {
    m_last_update = sim_time;
      
    if( Subscribed() > 0 )
    {
      // Get latest config
      player_position_config_t cfg;
      void* client;

      // sanity check that things are set up correctly
      //assert( sizeof(cfg) == m_config_len );

      if(GetConfig(&client, &cfg, sizeof(cfg)) > 0)
      {
	PRINT_DEBUG1( "checking config type %x\n", cfg[0] );
	
        switch( cfg.request ) // switch on the type of request
        {
          case PLAYER_POSITION_MOTOR_POWER_REQ:
            // motor state change request 
            // 1 = enable motors
            // 0 = disable motors
            // (default)
	    printf( "MOTOR POWER: %d\n", cfg.value ); 
	    // CONFIG NOT IMPLEMENTED
            PutReply(client, PLAYER_MSGTYPE_RESP_ACK, NULL, NULL, 0);
            break;

          case PLAYER_POSITION_VELOCITY_CONTROL_REQ:
            // velocity control mode:
            //   0 = direct wheel velocity control (default)
            //   1 = separate translational and rotational control
	    printf( "CONTROL MODE: %d\n", cfg.value ); 
	    // CONFIG NOT IMPLEMENTED
            PutReply(client, PLAYER_MSGTYPE_RESP_ACK, NULL, NULL, 0);
            break;

          case PLAYER_POSITION_RESET_ODOM_REQ:
            // reset position to 0,0,0: ignore value
	    puts( "RESET ODOMETRY"); 
            m_odo_px = m_odo_py = m_odo_pth = 0.0;
            PutReply(client, PLAYER_MSGTYPE_RESP_ACK, NULL, NULL, 0);
            break;

          default:
            printf("Position device got unknown config request \"%c\"\n",
                   cfg.request);
            PutReply(client, PLAYER_MSGTYPE_RESP_NACK, NULL, NULL, 0);
            break;
        }
      }

      // Get the latest command
      if( GetCommand( &m_command, sizeof(m_command)) == sizeof(m_command))
        ParseCommandBuffer();    // find out what to do    
      
      Move();      // do things

      // report the new state of things
      ComposeData();

      // generic device call
      PutData( &m_data, sizeof(m_data)  );     
    }
    else  
    {
      // the device is not subscribed,
      // reset to default settings.
      m_odo_px = m_odo_py = m_odo_pth = 0;
      // also set speeds to zero
      m_com_vr = m_com_vth = 0;
    }
  }
  
  double x, y, th;
  GetGlobalPose( x,y,th );

  // Redraw ourself it we have moved
  ReMap(x, y, th);
}


int CPositionDevice::Move()
{
  double step = m_world->m_sim_timestep;

  // Get the current robot pose
  double px, py, pth;
  GetPose(px, py, pth);

  // Compute a new pose
  // This is a zero-th order approximation
  double qx = px + m_com_vr * step * cos(pth);
  double qy = py + m_com_vr * step * sin(pth);
  double qth = pth + m_com_vth * step;

  // Check for collisions
  // and accept the new pose if ok
  if (TestCollision(qx, qy, qth ) != NULL)
  {
    this->stall = 1;
  }
  else
  {
    SetPose(qx, qy, qth);
    this->stall = 0;

    // if we moved, we mark ourselves dirty
    if( (px!=qx) || (py!=qy) || (pth!=qth) )
      MakeDirtyIfPixelChanged();
        
    // Compute the new odometric pose
    // Uses a first-order integration approximation
    //
    double dr = m_com_vr * step;
    double dth = m_com_vth * step;
    m_odo_px += dr * cos(m_odo_pth + dth / 2);
    m_odo_py += dr * sin(m_odo_pth + dth / 2);
    m_odo_pth += dth;

    // Normalise the odometric angle
    //
    m_odo_pth = fmod(m_odo_pth + TWOPI, TWOPI);
  }
  return true;
}


///////////////////////////////////////////////////////////////////////////
// Parse the command buffer to extract commands
//
void CPositionDevice::ParseCommandBuffer()
{
  double fv = (short) ntohs(m_command.speed);
  double fw = (short) ntohs(m_command.turnrate);

  // Store commanded speed
  // Linear is in m/s
  // Angular is in radians/sec
  m_com_vr = fv / 1000;
  m_com_vth = DTOR(fw);
}


///////////////////////////////////////////////////////////////////////////
// Compose data to send back to client
void CPositionDevice::ComposeData()
{
  // Compute odometric pose
  // Convert mm and degrees (0 - 360)
  double px = m_odo_px * 1000.0;
  double py = m_odo_py * 1000.0;
  double pth = RTOD(fmod(m_odo_pth + TWOPI, TWOPI));

  // Get actual global pose
  double gx, gy, gth;
  GetGlobalPose(gx, gy, gth);
    
  // normalized compass heading
  double compass = NORMALIZE(gth);
  if (gth < 0)
    gth += TWOPI;
  
  // Construct the data packet
  // Basically just changes byte orders and some units
  m_data.xpos = htonl((int) px);
  m_data.ypos = htonl((int) py);
  m_data.theta = htons((unsigned short) pth);

  m_data.speed = htons((unsigned short) (m_com_vr * 1000.0));
  m_data.turnrate = htons((short) RTOD(m_com_vth));  
  m_data.compass = htons((unsigned short)(RTOD(compass)));
  m_data.stalls = this->stall;
}

