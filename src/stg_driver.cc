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
 * $Id: stg_driver.cc,v 1.6 2004-09-25 23:21:27 rtv Exp $
 */

// STAGE-1.4 DRIVER CLASS ///////////////////////////////
//
// creates a single static Stage client. This is subclassed for each
// Player interface. Each instance shares the single Stage connection

#define PLAYER_ENABLE_TRACE 0
#define PLAYER_ENABLE_MSG 1

#include <string.h>
#include <sys/stat.h>
#include <termios.h>
#include <unistd.h>
#include <netinet/in.h>  /* for struct sockaddr_in, htons(3) */


#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdio.h>
#include <math.h>
#include <unistd.h>

#define DEBUG

#include "stg_time.h"
#include "stg_driver.h"

// init static vars
ConfigFile* Stage1p4::config = NULL;
stg_world_t* Stage1p4::world = NULL;


// declare Player's emergency stop function (defined in main.cc)
void Interrupt( int dummy );
				   
// constructor
//
Stage1p4::Stage1p4( ConfigFile* cf, int section, int interface, uint8_t access,
		    size_t datasz, size_t cmdsz, int rqlen, int rplen) :
  Driver(cf, section, interface, access, datasz, cmdsz, rqlen, rplen )
{
  PLAYER_TRACE1( "Stage1p4 device created for interface %s\n", interface );
  
  this->config = cf;

  if( this->device_id.code == PLAYER_SIMULATION_CODE )
    {
      printf( "Initializing Stage simulation device\n" );
    }
  else
    {
      char* model_name = 
	(char*)config->ReadString(section, "model", NULL );
      
      printf( "    connecting Stage model \"%s\" \n", 
	      model_name  );
      
      if( model_name == NULL )
	PLAYER_ERROR1( "device \"%s\" uses the Stage1p4 driver but has "
		       "no \"model\" value defined. You must specify a "
		       "model name that matches one of the models in "
		       "the worldfile.",
		       model_name );
      
      PLAYER_TRACE1( "attempting to resolve Stage model \"%s\"", model_name );
      
      this->model = 
	//stg_world_model_name_lookup( Stage1p4::world, model_name );
	stg_world_model_name_lookup( this->world, model_name );

      if( this->model  == NULL )
	{
	  PLAYER_ERROR1( "Failed to find a Stage model named \"%s\"",
			 model_name );
	  exit( -1 );
	}
    }  

  // insert the data notification callback
  //stg_model_register_data_callback( Driver::DataAvailableStatic,
  //			    this );

  // insert a data callback into our model
  this->model->data_notify = Driver::DataAvailableStatic;
  this->model->data_notify_arg = this;

}

// destructor
Stage1p4::~Stage1p4()
{
}

int Stage1p4::Setup()
{ 
  PRINT_DEBUG( "SETUP" );
  stg_model_subscribe( this->model );
  return 0;
};

int Stage1p4::Shutdown()
{ 
  PRINT_DEBUG( "SHUTDOWN" );
  stg_model_unsubscribe( this->model );
  return 0;
};


////////////////////////////////////////////////////////////////////////////////
// Extra stuff for building a shared object.

//#include "drivertable.h"


/* need the extern to avoid C++ name-mangling  */
void StgSimulation_Register(DriverTable *table);
void StgLaser_Register(DriverTable *table);
void StgPosition_Register(DriverTable *table);
void StgSonar_Register(DriverTable *table);
void StgEnergy_Register(DriverTable *table);
void StgBlobfinder_Register(DriverTable *table);
void StgFiducial_Register(DriverTable *table);
//void StgBlinkenlight_Register(DriverTable *table);


extern "C" {

  int player_driver_init(DriverTable* table)
  {
    printf( "Stage plugin: \"%s\"\n", stg_get_version_string() );

    StgSimulation_Register(table);
    StgLaser_Register(table);
    StgFiducial_Register(table);
    StgSonar_Register(table);
    StgPosition_Register(table);
    StgBlobfinder_Register(table);
    return(0);
  }
}
