///////////////////////////////////////////////////////////////////////////
//
// File: laserdevice.cc
// Author: Andrew Howard
// Date: 28 Nov 2000
// Desc: Simulates the Player CLaserDevice (the SICK laser)
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/laserdevice.cc,v $
//  $Author: vaughan $
//  $Revision: 1.11.2.23 $
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

#include <stage.h>
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
    // Laser update rate (readings/sec)
    //
    m_update_rate = 360 / 0.200;
    m_last_update = 0;

    m_scan_res = DTOR(0.50);
    m_scan_min = DTOR(-90);
    m_scan_max = DTOR(+90);
    m_scan_count = 361;
    m_intensity = false;
  
    m_max_range = 8.0;

    // Dimensions of laser
    //
    m_map_dx = 0.155;
    m_map_dy = 0.155;

#ifdef INCLUDE_XGUI
    exp.objectType = laserturret_o;
    hitCount = 0;
#endif

#ifdef INCLUDE_RTK 
    m_hit_count = 0;
#endif
}


///////////////////////////////////////////////////////////////////////////
// Update the laser data
//
void CLaserDevice::Update()
{
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

        // Check to see if it is time to update the laser
        //
        double interval = m_scan_count / m_update_rate;
        if (m_world->GetTime() - m_last_update > interval)
        {
            m_last_update = m_world->GetTime();
        
            // Generate new scan data and copy to data buffer
            //
            player_laser_data_t data;
            GenerateScanData(&data);
            PutData(&data, sizeof(data));
        }
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
    player_laser_config_t config;
    if (GetConfig(&config, sizeof(config)) == 0)
        return false;  

    // Swap some bytes
    //
    config.resolution = ntohs(config.resolution);
    config.min_angle = ntohs(config.min_angle);
    config.max_angle = ntohs(config.max_angle);
    config.intensity = ntohs(config.intensity);

    // Emulate behaviour of SICK laser range finder
    //
    if (config.resolution == 25)
    {
        config.min_angle = max(config.min_angle, -5000);
        config.min_angle = min(config.min_angle, +5000);
        config.max_angle = max(config.max_angle, -5000);
        config.max_angle = min(config.max_angle, +5000);
        
        m_scan_res = DTOR((double) config.resolution / 100.0);
        m_scan_min = DTOR((double) config.min_angle / 100.0);
        m_scan_max = DTOR((double) config.max_angle / 100.0);
        m_scan_count = (int) ((m_scan_max - m_scan_min) / m_scan_res) + 1;
    }
    else if (config.resolution == 50 || config.resolution == 100)
    {
        config.min_angle = max(config.min_angle, -9000);
        config.min_angle = min(config.min_angle, +9000);
        config.max_angle = max(config.max_angle, -9000);
        config.max_angle = min(config.max_angle, +9000);
        
        m_scan_res = DTOR((double) config.resolution / 100.0);
        m_scan_min = DTOR((double) config.min_angle / 100.0);
        m_scan_max = DTOR((double) config.max_angle / 100.0);
        m_scan_count = (int) ((m_scan_max - m_scan_min) / m_scan_res) + 1;
    }
    else
    {
        // Ignore invalid configurations
        //  
        PLAYER_MSG0("invalid laser configuration request");
        return false;
    }
        
    m_intensity = config.intensity;

    return true;
}


