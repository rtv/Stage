/*
 *  Player - One Hell of a Robot Server
 *  Copyright (C) 2004, 2005 Richard Vaughan
 *                      
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
 * Desc: A plugin driver for Player that gives access to Stage devices.
 * Author: Rich Mattes
 * Date: 29 May 2011
 */

// DOCUMENTATION ------------------------------------------------------------

/** @addtogroup player 
@par GPS interface
- PLAYER_GPS_DATA_STATE
*/

// CODE ----------------------------------------------------------------------
// Math borrowed from
// http://mathforum.org/library/drmath/view/51833.html

#include "p_driver.h" 
using namespace Stg;

InterfaceGPS::InterfaceGPS( player_devaddr_t addr, 
				StgDriver* driver,
				ConfigFile* cf,
				int section )
  : InterfaceModel( addr, driver, cf, section, "" )
{ 
    latitude_origin = cf->ReadTupleFloat(section, "gps_origin", 0, 0.0);
    longitude_origin = cf->ReadTupleFloat(section, "gps_origin", 1, 0.0);    
}

void InterfaceGPS::Publish( void )
{  
  player_gps_data_t gps;
  memset( &gps, 0, sizeof(gps));

  Pose pose = mod->GetPose();
	double lat, lon;
	
	this->XYtoLatLon( (double)pose.x, (double)pose.y, &lat, &lon);
	lat += latitude_origin;
	lon += longitude_origin;
	
	double now = M_PI;
	GlobalTime->GetTimeDouble(&now);
	
	gps.time_sec = floor(now);
	gps.time_usec = now - gps.time_sec * 100000;
	
	gps.latitude = lat * 1.e7;
	gps.longitude = lon * 1.e7;
	gps.altitude = pose.z * 1000;
	
	double utme, utmn;
	UTM(lat, lon, &utme, &utmn);
	
	gps.utm_e = utme;
	gps.utm_n = utmn;
	gps.quality = 2;
	gps.num_sats = 7;
	
  // Write localize data
  this->driver->Publish(this->addr,
			PLAYER_MSGTYPE_DATA,
			PLAYER_GPS_DATA_STATE,
			(void*)&gps, sizeof(gps), NULL);
}


int InterfaceGPS::ProcessMessage(QueuePointer &resp_queue,
				      player_msghdr_t* hdr,
				      void* data)
{
	// Don't know how to handle this message.
	PRINT_WARN2( "stage gps doesn't support message %d:%d.",
       hdr->type, hdr->subtype);
	return(-1);

}

void InterfaceGPS::XYtoLatLon(double xpos, double ypos, double *lat, double *lon)
{
  // Use planar earth approximation
  double R = 6378137.0;
  double d_lat = xpos / R;
  double d_lon = ypos / (R * cos(M_PI * latitude_origin / 180.0));
  
  *lat = d_lat * 180.0 / M_PI;
  *lon = d_lon * 180.0 / M_PI;
	return;
}

void InterfaceGPS::UTM(double lat, double lon, double *x, double *y)
{
// Borrowed from Player's garminnmea driver
	// constants
	const static double m0 = (1 - UTM_E2/4 - 3*UTM_E4/64 - 5*UTM_E6/256);
	const static double m1 = -(3*UTM_E2/8 + 3*UTM_E4/32 + 45*UTM_E6/1024);
	const static double m2 = (15*UTM_E4/256 + 45*UTM_E6/1024);
	const static double m3 = -(35*UTM_E6/3072);

	// compute the central meridian
	int cm = (lon >= 0.0) ? ((int)lon - ((int)lon)%6 + 3) : ((int)lon - ((int)lon)%6 - 3);

	// convert degrees into radians
	double rlat = lat * M_PI/180;
	double rlon = lon * M_PI/180;
	double rlon0 = cm * M_PI/180;

	// compute trigonometric functions
	double slat = sin(rlat);
	double clat = cos(rlat);
	double tlat = tan(rlat);

	// decide the flase northing at origin
	double fn = (lat > 0) ? UTM_FN_N : UTM_FN_S;

	double T = tlat * tlat;
	double C = UTM_EP2 * clat * clat;
	double A = (rlon - rlon0) * clat;
	double M = WGS84_A * (m0*rlat + m1*sin(2*rlat) + m2*sin(4*rlat) + m3*sin(6*rlat));
	double V = WGS84_A / sqrt(1 - UTM_E2*slat*slat);

	// compute the easting-northing coordinates
	*x = UTM_FE + UTM_K0 * V * (A + (1-T+C)*pow(A,3)/6 + (5-18*T+T*T+72*C-58*UTM_EP2)*pow(A,5)/120);
	*y = fn + UTM_K0 * (M + V * tlat * (A*A/2 + (5-T+9*C+4*C*C)*pow(A,4)/24 + (61-58*T+T*T+600*C-330*UTM_EP2)*pow(A,6)/720));

  return;
}


