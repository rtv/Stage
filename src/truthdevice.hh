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
 * Desc: A device for getting the true pose of things.
 * Author: Andrew Howard
 * Date: 6 Jun 2002
 * CVS info: $Id: truthdevice.hh,v 1.1 2002-08-23 00:19:39 rtv Exp $
 */
#ifndef TRUTHDEVICE_HH
#define TRUTHDEVICE_HH

#include "playerdevice.hh"

class CTruthDevice : public CPlayerEntity
{
  // Default constructor
  public: CTruthDevice(CWorld *world, CEntity *parent);
    
  // a static named constructor - a pointer to this function is given
  // to the Library object and paired with a string.  When the string
  // is seen in the worldfile, this function is called to create an
  // instance of this entity
public: static CTruthDevice* Creator( CWorld *world, CEntity *parent )
  { return( new CTruthDevice( world, parent ) ); }

  // Update the device
  public: virtual void Update(double sim_time);

  // Update config stuff
  public: virtual void UpdateConfig();

  // Update data
  public: virtual void UpdateData();
};

#endif






