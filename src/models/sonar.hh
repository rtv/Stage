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
 * CVS info: $Id: sonar.hh,v 1.1.2.5 2003-02-26 01:57:16 rtv Exp $
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
  public: CSonarModel( int id, char* token, char* color, CEntity *parent);
  
  // a static named constructor - a pointer to this function is given
  // to the Library object and paired with a string.  When the string
  // is seen in the worldfile, this function is called to create an
  // instance of this entity
public: static CSonarModel* Creator( int id, char* token, char* color, CEntity *parent )
  {
    return( new CSonarModel( id, token, color, parent ) );
  }
  
public: 
  // Update the device and publish data
  virtual int Update();
  
    
#ifdef INCLUDE_RTK2

  // Initialise the rtk gui
  public: virtual int RtkStartup();

  // Finalise the rtk gui
  public: virtual void RtkShutdown();

  // Update the rtk gui
  public: virtual int RtkUpdate();
  
  // For drawing the sonar beams
  private: rtk_fig_t *scan_fig;
#endif

};

#endif

