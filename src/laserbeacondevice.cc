///////////////////////////////////////////////////////////////////////////
//
// File: laserbeacondevice.cc
// Author: Andrew Howard
// Date: 12 Jan 2000
// Desc: Simulates the laser-based beacon detector
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/laserbeacondevice.cc,v $
//  $Author: ahoward $
//  $Revision: 1.2.2.7 $
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
#include "world.hh"
#include "playerrobot.hh"
#include "laserdevice.hh"
#include "laserbeacondevice.hh"


///////////////////////////////////////////////////////////////////////////
// Default constructor
//
CLaserBeaconDevice::CLaserBeaconDevice(CWorld *world, CObject *parent,
                             CPlayerRobot *robot, CLaserDevice *laser)
        : CPlayerDevice(world, parent, robot,
                        LASERBEACON_DATA_START,
                        LASERBEACON_TOTAL_BUFFER_SIZE,
                        LASERBEACON_DATA_BUFFER_SIZE,
                        LASERBEACON_COMMAND_BUFFER_SIZE,
                        LASERBEACON_CONFIG_BUFFER_SIZE)
{
    m_laser = laser;
    
    // *** HACK -- should measure the interval properly
    //
    m_update_interval = 0.2;
    m_last_update = 0;

    // Set detection ranges
    // Beacons can be detected a large distance,
    // but can only be uniquely identified close up
    //
    m_max_anon_range = 4.0;
    m_max_id_range = 1.5;

    #ifdef INCLUDE_RTK

    // Reset the gui data
    //
    m_hit_count = 0;
    
    #endif
}


///////////////////////////////////////////////////////////////////////////
// Update the beacon data
//
void CLaserBeaconDevice::Update()
{
    ASSERT(m_robot != NULL);
    ASSERT(m_world != NULL);
    
    // Check to see if it is time to update the laser
    //
    if (m_world->get_time() - m_last_update <= m_update_interval)
        return;
    m_last_update = m_world->get_time();
    
    // Get the pose of the detector in the global cs
    //
    double ox, oy, oth;
    GetGlobalPose(ox, oy, oth);

    // Reset the beacon data structure
    //
    player_laserbeacon_data_t data;
    data.count = 0;

    #ifdef INCLUDE_RTK

    // Reset the gui data
    //
    m_hit_count = 0;
    
    #endif
    
    // Get the laser range data
    //
    ASSERT(m_laser != NULL);
    uint16_t laser[PLAYER_NUM_LASER_SAMPLES];
    m_laser->GetData((void*) laser, sizeof(laser));
    
    // Go through range data looking for beacons
    //
    for (int i = 0; i < PLAYER_NUM_LASER_SAMPLES; i++)
    {
        // Look for range readings with high reflectivity
        //
        bool beacon = ((ntohs(laser[i]) & 0xE000) != 0);
        if (!beacon)
            continue;
        
        // Compute position of object in global cs
        //
        double range = (double) (ntohs(laser[i]) & 0x1FFF) / 1000;
        double bearing = DTOR((double) i / 2 - 90);
        double orient = -oth;
        double px = ox + range * cos(oth + bearing);
        double py = oy + range * sin(oth + bearing);

        // Ignore beacons that are too far away
        //
        if (range > m_max_anon_range)
            continue;

        // See if there is an id in the beacon layer
        //
        int id = m_world->GetCell(px, py, layer_beacon);
        if (id == 0)
            continue;

        // Make beacon anonymous if it is far
        //
        if (range > m_max_id_range)
            id = 0;

        // Check for data buffer overrun
        //
        if (data.count >= ARRAYSIZE(data.beacon))
            break;

        // Update data buffer
        //
        data.beacon[data.count].id = id;
        data.beacon[data.count].range = (int) (range * 1000);
        data.beacon[data.count].bearing = (int) RTOD(bearing);
        data.beacon[data.count].orient = (int) RTOD(orient);
        data.count++;

        #ifdef INCLUDE_RTK

        // Update the gui
        //
        if (m_hit_count < RTK_ARRAYSIZE(m_hit))
        {
            m_id[m_hit_count] = id;
            m_hit[m_hit_count][0] = px;
            m_hit[m_hit_count][1] = py;
            m_hit_count++;
        }
            
        #endif
    }

    // Get the byte ordering correct
    //
    for (int i = 0; i < data.count; i++)
    {
        data.beacon[i].range = htons(data.beacon[i].range);
        data.beacon[i].bearing = htons(data.beacon[i].bearing);
        data.beacon[i].orient = htons(data.beacon[i].orient);
    }
    data.count = htons(data.count);
    
    // Write data buffer to shared mem
    //
    PutData(&data, sizeof(data));
}



#ifdef INCLUDE_RTK

///////////////////////////////////////////////////////////////////////////
// Process GUI update messages
//
void CLaserBeaconDevice::OnUiUpdate(RtkUiDrawData *pData)
{
    // Draw our children
    //
    CObject::OnUiUpdate(pData);
    
    // Draw ourself
    //
    pData->begin_section("global", "beacons");
  
    if (pData->draw_layer("beacons", true))
    {
        if (IsSubscribed() && m_robot->ShowSensors())
        {
            pData->set_color(RTK_RGB(192, 192, 255));
            
            for (int i = 0; i < m_hit_count; i++)
            {
                double d = 0.25;
                double ox = m_hit[i][0];
                double oy = m_hit[i][1];
                pData->ellipse(ox - d, oy - d, ox + d, oy + d);

                char id[32];
                sprintf(id, "%03d", (int) m_id[i]);
                pData->draw_text(ox + d, oy + d, id);
            }
        }
    }
    
    pData->end_section();
}

#endif





