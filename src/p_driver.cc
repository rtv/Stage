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
 * CVS: $Id: p_driver.cc,v 1.27 2006-01-30 06:58:15 rtv Exp $
 */

// DOCUMENTATION ------------------------------------------------------------

/** @defgroup player libstageplugin - Stage plugin driver for Player

<b>libstageplugin</b> is a plugin for Player that allows Player
clients to access simulated robots as if they were normal Player
devices.

<p>Stage robots and sensors work like any other Player device: users
write robot controllers and sensor algorithms as `clients' to the
Player `server'. Typically, clients cannot tell the difference between
the real robot devices and their simulated Stage equivalents (unless
they try very hard). We have found that robot controllers developed as
Player clients controlling Stage robots will work with little or no
modification with the real robots and vice versa [1]. With a little
care, the same binary program can often control Stage robots and real
robots without even being recompiled [2]. Thus the Player/Stage system
allows rapid prototyping of controllers destined for real
robots. Stage can also be very useful by simulating populations of
realistic robot devices you don't happen to own [3].

@par Authors

Richard Vaughan

[@ref refs]
  
@par Configuration file examples:

Creating two models in a Stage worldfile, saved as "example.world":

@verbatim
# create a position model - it can drive around like a robot
position
(
  name "marvin"
  color "red"

  pose [ 1 1 0 ]

  # add a laser scanner on top of the robot
  laser() 
)

# create a position model with a gripper attached to each side
position
(
  name "gort"
  color "silver"
  pose [ 6 1 0 ]
  size [ 1 2 ]

  gripper( pose [0  1  90] )
  gripper( pose [0 -1 -90] )
)
@endverbatim


Attaching Player interfaces to the Stage models "marvin" and "gort" in a Player config (.cfg) file:

@verbatim
# Present a simulation interface on the default port (6665).
# Load the Stage plugin driver and create the world described
# in the worldfile "example.world" 
# The simulation interface MUST be created before any simulated devices 
# models can be used.
driver
(		
  name "stage"
  provides ["simulation:0"]
  plugin "libstageplugin"
  worldfile "example.world"	
)

# Present a position interface on the default port (6665), connected
# to the Stage position model "marvin", and a laser interface connected to
# the laser child of "marvin".
driver
(
  name "stage"
  provides ["position:0" "laser:0"]
  model "marvin"
)

# Present three interfaces on port 6666, connected to the Stage 
# position model "gort" and its two grippers.
driver
(
  name "stage"
  provides ["6666:position:0" "6666:gripper:0" "6666:gripper:1"]
  model "gort"
)
@endverbatim

More examples can be found in the Stage source tree, in directory
<stage-version>/worlds.

@par Player configuration file options

- worldfile \<string\>
  - where \<string\> is the filename of a Stage worldfile. Player will attempt to load the worldfile and attach interfaces to Stage models specified by the "model" keyword
- model \<string\>
  - where \<string\> is the name of a Stage position model that will be controlled by this interface. Stage will search down the tree of models starting at the named model to find a device of the right type.


@par Provides

The stage plugin driver provides the following device interfaces:

*/


// TODO - configs I should implement
//  - PLAYER_SONAR_POWER_REQ
//  - PLAYER_BLOBFINDER_SET_COLOR_REQ
//  - PLAYER_BLOBFINDER_SET_IMAGER_PARAMS_REQ

// CODE ------------------------------------------------------------

#include <unistd.h>
#include <string.h>
#include <math.h>

#include "p_driver.h"
#include "zoo_driver.h"

#define STG_DEFAULT_WORLDFILE "default.world"
#define DRIVER_ERROR(X) printf( "Stage driver error: %s\n", X )

// globals from Player
extern PlayerTime* GlobalTime;
//extern int global_argc;
//extern char** global_argv;
extern bool player_quiet_startup;
extern bool player_quit;

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
    
  if( !player_quiet_startup )
    {
      puts( "\n * Part of the Player/Stage Project [http://playerstage.sourceforge.net]\n"
	    " * Copyright 2000-2005 Richard Vaughan, Andrew Howard, Brian Gerkey\n * and contributors.\n"
	    " * Released under the GNU GPL." );
    }
    
  table->AddDriver( "stage", StgDriver_Init);
}
  
int player_driver_init(DriverTable* table)
{
  puts(" Stage driver plugin init");
  StgDriver_Register(table);
  //ZooDriver_Register(table);
  return(0);
}

