///////////////////////////////////////////////////////////////////////////
//
// File: fiducialfinderdevice.cc
// Author: Richard Vaughan
// Date: 12 Jan 2000
// Desc: Simulates the laser-based beacon detector
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/models/fiducialfinderdevice.cc,v $
//  $Author: rtv $
//  $Revision: 1.5 $
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
#include <stage.h>
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

  m_interval = 0.2; // matches laserdevice

  // fill in the default values for these fake parameters
  m_bit_count = 8;
  m_bit_size = 50;
  m_zero_thresh = m_one_thresh = 60;

  // we have a notional size of fiducial that we work with.
  // unfortunately the player spec doesn't allow us to record the
  // sizes of individual fiducials so we just store the size of the
  // last one we saw, or (0,0) if we've never seen one.
  fiducial_width = fiducial_height = 0.0;
 
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
  
  // space to store information about the fiducials we find
  int f_count = 0;
  int f_ids[PLAYER_FIDUCIAL_MAX_SAMPLES];
  double f_poses[PLAYER_FIDUCIAL_MAX_SAMPLES][3];
  double f_pose_errors[PLAYER_FIDUCIAL_MAX_SAMPLES][3];
  
  f_count = 0; // initialise the count in the export structure

   
  // Search for beacons in the list generated by the laser
  // Saves us from searching the bitmap again
  //
  for( LaserBeaconList::iterator it = this->laser->visible_beacons.begin();
       it != this->laser->visible_beacons.end() &&  
	 f_count <= PLAYER_FIDUCIAL_MAX_SAMPLES;
       it++ )
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
      double range = sqrt(dx * dx + dy * dy);
      double bearing = NORMALIZE(atan2(dy, dx) - oth);
      double orientation = NORMALIZE(pth - oth);
      
      // filter out very acute angles of incidence as unreadable
      int bi = (int) ((bearing - scan_min) / scan_res);
      if (bi < 0 || bi >= laser.range_count)
	continue;
      
      //SHOULD CHANGE THESE RANGES BASED ON CURRENT LASER RESOLUTION!
      
      // Now see if it is within detection range
      //
      if (range > this->max_range_anon * DTOR(0.50) / scan_res)
	continue;
      if (range > this->max_range_id * DTOR(0.50) / scan_res)
	id = -1;
      
      // store the fiducial location in the structure (if there's room)
      f_ids[f_count] = id;
      
      f_poses[f_count][0] = range; 
      f_poses[f_count][1] = bearing; 
      f_poses[f_count][2] = orientation; 
      
      f_pose_errors[f_count][0] = 0; // RANGE
      f_pose_errors[f_count][1] = 0; // BEARING
      f_pose_errors[f_count][2] = 0; // ORIENTATION
      
      f_count++;
      
      // record the size of this most recent target
      // as we can;t return individual sizes.
      this->fiducial_width = nbeacon->size_x;
      this->fiducial_height = nbeacon->size_y;
    }
  
  // pack into the network-safe player structure
  player_fiducial_data_t data;
  FiducialDataPack( &data, f_count, f_ids, f_poses, f_pose_errors ); 
  
  // Write beacon buffer to shared mem
  // Note that we apply the laser data's timestamp to this data.
  //
  PutData( &data, sizeof(data) );
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

	  FiducialGeomPack( &geom, 
			    this->origin_x, this->origin_y, 0,
			    this->size_x, this->size_y,
			    this->fiducial_width,
			    this->fiducial_height );
	  
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
      // unpack the data into native formats
      int count;
      int ids[PLAYER_FIDUCIAL_MAX_SAMPLES];
      double poses[PLAYER_FIDUCIAL_MAX_SAMPLES][3];
      double errors[PLAYER_FIDUCIAL_MAX_SAMPLES][3];

      FiducialDataUnpack( &data, &count, ids, poses, errors );

      char text[16];

      // Get global pose
      double gx, gy, gth;
      GetGlobalPose(gx, gy, gth);

      rtk_fig_origin( this->beacon_fig, gx, gy, gth );

      for( int b=0; b < count; b++ )
	{
	  double range = poses[b][0];
	  double bearing = poses[b][1];
	  double orientation = poses[b][2];

	  double px = range * cos(bearing);
	  double py = range * sin(bearing);
	  double pa = orientation;

	  rtk_fig_line( this->beacon_fig, 0, 0, px, py );	

	  double wx = this->fiducial_width;
	  double wy = this->fiducial_height;
	  
	  rtk_fig_rectangle(this->beacon_fig, px, py, pa, wx, wy, 0);
	  rtk_fig_arrow(this->beacon_fig, px, py, pa, wy, 0.10);
	  snprintf(text, sizeof(text), "  %d", ids[b] );
	  rtk_fig_text(this->beacon_fig, px, py, pa, text);
	}  
    }
    //else
    //  puts( "subscribed but no data avail!!!!!!!!!!!!!!" );
  }
}
#endif
