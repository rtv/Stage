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
 * CVS: $Id: stg_driver.cc,v 1.30 2005-02-24 04:18:39 rtv Exp $
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
  plugin "libstage"

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

#include "player.h"
#include "player/device.h"
#include "player/driver.h"
#include "player/configfile.h"
#include "player/drivertable.h"
#include <player/drivertable.h>
#include <player/driver.h>
#include <player/error.h>
#include "playercommon.h"

#include "stage_internal.h"
#include "stg_time.h"

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



// map interface macros
//
// compute linear index for given map coords
#define MAP_IDX(mf, i, j) ((mf->size_x) * (j) + (i))
// check that given coords are valid (i.e., on the map)
#define MAP_VALID(mf, i, j) ((i >= 0) && (i < mf->size_x) && (j >= 0) && (j < mf->size_y))



#define STG_DEFAULT_WORLDFILE "default.world"
#define DRIVER_ERROR(X) printf( "Stage driver error: %s\n", X )

// forward declare
class StgDriver;

typedef struct
{
  player_device_id_t id;
  stg_model_t* mod;

  size_t data_len;
  
  StgDriver* driver; // the driver instance that created this device

  void* cmd;
  size_t cmd_len;

  
} device_record_t;

////////////////////////////////////////////////////////////////////////////////
// The class for the driver
class StgDriver : public Driver
{
  // Constructor; need that
public: StgDriver(ConfigFile* cf, int section);
  
  // Destructor
public: ~StgDriver(void);
  
  // Must implement the following methods.
public: int Setup();
public: int Shutdown();
  
  
  // override the Driver method to grab configs inside the server thread
public: virtual int PutConfig(player_device_id_t id, void *client, 
			      void* src, size_t len,
			      struct timeval* timestamp);
  
  // Main function for device thread.
private: virtual void Main();
private: virtual void Update();
  
  //private: void CheckConfig();
private: void CheckCommands();
private: void RefreshData();
  
  // used as a callback when a model has new data
public: static void RefreshDataCallback( void* user );
  
protected: void RefreshDataDevice( device_record_t* device );
  
private:  
  // SIMULATION INTERFACE
  void InitSimulation( ConfigFile* cf, int section );
  void HandleCommandSimulation( device_record_t* device, void* buffer, size_t len );
  
  // POSITION INTERFACE
  void HandleCommandPosition( device_record_t* device, void* buffer, size_t len );
  void RefreshDataPosition( device_record_t* device );
  void HandleConfigPosition( device_record_t* device, void* client, void* buffer, size_t len );
  
  // SONAR INTERFACE
  void RefreshDataSonar( device_record_t* device );
  void HandleConfigSonar( device_record_t* device, void* client, void* buffer, size_t len );
  
  // ENERGY INTERFACE
  void RefreshDataEnergy( device_record_t* device );
  void HandleConfigEnergy( device_record_t* device, void* client, void* buffer, size_t len );
  
  // SIMULATION INTERFACE
  void RefreshDataSimulation( device_record_t* device );
  void HandleConfigSimulation( player_device_id_t id, void* client, void* buffer, size_t len);
  
  // BLOBFINDER INTERFACE
  void RefreshDataBlobfinder( device_record_t* device );
  void HandleConfigBlobfinder( device_record_t* device, void* client, void* buffer, size_t len);
  
  // LASER INTERFACE
  void HandleConfigLaser( device_record_t* device, void* client, void* buffer, size_t len );
  void RefreshDataLaser( device_record_t* device );  
  
  // FIDUCIAL INTERFACE
  void RefreshDataFiducial( device_record_t* device );
  void HandleConfigFiducial( device_record_t* device, void* client, void* buffer, size_t len );
    
  // MAP INTERFACE - TODO
  void RefreshDataMap( device_record_t* device );
  void HandleConfigMap( device_record_t* device, void* client, void* buffer, size_t len);
  void HandleConfigMapInfo( device_record_t* device, void* client, void* buffer, size_t len);
  void HandleConfigMapData( device_record_t* device, void* client, void* buffer, size_t len);
  
  /// an array of pointers to device_record_t structs
  private: GPtrArray* devices;

  protected: static GPtrArray* all_devices;

  /// all player devices share the same Stage world
  static stg_world_t* world;
};

// init static vars
stg_world_t* StgDriver::world = NULL;
GPtrArray* StgDriver::all_devices = NULL;

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
  table->AddDriver( "stage", StgDriver_Init);
}


////////////////////////////////////////////////////////////////////////////////
// Extra stuff for building a shared object.

// Need access to the global driver table
#include <player/drivertable.h>

void StgSimulation_Register(DriverTable *table);

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
	//return match; // if we found a match, we're done searching
	{
	  // if mod appears in devices already, it can not be used now
	  //printf( "[inspecting %d devices used already]", devices->len );
	  for( int i=0; i<(int)devices->len; i++ )
	    {
	      device_record_t* record =  
		(device_record_t*)g_ptr_array_index( devices, i );
	      
	      //printf( "comparing %p and %p (%d.%d.%d)\n", mod, record->mod,
	      //      record->id.port, record->id.code, record->id.index );
	      
	      if( match == record->mod )
		{
		  //printf( "[ALREADY USED]" );
		  return NULL;
		}
	    }
	  return match;
	}
    }

  return NULL;
}


