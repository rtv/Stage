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
 * $Id: stage1p4.cc,v 1.5 2003/08/02 02:
 *
*/

// STAGE-1.4 DRIVER CLASS  /////////////////////////////////

// include all the player stuff
#include "playercommon.h"
#include "player.h"
#include "player/device.h"
#include "player/driver.h"
#include "player/configfile.h"
#include "player/drivertable.h"


#include "stage.h"
#include "model.h" // TODO move into stage.h

class Stage1p4 : public Driver
{
 public:
  Stage1p4( ConfigFile* cf, int section, int interface, uint8_t access,
		   size_t datasz, size_t cmdsz, int rqlen, int rplen);
  virtual ~Stage1p4();
  
  virtual int Setup();
  virtual int Shutdown(); 

 protected:

  static ConfigFile* config;

  //static char worldfile_name[MAXPATHLEN]; // filename

  // all player devices share the same Stage client and world (for now)
  //static stg_client_t* stage_client;

  static world_t* world;

  
  model_t* model; // points inside the shared stg_client_t to our
  // individual model data
  
  // the property we automatically subscribe to on Setup();
  //stg_id_t subscribe_prop;

  //GList* subscribe_list;
};
