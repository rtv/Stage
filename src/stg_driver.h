/*
 *  Stage-1.4 driver for Player
 *  Copyright (C) 2003  Richard Vaughan (rtv) vaughan@hrl.com 
 *
 * This program is free software; you can redistribute it and/or modify
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
 * $Id: StgDriver.cc,v 1.5 2003/08/02 02:
 *
*/


// STAGE-1.4 DRIVER CLASS  /////////////////////////////////

/*! \file stg_driver.h 

  This header file contains the class StgDriver, derived from Player's
  Device class, and extended by Stage Plugin's various drivers.
*/


#include <pthread.h>

// include all the player stuff
#include "playercommon.h"
#include "player.h"
#include "player/device.h"
#include "player/driver.h"
#include "player/configfile.h"
#include "player/drivertable.h"


#include "stage_internal.h"

#define DRIVER_ERROR(X) printf( "Stage driver error: %s\n", X )

class StgDriver : public Driver
{
 public:
  StgDriver( ConfigFile* cf, int section );
  virtual ~StgDriver();
  
  //virtual int Setup();
  //virtual int Shutdown(); 

 protected:

  // set in startup(), unset in shutdown()
  //int ready;

};
