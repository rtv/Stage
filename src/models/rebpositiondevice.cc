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
 * Desc: Simulates the UMass Ubot, a differential robot.  Adapted from repositiondevice.cc.
 * Author: John Sweeney
 * Date: 2002
 * CVS info: $Id: rebpositiondevice.cc,v 1.1 2003-06-10 03:29:21 jazzfunk Exp $
 */

//#define DEBUG

#include <math.h>
#include "world.hh"
#include "rebpositiondevice.hh"

#ifndef SGN 
#define SGN(x) ((x) >= 0.0 ? 1.0 : -1.0)
#endif

///////////////////////////////////////////////////////////////////////////
// Constructor
CREBPositionDevice::CREBPositionDevice(LibraryItem *libit, CWorld *world, CEntity *parent)
  : CPlayerEntity( libit, world, parent )
{    

  PositionInit();

  // set up our sensor response
  laser_return = LaserTransparent;
  sonar_return = true;
  obstacle_return = true;
  puck_return = true;
  idar_return = IDARTransparent;

  com_vr = com_vth = 0;
  odo_px = odo_py = odo_pth = 0;
  stall = 0;

  // update this device VERY frequently
  m_interval = 0.01; 
  
  // assume robot is 2.5kg
  mass = 2.5;
  
  // Our UBot is an octagon, but that needs to be taken care of somewhere else...
  shape = ShapeRect;
  
  // this is the right size tho
  size_x = 0.19;
  size_y = 0.19;
  
  position_mode = false;
  heading_pd_control = false;

  motors_enabled = false;

  origin_x = 0;
  origin_y = 0;
}

///////////////////////////////////////////////////////////////////////////
// Startup routine
//
bool CREBPositionDevice::Startup()
{
  if (!CPlayerEntity::Startup()) {
    return false;
  }

  // from the "pose" keyword
  odo_px = init_px;
  odo_py = init_py;
  odo_pth = init_pth;

  return true;
}

///////////////////////////////////////////////////////////////////////////
// Update the position of the robot base
void CREBPositionDevice::Update( double sim_time )
{
  CPlayerEntity::Update(sim_time);

  ASSERT(m_world != NULL);

  // if its time to recalculate state
  if (sim_time - m_last_update >  m_interval) {
    m_last_update = sim_time;
    
    if(Subscribed() > 0) {
      // Process configuration requests
      UpdateConfig();
      
      // Get the latest command
      UpdateCommand();
      
      // do things
      Move();

      // report the new state of things
      //      UpdateData();
      PositionPutData(odo_px, odo_py, odo_pth,
		      com_vr, 0, com_vth, stall);
      
    } else  {
      // the device is not subscribed,
      // reset to default settings.
      // also set speeds to zero
      odo_px = odo_py = odo_pth = 0;
      com_vr = com_vth = 0;
    }
  }
  
  // Redraw ourself it we have moved
  double x, y, th;
  GetGlobalPose(x, y, th);
  ReMap(x, y, th);
}


int CREBPositionDevice::Move()
{
  double step = m_world->m_sim_timestep;

  // Get the current robot pose
  double px, py, pth;
  GetPose(px, py, pth);

  // Compute a new pose
  // This is a zero-th order approximation
  double qx = px + com_vr * step * cos(pth);
  double qy = py + com_vr * step * sin(pth);
  double qth = pth + com_vth * step;

  //  cout <<"REBPOSDEV: move: qx: "<<qx<<" qy: "<<qy<<" qth: "<<qth
  //       <<" comvr: "<<com_vr<<" comth: "<<com_vth<<endl;

  // Check for collisions
  // and accept the new pose if ok
  if (TestCollision(qx, qy, qth ) != NULL)
  {
    stall = true;
    SetGlobalVel(0,0,0);
  }
  else
  {
   
    // set pose now takes care of marking us dirty
    SetPose(qx, qy, qth);
    SetGlobalVel(com_vr * cos(com_vth),com_vr * sin(com_vth), 0.0);

    stall = false;

    // Compute the new odometric pose
    // Uses a first-order integration approximation
    //
    double dr = com_vr * step;
    double dth = com_vth * step;
    odo_px += dr * cos(odo_pth + dth / 2);
    odo_py += dr * sin(odo_pth + dth / 2);
    odo_pth += dth;

    // Normalise the odometric angle
    odo_pth = fmod(odo_pth + TWOPI, TWOPI);
  }
  return true;
}


