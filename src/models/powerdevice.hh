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
 * CVS info: $Id: powerdevice.hh,v 1.2 2002-11-01 19:12:32 rtv Exp $
 */

#ifndef POWERDEVICE_HH
#define POWERRDEVICE_HH

#include "playerdevice.hh"
#include "world.hh"
#include "library.hh"


class CPowerDevice : public CPlayerEntity
{
  // Default constructor
  public: CPowerDevice( LibraryItem *libit, CWorld *world, CEntity *parent);
  
  // a static named constructor - a pointer to this function is given
  // to the Library object and paired with a string.  When the string
  // is seen in the worldfile, this function is called to create an
  // instance of this entity
public: static CPowerDevice* Creator(  LibraryItem *libit, CWorld *world, CEntity *parent )
  {
    return( new CPowerDevice( libit, world, parent ) );
  }
    
  // Startup routine
  public: virtual bool Startup();

  // Update the device
  public: virtual void Update(double sim_time);

  // Process configuration requests.
  private: void UpdateConfig();
    
  public: int charge;
  
};

#endif

