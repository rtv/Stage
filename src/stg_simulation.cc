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
 * $Id: stg_simulation.cc,v 1.4 2004-09-24 20:58:30 rtv Exp $
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

class StgSimulation:public Stage1p4
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
  : Stage1p4( cf, section, 
	      PLAYER_SIMULATION_CODE, PLAYER_ALL_MODE,
	      sizeof(player_simulation_data_t), 
	      sizeof(player_simulation_cmd_t), 1, 1 )
{
  PLAYER_MSG0( "constructing stg_simulation device" );
  
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
      PLAYER_ERROR1( "worldfile \"%s\" does not exist", worldfile_name );
      return;
    }
  
  //printf( "    Creating Stage client..." ); fflush(stdout);
  //Stage1p4::stage_client = stg_client_create();
  //assert(Stage1p4::stage_client);
  //puts( " done" );
  
  
  // create a passel of Stage models in the local cache based on the
  // worldfile
  printf( "    Stage driver loading worldfile \"%s\" ", worldfile_name );      
  fflush(stdout);
  Stage1p4::world = NULL;

  this->world = stg_world_create_from_file( worldfile_name );
  assert(this->world);
  printf( " done.\n" );
   
  // steal the global clock - a bit aggressive, but a simple approach
  if( GlobalTime ) delete GlobalTime;
  assert( (GlobalTime = new StgTime( this->world ) ));

  // start the simulation
  printf( "    Starting world clock... " ); fflush(stdout);
  //stg_world_resume( world );

  world->paused = FALSE;
  puts( "done." );

  
  this->StartThread();

  // make Player call Update() on this device even when noone is subscribed
  //this->alwayson = TRUE;
}

StgSimulation::~StgSimulation()
{
  stg_world_destroy( this->world );
}

// Player registration ----------------------------------------------------

Driver* StgSimulation_Init( ConfigFile* cf, int section)
{
  return((Driver*)(new StgSimulation( cf, section)));
}


void StgSimulation_Register(DriverTable* table)
{
  table->AddDriver("stg_simulation",  StgSimulation_Init);
}


void StgSimulation::Main()
{
  //int d=0;

  while(1)
    {   
      // test if we are supposed to cancel
      pthread_testcancel();

      if( this->world )
	{
	  //printf( "  D = %d\r", ++d ); fflush(stdout);
	  //gui_poll(); // update the Stage window
	  
	  stg_world_update( this->world );

	  //usleep(1000); // take some strain off the CPU
	}
    }

  // in case we break out of the loop above
  pthread_exit(NULL);
}

int StgSimulation::Setup()
{
  PRINT_WARN( "stg_simulation setup" );

  return 0; //ok
}

int StgSimulation::Shutdown()
{
  PRINT_WARN( "stg_simulation shutdown" );
  return 0; // ok
}




