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
// CVS: $Id: stg_time.cc,v 1.1 2004-09-16 06:54:28 rtv Exp $
//
///////////////////////////////////////////////////////////////////////////

//#define DEBUG

#include "stg_driver.h"
#include "stg_time.h"
#include "math.h"

////////////////////////////////////////////////////////////////////////////////
// Constructor
StgTime::StgTime( world_t* world )
{
  this->world = world;
  return;
}


////////////////////////////////////////////////////////////////////////////////
// Destructor
StgTime::~StgTime()
{
  return;
}


////////////////////////////////////////////////////////////////////////////////
// Get the simulator time
int StgTime::GetTime(struct timeval* time)
{
  PRINT_DEBUG( "get time" );
  
  if( this->world ) // get the time from the Stage client
    {
      // client->stagetime is in milliseconds
      time->tv_sec  = world->sim_time / 1000;
      time->tv_usec = (world->sim_time % 1000) * 1000;
    }
  else // no time data available
    memset( time, 0, sizeof(struct timeval) );
  
  PRINT_DEBUG2( "time now %d sec %d usec", time->tv_sec, time->tv_usec );
  
  return 0;
}
