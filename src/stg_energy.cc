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
 * $Id: stg_energy.cc,v 1.1 2004-09-16 06:54:28 rtv Exp $
 */

#define PLAYER_ENABLE_TRACE 0
#define PLAYER_ENABLE_MSG 1

#include "playercommon.h"
#include "drivertable.h"
#include "player.h"
#include <stdlib.h>
#include "stageclient.h"

// DRIVER FOR ENERGY INTERFACE ///////////////////////////////////////////

class StgEnergy:public Stage1p4
{
public:
  StgEnergy( ConfigFile* cf, int section );
  
  virtual size_t GetData(void* client, unsigned char* dest, size_t len,
			 uint32_t* timestamp_sec, uint32_t* timestamp_usec);
};

StgEnergy::StgEnergy( ConfigFile* cf, int section ) 
  : Stage1p4( interface, cf, section, sizeof(player_energy_data_t), 0, 1, 1 )
{
  PLAYER_TRACE1( "constructing StgEnergy with interface %s", interface );
}

Driver* StgEnergy_Init( ConfigFile* cf, int section)
{
  if(strcmp( PLAYER_ENERGY_STRING))
    {
      PLAYER_ERROR1("driver \"stg_energy\" does not support interface \"%s\"\n",
		    interface);
      return(NULL);
    }
  else 
    return((Driver*)(new StgEnergy( cf, section)));
}


void StgEnergy_Register(DriverTable* table)
{
  table->AddDriver("stg_energy",  StgEnergy_Init);
}

// override GetData to get data from Stage on demand, rather than the
// standard model of the source filling a buffer periodically
size_t StgEnergy::GetData(void* client, unsigned char* dest, size_t len,
			 uint32_t* timestamp_sec, uint32_t* timestamp_usec)
{  
  
  stg_property_t* prop = stg_model_get_prop_cached( model, STG_PROP_ENERGYDATA);
  
  if( prop )
    {
      stg_energy_data_t *senergy = (stg_energy_data_t*)prop->data;
      
      if( senergy )
	{
	  // convert Stage data into Player energy packet
	  player_energy_data_t penergy;
	  memset( &penergy, 0, sizeof(penergy) );
	  
	  penergy.mjoules = htonl((uint32_t)(senergy->joules*1000.0));
	  penergy.mwatts = htonl((int32_t)(senergy->watts*1000.0));
	  penergy.charging = senergy->charging;
	  
	  Driver::PutData( &penergy, sizeof(penergy), 0,0 ); // time gets filled in
	}
    }
  
  // now inherit the standard data-getting behavior 
  return Driver::GetData(client,dest,len,timestamp_sec,timestamp_usec);
}


