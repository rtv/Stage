///////////////////////////////////////////////////////////////////////////
//
// File: beacondevice.cc
// Author: Andrew Howard
// Date: 12 Jan 2000
// Desc: Simulates the laser-based beacon detector
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/beacondevice.cc,v $
//  $Author: ahoward $
//  $Revision: 1.1.2.1 $
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
#include "world.hh"
#include "playerrobot.hh"
#include "laserdevice.hh"
#include "beacondevice.hh"


///////////////////////////////////////////////////////////////////////////
// Default constructor
//
CBeaconDevice::CBeaconDevice(CWorld *world, CObject *parent,
                             CPlayerRobot *robot, CLaserDevice *laser)
        : CPlayerDevice(world, parent, robot,
                        BEACON_DATA_START,
                        BEACON_TOTAL_BUFFER_SIZE,
                        BEACON_DATA_BUFFER_SIZE,
                        BEACON_COMMAND_BUFFER_SIZE,
                        BEACON_CONFIG_BUFFER_SIZE)
{
    m_laser = laser;
    
    // *** HACK -- should measure the interval properly
    //
    m_update_interval = 0.2;
    m_last_update = 0;

    #ifdef INCLUDE_RTK

    // Reset the gui data
    //
    m_hit_count = 0;
    
    #endif
}


///////////////////////////////////////////////////////////////////////////
// Update the beacon data
//
void CBeaconDevice::Update()
{
    ASSERT(m_robot != NULL);
    ASSERT(m_world != NULL);
    
    // Check to see if it is time to update the laser
    //
    if (m_world->GetTime() - m_last_update <= m_update_interval)
        return;
    m_last_update = m_world->GetTime();
    
    // Get the pose of the detector in the global cs
    //
    double ox, oy, oth;
    GetGlobalPose(ox, oy, oth);

    // Reset the beacon data structure
    //
    BeaconData data;
    data.count = 0;

    #ifdef INCLUDE_RTK

    // Reset the gui data
    //
    m_hit_count = 0;
    
    #endif
    
    // Get the laser range data
    //
    ASSERT(m_laser != NULL);
    UINT16 laser[LASER_NUM_SAMPLES];
    m_laser->GetData((void*) laser, sizeof(laser));
    
    // Go through range data looking for beacons
    //
    for (int i = 0; i < LASER_NUM_SAMPLES; i++)
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
        double px = ox + range * cos(oth + bearing);
        double py = oy + range * sin(oth + bearing);

        // See if there is an id in the beacon layer
        //
        int id = m_world->GetCell(px, py, layer_beacon);
        if (id == 0)
            continue;

        // Check for data buffer overrun
        //
        if (data.count >= ARRAYSIZE(data.beacon))
            break;

        // Update data buffer
        //
        data.beacon[data.count].id = id;
        data.beacon[data.count].range = (int) (range * 1000);
        data.beacon[data.count].bearing = (int) RTOD(bearing);
        data.count++;

        #ifdef INCLUDE_RTK

        // Update the gui
        //
        if (m_hit_count < ARRAYSIZE(m_hit))
        {
            m_id[m_hit_count] = id;
            m_hit[m_hit_count][0] = px;
            m_hit[m_hit_count][1] = py;
            m_hit_count++;
        }
            
        #endif
    }

    // Write data buffer to shared mem
    //
    PutData(&data, sizeof(data));
}



#ifdef INCLUDE_RTK

///////////////////////////////////////////////////////////////////////////
// Process GUI update messages
//
void CBeaconDevice::OnUiUpdate(RtkUiDrawData *pData)
{
    // Draw our children
    //
    CObject::OnUiUpdate(pData);
    
    // Draw ourself
    //
    pData->BeginSection("global", "beacons");
  
    if (pData->DrawLayer("beacons", true))
    {
        if (IsSubscribed() && m_robot->ShowSensors())
        {
            pData->SetColor(RTK_RGB(192, 192, 255));
            
            for (int i = 0; i < m_hit_count; i++)
            {
                double d = 0.25;
                double ox = m_hit[i][0];
                double oy = m_hit[i][1];
                pData->Ellipse(ox - d, oy - d, ox + d, oy + d);

                char id[32];
                sprintf(id, "%03d", (int) m_id[i]);
                pData->DrawText(ox + d, oy + d, id);
            }
        }
    }
    
    pData->EndSection();
}

#endif





