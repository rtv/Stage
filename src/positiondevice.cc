/*
 *  Stage : a multi-robot simulator.
 *  Copyright (C) 2001, 2002 Richard Vaughan, Andrew Howard and Brian Gerkey.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */
/*
 * Desc: Simulates a differential mobile robot.
 * Author: Andrew Howard, Richard Vaughan
 * Date: 5 Dec 2000
 * CVS info: $Id: positiondevice.cc,v 1.34 2002-09-07 02:05:25 rtv Exp $
 */

//#define DEBUG

#include <math.h>
#include "world.hh"
#include "positiondevice.hh"

CEntity position_bootstrap( "position", 
			    PositionType, 
			    (void*)&CPositionDevice::Creator );

///////////////////////////////////////////////////////////////////////////
// Constructor
CPositionDevice::CPositionDevice(CWorld *world, CEntity *parent)
  : CPlayerEntity( world, parent )
{    
  // set the Player IO sizes correctly for this type of Entity
  m_data_len = sizeof( player_position_data_t );
  m_command_len = sizeof( player_position_cmd_t );
  m_config_len = 1; 
  m_reply_len = 1; 
  
  m_player.code = PLAYER_POSITION_CODE;
  this->stage_type = PositionType;
  this->color = ::LookupColor(POSITION_COLOR);

  // set up our sensor response
  laser_return = LaserTransparent;
  sonar_return = true;
  obstacle_return = true;
  puck_return = true;
  idar_return = IDARTransparent;

  this->com_vr = this->com_vth = 0;
  this->odo_px = this->odo_py = this->odo_pth = 0;
  this->stall = 0;

  // update this device VERY frequently
  m_interval = 0.01; 
  
  // assume robot is 20kg
  this->mass = 20.0;
  
  // Set the default shape and geometry
  // to correspond to a Pioneer2 DX
  this->shape = ShapeRect;
  this->size_x = 0.440;
  this->size_y = 0.380;
  
  // took this out - now use the 'offset [X Y]' worldfile setting - rtv.
  // pioneer.inc now defines a pioneer as a positiondevice with a 4cm offset
  //this->origin_x = -0.04; 

  this->origin_x = 0;
  this->origin_y = 0;
}


///////////////////////////////////////////////////////////////////////////
// Update the position of the robot base
void CPositionDevice::Update( double sim_time )
{
  CPlayerEntity::Update(sim_time);

  ASSERT(m_world != NULL);

  // if its time to recalculate state
  if( sim_time - m_last_update >  m_interval )
  {
    m_last_update = sim_time;
      
    if( Subscribed() > 0 )
    {
      // Process configuration requests
      UpdateConfig();

      // Get the latest command
      UpdateCommand();

      // do things
      Move();

      // report the new state of things
      UpdateData();
    }
    else  
    {
      // the device is not subscribed,
      // reset to default settings.
      // also set speeds to zero
      this->odo_px = this->odo_py = this->odo_pth = 0;
      this->com_vr = this->com_vth = 0;
    }
  }

  // Redraw ourself it we have moved
  double x, y, th;
  GetGlobalPose(x, y, th);
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
  double qx = px + this->com_vr * step * cos(pth);
  double qy = py + this->com_vr * step * sin(pth);
  double qth = pth + this->com_vth * step;

  // Check for collisions
  // and accept the new pose if ok
  if (TestCollision(qx, qy, qth ) != NULL)
  {
    this->stall = 1;
  }
  else
  {
    // set pose now takes care of marking us dirty
    SetPose(qx, qy, qth);
    this->stall = 0;

    // Compute the new odometric pose
    // Uses a first-order integration approximation
    //
    double dr = this->com_vr * step;
    double dth = this->com_vth * step;
    this->odo_px += dr * cos(this->odo_pth + dth / 2);
    this->odo_py += dr * sin(this->odo_pth + dth / 2);
    this->odo_pth += dth;

    // Normalise the odometric angle
    this->odo_pth = fmod(this->odo_pth + TWOPI, TWOPI);
  }
  return true;
}


