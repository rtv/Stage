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
 * Desc: Simulates a sonar ring.
 * Author: Andrew Howard, Richard Vaughan
 * Date: 28 Nov 2000
 * CVS info: $Id: sonardevice.cc,v 1.39 2002-10-26 00:52:38 rtv Exp $
 */

#include <math.h>
#include "world.hh"
#include "sonardevice.hh"
#include "raytrace.hh"

#include "world.hh"

// register this device type with the library
CEntity sonar_bootstrap( "sonar", 
			 SonarType, 
			 (void*)CSonarDevice::Creator );

// constructor
CSonarDevice::CSonarDevice(CWorld *world, CEntity *parent )
  : CPlayerEntity(world, parent )    
{
  // set the Player IO sizes correctly for this type of Entity
  m_data_len    = sizeof( player_sonar_data_t );
  m_command_len = 0;
  m_config_len  = 1;
  m_reply_len  = 1;
  
  m_player.code = PLAYER_SONAR_CODE; // from player's messages.h
  this->stage_type = SonarType;
  this->color = ::LookupColor(SONAR_COLOR);

  this->min_range = 0.20;
  this->max_range = 5.0;

  this->power_on = true;

  // Initialise the sonar poses to default values
  this->sonar_count = SONARSAMPLES;
  for (int i = 0; i < this->sonar_count; i++)
  {
    this->sonars[i][0] = 0;
    this->sonars[i][1] = 0;
    this->sonars[i][2] = i * 2 * M_PI / this->sonar_count;
  }
  
  // zero the data
  memset( &this->data, 0, sizeof(this->data) );
}

///////////////////////////////////////////////////////////////////////////
// Startup routine
//
bool CSonarDevice::Startup()
{
  if (!CPlayerEntity::Startup())
    return false;

  SetDriverName("p2os_sonar");
  return true;
}

///////////////////////////////////////////////////////////////////////////
// Load the entity from the world file
bool CSonarDevice::Load(CWorldFile *worldfile, int section)
{
  int i;
  int scount;
  char key[256];
  
  if (!CPlayerEntity::Load(worldfile, section))
    return false;

  // Load sonar min and max range
  this->min_range = worldfile->ReadLength(section, "min_range", this->min_range);
  this->max_range = worldfile->ReadLength(section, "max_range", this->max_range);
  
  // power on or off? (TODO - have this setting saved)
  this->power_on = worldfile->ReadBool(section, "power_on", this->power_on );

  // Load the geometry of the sonar ring
  scount = worldfile->ReadInt(section, "scount", 0);
  if (scount > 0)
  {
    this->sonar_count = scount;
    for (i = 0; i < this->sonar_count; i++)
    {
      snprintf(key, sizeof(key), "spose[%d]", i);
      this->sonars[i][0] = worldfile->ReadTupleLength(section, key, 0, 0);
      this->sonars[i][1] = worldfile->ReadTupleLength(section, key, 1, 0);
      this->sonars[i][2] = worldfile->ReadTupleAngle(section, key, 2, 0);
    }
  }
  return true;
}


///////////////////////////////////////////////////////////////////////////
// Update the sonar data
void CSonarDevice::Update( double sim_time ) 
{
  CPlayerEntity::Update( sim_time );

  // dump out if noone is subscribed
  if(!Subscribed())
    return;

  // Check to see if it is time to update
  //  - if not, return right away.
  if( sim_time - m_last_update < m_interval)
    return;
  m_last_update = sim_time;

  // Process configuration requests
  UpdateConfig();
  
  // Check bounds
  assert((size_t) this->sonar_count <= ARRAYSIZE(this->data.ranges));
  

  if( this->power_on )
    // Do each sonar
    for (int s = 0; s < this->sonar_count; s++)
      {
	// Compute parameters of scan line
	double ox = this->sonars[s][0];
	double oy = this->sonars[s][1];
	double oth = this->sonars[s][2];
	LocalToGlobal(ox, oy, oth);
	
	double range = this->max_range;
	
	CLineIterator lit( ox, oy, oth, this->max_range, 
			   m_world->ppm, m_world->matrix, PointToBearingRange );
	CEntity* ent;
	
	while( (ent = lit.GetNextEntity()) )
	  {
	    if( ent != this && ent != m_parent_entity && ent->sonar_return ) 
	      {
		range = lit.GetRange();
		break;
	      }
	  }
	
	uint16_t v = (uint16_t)(1000.0 * range);
	
	// Store range in mm in network byte order
	this->data.range_count = htons(this->sonar_count);
	this->data.ranges[s] = htons(v);
      }

  PutData( &this->data, sizeof(this->data) );

  return;
}


