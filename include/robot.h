/*************************************************************************
 * robot.h - CRobot defintion - most of the action is here
            
 * RTV
 * $Id: robot.h,v 1.10 2001-01-13 02:45:54 gerkey Exp $
 ************************************************************************/

//#include "offsets.h" // for the ACTS size defines
#include "stage.h" 
#include "image.h"

#include <X11/Xlib.h> // for XPoint

#ifndef ROBOT_H
#define ROBOT_H

//#define PIONEER1
//#define SONARSAMPLES 7

//#define PIONEER2
#define LASERSAMPLES 361
#define SONARSAMPLES 16

#define MAXBLOBS 100 // PDOMA!
#define MAXDEVICES 32


// Forward declare some of the class we will use
//
class CDevice;
class CWorld;

typedef struct
{
  unsigned char channel;
  int area;
  int x, y;
  int left, top, right, bottom;
} ColorBlob;


class CRobot
{
  // BPG
  pid_t player_pid;
  // GPB

public:
  CRobot( CWorld* ww, int iid, double w, double l, 
	  double startx, double starty, double starta );
  ~CRobot( void );
  
  CWorld* world; 
  CRobot* next; // for linked list implementation

  // position, position at t-1, and position origin variables
  double x, y, a, oldx, oldy, olda, xorigin, yorigin, aorigin;

  unsigned char color; // unique ID and value drawn into world bitmap
  unsigned char channel; // the apparent color of this robot in ACTS

  caddr_t playerIO; // ptr to shared memory for player I/O
  char tmpName[128]; // name of shared memory device in filesystem

  // Device list
  //
  int m_device_count;
  CDevice *m_device[ MAXDEVICES ]; 

  // Start all the devices
  //
  bool Startup();

  // Shutdown the devices
  //
  bool Shutdown();
 
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
  void Update();

  // reset the position and origin 
  void ResetPosition( double xx, double yy, double aa );

};

#endif













