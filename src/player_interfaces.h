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

// foward declare;
struct device_record;

class StgDriver : public Driver
{
 public:
  // Constructor; need that
  StgDriver(ConfigFile* cf, int section);
  
  // Destructor
  ~StgDriver(void);
  
  // Must implement the following methods.
  virtual int Setup();
  virtual int Shutdown();
  
  virtual int Subscribe(player_device_id_t id);
  virtual int Unsubscribe(player_device_id_t id);
  
  /// Main function for device thread.
  virtual void Main();

  /// The server thread calls this method frequently. We use it to
  /// check for new commands
  virtual void Update();
  
  // override the Driver method to grab configs inside the server thread
  virtual int PutConfig(player_device_id_t id, void *client, 
			void* src, size_t len,
			struct timeval* timestamp);
  
 protected: 
  /// find the device record with this Player id
  // todo - faster lookup with a better data structure
  struct device_record* LookupDevice( player_device_id_t id );
  
  /// used as a callback when a model has new data
  static void RefreshDataCallback( void* user, void* data, size_t len );
  //static GPtrArray* all_devices;
  
  // SIMULATION INTERFACE
  void InitSimulation( ConfigFile* cf, int section );
  
  /// an array of pointers to device_record_t structs, defined below
  GPtrArray* devices;
  
  /// all player devices share the same Stage world
  static stg_world_t* world;
};


typedef struct device_record
{
  player_device_id_t id;
  stg_model_t* mod;

  size_t data_len;
  
  StgDriver* driver; // the driver instance that created this device

  void* cmd;
  size_t cmd_len;
  
  void (*command_callback)( struct device_record* device, void* buffer, size_t len );
  void (*data_callback)( struct device_record* device, void* data, size_t len );
  void (*config_callback)( struct device_record* device, void* client, void* buffer, size_t len );
  
} device_record_t;


// declare a raft of functions that interface between Player and Stage interfaces

// SIMULATION INTERFACE
void SimulationCommand( device_record_t* device, void* buffer, size_t len );
void SimulationConfig(  device_record_t* device,  void* client, void* buffer, size_t len );

// POSITION INTERFACE
void PositionCommand( device_record_t* device, void* buffer, size_t len );
void PositionData( device_record_t* device, void* data, size_t len );
void PositionConfig( device_record_t* device, void* client, void* buffer, size_t len );

// LOCALIZE INTERFACE
void LocalizeCommand( device_record_t* device, void* buffer, size_t len );
void LocalizeData( device_record_t* device, void* data, size_t len );
void LocalizeConfig( device_record_t* device, void* client, void* buffer, size_t len );

// SONAR INTERFACE
void SonarData( device_record_t* device, void* data, size_t len );
void SonarConfig( device_record_t* device, void* client, void* buffer, size_t len );

// ENERGY INTERFACE
void EnergyData( device_record_t* device, void* data, size_t len );
void EnergyConfig( device_record_t* device, void* client, void* buffer, size_t len );

// SIMULATION INTERFACE
void SimulationData( device_record_t* device, void* data, size_t len );
void SimulationConfig( player_device_id_t id, void* client, void* buffer, size_t len);

// BLOBFINDER INTERFACE
void BlobfinderData( device_record_t* device, void* data, size_t len );
void BlobfinderConfig( device_record_t* device, void* client, void* buffer, size_t len);

// LASER INTERFACE
void LaserConfig( device_record_t* device, void* client, void* buffer, size_t len );
void LaserData( device_record_t* device, void* data, size_t len );  

// FIDUCIAL INTERFACE
void FiducialData( device_record_t* device, void* data, size_t len );
void FiducialConfig( device_record_t* device, void* client, void* buffer, size_t len );

// MAP INTERFACE
void MapData( device_record_t* device, void* data, size_t len );
void MapConfig( device_record_t* device, void* client, void* buffer, size_t len);
void MapConfigInfo( device_record_t* device, void* client, void* buffer, size_t len);
void MapConfigData( device_record_t* device, void* client, void* buffer, size_t len);


