/*************************************************************************
 * robot.h - CRobot defintion - most of the action is here
            
 * RTV
 * $Id: robot.h,v 1.8.2.2 2000-12-06 03:57:16 ahoward Exp $
 ************************************************************************/

#include "offsets.h" // for the ACTS size defines
#include "image.h"

#include <X11/Xlib.h> // for XPoint

#ifndef ROBOT_H
#define ROBOT_H

/* *** remove ahoward
//#define PIONEER1
//#define SONARSAMPLES 7

//#define PIONEER2




*/

// For CObject
//
#include "object.hh"

// Forward declare some of the classes we will use
//
class CDevice;
class CWorld;

// Maximum number of devices
//
#define MAXDEVICES 32

class CRobot : public CObject
{
  // BPG
  pid_t player_pid;
  // GPB

public:
  CRobot( CWorld* world, CObject *parent);
  ~CRobot( void );
  
  // *** REMOVE CWorld* world; 
  CRobot* next; // for linked list implementation

  // position, position at t-1, and position origin variables
  float x, y, a, oldx, oldy, olda, xorigin, yorigin, aorigin;

  unsigned char color; // unique ID and value drawn into world bitmap
  unsigned char channel; // the apparent color of this robot in ACTS

  caddr_t playerIO; // ptr to shared memory for player I/O
  char tmpName[16]; // name of shared memory device in filesystem

  // Device list
  //
  int m_device_count;
  CDevice *m_device[ MAXDEVICES ]; 
 
  // render robot in the shared world representation
  void MapUnDraw( void );
  // erase robot from the shared world representation
  void MapDraw( void );

  // render robot in the GUI
  void GUIUnDraw( void );
  // erase robot from the GUI
  void GUIDraw( void );

  bool showDeviceDetail;

  void ToggleSensorDisplay( void )
    {
      showDeviceDetail = !showDeviceDetail;
    }

  bool HasMoved( void );

  // Update robot and all its devices
  //
  public: virtual void Update();

  // Start all the devices
  //
  public: virtual bool Startup();

  // Shutdown the devices
  //
  public: virtual void Shutdown();

  // Start player
  //
  private: bool StartupPlayer();

  // Stop player
  //
  private: void ShutdownPlayer();
  
  // RTK interface
  //
#ifdef INCLUDE_RTK
    
  // Process GUI update messages
  //
  public: virtual void OnUiUpdate(RtkUiDrawData *pData);

  // Process GUI mouse messages
  //
  public: virtual void OnUiMouse(RtkUiMouseData *pData);

#endif
};

#endif













