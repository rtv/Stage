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
 * CVS info: $Id: sonardevice.hh,v 1.1 2002-10-27 21:46:13 rtv Exp $
 */

#ifndef SONARDEVICE_HH
#define SONARDEVICE_HH

#include "playerdevice.hh"
#include "world.hh"
#include "library.hh"


#define SONARSAMPLES PLAYER_SONAR_MAX_SAMPLES

enum SonarReturn { SonarTransparent=0, SonarOpaque };

class CSonarDevice : public CPlayerEntity
{
  // Default constructor
  public: CSonarDevice(CWorld *world, CEntity *parent);
  
  // a static named constructor - a pointer to this function is given
  // to the Library object and paired with a string.  When the string
  // is seen in the worldfile, this function is called to create an
  // instance of this entity
public: static CSonarDevice* Creator( CWorld *world, CEntity *parent )
  {
    return( new CSonarDevice( world, parent ) );
  }
    
  // Startup routine
  public: virtual bool Startup();
  
  // Load the entity from the world file
  public: virtual bool Load(CWorldFile *worldfile, int section);

  // Update the device
  public: virtual void Update(double sim_time);

  // Process configuration requests.
  private: void UpdateConfig();
    
  // Maximum range of sonar in meters
  private: double min_range;
  private: double max_range;

  // Array holding the sonar poses
  public: int sonar_count;
  public: double sonars[SONARSAMPLES][3];
  
  public: bool power_on;

  // Structure holding the sonar data
  private: player_sonar_data_t data;

  //protected: virtual size_t PutData( void* vdata, size_t len );

#ifdef INCLUDE_RTK2

  // Initialise the rtk gui
  protected: virtual void RtkStartup();

  // Finalise the rtk gui
  protected: virtual void RtkShutdown();

  // Update the rtk gui
  protected: virtual void RtkUpdate();
  
  // For drawing the sonar beams
  private: rtk_fig_t *scan_fig;
#endif

#ifdef USE_GNOME2
  // cache the data between updates in this buffer. we compare the
  // range readings to see if we need to re-render the range polygon
  player_sonar_data_t last_data;
#endif
};

#endif

