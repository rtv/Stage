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
 * $Id: stg_simulation.cc,v 1.9 2004-12-10 10:15:13 rtv Exp $
 */

/** @defgroup stg_sim stg_sim Player driver

This driver gives Player access to Stage's simulation engine

@par Provides

The stg_sim driver provides the following device interfaces:

- player_interface_simulation 
 - This interface controls simulators. It is rather minimal right now: it just loads a simulation, and  accepts no commmands and produces no data. It will eventually expand to allow starting and stopping the clock, saving the state, and other generic simulation controls.

@par Supported configuration requests

  - none

@par Player configuration file options

- worldfile (string)
  - where (string) is the filename of a Stage worldfile.
  
@par Player configuration (.cfg) file example:

@verbatim
driver
( 
  # load the stage plugin to get access to stg_sim and stg_mod
  plugin "libstage"

  # load the stg_sim driver as a simulation device
  name "stg_sim"
  provides ["simulation:0" ]

  # options for the stg_sim driver
  worldfile "worlds/simple.world"
)
@endverbatim

@par Authors

Richard Vaughan
*/


#include "stg_driver.h"
#include "stg_time.h"
extern PlayerTime* GlobalTime;

extern int global_argc;
extern char** global_argv;

#define STG_DEFAULT_WORLDFILE "default.world"

// DRIVER FOR SIMULATION INTERFACE ////////////////////////////////////////

// Player's command line args
extern int global_argc;
extern char** global_argv;

// CLASS -------------------------------------------------------------

class StgSimulation:public StgDriver
{
public:
  StgSimulation( ConfigFile* cf, int section);
  ~StgSimulation();
  
  // TODO soon - implement some simulator interface things - pause, resume,
  // save, load, etc.
  
  // TODO eventually - add interface code to change everything in the simulator
  
  virtual void Main();
  virtual int Setup();
  virtual int Shutdown();
};



StgSimulation::StgSimulation( ConfigFile* cf, int section ) 
  : StgDriver( cf, section )
{
  //PLAYER_MSG0( "constructing stg_simulation device" );
  
  // boot libstage 
  stg_init( global_argc, global_argv );
  
  this->world = NULL;

  // load a worldfile
  char worldfile_name[MAXPATHLEN];
  const char* wfn = 
    config->ReadString(section, "worldfile", STG_DEFAULT_WORLDFILE);
  strncpy( worldfile_name, wfn, MAXPATHLEN );
  
  // Find the worldfile.  If the filename begins with a '/', it's
  // an absolute path and we leave it alone. Otherwise it's a
  // relative path so we need to append the config file's path to
  // the front.
  
  // copy the config filename
  char tmp[MAXPATHLEN];
  strncpy( tmp, cf->filename, MAXPATHLEN );
  
  // cut off the filename, leaving just the path prefix
  char* last_slash = strrchr( tmp, '/' );
  if( last_slash )
    {
      *last_slash = 0; // turn the last slash into a terminator 
      assert( chdir( tmp ) == 0 );
    }
  
  //printf( "cfg.filename \"%s\" (path: \"%s\")"
  //"\nwfn \"%s\"\nWorldfile_name \"%s\"\n", 
  //cf->filename, tmp, wfn, worldfile_name );
  
  // a little sanity testing
  if( !g_file_test( worldfile_name, G_FILE_TEST_EXISTS ) )
    {
      PRINT_ERR1( "worldfile \"%s\" does not exist", worldfile_name );
      return;
    }
  
  // create a passel of Stage models in the local cache based on the
  // worldfile
  printf( "  Stage driver loading worldfile \"%s\" ", worldfile_name );      
  fflush(stdout);
  StgDriver::world = NULL;

  this->world = stg_world_create_from_file( worldfile_name );
  assert(this->world);
  printf( " done.\n" );
   
  // steal the global clock - a bit aggressive, but a simple approach
  if( GlobalTime ) delete GlobalTime;
  assert( (GlobalTime = new StgTime( this->world ) ));

  // start the simulation
  printf( "  Starting world clock... " ); fflush(stdout);
  //stg_world_resume( world );

  world->paused = FALSE;
  puts( "done." );
  

  // Create simulation interface
  player_device_id_t id;
  if (cf->ReadDeviceId(&id, section, "provides", PLAYER_SIMULATION_CODE, 0, NULL) != 0)
    {
      this->SetError(-1);
      return;
    }  

  if (this->AddInterface(id, PLAYER_ALL_MODE,
                         sizeof(player_simulation_data_t),
                         sizeof(player_simulation_cmd_t), 10, 10) != 0)
    {
      DRIVER_ERROR( "failed to add simulation interface" );
      this->SetError(-1);    
      return;
    }
  
 
  this->StartThread();

  // make Player call Update() on this device even when noone is subscribed
  //this->alwayson = TRUE;
}



void StgSimulation::Main()
{




