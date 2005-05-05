/*
 *  Player - One Hell of a Robot Server
 *  Copyright (C) 2004, 2005 Richard Vaughan
 *                      
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
 * CVS: $Id: player_driver.cc,v 1.9 2005-05-05 20:10:30 rtv Exp $
 */

// DOCUMENTATION ------------------------------------------------------------

/** @defgroup driver_stage Stage plugin driver for Player

This driver gives Player access to Stage's models.

@par Provides

The stage plugin driver provides the following device interfaces and
configuration requests. The name of each interface
is a link to its entry in the Player manual:

- <a href="http://playerstage.sourceforge.net/doc/Player-1.6-html/player/group__player__interface__blobfinder.html">blobfinder</a>
 - (none)

- <a href="http://playerstage.sourceforge.net/doc/Player-1.6-html/player/group__player__interface__fiducial.html">fiducial</a>
  - PLAYER_FIDUCIAL_GET_GEOM_REQ
  - PLAYER_FIDUCIAL_SET_FOV_REQ
  - PLAYER_FIDUCIAL_GET_FOV_REQ
  - PLAYER_FIDUCIAL_SET_ID_REQ
  - PLAYER_FIDUCIAL_GET_ID_REQ

- <a href="http://playerstage.sourceforge.net/doc/Player-1.6-html/player/group__player__interface__laser.html">laser</a>
 - PLAYER_LASER_SET_CONFIG
 - PLAYER_LASER_SET_CONFIG
 - PLAYER_LASER_GET_GEOM

- <a href="http://playerstage.sourceforge.net/doc/Player-1.6-html/player/group__player__interface__position.html">position</a>
  - PLAYER_POSITION_SET_ODOM_REQ
  - PLAYER_POSITION_RESET_ODOM_REQ
  - PLAYER_POSITION_GET_GEOM_REQ
  - PLAYER_POSITION_MOTOR_POWER_REQ
  - PLAYER_POSITION_VELOCITY_MODE_REQ

- <a href="http://playerstage.sourceforge.net/doc/Player-1.6-html/player/group__player__interface__sonar.html">sonar</a>
  - PLAYER_SONAR_GET_GEOM_REQ

- <a href="http://playerstage.sourceforge.net/doc/Player-1.6-html/player/group__player__interface__simulation.html">simulation</a>
  - (none)



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

#include "player_interfaces.h"

extern PlayerTime* GlobalTime;
extern int global_argc;
extern char** global_argv;

//#define HTON_M(m) htonl(m)   // byte ordering for METERS
//#define NTOH_M(m) ntohl(m)
//#define HTON_RAD(r) htonl(r) // byte ordering for RADIANS
//#define NTOH_RAD(r) ntohl(r)
//#define HTON_SEC(s) htonl(s) // byte ordering for SECONDS
//#define NTOH_SEC(s) ntohl(s)
//#define HTON_SEC(s) htonl(s) // byte ordering for SECONDS
//#define NTOH_SEC(s) ntohl(s)


#define STG_DEFAULT_WORLDFILE "default.world"
#define DRIVER_ERROR(X) printf( "Stage driver error: %s\n", X )



// init static vars
stg_world_t* StgDriver::world = NULL;


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
  printf( "\n Stage plugin driver %s\n", PACKAGE_VERSION );
  table->AddDriver( "stage", StgDriver_Init);
}


////////////////////////////////////////////////////////////////////////////////
// Extra stuff for building a shared object.

// Need access to the global driver table
#include <player/drivertable.h>

/* need the extern to avoid C++ name-mangling  */
extern "C"
{
  int player_driver_init(DriverTable* table)
  {
    //puts(" Stage driver plugin init");
    StgDriver_Register(table);
    return(0);
  }
}

