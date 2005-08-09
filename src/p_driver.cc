/*
 *  Stage plugin driver for Player
 *  Copyright (C) 2004-2005 Richard Vaughan
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

/*
 * Desc: A plugin driver for Player that gives access to Stage devices.
 * Author: Richard Vaughan
 * Date: 10 December 2004
 * CVS: $Id: p_driver.cc,v 1.10 2005-08-09 06:28:57 rtv Exp $
 */

// DOCUMENTATION ------------------------------------------------------------

/** @defgroup player libstageplugin - Stage plugin driver for Player

<b>libstageplugin</b> is a plugin for Player that allows Player
clients to access simulated robots as if they were normal Player
devices.


@par Player configuration file options

- model (string)
  - where (string) is the name of a Stage position model that will be controlled by this interface. Stage will search the tree of models below the named model to find a device of the right type.
  
@par Configuration file examples:

Creating models in a Stage worldfile, saved as "example.world":

@verbatim
# create a position model - it can drive around like a robot
position
(
  pose [ 1 1 0 ]
  color "red"
  name "marvin"

  # add a laser scanner on top of the robot
  laser() 
)
@endverbatim


Using Stage models in a Player config (.cfg) file:

@verbatim
# load the Stage plugin and create a world from a worldfile
driver
(		
  name "stage"
  provides ["simulation:0"]
  plugin "libstageplugin"

  # create the simulated world described by this worldfile
  worldfile "example.world"	
)

# create a position device, connected to a Stage position model 
driver
(
  name "stage"
  provides ["position:0" ]
  model "marvin"
)

# create a laser device, connected to a Stage laser model
driver
(
  name "stage"
  provides ["laser:0" ]
  model "marvin"
)
@endverbatim

More examples can be found in the Stage source tree, in directory
<stage-version>/worlds.

@par Authors

Richard Vaughan

@par Provides

The stage plugin driver provides the following device interfaces and
configuration requests. 

*/


// TODO - configs I should implement
//  - PLAYER_SONAR_POWER_REQ
//  - PLAYER_BLOBFINDER_SET_COLOR_REQ
//  - PLAYER_BLOBFINDER_SET_IMAGER_PARAMS_REQ

// CODE ------------------------------------------------------------

#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <math.h>
#include <player/drivertable.h>

#include "p_driver.h"
#include "zoo_driver.h"

#define STG_DEFAULT_WORLDFILE "default.world"
#define DRIVER_ERROR(X) printf( "Stage driver error: %s\n", X )

// globals from Player
extern PlayerTime* GlobalTime;
extern int global_argc;
extern char** global_argv;
extern bool quiet_startup;
extern bool quit;

// init static vars
stg_world_t* StgDriver::world = NULL;

int update_request = 0;

/* need the extern to avoid C++ name-mangling  */
extern "C"
{
  int player_driver_init(DriverTable* table);
}

// A factory creation function, declared outside of the class so that it
// can be invoked without any object context (alternatively, you can
// declare it static in the class).  In this function, we create and return
// (as a generic Driver*) a pointer to a new instance of this driver.
Driver* StgDriver_Init(ConfigFile* cf, int section)
{
  // Create and return a new instance of this driver
  return ((Driver*) (new StgDriver(cf, section)));
}
  
// A driver registration function, again declared outside of the class so
// that it can be invoked without object context.  In this function, we add
// the driver into the given driver table, indicating which interface the
// driver can support and how to create a driver instance.
void StgDriver_Register(DriverTable* table)
{
  printf( "\n ** Stage plugin v%s **", PACKAGE_VERSION );
    
  if( !quiet_startup )
    {
      puts( "\n * Part of the Player/Stage Project [http://playerstage.sourceforge.net]\n"
	    " * Copyright 2000-2005 Richard Vaughan, Andrew Howard, Brian Gerkey\n * and contributors.\n"
	    " * Released under the GNU GPL." );
    }
    
  table->AddDriver( "stage", StgDriver_Init);
}
  
int player_driver_init(DriverTable* table)
{
  //puts(" Stage driver plugin init");
  StgDriver_Register(table);
  ZooDriver_Register(table);
  return(0);
}

// find a model to attach to a Player interface
stg_model_t* model_match( stg_model_t* mod, stg_model_initializer_t init, GPtrArray* devices )
{
  if( mod->initializer == init )
    return mod;
 
  // else try the children
  stg_model_t* match=NULL;

  // for each model in the child list
  for(int i=0; i<(int)mod->children->len; i++ )
    {
      // recurse
      match = 
	model_match( (stg_model_t*)g_ptr_array_index( mod->children, i ), 
		     init, devices );      
      if( match )
	{
	  // if mod appears in devices already, it can not be used now
	  //printf( "[inspecting %d devices used already]", devices->len );
	  for( int i=0; i<(int)devices->len; i++ )
	    {
	      Interface* interface = 
		(Interface*)g_ptr_array_index( devices, i );
	      
	      //printf( "comparing %p and %p (%d.%d.%d)\n", mod, record->mod,
	      //      record->id.port, record->id.code, record->id.index );
	      
	      if( match == interface->mod )//&& ! tp == STG_MODEL_BASIC )
		{
		  //printf( "[ALREADY USED]" );
		  //return NULL;
		  match = NULL;
		}
	    }
	  // if we found a match, we're done searching
	  //return match;
	  if( match ) return match;
	}
    }

  return NULL;
}