////////////////////////////////////////////////////////////////////////////////
// Constructor.  Retrieve options from the configuration file and do any
// pre-Setup() setup.
StgDriver::StgDriver(ConfigFile* cf, int section)
    : Driver(cf, section)
{  
  // init the array of device ids
  this->devices = g_ptr_array_new();
  
  if( this->all_devices == NULL )
    this->all_devices = g_ptr_array_new();

  char* model_name = 
    (char*)cf->ReadString(section, "model", NULL );
  
  int device_count = cf->GetTupleCount( section, "provides" );
  
  printf( "  Stage driver creating %d %s\n", 
	  device_count, 
	  device_count == 1 ? "device" : "devices" );

  for( int d=0; d<device_count; d++ )
    {
      device_record_t* device = 
	(device_record_t*)calloc(sizeof(device_record_t),1);
      
      // tell the device which driver created it
      device->driver = this;

      if (cf->ReadDeviceId( &device->id, section, "provides", 0, d, NULL) != 0)
	{
	  this->SetError(-1);
	  return;
	}  

      printf( "    mapping device %d.%d.%d => ", 
	      device->id.port, device->id.code, device->id.index );
      fflush(stdout);
      
      // handle simulation a little differently
      // this is a little ugly - tidy it up one day
      if( device->id.code ==  PLAYER_SIMULATION_CODE )
	{
	  this->InitSimulation( cf, section );
	  return;
	}

      // otherwise...

      stg_model_type_t mod_type = (stg_model_type_t)0;
      
      switch( device->id.code )
	{	  
	case PLAYER_POSITION_CODE:
	  mod_type = STG_MODEL_POSITION; 
	  device->data_len = sizeof(player_position_data_t);
	  device->cmd_len = sizeof(player_position_cmd_t);
	  break;
	  
	case PLAYER_LASER_CODE:
	  mod_type = STG_MODEL_LASER;
	  device->data_len = sizeof(player_laser_data_t);
	  device->cmd_len = 0;
	  break;
	  
	case PLAYER_FIDUCIAL_CODE:
	  mod_type = STG_MODEL_FIDUCIAL;
	  device->data_len = sizeof(player_fiducial_data_t);
	  device->cmd_len = 0;
	  break;
	  
	case PLAYER_SONAR_CODE:
	  mod_type = STG_MODEL_RANGER;
	  device->data_len = sizeof(player_sonar_data_t);
	  device->cmd_len = 0;
	  break;

	case PLAYER_ENERGY_CODE:
	  mod_type = STG_MODEL_ENERGY;
	  device->data_len = sizeof(player_energy_data_t);
	  device->cmd_len = 0;
	  break;
	  
	case PLAYER_BLOBFINDER_CODE:
	  mod_type = STG_MODEL_BLOB;
	  device->data_len = sizeof(player_blobfinder_data_t);
	  device->cmd_len = 0;
	  break;
	  
	case PLAYER_MAP_CODE:
	  mod_type = STG_MODEL_BASIC; 
	  device->data_len = 0; // no cmds or data for maps
	  device->cmd_len = 0;
	  break;
	  
	default:
	  PRINT_ERR1( "error: stage driver doesn't support interface type %d\n",
		      device->id.code );
	  this->SetError(-1);
	  return;
	}
      
      //printf( "  Read base model name \"%s\" \n", 
      //      model_name  );
      
      if( model_name == NULL )
	PRINT_ERR1( "device \"%s\" uses the Stage driver but has "
		    "no \"model\" value defined. You must specify a "
		    "model name that matches one of the models in "
		    "the worldfile.",
		    model_name );
      
      //PLAYER_TRACE1( "attempting to resolve Stage model \"%s\"", model_name );
      //printf( "attempting to resolve Stage model \"%s\"", model_name );
      
      stg_model_t* base_model = 
	stg_world_model_name_lookup( StgDriver::world, model_name );
      
      if( base_model == NULL )
	{
	  PRINT_ERR1( " Error! find a Stage model named \"%s\"", model_name );
	  this->SetError(-1);
	  return;
	}
      
      // printf( "found base model %s\n", base_model->token );

      // now find the model for this player device
      // find the first model in the tree that is the right type and
      // has not been used before
      device->mod = model_match( base_model, mod_type, this->devices );
      
      if( device->mod )
	{
	  printf( "\"%s\"\n", device->mod->token );
		 	  
	  // now poke a data callback function into the model					    
	  device->mod->data_notify = StgDriver::RefreshDataCallback;
	  device->mod->data_notify_arg = device;

	  g_ptr_array_add( this->devices, device );
	  g_ptr_array_add( this->all_devices, device );
	}
      else
	{
	  printf( " ERROR! no model available for this device."
		  " Check your world and config files.\n" );
	  this->SetError(-1);
	  return;
	}
                 
      if (this->AddInterface( device->id, 
			      PLAYER_ALL_MODE, 
			      device->data_len, 
			      device->cmd_len,
			      10, 10) != 0)
	{
	  this->SetError(-1);    
	  return;
	}
    }
  
  //puts( "  Stage driver loaded successfully." );
}



void StgDriver::InitSimulation( ConfigFile* cf, int section ) 
{
  // boot libstage 
  stg_init( global_argc, global_argv );
  
  StgDriver::world = NULL;
  
  // load a worldfile
  char worldfile_name[MAXPATHLEN];
  const char* wfn = 
    cf->ReadString(section, "worldfile", STG_DEFAULT_WORLDFILE);
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
  printf( "\"%s\" ", worldfile_name );      
  fflush(stdout);

  
  StgDriver::world = stg_world_create_from_file( worldfile_name );
  assert(StgDriver::world);
  printf( " done.\n" );
  
  // steal the global clock - a bit aggressive, but a simple approach
  if( GlobalTime ) delete GlobalTime;
  assert( (GlobalTime = new StgTime( StgDriver::world ) ));
  
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

  // Start the device thread; spawns a new thread and executes
  // StgDriver::Main(), which contains the main loop for the driver.
  this->StartThread();

  // start the simulation
  // printf( "  Starting world clock... " ); fflush(stdout);
  //stg_world_resume( world );
  
  world->paused = FALSE;
  
  // this causes Driver::Update() to be called even when the device is
  // not subscribed
  this->alwayson = TRUE;    

  // puts( "done." );
}





////////////////////////////////////////////////////////////////////////////////
// Set up the device.  Return 0 if things go well, and -1 otherwise.
int StgDriver::Setup()
{   
  //puts("stage driver initialising");

  // Here you do whatever is necessary to setup the device, like open and
  // configure a serial port.
  
  // subscribe to data from all the devices
  for( int i=0; i<(int)this->devices->len; i++ )
    {
      device_record_t* device = (device_record_t*)g_ptr_array_index( this->devices, i );
      stg_model_subscribe( device->mod );
    }
  
  return(0);
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

  // unsubscribe to data from all the devices
  for( int i=0; i<(int)this->devices->len; i++ )
    {
      device_record_t* device = (device_record_t*)g_ptr_array_index( this->devices, i );
      stg_model_unsubscribe( device->mod );
    }

  puts("stage driver has been shutdown");

  return(0);
}