///////////////////////////////////////////////////////////////////////////
// Generate scan data
//
bool CLaserDevice::GenerateScanData(player_laser_data_t *data)
{    
    // Get the pose of the laser in the global cs
    //
    double ox, oy, oth;
    GetGlobalPose(ox, oy, oth);

    // Compute laser fov, range, etc
    //
    double dr = 1.0 / m_world->ppm;
    double max_range = m_max_range;

#ifdef INCLUDE_RTK
    // Initialise gui data
    //
    m_hit_count = 0;
#endif

#ifdef INCLUDE_XGUI
    hitCount = 0;
#endif

    // Set the header part of the data packet
    //
    data->range_count = htons(m_scan_count);
    data->resolution = htons((int) (100 * RTOD(m_scan_res)));
    data->min_angle = htons((int) (100 * RTOD(m_scan_min)));
    data->max_angle = htons((int) (100 * RTOD(m_scan_max)));

    // Make sure the data buffer is big enough
    //
    ASSERT(m_scan_count <= ARRAYSIZE(data->ranges));
        
    // Do each scan
    //
    for (int s = 0; s < m_scan_count; s++)
    {
        int intensity = 0;
        double range = 0;
        double bearing = s * m_scan_res + m_scan_min;

        // Compute parameters of scan line
        //
        double px = ox;
        double py = oy;
        double pth = oth + bearing;

        // Compute the step for simple ray-tracing
        //
        double dx = dr * cos(pth);
        double dy = dr * sin(pth);

        // Look along scan line for obstacles
        // Could make this an int again for a slight speed-up.
        //
        for (range = 0; range < max_range; range += dr)
        {
            // Look in the laser layer for obstacles
            // Also look at the two cells to the right and above
            // so we dont sneak through gaps.
            //
            uint8_t cell = 0;
            cell |= m_world->GetCell(px, py, layer_laser);
            cell |= m_world->GetCell(px + dr, py, layer_laser);
            cell |= m_world->GetCell(px, py + dr, layer_laser);
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
        uint16_t v = (uint16_t) (1000.0 * range);

        // Add in the intensity values in the top 3 bits
        //
        if (m_intensity)
            v = v | (((uint16_t) intensity) << 13);
        
        // Set the range
        //
        data->ranges[s] = htons(v);

#ifdef INCLUDE_RTK
        // Update the gui data
        //
        m_hit[m_hit_count][0] = px;
        m_hit[m_hit_count][1] = py;
        m_hit_count++;
#endif

#ifdef INCLUDE_XGUI
	hitPts[hitCount].x = px;
	hitPts[hitCount].y = py;
	hitCount++;
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

#ifdef INCLUDE_XGUI

////////////////////////////////////////////////////////////////////////////
// compose and return the export data structure for external rendering
// return null if we're not exporting data right now.
ExportData* CLaserDevice::GetExportData( void )
{
  if( !exporting ) return 0;

  // fill in the exp structure
  // exp.type, exp.id, exp.dataSize are set in the constructor
  GetGlobalPose( exp.x, exp.y, exp.th );

  exp.dataSize = 361 * sizeof( DPoint );
  exp.data = (unsigned char*)hitPts;
  
  return &exp;
}

#endif

#ifdef INCLUDE_RTK

///////////////////////////////////////////////////////////////////////////
// Process GUI update messages
//
void CLaserDevice::OnUiUpdate(RtkUiDrawData *event)
{
    // Draw our children
    //
    CObject::OnUiUpdate(event);
    
    // Draw ourself
    //
    event->begin_section("global", "laser");
    
    if (event->draw_layer("", true))
        DrawTurret(event);
    if (event->draw_layer("scan", true))
        if (IsSubscribed())
            DrawScan(event);
    
    event->end_section();
}


///////////////////////////////////////////////////////////////////////////
// Process GUI mouse messages
//
void CLaserDevice::OnUiMouse(RtkUiMouseData *event)
{
    CObject::OnUiMouse(event);
}


///////////////////////////////////////////////////////////////////////////
// Draw the laser turret
//
void CLaserDevice::DrawTurret(RtkUiDrawData *event)
{
    #define TURRET_COLOR RTK_RGB(0, 0, 255)
    
    event->set_color(TURRET_COLOR);

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
    event->ex_rectangle(gx, gy, gth, dx, dy); 
}


///////////////////////////////////////////////////////////////////////////
// Draw the laser scan
//
void CLaserDevice::DrawScan(RtkUiDrawData *event)
{
    #define SCAN_COLOR RTK_RGB(0, 0, 255)
    
    event->set_color(SCAN_COLOR);

    // Get global pose
    //
    double gx, gy, gth;
    GetGlobalPose(gx, gy, gth);

    double qx, qy;
    qx = gx;
    qy = gy;
    
    for (int i = 0; i < m_hit_count; i++)
    {
        double px = m_hit[i][0];
        double py = m_hit[i][1];
        event->line(qx, qy, px, py);
        qx = px;
        qy = py;
    }
    event->line(qx, qy, gx, gy);
}

#endif



