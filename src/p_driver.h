#ifndef _STAGE_PLAYER_DRIVER_H
#define _STAGE_PLAYER_DRIVER_H

#include <unistd.h>
#include <string.h>
#include <math.h>

#include <libplayercore/playercore.h>

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
  virtual int Setup();
  virtual int Shutdown();
  virtual int ProcessMessage(MessageQueue* resp_queue, 
			     player_msghdr * hdr, 
			     void * data);
  virtual int Subscribe(player_devaddr_t addr);
  virtual int Unsubscribe(player_devaddr_t addr);
    
  /// The server thread calls this method frequently. We use it to
  /// check for new commands and configs
  virtual void Update();

  /// all player devices share the same Stage world (for now)
  static stg_world_t* world;
  
  /// find the device record with this Player id
  Interface* LookupDevice( player_devaddr_t addr );
  
  stg_model_t* LocateModel( char* basename, 
			    player_devaddr_t* addr,
			    stg_model_initializer_t init );
  
 protected: 
  
  /// an array of pointers to Interface objects, defined below
  GPtrArray* devices;  
};


class Interface
{
 public:
  Interface(player_devaddr_t addr,  
            StgDriver* driver,
            ConfigFile* cf, 
            int section );
  
  virtual ~Interface( void ){ /* TODO: clean up*/ };

  player_devaddr_t addr;
  stg_model_t* mod;
  double last_publish_time;
  
  StgDriver* driver; // the driver instance that created this device
  
  virtual int ProcessMessage(MessageQueue* resp_queue,
       			     player_msghdr_t* hdr,
			     void* data) { return(-1); } // empty implementation
  virtual void Publish( void ){}; // empty implementation
};


class InterfaceSimulation : public Interface
{
 public: 
  InterfaceSimulation( player_devaddr_t addr,  StgDriver* driver,ConfigFile* cf, int section );
  virtual ~InterfaceSimulation( void ){ /* TODO: clean up*/ };
  virtual int ProcessMessage(MessageQueue* resp_queue,
                             player_msghdr_t* hdr,
                             void* data);
};


class InterfaceModel : public Interface
{
 public: 
  InterfaceModel( player_devaddr_t addr,  
		  StgDriver* driver,
		  ConfigFile* cf, 
		  int section, 
		  stg_model_initializer_t init );
  
  virtual ~InterfaceModel( void ){ /* TODO: clean up*/ };
};

class InterfacePosition : public InterfaceModel
{
 private:
  bool stopped;
  int last_cmd_time;
 public: 
  InterfacePosition( player_devaddr_t addr, StgDriver* driver, ConfigFile* cf, int section );
  virtual ~InterfacePosition( void ){ /* TODO: clean up*/ };
  virtual void Publish( void );
  virtual int ProcessMessage(MessageQueue* resp_queue,
                             player_msghdr_t* hdr,
                             void* data);
};
 
class InterfaceGripper : public InterfaceModel
{
 public: 
  InterfaceGripper( player_devaddr_t addr, StgDriver* driver, ConfigFile* cf, int section );
  virtual ~InterfaceGripper( void ){ /* TODO: clean up*/ };
  virtual int ProcessMessage(MessageQueue* resp_queue,
                             player_msghdr_t* hdr,
                             void* data);
  virtual void Publish( void );
};

class InterfaceWifi : public InterfaceModel
{
 public: 
  InterfaceWifi( player_devaddr_t addr, StgDriver* driver, ConfigFile* cf, int section );
  virtual ~InterfaceWifi( void ){ /* TODO: clean up*/ };
  virtual int ProcessMessage(MessageQueue* resp_queue,
                             player_msghdr_t* hdr,
                             void* data);
  virtual void Publish( void );
};

class InterfaceSpeech : public InterfaceModel
{
 public: 
  InterfaceSpeech( player_devaddr_t addr, StgDriver* driver, ConfigFile* cf, int section );
  virtual ~InterfaceSpeech( void ){ /* TODO: clean up*/ };
  virtual int ProcessMessage(MessageQueue* resp_queue,
                             player_msghdr_t* hdr,
                             void* data);
  virtual void Publish( void );
};

class InterfaceAudio : public InterfaceModel
{
 public: 
  InterfaceAudio( player_devaddr_t addr, StgDriver* driver, ConfigFile* cf, int section );
  virtual ~InterfaceAudio( void ){ /* TODO: clean up*/ };
  virtual int ProcessMessage(MessageQueue* resp_queue,
                             player_msghdr_t* hdr,
                             void* data);
  virtual void Publish( void );
};

