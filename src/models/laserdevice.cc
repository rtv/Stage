/*
 *  Stage : a multi-robot simulator.
 *  Copyright (C) 2001, 2002 Richard Vaughan, Andrew Howard and Brian Gerkey.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */
/*
 * Desc: Simulates a scanning laser range finder (SICK LMS200)
 * Author: Andrew Howard, Richard Vaughan
 * Date: 28 Nov 2000
 * CVS info: $Id: laserdevice.cc,v 1.10.2.1 2003-02-12 08:48:48 rtv Exp $
 */

#define DEBUG
#undef VERBOSE

#include <iostream>
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
CLaserDevice::CLaserDevice(LibraryItem* libit, CWorld *world, CEntity *parent )
    : CPlayerEntity(libit, world, parent )
{
  // set the Player IO sizes correctly for this type of Entity
  m_data_len    = sizeof( player_laser_data_t );
  m_command_len = 0;
  m_config_len  = 1;
  m_reply_len  = 1;
  
  m_player.code = PLAYER_LASER_CODE; // from player's messages.h
  
  // Default visibility settings
  this->laser_return = LaserVisible;
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
  
#ifdef INCLUDE_RTK2
  this->scan_fig = NULL;
#endif
}

///////////////////////////////////////////////////////////////////////////
// Startup routine
//
bool CLaserDevice::Startup()
{
  if (!CPlayerEntity::Startup())
    return false;

  return true;
}


///////////////////////////////////////////////////////////////////////////
// Load the entity from the world file
bool CLaserDevice::Load(CWorldFile *worldfile, int section)
{
  if (!CPlayerEntity::Load(worldfile, section))
    return false;

  // Read laser settings
  this->min_res = worldfile->ReadAngle(section, "min_res", this->min_res);
  this->max_range = worldfile->ReadLength(section, "max_range", this->max_range);

  this->scan_rate = worldfile->ReadFloat(section, "scan_rate", this->scan_rate);
  this->scan_min = worldfile->ReadAngle(section, "scan_min", this->scan_min);
  this->scan_max = worldfile->ReadAngle(section, "scan_max", this->scan_max);
  this->scan_res = worldfile->ReadAngle(section, "scan_res", this->scan_res);

  this->scan_count = (int) ((this->scan_max - this->scan_min) / this->scan_res) + 1;
  
  return true;
}


///////////////////////////////////////////////////////////////////////////
// Update the laser data
void CLaserDevice::Update( double sim_time )
{
  CPlayerEntity::Update( sim_time );

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
	
    if( Subscribed() > 0 )
    {
      // Check to see if the configuration has changed
      CheckConfig();
      
      // Generate new scan data and copy to data buffer
      player_laser_data_t scan_data;
      GenerateScanData( &scan_data );
      PutData( &scan_data, sizeof( scan_data) );
	
      // we may need to tell clients about this data
      SetDirty( PropData, 1 );
    }
    else
    {
      // reset configuration to default.
      /* REMOVE
      this->scan_res = DTOR(DEFAULT_RES);
      this->scan_min = DTOR(DEFAULT_MIN);
      this->scan_max = DTOR(DEFAULT_MAX);
      this->scan_count = 361;
      this->intensity = false;
      */
	
      // and indicate that the data is no longer available
      //Lock();
      //m_info_io->data_avail = 0;
      //Unlock();
	
      // empty the laser beacon list
      this->visible_beacons.clear();
    }
  }
}


