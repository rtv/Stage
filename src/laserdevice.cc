//////////////////////////////////////////////////////////////////////////
//
// File: laserdevice.cc
// Author: Andrew Howard
// Date: 28 Nov 2000
// Desc: Simulates the Player CLaserDevice (the SICK laser)
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/laserdevice.cc,v $
//  $Author: inspectorg $
//  $Revision: 1.44 $
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

#define DEBUG
#undef VERBOSE

#include <iostream.h>
#include <stage.h>
#include <math.h>
#include "world.hh"
#include "laserdevice.hh"
#include "raytrace.hh"

#define DEFAULT_RES 0.5
#define DEFAULT_MIN -90
#define DEFAULT_MAX +90

///////////////////////////////////////////////////////////////////////////
// Default constructor
//
CLaserDevice::CLaserDevice(CWorld *world, 
			   CEntity *parent )
        : CEntity(world, parent )
{
  // set the Player IO sizes correctly for this type of Entity
  m_data_len    = sizeof( player_laser_data_t );
  m_command_len = 0;
  m_config_len  = sizeof( player_laser_config_t );
  
  m_player_type = PLAYER_LASER_CODE; // from player's messages.h
  m_stage_type = LaserTurretType;

  SetColor(LASER_COLOR);
  
  // Default visibility settings
  this->laser_return = LaserReflect;
  this->sonar_return = 0;
  this->obstacle_return = 0;
  
  // Default laser simulation settings
  this->scan_rate = 360 / 0.200; // 5Hz
  this->min_res =  DTOR(0.25);
  this->max_range = 8.0;

  // Current laser data configuration
  this->scan_res = DTOR(DEFAULT_RES);
  this->scan_min = DTOR(DEFAULT_MIN);
  this->scan_max = DTOR(DEFAULT_MAX);
  this->scan_count = 361;
  this->intensity = false;

  m_last_update = 0;
  
  // Dimensions of laser
  this->shape = ShapeRect;
  this->size_x = 0.155;
  this->size_y = 0.155;
  
#ifdef INCLUDE_RTK
  this->hit_count = 0;
#endif

}


///////////////////////////////////////////////////////////////////////////
// Load the entity from the world file
bool CLaserDevice::Load(CWorldFile *worldfile, int section)
{
  if (!CEntity::Load(worldfile, section))
    return false;

  // Minimum laser resolution
  this->min_res = worldfile->ReadAngle(0, "laser_min_res", this->min_res);
  this->min_res = worldfile->ReadAngle(section, "min_res", this->min_res);

  // Maximum laser range
  this->max_range = worldfile->ReadLength(0, "laser_max_range", this->max_range);
  this->max_range = worldfile->ReadLength(section, "max_range", this->max_range);
  
  // Laser scan rate (samples/sec)
  this->scan_rate = worldfile->ReadFloat(0, "laser_scan_rate", this->scan_rate);
  this->scan_rate = worldfile->ReadFloat(section, "scan_rate", this->scan_rate);
  
  return true;
}


///////////////////////////////////////////////////////////////////////////
// Update the laser data
//
void CLaserDevice::Update( double sim_time )
{
  CEntity::Update( sim_time );

  ASSERT(m_world != NULL);
    
  // UPDATE OUR RENDERING
  double x, y, th;
  GetGlobalPose( x,y,th );
    
  // if we've moved
  ReMap(x, y, th);
    
  // UPDATE OUR SENSOR DATA

  // Check to see if it's time to update the laser scan
  double interval = this->scan_count / this->scan_rate;
  if( sim_time - m_last_update > interval )
  {
    m_last_update = sim_time;
	
    if( Subscribed() )
    {
      // Check to see if the configuration has changed
      //
      CheckConfig();

      // Generate new scan data and copy to data buffer
      player_laser_data_t scan_data;
      GenerateScanData( &scan_data );
      PutData( &scan_data, sizeof( scan_data) );
    }
    else
    {
      // If not subscribed,
      // reset configuration to default.
      this->scan_res = DTOR(DEFAULT_RES);
      this->scan_min = DTOR(DEFAULT_MIN);
      this->scan_max = DTOR(DEFAULT_MAX);
      this->scan_count = 361;
      this->intensity = false;

      // empty the laser beacon list
      m_visible_beacons.clear();
    }
  }
}