///////////////////////////////////////////////////////////////////////////
// Process configuration requests.
void CPositionDevice::UpdateConfig()
{
  int len;
  void *client;
  char buffer[PLAYER_MAX_REQREP_SIZE];
  player_position_geom_t geom;

  while (true)
  {
    len = GetConfig(&client, buffer, sizeof(buffer));
    if (len <= 0)
      break;

    switch (buffer[0])
    {
      case PLAYER_POSITION_MOTOR_POWER_REQ:
        // motor state change request 
        // 1 = enable motors
        // 0 = disable motors
        // (default)
        // CONFIG NOT IMPLEMENTED
        PutReply(client, PLAYER_MSGTYPE_RESP_ACK);
        break;

      case PLAYER_POSITION_VELOCITY_MODE_REQ:
        // velocity control mode:
        //   0 = direct wheel velocity control (default)
        //   1 = separate translational and rotational control
        // CONFIG NOT IMPLEMENTED
        PutReply(client, PLAYER_MSGTYPE_RESP_ACK);
        break;

      case PLAYER_POSITION_RESET_ODOM_REQ:
        // reset position to 0,0,0: ignore value
        this->odo_px = this->odo_py = this->odo_pth = 0.0;
        PutReply(client, PLAYER_MSGTYPE_RESP_ACK);
        break;

      case PLAYER_POSITION_GET_GEOM_REQ:
        // Return the robot geometry
        geom.pose[0] = htons((short) (this->origin_x * 1000));
        geom.pose[1] = htons((short) (this->origin_y * 1000));
        geom.pose[2] = 0;
        geom.size[0] = htons((short) (this->size_x * 1000));
        geom.size[1] = htons((short) (this->size_y * 1000));
        PutReply(client, PLAYER_MSGTYPE_RESP_ACK, NULL, &geom, sizeof(geom));
        break;

      default:
        PRINT_WARN1("got unknown config request \"%c\"\n", buffer[0]);
        PutReply(client, PLAYER_MSGTYPE_RESP_NACK);
        break;
    }
  }
}


///////////////////////////////////////////////////////////////////////////
// Parse the command buffer to extract commands
void CPositionDevice::UpdateCommand()
{
  if (GetCommand(&this->cmd, sizeof(this->cmd)) == sizeof(this->cmd))
  {
    double fv = (short) ntohs(this->cmd.xspeed);
    double fw = (short) ntohs(this->cmd.yawspeed);

    // Store commanded speed
    // Linear is in m/s
    // Angular is in radians/sec
    this->com_vr = fv / 1000;
    this->com_vth = DTOR(fw);

    // Set our x,y,th velocity components so that other entities (like pucks)
    // can get at them.
    SetGlobalVel(this->com_vr * cos(com_vth),this->com_vr * sin(com_vth), 0.0);
  }
}


///////////////////////////////////////////////////////////////////////////
// Compose data to send back to client
void CPositionDevice::UpdateData()
{
  // Compute odometric pose
  // Convert mm and degrees (0 - 360)
  double px = this->odo_px * 1000.0;
  double py = this->odo_py * 1000.0;
  double pth = RTOD(fmod(this->odo_pth + TWOPI, TWOPI));

  // Get actual global pose
  double gx, gy, gth;
  GetGlobalPose(gx, gy, gth);
    
  // normalized compass heading
  double compass = NORMALIZE(gth);
  if (compass < 0)
    compass += TWOPI;
  
  // Construct the data packet
  // Basically just changes byte orders and some units
  this->data.xpos = htonl((int) px);
  this->data.ypos = htonl((int) py);
  this->data.yaw = htons((unsigned short) pth);

  this->data.xspeed = htons((unsigned short) (this->com_vr * 1000.0));
  this->data.yawspeed = htons((short) RTOD(this->com_vth));  
  //this->data.compass = htons((unsigned short)(RTOD(compass)));
  this->data.stall = this->stall;

  PutData(&this->data, sizeof(this->data));     
}



#ifdef INCLUDE_RTK2

///////////////////////////////////////////////////////////////////////////
// Initialise the rtk gui
void CPositionDevice::RtkStartup()
{
  CPlayerEntity::RtkStartup();
  
  // add a 'nose line' indicating forward to the entity's normal
  // rectangle or circle. draw from the center of rotation to the front.
  rtk_fig_line( this->fig, 
		this->origin_x, this->origin_y, 
		this->size_x/2.0, this->origin_y );
}


#endif
