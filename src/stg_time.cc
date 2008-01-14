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
// CVS: $Id: stg_time.cc,v 1.12 2008-01-14 20:23:01 rtv Exp $
//
///////////////////////////////////////////////////////////////////////////

#include "stg_time.h"
#include "math.h"
#include "p_driver.h"

// Stage's global quit flag
extern int _stg_quit;

// Constructor
StgTime::StgTime( StgDriver* driver )
{
  assert(driver);
  this->driver = driver;
  return;
}

// Destructor
StgTime::~StgTime()
{
  return;
}

extern int update_request;

// Get the simulator time
int StgTime::GetTime(struct timeval* time)
{
  PRINT_DEBUG( "get time" );
  
  assert( this->driver );
  
  stg_world_t* world = driver->world;
  
  time->tv_sec  = (int)floor(world->sim_time / 1e3);
  time->tv_usec = (int)rint(fmod(world->sim_time,1e3) * 1e3);
  
  PRINT_DEBUG2( "time now %ld sec %ld usec", time->tv_sec, time->tv_usec );
  
  return 0;
}

int StgTime::GetTimeDouble(double* time)
{
  PRINT_DEBUG( "get time (double)" );
  
  assert( this->driver );
  
  stg_world_t* world = driver->world;
  
  *time = world->sim_time / 1e3;
  
  PRINT_DEBUG1( "time now %f sec ", *time);
  
  return 0;
}
