/*
 *  Player - One Hell of a Robot Server
 *     Brian Gerkey, Kasper Stoy, Richard Vaughan, & Andrew Howard
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
///////////////////////////////////////////////////////////////////////////
//
// Desc: Stage (simulator) time
// Author: Richard Vaughan
// Date: 7 May 2003
// CVS: $Id: stg_time.h,v 1.2 2004-09-22 20:47:22 rtv Exp $
//
///////////////////////////////////////////////////////////////////////////

#ifndef STG_TIME_H
#define STG_TIME_H

#include "player/playertime.h"
#include "stage.h"

class StgTime : public PlayerTime
{
  // Constructor
 public: StgTime( stg_world_t* world );
 
 // Destructor
 public: virtual ~StgTime();
 
 // Get the simulator time
 public: int GetTime(struct timeval* time);
 
 private:
 //stg_client_t* client;

 stg_world_t* world;
};

#endif
