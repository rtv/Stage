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
  void Update();
  
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


#endif
