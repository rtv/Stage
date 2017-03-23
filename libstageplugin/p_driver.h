#ifndef _STAGE_PLAYER_DRIVER_H
#define _STAGE_PLAYER_DRIVER_H

#include <unistd.h>
#include <string.h>
#include <math.h>

#include <libplayercore/playercore.h>

#include "../libstage/stage.hh"

#define DRIVER_ERROR(X) printf("Stage driver error: %s\n", X)

// foward declare;
class Interface;
class StgTime;

class StgDriver : public Driver {
public:
  // Constructor; need that
  StgDriver(ConfigFile *cf, int section);

  // Destructor
  ~StgDriver(void);

  // Must implement the following methods.
  virtual int Setup();
  virtual int Shutdown();
  virtual int ProcessMessage(QueuePointer &resp_queue, player_msghdr *hdr, void *data);
  virtual int Subscribe(QueuePointer &queue, player_devaddr_t addr);
  virtual int Unsubscribe(QueuePointer &queue, player_devaddr_t addr);

  /// The server thread calls this method frequently. We use it to
  /// check for new commands and configs
  virtual void Update();

  /// all player devices share the same Stage world (for now)
  static Stg::World *world;
  static StgDriver *master_driver;
  static bool usegui;

  /// find the device record with this Player id
  Interface *LookupInterface(player_devaddr_t addr);

  Stg::Model *LocateModel(char *basename, player_devaddr_t *addr, const std::string &type);

protected:
  /// an array of pointers to Interface objects, defined below
  std::vector<Interface *> ifaces;
};

class Interface {
public:
  Interface(player_devaddr_t addr, StgDriver *driver, ConfigFile *cf, int section);

  virtual ~Interface(void){ /* TODO: clean up*/ };

  player_devaddr_t addr;

  StgDriver *driver; // the driver instance that created this device

  virtual int ProcessMessage(QueuePointer &resp_queue, player_msghdr_t *hdr, void *data)
  {
    return (-1);
  } // empty implementation

  virtual void Publish(void){}; // do nothing
  virtual void StageSubscribe(void){}; // do nothing
  virtual void StageUnsubscribe(void){}; // do nothing

  virtual void Subscribe(QueuePointer &queue){}; // do nothing
  virtual void Unsubscribe(QueuePointer &queue){}; // do nothing};
};

class InterfaceSimulation : public Interface {
public:
  InterfaceSimulation(player_devaddr_t addr, StgDriver *driver, ConfigFile *cf, int section);
  virtual ~InterfaceSimulation(void){ /* TODO: clean up*/ };
  virtual int ProcessMessage(QueuePointer &resp_queue, player_msghdr_t *hdr, void *data);
};

// base class for all interfaces that are associated with a model
class InterfaceModel

    : public Interface {
public:
  InterfaceModel(player_devaddr_t addr, StgDriver *driver, ConfigFile *cf, int section,
                 const std::string &type);

  virtual ~InterfaceModel(void) { StageUnsubscribe(); };
  virtual void StageSubscribe(void);
  virtual void StageUnsubscribe(void);

protected:
  Stg::Model *mod;

private:
  bool subscribed;
};

class InterfacePosition : public InterfaceModel {
public:
  InterfacePosition(player_devaddr_t addr, StgDriver *driver, ConfigFile *cf, int section);
  virtual ~InterfacePosition(void){ /* TODO: clean up*/ };
  virtual void Publish(void);
  virtual int ProcessMessage(QueuePointer &resp_queue, player_msghdr_t *hdr, void *data);
};

class InterfaceGripper : public InterfaceModel {
public:
  InterfaceGripper(player_devaddr_t addr, StgDriver *driver, ConfigFile *cf, int section);
  virtual ~InterfaceGripper(void){ /* TODO: clean up*/ };
  virtual int ProcessMessage(QueuePointer &resp_queue, player_msghdr_t *hdr, void *data);
  virtual void Publish(void);
};

class InterfaceWifi : public InterfaceModel {
public:
  InterfaceWifi(player_devaddr_t addr, StgDriver *driver, ConfigFile *cf, int section);
  virtual ~InterfaceWifi(void){ /* TODO: clean up*/ };
  virtual int ProcessMessage(QueuePointer &resp_queue, player_msghdr_t *hdr, void *data);
  virtual void Publish(void);
};

class InterfaceSpeech : public InterfaceModel {
public:
  InterfaceSpeech(player_devaddr_t addr, StgDriver *driver, ConfigFile *cf, int section);
  virtual ~InterfaceSpeech(void){ /* TODO: clean up*/ };
  virtual int ProcessMessage(QueuePointer &resp_queue, player_msghdr_t *hdr, void *data);
  virtual void Publish(void);
};

class InterfaceRanger : public InterfaceModel {
private:
  int scan_id;

public:
  InterfaceRanger(player_devaddr_t addr, StgDriver *driver, ConfigFile *cf, int section);
  virtual ~InterfaceRanger(void){ /* TODO: clean up*/ };
  virtual int ProcessMessage(QueuePointer &resp_queue, player_msghdr_t *hdr, void *data);
  virtual void Publish(void);
};

/*  class InterfaceAio : public InterfaceModel */
/* { */
/*  public: */
/*   InterfaceAio( player_devaddr_t addr, StgDriver* driver, ConfigFile* cf, int section ); */
/*   virtual ~InterfaceAio( void ){ /\* TODO: clean up*\/ }; */
/*   virtual int ProcessMessage(QueuePointer & resp_queue, */
/* 			      player_msghdr_t* hdr, */
/* 			      void* data); */
/*   virtual void Publish( void ); */
/* }; */

