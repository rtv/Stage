///////////////////////////////////////////////////////////////////////////
//
// File: pioneermobiledevice.cc
// Author: Richard Vaughan, Andrew Howard
// Date: 5 Dec 2000
// Desc: Simulates the Pioneer robot base
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/pioneermobiledevice.cc,v $
//  $Author: ahoward $
//  $Revision: 1.9.2.21 $
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
#include "pioneermobiledevice.hh"

const double TWOPI = 6.283185307;


///////////////////////////////////////////////////////////////////////////
// Constructor
//
CPioneerMobileDevice::CPioneerMobileDevice(CWorld *world, CEntity *parent, CPlayerServer* server)
        : CPlayerDevice(world, parent, server,
                        POSITION_DATA_START,
                        POSITION_TOTAL_BUFFER_SIZE,
                        POSITION_DATA_BUFFER_SIZE,
                        POSITION_COMMAND_BUFFER_SIZE,
                        POSITION_CONFIG_BUFFER_SIZE)
{    
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

#ifdef INCLUDE_RTK
    m_mouse_radius = 0.400;
    m_draggable = true;
#endif

#ifdef INCLUDE_XGUI
    exp.objectType = pioneer_o;
#endif
}


///////////////////////////////////////////////////////////////////////////
// Update the position of the robot base
//
void CPioneerMobileDevice::Update()
{
    //RTK_TRACE0("updating CPioneerMobileDevice");
    
    // Get the latest command
    //
    if( GetCommand( &m_command, sizeof(m_command)) == sizeof(m_command))
    {
        ParseCommandBuffer();    // find out what to do    
    }
 
    Map(false); // erase myself
   
    Move();      // do things
    
    Map(true);   // draw myself 

    ComposeData();     // report the new state of things
    PutData( &m_data, sizeof(m_data)  );     // generic device call

    // Update our children
    //
    CPlayerDevice::Update();
}


int CPioneerMobileDevice::Move()
{
    // Note
    // Rather than moving this device, we actually move the robot device
    // with which we are associated.  While it's a bit of a hack, it
    // simplifies a lot of things in other places.
    //     ahoward
    
    double step_time = m_world->GetTime() - m_last_time;
    m_last_time += step_time;

    // Get the current robot pose
    //
    double px, py, pth;
    GetPose(px, py, pth);

    // Compute a new pose
    // This is a zero-th order approximation
    //
    double qx = px + m_com_vr * step_time * cos(pth);
    double qy = py + m_com_vr * step_time * sin(pth);
    double qth = pth + m_com_vth * step_time;

    // Normalise the angle
    //
    // *** ? qth = fmod(qth + 2 * M_PI, 2 * M_PI);

    // Check for collisions
    // and accept the new pose if ok
    //
    if (!InCollision(qx, qy, qth))
        SetPose(qx, qy, qth);

    // Compute the new odometric pose
    // Uses a first-order integration approximation
    //
    double dr = m_com_vr * step_time;
    double dth = m_com_vth * step_time;
    m_odo_px += dr * cos(m_odo_pth + dth / 2);
    m_odo_py += dr * sin(m_odo_pth + dth / 2);
    m_odo_pth += dth;

    // Normalise the odometric angle
    //
    m_odo_pth = fmod(m_odo_pth + TWOPI, TWOPI);
    
    return true;
  
    /* *** RETIRE ahoward
  Nimage* img = m_world->img;

  StoreRect(); // save my current rectangle in cased I move

  int moved = false;

  if( ( m_world->win && m_world->win->dragging != m_robot )  
      &&  (speed != 0.0 || turnRate != 0.0) )
    {
      // record speeds now - they can be altered in another thread
      float nowSpeed = speed;
      float nowTurn = turnRate;
      float nowTimeStep = m_world->timeStep;

      // find the new position for the robot
      float tx = m_robot->x 
	+ nowSpeed * m_world->ppm * cos( m_robot->a ) * nowTimeStep;
      
      float ty = m_robot->y 
	+ nowSpeed * m_world->ppm * sin( m_robot->a ) * nowTimeStep;
      
      float ta = m_robot->a + (nowTurn * nowTimeStep);
      
      ta = fmod( ta + TWOPI, TWOPI );  // normalize angle
      
      // calculate the rectangle for the robot's tentative new position
      CalculateRect( tx, ty, ta );

      //cout << "txya " << tx << ' ' << ty << ' ' << ta << endl;

      // trace the robot's outline to see if it will hit anything
      char hit = 0;
      if( hit = img->rect_detect( rect, m_robot->color ) > 0 )
	// hit! so don't do the move - just redraw the robot where it was
	{
	  //cout << "HIT! " << endl;
	  // restore from the saved rect
	  memcpy( &rect, &oldRect, sizeof( struct Rect ) );
	  
	  stall = 1; // motors stalled due to collision
	}
      else // do the move
	{
	  moved = true;
	  
	  stall = 0; // motors not stalled
	  
	  m_robot->x = tx; // move to the new position
	  m_robot->y = ty;
	  m_robot->a = ta;
	  
	  //update the robot's odometry estimate
	  // don't have to scale into pixel coords, 'cos these are in meters
	  xodom +=  nowSpeed * cos( aodom ) * nowTimeStep;
	  yodom +=  nowSpeed * sin( aodom ) * nowTimeStep;
	  
	  if( m_world->maxAngularError != 0.0 ) //then introduce some error
	    {
	      // could make this a +/- error, instead of a pure bias
	      // though i think the pioneer generally underestimates its turn
	      // due to wheel slippage
	      float error = 1.0 + ( drand48()* m_world->maxAngularError );
	      aodom += nowTurn * nowTimeStep * error;
	    }
	  else
	    aodom += nowTurn * nowTimeStep;
	  
	  aodom = fmod( aodom + TWOPI, TWOPI );  // normalize angle
	 	  
	  // update the `old' stuff
	  memcpy( &oldRect, &rect, sizeof( struct Rect ) );
	  oldCenterx = centerx;
	  oldCentery = centery;
	}
    }
  
  m_robot->oldx = m_robot->x;
  m_robot->oldy = m_robot->y;
  m_robot->olda = m_robot->a;

  return moved;
    */
}


