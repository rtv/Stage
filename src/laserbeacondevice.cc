///////////////////////////////////////////////////////////////////////////
//
// File: laserbeacondevice.cc
// Author: Andrew Howard
// Date: 12 Jan 2000
// Desc: Simulates the laser-based beacon detector
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/laserbeacondevice.cc,v $
//  $Author: vaughan $
//  $Revision: 1.10 $
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
CLBDDevice::CLBDDevice(CWorld *world, CLaserDevice *parent )
  : CEntity(world, parent )
{
  // set the Player IO sizes correctly for this type of Entity
  m_data_len    = sizeof( player_laserbeacon_data_t );
  m_command_len = 0; //sizeof( player_laserbeacon_command_t );
  m_config_len  = sizeof( player_laserbeacon_config_t );

  m_player_type = PLAYER_LASERBEACON_CODE;
 
  m_stage_type = LBDType;

  // the parent MUST be a laser device
  ASSERT( parent );
  ASSERT( parent->m_player_type == PLAYER_LASER_CODE );
  
  m_laser = parent; 
  //m_laser->m_dependent_attached = true;
  
  m_size_x = 0.9 * m_laser->m_size_x;
  m_size_y = 0.9 * m_laser->m_size_y;
  
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

  expBeacon.beaconCount = 0; // for rtkstage

  m_interval = 0.2; // matches laserdevice
}

///////////////////////////////////////////////////////////////////////////
// Load the object from an argument list
//
bool CLBDDevice::Load(int argc, char **argv)
{  
    if (!CEntity::Load(argc, argv))
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
bool CLBDDevice::Save(int &argc, char **argv)
{
    if (!CEntity::Save(argc, argv))
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
void CLBDDevice::Update( double sim_time )
{
  //CEntity::Update( sim_time ); // inherit debug output

  ASSERT(m_world != NULL );
  ASSERT(m_laser != NULL );

  if(!Subscribed())
    return;
  
  // if its time to recalculate gripper state
  //
  if( sim_time - m_last_update < m_interval )
    return;

  m_last_update = sim_time;

    
    // Get the laser range data
    //
    uint32_t time_sec=0, time_usec=0;
    player_laser_data_t laser;
    if (m_laser->GetData(&laser, sizeof(laser) ) == 0)
        return;
    
    expBeacon.beaconCount = 0; // initialise the count in the export structure


    // Do some byte swapping on the laser data
    //
    laser.resolution = ntohs(laser.resolution);
    laser.min_angle = ntohs(laser.min_angle);
    laser.max_angle = ntohs(laser.max_angle);
    laser.range_count = ntohs(laser.range_count);
    ASSERT(laser.range_count < ARRAYSIZE(laser.ranges));
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

	//printf( "beacon at: %.2f %.2f %.2f\n", px, py, pth );
	//fflush( stdout );

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
    PutData( &beacon, sizeof(beacon) );
    m_time_sec = time_sec;
    m_time_usec = time_usec;
}



#ifdef INCLUDE_RTK

///////////////////////////////////////////////////////////////////////////
// Process GUI update messages
//
void CLBDDevice::OnUiUpdate(RtkUiDrawData *event)
{
    // Default draw
    //
    CEntity::OnUiUpdate(event);

    // Draw debugging info
    //
    event->begin_section("global", "laser_beacon");
    
    if (event->draw_layer("data", true))
    {
      if(Subscribed())
      {
        DrawData(event);
        // call Update(), because we may have stolen the truth_poked
        Update(m_world->GetTime());
      }
    }
    
    event->end_section();
}


///////////////////////////////////////////////////////////////////////////
// Draw the beacon data
//

void CLBDDevice::DrawData(RtkUiDrawData *event)
{
    #define SCAN_COLOR RTK_RGB(0, 0, 255)
    
    event->set_color(SCAN_COLOR);

    // Get global pose
    //
    double gx, gy, gth;
    GetGlobalPose(gx, gy, gth);

    for (int i = 0; i < expBeacon.beaconCount; i++)
    {
        //int id = expBeacon.beacons[i].id;
        double px = expBeacon.beacons[i].x;
        double py = expBeacon.beacons[i].y;
        event->ex_arrow(gx, gy, px, py, 0, 0.10);
    }
}


#endif