/* class InterfaceDio : public InterfaceModel */
/* { */
/* public: */
/* 	InterfaceDio(player_devaddr_t addr, StgDriver* driver, ConfigFile* cf, int section); */
/* 	virtual ~InterfaceDio(); */
/* 	virtual int ProcessMessage(QueuePointer & resp_queue, player_msghdr_t* hdr, void* data); */
/* 	virtual void Publish(); */
/* }; */

class InterfacePower : public InterfaceModel {
public:
  InterfacePower(player_devaddr_t addr, StgDriver *driver, ConfigFile *cf, int section);
  virtual ~InterfacePower(void){ /* TODO: clean up*/ };

  virtual int ProcessMessage(QueuePointer &resp_queue, player_msghdr *hdr, void *data);

  virtual void Publish(void);
};

class InterfaceFiducial : public InterfaceModel {
public:
  InterfaceFiducial(player_devaddr_t addr, StgDriver *driver, ConfigFile *cf, int section);
  virtual ~InterfaceFiducial(void){ /* TODO: clean up*/ };

  virtual void Publish(void);
  virtual int ProcessMessage(QueuePointer &resp_queue, player_msghdr_t *hdr, void *data);
};

class InterfaceActArray : public InterfaceModel {
public:
  InterfaceActArray(player_devaddr_t addr, StgDriver *driver, ConfigFile *cf, int section);
  virtual ~InterfaceActArray(void){ /* TODO: clean up*/ };

  virtual int ProcessMessage(QueuePointer &resp_queue, player_msghdr *hdr, void *data);
  virtual void Publish(void);
};

class InterfaceBlobfinder : public InterfaceModel {
public:
  InterfaceBlobfinder(player_devaddr_t addr, StgDriver *driver, ConfigFile *cf, int section);
  virtual ~InterfaceBlobfinder(void){ /* TODO: clean up*/ };

  virtual int ProcessMessage(QueuePointer &resp_queue, player_msghdr *hdr, void *data);
  virtual void Publish(void);
};

class InterfaceCamera : public InterfaceModel {
public:
  InterfaceCamera(player_devaddr_t addr, StgDriver *driver, ConfigFile *cf, int section);
  virtual ~InterfaceCamera(void){ /* TODO: clean up*/ };

  virtual int ProcessMessage(QueuePointer &resp_queue, player_msghdr *hdr, void *data);
  virtual void Publish(void);
};

class InterfacePtz : public InterfaceModel {
public:
  InterfacePtz(player_devaddr_t addr, StgDriver *driver, ConfigFile *cf, int section);
  virtual ~InterfacePtz(void){ /* TODO: clean up*/ };

  virtual int ProcessMessage(QueuePointer &resp_queue, player_msghdr *hdr, void *data);
  virtual void Publish(void);
};

class InterfaceBumper : public InterfaceModel {
public:
  InterfaceBumper(player_devaddr_t addr, StgDriver *driver, ConfigFile *cf, int section);
  virtual ~InterfaceBumper(void){ /* TODO: clean up*/ };

  virtual int ProcessMessage(QueuePointer &resp_queue, player_msghdr *hdr, void *data);
  virtual void Publish(void);
};

class InterfaceLocalize : public InterfaceModel {
public:
  InterfaceLocalize(player_devaddr_t addr, StgDriver *driver, ConfigFile *cf, int section);

  virtual ~InterfaceLocalize(void){ /* TODO: clean up*/ };

  virtual void Publish(void);
  virtual int ProcessMessage(QueuePointer &resp_queue, player_msghdr_t *hdr, void *data);
};

class InterfaceMap : public InterfaceModel {
public:
  InterfaceMap(player_devaddr_t addr, StgDriver *driver, ConfigFile *cf, int section);
  virtual ~InterfaceMap(void){ /* TODO: clean up*/ };

  virtual int ProcessMessage(QueuePointer &resp_queue, player_msghdr *hdr, void *data);

  // called by ProcessMessage to handle individual messages

  int HandleMsgReqInfo(QueuePointer &resp_queue, player_msghdr *hdr, void *data);
  int HandleMsgReqData(QueuePointer &resp_queue, player_msghdr *hdr, void *data);
};

class PlayerGraphics2dVis;
class InterfaceGraphics2d : public InterfaceModel {
public:
  InterfaceGraphics2d(player_devaddr_t addr, StgDriver *driver, ConfigFile *cf, int section);
  virtual ~InterfaceGraphics2d(void);

  virtual void Subscribe(QueuePointer &queue);
  virtual void Unsubscribe(QueuePointer &queue);

  virtual int ProcessMessage(QueuePointer &resp_queue, player_msghdr *hdr, void *data);

  PlayerGraphics2dVis *vis;
};

class PlayerGraphics3dVis;
class InterfaceGraphics3d : public InterfaceModel {
public:
  InterfaceGraphics3d(player_devaddr_t addr, StgDriver *driver, ConfigFile *cf, int section);
  virtual ~InterfaceGraphics3d(void);

  virtual void Subscribe(QueuePointer &queue);
  virtual void Unsubscribe(QueuePointer &queue);

  virtual int ProcessMessage(QueuePointer &resp_queue, player_msghdr *hdr, void *data);

  PlayerGraphics3dVis *vis;
};

/** Replaces Player's real time clock object */
class StTime : public PlayerTime {
private:
  StgDriver *driver;

public:
  // Constructor
  explicit StTime(StgDriver *driver);

  // Destructor
  virtual ~StTime();

  // Get the simulator time
  int GetTime(struct timeval *time);
  int GetTimeDouble(double *time);
};

#endif