// find a model to attach to a Player interface
stg_model_t* model_match( stg_model_t* mod, stg_model_type_t tp, GPtrArray* devices )
{
  if( mod->type == tp )
    return mod;
 
  // else try the children
  stg_model_t* match=NULL;

  // for each model in the child list
  for(int i=0; i<(int)mod->children->len; i++ )
    {
      // recurse
      match = 
	model_match( (stg_model_t*)g_ptr_array_index( mod->children, i ), 
		     tp, devices );      
      if( match )
	{
	  // if mod appears in devices already, it can not be used now
	  //printf( "[inspecting %d devices used already]", devices->len );
	  for( int i=0; i<(int)devices->len; i++ )
	    {
	      device_record_t* record =  
		(device_record_t*)g_ptr_array_index( devices, i );
	      
	      //printf( "comparing %p and %p (%d.%d.%d)\n", mod, record->mod,
	      //      record->id.port, record->id.code, record->id.index );
	      
	      if( match == record->mod )//&& ! tp == STG_MODEL_BASIC )
		{
		  printf( "[ALREADY USED]" );
		  return NULL;
		}
	    }
	  // if we found a match, we're done searching
	  return match;
	}
    }

  return NULL;
}


int StgDriver::CreateDeviceSimulation( player_device_id_t id, 
				       ConfigFile* cf, int section )
{
  // boot libstage 
  stg_init( global_argc, global_argv );
  
  device_record_t* device = 
    (device_record_t*)calloc(sizeof(device_record_t),1);
  
  memcpy( &device->id, &id, sizeof(id) );
  
  // tell the device which driver created it
  device->driver = this;
  
  device->data_len = sizeof(player_simulation_data_t);
  device->cmd_len = sizeof(player_simulation_cmd_t);
  device->command_callback = NULL;
  device->data_callback = SimulationData;
  device->config_callback = SimulationConfig;
  
  const char* worldfile_name = cf->ReadString(section, "worldfile", NULL );
  
  if( worldfile_name == NULL )
    {
      PRINT_ERR1( "device \"%s\" uses the Stage driver but has "
		  "no \"model\" value defined. You must specify a "
		  "model name that matches one of the models in "
		  "the worldfile.",
		  worldfile_name );
      return 1; // error
    }
  
  char fullname[MAXPATHLEN];
  
  if( worldfile_name[0] == '/' )
    strcpy( fullname, worldfile_name );
  else
    {
      char *tmp = strdup(cf->filename);
      snprintf( fullname, MAXPATHLEN, 
		"%s/%s", dirname(tmp), worldfile_name );      
      free(tmp);
    }
  
  // a little sanity testing
  if( !g_file_test( fullname, G_FILE_TEST_EXISTS ) )
    {
      PRINT_ERR1( "worldfile \"%s\" does not exist", worldfile_name );
      return -1;
    }
  
  // create a passel of Stage models in the local cache based on the
  // worldfile
  printf( "\"%s\" ", fullname );      
  fflush(stdout);

  StgDriver::world = stg_world_create_from_file( fullname );
  assert(StgDriver::world);
  //printf( " done.\n" );
  
  // steal the global clock - a bit aggressive, but a simple approach
  if( GlobalTime ) delete GlobalTime;
  assert( (GlobalTime = new StgTime( StgDriver::world ) ));
  
  // start the simulation
  // printf( "  Starting world clock... " ); fflush(stdout);
  //stg_world_resume( world );
  
  world->paused = FALSE;
  
  // this causes Driver::Update() to be called even when the device is
  // not subscribed
  this->alwayson = TRUE;    

  // Start the device thread; spawns a new thread and executes
  // StgDriver::Main(), which contains the main loop for the driver.
  //puts( "\nStarting thread" );
  this->StartThread();

  puts( "" ); // end the Stage startup line

  // now poke a data callback function into the model  
  //device->mod->data_notify = StgDriver::RefreshDataCallback;
  //device->mod->data_notify_arg = device;
  
  // and add the device to the driver's list of devices
  g_ptr_array_add( this->devices, device );
  
  // attempt to add this interface and we're done
  return( this->AddInterface( device->id, 
			      PLAYER_ALL_MODE, 
			      device->data_len, 
			      device->cmd_len,
			      10, 10) );
}      

