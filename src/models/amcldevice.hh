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
 * Desc: Dummy adaptive MCL device (the real driver is implemented in Player)
 * Author: Andrew Howard
 * Date: 6 Feb 2003
 * $Id: amcldevice.hh,v 1.1 2003-02-08 07:54:42 inspectorg Exp $
 */

#ifndef ADAPTIVEMCLDEVICE_HH
#define ADAPTIVEMCLDEVICE_HH

#include "playerdevice.hh"

//  Dummy device for adaptive MCL
class CAdaptiveMCLDevice : public CPlayerEntity 
{
	// a static named constructor 
	public: static CAdaptiveMCLDevice* Creator(LibraryItem *libit,
                                             CWorld *world, CEntity *parent);
	// Default constructor
	public: CAdaptiveMCLDevice(LibraryItem *libit, CWorld *world, CEntity *parent);
};


#endif
