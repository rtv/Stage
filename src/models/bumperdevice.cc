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
 * Desc: Simulates a set of binary contact switches
 *        useful as bumpers or whiskers.
 * Author: Richard Vaughan
 * Date: 28 Nov 2000
 * CVS info: $Id: bumperdevice.cc,v 1.2 2002-11-02 02:00:52 inspectorg Exp $
 */

#include <math.h>
#include "world.hh"
#include "bumperdevice.hh"
#include "raytrace.hh"
//#include "player.h"
#include "world.hh"

// constructor
CBumperDevice::CBumperDevice( LibraryItem *libit, 
			    CWorld *world, CEntity *parent )
  : CPlayerEntity(libit, world, parent )    
{
  // set the Player IO sizes correctly for this type of Entity
  m_data_len    = sizeof( player_bumper_data_t );
  m_command_len = 0;
  m_config_len  = 1; // can request geometry of bumper set
  m_reply_len  = 1;
  
  m_player.code = PLAYER_BUMPER_CODE; // from player.h

  m_interval = 0.1; // update interval in seconds 

  // zero the config data
  memset( this->bumpers, 0, 
	  sizeof(this->bumpers[0])*PLAYER_BUMPER_MAX_SAMPLES );

  // a sensible default is 1 straight 10cm sensor facing forwards
  this->bumper_count = 1;
  this->bumpers[0].x_offset = 0; 
  this->bumpers[0].y_offset = 0; 
  this->bumpers[0].th_offset = 0; 
  this->bumpers[0].length = 0.1; 
  this->bumpers[0].radius = 0; 
}

///////////////////////////////////////////////////////////////////////////
// Startup routine
//
bool CBumperDevice::Startup()
{
  if (!CPlayerEntity::Startup())
    return false;

  return true;
}

///////////////////////////////////////////////////////////////////////////
// Load the entity from the world file
bool CBumperDevice::Load(CWorldFile *worldfile, int section)
{
  int i;
  int scount;
  char key[256];
  
  if (!CPlayerEntity::Load(worldfile, section))
    return false;

  // Load the geometry of the bumper ring
  scount = worldfile->ReadInt(section, "bcount", 0);
  if (scount > 0)
  {
    this->bumper_count = scount;
    for (i = 0; i < this->bumper_count; i++)
    {
      snprintf(key, sizeof(key), "bpose[%d]", i);

      this->bumpers[i].x_offset = 
        worldfile->ReadTupleLength(section, key, 0,  
                                   this->bumpers[i].x_offset );

      this->bumpers[i].y_offset = 
        worldfile->ReadTupleLength(section, key, 1, 
                                   this->bumpers[i].y_offset );

      this->bumpers[i].th_offset = 
        worldfile->ReadTupleAngle(section, key, 2, 
                                  this->bumpers[i].th_offset );

      this->bumpers[i].length = 
        worldfile->ReadTupleLength(section, key, 3, 
                                   this->bumpers[i].length );
      
      this->bumpers[i].radius = 
        worldfile->ReadTupleLength(section, key, 4,
                                   this->bumpers[i].radius );
    }
  }
  
  //printf( "bumper count %d\n", this->bumper_count );

  // convert those specs into internal scanline repn.
  for (i = 0; i < this->bumper_count; i++)
  {
    if( this->bumpers[i].radius == 0 )
    { 
      //printf( "bumper #%d is a straight line\n", i );

      // straight line scan
      // shift from center to end of arc      
      double cx = this->bumpers[i].x_offset ;
      double cy = this->bumpers[i].y_offset;
      // rotate the angle 90 deg
      double oth = this->bumpers[i].th_offset + M_PI/2.0;
      double len = this->bumpers[i].length;
	  
      double dx = len/2.0 * cos(oth);
      double dy = len/2.0 * sin(oth);
	  
      this->scanlines[i].x_offset = cx - dx;
      this->scanlines[i].y_offset = cy - dy;
      this->scanlines[i].th_offset = oth;
      this->scanlines[i].length = len;
      this->scanlines[i].radius = this->bumpers[i].radius;
    }
    else // arc scan
    {
      //printf( "bumper #%d is an arc\n", i );

      // no need for processing for arcs - just copy the setup in
      memcpy( &this->scanlines[i], 
              &this->bumpers[i], 
              sizeof(this->bumpers[i]) );
    }
  }

  return true;
}


///////////////////////////////////////////////////////////////////////////
// Update the bumper data
void CBumperDevice::Update( double sim_time ) 
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
  
  // the data to export to player
  player_bumper_data_t data;

  // Check bounds
  assert((size_t) this->bumper_count <= ARRAYSIZE(data.bumpers));

  // Do each bumper
  for (int s = 0; s < this->bumper_count; s++)
  {
    double ox = this->scanlines[s].x_offset;
    double oy = this->scanlines[s].y_offset;
    double oth = this->scanlines[s].th_offset;
    double len = this->scanlines[s].length;
    double radius = this->scanlines[s].radius;
      
    LocalToGlobal(ox, oy, oth);
      
    if(  radius == 0.0 )
    {
      // straight line scan
      CLineIterator lit( ox, oy, oth, len, 
                         m_world->ppm, m_world->matrix, PointToBearingRange );
      data.bumpers[s] = (uint8_t)0; // no hit so far
	  
      CEntity* ent;
      while( (ent = lit.GetNextEntity()) )
	    {
	      if( ent!=this && ent!=m_parent_entity && ent->obstacle_return ) 
        {
          data.bumpers[s] = (uint8_t)1; // we're touching something!
          break;
        }
	    }
    }
    else // arc scan
    {	  
      double scan_th = len/radius;

      //printf( "request scan (%.2f %.2f %.2f) radius %.2f scan_th %.2f\n",
      //  ox, oy, oth, radius, scan_th );

      CArcIterator ait( ox, oy, oth, radius, scan_th, 
                        m_world->ppm, m_world->matrix );
	  
      data.bumpers[s] = (uint8_t)0; // no hit so far
	  
      CEntity* ent;
      while( (ent = ait.GetNextEntity()) )
	    {
	      if( ent!=this && ent!=m_parent_entity && ent->obstacle_return
            && ent != m_world->root ) 
        {
          data.bumpers[s] = (uint8_t)1; // we're touching something!
          break;
        }
	    }
    }
  }
  
  
  data.bumper_count = (uint8_t)this->bumper_count;
  PutData( &data, sizeof(data) );
  
  return;
}