int StgDriver::CreateDeviceModel( player_device_id_t id, 
				  ConfigFile* cf, int section )
{
  device_record_t* device = 
    (device_record_t*)calloc(sizeof(device_record_t),1);
  
  memcpy( &device->id, &id, sizeof(id) );

  const char* model_name = cf->ReadString(section, "model", NULL );

  if( model_name == NULL )
    {
      PRINT_ERR1( "device \"%s\" uses the Stage driver but has "
		  "no \"model\" value defined. You must specify a "
		  "model name that matches one of the models in "
		  "the worldfile.",
		  model_name );
      return 1; // error
    }
  
  // tell the device which driver created it
  device->driver = this;
  
  switch( device->id.code )
    {	  
    case PLAYER_SIMULATION_CODE:
      puts( "Error: attempt to create a simulation device as a model" );
      return 1;
      
    case PLAYER_POSITION_CODE:
      device->data_len = sizeof(player_position_data_t);
      device->cmd_len = sizeof(player_position_cmd_t);
      device->command_callback = PositionCommand;
      device->data_callback = PositionData;
      device->config_callback = PositionConfig;
      device->mod = this->LocateModel( model_name, STG_MODEL_POSITION );
      break;
      
      /*
	case PLAYER_LOCALIZE_CODE:
	device->data_len = sizeof(player_position_data_t);
	device->cmd_len = sizeof(player_position_cmd_t);
	device->command_callback = LocalizeCommand;
	device->data_callback = LocalizeData;
	device->config_callback = LocalizeConfig;
	this->SetError( this->InitModel(device, cf, section, STG_MODEL_POSITION));
	break;
      */
      
    case PLAYER_LASER_CODE:
      device->data_len = sizeof(player_laser_data_t);
      device->cmd_len = 0;
      device->command_callback = NULL;
      device->data_callback = LaserData;
      device->config_callback = LaserConfig;
      device->mod = this->LocateModel( model_name, STG_MODEL_LASER );
      break;
      
    case PLAYER_FIDUCIAL_CODE:
      device->data_len = sizeof(player_fiducial_data_t);
      device->cmd_len = 0;
      device->command_callback = NULL;
      device->data_callback = FiducialData;
      device->config_callback = FiducialConfig;
      device->mod = this->LocateModel( model_name, STG_MODEL_FIDUCIAL );
      break;
      
    case PLAYER_SONAR_CODE:
      device->data_len = sizeof(player_sonar_data_t);
      device->cmd_len = 0;
      device->command_callback = NULL;
      device->data_callback = SonarData;
      device->config_callback = SonarConfig;
      device->mod = this->LocateModel( model_name, STG_MODEL_RANGER );
      break;
      
    case PLAYER_ENERGY_CODE:
      device->data_len = sizeof(player_energy_data_t);
      device->cmd_len = 0;
      device->command_callback = NULL;
      device->data_callback = EnergyData;
      device->config_callback = EnergyConfig;
      device->mod = this->LocateModel( model_name, STG_MODEL_ENERGY );
      break;
      
    case PLAYER_BLOBFINDER_CODE:
      device->data_len = sizeof(player_blobfinder_data_t);
      device->cmd_len = 0;
      device->command_callback = NULL;
      device->data_callback = BlobfinderData;
      device->config_callback = BlobfinderConfig;
      device->mod = this->LocateModel( model_name, STG_MODEL_BLOB );
      break;
      
    case PLAYER_MAP_CODE:
      device->data_len = 0; // no cmds or data for maps
      device->cmd_len = 0;
      device->command_callback = NULL;
      device->data_callback = NULL;
      device->config_callback = MapConfig;
      device->mod = stg_world_model_name_lookup( StgDriver::world, model_name );
      break;
      
    default:
      PRINT_ERR1( "error: stage driver doesn't support interface type %d\n",
		  device->id.code );
      this->SetError(-1);
      return 2; // fail
    }
  

  if( !device->mod )
    {
      printf( " ERROR! no model available for this device."
	      " Check your world and config files.\n" );
      return -1;
    }
  
  printf( "\"%s\"\n", device->mod->token );
  
  // now poke a data callback function into the model  
  device->mod->data_notify = StgDriver::RefreshDataCallback;
  device->mod->data_notify_arg = device;
  
  // and add the device to the driver's list of devices
  g_ptr_array_add( this->devices, device );
  
  // attempt to add this interface and we're done
  return( this->AddInterface( device->id, 
			      PLAYER_ALL_MODE, 
			      device->data_len, 
			      device->cmd_len,
			      10, 10) );
}      

