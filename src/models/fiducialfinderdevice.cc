///////////////////////////////////////////////////////////////////////////
//
// File: fiducialfinderdevice.cc
// Author: Richard Vaughan
// Date: 12 Jan 2000
// Desc: Simulates the laser-based beacon detector
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/models/fiducialfinderdevice.cc,v $
//  $Author: gerkey $
//  $Revision: 1.2.4.2.2.2 $
//
// Usage: detects objects that were laser bright and had non-zero
// ficucial_return
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

//#define DEBUG

#include <math.h>
#include <stage1p3.h>
#include "world.hh"
#include "laserdevice.hh"
#include "fiducialfinderdevice.hh"


///////////////////////////////////////////////////////////////////////////
// Default constructor
CFiducialFinder::CFiducialFinder(LibraryItem* libit,CWorld *world, CLaserDevice *parent )
  : CPlayerEntity(libit,world, parent )
{
  // set the Player IO sizes correctly for this type of Entity
  m_data_len    = sizeof( player_fiducial_data_t );
  m_command_len = 0; 
  m_config_len  = 1;
  m_reply_len  = 1;

  m_player.code = PLAYER_FIDUCIAL_CODE;
  
  // the parent MUST be a laser device
  ASSERT( parent );
  ASSERT( parent->m_player.code == PLAYER_LASER_CODE );
  
  this->laser = parent; 
  //this->laser->m_dependent_attached = true;

  // This is not a real object, so by default we dont see it
  this->obstacle_return = false;
  this->sonar_return = false;
  this->puck_return = false;
  this->vision_return = false;
  this->idar_return = IDARTransparent;
  this->laser_return = LaserTransparent;

  this->time_sec = 0;
  this->time_usec = 0;

  // Set detection ranges
  // Beacons can be detected a large distance,
  // but can only be uniquely identified close up
    
  // These are the ranges for 0.5 degree resolution;
  // ranges for other resolutions are twice or half these values.
  //
  this->max_range_anon = 4.0;
  this->max_range_id = 1.5;

  expBeacon.beaconCount = 0; // for rtkstage

  m_interval = 0.2; // matches laserdevice

  // fill in the default values for these fake parameters
  m_bit_count = 8;
  m_bit_size = 50;
  m_zero_thresh = m_one_thresh = 60;

  m_laser_subscribedp = false;

#ifdef INCLUDE_RTK2
  this->beacon_fig = NULL;
#endif

}

///////////////////////////////////////////////////////////////////////////
// Startup routine
//
bool CFiducialFinder::Startup()
{
  if (!CPlayerEntity::Startup())
    return false;

  return true;
}


///////////////////////////////////////////////////////////////////////////
// Load the entity from the world file
bool CFiducialFinder::Load(CWorldFile *worldfile, int section)
{
  if (!CPlayerEntity::Load(worldfile, section))
    return false;

  this->max_range_anon = worldfile->ReadLength(0, "lbd_range_anon",
                                               this->max_range_anon);
  this->max_range_anon = worldfile->ReadLength(section, "range_anon",
                                               this->max_range_anon);
  this->max_range_id = worldfile->ReadLength(0, "lbd_range_id",
                                             this->max_range_id);
  this->max_range_id = worldfile->ReadLength(section, "range_id",
                                             this->max_range_id);

  return true;
}


