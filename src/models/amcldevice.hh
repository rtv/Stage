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
 * $Id: amcldevice.hh,v 1.2 2003-02-12 18:58:35 inspectorg Exp $
 */

#ifndef ADAPTIVEMCLDEVICE_HH
#define ADAPTIVEMCLDEVICE_HH

#include "playerdevice.hh"


// This is a god-awful hack; this structure must be duplicated in both
// Player and Stage.
// Shared Player/Stage configuration info
typedef struct
{
  char map_file[PATH_MAX];
  double map_scale;
  
} amcl_stage_data_t;


//  Dummy device for adaptive MCL
class CAdaptiveMCLDevice : public CPlayerEntity 
{
	// a static named constructor 
	public: static CAdaptiveMCLDevice* Creator(LibraryItem *libit,
                                             CWorld *world, CEntity *parent);
	// Default constructor
	public: CAdaptiveMCLDevice(LibraryItem *libit, CWorld *world, CEntity *parent);

  // Load settings from the world file
  public: bool Load(CWorldFile *worldfile, int section);

  // Initialise entity
  public: virtual bool Startup(void);

  // Configuration data
  amcl_stage_data_t data;
};


#endif
