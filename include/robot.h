/*************************************************************************
 * robot.h - CRobot defintion - most of the action is here
            
 * RTV
 * $Id: robot.h,v 1.6 2000-12-01 02:50:36 vaughan Exp $
 ************************************************************************/

#include "offsets.h" // for the ACTS size defines
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
  CRobot( CWorld* ww, int iid, float w, float l, 
	  float startx, float starty, float starta );
  ~CRobot( void );
  
  CWorld* world;
  
  float x, y, a, oldx, oldy, olda;  

  unsigned char color;
  unsigned char channel;

  float xorigin, yorigin, aorigin;

  int fd;

  caddr_t playerIO;
  char tmpName[16];

  ofstream* log;

  //float sonar[SONARSAMPLES]; 
  //float laser[LASERSAMPLES]; 

  //double lastLaser, lastSonar, lastVision, lastPtz;
  double lastSonar, lastVision, lastPtz;

  double cameraAngleMax, cameraAngleMin, cameraFOV;
  double cameraPanMin, cameraPanMax, cameraPan;
  int cameraImageWidth, cameraImageHeight;

  int numBlobs;
  ColorBlob blobs[MAXBLOBS];

  unsigned char actsBuf[ACTS_TOTAL_MAX_SIZE];

  int redrawSonar;
  int redrawLaser;
  int leaveTrails;

  int leaveTrail;

  //XPoint hitPts[SONARSAMPLES];
  //XPoint oldHitPts[SONARSAMPLES];

  XPoint lhitPts[LASERSAMPLES];
  XPoint loldHitPts[LASERSAMPLES];

  CRobot* next;

  double lastCommand; // remember the time of the last command

  // Device list
  //
  int m_device_count;
  CDevice *m_device[32];

  // Start all the devices
  //
  bool Startup();

  // Shutdown the devices
  //
  bool Shutdown();
  
  void DumpSonar( void );
  void DumpOdometry( void );
  void LogPosition( void );

  void MapUnDraw( void );
  void MapDraw( void );

  void GUIUnDraw( void );
  void GUIDraw( void );

  bool HasMoved( void );

  void Update();
  //int UpdateSonar( Nimage* img ); 
  //int UpdateLaser( Nimage* img ); 
  //int UpdateVision( Nimage* img ); 


  //void PublishVision( void );
  //void PublishLaser( void );
  //void PublishSonar( void );
  //void PublishPosition( void );
  //void GetPositionCommands( void );

  int UpdateAndPublishPtz( void );
};

#endif
