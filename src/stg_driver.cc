/*
 *  Player - One Hell of a Robot Server
 *  Copyright (C) 2003  
 *     Brian Gerkey, Andrew Howard
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
 * Desc: A simple example of how to write a driver that supports multiple interface.
 * Also demonstrates use of a driver as a loadable object.
 * Author: Andrew Howard
 * Date: 25 July 2004
 * CVS: $Id: stg_driver.cc,v 1.14 2004-12-10 10:15:12 rtv Exp $
 */

/** @defgroup stg_driver stg_driver Player driver

This driver gives Player access to Stage's models.

@par Provides

The stg_position driver provides the following device interfaces:

- player_interface_position
  - This interface returns odometry data, and accepts velocity commands.

@par Supported configuration requests

  - PLAYER_POSITION_SET_ODOM_REQ
  - PLAYER_POSITION_RESET_ODOM_REQ
  - PLAYER_POSITION_GET_GEOM_REQ
  - PLAYER_POSITION_MOTOR_POWER_REQ
  - PLAYER_POSITION_VELOCITY_MODE_REQ

@par Player configuration file options

- model (string)
  - where (string) is the name of a Stage position model that will be controlled by this interface.
  
@par Configuration file examples:

Creating a @model_position in a Stage world (.world) file:

@verbatim
position
(
  name "marvin"
  pose [ 1 1 0 ]
  color "red"
)
@endverbatim

Using the model in a Player config (.cfg) file:

@verbatim
driver
(
  name "stg_position"
  provides ["position:0" ]
  model "marvin"
)
@endverbatim

@par Authors

Richard Vaughan
*/


// ONLY if you need something that was #define'd as a result of configure 
// (e.g., HAVE_CFMAKERAW), then #include <config.h>, like so:
/*
#if HAVE_CONFIG_H
  #include <config.h>
#endif
*/

#include <unistd.h>
#include <string.h>
#include <netinet/in.h>

#include "player.h"
#include "player/device.h"
#include "player/driver.h"
#include "player/configfile.h"
#include "player/drivertable.h"
#include <player/drivertable.h>
#include <player/driver.h>
#include "playercommon.h"

#include "stage_internal.h"
#include "stg_time.h"

extern PlayerTime* GlobalTime;
extern int global_argc;
extern char** global_argv;

#define STG_DEFAULT_WORLDFILE "default.world"
#define DRIVER_ERROR(X) printf( "Stage driver error: %s\n", X )

typedef struct
{
  player_device_id_t id;
  stg_model_t* mod;
} device_record_t;

// init static vars
//ConfigFile* StgDriver::config = NULL;



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
  
  // Main function for device thread.
  private: virtual void Main();
  
  private: void CheckConfig();
  private: void CheckCommands();
  private: void RefreshData();
  
  // one method for each type of data we export. 
  private: void RefreshDataPosition( device_record_t* device );
  private: void RefreshDataLaser( device_record_t* device );
  
  private: void InitSimulation( ConfigFile* cf, int section );

  // an array of device_record_t structs
  //private: GArray* devices;
  static GArray* devices;

  /// all player devices share the same Stage world
  static stg_world_t* world;
};

// init static vars
stg_world_t* StgDriver::world = NULL;
GArray* StgDriver::devices = NULL;

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
    puts(" Stage driver plugin init");
    StgDriver_Register(table);
    return(0);
  }
}


