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
 * CVS info: $Id: sonardevice.hh,v 1.12 2002-08-22 02:04:38 rtv Exp $
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
  private: int sonar_count;
  private: double sonars[SONARSAMPLES][3];
    
  // Structure holding the sonar data
  private: player_sonar_data_t data;

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
};

#endif