////////////////////////////////////////////////////////////////////////////////
// Constructor.  Retrieve options from the configuration file and do any
// pre-Setup() setup.
StgDriver::StgDriver(ConfigFile* cf, int section)
    : Driver(cf, section)
{  
  // init the array of device ids
  this->devices = g_ptr_array_new();
  
  int device_count = cf->GetTupleCount( section, "provides" );
  
  printf( "  Stage driver creating %d %s\n", 
	  device_count, 
	  device_count == 1 ? "device" : "devices" );

  for( int d=0; d<device_count; d++ )
    {
      player_device_id_t player_id;
      
      if (cf->ReadDeviceId( &player_id, section, "provides", 0, d, NULL) != 0)
	{
	  this->SetError(-1);
	  return;
	}  
            
      printf( "    mapping %d.%d.%d => ", 
	      player_id.port, player_id.code, player_id.index );
      fflush(stdout);
      
      if( player_id.code == PLAYER_SIMULATION_CODE )
	assert( this->CreateDeviceSimulation( player_id, cf, section ) == 0 );
      else
	assert( this->CreateDeviceModel( player_id, cf, section ) ==0 );
    } 
  
  //puts( "  Stage driver loaded successfully." );
}

stg_model_t*  StgDriver::LocateModel( const char* basename,  
				      stg_model_type_t mod_type )
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
  
  // now find the model for this player device
  // find the first model in the tree that is the right type and
  // has not been used before
  return( model_match( base_model, mod_type, this->devices ) );
}

////////////////////////////////////////////////////////////////////////////////
// Set up the device.  Return 0 if things go well, and -1 otherwise.
int StgDriver::Setup()
{   
  puts("stage driver setup");  
  return(0);
}