///////////////////////////////////////////////////////////////////////////
// Check to see if the configuration has changed
// This currently emulates the behaviour of SICK laser range finder.
bool CLaserDevice::CheckConfig()
{
  int len;
  void* client;
  char buffer[PLAYER_MAX_REQREP_SIZE];
  player_laser_config_t config;
  player_laser_geom_t geom;

  while (true)
  {
    len = GetConfig(&client, &buffer, sizeof(buffer));
    if (len <= 0)
      break;

    switch (buffer[0])
    {
      case PLAYER_LASER_SET_CONFIG:

        if (len < (int)sizeof(config))
        {
          PRINT_WARN2("request has unexpected len (%d < %d)", len, sizeof(config));
          break;
        }
        
        memcpy(&config, buffer, sizeof(config));
        config.resolution = ntohs(config.resolution);
        config.min_angle = ntohs(config.min_angle);
        config.max_angle = ntohs(config.max_angle);

        if (config.resolution == 25)
        {
          if (abs(config.min_angle) > 5000 || abs(config.max_angle) > 5000)
          {
            PRINT_MSG("warning: invalid laser configuration request");
            PutReply(client, PLAYER_MSGTYPE_RESP_NACK);
          }
          else
          {
            this->scan_res = DTOR((double) config.resolution / 100.0);
            this->scan_min = DTOR((double) config.min_angle / 100.0);
            this->scan_max = DTOR((double) config.max_angle / 100.0);
            this->scan_count = (int) ((this->scan_max - this->scan_min) / this->scan_res) + 1;
            this->intensity = config.intensity;
            PutReply(client, PLAYER_MSGTYPE_RESP_ACK);
          }
        }
        else if (config.resolution == 50 || config.resolution == 100)
        {
          if (abs(config.min_angle) > 9000 || abs(config.max_angle) > 9000)
          {
            PRINT_MSG("warning: invalid laser configuration request");
            PutReply(client, PLAYER_MSGTYPE_RESP_NACK);
          }
          else
          {
            this->scan_res = DTOR((double) config.resolution / 100.0);
            this->scan_min = DTOR((double) config.min_angle / 100.0);
            this->scan_max = DTOR((double) config.max_angle / 100.0);
            this->scan_count = (int) ((this->scan_max - this->scan_min) / this->scan_res) + 1;
            this->intensity = config.intensity;
            PutReply(client, PLAYER_MSGTYPE_RESP_ACK);
          }
        }
        else
        {
          PRINT_MSG("warning: invalid laser configuration request");
          PutReply(client, PLAYER_MSGTYPE_RESP_NACK);
        }
        break;

      case PLAYER_LASER_GET_CONFIG:
        // Return the laser configuration
        config.resolution = htons((int) (RTOD(this->scan_res) * 100));
        config.min_angle = htons((unsigned int) (int) (RTOD(this->scan_min) * 100));
        config.max_angle = htons((unsigned int) (int) (RTOD(this->scan_max) * 100));
        config.intensity = this->intensity;
        PutReply(client, PLAYER_MSGTYPE_RESP_ACK, NULL, &config, sizeof(config));
        break;

      case PLAYER_LASER_GET_GEOM:
        // Return the laser geometry
        geom.pose[0] = htons((short) (this->origin_x * 1000));
        geom.pose[1] = htons((short) (this->origin_y * 1000));
        geom.pose[2] = 0;
        geom.size[0] = htons((short) (this->size_x * 1000));
        geom.size[1] = htons((short) (this->size_y * 1000));
        PutReply(client, PLAYER_MSGTYPE_RESP_ACK, NULL, &geom, sizeof(geom));
        break;

      default:
        PRINT_WARN1("invalid laser configuration request [%c]", buffer[0]);
        PutReply(client, PLAYER_MSGTYPE_RESP_NACK);
        break;
    }
  }
  return true;
}


///////////////////////////////////////////////////////////////////////////
// Generate scan data
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

  // initialise our beacon detecting array
  this->visible_beacons.clear(); 

  // Set the header part of the data packet
  data->range_count = htons(this->scan_count);
  data->resolution = htons((int) (100 * RTOD(this->scan_res)));
  data->min_angle = htons((int) (100 * RTOD(this->scan_min)));
  data->max_angle = htons((int) (100 * RTOD(this->scan_max)));

  // Make sure the data buffer is big enough
  ASSERT(this->scan_count <= ARRAYSIZE(data->ranges));
  
  // Do each scan
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
      // and things that we are attached to (except root)
      // The latter is useful if you want to stack beacons
      // on the laser or the laser on somethine else.
      if (ent == this || this->IsDescendent(ent) || 
          (ent != m_world->root && ent->IsDescendent(this)))
        continue;

      // Construct a list of entities that have a fiducial value
      if( ent->fiducial_return != FiducialNone )
      {
        this->visible_beacons.push_front( (int)ent );
	 
        // remove duplicates
        this->visible_beacons.sort();
        this->visible_beacons.unique();
      }

      // Stop looking when we see something
      if(ent->laser_return != LaserTransparent) 
      {
        range = lit.GetRange();
        break;
      }	
    }
	
    // Construct the return value for the laser
    uint16_t v = (uint16_t) (1000.0 * range);
    data->ranges[s] = htons(v);
    if (ent && ent->laser_return >= LaserBright)
      data->intensity[s] = 1;
    else
      data->intensity[s] = 0;
    s++;

    // Skip some values to save time
    for (int i = 0; i < skip && s < this->scan_count; i++)
    {
      data->ranges[s] = data->ranges[s - 1];
      data->intensity[s] = data->intensity[s - 1];
      s++;
    }
  }
  
  // remove all duplicates from the beacon list
  this->visible_beacons.unique(); 

  return true;
}


