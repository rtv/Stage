///////////////////////////////////////////////////////////////////////////
//
// File: pioneermobiledevice.cc
// Author: Richard Vaughan, Andrew Howard
// Date: 5 Dec 2000
// Desc: Simulates the Pioneer robot base
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/pioneermobiledevice.cc,v $
//  $Author: vaughan $
//  $Revision: 1.9.2.27 $
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

    exp.objectType = pioneer_o;
    exp.width = m_size_x;
    exp.height = m_size_y;
    strcpy( exp.label, "Pioneer" );
}


///////////////////////////////////////////////////////////////////////////
// Update the position of the robot base
//
void CPioneerMobileDevice::Update()
{
    // If the device is not subscribed,
    // reset to default settings.
    //
    if (!IsSubscribed())
        m_odo_px = m_odo_py = m_odo_pth = 0;
    
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

    // Check for collisions
    // and accept the new pose if ok
    //
    if (!InCollision(qx, qy, qth))
    {
        SetPose(qx, qy, qth);
        this->stall = 0;
    }
    else
        this->stall = 1;
        
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
    m_data.stalls = this->stall;
}


///////////////////////////////////////////////////////////////////////////
// Check to see if the given pose will yield a collision
//
bool CPioneerMobileDevice::InCollision(double px, double py, double pth)
{
    double qx = px + m_offset_x * cos(pth);
    double qy = py + m_offset_x * sin(pth);
    double sx = m_size_x;
    double sy = m_size_y;

    if (m_world->GetRectangle(qx, qy, pth, sx, sy, layer_obstacle) > 0)
        return true;
    
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