///////////////////////////////////////////////////////////////////////////
// Update the beacon data
//
void CFiducialFinder::Update( double sim_time )
{
  CPlayerEntity::Update( sim_time ); // inherit debug output

  ASSERT(m_world != NULL );
  ASSERT(this->laser != NULL );

  if(!Subscribed())
  {
    if(m_laser_subscribedp)
    {
      this->laser->Unsubscribe();
      m_laser_subscribedp = false;
    }
    return;
  }

  if(!m_laser_subscribedp)
  {
    this->laser->Subscribe();
    m_laser_subscribedp = true;
  }
  
  
  // if its time to recalculate 
  //
  if( sim_time - m_last_update < m_interval )
    return;

  m_last_update = sim_time;

  // check for configuration requests
  // the client can request the geometry of all beacons

  UpdateConfig();

  // Get the laser range data
  //
  uint32_t time_sec=0, time_usec=0;
  player_laser_data_t laser;
  if (this->laser->GetData(&laser, sizeof(laser) ) == 0)
  {
    //puts( "Stage warning: LBD device found no laser data" );
    return;
  }

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
  //double tolerance = 3.0 / m_world->ppm; //*** 0.10;

  // Reset the beacon data structure
  //
  player_fiducial_data_t beacon;
  beacon.count = 0;
   
  // Search for beacons in the list generated by the laser
  // Saves us from searching the bitmap again
  //
  for( LaserBeaconList::iterator it = this->laser->visible_beacons.begin();
       it != this->laser->visible_beacons.end(); it++ )
  {
    CEntity *nbeacon = (CEntity*)*it;        
    int id = nbeacon->fiducial_return;
    double px, py, pth;   
    nbeacon->GetGlobalPose( px, py, pth );

    //printf( "beacon at: %.2f %.2f %.2f\n", px, py, pth );
    //fflush( stdout );

    // Compute range and bearing of beacon relative to sensor
    //
    double dx = px - ox;
    double dy = py - oy;
    double r = sqrt(dx * dx + dy * dy);
    double b = NORMALIZE(atan2(dy, dx) - oth);
    double o = NORMALIZE(pth - oth);

    // filter out very acute angles of incidence as unreadable
    int bi = (int) ((b - scan_min) / scan_res);
    if (bi < 0 || bi >= laser.range_count)
      continue;
	
    //SHOULD CHANGE THESE RANGES BASED ON CURRENT LASER RESOLUTION!

    // Now see if it is within detection range
    //
    if (r > this->max_range_anon * DTOR(0.50) / scan_res)
      continue;
    if (r > this->max_range_id * DTOR(0.50) / scan_res)
      id = 0;

    // pack the beacon data into the export structure
    expBeacon.beacons[ expBeacon.beaconCount ].x = px;
    expBeacon.beacons[ expBeacon.beaconCount ].y = py;
    expBeacon.beacons[ expBeacon.beaconCount ].th = pth;
    expBeacon.beacons[ expBeacon.beaconCount ].id = id;
    expBeacon.beaconCount++;

    // Record beacons
    //
    if( beacon.count >=  ARRAYSIZE(beacon.fiducials) )
      {
	static bool print_warning = true;

	if( print_warning )
	  {
	    PRINT_WARN3( "FiducialFinder sees more beacons than will "
			 "fit in the player_fiducial_data_t structure "
			 "(%d beacons, space for %d records). The first "
			 "%d beacons will be returned, the rest ignored.\n"
			 "If you need to see more beacons, you can change "
			 "the size of the beacons array "
			 "(see player/server/player.h). Be wanred that "
			 "This will break "
			 "compatibility with P/S code compiled against "
			 "the standard player.h. \n\nThis message will "
			 "only appear once.", 
			 beacon.count, 
			 ARRAYSIZE(beacon.fiducials), 
			 ARRAYSIZE(beacon.fiducials) );

	    print_warning = false;
	  }
	beacon.count = ARRAYSIZE(beacon.fiducials);
      }
    
    beacon.fiducials[beacon.count].id = id;
    beacon.fiducials[beacon.count].pos[0] = (int) (r * 1000);
    beacon.fiducials[beacon.count].pos[1] = (int) RTOD(b);
    beacon.fiducials[beacon.count].pos[2] = (int) RTOD(o);
    beacon.count++;
  }

  // Get the byte ordering correct
  //
  for (int i = 0; i < beacon.count; i++)
  {
    beacon.fiducials[i].id = htons(beacon.fiducials[i].id);
    beacon.fiducials[i].pos[0] = htons(beacon.fiducials[i].pos[0]);
    beacon.fiducials[i].pos[1] = htons(beacon.fiducials[i].pos[1]);
    beacon.fiducials[i].pos[2] = htons(beacon.fiducials[i].pos[2]);
  }
  beacon.count = htons(beacon.count);
    
  // Write beacon buffer to shared mem
  // Note that we apply the laser data's timestamp to this data.
  //
  PutData( &beacon, sizeof(beacon) );
  this->time_sec = time_sec;
  this->time_usec = time_usec;
}

