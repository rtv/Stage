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
// CVS: $Id: stg_time.cc,v 1.7 2005-07-22 21:02:03 rtv Exp $
//
///////////////////////////////////////////////////////////////////////////

//#define DEBUG

#include "stage_internal.h"
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

   // // get real time in msec since stg_timenow was first called
   stg_msec_t timenow = stg_timenow();
  
//   // time since the world was last updated
   stg_msec_t elapsed =  timenow - driver->world->wall_last_update;

   stg_world_t* world = driver->world;
  
//   // if it's time for an update, update all the models and remember
//   // the time we did it
    if( world->wall_interval < elapsed )
      {
        if( ! world->paused )
 	 {
	   stg_msec_t real_interval = timenow - world->wall_last_update;
	   world->real_interval_measured = real_interval;      
	   g_hash_table_foreach( world->models, model_update_cb, world );            
	   world->wall_last_update = timenow;      
	   world->sim_time += world->sim_interval;
	 }
	
	// If There's a GUI, deal with it
	if( world->win )
	  {
	    gui_poll();
	    if( gui_world_update( world ) )
	      stg_quit_request();	  
	  }
	
	// if anyone raised the quit flag, bail here.
	if( _stg_quit )
	  exit(0);
      }
    
    time->tv_sec  = (int)floor(world->sim_time / 1e3);
    time->tv_usec = (int)rint(fmod(world->sim_time,1e3) * 1e3);
    
    PRINT_DEBUG2( "time now %ld sec %ld usec", time->tv_sec, time->tv_usec );
    
    return 0;
}
