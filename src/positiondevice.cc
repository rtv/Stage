///////////////////////////////////////////////////////////////////////////
//
// File: pioneermobiledevice.cc
// Author: Richard Vaughan, Andrew Howard
// Date: 5 Dec 2000
// Desc: Simulates the Pioneer robot base
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/positiondevice.cc,v $
//  $Author: ahoward $
//  $Revision: 1.14.2.1 $
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
#include "positiondevice.hh"
#include "raytrace.hh"

///////////////////////////////////////////////////////////////////////////
// Constructor
//
CPositionDevice::CPositionDevice(CWorld *world, CEntity *parent )
  : CEntity( world, parent )
{    
  // set the Player IO sizes correctly for this type of Entity
  m_data_len = sizeof( player_position_data_t );
  m_command_len = sizeof( player_position_cmd_t );
  m_config_len = 1; // configurable!
  
  m_player_type = PLAYER_POSITION_CODE; // from player's messages.h
  
  m_stage_type = RectRobotType;

  // set up our sensor response
  laser_return = LaserNothing;
  sonar_return = true;
  obstacle_return = true;
  puck_return = true;

  //  m_com_vr = 0.0;
  //m_com_vth = 0;
  m_com_vr = m_com_vth = 0;
  m_map_px = m_map_py = m_map_pth = 0;
  
  // Set the robot dimensions
  // Due to the unusual shape of the pioneer,
  // it is modelled as a rectangle offset from the origin
  //
  m_size_x = 0.440;
  m_size_y = 0.380;
  m_offset_x = -0.04;
  
  m_odo_px = m_odo_py = m_odo_pth = 0;
  
  stall = 0;
  
  m_interval = 0.01; // update this device VERY frequently
  
  // assume robot is 20kg
  m_mass = 20.0;
}

