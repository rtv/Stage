///////////////////////////////////////////////////////////////////////////
//
// File: omnipositiondevice.cc
// Author: Andrew Howard
// Date: 19 Oct 2001
// Desc: Simulates an omni robot
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/omnipositiondevice.cc,v $
//  $Author: rtv $
//  $Revision: 1.14 $
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


// register this device type with the Library
CEntity omnipos_bootstrap( string("omnipos"), 
			   OmniPositionType, 
			   (void*)&COmniPositionDevice::Creator ); 

///////////////////////////////////////////////////////////////////////////
// Constructor
COmniPositionDevice::COmniPositionDevice(CWorld *world, CEntity *parent)
  : CPlayerEntity( world, parent )
{    
  // set the Player IO sizes correctly for this type of Entity
  m_data_len = sizeof( player_position_data_t );
  m_command_len = sizeof( player_position_cmd_t );
  m_config_len = 0;
  m_reply_len = 0;
  
  m_player.code = PLAYER_POSITION_CODE;
  this->stage_type = OmniPositionType;

  // set up our sensor response
  this->laser_return = LaserTransparent;
  this->sonar_return = true;
  this->obstacle_return = true;
  this->puck_return = true;

  // Set default shape and geometry
  this->shape = ShapeCircle;
  this->size_x = 0.30;
  this->size_y = 0.30;

  this->com_vx = this->com_vy = this->com_va = 0;
  this->odo_px = this->odo_py = this->odo_pa = 0;

  // update this device VERY frequently
  m_interval = 0.01; 
  
  // assume robot is 20kg
  this->mass = 20.0;
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
    // Get the latest command
    if (GetCommand(&this->command, sizeof(this->command)) == sizeof(this->command))
      ParseCommandBuffer();
        
    // Move the robot
    Move();
        
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
    this->stall = 1;
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
        
    this->stall = 0;
  }
}


///////////////////////////////////////////////////////////////////////////
// Parse the command buffer to extract commands
void COmniPositionDevice::ParseCommandBuffer()
{
  double vx = (short) ntohs(this->command.xspeed);
  double vy = (short) ntohs(this->command.yspeed);
  double va = (short) ntohs(this->command.yawspeed);
    
  // Store commanded speed
  // Linear is in m/s
  // Angular is in radians/sec
  this->com_vx = vx / 1000;
  this->com_vy = vy / 1000;
  this->com_va = DTOR(va);
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
  this->data.yaw = htons((unsigned short) pa);

  this->data.xspeed = htons((unsigned short) (this->com_vx * 1000.0));
  this->data.yspeed = htons((unsigned short) (this->com_vy * 1000.0));
  this->data.yawspeed = htons((short) RTOD(this->com_va));  
  //this->data.compass = 0;
  this->data.stall = this->stall;
}