///////////////////////////////////////////////////////////////////////////
// Process configuration requests.
void CSonarDevice::UpdateConfig()
{
  int s, len;
  void* client;
  char buffer[PLAYER_MAX_REQREP_SIZE];
  player_sonar_geom_t geom;
  //player_sonar_power_config_t power;

  while (true)
  {
    len = GetConfig(&client, &buffer, sizeof(buffer));
    if (len <= 0)
      break;

    switch (buffer[0])
    {
      case PLAYER_SONAR_POWER_REQ:
        // we got a sonar power config
	// set the new power status
	this->power_on = ((player_sonar_power_config_t*)buffer)->value;
        PutReply(client, PLAYER_MSGTYPE_RESP_ACK);
        break;

      case PLAYER_SONAR_GET_GEOM_REQ:
        // Return the sonar geometry
        assert(this->sonar_count <= ARRAYSIZE(geom.poses));
        geom.pose_count = htons(this->sonar_count);
        for (s = 0; s < this->sonar_count; s++)
        {
          geom.poses[s][0] = htons((short) (this->sonars[s][0] * 1000));
          geom.poses[s][1] = htons((short) (this->sonars[s][1] * 1000));
          geom.poses[s][2] = htons((short) (this->sonars[s][2] * 180 / M_PI));
        }
        PutReply(client, PLAYER_MSGTYPE_RESP_ACK, NULL, &geom, sizeof(geom));
        break;

      default:
        PRINT_WARN1("invalid sonar configuration request [%c]", buffer[0]);
        PutReply(client, PLAYER_MSGTYPE_RESP_NACK);
        break;
    }
  }
}

//size_t CSonarDevice::PutData( void* vdata, size_t len )
//{
  // export the data right away
//size_t retval = CPlayerEntity::PutData( vdata, len );
  
//player_sonar_data_t* data = (player_sonar_data_t*)vdata;
  

//return( retval );
//}



#ifdef INCLUDE_RTK2

///////////////////////////////////////////////////////////////////////////
// Initialise the rtk gui
void CSonarDevice::RtkStartup()
{
  CPlayerEntity::RtkStartup();
  
  // Create a figure representing this object
  this->scan_fig = rtk_fig_create(m_world->canvas, NULL, 49);

  // Set the color
  rtk_fig_color_rgb32(this->scan_fig, this->color);
}


///////////////////////////////////////////////////////////////////////////
// Finalise the rtk gui
void CSonarDevice::RtkShutdown()
{
  // Clean up the figure we created
  rtk_fig_destroy(this->scan_fig);

  CPlayerEntity::RtkShutdown();
} 


///////////////////////////////////////////////////////////////////////////
// Update the rtk gui
void CSonarDevice::RtkUpdate()
{
  CPlayerEntity::RtkUpdate();
   
  // Get global pose
  //double gx, gy, gth;
  //GetGlobalPose(gx, gy, gth);
  //rtk_fig_origin(this->scan_fig, gx, gy, gth );

  rtk_fig_clear(this->scan_fig);

  // see the comment in CLaserDevice for why this gets the data out of
  // the buffer instead of storing hit points in ::Update() - RTV
  player_sonar_data_t data;
  
  if( Subscribed() > 0 && m_world->ShowDeviceData( this->stage_type) )
  {
    size_t res = GetData( &data, sizeof(data));

    if( res == sizeof(data) )
    {
      for (int s = 0; s < this->sonar_count; s++)
      {
        double ox = this->sonars[s][0];
        double oy = this->sonars[s][1];
        double oth = this->sonars[s][2];
        LocalToGlobal(ox, oy, oth);
        
	// convert from integer mm to double m
        double range = (double)ntohs(data.ranges[s]) / 1000.0;

	//if( range < max_range )
	  {
	    double x1 = ox;
	    double y1 = oy;
	    double x2 = x1 + range * cos(oth); 
	    double y2 = y1 + range * sin(oth);       
	    
	    rtk_fig_line(this->scan_fig, x1, y1, x2, y2 );
	  }
      }
    }
    else
      PRINT_WARN2( "GET DATA RETURNED WRONG AMOUNT (%d/%d bytes)", res, sizeof(data) );
  }  
}

#endif