stg_model_t* model_match( stg_model_t* mod, stg_model_type_t tp, GArray* devices )
{
  //printf( "[not used]" );

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
		&g_array_index( devices, device_record_t, i );
	      
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
  if( this->devices == NULL )
    this->devices = g_array_new( FALSE, TRUE, sizeof(device_record_t) );
  
  //device_record_t* dev = calloc(sizeof(device_record_t,1));
  
  
  char* model_name = 
    (char*)cf->ReadString(section, "model", NULL );
  
  int device_count = cf->GetTupleCount( section, "provides" );
  
  printf( "  Stage loading %d devices\n", device_count );

  for( int d=0; d<device_count; d++ )
    {
      device_record_t device;
      memset(&device, 0, sizeof(device));
      
      if (cf->ReadDeviceId( &device.id, section, "provides", 0, d, NULL) != 0)
	{
	  this->SetError(-1);
	  return;
	}  
      
      printf( "   mapping device %d.%d.%d => ", 
	      device.id.port, device.id.code, device.id.index );
      fflush(stdout);

      
      size_t data_size=0, cmd_size=0;
      char* type_str = NULL;
      stg_model_type_t mod_type = (stg_model_type_t)0;

      switch( device.id.code )
	{
	  // handle simulation a little differently
	  // this is a little ugly - tidy it up one day
	case PLAYER_SIMULATION_CODE:
	  this->InitSimulation( cf, section );
	  return;

	case PLAYER_POSITION_CODE:
	  data_size = sizeof(player_position_data_t);
	  cmd_size = sizeof(player_position_cmd_t);
	  type_str = "position";
	  mod_type = STG_MODEL_POSITION;
	  break;

	case PLAYER_LASER_CODE:
	  data_size = sizeof(player_laser_data_t);
	  cmd_size = 0;
	  type_str = "laser";
	  mod_type = STG_MODEL_LASER;
	  break;

	case PLAYER_FIDUCIAL_CODE:
	  data_size = sizeof(player_fiducial_data_t);
	  cmd_size =  0;
	  type_str = "fiducial";
	  mod_type = STG_MODEL_FIDUCIAL;
	  break;

	case PLAYER_SONAR_CODE:
	  data_size = sizeof(player_sonar_data_t);
	  cmd_size = 0;
	  type_str = "ranger";
	  mod_type = STG_MODEL_RANGER;
	  break;
	  
	case PLAYER_BLOBFINDER_CODE:
	  data_size = sizeof(player_sonar_data_t);
	  cmd_size = 0;
	  type_str = "blob";
	  mod_type = STG_MODEL_BLOB;
	  break;	  

	default:
	  printf( "error: stage driver doesn't support interface type %d\n",
		 device.id.code );
	}

      
      if (this->AddInterface( device.id, PLAYER_ALL_MODE, 
			      data_size, cmd_size, 10, 10) != 0)
	{
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
      
      stg_model_t* base_model = 
	stg_world_model_name_lookup( StgDriver::world, model_name );
      
      if( base_model == NULL )
	{
	  PRINT_ERR1( " Error! find a Stage model named \"%s\"", model_name );
	  this->SetError(-1);
	  return;
	}
      
      // now find the model for this player device
      // find the first model in the tree that is the right type and
      // has not been used before
      device.mod = model_match( base_model, mod_type, this->devices );
      
      if( device.mod )
	{
	  printf( "model \"%s\"\n", device.mod->token );
	  
	  this->devices = g_array_append_val( this->devices, device );
	  
	  // now poke a data callback function into the model
	  //device.mod->data_notify = StgDriver::PublishLaserData;
	  //device.mod->data_notify_arg = (void*)this;
	}
      else
	{
	  printf( " ERROR! no model available for this device."
		  "Check your world and config files.\n" );
	  this->SetError(-1);
	  return;
	}
    }
  

  puts( "  Stage driver loaded successfully." );
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
  printf( "  Stage driver loading worldfile \"%s\" ", worldfile_name );      
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
  printf( "  Starting world clock... " ); fflush(stdout);
  //stg_world_resume( world );
  
  world->paused = FALSE;
  puts( "done." );
}





////////////////////////////////////////////////////////////////////////////////
// Set up the device.  Return 0 if things go well, and -1 otherwise.
int StgDriver::Setup()
{   
  puts("stg_mod driver initialising");

  // Here you do whatever is necessary to setup the device, like open and
  // configure a serial port.
  
  // subscribe to data from all the devices
  for( int i=0; i<(int)this->devices->len; i++ )
    {
      device_record_t* device = &g_array_index( this->devices, device_record_t, i );
      stg_model_subscribe( device->mod );
    }
  

  puts("stg_mod driver ready");


  return(0);
}

StgDriver::~StgDriver()
{
  // todo - when the sim thread exits, destroy the world it's not
  // urgent, because right now this only happens when Player quits.

  // stg_world_destroy( StgDriver::world );
}




////////////////////////////////////////////////////////////////////////////////
// Shutdown the device
int StgDriver::Shutdown()
{
  puts("Shutting stg_mod driver down");

  // Stop and join the driver thread
  // this->StopThread(); // todo - the thread only runs in the sim instance

  // unsubscribe to data from all the devices
  for( int i=0; i<(int)this->devices->len; i++ )
    {
      device_record_t* device = &g_array_index( this->devices, device_record_t, i );
      stg_model_unsubscribe( device->mod );
    }

  puts("stg_mod driver has been shutdown");

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
      
    // Check for and handle configuration requests
    this->CheckConfig();

    // Check for commands
    this->CheckCommands();

    gdk_threads_enter();

    // update the world, asking it to sleep a little if it has any
    // spare  time.
    if( stg_world_update( StgDriver::world, TRUE ) )
      exit( 0 );

    gdk_threads_leave();

    // Publish all the nice new data
    this->RefreshData();
  }
  return;
}