// find the device record with this Player id
// todo - faster lookup with a better data structure
device_record_t* StgDriver::LookupDevice( player_device_id_t id )
{
  for( int i=0; i<(int)this->devices->len; i++ )
    {
      device_record_t* candidate = 
	(device_record_t*)g_ptr_array_index( this->devices, i );
      
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

  device_record_t* device = this->LookupDevice( id );
  
  if( device )
    {      
      stg_model_subscribe( device->mod );  
      return Driver::Subscribe(id);
    }
  else
    return 1; // error
}


// unsubscribe to a device
int StgDriver::Unsubscribe(player_device_id_t id)
{
  if( id.code == PLAYER_SIMULATION_CODE )
    return 0; // ok

  device_record_t* device = this->LookupDevice( id );
  
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
  //  device_record_t* device = (device_record_t*)g_ptr_array_index( this->devices, i );
  //  stg_model_unsubscribe( device->mod );
  // }

  puts("stage driver has been shutdown");

  return(0);
}


////////////////////////////////////////////////////////////////////////////////
// Main function for device thread
void StgDriver::Main() 
{
  //puts( "stage driver main" );

  assert( StgDriver::world );

  // The main loop; interact with the device here
  for(;;)
  {
    // test if we are supposed to cancel
    pthread_testcancel();

    if( stg_world_update( StgDriver::world, TRUE ) )
      exit( 0 );

    // todo - sleep out here only if we can afford the time
    //usleep( 10000 ); // 1ms
  }
}


// override Device::PutConfig()
int StgDriver::PutConfig(player_device_id_t id, void *client, 
			 void* src, size_t len,
			 struct timeval* timestamp)
{  
  //puts( "StgDriver::PutConfig");

  if( len < 1 )
    {
      PRINT_WARN( "received a config of zero length. Ignoring it." );
      return 0;
    }
  
  struct timeval ts;
  
  if(timestamp)
    ts = *timestamp;
  else
    GlobalTime->GetTime(&ts);
  
  // find this device:  
  device_record_t* drec = NULL;
  
  for( int i=0; i<(int)this->devices->len; i++ )
    {
      device_record_t* candidate = 
	(device_record_t*)g_ptr_array_index( this->devices, i );
      
      if( candidate->id.port == id.port &&
	  candidate->id.code == id.code &&
	  candidate->id.index == id.index )
	{
	  drec = candidate;
	  break;
	}
    }
  
  // if the device was found, call the appropriate config handler
  if( drec && drec->config_callback ) 
    (*drec->config_callback)( drec, client, src, len );
  else
    {	
      PRINT_WARN3( "Failed to find device id (%d:%d:%d)", 
		   id.port, id.code, id.index );
      
      if (this->PutReply( id, client, 
			  PLAYER_MSGTYPE_RESP_NACK, NULL, 0, NULL) != 0)
	DRIVER_ERROR("PutReply() failed");	  
    }
  
  return(0);
}

// Moved this declaration here (and changed its name) because 
// when it's declared on the stack inside StgDriver::Update below, it 
// corrupts the stack.  This makes makes valgrind unhappy (errors about
// invalid reads/writes to another thread's stack.  This may also be related
// to occasional mysterious crashed with Stage that I've seen.
//        - BPG
uint8_t stage_cmd_buffer[ PLAYER_MAX_MESSAGE_SIZE ];

void StgDriver::Update(void)
{
  // puts( "stgdriver update" );

  // Can't declare objects this big (PLAYER_MAX_MESSAGE_SIZE = 2MB) 
  // on the stack.  Moved this declaration above - BPG
  //
  // Check for commands
  // uint8_t cmd[ PLAYER_MAX_MESSAGE_SIZE ];
  
  //printf( "driver %p has %d devices to inspect\n",
  //  this, (int)this->devices->len );
  
  for( int i=0; i<(int)this->devices->len; i++ )
    {
      device_record_t* device = (device_record_t*)g_ptr_array_index( this->devices, i );
      
      //printf( "checking command for %d:%d:%d\n",
      //      device->id.port, 
      //      device->id.code, 
      //      device->id.index );
      
      if( device->cmd_len > 0 )
	{    
	  // copy the command from Player
	  size_t cmd_len = 
	    this->GetCommand( device->id, stage_cmd_buffer, device->cmd_len, NULL);
	  
	  //printf( "command for %d:%d:%d is %d bytes\n",
	  //  device->id.port, 
	  //  device->id.code, 
	  //  device->id.index, 
	  //  (int)cmd_len );
	    
	  if( cmd_len && device && device->command_callback )
	    (*device->command_callback)( device, stage_cmd_buffer, cmd_len );	  
	}
    }
  
  // update the world
  return;
}


void StgDriver::RefreshDataCallback( void* user, void* data, size_t len )
{
  //puts( "refresh data callback" );

  device_record_t* device = (device_record_t*)user;

  if( device->data_callback )
    {
      //printf( "calling data callback for model \"%s\" device %d:%d:%d\n", 
      //      device->mod->token, device->id.port, device->id.code, device->id.index );
      
      (*device->data_callback)( device, data, len );
    }
  else
    printf( "warn: no data callback installed for model \"%s\" device %d:%d:%d\n", 
	    device->mod->token, device->id.port, device->id.code, device->id.index );
}