void CFiducialFinder::UpdateConfig( void )
{
  int len;
  void *client;
  char buffer[PLAYER_MAX_REQREP_SIZE];
  
  while (true)
    {
      len = GetConfig(&client, buffer, sizeof(buffer));
      if (len <= 0)
        break;
      
      switch (buffer[0])
        {
	case PLAYER_FIDUCIAL_GET_GEOM: 
	  // Return geometry of this sensor and size of most recently
	  // seen fiducial
	  player_fiducial_geom_t geom;	  

          geom.pose[0] = htons((short)(this->origin_x*1000.0));
          geom.pose[1] = htons((short)(this->origin_y*1000.0));
          geom.pose[2] = htons(0);
          geom.size[0] = htons((short)(this->size_x*1000.0));
          geom.size[1] = htons((short)(this->size_y*1000.0));
          // TODO: where should these dimensions come from?
          geom.fiducial_size[0] = htons(500);
          geom.fiducial_size[1] = htons(250);

  	  PutReply(client, PLAYER_MSGTYPE_RESP_ACK, NULL, &geom, sizeof(geom));
	  
	  break;
	  
	default:
	  PRINT_WARN1("got unknown fiducialfinder config request \"%c\"\n", 
		      buffer[0]);
	  PutReply(client, PLAYER_MSGTYPE_RESP_NACK);
	  break;
	}
    }
}

#ifdef INCLUDE_RTK2

///////////////////////////////////////////////////////////////////////////
// Initialise the rtk gui
void CFiducialFinder::RtkStartup()
{
  CPlayerEntity::RtkStartup();
  
  // Create a figure representing this object
  this->beacon_fig = rtk_fig_create(m_world->canvas, NULL, 49);

  // Set the color
  rtk_fig_color_rgb32(this->beacon_fig, this->color);
}


///////////////////////////////////////////////////////////////////////////
// Finalise the rtk gui
void CFiducialFinder::RtkShutdown()
{
  // Clean up the figure we created
  rtk_fig_destroy(this->beacon_fig);

  CPlayerEntity::RtkShutdown();
} 


///////////////////////////////////////////////////////////////////////////
// Update the rtk gui
void CFiducialFinder::RtkUpdate()
{
  CPlayerEntity::RtkUpdate();
 
  rtk_fig_clear(this->beacon_fig);
  
  // if a client is subscribed to this device
  if( Subscribed() > 0 && m_world->ShowDeviceData( this->lib_entry->type_num ) )
  {
    player_fiducial_data_t data;
    
    // attempt to get the right size chunk of data from the mmapped buffer
    if( GetData( &data, sizeof(data) ) == sizeof(data) )
    { 
      char text[16];

      // Get global pose
      double gx, gy, gth;
      GetGlobalPose(gx, gy, gth);

      rtk_fig_origin( this->beacon_fig, gx, gy, gth );

      int beacon_count = (int)ntohs(data.count);
      for( int b=0; b < beacon_count; b++ )
	{
	  int16_t id = ntohs((int16_t)data.fiducials[b].id);
	  uint16_t range_mm = ntohs(data.fiducials[b].pos[0]);
	  int16_t bearing_deg = ntohs(data.fiducials[b].pos[1]);
	  int16_t orient_deg = ntohs(data.fiducials[b].pos[2]);

	  double range = (double)range_mm / 1000.0;
	  double bearing = DTOR((double)bearing_deg);

	  double px = range * cos(bearing);
	  double py = range * sin(bearing);
	  double pa = DTOR((double)orient_deg);

	  rtk_fig_line( this->beacon_fig, 0, 0, px, py );	

	  // TODO: use the configuration info to determine beacon size
	  // for now we use these canned values
	  double wx = 0.05;
	  double wy = 0.40;
	  
	  rtk_fig_rectangle(this->beacon_fig, px, py, pa, wx, wy, 0);
	  rtk_fig_arrow(this->beacon_fig, px, py, pa, wy, 0.10);
	  snprintf(text, sizeof(text), "  %d", id);
	  rtk_fig_text(this->beacon_fig, px, py, pa, text);
	}  
    }
    //else
    //  puts( "subscribed but no data avail!!!!!!!!!!!!!!" );
  }
}
#endif