void StgDriver::CheckConfig()
{
  void *client;
  //unsigned char buffer[PLAYER_MAX_REQREP_SIZE];
  
  /*
  while (this->GetConfig(this->position_id, &client, &buffer, sizeof(buffer), NULL) > 0)
  {
    printf("got position request\n");
    if (this->PutReply(this->position_id, client, PLAYER_MSGTYPE_RESP_NACK, NULL, 0, NULL) != 0)
      PLAYER_ERROR("PutReply() failed");
  }

  while (this->GetConfig(this->laser_id, &client, &buffer, sizeof(buffer), NULL) > 0)
  {
    printf("got laser request\n");
    if (this->PutReply(this->laser_id, client, PLAYER_MSGTYPE_RESP_NACK, NULL, 0, NULL) != 0)
      PLAYER_ERROR("PutReply() failed");
  }
  */

  return;
}

void StgDriver::CheckCommands()
{
  /*
  this->GetCommand(this->position_id, &this->position_cmd, sizeof(this->position_cmd), NULL);

  printf("%d %d\n", ntohl(this->position_cmd.xspeed), ntohl(this->position_cmd.yawspeed));
  */

  return;
}

void StgDriver::RefreshData()
{ 
  
  for( int i=0; i<(int)this->devices->len; i++ )
    {
      device_record_t* device = &g_array_index( this->devices, device_record_t, i );
     
      switch( device->id.code )
	{
	case PLAYER_LASER_CODE:
	  this->RefreshDataLaser( device );
	  break;
	  
	case PLAYER_POSITION_CODE:
	  this->RefreshDataPosition( device );
	  break;

	case PLAYER_FIDUCIAL_CODE:
	  //this->RefreshDataFiducial( device );
	  break;

	case PLAYER_BLOBFINDER_CODE:
	  //this->RefreshDataFiducial( device );
	  break;
	case PLAYER_SONAR_CODE:
	  //this->RefreshDataRanger( device );
	  break;
	  
	default:
	  printf( "Stage driver error: unknown player device code (%d)\n",
		  device->id.code );	  	  
	}
    }
  
  return;
}

void StgDriver::RefreshDataLaser( device_record_t* device )
{
  //puts( "publishing laser data" );
  
  size_t dlen=0;
  stg_laser_sample_t* samples = 
    (stg_laser_sample_t*)stg_model_get_data( device->mod, &dlen );
  
  int sample_count = dlen / sizeof( stg_laser_sample_t );

  //for( int i=0; i<sample_count; i++ )
  //  printf( "rrrange %d %d\n", i, samples[i].range);

  player_laser_data_t pdata;
  memset( &pdata, 0, sizeof(pdata) );

  size_t clen=0;
  stg_laser_config_t* cfg = 
    (stg_laser_config_t*)stg_model_get_config( device->mod, &clen );
  
  assert(clen == sizeof(stg_laser_config_t));
  
  if( sample_count != cfg->samples )
    {
      //PRINT_ERR2( "bad laser data: got %d/%d samples",
      //	  sample_count, cfg->samples );
    }
  else
    {      
      double min_angle = -RTOD(cfg->fov/2.0);
      double max_angle = +RTOD(cfg->fov/2.0);
      double angular_resolution = RTOD(cfg->fov / cfg->samples);
      double range_multiplier = 1; // TODO - support multipliers
      
      pdata.min_angle = htons( (int16_t)(min_angle * 100 )); // degrees / 100
      pdata.max_angle = htons( (int16_t)(max_angle * 100 )); // degrees / 100
      pdata.resolution = htons( (uint16_t)(angular_resolution * 100)); // degrees / 100
      pdata.range_res = htons( (uint16_t)range_multiplier);
      pdata.range_count = htons( (uint16_t)cfg->samples );
      
      for( int i=0; i<cfg->samples; i++ )
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
  
  size_t datalen=0;
  stg_position_data_t* data = (stg_position_data_t*)
    stg_model_get_data( device->mod, &datalen );
  
  if( data && datalen == sizeof(stg_position_data_t) )      
    {      
      
      //PLAYER_MSG3( "get data data %.2f %.2f %.2f", data->x, data->y, data->a );
      player_position_data_t position_data;
      memset( &position_data, 0, sizeof(position_data) );
      // pack the data into player format
      position_data.xpos = ntohl((int32_t)(1000.0 * data->pose.x));
      position_data.ypos = ntohl((int32_t)(1000.0 * data->pose.y));
      position_data.yaw = ntohl((int32_t)(RTOD(data->pose.a)));
      
      // speeds
      position_data.xspeed= ntohl((int32_t)(1000.0 * data->velocity.x)); // mm/sec
      position_data.yspeed = ntohl((int32_t)(1000.0 * data->velocity.y));// mm/sec
      position_data.yawspeed = ntohl((int32_t)(RTOD(data->velocity.a))); // deg/sec
      
      position_data.stall = data->stall;// ? 1 : 0;
      
      //printf( "getdata called at %lu ms\n", stage_client->stagetime );
      
      // publish this data
      //pos->PutData( &position_data, sizeof(position_data), NULL); 

      // Write position data
      this->PutData( device->id, &position_data, sizeof(position_data), NULL);      
    }
}