///////////////////////////////////////////////////////////////////////////
// Update the position of the robot base
void CPositionDevice::Update( double sim_time )
{
  ASSERT(m_world != NULL);
  
  // if its time to recalculate state
  //
  if( sim_time - m_last_update >  m_interval )
  {
    m_last_update = sim_time;
      
    if( Subscribed() > 0 )
    {  
      // Get latest config
      unsigned char cfg[m_config_len];
      int cfg_size;
      if((cfg_size = GetConfig(cfg,sizeof(cfg))))
      {
        switch(cfg[0])
        {
          case PLAYER_POSITION_MOTOR_POWER_REQ:
            /* motor state change request 
             * 1 = enable motors
             * 0 = disable motors
             * (default)
             *                                  
             */
            if(cfg_size-1 != 1)
            {
              puts("Arg to motor state change request is wrong size; ignoring");
              break;
            }
            // right size, but who cares.
            break;
          case PLAYER_POSITION_VELOCITY_CONTROL_REQ:
            /* velocity control mode:
             *   0 = direct wheel velocity control (default)
             *   1 = separate translational and rotational control
             */
            if(cfg_size-1 != 1)
            {
              puts("Arg to velocity control mode change request is wrong "
                   "size; ignoring");
              break;
            }
            // right size, but who cares.
            break;
          case PLAYER_POSITION_RESET_ODOM_REQ:
            /* reset position to 0,0,0: no args */
            if(cfg_size-1 != 0)
            {
              puts("Arg to reset position request is wrong size; ignoring");
              break;
            }
            m_odo_px = m_odo_py = m_odo_pth = 0.0;
            break;
          default:
            printf("Position device got unknown config request \"%c\"\n",
                   cfg[0]);
            break;
        }
      }

      // Get the latest command
      //
      if( GetCommand( &m_command, sizeof(m_command)) == sizeof(m_command))
	    {
	      ParseCommandBuffer();    // find out what to do    
	    }
	  
      Move();      // do things
  
      ComposeData();     // report the new state of things
      PutData( &m_data, sizeof(m_data)  );     // generic device call
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
    
  // if we've moved 
  if( (m_map_px != x) || (m_map_py != y) || (m_map_pth != th ) )
  {
    Map(false); // erase myself
      
    m_map_px = x; // update the render positions
    m_map_py = y;
    m_map_pth = th;
      
    Map(true);   // draw myself 
  }      
}


int CPositionDevice::Move()
{
  double step = m_world->m_sim_timestep;

  // Get the current robot pose
  double px, py, pth;
  GetPose(px, py, pth);

  // Compute current velocities
  double vx = m_com_vr * cos(pth);
  double vy = m_com_vr * sin(pth);
  double vth = m_com_vth;
  
  // Compute a new pose
  // This is a zero-th order approximation
  double qx = px + m_com_vr * step * cos(pth);
  double qy = py + m_com_vr * step * sin(pth);
  double qth = pth + m_com_vth * step;

  // Check for collisions
  // and accept the new pose if ok
  //
  if (InCollision(qx, qy, qth))
  {
    SetGlobalVel(0, 0, 0);
    this->stall = 1;
  }
  else
  {
    SetGlobalVel(vx, vy, vth);
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
  //
  m_com_vr = fv / 1000;
  m_com_vth = DTOR(fw);
}


///////////////////////////////////////////////////////////////////////////
// Compose data to send back to client
//
void CPositionDevice::ComposeData()
{
  // Compute odometric pose
  // Convert mm and degrees (0 - 360)
  //
  double px = m_odo_px * 1000.0;
  double py = m_odo_py * 1000.0;
  double pth = RTOD(fmod(m_odo_pth + TWOPI, TWOPI));

  // Get actual global pose
  //
  double gx, gy, gth;
  GetGlobalPose(gx, gy, gth);
    
  // normalized compass heading
  //
  double compass = fmod( gth + M_PI/2.0 + TWOPI, TWOPI ); 
  
  // Construct the data packet
  // Basically just changes byte orders and some units
  //
  m_data.xpos = htonl((int) px);
  m_data.ypos = htonl((int) py);
  m_data.theta = htons((unsigned short) pth);

  m_data.speed = htons((unsigned short) (m_com_vr * 1000.0));
  m_data.turnrate = htons((short) RTOD(m_com_vth));  
  m_data.compass = htons((unsigned short)(RTOD(compass)));
  m_data.stalls = this->stall;
}


///////////////////////////////////////////////////////////////////////////
// Check to see if the given pose will yield a collision
//


bool CPositionDevice::InCollision(double px, double py, double pth)
{
  switch( this->shape ) 
  {
    case ShapeRect:
    {
      double qx = px + m_offset_x * cos(pth);
      double qy = py + m_offset_y * sin(pth);
      CEntity* ent;
      CRectangleIterator rit( qx, qy, pth, m_size_x, m_size_y, 
                              m_world->ppm, m_world->matrix );
	
      while( (ent = rit.GetNextEntity()) )
      {
        if( ent != this && ent->obstacle_return )
        {
          //printf( "hit ent %p (%d)\n",
          //ent, ent->m_stage_type );
	      
          return true;
        }
      }
      return false;
    }
    case ShapeCircle:
    {
      CEntity* ent;
      CCircleIterator rit( px, py, m_size_x/2.0,  
                           m_world->ppm, m_world->matrix );
	
      while( (ent = rit.GetNextEntity()) )
      {
        if( ent != this && ent->obstacle_return )
        {
          //printf( "hit ent %p (%d)\n",
          //ent, ent->m_stage_type );
	      
          return true;
        }
      }
      return false;
    }
    default:
      printf( "unknown shape in positiondevice::incollision" );
      break;
  }
  return false;
}


///////////////////////////////////////////////////////////////////////////
// Render the object in the world rep
void CPositionDevice::Map(bool render )
{
  if (!render)
  {
    m_world->matrix->SetMode( mode_unset );
      
    if(this->shape == ShapeRect)
    {
      // Remove ourself from the obstacle map
      double px = m_map_px;
      double py = m_map_py;
      double pa = m_map_pth;
	  
      double qx = px + m_offset_x * cos(pa);
      double qy = py + m_offset_x * sin(pa);
      double sx = m_size_x;
      double sy = m_size_y;
	  
      m_world->SetRectangle( qx, qy, pa, sx, sy, this );
    }
    else if(this->shape == ShapeCircle)
      m_world->SetCircle( m_map_px, m_map_py, m_size_x/2.0, this );
    else
      PRINT_MSG("CPositionDevice::Map(): unknown shape!");
  }
  else
  {
    m_world->matrix->SetMode( mode_set );
  
    // Add ourself to the obstacle map
    double px, py, pa;
    GetGlobalPose(px, py, pa);
    if(this->shape == ShapeRect)
	  {
	    double qx = px + m_offset_x * cos(pa);
	    double qy = py + m_offset_x * sin(pa);
	    double sx = m_size_x;
	    double sy = m_size_y;

	    m_world->SetRectangle( qx, qy, pa, sx, sy, this );
	  }
    else if (this->shape == ShapeCircle)
      m_world->SetCircle( px, py, m_size_x/2.0, this );
    else
      PRINT_MSG("CPositionDevice::Map(): unknown shape!");
	
    // Store the place we added ourself
    m_map_px = px;
    m_map_py = py;
    m_map_pth = pa;
  }
}



#ifdef INCLUDE_RTK

///////////////////////////////////////////////////////////////////////////
// Process GUI update messages
//
void CPositionDevice::OnUiUpdate(RtkUiDrawData *data)
{
  CEntity::OnUiUpdate(data);
    
  data->begin_section("global", "");
    
  if (data->draw_layer("chassis", true))
    DrawChassis(data);
    
  data->end_section();
}


///////////////////////////////////////////////////////////////////////////
// Process GUI mouse messages
//
void CPositionDevice::OnUiMouse(RtkUiMouseData *data)
{
  CEntity::OnUiMouse(data);
}


///////////////////////////////////////////////////////////////////////////
// Draw the pioneer chassis
//
void CPositionDevice::DrawChassis(RtkUiDrawData *data)
{
  data->set_color(RTK_RGB(m_color.red, m_color.green, m_color.blue));

  // Get global pose
  double px, py, pa;
  GetGlobalPose(px, py, pa);

  if (this->shape == ShapeRect)
  {
    // Compute an offset
    double qx = px + m_offset_x * cos(pa);
    double qy = py + m_offset_x * sin(pa);
    double sx = m_size_x;
    double sy = m_size_y;
    data->ex_rectangle(qx, qy, pa, sx, sy);

    // Draw the direction indicator
    for (int i = 0; i < 3; i++)
    {
      double px = (sx < sy ? sx : sy) / 2 * cos(DTOR(i * 45 - 45));
      double py = (sx < sy ? sx : sy) / 2 * sin(DTOR(i * 45 - 45));
      double pth = 0;

      // This is ugly, but it works
      if (i == 1)
        px = py = 0;

      LocalToGlobal(px, py, pth);

      if (i > 0)
        data->line(qx, qy, px, py);
      qx = px;
      qy = py;
    }
  }
  else if (this->shape == ShapeCircle)
  {
    data->ellipse(px - m_size_x/2.0, py - m_size_x/2.0, 
                  px + m_size_x/2.0, py + m_size_x/2.0);
      
    // Draw the direction indicator
    for (int i = 0; i < 3; i++)
    {
      double qx = m_size_x/2.0 * cos(DTOR(i * 45 - 45));
      double qy = m_size_x/2.0 * sin(DTOR(i * 45 - 45));
      double qth = 0;

      // This is ugly, but it works
      if (i == 1)
        qx = qy = 0;

      LocalToGlobal(qx, qy, qth);

      if (i > 0)
        data->line(px, py, qx, qy);
      px = qx;
      py = qy;
    }
  }
  else
    PRINT_MSG("CPositionDevice::DrawChassis(): unknown shape!");
}

#endif