#ifdef INCLUDE_RTK2

///////////////////////////////////////////////////////////////////////////
// Initialise the rtk gui
void CLaserDevice::RtkStartup()
{
  CPlayerEntity::RtkStartup();
  
  // Create a figure representing this object
  this->scan_fig = rtk_fig_create(m_world->canvas, NULL, 49);

  // Set the color
  rtk_fig_color_rgb32(this->scan_fig, this->color);

}


///////////////////////////////////////////////////////////////////////////
// Finalise the rtk gui
void CLaserDevice::RtkShutdown()
{
  // Clean up the figure we created
  rtk_fig_destroy(this->scan_fig);

  CPlayerEntity::RtkShutdown();
} 


///////////////////////////////////////////////////////////////////////////
// Update the rtk gui
void CLaserDevice::RtkUpdate()
{
  CPlayerEntity::RtkUpdate();
#if 0 
  rtk_fig_clear(this->scan_fig);
   
  // draw a figure from the data in the data buffer
  // we might have put it there ourselves, or another stage
  // might have generated it
  
  // this is a good way to see the _real_ output of Stage, rather than
  // some intermediate values. also it allows us to view data that was
  // generated elsewhere, without this Stage being subscribed to a
  // device.  so even though it does use a few more flops, i think
  // it's the Right Thing to do - RTV.
  
  //gth -= M_PI / 2.0;
  
  // if a client is subscribed to this device
  if( Subscribed() > 0 && m_world->ShowDeviceData( this->lib_entry->type_num) )
  {
    player_laser_data_t data;
    
    // attempt to get the right size chunk of data from the mmapped buffer
    if( GetData( &data, sizeof(data) ) == sizeof(data) )
    { 
      // Get global pose
      double gx, gy, gth;
      GetGlobalPose(gx, gy, gth);

      rtk_fig_origin( this->scan_fig, gx, gy, gth);
      
      // we got it, so parse out the data and display it
      short min_ang_deg = ntohs(data.min_angle);
      short max_ang_deg = ntohs(data.max_angle);
      unsigned short samples = ntohs(data.range_count); 
	
      double min_ang_rad = DTOR( (double)min_ang_deg ) / 100.0;
      double max_ang_rad = DTOR( (double)max_ang_deg ) / 100.0;

      double incr = (double)(max_ang_rad - min_ang_rad) / (double)samples;
	
      double lx = 0.0;
      double ly = 0.0;

      double bearing = min_ang_rad;
      for( int i=0; i < (int)samples; i++ )
      {
        // get range, converting from mm to m
        unsigned short range_mm = ntohs(data.ranges[i]) & 0x1FFF;
        double range_m = (double)range_mm / 1000.0;
	  
        //if( range_m == this->max_range ) range_m = 0;

        bearing += incr;

        //if( range_m < this->max_range )
        //{
        double px = range_m * cos(bearing);
        double py = range_m * sin(bearing);
	  
        //rtk_fig_line(this->scan_fig, 0.0, 0.0, px, py);
	    
        rtk_fig_line( this->scan_fig, lx, ly, px, py );
        lx = px;
        ly = py;
	    
        // add little boxes at high intensities (like in playerv)
        if(  (unsigned char)(ntohs(data.ranges[i]) >> 13) > 0 )
	      {
          rtk_fig_rectangle(this->scan_fig, px, py, 0, 0.05, 0.05, 1);
          //rtk_fig_line( this->scan_fig, 0,0,px, py );
	      }
	  
      }

      // RTV - son demo
      rtk_fig_line( this->scan_fig, 0.0, 0.0, lx, ly );
	
    }
    //else
    //  puts( "subscribed but no data avail!!!!!!!!!!!!!!" );
  }
#endif
}
#endif