///////////////////////////////////////////////////////////////////////////
// Process configuration requests.
void CREBPositionDevice::UpdateConfig()
{
  int len;
  void *client;
  char buffer[PLAYER_MAX_REQREP_SIZE];
  player_position_geom_t geom;
  
  while (1) {
    len = GetConfig(&client, buffer, sizeof(buffer));

    if (len <= 0) {
      break;
    }
    
    switch (buffer[0]) {
    case PLAYER_POSITION_MOTOR_POWER_REQ:
      // motor state change request 
      // 1 = enable motors
      // 0 = disable motors
      // (default)
      // CONFIG NOT IMPLEMENTED
      if (buffer[1]) {
	motors_enabled = true;
      } else {
	motors_enabled = false;
      }

      if (!motors_enabled) {
	SetGlobalVel(0, 0, 0);
      }

      //      cout <<"REBPOSITION: MOTOR POWER enabled: "<<motors_enabled<<endl;

      PutReply(client, PLAYER_MSGTYPE_RESP_ACK);
      break;
      
    case PLAYER_POSITION_VELOCITY_MODE_REQ: {
      // velocity control mode:
      //   0 = direct wheel velocity control (default)
      //   1 = velocity-based heading PD controller
      // know we are not in position mode
      position_mode = false;

      player_position_velocitymode_config_t *velcont = 
	(player_position_velocitymode_config_t *)buffer;

      if (!velcont->value) {
	heading_pd_control = false;
      } else {
	heading_pd_control = true;
      }

      //      cout <<"REBPOSITION: VELOCITY MODE: "<<heading_pd_control<<endl;

      PutReply(client, PLAYER_MSGTYPE_RESP_ACK);
    }
    break;
      
    case PLAYER_POSITION_RESET_ODOM_REQ:
      // reset position to 0,0,0: ignore value
      odo_px = odo_py = odo_pth = 0.0;
      PutReply(client, PLAYER_MSGTYPE_RESP_ACK);
      break;
      
    case PLAYER_POSITION_GET_GEOM_REQ:
      // Return the robot geometry
      geom.pose[0] = htons((short) (origin_x * 1000));
      geom.pose[1] = htons((short) (origin_y * 1000));
      geom.pose[2] = 0;
      geom.size[0] = htons((short) (size_x * 1000));
      geom.size[1] = htons((short) (size_y * 1000));
      PutReply(client, PLAYER_MSGTYPE_RESP_ACK, NULL, &geom, sizeof(geom));
      break;
      
    case PLAYER_POSITION_POSITION_MODE_REQ: {

      player_position_position_mode_req_t *pmode = 
	(player_position_position_mode_req_t *)buffer;

      position_mode = pmode->state;

      PutReply(client, PLAYER_MSGTYPE_RESP_ACK);
    }
    break;
    
    case PLAYER_POSITION_SET_ODOM_REQ: {
 
      player_position_set_odom_req_t *req = 
	(player_position_set_odom_req_t *)buffer;

      /*
      int x = ntohl(req->x);
      int y = ntohl(req->y);
      short t = ntohs(req->theta);
      odo_px = (double) x / 1000.0;
      odo_py = (double) y / 1000.0;
      odo_pth = (double) DTOR(t);
      */
      odo_px = ((int) ntohl(req->x))/ 1000.0;
      odo_py = ((int) ntohl(req->y))/1000.0;
      odo_pth = DTOR((int)ntohs(req->theta));

      PutReply(client, PLAYER_MSGTYPE_RESP_ACK);
      SetPose(odo_px, odo_py, odo_pth);
      // Redraw ourself it we have moved
      double xx, yy, th;
      GetGlobalPose(xx, yy, th);
      //      printf("REBPOSDEV: pose=(%g,%g,%g)\tglobal=(%g,%g,%g)\n",
      //	     odo_px, odo_py, odo_pth, xx, yy, th);
      ReMap(xx, yy, th);

    }
    break;
      
    default:
      PRINT_WARN1("got unknown config request \"%x\"\n", buffer[0]);
      PutReply(client, PLAYER_MSGTYPE_RESP_NACK);
      break;
    }
  }
}