///////////////////////////////////////////////////////////////////////////
// Parse the command buffer to extract commands
//
void CPioneerMobileDevice::ParseCommandBuffer()
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
void CPioneerMobileDevice::ComposeData()
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
    m_data.stalls = stall;
}


///////////////////////////////////////////////////////////////////////////
// Check to see if the given pose will yield a collision
//
bool CPioneerMobileDevice::InCollision(double px, double py, double pth)
{
    return false;
}


///////////////////////////////////////////////////////////////////////////
// Render the object in the world rep
//
bool CPioneerMobileDevice::Map(bool render)
{
    if (!render)
    {
        // Remove ourself from the obstacle map
        //
        double px = m_map_px;
        double py = m_map_py;
        double pa = m_map_pth;

        double qx = px + m_offset_x * cos(pa);
        double qy = py + m_offset_x * sin(pa);
        double sx = m_size_x;
        double sy = m_size_y;
        m_world->SetRectangle(qx, qy, pa, sx, sy, layer_obstacle, 0);

	//#ifdef INCLUDE_XGUI
	//if( win ) win->
    }
    else
    {
        // Add ourself to the obstacle map
        //
        double px, py, pa;
        GetGlobalPose(px, py, pa);

        double qx = px + m_offset_x * cos(pa);
        double qy = py + m_offset_x * sin(pa);
        double sx = m_size_x;
        double sy = m_size_y;
        m_world->SetRectangle(qx, qy, pa, sx, sy, layer_obstacle, 1);
        
        // Store the place we added ourself
        //
        m_map_px = px;
        m_map_py = py;
        m_map_pth = pa;
    }
    return true;
}



#ifdef INCLUDE_RTK

///////////////////////////////////////////////////////////////////////////
// Process GUI update messages
//
void CPioneerMobileDevice::OnUiUpdate(RtkUiDrawData *data)
{
    CEntity::OnUiUpdate(data);
    
    // Draw ourself
    //
    data->begin_section("global", "");
    
    if (data->draw_layer("chassis", true))
        DrawChassis(data);
    
    data->end_section();
}


///////////////////////////////////////////////////////////////////////////
// Process GUI mouse messages
//
void CPioneerMobileDevice::OnUiMouse(RtkUiMouseData *data)
{
    CEntity::OnUiMouse(data);
}


///////////////////////////////////////////////////////////////////////////
// Draw the pioneer chassis
//
void CPioneerMobileDevice::DrawChassis(RtkUiDrawData *data)
{
    #define ROBOT_COLOR RTK_RGB(255, 0, 192)
    
    data->set_color(ROBOT_COLOR);

    // Get global pose
    //
    double px, py, pa;
    GetGlobalPose(px, py, pa);

    // Compute an offset
    //
    double qx = px + m_offset_x * cos(pa);
    double qy = py + m_offset_x * sin(pa);
    double sx = m_size_x;
    double sy = m_size_y;
    data->ex_rectangle(qx, qy, pa, sx, sy);

    // Draw the direction indicator
    //
    for (int i = 0; i < 3; i++)
    {
        double px = min(sx, sy) / 2 * cos(DTOR(i * 45 - 45));
        double py = min(sx, sy) / 2 * sin(DTOR(i * 45 - 45));
        double pth = 0;

        // This is ugly, but it works
        //
        if (i == 1)
            px = py = 0;
        
        LocalToGlobal(px, py, pth);
        
        if (i > 0)
            data->line(qx, qy, px, py);
        qx = px;
        qy = py;
    }
}

#endif



