///////////////////////////////////////////////////////////////////////////
//
// File: laserdevice.cc
// Author: Andrew Howard
// Date: 28 Nov 2000
// Desc: Simulates the Player CLaserDevice (the SICK laser)
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/laserdevice.cc,v $
//  $Author: ahoward $
//  $Revision: 1.11.2.7 $
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

#define ENABLE_TRACE 1

#include <offsets.h>
#include <math.h> // RTV - RH-7.0 compiler needs explicit declarations
#include "world.hh"
#include "playerrobot.hh"
#include "laserdevice.hh"


///////////////////////////////////////////////////////////////////////////
// Default constructor
//
CLaserDevice::CLaserDevice(CWorld *world, CObject *parent, CPlayerRobot* robot)
        : CPlayerDevice(world, parent, robot,
                        LASER_DATA_START,
                        LASER_TOTAL_BUFFER_SIZE,
                        LASER_DATA_BUFFER_SIZE,
                        LASER_COMMAND_BUFFER_SIZE,
                        LASER_CONFIG_BUFFER_SIZE)
{
    // *** HACK -- should measure the interval properly
    //
    m_update_interval = 0.2;
    m_last_update = 0;

    m_min_segment = 0;
    m_max_segment = 361;
    m_intensity = false;
  
    m_samples = 361;
    m_max_range = 8.0; // meters - this could be dynamic one day
                     // but this matches the default laser setup.

    // Dimensions of laser
    // *** HACK I just made these up ahoward
    //
    m_map_dx = 0.20;
    m_map_dy = 0.20;
    
    #ifndef INCLUDE_RTK 
        undrawRequired = false;
    #else
        m_hit_count = 0;
    #endif
}


///////////////////////////////////////////////////////////////////////////
// Update the laser data
//
void CLaserDevice::Update()
{
    //TRACE0("updating laser data");
    ASSERT(m_robot != NULL);
    ASSERT(m_world != NULL);

    // Undraw ourselves from the world
    //
    Map(false);

    // Dont update anything if we are not subscribed
    //
    if (IsSubscribed())
    {
        // Check to see if the configuration has changed
        //
        CheckConfig();

        // Generate new scan data and copy to data buffer
        //
        if (UpdateScanData())
            PutData(m_data, m_samples * sizeof(UINT16));
    }

    // Redraw outselves in the world
    //
    Map(true);
}


///////////////////////////////////////////////////////////////////////////
// Check to see if the configuration has changed
//
bool CLaserDevice::CheckConfig()
{
    BYTE config[5];

    if (GetConfig(config, sizeof(config)) == 0)
        return false;  
    
    m_min_segment = ntohs(MAKEUINT16(config[0], config[1]));
    m_max_segment = ntohs(MAKEUINT16(config[2], config[3]));
    m_intensity = (bool) config[4];
    MSG3("new scan range [%d %d], intensity [%d]",
         (int) m_min_segment, (int) m_max_segment, (int) m_intensity);

    // *** HACK -- change the update rate based on the scan size
    // This is a guess only
    //
    m_update_interval = 0.2 * (m_max_segment - m_min_segment + 1) / 361;
        
    return true;
}


///////////////////////////////////////////////////////////////////////////
// Generate scan data
//
bool CLaserDevice::UpdateScanData()
{
    // Check to see if it is time to update the laser
    //
    if (m_world->timeNow - m_last_update <= m_update_interval)
        return false;
    m_last_update = m_world->timeNow;

    // Get the pose of the laser in the global cs
    //
    double ox, oy, oth;
    GetGlobalPose(ox, oy, oth);

    // Compute laser fov, range, etc
    //
    oth = oth - M_PI / 2;
    double dth = M_PI / m_samples;
    double dr = 1.0 / m_world->ppm;
    double max_range = m_max_range;

    // Initialise gui data
    //
    #ifdef INCLUDE_RTK
        m_hit_count = 0;
    #endif

    // Make sure the data buffer is big enough
    //
    ASSERT(m_samples <= ARRAYSIZE(m_data));

    // Do each scan
    //
    for (int s = 0; s < m_samples; s++)
    {
        // Allow for partial scans
        //
        if (s < m_min_segment || s > m_max_segment)
        {
            m_data[s] = 0;
            continue;
        }

        // Compute parameters of scan line
        //
        double px = ox;
        double py = oy;
        double pth = oth + s * dth;

        // Compute the step for simple ray-tracing
        //
        double dx = dr * cos(pth);
        double dy = dr * sin(pth);

        // Look along scan line for obstacles
        // Could make this an int again for a slight speed-up.
        //
        int intensity = 0;
        double range;
        for (range = 0; range < max_range; range += dr)
        {
            // Look in the laser layer for obstacles
            //
            BYTE cell = m_world->GetCell(px, py, layer_laser);
            if (cell != 0)
            {
                if (cell > 1)
                    intensity = 1;                
                break;
            }
            px += dx;
            py += dy;
        }

        // set laser value, scaled to current ppm
        // and converted to mm
        //
        UINT16 v = (UINT16) (1000.0 * range);

        // Add in the intensity values in the top 3 bits
        //
        if (m_intensity)
            v = v | (((UINT16) intensity) << 13);
        
        // Set the range
        // Swap the bytes while we're at it
        //
        m_data[s] = htons(v);

        // Update the gui data
        //
        #ifdef INCLUDE_RTK
            m_hit[m_hit_count][0] = px;
            m_hit[m_hit_count][1] = py;
            m_hit_count++;
        #endif
    }
    return true;
}


