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
 * CVS info: $Id: sonar.hh,v 1.1.2.1 2003-02-08 01:20:37 rtv Exp $
 */

#ifndef SONARDEVICE_HH
#define SONARDEVICE_HH

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include "entity.hh"
#include "library.hh"

#define SONARSAMPLES STG_SONAR_MAX_SAMPLES

enum SonarReturn { SonarTransparent=0, SonarOpaque };

class CSonarModel : public CEntity
{
  // Default constructor
  public: CSonarModel( LibraryItem *libit, int id, CEntity *parent);
  
  // a static named constructor - a pointer to this function is given
  // to the Library object and paired with a string.  When the string
  // is seen in the worldfile, this function is called to create an
  // instance of this entity
public: static CSonarModel* Creator( LibraryItem *libit,
				      int id, CEntity *parent )
  {
    return( new CSonarModel( libit, id, parent ) );
  }
    
  // Update the device
  public: virtual int Update();

  //virtual int SetCommand( char* data, size_t len ); // receive command
  virtual int SetConfig( char* data, size_t len ); // receive config
  
  // Maximum range of sonar in meters
  private: double min_range;
  private: double max_range;

  // Array holding the sonar poses
  public: int sonar_count;
  public: double sonars[SONARSAMPLES][3];
  
  public: bool power_on;

#ifdef INCLUDE_RTK2

  // Initialise the rtk gui
  public: virtual void RtkStartup();

  // Finalise the rtk gui
  public: virtual void RtkShutdown();

  // Update the rtk gui
  public: virtual void RtkUpdate();
  
  // For drawing the sonar beams
  private: rtk_fig_t *scan_fig;
#endif

};

#endif