///////////////////////////////////////////////////////////////////////////
// Parse the command buffer to extract commands
void CREBPositionDevice::UpdateCommand()
{
  double trans, rot;
  double desired_heading=0.0;
  double trans_command=0.0;
  double rot_command=0.0;
  
  if (GetCommand(&cmd, sizeof(cmd)) != sizeof(cmd)) {
    PRINT_DEBUG("REBPOSITION: WRONG SIZE COMMAND");    
    return;
  }

  desired_heading = (int)ntohl(cmd.yaw);
  trans_command = (int)ntohl(cmd.xspeed);
  rot_command = (int)ntohl(cmd.yawspeed);

  //  cout <<"REBPOSDEV: CMD: heading: "<<desired_heading<<" trans: "<<trans_command
  //<<" rot: "<<rot_command<<endl;
  // command mean different things in different modes
  if (!position_mode) {
    // VELOCITY MODE
    if (heading_pd_control) {
      // HEADING PD CONTROL MODE
      //	printf("REBPOSDEV: heading PD: des=%g trans=%g rot=%g\n",
      //	       desired_heading, trans_command, rot_command);
      double diff = desired_heading - RTOD(odo_pth);
      //cout <<"REBPOSDEV: odo_pth: "<<odo_pth<<" diff: "<<diff<<endl;
      if (diff > 180.0) {
	diff += -360.0;
      } else if (diff < -180.0) {
	diff += 360.0;
      }

      double err_ratio = diff /  180.0;
      
      trans = (1.0 - err_ratio)*trans_command;
      
      rot = err_ratio*3.0*rot_command;
      
      if (fabs(trans_command) - fabs(trans) < 0) {
	trans = SGN(trans) * trans_command;
      }
      
      if (fabs(rot_command) - fabs(rot) < 0) {
	rot = SGN(rot) * rot_command;
      }
      
    } else {
      trans = (int) ntohl(cmd.xspeed);
      rot = (int) ntohl(cmd.yawspeed);
    }
    
    // Store commanded speed
    // Linear is in m/s
    // Angular is in radians/sec
    com_vr = trans / 1000;
    com_vth = DTOR(rot);
    //cout <<"REBPOSDEV: com_vr: "<<com_vr<<" com_vth: "<<com_vth<<endl;
  } else {
    // POSITION MODE STUFF HERE
    // FIX not implemented
    com_vr = com_vth = 0.0;
  }
}


///////////////////////////////////////////////////////////////////////////
// Compose data to send back to client
void CREBPositionDevice::UpdateData()
{
  // Compute odometric pose
  // Convert mm and degrees (0 - 360)
  double px = odo_px * 1000.0;
  double py = odo_py * 1000.0;
  double pth = RTOD(fmod(odo_pth + TWOPI, TWOPI));

  // Get actual global pose
  double gx, gy, gth;
  GetGlobalPose(gx, gy, gth);
    
  // normalized compass heading
  double compass = NORMALIZE(gth);
  if (compass < 0)
    compass += TWOPI;
  
  // Construct the data packet
  // Basically just changes byte orders and some units
  data.xpos = htonl((int) rint(px));
  data.ypos = htonl((int) rint(py));
  data.yaw = htonl((int) rint(pth));

  data.xspeed = htonl((int) rint((com_vr * 1000.0)));
  data.yawspeed = htonl((int) rint(RTOD(com_vth)));  
  //data.compass = htons((unsigned short)(RTOD(compass)));
  data.stall = stall;

  //  cout <<"RPD: UD: x: "<<ntohl(data.xpos)<<" y: "<<ntohl(data.ypos)
  //<<" yaw: "<<ntohl(data.yaw)<<endl;
  //  PutData(&data, sizeof(data));     
  PositionPutData(odo_px, odo_py, odo_pth,
		  com_vr, 0, com_vth, stall);
}



#ifdef INCLUDE_RTK2

///////////////////////////////////////////////////////////////////////////
// Initialise the rtk gui
void CREBPositionDevice::RtkStartup()
{
  CPlayerEntity::RtkStartup();
  
  // add a 'nose line' indicating forward to the entity's normal
  // rectangle or circle. draw from the center of rotation to the front.
  rtk_fig_line( fig, 
		origin_x, origin_y, 
		size_x/2.0, origin_y );
}


#endif