///////////////////////////////////////////////////////////////////////////
// Draw ourselves into the world rep
//
void CLaserDevice::Map(bool render)
{
    double dx = m_map_dx;
    double dy = m_map_dy;
    
    if (!render)
    {
        // Remove ourself from the map
        //
        double px = m_map_px;
        double py = m_map_py;
        double pth = m_map_pth;
        m_world->SetRectangle(px, py, pth, dx, dy, layer_laser, 0);
    }
    else
    {
        // Add ourself to the map
        //
        double px, py, pth;
        GetGlobalPose(px, py, pth);
        m_world->SetRectangle(px, py, pth, dx, dy, layer_laser, 1);
        m_map_px = px;
        m_map_py = py;
        m_map_pth = pth;
    }
}


#ifndef INCLUDE_RTK

bool CLaserDevice::GUIDraw()
{ 
  // dump out if noone is subscribed or the device
  if( !IsSubscribed() || !m_robot->showDeviceDetail ) return true;

  // replicate the first point at the end in order to draw a closed polygon
  hitPts[LASERSAMPLES].x = hitPts[0].x;
  hitPts[LASERSAMPLES].y = hitPts[0].y;

  m_world->win->SetForeground( m_world->win->RobotDrawColor( m_robot) );
  m_world->win->DrawLines( hitPts, LASERSAMPLES+1 );
    
  memcpy( oldHitPts, hitPts, sizeof( XPoint ) * (LASERSAMPLES+1) );
  
  undrawRequired = true;

  return true; 
};  

bool CLaserDevice::GUIUnDraw()
{ 
  if( undrawRequired )
  {
    m_world->win->SetForeground( m_world->win->RobotDrawColor( m_robot) );
    m_world->win->DrawLines( oldHitPts, LASERSAMPLES+1 );

    undrawRequired = false;
  }   
  return true; 
};

#else

///////////////////////////////////////////////////////////////////////////
// Process GUI update messages
//
void CLaserDevice::OnUiUpdate(RtkUiDrawData *pData)
{
    // Draw our children
    //
    CObject::OnUiUpdate(pData);
    
    // Draw ourself
    //
    pData->BeginSection("global", "laser");
    
    if (pData->DrawLayer("turret", true))
        DrawTurret(pData);
    if (pData->DrawLayer("scan", true) && IsSubscribed())
        DrawScan(pData);
    
    pData->EndSection();
}


///////////////////////////////////////////////////////////////////////////
// Process GUI mouse messages
//
void CLaserDevice::OnUiMouse(RtkUiMouseData *pData)
{
    CObject::OnUiMouse(pData);
}


///////////////////////////////////////////////////////////////////////////
// Draw the laser turret
//
void CLaserDevice::DrawTurret(RtkUiDrawData *pData)
{
    #define TURRET_COLOR RTK_RGB(0, 0, 255)
    
    pData->SetColor(TURRET_COLOR);

    // Turret dimensions
    //
    double dx = m_map_dx;
    double dy = m_map_dy;

    // Get global pose
    //
    double gx, gy, gth;
    GetGlobalPose(gx, gy, gth);
    
    // Draw the outline of the turret
    //
    pData->ExRectangle(gx, gy, gth, dx, dy); 
}


///////////////////////////////////////////////////////////////////////////
// Draw the laser scan
//
void CLaserDevice::DrawScan(RtkUiDrawData *pData)
{
    #define SCAN_COLOR RTK_RGB(0, 0, 255)
    
    pData->SetColor(SCAN_COLOR);

    // Get global pose
    //
    double gx, gy, gth;
    GetGlobalPose(gx, gy, gth);

    pData->MoveTo(gx, gy);
    for (int i = 0; i < m_hit_count; i++)
        pData->LineTo(m_hit[i][0], m_hit[i][1]);
    pData->LineTo(gx, gy);
}

#endif



