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
 * Desc: Simulates the mcom device (just a place-holder really; most of
 *       the work is done by Player).
 * Author: Reed Hedges <reed@zerohour.net> (UMass/Amherst), based on code by  
 *          Andrew Howard
 * Date: Feb 2003
 * CVS info: $Id: mcomdevice.hh,v 1.1.2.1 2003-03-08 18:32:20 gerkey Exp $
 */

#ifndef MCOMDEVICE_STAGE_HH
#define MCOMDEVICE_STAGE_HH

#include "playerdevice.hh"

class CMComDevice : public CPlayerEntity
{
  // Default constructor
  public: CMComDevice( LibraryItem *libit, CWorld *world, CEntity *parent);
    
  // a static named constructor - a pointer to this function is given
  // to the Library object and paired with a string.  When the string
  // is seen in the worldfile, this function is called to create an
  // instance of this entity
public: static CMComDevice* Creator(  LibraryItem *libit, CWorld *world, CEntity *parent )
  { return( new CMComDevice( libit, world, parent ) ); }
};

#endif






