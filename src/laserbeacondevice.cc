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
//  $Revision: 1.4 $
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

#include <math.h>
#include <stage.h>
#include "world.hh"
#include "laserdevice.hh"
#include "laserbeacondevice.hh"


///////////////////////////////////////////////////////////////////////////
// Default constructor
//
CLaserBeaconDevice::CLaserBeaconDevice(CWorld *world, CEntity *parent,
                             CPlayerServer *server, CLaserDevice *laser)
        : CPlayerDevice(world, parent, server,
                        LASERBEACON_DATA_START,
                        LASERBEACON_TOTAL_BUFFER_SIZE,
                        LASERBEACON_DATA_BUFFER_SIZE,
                        LASERBEACON_COMMAND_BUFFER_SIZE,
                        LASERBEACON_CONFIG_BUFFER_SIZE)
{
    if (laser == NULL)
    {
        // *** Should check the type here
        m_laser = (CLaserDevice*) parent;
    }
    else
        m_laser = laser;
    
    m_time_sec = 0;
    m_time_usec = 0;
    
    // Set detection ranges
    // Beacons can be detected a large distance,
    // but can only be uniquely identified close up
    // These are the ranges for 0.5 degree resolution;
    // ranges for other resolutions are twice or half these values.
    //
    m_max_anon_range = 4.0;
    m_max_id_range = 1.5;

    exp.objectId = this;
    exp.objectType = laserbeacondetector_o;
    exp.data = (char*)&expBeacon;
    expBeacon.beaconCount = 0;
    strcpy( exp.label, "Laser Beacon Detector" );
}



///////////////////////////////////////////////////////////////////////////
// Load the object from an argument list
//
bool CLaserBeaconDevice::Load(int argc, char **argv)
{  
    if (!CPlayerDevice::Load(argc, argv))
        return false;

    for (int i = 0; i < argc;)
    {
        if (strcmp(argv[i], "range") == 0 && i + 1 < argc)
        {
            m_max_id_range = atof(argv[i + 1]);
            m_max_anon_range = 2 * m_max_id_range;
            i += 2;
        }
        else
            i++;
    }
    return true;
} 


///////////////////////////////////////////////////////////////////////////
// Save the object
//
bool CLaserBeaconDevice::Save(int &argc, char **argv)
{
    if (!CPlayerDevice::Save(argc, argv))
        return false;

    char range[32];
    snprintf(range, sizeof(range), "%.2f", (double) m_max_id_range); 
    argv[argc++] = strdup("range");
    argv[argc++] = strdup(range);
    
    return true;
}


///////////////////////////////////////////////////////////////////////////
// Update the beacon data
//
void CLaserBeaconDevice::Update()
{
    ASSERT(m_server != NULL);
    ASSERT(m_world != NULL);
    ASSERT(m_laser != NULL);
    
   
    // Get the laser range data
    //
    uint32_t time_sec, time_usec;
    player_laser_data_t laser;
    if (m_laser->GetData(&laser, sizeof(laser), &time_sec, &time_usec) == 0)
        return;

    // If this is old data, do nothing
    //
    if (time_sec == m_time_sec && time_usec == m_time_usec)
        return;

    expBeacon.beaconCount = 0; // initialise the count in the export structure

    // Do some byte swapping on the laser data
    //
    laser.resolution = ntohs(laser.resolution);
    laser.min_angle = ntohs(laser.min_angle);
    laser.max_angle = ntohs(laser.max_angle);
    laser.range_count = ntohs(laser.range_count);
    for (int i = 0; i < laser.range_count; i++)
        laser.ranges[i] = ntohs(laser.ranges[i]);
    
    // Get the pose of the detector in the global cs
    //
    double ox, oy, oth;
    GetGlobalPose(ox, oy, oth);

    // Compute resolution of laser scan data
    //
    double scan_min = laser.min_angle / 100.0 * M_PI / 180.0;
    double scan_res = laser.resolution / 100.0 * M_PI / 180.0;

    // Amount of tolerance to allow in range readings
    //
    double tolerance = 3.0 / m_world->ppm; //*** 0.10;

    // Reset the beacon data structure
    //
    player_laserbeacon_data_t beacon;
    beacon.count = 0;
   
    // Search for beacons in the list
    // Is quicker than searching the bitmap!
    //
    for (int i = 0; true; i++)
    {
        // Get the position of the laser beacon (global coords)
        //
        int id;
        double px, py, pth;
        if (!m_world->GetLaserBeacon(i, &id, &px, &py, &pth))
            break;

        // Compute range and bearing of beacon relative to sensor
        //
        double dx = px - ox;
        double dy = py - oy;
        double r = sqrt(dx * dx + dy * dy);
        double b = NORMALIZE(atan2(dy, dx) - oth);
        double o = NORMALIZE(pth - oth);

        // See if it is in the laser scan
        //
        int bi = (int) ((b - scan_min) / scan_res);
        if (bi < 0 || bi >= laser.range_count)
            continue;
        if (r > (laser.ranges[bi] & 0x1FFF) / 1000.0 + tolerance)
            continue;

        // Now see if it is within detection range
        //
        if (r > m_max_anon_range * DTOR(0.50) / scan_res)
            continue;
        if (r > m_max_id_range * DTOR(0.50) / scan_res)
            id = 0;

	// pack the beacon data into the export structure
	expBeacon.beacons[ expBeacon.beaconCount ].x = px;
	expBeacon.beacons[ expBeacon.beaconCount ].y = py;
	expBeacon.beacons[ expBeacon.beaconCount ].th = pth;
	expBeacon.beacons[ expBeacon.beaconCount ].id = id;
	expBeacon.beaconCount++;

        // Record beacons
        //
        assert(beacon.count < ARRAYSIZE(beacon.beacon));
        beacon.beacon[beacon.count].id = id;
        beacon.beacon[beacon.count].range = (int) (r * 1000);
        beacon.beacon[beacon.count].bearing = (int) RTOD(b);
        beacon.beacon[beacon.count].orient = (int) RTOD(o);
        beacon.count++;
    }

    // Get the byte ordering correct
    //
    for (int i = 0; i < beacon.count; i++)
    {
        beacon.beacon[i].range = htons(beacon.beacon[i].range);
        beacon.beacon[i].bearing = htons(beacon.beacon[i].bearing);
        beacon.beacon[i].orient = htons(beacon.beacon[i].orient);
    }
    beacon.count = htons(beacon.count);
    
    // Write beacon buffer to shared mem
    // Note that we apply the laser data's timestamp to this data.
    //
    PutData(&beacon, sizeof(beacon), time_sec, time_usec);
    m_time_sec = time_sec;
    m_time_usec = time_usec;
}



#ifdef INCLUDE_RTK

///////////////////////////////////////////////////////////////////////////
// Process GUI update messages
//
void CLaserBeaconDevice::OnUiUpdate(RtkUiDrawData *pData)
{
    // Draw our children
    //
    CEntity::OnUiUpdate(pData);    
}

#endif


