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
 * Desc: Simulates a noisy IR ring
 * Author: John Sweeney, adapted from sonardevice.cc
 * Date: 22 Aug 2002
 * CVS info: $Id: irdevice.cc,v 1.2 2003-06-17 19:15:26 rtv Exp $
 */

#include <iostream>
#include <math.h>
#include "world.hh"
#include "irdevice.hh"
#include "raytrace.hh"

double gen_norm();

// constructor
CIRDevice::CIRDevice(LibraryItem *libit, CWorld *world, CEntity *parent )
    : CPlayerEntity(libit, world, parent )
{
  // set the Player IO sizes correctly for this type of Entity
  m_data_len    = sizeof( player_ir_data_t );
  m_command_len = 0;
  m_config_len  = 1;
  m_reply_len  = 1;
  
  m_player.code = PLAYER_IR_CODE; // from player's messages.h

  min_range = 0.10;
  max_range = 0.80;

  ir_power = false;

  add_noise = false;

  // Initialise the ir poses to default values
  ir_count = IRSAMPLES;
  for (int i = 0; i < ir_count; i++)
  {
    irs[i][0] = 0;
    irs[i][1] = 0;
    irs[i][2] = i * 2 * M_PI / ir_count;
  }

  // these are used to estimate the noise we've seen with our IRs
  sparams[IR_M_PARAM] =  1.913005560938;
  sparams[IR_B_PARAM] = -7.728130591833;
  // zero the data
  memset( &data, 0, sizeof(data) );
}

bool
CIRDevice::Startup()
{
  if (!CPlayerEntity::Startup()) {
    return false;
  }

  //  SetDriverName("reb_ir");
  return true;
}


///////////////////////////////////////////////////////////////////////////
// Load the entity from the world file
bool CIRDevice::Load(CWorldFile *worldfile, int section)
{
  int i;
  int ircount;
  char key[256];

  if (!CPlayerEntity::Load(worldfile, section)) {
    return false;
  }

  // Load sonar min and max range
  min_range = worldfile->ReadLength(section, "min_range", min_range);
  max_range = worldfile->ReadLength(section, "max_range", max_range);

  // power on or off? (TODO - have this setting saved)
  ir_power = worldfile->ReadInt(section, "power_on", ir_power );

  // do we add noise to the readings?
  add_noise = worldfile->ReadInt(section, "add_noise", add_noise);

  std::cout <<"IR: noise: "<<add_noise<<std::endl;
  // load noise parameters
  snprintf(key, sizeof(key), "noiseparams");
  for (i =0; i < 2; i++) {
    sparams[i] = worldfile->ReadTupleFloat(section, key, i, 1.0);
    //    cout <<"IR: sparams["<<i<<"]: "<<sparams[i]<<endl;
  }


  // Load the geometry of the ir ring
  ircount = worldfile->ReadInt(section, "ircount", 0);
  if (ircount > 0)
  {
    ir_count = ircount;
    for (i = 0; i < ir_count; i++)
    {
      snprintf(key, sizeof(key), "irpose[%d]", i);
      irs[i][0] = worldfile->ReadTupleLength(section, key, 0, 0);
      irs[i][1] = worldfile->ReadTupleLength(section, key, 1, 0);
      irs[i][2] = worldfile->ReadTupleAngle(section, key, 2, 0);
      //      printf("IRDEV: LOAD: ir%d= %g %g %g\n", i, irs[i][0], irs[i][1], irs[i][2]);
    }
  }
  return true;
}


///////////////////////////////////////////////////////////////////////////
// Update the ir data
void CIRDevice::Update( double sim_time ) 
{
  uint16_t v; //the value we calculate
  double x;

  CPlayerEntity::Update( sim_time ); // inherit some debug output

  // dump out if noone is subscribed
  if(!Subscribed()) {
    return;
  }

  // Check to see if it is time to update
  //  - if not, return right away.
  if( sim_time - m_last_update < m_interval) {
    return;
  }
  m_last_update = sim_time;

  // Process configuration requests
  UpdateConfig();
  
  // Check bounds
  assert((size_t) ir_count <= ARRAYSIZE(data.ranges));
  
  // if IRs have power
  if (ir_power) {
    // Do each ir
    for (int s = 0; s < ir_count; s++) {
      // Compute parameters of scan line
      double ox = irs[s][0];
      double oy = irs[s][1];
      double oth = irs[s][2];
      LocalToGlobal(ox, oy, oth);
      
      double range = max_range;
      
      CLineIterator lit( ox, oy, oth, max_range, 
			 m_world->ppm, m_world->matrix, PointToBearingRange );
      CEntity* ent;
      
      while( (ent = lit.GetNextEntity()) ) {
	// check for same returns as sonar
	if( ent != this && ent != m_parent_entity && ent->sonar_return ) {
	  range = lit.GetRange();
	  break;
	}
      }
      	
      // now noise up the range... based on the detected range, determine
      // the standard deviation of the gaussian, then sample that distribution
      double std = exp( log( (double) range) * sparams[IR_M_PARAM] +
			sparams[IR_B_PARAM] );
	
      if (add_noise) {
	// generate random following standard normal
	x = gen_norm();

      } else {
	x = 0.0;
      }

      // now fit to our dist	
      x = x*std*5.0 + range*1000.0;
      if (x < 0.0) {
	x = 0.0;
      }
      
      v = (uint16_t) rint(x);
      
      // Store range in mm in network byte order
      //    data.range_count = htons(ir_count);
      data.ranges[s] = htons(v);

    }
  } 
  
  PutData( &data, sizeof(data) );

  return;
}