///////////////////////////////////////////////////////////////////////////
// Check to see if the configuration has changed
//
bool CLaserDevice::CheckConfig()
{
  player_laser_config_t config;
  if (GetConfig( &config, sizeof(config) ) == 0)
    return false;  

  // Swap some bytes
  //
  config.resolution = ntohs(config.resolution);
  config.min_angle = ntohs(config.min_angle);
  config.max_angle = ntohs(config.max_angle);

  // Emulate behaviour of SICK laser range finder
  //
  if (config.resolution == 25)
  {
    config.min_angle = max((int)(config.min_angle), -5000);
    config.min_angle = min((int)config.min_angle, +5000);
    config.max_angle = max((int)config.max_angle, -5000);
    config.max_angle = min((int)config.max_angle, +5000);
        
    this->scan_res = DTOR((double) config.resolution / 100.0);
    this->scan_min = DTOR((double) config.min_angle / 100.0);
    this->scan_max = DTOR((double) config.max_angle / 100.0);
    this->scan_count = (int) ((this->scan_max - this->scan_min) / this->scan_res) + 1;
  }
  else if (config.resolution == 50 || config.resolution == 100)
  {
    if (abs(config.min_angle) > 9000 || abs(config.max_angle) > 9000)
      PRINT_MSG("warning: invalid laser configuration request");
        
    this->scan_res = DTOR((double) config.resolution / 100.0);
    this->scan_min = DTOR((double) config.min_angle / 100.0);
    this->scan_max = DTOR((double) config.max_angle / 100.0);
    this->scan_count = (int) ((this->scan_max - this->scan_min) / this->scan_res) + 1;
  }
  else
  {
    // Ignore invalid configurations
    //  
    PRINT_MSG("invalid laser configuration request");
    return false;
  }
        
  this->intensity = config.intensity;

  return true;
}


///////////////////////////////////////////////////////////////////////////
// Generate scan data
//
bool CLaserDevice::GenerateScanData( player_laser_data_t *data )
{    
  // Get the pose of the laser in the global cs
  //
  double x, y, th;
  GetGlobalPose(x, y, th);
  
  // See how many scan readings to interpolate.
  // To save time generating laser scans, we can
  // generate a scan with lower resolution and interpolate
  // the intermediate values.
  // We will interpolate <skip> out of <skip+1> readings.
  int skip = (int) (this->min_res / this->scan_res - 0.5);

#ifdef INCLUDE_RTK
  // Initialise gui data
  //
  this->hit_count = 0;
#endif

  // initialise our beacon detecting array
  m_visible_beacons.clear(); 

  // Set the header part of the data packet
  //
  data->range_count = htons(this->scan_count);
  data->resolution = htons((int) (100 * RTOD(this->scan_res)));
  data->min_angle = htons((int) (100 * RTOD(this->scan_min)));
  data->max_angle = htons((int) (100 * RTOD(this->scan_max)));

  // Make sure the data buffer is big enough
  //
  ASSERT(this->scan_count <= ARRAYSIZE(data->ranges));
            
  // Do each scan
  //
  for (int s = 0; s < this->scan_count;)
  {
    double ox = x;
    double oy = y;
    double oth = th;

    // Compute parameters of scan line
    double bearing = s * this->scan_res + this->scan_min;
    double pth = oth + bearing;
    CLineIterator lit( ox, oy, pth, this->max_range, 
                       m_world->ppm, m_world->matrix, PointToBearingRange );
	
    CEntity* ent;
    double range = this->max_range;
	
    while( (ent = lit.GetNextEntity()) ) 
    {
      // Ignore ourself, things which are attached to us,
      // and things that we are attached to.
      // The latter is useful if you want to stack beacons
      // on the laser or the laser on somethine else.
      if (ent == this || this->IsDescendent(ent) || ent->IsDescendent(this))
        continue;

      // Construct a list of beacons we have seen
      if( ent->m_stage_type == LaserBeaconType )
        m_visible_beacons.push_front( (int)ent );

      // Stop looking when we see something
      if(ent->laser_return != LaserTransparent) 
      {
        range = lit.GetRange();
        break;
      }	
    }
	
    // Construct the return value for the laser
    uint16_t v = (uint16_t)(1000.0 * range);
    if( ent && ent->laser_return >= LaserBright1 )
      v = v | (((uint16_t)1) << 13); 

    // Set the range
    data->ranges[s++] = htons(v);

    // Skip some values to save time
    for (int i = 0; i < skip && s < this->scan_count; i++)
      data->ranges[s++] = htons(v);
	
#ifdef INCLUDE_RTK
    // Update the gui data
    // Show laser just a bit short of objects
    this->hit[this->hit_count][0] = ox + (range - 0.1) * cos(pth);
    this->hit[this->hit_count][1] = oy + (range - 0.1) * sin(pth);
    this->hit_count++;
#endif
  }

  // remove all duplicates from the beacon list
  m_visible_beacons.unique(); 

  return true;
}