class InterfaceLaser : public InterfaceModel
{
  private:
    int scan_id;
 public: 
  InterfaceLaser( player_devaddr_t addr, StgDriver* driver, ConfigFile* cf, int section );
  virtual ~InterfaceLaser( void ){ /* TODO: clean up*/ };
  virtual int ProcessMessage(MessageQueue* resp_queue,
			      player_msghdr_t* hdr,
			      void* data);
  virtual void Publish( void );
};

class InterfacePower : public InterfaceModel
{
 public: 
  InterfacePower( player_devaddr_t addr, StgDriver* driver, ConfigFile* cf, int section );
  virtual ~InterfacePower( void ){ /* TODO: clean up*/ };
  
  virtual int ProcessMessage( MessageQueue* resp_queue, 
			      player_msghdr * hdr, 
			      void * data );
  
  virtual void Publish( void );
};

class InterfaceFiducial : public InterfaceModel
{
 public: 
  InterfaceFiducial( player_devaddr_t addr, StgDriver* driver, ConfigFile* cf, int section );
  virtual ~InterfaceFiducial( void ){ /* TODO: clean up*/ };

  virtual void Publish( void );
  virtual int ProcessMessage(MessageQueue* resp_queue,
                             player_msghdr_t* hdr,
                             void* data);
};

 
class InterfaceBlobfinder : public InterfaceModel
{
 public: 
  InterfaceBlobfinder( player_devaddr_t addr, StgDriver* driver, ConfigFile* cf, int section );
  virtual ~InterfaceBlobfinder( void ){ /* TODO: clean up*/ };
  
  virtual int ProcessMessage( MessageQueue* resp_queue, 
			      player_msghdr * hdr, 
			      void * data );
  virtual void Publish( void );
};

class InterfacePtz : public InterfaceModel
{
 public: 
  InterfacePtz( player_devaddr_t addr, StgDriver* driver, ConfigFile* cf, int section );
  virtual ~InterfacePtz( void ){ /* TODO: clean up*/ };
  
  virtual int ProcessMessage( MessageQueue* resp_queue, 
			      player_msghdr * hdr, 
			      void * data );
  virtual void Publish( void );
};

class InterfaceSonar : public InterfaceModel
{
 public: 
  InterfaceSonar( player_devaddr_t addr, StgDriver* driver, ConfigFile* cf, int section );
  virtual ~InterfaceSonar( void ){ /* TODO: clean up*/ };
  
  virtual int ProcessMessage( MessageQueue* resp_queue, 
			      player_msghdr * hdr, 
			      void * data );
  virtual void Publish( void );
};
 

class InterfaceBumper : public InterfaceModel
{
 public: 
  InterfaceBumper( player_devaddr_t addr, StgDriver* driver, ConfigFile* cf, int section );
  virtual ~InterfaceBumper( void ){ /* TODO: clean up*/ };
  
  virtual int ProcessMessage( MessageQueue* resp_queue, 
			      player_msghdr * hdr, 
			      void * data );
  virtual void Publish( void );
};
 
class InterfaceLocalize : public InterfaceModel
{
 public: 
  InterfaceLocalize( player_devaddr_t addr, 
		     StgDriver* driver, 
		     ConfigFile* cf, 
		     int section );

  virtual ~InterfaceLocalize( void ){ /* TODO: clean up*/ };

  virtual void Publish( void );
  virtual int ProcessMessage(MessageQueue* resp_queue,
                             player_msghdr_t* hdr,
                             void* data);
};

class InterfaceMap : public InterfaceModel
{
 public: 
  InterfaceMap( player_devaddr_t addr, StgDriver* driver, ConfigFile* cf, int section );
  virtual ~InterfaceMap( void ){ /* TODO: clean up*/ };
  
  virtual int ProcessMessage( MessageQueue* resp_queue, 
			      player_msghdr * hdr, 
			      void * data );
  //virtual void Publish( void );
  
  // called by ProcessMessage to handle individual messages

  int HandleMsgReqInfo( MessageQueue* resp_queue, 
			player_msghdr * hdr, 
			void * data );
  int HandleMsgReqData( MessageQueue* resp_queue, 
			player_msghdr * hdr, 
			void * data );
};

class InterfaceGraphics2d : public InterfaceModel
{
 public: 
  InterfaceGraphics2d( player_devaddr_t addr, StgDriver* driver, ConfigFile* cf, int section );
  virtual ~InterfaceGraphics2d( void );
  
  virtual int ProcessMessage( MessageQueue* resp_queue, 
			      player_msghdr * hdr, 
			      void * data );
 private:
  stg_rtk_fig_t* fig; // a figure we can draw in
};



#endif