////////////////////////////////////////////////////////////////////////////////
// Main function for device thread
void StgDriver::Main() 
{
  assert( StgDriver::world );

  // The main loop; interact with the device here
  for(;;)
  {
    // test if we are supposed to cancel
    pthread_testcancel();
    
    // update the world
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
  
  Device *device = NULL;
  //int retval;
  struct timeval ts;
  
  if(timestamp)
    ts = *timestamp;
  else
    GlobalTime->GetTime(&ts);
  
  // handle simulation requests directly
  if( id.code == PLAYER_SIMULATION_CODE )
    {
      this->HandleConfigSimulation( id, client, src, len );      
    }
  else // it's a device
    {
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
      if( drec ) 
	switch( drec->id.code )
	  {
	  case PLAYER_LASER_CODE:
	    this->HandleConfigLaser( drec, client, src, len  );
	    break;
	    
	  case PLAYER_POSITION_CODE:
	    this->HandleConfigPosition( drec, client, src, len  );
	    break;
	    
	  case PLAYER_FIDUCIAL_CODE:
	    this->HandleConfigFiducial( drec, client, src, len  );
	    break;
	    
	  case PLAYER_BLOBFINDER_CODE:
	    this->HandleConfigFiducial( drec, client, src, len  );
	    break;

	  case PLAYER_SONAR_CODE:
	    this->HandleConfigSonar( drec, client, src, len  );
	    break;

	  case PLAYER_ENERGY_CODE:
	    this->HandleConfigEnergy( drec, client, src, len  );
	    break;

	  case PLAYER_MAP_CODE:
	    this->HandleConfigMap( drec, client, src, len  );
	    break;
	    
	  default:
	    printf( "Stage driver error: unknown player device code (%d)\n",
		    device->id.code );	     	      
	    
	    // we don't recognize this interface at all, but we'll send
	    // a NACK as a minimum reply
	    if (this->PutReply( device->id, client, PLAYER_MSGTYPE_RESP_NACK, NULL, 0, NULL) != 0)
	      DRIVER_ERROR("PutReply() failed");	  
	  }      
      else
	{
	  PRINT_WARN3( "Failed to find device id (%d:%d:%d)", 
		       id.port, id.code, id.index );
	  
	  if (this->PutReply( device->id, client, PLAYER_MSGTYPE_RESP_NACK, NULL, 0, NULL) != 0)
	    DRIVER_ERROR("PutReply() failed");	  
	}
    } 
  return(0);
}


void StgDriver::HandleConfigSimulation( player_device_id_t id, 
					void* client, 
					void* buffer, size_t len )
{
  printf("got simulation request\n");

  // switch on the config type (first byte)
  uint8_t* buf = (uint8_t*)buffer;
  switch( buf[0] )
    {  

// if Player has defined this config, implement it
#ifdef PLAYER_SIMULATION_SET_POSE2D
    case PLAYER_SIMULATION_SET_POSE2D:
      {
	player_simulation_pose2d_req_t* req = (player_simulation_pose2d_req_t*)buffer;
	
	stg_pose_t pose;
	pose.x = (int32_t)ntohl(req->x) / 1000.0;
	pose.y = (int32_t)ntohl(req->y) / 1000.0;
	pose.a = DTOR( ntohl(req->a) );

	printf( "Stage: received request to move object \"%s\" to (%.2f,%.2f,%.2f)\n",
		req->name, pose.x, pose.y, pose.a );
	
	// look up the named model
	
	stg_model_t* mod = 
	  stg_world_model_name_lookup( this->world, req->name );
 	
	if( mod )
	  {
	    // move it 
	    // printf( "moving model \"%s\"", req->name );	    
	    stg_model_set_pose( mod, &pose );  
	    this->PutReply( id, client, PLAYER_MSGTYPE_RESP_ACK, NULL );
	  }
	else
	  {
	    PRINT_WARN1( "simulation model \"%s\" not found", req->name );
	    this->PutReply( id, client, PLAYER_MSGTYPE_RESP_NACK, NULL );
	  }
      }      
      break;
#endif
      
    default:      
      PRINT_WARN1( "Stage simulation doesn't implement config code %d.", buf[0] );
      if (this->PutReply( id, client, PLAYER_MSGTYPE_RESP_NACK, NULL, 0, NULL) != 0)
	DRIVER_ERROR("PutReply() failed");  
      break;
    }
}

void StgDriver::HandleConfigBlobfinder( device_record_t* device, void* client, void* buffer, size_t len )
{
  printf("got blobfinder request\n");
  
  if (this->PutReply( device->id, client, PLAYER_MSGTYPE_RESP_NACK, NULL, 0, NULL) != 0)
    DRIVER_ERROR("PutReply() failed");  
}

char* testdata = "hello world";

typedef struct
{
  double ppm;
  unsigned int width, height;
  int8_t* data;
} stg_map_info_t;

// Handle map info request - adapted from Player's mapfile driver by
// Brian Gerkey
void  StgDriver::HandleConfigMapInfo( device_record_t* device,  
				      void *client, 
				      void *request, 
				      size_t len)
{
  printf( "Stage: device \"%s\" received map info request\n", device->mod->token );
  
  // prepare the map info for the client
  player_map_info_t info;  
  size_t reqlen =  sizeof(info.subtype); // Expected length of request
  
  // check if the config request is valid
  if(len != reqlen)
  {
    PLAYER_ERROR2("config request len is invalid (%d != %d)", len, reqlen);
    if (PutReply(client, PLAYER_MSGTYPE_RESP_NACK,NULL) != 0)
      PLAYER_ERROR("PutReply() failed");
    return;
  }

  // create and render a map for this model
  stg_geom_t geom;
  stg_model_get_geom( device->mod, &geom );
  
  // if we already have a map info for this model, destroy it
  stg_map_info_t* minfo = (stg_map_info_t*)stg_model_get_prop( device->mod, "_map" );
  if( minfo )
    {
      if( minfo->data ) delete [] minfo->data;
      delete minfo;
    }
  
  minfo = new stg_map_info_t();
  minfo->ppm = device->mod->world->ppm;  
  minfo->width = (unsigned int)( geom.size.x * minfo->ppm );
  minfo->height = (unsigned int)( geom.size.y * minfo->ppm );
  
  // create an occupancy grid of the correct size, initially full of
  // unknown cells
  //int8_t* map = NULL; 
  assert( (minfo->data = new int8_t[minfo->width*minfo->height] ));
  memset( minfo->data, 0, minfo->width * minfo->height );
  
  printf( "Stage: created map for model \"%s\" of %d by %d cells\n", 
	  device->mod->token, minfo->width, minfo->height );
  
  size_t count=0;
  stg_polygon_t* polys = stg_model_get_polygons( device->mod, &count);
  
  if( polys && count ) // if we have something to render
    {
      
      // we'll use a temporary matrix, as it knows how to render objects
      
      stg_matrix_t* matrix = stg_matrix_create( minfo->ppm, 1.0, 1.0 );
            
      if( count > 0 ) 
	{
	  // render the model into the matrix
	  stg_matrix_polygons( matrix, 
			       geom.size.x/2.0,
			       geom.size.y/2.0,
			       0,
			       polys, count, 
			       device->mod, 1 );  
	  
	  if( device->mod->guifeatures.boundary )    
	    stg_matrix_rectangle( matrix,
				  geom.size.x/2.0,
				  geom.size.y/2.0,
				  0,
				  geom.size.x,
				  geom.size.y,
				  device->mod, 1 );
	}
      
      // Now convert the matrix to a Player occupancy grid. if the
      // matrix contains anything in a cell, we set that map cell to
      // be occupied, else unoccupied
      
      for( unsigned int p=0; p<minfo->width; p++ )
	for( unsigned int q=0; q<minfo->height; q++ )
	  {
	    GPtrArray* pa = stg_table_cell( matrix->table, p, q );
	    minfo->data[ q * minfo->width + p ] =  pa ? 1 : -1;
	  }
      
      // we're done with the matrix
      stg_matrix_destroy( matrix );
    }

  // store the map as a model property named "_map"
  stg_model_set_prop( device->mod, "_map", (void*)minfo );

  //if(this->mapdata == NULL)
  //{
  //PLAYER_ERROR("NULL map data");
  //if(PutReply(client, PLAYER_MSGTYPE_RESP_NACK,NULL) != 0)
  //  PLAYER_ERROR("PutReply() failed");
  // return;
  
  // pixels per kilometer
  info.scale = htonl((uint32_t)rint(minfo->ppm * 1e3 ));
  
  // size in pixels
  info.width = htonl((uint32_t)minfo->width);
  info.height = htonl((uint32_t)minfo->height);
  
  // Send map info to the client
  if (PutReply(client, PLAYER_MSGTYPE_RESP_ACK, &info, sizeof(info), NULL) != 0)
    PLAYER_ERROR("PutReply() failed");

  return;
}

// Handle map data request
void StgDriver::HandleConfigMapData( device_record_t* device,  
				     void *client, 
				     void *request, 
				     size_t len)
{
  //printf( "device %s received map data request", device->mod->token );

  //int i, j;
  unsigned int oi, oj, si, sj;
  size_t reqlen;
  player_map_data_t data;
  
  // Expected length of request
  reqlen = sizeof(data) - sizeof(data.data);
  
  // check if the config request is valid
  if(len != reqlen)
    {
      PLAYER_ERROR2("config request len is invalid (%d != %d)", len, reqlen);
      if(PutReply(client, PLAYER_MSGTYPE_RESP_NACK,NULL) != 0)
      PLAYER_ERROR("PutReply() failed");
      return;
    }
  
  stg_map_info_t* minfo = NULL;
  assert( (minfo=(stg_map_info_t*)stg_model_get_prop( device->mod, "_map" )));
  
  int8_t* map = NULL;  
  assert( (map = minfo->data ) );

  //printf( "fetched map: %s\n", str );
  
  // Construct reply
  memcpy(&data, request, len);
  
   oi = ntohl(data.col);
   oj = ntohl(data.row);
   si = ntohl(data.width);
   sj = ntohl(data.height);
   
   //printf( "map data request: col: %d row: %d width: %d height: %d\n",
   //   oi, oj, si, sj );
   
   //   // Grab the pixels from the map
   for( unsigned int j = 0; j < sj; j++)
     {
       for( unsigned int i = 0; i < si; i++)
	 {
	   if((i * j) <= PLAYER_MAP_MAX_CELLS_PER_TILE)
	     {
	       if( i+oi >= 0 &&
		   i+oi < minfo->width &&
		   j+oj >= 0 &&
		   j+oj < minfo->height )
		 data.data[i + j * si] = map[ i+oi + (minfo->width * (j+oj)) ];
	       else
		 {
		   PLAYER_WARN2("requested cell (%d,%d) is offmap", i+oi, j+oj);
		   data.data[i + j * si] = 0;
		 }
	     }
	   else
	     {
	       PLAYER_WARN("requested tile is too large; truncating");
	       if(i == 0)
		 {
		   data.width = htonl(si-1);
		   data.height = htonl(j-1);
		 }
	       else
		 {
		   data.width = htonl(i);
		   data.height = htonl(j);
		 }
	     }
	 }
     }
   
  // Send map info to the client
  if(PutReply(client, PLAYER_MSGTYPE_RESP_ACK, &data, 
              sizeof(data) - sizeof(data.data) + 
              ntohl(data.width) * ntohl(data.height),NULL) != 0)
    PLAYER_ERROR("PutReply() failed");
  return;
}


void StgDriver::HandleConfigMap( device_record_t* device, void* client, void* buffer, size_t len )
{
  switch( ((unsigned char*)buffer)[0])
  {
    case PLAYER_MAP_GET_INFO_REQ:
      HandleConfigMapInfo( device, client, buffer, len);
      break;
  case PLAYER_MAP_GET_DATA_REQ:
      HandleConfigMapData( device, client, buffer, len);
      break;
    default:
      PLAYER_ERROR("got unknown map config request; ignoring");
      if(PutReply(client, PLAYER_MSGTYPE_RESP_NACK,NULL) != 0)
        PLAYER_ERROR("PutReply() failed");
      break;
  }
}





void StgDriver::HandleConfigPosition( device_record_t* device, void* client, void* buffer, size_t len  )
{
  printf("got position request\n");
  
 // switch on the config type (first byte)
  uint8_t* buf = (uint8_t*)buffer;
  switch( buf[0] )
    {  
    case PLAYER_POSITION_GET_GEOM_REQ:
      {
	stg_geom_t geom;
	stg_model_get_geom( device->mod,&geom );

	// fill in the geometry data formatted player-like
	player_position_geom_t pgeom;
	pgeom.pose[0] = ntohs((uint16_t)(1000.0 * geom.pose.x));
	pgeom.pose[1] = ntohs((uint16_t)(1000.0 * geom.pose.y));
	pgeom.pose[2] = ntohs((uint16_t)RTOD(geom.pose.a));
	
	pgeom.size[0] = ntohs((uint16_t)(1000.0 * geom.size.x)); 
	pgeom.size[1] = ntohs((uint16_t)(1000.0 * geom.size.y)); 
	
	this->PutReply( device->id, client, 
		  PLAYER_MSGTYPE_RESP_ACK, 
		  &pgeom, sizeof(pgeom), NULL );
      }
      break;
      
    case PLAYER_POSITION_RESET_ODOM_REQ:
      {
	PRINT_DEBUG( "resetting odometry" );
	
	stg_pose_t origin;
	memset(&origin,0,sizeof(origin));
	stg_model_position_set_odom( device->mod, &origin );
	
	this->PutReply( device->id, client, PLAYER_MSGTYPE_RESP_ACK, NULL );
      }
      break;

    case PLAYER_POSITION_SET_ODOM_REQ:
      {
	player_position_set_odom_req_t* req = 
	  (player_position_set_odom_req_t*)buffer;
	
	PRINT_DEBUG3( "setting odometry to (%.d,%d,%.2f)",
		      (int32_t)ntohl(req->x),
		      (int32_t)ntohl(req->y),
		      DTOR((int32_t)ntohl(req->theta)) );
	
	stg_pose_t pose;
	pose.x = ((double)(int32_t)ntohl(req->x)) / 1000.0;
	pose.y = ((double)(int32_t)ntohl(req->y)) / 1000.0;
	pose.a = DTOR( (double)(int32_t)ntohl(req->theta) );

	stg_model_position_set_odom( device->mod, &pose );
	
	PRINT_DEBUG3( "set odometry to (%.2f,%.2f,%.2f)",
		      pose.x,
		      pose.y,
		      pose.a );

	PRINT_DEBUG3( "set odometry to (%.2f,%.2f,%.2f)",
		      device->mod->odom.x,
		      device->mod->odom.y,
		      device->mod->odom.a );
	
	this->PutReply( device->id, client, PLAYER_MSGTYPE_RESP_ACK, NULL );
      }
      break;

    case PLAYER_POSITION_MOTOR_POWER_REQ:
      {
	player_position_power_config_t* req = 
	  (player_position_power_config_t*)buffer;

	int motors_on = req->value;

	PRINT_WARN1( "Stage ignores motor power state (%d)",
		     motors_on );
	this->PutReply( device->id, client, PLAYER_MSGTYPE_RESP_ACK, NULL );
      }
      break;

    default:
      {
	PRINT_WARN1( "stg_position doesn't support config id %d", buf[0] );
        if( this->PutReply( device->id, client, PLAYER_MSGTYPE_RESP_NACK, NULL) != 0)
          DRIVER_ERROR("PutReply() failed");
        break;
      }
    }
}

void StgDriver::HandleConfigFiducial( device_record_t* device, void* client, void* src, size_t len  )
{
  printf("got fiducial request\n");
   // switch on the config type (first byte)
  uint8_t* buf = (uint8_t*)src;
  switch( buf[0] )
    {  
    case PLAYER_FIDUCIAL_GET_GEOM:
      {	
	// just get the model's geom - Stage doesn't have separate
	// fiducial geom (yet)
	stg_geom_t geom;
	stg_model_get_geom(device->mod,&geom);
	
	// fill in the geometry data formatted player-like
	player_fiducial_geom_t pgeom;
	pgeom.pose[0] = htons((uint16_t)(1000.0 * geom.pose.x));
	pgeom.pose[1] = htons((uint16_t)(1000.0 * geom.pose.y));
	pgeom.pose[2] = htons((uint16_t)RTOD( geom.pose.a));
	
	pgeom.size[0] = htons((uint16_t)(1000.0 * geom.size.x)); 
	pgeom.size[1] = htons((uint16_t)(1000.0 * geom.size.y)); 
	
	pgeom.fiducial_size[0] = ntohs((uint16_t)100); // TODO - get this info
	pgeom.fiducial_size[1] = ntohs((uint16_t)100);
	
	if( PutReply(  device->id, client, PLAYER_MSGTYPE_RESP_ACK,  
		      &pgeom, sizeof(pgeom), NULL ) != 0 )
	  DRIVER_ERROR("PutReply() failed for PLAYER_LASER_GET_GEOM");      
      }
      break;
      
    case PLAYER_FIDUCIAL_SET_FOV:
      
      if( len == sizeof(player_fiducial_fov_t) )
	{
	  player_fiducial_fov_t* pfov = (player_fiducial_fov_t*)src;
	  
	  // convert from player to stage FOV packets
	  stg_fiducial_config_t setcfg;
	  memset( &setcfg, 0, sizeof(setcfg) );
	  setcfg.min_range = (uint16_t)ntohs(pfov->min_range) / 1000.0;
	  setcfg.max_range_id = (uint16_t)ntohs(pfov->max_range) / 1000.0;
	  setcfg.max_range_anon = setcfg.max_range_id;
	  setcfg.fov = DTOR((uint16_t)ntohs(pfov->view_angle));
	  
	  //printf( "setting fiducial FOV to min %f max %f fov %f\n",
	  //  setcfg.min_range, setcfg.max_range_anon, setcfg.fov );
	  
	  stg_model_set_config( device->mod, &setcfg, sizeof(setcfg));
	}    
      else
	PRINT_ERR2("Incorrect packet size setting fiducial FOV (%d/%d)",
		      (int)len, (int)sizeof(player_fiducial_fov_t) );      
      
      // deliberate no-break - SET_FOV needs the current FOV as a reply
      
    case PLAYER_FIDUCIAL_GET_FOV:
      {
	stg_fiducial_config_t cfg;
	assert( stg_model_get_config( device->mod, &cfg, sizeof(stg_fiducial_config_t))
		== sizeof(stg_fiducial_config_t) );
	
	// fill in the geometry data formatted player-like
	player_fiducial_fov_t pfov;
	pfov.min_range = htons((uint16_t)(1000.0 * cfg.min_range));
	pfov.max_range = htons((uint16_t)(1000.0 * cfg.max_range_anon));
	pfov.view_angle = htons((uint16_t)RTOD(cfg.fov));
	
	if( PutReply(  device->id, client, PLAYER_MSGTYPE_RESP_ACK, 
		      &pfov, sizeof(pfov), NULL ) != 0 )
	  DRIVER_ERROR("PutReply() failed for "
		       "PLAYER_FIDUCIAL_GET_FOV or PLAYER_FIDUCIAL_SET_FOV");      
      }
      break;
      
    case PLAYER_FIDUCIAL_SET_ID:
      
      if( len == sizeof(player_fiducial_id_t) )
	{
	  int id = ntohl(((player_fiducial_id_t*)src)->id);

	  stg_model_set_fiducialreturn( device->mod, &id );
	}
      else
	PRINT_ERR2("Incorrect packet size setting fiducial ID (%d/%d)",
		     (int)len, (int)sizeof(player_fiducial_id_t) );      
      
      // deliberate no-break - SET_ID needs the current ID as a reply

  case PLAYER_FIDUCIAL_GET_ID:
      {
	stg_fiducial_return_t ret;
	stg_model_get_fiducialreturn(device->mod,&ret); 

	// fill in the data formatted player-like
	player_fiducial_id_t pid;
	pid.id = htonl((int)ret);
	
	if( PutReply(  device->id, client, PLAYER_MSGTYPE_RESP_ACK, 
		      &pid, sizeof(pid), NULL ) != 0 )
	  DRIVER_ERROR("PutReply() failed for "
		       "PLAYER_FIDUCIAL_GET_ID or PLAYER_FIDUCIAL_SET_ID");      
      }
      break;


    default:
      {
	PRINT_WARN1( "Warning: stg_fiducial doesn't support config id %d\n", buf[0] );
        if (PutReply( device->id, client, PLAYER_MSGTYPE_RESP_NACK, NULL) != 0) 
          DRIVER_ERROR("PutReply() failed");
      }
    }  
}


void StgDriver::HandleConfigLaser( device_record_t* device, void* client, void* buffer, size_t len )
{
  printf("got laser request\n");
  
  // switch on the config type (first byte)
  uint8_t* buf = (uint8_t*)buffer;
  switch( buf[0] )
    {  
     case PLAYER_LASER_SET_CONFIG:
      {
	player_laser_config_t* plc = (player_laser_config_t*)buffer;
	
        if( len == sizeof(player_laser_config_t) )
	  {
	    int min_a = (int16_t)ntohs(plc->min_angle);
	    int max_a = (int16_t)ntohs(plc->max_angle);
	    int ang_res = (int16_t)ntohs(plc->resolution);
	    // TODO
	    // int intensity = plc->intensity;
	    
	    //PRINT_DEBUG4( "requested laser config:\n %d %d %d %d",
	    //	  min_a, max_a, ang_res, intensity );
	    
	    min_a /= 100;
	    max_a /= 100;
	    ang_res /= 100;
	    
	    stg_laser_config_t current;
	    assert( stg_model_get_config( device->mod, &current, sizeof(current))
		     == sizeof(current));
	    
	    stg_laser_config_t slc;
	    // copy the existing config
	    memcpy( &slc, &current, sizeof(slc));
	    
	    // tweak the parts that player knows about
	    slc.fov = DTOR(max_a - min_a);
	    slc.samples = (int)(slc.fov / DTOR(ang_res));
	    
	    PRINT_DEBUG2( "setting laser config: fov %.2f samples %d", 
			  slc.fov, slc.samples );
	    
	    stg_model_set_config( device->mod, &slc, sizeof(slc));
	    
	    if(this->PutReply( device->id, client, PLAYER_MSGTYPE_RESP_ACK, plc, len, NULL) != 0)
	      DRIVER_ERROR("PutReply() failed for PLAYER_LASER_SET_CONFIG");
	  }
	else
	  {
	    PRINT_ERR2("config request len is invalid (%d != %d)", 
	           (int)len, (int)sizeof(player_laser_config_t));

	    if(this->PutReply( device->id, client, PLAYER_MSGTYPE_RESP_NACK, NULL) != 0)
	      DRIVER_ERROR("PutReply() failed for PLAYER_LASER_SET_CONFIG");
	  }
      }
      break;
      
    case PLAYER_LASER_GET_CONFIG:
      {   
	if( len == 1 )
	  {
	    stg_laser_config_t slc;
	    assert( stg_model_get_config( device->mod, &slc, sizeof(slc)) 
		    == sizeof(slc) );

	    // integer angles in degrees/100
	    int16_t  min_angle = (int16_t)(-RTOD(slc.fov/2.0) * 100);
	    int16_t  max_angle = (int16_t)(+RTOD(slc.fov/2.0) * 100);
	    //uint16_t angular_resolution = RTOD(slc.fov / slc.samples) * 100;
	    uint16_t angular_resolution =(uint16_t)(RTOD(slc.fov / (slc.samples-1)) * 100);
	    uint16_t range_multiplier = 1; // TODO - support multipliers
	    
	    int intensity = 1; // todo
	    
	    //printf( "laser config:\n %d %d %d %d\n",
	    //	    min_angle, max_angle, angular_resolution, intensity );
	    
	    player_laser_config_t plc;
	    memset(&plc,0,sizeof(plc));
	    plc.min_angle = htons(min_angle); 
	    plc.max_angle = htons(max_angle); 
	    plc.resolution = htons(angular_resolution);
	    plc.range_res = htons(range_multiplier);
	    plc.intensity = intensity;

	    if(this->PutReply( device->id, client, PLAYER_MSGTYPE_RESP_ACK, &plc, 
			sizeof(plc), NULL) != 0)
	      DRIVER_ERROR("PutReply() failed for PLAYER_LASER_GET_CONFIG");      
	  }
	else
	  {
	    PRINT_ERR2("config request len is invalid (%d != %d)", (int)len, 1);
	    if(this->PutReply( device->id, client, PLAYER_MSGTYPE_RESP_NACK, NULL) != 0)
	      DRIVER_ERROR("PutReply() failed for PLAYER_LASER_GET_CONFIG");
	  }
      }
      break;

    case PLAYER_LASER_GET_GEOM:
      {	
	stg_geom_t geom;
	stg_model_get_geom( device->mod, &geom );
	
	PRINT_DEBUG5( "received laser geom: %.2f %.2f %.2f -  %.2f %.2f",
		      geom.pose.x, 
		      geom.pose.y, 
		      geom.pose.a, 
		      geom.size.x, 
		      geom.size.y ); 
	
	// fill in the geometry data formatted player-like
	player_laser_geom_t pgeom;
	pgeom.pose[0] = htons((uint16_t)(1000.0 * geom.pose.x));
	pgeom.pose[1] = htons((uint16_t)(1000.0 * geom.pose.y));
	pgeom.pose[2] = htons((uint16_t)RTOD( geom.pose.a));
	
	pgeom.size[0] = htons((uint16_t)(1000.0 * geom.size.x)); 
	pgeom.size[1] = htons((uint16_t)(1000.0 * geom.size.y)); 
	
	if( this->PutReply( device->id, client, PLAYER_MSGTYPE_RESP_ACK, 
		     &pgeom, sizeof(pgeom), NULL ) != 0 )
	  DRIVER_ERROR("PutReply() failed for PLAYER_LASER_GET_GEOM");      
      }
      break;
      
    default:
      {
	PRINT_WARN1( "stg_laser doesn't support config id %d", buf[0] );
        if (PutReply( device->id, client, PLAYER_MSGTYPE_RESP_NACK, NULL, 0, NULL) != 0)
	DRIVER_ERROR("PutReply() failed");
        break;
      }
      
    }
}

void StgDriver::HandleConfigSonar( device_record_t* device, void* client, void* src, size_t len )
{
  printf("got sonar request\n");
  
 // switch on the config type (first byte)
  uint8_t* buf = (uint8_t*)src;
  switch( buf[0] )
    {  
    case PLAYER_SONAR_GET_GEOM_REQ:
      { 
	stg_ranger_config_t cfgs[100];
	size_t cfglen = 0;
	cfglen = stg_model_get_config( device->mod, cfgs, sizeof(cfgs[0])*100);
	
	//assert( cfglen == sizeof(cfgs[0])*100 );

	size_t rcount = cfglen / sizeof(stg_ranger_config_t);
	
	// convert the ranger data into Player-format sonar poses	
	player_sonar_geom_t pgeom;
	memset( &pgeom, 0, sizeof(pgeom) );
	
	pgeom.subtype = PLAYER_SONAR_GET_GEOM_REQ;
	
	// limit the number of samples to Player's maximum
	if( rcount > PLAYER_SONAR_MAX_SAMPLES )
	  rcount = PLAYER_SONAR_MAX_SAMPLES;

	pgeom.pose_count = htons((uint16_t)rcount);

	for( int i=0; i<(int)rcount; i++ )
	  {
	    // fill in the geometry data formatted player-like
	    pgeom.poses[i][0] = 
	      ntohs((uint16_t)(1000.0 * cfgs[i].pose.x));
	    
	    pgeom.poses[i][1] = 
	      ntohs((uint16_t)(1000.0 * cfgs[i].pose.y));
	    
	    pgeom.poses[i][2] 
	      = ntohs((uint16_t)RTOD( cfgs[i].pose.a));	    
	  }

	  if(PutReply( device->id, client, PLAYER_MSGTYPE_RESP_ACK, 
	  &pgeom, sizeof(pgeom), NULL ) )
	  DRIVER_ERROR("failed to PutReply");
      }
      break;
      
//     case PLAYER_SONAR_POWER_REQ:
//       /*
// 	 * 1 = enable sonars
// 	 * 0 = disable sonar
// 	 */
// 	if( len != sizeof(player_sonar_power_config_t))
// 	  {
// 	    PRINT_WARN2( "stg_sonar: arg to sonar state change "
// 			  "request wrong size (%d/%d bytes); ignoring",
// 			  (int)len,(int)sizeof(player_sonar_power_config_t) );
	    
// 	    if(PutReply( device->id, client, PLAYER_MSGTYPE_RESP_NACK, NULL ))
// 	      DRIVER_ERROR("failed to PutReply");
// 	  }
	
// 	this->power_on = ((player_sonar_power_config_t*)src)->value;
	
// 	if(PutReply( device->id, client, PLAYER_MSGTYPE_RESP_ACK, NULL ))
// 	  DRIVER_ERROR("failed to PutReply");
	
// 	break;
	
    default:
      {
	PRINT_WARN1( "stage sonar model doesn't support config id %d\n", buf[0] );
	if (PutReply( device->id, client, PLAYER_MSGTYPE_RESP_NACK, NULL) != 0)
	  DRIVER_ERROR("PutReply() failed");
	break;
      }
      
    }
}

void StgDriver::HandleConfigEnergy( device_record_t* device, 
				    void* client, 
				    void* src, 
				    size_t len )
{
  printf("got energy request\n");
  
 // switch on the config type (first byte)
  uint8_t* buf = (uint8_t*)src;
  switch( buf[0] )
    {  
    default:
      {
	PRINT_WARN1( "stage energy model doesn't support config id %d\n", buf[0] );
	if (PutReply( device->id, client, PLAYER_MSGTYPE_RESP_NACK, NULL) != 0)
	  DRIVER_ERROR("PutReply() failed");
	break;
      }      
    }
}

void StgDriver::CheckCommands()
{  
  //static int g=0;
  //printf( "check cmd %d\n", g++ );

  static char* cmd = NULL;
  
  for( int i=0; i<(int)this->devices->len; i++ )
    {
      device_record_t* device = (device_record_t*)g_ptr_array_index( this->devices, i );
      
      if( device->cmd_len > 0 )
	{    
	  cmd = (char*)realloc( cmd, device->cmd_len );
	  
	  // copy the command from Player
	  size_t cmd_len = 
	    this->GetCommand( device->id, cmd, device->cmd_len, NULL);
       
	  switch( device->id.code )
	    {
	    case PLAYER_POSITION_CODE:
	      if( cmd_len > 0 ) this->HandleCommandPosition( device, cmd, cmd_len );
	      break;

	    default:
	      PRINT_WARN1( "Stage received a command for a device of type %d. Ignoring.",
			    device->id.code );
	    }
	}
    }
  
  // buf is static, so no need to free it here
  return;
}

void StgDriver::HandleCommandPosition( device_record_t* device, void* src, size_t len )
{
  if( len == sizeof(player_position_cmd_t) )
    {
      // convert from Player to Stage format
      player_position_cmd_t* pcmd = (player_position_cmd_t*)src;
      
      // only velocity control mode works yet
      stg_position_cmd_t cmd; 
      cmd.x = ((double)((int32_t)ntohl(pcmd->xspeed))) / 1000.0;
      cmd.y = ((double)((int32_t)ntohl(pcmd->yspeed))) / 1000.0;
      cmd.a = DTOR((double)((int32_t)ntohl(pcmd->yawspeed)));
      cmd.mode = STG_POSITION_CONTROL_VELOCITY;
      
      // we only set the command if it's different from the old one
      // (saves aquiring a mutex lock from the sim thread)
      
      char buf[32767];
      stg_model_get_command( device->mod, buf, 32767 );
      
      if( memcmp( &cmd, buf, sizeof(cmd) ))
	{	  
	  //static int g=0;
	  //printf( "setting command %d\n", g++ );
	  stg_model_set_command( device->mod, &cmd, sizeof(cmd) ) ;
	}
    }
  else
    PRINT_ERR2( "wrong size position command packet (%d/%d bytes)",
		(int)len, (int)sizeof(player_position_cmd_t) );
}


void StgDriver::Update(void)
{
  // Check for and handle configuration requests
  //this->CheckConfig();
  
  // Check for commands
  this->CheckCommands();
}

// a static method used as a callback when a device generates new data
void StgDriver::RefreshDataCallback( void* user )
{
  device_record_t* device = (device_record_t*)user;
  
  device->driver->RefreshDataDevice( device );
}

void StgDriver::RefreshData()
{   
  for( int i=0; i<(int)this->devices->len; i++ )
    this->RefreshDataDevice( (device_record_t*)g_ptr_array_index( this->devices, i ));
}

void StgDriver::RefreshDataDevice( device_record_t* device )
{ 
  assert( device );
  
  switch( device->id.code )
    {
    case PLAYER_SIMULATION_CODE:
      this->RefreshDataSimulation( device );
      break;
      
    case PLAYER_LASER_CODE:
      this->RefreshDataLaser( device );
      break;
      
    case PLAYER_POSITION_CODE:
      this->RefreshDataPosition( device );
      break;
      
    case PLAYER_FIDUCIAL_CODE:
      this->RefreshDataFiducial( device );
      break;
      
    case PLAYER_BLOBFINDER_CODE:
      this->RefreshDataBlobfinder( device );
      break;
      
    case PLAYER_SONAR_CODE:
      this->RefreshDataSonar( device );
      break;

    case PLAYER_ENERGY_CODE:
      this->RefreshDataEnergy( device );
      break;

    case PLAYER_MAP_CODE:
      this->RefreshDataMap( device );
      break;
      
    default:
      printf( "Stage driver error: unknown player device code (%d)\n",
	      device->id.code );	  	  
    }
}


void StgDriver::RefreshDataBlobfinder( device_record_t* device )
{
  stg_blobfinder_blob_t blobs[100];
  size_t datalen = stg_model_get_data( device->mod, blobs, 100*sizeof(blobs[0]) );
  
  if( datalen == 0 )
    {
      this->PutData( device->id, NULL, 0, NULL);
    }
  else
    {
      size_t bcount = datalen / sizeof(stg_blobfinder_blob_t);
      

      // limit the number of samples to Player's maximum
      if( bcount > PLAYER_BLOBFINDER_MAX_BLOBS )
	bcount = PLAYER_BLOBFINDER_MAX_BLOBS;      
     
      player_blobfinder_data_t bfd;
      memset( &bfd, 0, sizeof(bfd) );
      
      // get the configuration
      stg_blobfinder_config_t cfg;      
      assert( stg_model_get_config( device->mod, &cfg, sizeof(cfg) ) == sizeof(cfg) );
      
      // and set the image width * height
      bfd.width = htons((uint16_t)cfg.scan_width);
      bfd.height = htons((uint16_t)cfg.scan_height);
      bfd.blob_count = htons((uint16_t)bcount);

      // now run through the blobs, packing them into the player buffer
      // counting the number of blobs in each channel and making entries
      // in the acts header 
      unsigned int b;
      for( b=0; b<bcount; b++ )
	{
	  // I'm not sure the ACTS-area is really just the area of the
	  // bounding box, or if it is in fact the pixel count of the
	  // actual blob. Here it's just the rectangular area.
	  
	  // useful debug - leave in
	  /*
	    cout << "blob "
	    << " channel: " <<  (int)blobs[b].channel
	    << " area: " <<  blobs[b].area
	    << " left: " <<  blobs[b].left
	    << " right: " <<  blobs[b].right
	    << " top: " <<  blobs[b].top
	    << " bottom: " <<  blobs[b].bottom
	    << endl;
	  */
	  
	  bfd.blobs[b].x      = htons((uint16_t)blobs[b].xpos);
	  bfd.blobs[b].y      = htons((uint16_t)blobs[b].ypos);
	  bfd.blobs[b].left   = htons((uint16_t)blobs[b].left);
	  bfd.blobs[b].right  = htons((uint16_t)blobs[b].right);
	  bfd.blobs[b].top    = htons((uint16_t)blobs[b].top);
	  bfd.blobs[b].bottom = htons((uint16_t)blobs[b].bottom);
	  
	  bfd.blobs[b].color = htonl(blobs[b].color);
	  bfd.blobs[b].area  = htonl(blobs[b].area);
	  

	}

      //if( bf->ready )
	{
	  size_t size = sizeof(bfd) - sizeof(bfd.blobs) + bcount * sizeof(bfd.blobs[0]);
	  
	  //PRINT_WARN1( "blobfinder putting %d bytes of data", size );
	  this->PutData( device->id, &bfd, size, NULL); // time gets filled in
	}
    }
}

void StgDriver::RefreshDataSonar( device_record_t* device )
{
  stg_ranger_sample_t rangers[100];
  size_t datalen = stg_model_get_data( device->mod, rangers, 100*sizeof(rangers[0]) );
  
  if( datalen > 0 )
    {
      size_t rcount = datalen / sizeof(stg_ranger_sample_t);
      
      // limit the number of samples to Player's maximum
      if( rcount > PLAYER_SONAR_MAX_SAMPLES )
	rcount = PLAYER_SONAR_MAX_SAMPLES;
      
      player_sonar_data_t sonar;
      memset( &sonar, 0, sizeof(sonar) );
      
      //if( son->power_on ) // set with a sonar config
	{
	  sonar.range_count = htons((uint16_t)rcount);
	  
	  for( int i=0; i<(int)rcount; i++ )
	    sonar.ranges[i] = htons((uint16_t)(1000.0*rangers[i].range));
	}
      
      this->PutData( device->id, &sonar, sizeof(sonar), NULL); // time gets filled in
    }
}


void StgDriver::RefreshDataMap( device_record_t* device )
{
  PRINT_WARN( "someone subscribed to the map interface, but we don't generate any data" );
  this->PutData( device->id, NULL, 0, NULL); 
}

void StgDriver::RefreshDataEnergy( device_record_t* device )
{
  stg_energy_data_t data;
  size_t datalen = stg_model_get_data( device->mod, &data, sizeof(data) );

  if( datalen == sizeof(data) )
    {
      // translate stage data to player data packet
      player_energy_data_t pen;
      pen.mjoules = htonl( (int32_t)(data.stored * 1000.0));
      pen.mwatts  = htonl( (int32_t)(data.output_watts * 1000.0));
      pen.charging = (uint8_t)( data.charging );      
      this->PutData( device->id, &pen, sizeof(pen), NULL); 
    }
  else
    PRINT_ERR2( "energy device returned wrong size data (%d/%d)", 
		datalen, sizeof(data));
}


void StgDriver::RefreshDataFiducial( device_record_t* device )
{
  player_fiducial_data_t pdata;
  memset( &pdata, 0, sizeof(pdata) );
  
  stg_fiducial_t fids[100];
  size_t datalen  = stg_model_get_data(device->mod, fids, 100*sizeof(fids[0]) );
				       
  if( datalen > 0 )
    {
      size_t fcount = datalen / sizeof(stg_fiducial_t);      
      assert( fcount > 0 );
      
      pdata.count = htons((uint16_t)fcount);
      
      for( int i=0; i<(int)fcount; i++ )
	{
	  pdata.fiducials[i].id = htons((int16_t)fids[i].id);
	  
	  // 2D x,y only
	  
	  double xpos = fids[i].range * cos(fids[i].bearing);
	  double ypos = fids[i].range * sin(fids[i].bearing);
	  
	  pdata.fiducials[i].pos[0] = htonl((int32_t)(xpos*1000.0));
	  pdata.fiducials[i].pos[1] = htonl((int32_t)(ypos*1000.0));
	  
	  // yaw only
	  pdata.fiducials[i].rot[2] = htonl((int32_t)(fids[i].geom.a*1000.0));	      
	  
	  // player can't handle per-fiducial size.
	  // we leave uncertainty (upose) at zero
	}
    }
  
  // publish this data
  this->PutData( device->id, &pdata, sizeof(pdata), NULL);
}


void StgDriver::RefreshDataLaser( device_record_t* device )
{
  //puts( "publishing laser data" );
  
  stg_laser_sample_t samples[1000];
  size_t dlen = stg_model_get_data( device->mod, samples, 1000*sizeof(samples[0]) );
  
  int sample_count = dlen / sizeof( stg_laser_sample_t );

  //for( int i=0; i<sample_count; i++ )
  //  printf( "rrrange %d %d\n", i, samples[i].range);

  player_laser_data_t pdata;
  memset( &pdata, 0, sizeof(pdata) );
  
  stg_laser_config_t cfg;
  assert( stg_model_get_config( device->mod, &cfg, sizeof(cfg)) == sizeof(cfg));
  
  if( sample_count != cfg.samples )
    {
      //PRINT_ERR2( "bad laser data: got %d/%d samples",
      //	  sample_count, cfg.samples );
    }
  else
    {      
      double min_angle = -RTOD(cfg.fov/2.0);
      double max_angle = +RTOD(cfg.fov/2.0);
      double angular_resolution = RTOD(cfg.fov / cfg.samples);
      double range_multiplier = 1; // TODO - support multipliers
      
      pdata.min_angle = htons( (int16_t)(min_angle * 100 )); // degrees / 100
      pdata.max_angle = htons( (int16_t)(max_angle * 100 )); // degrees / 100
      pdata.resolution = htons( (uint16_t)(angular_resolution * 100)); // degrees / 100
      pdata.range_res = htons( (uint16_t)range_multiplier);
      pdata.range_count = htons( (uint16_t)cfg.samples );
      
      for( int i=0; i<cfg.samples; i++ )
	{
	  //printf( "range %d %d\n", i, samples[i].range);
	  
	  pdata.ranges[i] = htons( (uint16_t)(samples[i].range) );
	  pdata.intensity[i] = (uint8_t)samples[i].reflectance;
	}
      
      // Write laser data
      this->PutData( device->id, &pdata, sizeof(pdata), NULL);
    }
}

void StgDriver::RefreshDataPosition( device_record_t* device )
{
  //puts( "publishing position data" ); 
  
  stg_position_data_t data;
  assert( stg_model_get_data( device->mod, &data, sizeof(data) ) == sizeof(data));
  
  //  if( datalen == sizeof(stg_position_data_t) )      
    {      
      
      //PLAYER_MSG3( "get data data %.2f %.2f %.2f", data.x, data.y, data.a );
      player_position_data_t position_data;
      memset( &position_data, 0, sizeof(position_data) );
      // pack the data into player format
      position_data.xpos = ntohl((int32_t)(1000.0 * data.pose.x));
      position_data.ypos = ntohl((int32_t)(1000.0 * data.pose.y));
      position_data.yaw = ntohl((int32_t)(RTOD(data.pose.a)));
      
      // speeds
      position_data.xspeed= ntohl((int32_t)(1000.0 * data.velocity.x)); // mm/sec
      position_data.yspeed = ntohl((int32_t)(1000.0 * data.velocity.y));// mm/sec
      position_data.yawspeed = ntohl((int32_t)(RTOD(data.velocity.a))); // deg/sec
      
      position_data.stall = data.stall;// ? 1 : 0;
      
      //printf( "getdata called at %lu ms\n", stage_client->stagetime );
      
      // publish this data
      //pos->PutData( &position_data, sizeof(position_data), NULL); 

      // Write position data
      this->PutData( device->id, &position_data, sizeof(position_data), NULL);      
    }
}