InterfaceModel::InterfaceModel(  player_device_id_t id, 
				 StgDriver* driver,
				 ConfigFile* cf, 
				 int section,
				 stg_model_initializer_t init )
  : Interface( id, driver, cf, section )
{
  //puts( "InterfaceModel constructor" );
  
  const char* model_name = cf->ReadString(section, "model", NULL );
  
  if( model_name == NULL )
    {
      PRINT_ERR1( "device \"%s\" uses the Stage driver but has "
		  "no \"model\" value defined. You must specify a "
		  "model name that matches one of the models in "
		  "the worldfile.",
		  model_name );
      return; // error
    }
  
  this->mod = driver->LocateModel( model_name, init );
  
  if( !this->mod )
    {
      printf( " ERROR! no model available for this device."
	      " Check your world and config files.\n" );

      exit(-1);
      return;
    }
  
  if( !quiet_startup )
    printf( "\"%s\"\n", this->mod->token );
}      



// Constructor.  Retrieve options from the configuration file and do any
// pre-Setup() setup.
StgDriver::StgDriver(ConfigFile* cf, int section)
    : Driver(cf, section)
{  
  // init the array of device ids
  this->devices = g_ptr_array_new();
  
  int device_count = cf->GetTupleCount( section, "provides" );
  
  if( !quiet_startup )
    {  
      printf( "  Stage driver creating %d %s\n", 
	      device_count, 
	      device_count == 1 ? "device" : "devices" );
    }
  
  for( int d=0; d<device_count; d++ )
    {
      player_device_id_t player_id;
      
      if (cf->ReadDeviceId( &player_id, section, "provides", 0, d, NULL) != 0)
	{
	  this->SetError(-1);
	  return;
	}  
            
      if( !quiet_startup )
	{
	  printf( "    mapping %d.%d.%d => ", 
		  player_id.port, player_id.code, player_id.index );
	  fflush(stdout);
	}
      
      
      Interface *ifsrc = NULL;

      switch( player_id.code )
	{
	case PLAYER_SIMULATION_CODE:
	  ifsrc = new InterfaceSimulation( player_id, this, cf, section );
	  break;
	  
	case PLAYER_POSITION_CODE:	  
	  ifsrc = new InterfacePosition( player_id, this,  cf, section );
	  break;
	  
	case PLAYER_LASER_CODE:	  
	  ifsrc = new InterfaceLaser( player_id,  this, cf, section );
	  break;

	case PLAYER_FIDUCIAL_CODE:
	  ifsrc = new InterfaceFiducial( player_id,  this, cf, section );
	  break;
	  
	case PLAYER_BLOBFINDER_CODE:
	  ifsrc = new InterfaceBlobfinder( player_id,  this, cf, section );
	  break;
	  
	case PLAYER_SONAR_CODE:
	  ifsrc = new InterfaceSonar( player_id,  this, cf, section );
	  break;
	  
	case PLAYER_GRIPPER_CODE:
	  ifsrc = new InterfaceGripper( player_id,  this, cf, section );
	  break;
	  
	  //case PLAYER_MAP_CODE:
	  //break;

	default:
	  PRINT_ERR1( "error: stage driver doesn't support interface type %d\n",
		      player_id.code );
	  this->SetError(-1);
	  return; 
	}
      
      if( ifsrc )
	{
	  // attempt to add this interface and we're done
	  if( this->AddInterface( ifsrc->id, 
				  PLAYER_ALL_MODE, 
				  ifsrc->data_len, 
				  ifsrc->cmd_len,
				  ifsrc->req_qlen,
				  ifsrc->req_qlen ) )
	    {
	      DRIVER_ERROR( "AddInterface() failed" );
	      this->SetError(-2);
	      return;
	    }
	  
	  // store the Interaface in our device list
	  g_ptr_array_add( this->devices, ifsrc );
	}
      else
	{
	  PRINT_ERR3( "No Stage source found for interface %d:%d:%d",
		      player_id.port, player_id.code, player_id.index );
	  
	  this->SetError(-3);
	  return;
	} 
    }
  //puts( "  Stage driver loaded successfully." );
}

