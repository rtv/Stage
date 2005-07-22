#ifndef _STAGE_PLAYER_DRIVER_H
#define _STAGE_PLAYER_DRIVER_H

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

#define DRIVER_ERROR(X) printf( "Stage driver error: %s\n", X )

// foward declare;
class Interface;

class StgDriver : public Driver
{
 public:
  // Constructor; need that
  StgDriver(ConfigFile* cf, int section);
  
  // Destructor
  ~StgDriver(void);
  
  // Must implement the following methods.
  int Setup();
  int Shutdown();
  
  int Subscribe(player_device_id_t id);
  int Unsubscribe(player_device_id_t id);
  
  /// Main function for device thread.
  void Main();
  
  /// The server thread calls this method frequently. We use it to
  /// check for new commands and configs
  virtual void Update();
  
  // override the Driver method to grab configs inside the server thread
  //int PutConfig(player_device_id_t id, void *client, 
  //		void* src, size_t len,
  //		struct timeval* timestamp);
  
  /// all player devices share the same Stage world
  static stg_world_t* world;
  
  /// find the device record with this Player id
  Interface* LookupDevice( player_device_id_t id );
  
  stg_model_t* LocateModel( const char* basename, 
			    stg_model_type_t mod_type );
  
 protected: 
  
  /// an array of pointers to device_record_t structs, defined below
  GPtrArray* devices;  
};


class Interface
{
 public:
  Interface( player_device_id_t id,  StgDriver* driver,ConfigFile* cf, int section );
  
  virtual ~Interface( void ){ /* TODO: clean up*/ };

  player_device_id_t id;
  stg_model_t* mod;
  
  StgDriver* driver; // the driver instance that created this device
  
  size_t data_len;
  size_t cmd_len;
  size_t req_qlen;
  size_t rep_qlen;

  // pure virtual methods
  virtual void Command( void* buffer, size_t len ){}; // empty implementation
  virtual void Configure( void* client, void* buffer, size_t len ){}; // empty implementation
  virtual void Publish( void ){}; // empty implementation
};


class InterfaceSimulation : public Interface
{
 public: 
  InterfaceSimulation( player_device_id_t id,  StgDriver* driver,ConfigFile* cf, int section );
  virtual ~InterfaceSimulation( void ){ /* TODO: clean up*/ };

  //virtual void Command( void* buffer, size_t len ); 
  virtual void Configure( void* client, void* buffer, size_t len );
};


class InterfaceModel : public Interface
{
 public: 
  InterfaceModel( player_device_id_t id,  
		  StgDriver* driver,
		  ConfigFile* cf, 
		  int section, 
		  stg_model_type_t modtype );

  virtual ~InterfaceModel( void ){ /* TODO: clean up*/ };
};

class InterfacePosition : public InterfaceModel
{
 public: 
  InterfacePosition( player_device_id_t id, StgDriver* driver, ConfigFile* cf, int section );
  virtual ~InterfacePosition( void ){ /* TODO: clean up*/ };
  virtual void Command( void* buffer, size_t len ); 
  virtual void Configure( void* client, void* buffer, size_t len );
  virtual void Publish( void );
};
 
class InterfaceGripper : public InterfaceModel
{
 public: 
  InterfaceGripper( player_device_id_t id, StgDriver* driver, ConfigFile* cf, int section );
  virtual ~InterfaceGripper( void ){ /* TODO: clean up*/ };
  virtual void Command( void* buffer, size_t len ); 
  virtual void Configure( void* client, void* buffer, size_t len );
  virtual void Publish( void );
};

class InterfaceLaser : public InterfaceModel
{
 public: 
  InterfaceLaser( player_device_id_t id, StgDriver* driver, ConfigFile* cf, int section );
  virtual ~InterfaceLaser( void ){ /* TODO: clean up*/ };
  //virtual void Command( void* buffer, size_t len ); 
  virtual void Configure( void* client, void* buffer, size_t len );
  virtual void Publish( void );
};

class InterfaceFiducial : public InterfaceModel
{
 public: 
  InterfaceFiducial( player_device_id_t id, StgDriver* driver, ConfigFile* cf, int section );
  virtual ~InterfaceFiducial( void ){ /* TODO: clean up*/ };
  //virtual void Command( void* buffer, size_t len ); 
  virtual void Configure( void* client, void* buffer, size_t len );
  virtual void Publish( void );
};
 
class InterfaceBlobfinder : public InterfaceModel
{
 public: 
  InterfaceBlobfinder( player_device_id_t id, StgDriver* driver, ConfigFile* cf, int section );
  virtual ~InterfaceBlobfinder( void ){ /* TODO: clean up*/ };
  //virtual void Command( void* buffer, size_t len ); 
  virtual void Configure( void* client, void* buffer, size_t len );
  virtual void Publish( void );
};

class InterfaceSonar : public InterfaceModel
{
 public: 
  InterfaceSonar( player_device_id_t id, StgDriver* driver, ConfigFile* cf, int section );
  virtual ~InterfaceSonar( void ){ /* TODO: clean up*/ };
  //virtual void Command( void* buffer, size_t len ); 
  virtual void Configure( void* client, void* buffer, size_t len );
  virtual void Publish( void );
};



#endif
