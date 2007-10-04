#ifndef _STAGE_PLAYER_INTERFACES_H
#define _STAGE_PLAYER_INTERFACES_H

#include "player_driver.h"

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


// declare a raft of functions that interface between Player and Stage interfaces


// POSITION INTERFACE
//void PositionCommand( Interface* device, void* buffer, size_t len );
//void PositionData( Interface* device, void* data, size_t len );
//int PositionData( stg_model_t* mod, char* name, void* data, size_t len, void* userp );  
//void PositionConfig( Interface* device, void* client, void* buffer, size_t len );

// LOCALIZE INTERFACE
//void LocalizeCommand( Interface* device, void* buffer, size_t len );
//void LocalizeData( Interface* device, void* data, size_t len );
//void LocalizeConfig( Interface* device, void* client, void* buffer, size_t len );

// SONAR INTERFACE
//void SonarData( Interface* device, void* data, size_t len );
//void SonarConfig( Interface* device, void* client, void* buffer, size_t len );

// ENERGY INTERFACE
//void EnergyData( Interface* device, void* data, size_t len );
//void EnergyConfig( Interface* device, void* client, void* buffer, size_t len );

// SIMULATION INTERFACE
//void SimulationData( Interface* device, void* data, size_t len );
//void SimulationConfig( player_device_id_t id, void* client, void* buffer, size_t len);

// BLOBFINDER INTERFACE
//void BlobfinderData( Interface* device, void* data, size_t len );
//void BlobfinderConfig( Interface* device, void* client, void* buffer, size_t len);

// LASER INTERFACE
//void LaserConfig( Interface* device, void* client, void* buffer, size_t len );
//int LaserData( stg_model_t* mod, char* name, void* data, size_t len, void* userp );  

// GRIPPER INTERFACE
//void GripperCommand( Interface* device, void* src, size_t len );
//void GripperConfig( Interface* device, void* client, void* buffer, size_t len );
//void GripperData( Interface* device, void* data, size_t len );  

// FIDUCIAL INTERFACE
//void FiducialData( Interface* device, void* data, size_t len );
//int FiducialData( stg_model_t* mod, char* name, void* data, size_t len, void* userp );  
//void FiducialConfig( Interface* device, void* client, void* buffer, size_t len );

// MAP INTERFACE
//void MapData( Interface* device, void* data, size_t len );
//void MapConfig( Interface* device, void* client, void* buffer, size_t len);
//void MapConfigInfo( Interface* device, void* client, void* buffer, size_t len);
//void MapConfigData( Interface* device, void* client, void* buffer, size_t len);


#endif