///////////////////////////////////////////////////////////////////////////
// Process configuration requests.
void CBumperDevice::UpdateConfig()
{
  int len;
  void* client;
  char buffer[PLAYER_MAX_REQREP_SIZE];
  player_bumper_geom_t geom;

  while (true)
  {
    len = GetConfig(&client, &buffer, sizeof(buffer));
    if (len <= 0)
      break;

    switch (buffer[0] )
    {
      case PLAYER_BUMPER_GET_GEOM_REQ:
        // Return the bumper geometry
        assert(this->bumper_count <= ARRAYSIZE(geom.bumper_def));
        geom.bumper_count = htons(this->bumper_count);
        for (int s = 0; s < this->bumper_count; s++)
        {
          geom.bumper_def[s].x_offset = 
            htons((short) (this->bumpers[s].x_offset * 1000));

          geom.bumper_def[s].y_offset = 
            htons((short) (this->bumpers[s].y_offset * 1000));

          geom.bumper_def[s].th_offset = 
            htons((short) (RTOD(this->bumpers[s].th_offset) ));

          geom.bumper_def[s].length = 
            htons((short) (this->bumpers[s].length * 1000));

          geom.bumper_def[s].radius = 
            htons((short) (this->bumpers[s].radius * 1000));
        }
        PutReply(client, PLAYER_MSGTYPE_RESP_ACK, NULL, &geom, sizeof(geom));
        break;


      default:
        PRINT_WARN1("invalid bumper configuration request [%c]", buffer[0]);
        PutReply(client, PLAYER_MSGTYPE_RESP_NACK);
        break;
    }
  }
}

#ifdef INCLUDE_RTK2

///////////////////////////////////////////////////////////////////////////
// Initialise the rtk gui
void CBumperDevice::RtkStartup()
{
  CPlayerEntity::RtkStartup();
  
  // draw the bumpers into the body fig
  // Set the color
  rtk_fig_color_rgb32(this->fig, this->color);
  
  for( int s=0; s<this->bumper_count; s++ )
    this->RtkRenderBumper( this->fig, s );
  
  // Create a figure representing the data
  this->scan_fig = rtk_fig_create(m_world->canvas, NULL, 55);
  rtk_fig_linewidth( this->scan_fig, 2 ); // thick line
  rtk_fig_color_rgb32(this->scan_fig, RGB(255,0,0) ); // RED!
}


///////////////////////////////////////////////////////////////////////////
// Finalise the rtk gui
void CBumperDevice::RtkShutdown()
{
  // Clean up the figure we created
  rtk_fig_destroy(this->scan_fig);

  CPlayerEntity::RtkShutdown();
} 


///////////////////////////////////////////////////////////////////////////
// Update the rtk gui
void CBumperDevice::RtkUpdate()
{
  CPlayerEntity::RtkUpdate();
     
  // Get global pose
  double gx, gy, gth;
  GetGlobalPose(gx, gy, gth);
  rtk_fig_origin(this->scan_fig, gx, gy, gth );
  rtk_fig_clear(this->scan_fig);


  // see the comment in CLaserDevice for why this gets the data out of
  // the buffer instead of storing hit points in ::Update() - RTV
  player_bumper_data_t data;
  
  if( Subscribed() > 0 && m_world->ShowDeviceData( this->lib_entry->type_num ) )
  {
    size_t res = GetData( &data, sizeof(data));
      
    if( res == sizeof(data) )
    {
      for (int s = 0; s < this->bumper_count; s++)
	    {	      
        // set the color dependent on the bumper state 
        // Set the color
        if( data.bumpers[s] )
          this->RtkRenderBumper( this->scan_fig, s );
	    }
    }
    else
      PRINT_WARN2( "GET DATA RETURNED WRONG AMOUNT (%d/%d bytes)", res, sizeof(data) );
  }  
}


void CBumperDevice::RtkRenderBumper( rtk_fig_t* fig, int bumper )
{

  double ox = this->scanlines[bumper].x_offset;
  double oy = this->scanlines[bumper].y_offset;
  double oth = this->scanlines[bumper].th_offset;
  double len = this->scanlines[bumper].length;
  
  if( this->scanlines[bumper].radius == 0 ) // straight line
  {
    double x2 = ox + len * cos( oth );
    double y2 = oy + len * sin( oth );
    rtk_fig_line( fig, ox, oy, x2, y2 );
  }
  else // arc
  {
    double xy_size = 2.0 * this->scanlines[bumper].radius;	  
    double arc_theta = 
      this->scanlines[bumper].length / this->scanlines[bumper].radius;
      
    double start_angle = -arc_theta/2.0;
    double end_angle = +arc_theta/2.0;
      
    rtk_fig_ellipse_arc( fig, ox, oy, oth, 
                         xy_size, xy_size, 
                         start_angle, end_angle ); 
  }
}

#endif