#ifdef INCLUDE_RTK

///////////////////////////////////////////////////////////////////////////
// Process GUI update messages
//
void CLaserDevice::OnUiUpdate(RtkUiDrawData *data)
{
  CEntity::OnUiUpdate(data);

  data->begin_section("global", "laser");

  if (data->draw_layer("", true))
      DrawTurret(data);

  if (data->draw_layer("outline", false, 49))
  {
    if(Subscribed())
      DrawScan(data, 0);
  } 

  if (data->draw_layer("coverage", false, 49))
  {
    if(Subscribed())
      DrawScan(data, 1);
  }

  data->end_section();
}


///////////////////////////////////////////////////////////////////////////
// Process GUI mouse messages
//
void CLaserDevice::OnUiMouse(RtkUiMouseData *data)
{
    CEntity::OnUiMouse(data);
}


///////////////////////////////////////////////////////////////////////////
// Draw the laser turret
//
void CLaserDevice::DrawTurret(RtkUiDrawData *data)
{
    data->set_color(RTK_RGB(m_color.red, m_color.green, m_color.blue));

    // Turret dimensions
    double dx = m_size_x;
    double dy = m_size_y;

    // Get global pose
    double gx, gy, gth;
    GetGlobalPose(gx, gy, gth);
    
    // Draw the outline of the turret
    data->ex_rectangle(gx, gy, gth, dx, dy); 
}


///////////////////////////////////////////////////////////////////////////
// Draw the laser scan
//
void CLaserDevice::DrawScan(RtkUiDrawData *data, int style)
{
  data->set_color(RTK_RGB(m_color.red, m_color.green, m_color.blue));
  
  // Get global pose
  double gx, gy, gth;
  GetGlobalPose(gx, gy, gth);

  // Draw the scan outline
  if (style == 0)
  {
    double qx, qy;
    qx = gx;
    qy = gy;
    
    for (int i = 0; i < this->hit_count; i++)
    {
      double px = this->hit[i][0];
      double py = this->hit[i][1];
      data->line(qx, qy, px, py);
      qx = px;
      qy = py;
    }
    data->line(qx, qy, gx, gy);
  }

  // Draw the dense scan coverage
  else
  {
    for (int i = 0; i < this->hit_count; i++)
    {
      double px = this->hit[i][0];
      double py = this->hit[i][1];
      data->line(gx, gy, px, py);
    }
  }
}

#endif