stg_model_t*  StgDriver::LocateModel( const char* basename,  
				      stg_model_initializer_t init )
{  
  //PLAYER_TRACE1( "attempting to resolve Stage model \"%s\"", model_name );
  //printf( "attempting to resolve Stage model \"%s\"", model_name );
  
  stg_model_t* base_model = 
    stg_world_model_name_lookup( StgDriver::world, basename );
  
  if( base_model == NULL )
    {
      PRINT_ERR1( " Error! can't find a Stage model named \"%s\"", 
		  basename );
      return NULL;
    }
  
  // printf( "found base model %s\n", base_model->token );
  
  // todo
  // map interface can attach only to the base model
  //if( device->id.code == PLAYER_MAP_CODE )
  //return base_model;
  
  // now find the model for this player device find the first model in
  // the tree that is the right type (i.e. has the right
  // initialization function) and has not been used before
  return( model_match( base_model, init, this->devices ) );
}

////////////////////////////////////////////////////////////////////////////////
// Set up the device.  Return 0 if things go well, and -1 otherwise.
int StgDriver::Setup()
{   
  //puts("stage driver setup");  
  return(0);
}

// find the device record with this Player id
// todo - faster lookup with a better data structure
Interface* StgDriver::LookupDevice( player_device_id_t id )
{
  for( int i=0; i<(int)this->devices->len; i++ )
    {
      Interface* candidate = 
	(Interface*)g_ptr_array_index( this->devices, i );
      
      if( candidate->id.port == id.port &&
	  candidate->id.code == id.code &&
	  candidate->id.index == id.index )
	return candidate; // found
    }

  return NULL; // not found
}


// subscribe to a device
int StgDriver::Subscribe(player_device_id_t id)
{
  if( id.code == PLAYER_SIMULATION_CODE )
    return 0; // ok

  Interface* device = this->LookupDevice( id );
  
  if( device )
    {      
      stg_model_subscribe( device->mod );  
      return Driver::Subscribe(id);
    }

  puts( "failed to find a device" );
  return 1; // error
}


// unsubscribe to a device
int StgDriver::Unsubscribe(player_device_id_t id)
{
  if( id.code == PLAYER_SIMULATION_CODE )
    return 0; // ok

  Interface* device = this->LookupDevice( id );
  
  if( device )
    {
      stg_model_unsubscribe( device->mod );  
      return Driver::Unsubscribe(id);
    }
  else
    return 1; // error
}

StgDriver::~StgDriver()
{
  // todo - when the sim thread exits, destroy the world. It's not
  // urgent, because right now this only happens when Player quits.
  // stg_world_destroy( StgDriver::world );

  //puts( "Stage driver destroyed" );
}


////////////////////////////////////////////////////////////////////////////////
// Shutdown the device
int StgDriver::Shutdown()
{
  puts("Shutting stage driver down");

  // Stop and join the driver thread
  // this->StopThread(); // todo - the thread only runs in the sim instance

  // shutting unsubscribe to data from all the devices
  //for( int i=0; i<(int)this->devices->len; i++ )
  //{
  //  Interface* device = (Interface*)g_ptr_array_index( this->devices, i );
  //  stg_model_unsubscribe( device->mod );
  // }

  puts("stage driver has been shutdown");

  return(0);
}


// Moved this declaration here (and changed its name) because 
// when it's declared on the stack inside StgDriver::Update below, it 
// corrupts the stack.  This makes makes valgrind unhappy (errors about
// invalid reads/writes to another thread's stack.  This may also be related
// to occasional mysterious crashed with Stage that I've seen.
//        - BPG
uint8_t stage_buffer[ PLAYER_MAX_MESSAGE_SIZE ];

int StgDriver::PutConfig(player_device_id_t id, void* cli, 
			 void* src, size_t len,
			 struct timeval* timestamp)
{
  // inherit normal behaviour, possibly failing
  if( Driver::PutConfig( id, cli, src,len, timestamp ) < 0 )
    return -1;
  
  // now handle the config right here.
  void* cfg_client = NULL;
  int cfg_len = 
    this->GetConfig( id, &cfg_client, 
		     stage_buffer, PLAYER_MAX_MESSAGE_SIZE, NULL );
  
  if( cfg_len > 0 )
    {
      // find the right interface to handle this config
      Interface* in = this->LookupDevice( id );
      if( in )
	in->Configure( cfg_client, stage_buffer, cfg_len );
      else
	PRINT_WARN3( "can't find interface for device %d.%d.%d",
		     id.port, id.code, id.index );
    }
  return 0; //ok
}                 


void StgDriver::Update(void)
{  
  for( int i=0; i<(int)this->devices->len; i++ )
    {
      Interface* interface = (Interface*)g_ptr_array_index( this->devices, i );
      
      if( interface && interface->id.code == PLAYER_SIMULATION_CODE )
	{
	  if( stg_world_update( this->world, FALSE ) )
	    quit = TRUE; // set Player's global quit flag
	}

      if( interface && interface->cmd_len > 0 )
	{    
	  // copy the command from Player
	  size_t cmd_len = 
	    this->GetCommand( interface->id, stage_buffer, 
			      interface->cmd_len, NULL);
	  
	  if( cmd_len > 0 )
	    interface->Command( stage_buffer, cmd_len );
	}
      
      interface->Publish();
    }
  
  // update the world
  return;
}

