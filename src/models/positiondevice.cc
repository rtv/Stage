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
 * CVS info: $Id: positiondevice.cc,v 1.2.2.5.2.2 2004-05-31 23:36:33 gerkey Exp $
 */

//#define DEBUG

#include <math.h>
#include "player.h"
#include "world.hh"
#include "positiondevice.hh"

// MACROS for packing Player buffers (borrowed from libplayerpacket)

// convert meters to Player mm in NBO in various sizes
#define MM_32(A)  (htonl((int32_t)(A * 1000.0)))
#define MM_16(A)  (htons((int16_t)(A * 1000.0)))
#define MM_U16(A) (htons((uint16_t)(A * 1000.0)))

// convert mm of various sizes in NBO to local meters
#define M_32(A)  ((int)ntohl((int32_t)A) / 1000.0)
#define M_16(A)  ((int)ntohs((int16_t)A) / 1000.0)
#define M_U16(A) ((unsigned short)ntohs((uint16_t)A) / 1000.0)

// convert local radians to Player degrees in various sizes
#define Deg_32(A) (htonl((int32_t)(RTOD(A))))
#define Deg_16(A) (htons((int16_t)(RTOD(A))))
#define Deg_U16(A) (htons((uint16_t)(RTOD(A))))

// convert Player degrees in various sizes to local radians
#define Rad_32(A) (DTOR((int)ntohl((int32_t)A)))
#define Rad_16(A) (DTOR((short)ntohs((int16_t)A)))
#define Rad_U16(A) (DTOR((unsigned short)ntohs((uint16_t)A)))


///////////////////////////////////////////////////////////////////////////
// Constructor
CPositionDevice::CPositionDevice(  LibraryItem *libit, 
				   CWorld *world, CEntity *parent)
  : CPlayerEntity( libit, world, parent )
{    
  // set the Player IO sizes correctly for this type of Entity
  m_data_len = sizeof( player_position_data_t );
  m_command_len = sizeof( player_position_cmd_t );
  m_config_len = 1; 
  m_reply_len = 1; 
  
  m_player.code = PLAYER_POSITION_CODE;

  // set up our sensor response
  laser_return = LaserTransparent;
  sonar_return = true;
  obstacle_return = true;
  puck_return = true;
  idar_return = IDARTransparent;
  vision_return = true;

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

  // Pioneer-like behavior by default
  this->reset_odom_on_disconnect = true;

  // took this out - now use the 'offset [X Y]' worldfile setting - rtv.
  // pioneer.inc now defines a pioneer as a positiondevice with a 4cm offset
  //this->origin_x = -0.04; 

  this->origin_x = 0;
  this->origin_y = 0;
}

///////////////////////////////////////////////////////////////////////////
// Startup routine
//
bool CPositionDevice::Startup()
{
  if (!CPlayerEntity::Startup())
    return false;

  return true;
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
      if( reset_odom_on_disconnect )
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
  player_position_set_odom_req_t* setOdom;

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
	puts( "Warning: positiondevice motor power request not implemented");
        PutReply(client, PLAYER_MSGTYPE_RESP_ACK);
        break;
	
      case PLAYER_POSITION_VELOCITY_MODE_REQ:
        // velocity control mode:
        //   0 = direct wheel velocity control (default)
        //   1 = separate translational and rotational control
        // CONFIG NOT IMPLEMENTED
	puts( "Warning: positiondevice velocity mode request not implemented");
        PutReply(client, PLAYER_MSGTYPE_RESP_ACK);
        break;
	
      case PLAYER_POSITION_RESET_ODOM_REQ:
	// reset position to 0,0,0: ignore value
        this->odo_px = this->odo_py = this->odo_pth = 0.0;
        PutReply(client, PLAYER_MSGTYPE_RESP_ACK);
        break;
	
      case PLAYER_POSITION_SET_ODOM_REQ:
        
        if(len != sizeof(player_position_set_odom_req_t)) {
            fprintf(stderr, "ERROR: SET_ODOM request had the wrong size data buffer. (was %d, should be %d.)\n", len, sizeof(player_position_set_odom_req_t));
            PutReply(client, PLAYER_MSGTYPE_RESP_NACK);
            break;
        }

        setOdom = (player_position_set_odom_req_t *) buffer;

        if(setOdom->subtype != PLAYER_POSITION_SET_ODOM_REQ) {
            fprintf(stderr, "ERROR: SET_ODOM request had the wrong subtype. (was %d, should be %d.)\n", setOdom->subtype, PLAYER_POSITION_SET_ODOM_REQ);
            break;
        }

//        printf("DEBUG: request is for set pose %d, %d, %d.\n", ntohl(setOdom->x), ntohl(setOdom->y), ntohs(setOdom->theta));
	    // set my odometry to the values in the packet
        this->odo_px =  M_32(setOdom->x);
        this->odo_py =  M_32(setOdom->y);
        this->odo_pth = Rad_32(setOdom->theta);
	    printf( "DEBUG: set pose to %f, %f x %f.\n", this->odo_px, this->odo_py, this->odo_pth);
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
    // this device only uses x and theta velocities and ignores
    // everything else
    this->com_vr = M_32(this->cmd.xspeed);
    this->com_vth = Rad_32(this->cmd.yawspeed);
        
    // normalize yaw +-PI
    this->com_vth = atan2(sin(this->com_vth), cos(this->com_vth));

    //printf( "vr %.2f vth %.2f * \n", com_vr, com_vth );

    // Set our x,y,th velocity components so that other entities (like pucks)
    // can get at them.
    SetGlobalVel(this->com_vr * cos(com_vth),this->com_vr * sin(com_vth), 0.0);
  }
}


///////////////////////////////////////////////////////////////////////////
// Compose data to send back to client
void CPositionDevice::UpdateData()
{ 
  // normalize yaw 0-2PI
 
  // Construct the data packet
  // Basically just changes byte orders and some units
  this->data.xpos = MM_32(this->odo_px);
  this->data.ypos = MM_32(this->odo_py);
  this->data.yaw = Deg_32( fmod( this->odo_pth + TWOPI, TWOPI ) );

  this->data.xspeed = MM_32(this->com_vr);
  this->data.yspeed = MM_32(0); // this device can't move sideways
  this->data.yawspeed = Deg_32(NORMALIZE(this->com_vth));
  
  this->data.stall =  (uint8_t)(this->stall ? 1 : 0 );  

  PutData(&this->data, sizeof(this->data));     
}

///////////////////////////////////////////////////////////////////////////
// Load the entity from the world file
bool CPositionDevice::Load(CWorldFile *worldfile, int section)
{
  if (!CPlayerEntity::Load(worldfile, section))
    return false;

  const char *rvalue;
  
  // Obstacle return values
  if (this->reset_odom_on_disconnect )
    rvalue = "reset";
  else
    rvalue = "keep";
  rvalue = worldfile->ReadString(section, "odometry", rvalue);
  if (strcmp(rvalue, "keep") == 0)
    this->reset_odom_on_disconnect = false;
  else
    this->reset_odom_on_disconnect = true;

  return true;
}


#ifdef INCLUDE_RTK2

///////////////////////////////////////////////////////////////////////////
// Initialise the rtk gui
void CPositionDevice::RtkStartup()
{
  CPlayerEntity::RtkStartup();
  
  // add a 'nose line' indicating forward to the entity's normal
  // rectangle or circle. draw from the center to the front.
  rtk_fig_line( this->fig, 
		0, 0, 
		this->size_x/2.0 + this->origin_x, this->origin_y );
}


#endif