///////////////////////////////////////////////////////////////////////////
// Process configuration requests.
void CIRDevice::UpdateConfig()
{
  int s, len;
  void* client;
  char buffer[PLAYER_MAX_REQREP_SIZE];
  player_ir_pose_t geom;

  while (1) {
    len = GetConfig(&client, &buffer, sizeof(buffer));
    if (len <= 0)
      break;
    
    switch (buffer[0]) {
    case PLAYER_IR_POWER_REQ:
      ir_power = ((player_ir_power_req_t *)buffer)->state;
      PutReply(client, PLAYER_MSGTYPE_RESP_ACK);
      break;
      
    case PLAYER_IR_POSE_REQ:
      // Return the ir geometry
      assert(ir_count <= ARRAYSIZE(geom.poses));
      geom.pose_count = htons(ir_count);
      for (s = 0; s < ir_count; s++) {
	geom.poses[s][0] = htons((short) (irs[s][0] * 1000));
	geom.poses[s][1] = htons((short) (irs[s][1] * 1000));
	geom.poses[s][2] = htons((short) (irs[s][2] * 180 / M_PI));
      }
      PutReply(client, PLAYER_MSGTYPE_RESP_ACK, NULL, &geom, sizeof(geom));
      break;
      
    default:
      PRINT_WARN1("invalid ir configuration request [%c]", buffer[0]);
      PutReply(client, PLAYER_MSGTYPE_RESP_NACK);
      break;
    }
  }
}

#ifdef INCLUDE_RTK2

///////////////////////////////////////////////////////////////////////////
// Initialise the rtk gui
void CIRDevice::RtkStartup()
{
  CEntity::RtkStartup();
  
  // Create a figure representing this object
  scan_fig = rtk_fig_create(m_world->canvas, NULL, 49);

  // Set the color
  rtk_fig_color_rgb32(scan_fig, color);
}


///////////////////////////////////////////////////////////////////////////
// Finalise the rtk gui
void CIRDevice::RtkShutdown()
{
  // Clean up the figure we created
  rtk_fig_destroy(scan_fig);

  CEntity::RtkShutdown();
} 


///////////////////////////////////////////////////////////////////////////
// Update the rtk gui
void CIRDevice::RtkUpdate()
{
  CEntity::RtkUpdate();
   
  // Get global pose
  //double gx, gy, gth;
  //GetGlobalPose(gx, gy, gth);
  //rtk_fig_origin(scan_fig, gx, gy, gth );

  rtk_fig_clear(scan_fig);

  // see the comment in CLaserDevice for why this gets the data out of
  // the buffer instead of storing hit points in ::Update() - RTV
  player_ir_data_t rdata;
  
  if( Subscribed() > 0 && m_world->ShowDeviceData( lib_entry->type_num ) )
  {
    size_t res = GetData( &rdata, sizeof(rdata));

    if( res == sizeof(rdata) )
    {
      for (int s = 0; s < ir_count; s++)
      {
        double ox = irs[s][0];
        double oy = irs[s][1];
        double oth = irs[s][2];
        LocalToGlobal(ox, oy, oth);
        
	// convert from integer mm to double m
        double range = (double)ntohs(rdata.ranges[s]) / 1000.0;
	
        double x1 = ox;
        double y1 = oy;
        double x2 = x1 + range * cos(oth); 
        double y2 = y1 + range * sin(oth);       
        rtk_fig_line(scan_fig, x1, y1, x2, y2 );
      }
    }
    else
      PRINT_WARN2( "GET DATA RETURNED WRONG AMOUNT (%d/%d bytes)", res, sizeof(data) );
  }  
}

#endif


/* This code was written by Kevin Karplus  7 Nov 2000
   Generate one number from the standard normal distribution.

     Method: Ratio of uniforms, as improved by Knuth.  The ratio of
     uniforms was chosen because of the simplicity of the method.  The
     popular "polar Box-Mueller" method may make fewer than half as
     many calls to DRAND, but to do so it requires almost twice ans
     many class to log() and requires saving state between calls to
     the method.  I prefer the stateless ratio-of-uniforms method.
     
     Method taken from on pages 99-101 in 
     Dagpunar, John.
     Principles of Random Variate Generation.
     Oxford University Press, 1988.
*/
/* pick which underlying uniform random number generator to use. */
#define DRAND() ((random()+0.5)/ 2147483648.0)
#define SQRT_2_OVER_E	0.857763884961	/* sqrt(2/e) */
#define EXP_135	0.259240260646		/* exp(-1.35) */

double gen_norm(void)
{
    double r1, r2, x, w;
    
    do
    {	r1=DRAND();
    	r2=DRAND();
	x = SQRT_2_OVER_E * (r2+r2-1)/ r1;
	w = 0.25 * x * x;
	if (1-r1 >= w) return x;
    } while ( EXP_135 < (w-0.35)*r1
    	|| log(r1) > -w);
    return x;
}