// find a model to attach to a Player interface
stg_model_t* model_match( stg_model_t* mod, 
			  player_devaddr_t *addr, 
			  stg_model_initializer_t init, 
			  GPtrArray* devices )
{
  if( mod->typerec->initializer == init )
    return mod;
 
  // else try the children
  stg_model_t* match=NULL;

  // for each model in the child list
  for(int i=0; i<(int)mod->children->len; i++ )
    {
      // recurse
      match = 
	model_match( (stg_model_t*)g_ptr_array_index( mod->children, i ), 
		     addr, init, devices );      
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
	      
	      // if we have this type of interface on this model already, it's no-go.
	      if( match == interface->mod && interface->addr.interf == addr->interf )
		{
		  printf( "[MODEL ALREADY HAS AN INTERFACE]" );
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



InterfaceModel::InterfaceModel(  player_devaddr_t addr, 
				 StgDriver* driver,
				 ConfigFile* cf, 
				 int section,
				 stg_model_initializer_t init )
  : Interface( addr, driver, cf, section )
{
  //puts( "InterfaceModel constructor" );
  
  char* model_name = (char*)cf->ReadString(section, "model", NULL );
  
  if( model_name == NULL )
    {
      PRINT_ERR1( "device \"%s\" uses the Stage driver but has "
		  "no \"model\" value defined. You must specify a "
		  "model name that matches one of the models in "
		  "the worldfile.",
		  model_name );
      return; // error
    }
  
  // find an appropriate model for this interface and model type
  this->mod = driver->LocateModel( model_name, &addr, init );
  
  if( !this->mod )
    {
      printf( " ERROR! no model available for this device."
	      " Check your world and config files.\n" );

      exit(-1);
      return;
    }
  
  if( !player_quiet_startup )
    printf( "\"%s\"\n", this->mod->token );
}      



// Constructor.  Retrieve options from the configuration file and do any
// pre-Setup() setup.
StgDriver::StgDriver(ConfigFile* cf, int section)
    : Driver(cf, section, false, PLAYER_MSGQUEUE_DEFAULT_MAXLEN)
{  
  // init the array of device ids
  this->devices = g_ptr_array_new();
  
  int device_count = cf->GetTupleCount( section, "provides" );
  
  if( !player_quiet_startup )
    {  
      printf( "  Stage driver creating %d %s\n", 
	      device_count, 
	      device_count == 1 ? "device" : "devices" );
    }
  
  for( int d=0; d<device_count; d++ )
    {
      player_devaddr_t player_addr;
      
      if (cf->ReadDeviceAddr( &player_addr, section, 
                              "provides", 0, d, NULL) != 0)
	{
	  this->SetError(-1);
	  return;
	}  
            
      if( !player_quiet_startup )
	{
	  printf( "    mapping %d.%d.%d => ", 
		  player_addr.robot, player_addr.interf, player_addr.index );
	  fflush(stdout);
	}
      
      
      Interface *ifsrc = NULL;
      
      switch( player_addr.interf )
	{
        case PLAYER_SIMULATION_CODE:
          ifsrc = new InterfaceSimulation( player_addr, this, cf, section );
          break;
	  
	case PLAYER_POWER_CODE:	  
	  ifsrc = new InterfacePower( player_addr,  this, cf, section );
	  break;

	case PLAYER_LASER_CODE:	  
	  ifsrc = new InterfaceLaser( player_addr,  this, cf, section );
	  break;
	  
	case PLAYER_POSITION2D_CODE:	  
	  ifsrc = new InterfacePosition( player_addr, this,  cf, section );
	  break;
	  
	case PLAYER_SONAR_CODE:
	  ifsrc = new InterfaceSonar( player_addr,  this, cf, section );
	  break;
	  
	case PLAYER_BLOBFINDER_CODE:
	  ifsrc = new InterfaceBlobfinder( player_addr,  this, cf, section );
	  break;
	  
 	case PLAYER_PTZ_CODE:
 	  ifsrc = new InterfacePtz( player_addr,  this, cf, section );
 	  break;
	  
	case PLAYER_FIDUCIAL_CODE:
	  ifsrc = new InterfaceFiducial( player_addr,  this, cf, section );
	  break;	  

	case PLAYER_LOCALIZE_CODE:
	  ifsrc = new InterfaceLocalize( player_addr,  this, cf, section );
	  break;	  
	  
	case PLAYER_MAP_CODE:
	  ifsrc = new InterfaceMap( player_addr,  this, cf, section );
	  break;	  
	  
	case PLAYER_GRIPPER_CODE:
	  ifsrc = new InterfaceGripper( player_addr,  this, cf, section );
	  break;	  

	default:
	  PRINT_ERR1( "error: stage driver doesn't support interface type %d\n",
		      player_addr.interf );
	  this->SetError(-1);
	  return; 
	}
      
      if( ifsrc )
	{
	  // attempt to add this interface and we're done
	  if( this->AddInterface( ifsrc->addr))
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
		      player_addr.robot, 
                      player_addr.interf, 
                      player_addr.index );
	  
	  this->SetError(-3);
	  return;
	} 
    }
  //puts( "  Stage driver loaded successfully." );
}

stg_model_t*  StgDriver::LocateModel( char* basename,  
				      player_devaddr_t* addr,
				      stg_model_initializer_t init )
{  
  //printf( "attempting to resolve Stage model \"%s\"", basename );
  
  stg_model_t* base_model = 
    stg_world_model_name_lookup( StgDriver::world, basename );
  
  if( base_model == NULL )
    {
      PRINT_ERR1( " Error! can't find a Stage model named \"%s\"", 
		  basename );
      return NULL;
    }

  // if there's no init function specified, we'll just match the named
  // model
  if( init == NULL )
    return base_model;

  //printf( "found base model %s\n", base_model->token );
  
  // now find the model for this player device find the first model in
  // the tree that is the right type (i.e. has the right
  // initialization function) and has not been used before
  return( model_match( base_model, addr, init, this->devices ) );
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
Interface* StgDriver::LookupDevice( player_devaddr_t addr )
{
  for( int i=0; i<(int)this->devices->len; i++ )
    {
      Interface* candidate = 
	(Interface*)g_ptr_array_index( this->devices, i );
      
      if( candidate->addr.robot == addr.robot &&
	  candidate->addr.interf == addr.interf &&
	  candidate->addr.index == addr.index )
	return candidate; // found
    }

  return NULL; // not found
}


// subscribe to a device
int StgDriver::Subscribe(player_devaddr_t addr)
{
  if( addr.interf == PLAYER_SIMULATION_CODE )
    return 0; // ok

  Interface* device = this->LookupDevice( addr );
  
  if( device )
    {      
      stg_model_subscribe( device->mod );  
      return Driver::Subscribe(addr);
    }

  puts( "failed to find a device" );
  return 1; // error
}


// unsubscribe to a device
int StgDriver::Unsubscribe(player_devaddr_t addr)
{
  if( addr.interf == PLAYER_SIMULATION_CODE )
    return 0; // ok

  Interface* device = this->LookupDevice( addr );
  
  if( device )
    {
      stg_model_unsubscribe( device->mod );  
      return Driver::Unsubscribe(addr);
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


// All incoming messages get processed here.  This method is called by
// Driver::ProcessMessages() once for each message.
// Driver::ProcessMessages() is called by StgDriver::Update(), which is
// called periodically by player.
int 
StgDriver::ProcessMessage(MessageQueue* resp_queue, 
			  player_msghdr * hdr, 
			  void * data)
{
  // find the right interface to handle this config
  Interface* in = this->LookupDevice( hdr->addr );
  if( in )
  {
    return(in->ProcessMessage( resp_queue, hdr, data));
  }
  else
  {
    PRINT_WARN3( "can't find interface for device %d.%d.%d",
		 this->device_addr.robot, 
		 this->device_addr.interf, 
		 this->device_addr.index );
    return(-1);
  }
}

void StgDriver::Update(void)
{
  Driver::ProcessMessages();

  for( int i=0; i<(int)this->devices->len; i++ )
  {
    Interface* interface = (Interface*)g_ptr_array_index( this->devices, i );

    // Should this be an assertion? - BPG
    if(!interface)
      continue;

    if( interface->addr.interf == PLAYER_SIMULATION_CODE )
    {
      if( stg_world_update( this->world, FALSE ) )
	player_quit = TRUE; // set Player's global quit flag
    }
    else
    {
      // Has enough time elapsed since the last time we published on this
      // interface?  This really needs some thought, as each model/interface
      // should have a configurable publishing rate. For now, I'm using the
      // world's update rate (which appears to be stored as msec).  - BPG
      double currtime;
      GlobalTime->GetTimeDouble(&currtime);
      if((currtime - interface->last_publish_time) >= 
         (interface->mod->world->sim_interval / 1e3))
      {
        interface->Publish();
        interface->last_publish_time = currtime;
      }
    }
  }

  // update the world
  return;
}


