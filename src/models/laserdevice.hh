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
 * Author: Andrew Howard
 * Date: 28 Nov 2000
 * CVS info: $Id: laserdevice.hh,v 1.1 2002-10-27 21:46:13 rtv Exp $
 */

#ifndef LASERDEVICE_HH
#define LASERDEVICE_HH

#include "playerdevice.hh"
#include "laserbeacon.hh"

#include <list> // STL

typedef std::list< int > LaserBeaconList; 

class CLaserDevice : public CPlayerEntity
{
  // Default constructor
  public: CLaserDevice(CWorld *world, CEntity *parent );

  // a static named constructor - a pointer to this function is given
  // to the Library object and paired with a string.  When the string
  // is seen in the worldfile, this function is called to create an
  // instance of this entity
public: static CLaserDevice* Creator( CWorld *world, CEntity *parent )
  {
    return( new CLaserDevice( world, parent ) );
  }
    
  // Startup routine
  public: virtual bool Startup();

  // Load the entity from the worldfile
  public: virtual bool Load(CWorldFile *worldfile, int section);
  
  // Update the device
  public: virtual void Update( double sim_time );

  // Check to see if the configuration has changed
  private: bool CheckConfig();

  // Generate scan data
  private: bool GenerateScanData(player_laser_data_t *data);

  // protected: virtual size_t PutData( void* vdata, size_t len );


  // Laser scan rate in samples/sec
  private: double scan_rate;

  // Minimum resolution in degrees
  private: double min_res;
  
  // Maximum range of sample in meters
  private: double max_range;

  // Laser configuration parameters
  private: double scan_res;
  private: double scan_min;
  private: double scan_max;
  private: int scan_count;
  private: bool intensity;

  // List of beacons detected in last scan
  public: LaserBeaconList visible_beacons;

#ifdef INCLUDE_RTK2

  // Initialise the rtk gui
  protected: virtual void RtkStartup();

  // Finalise the rtk gui
  protected: virtual void RtkShutdown();

  // Update the rtk gui
  protected: virtual void RtkUpdate();
  
  // For drawing the laser scan & intensity values
  private: rtk_fig_t *scan_fig;
  private: rtk_fig_t *intensity_fig;

#endif
  
  //#ifdef USE_GNOME2
  // cache the data between updates in this buffer. we compare the
  // range readings to see if we need to re-render the range polygon
  //player_laser_data_t last_data;
  //#endif

};

#endif















