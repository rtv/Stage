/*************************************************************************
 * robot.h - CRobot defintion - most of the action is here
            
 * RTV
 * $Id: robot.h,v 1.3 2000-11-29 22:44:49 vaughan Exp $
 ************************************************************************/

#include <offsets.h> // for the ACTS size defines
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
  CRobot( int iid, float w, float l, 
	  float startx, float starty, float starta );
  ~CRobot( void );
  
  
  float x, y, a, oldx, oldy, olda;  
  float width, length, speed, turnRate;
  float halfWidth, halfLength;

  unsigned char color;
  unsigned char channel;

  float xodom;
  float yodom;
  float aodom; 
  float xorigin, yorigin, aorigin;

  int id;
  int fd;
  unsigned char stall;

  caddr_t playerIO;
  char tmpName[16];

  ofstream* log;

  float sonar[SONARSAMPLES]; 
  float laser[LASERSAMPLES]; 

  double lastLaser, lastSonar, lastVision, lastPtz;

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

  XPoint hitPts[SONARSAMPLES];
  XPoint oldHitPts[SONARSAMPLES];

  XPoint lhitPts[LASERSAMPLES];
  XPoint loldHitPts[LASERSAMPLES];

  Rect rect, oldRect;
  // center pixel positions are used to draw the direction indicator 
  int centerx, oldCenterx, centery, oldCentery;
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
  
  void Stop( void );
  void Move( void );
  void HitObstacle( void );  
  void DumpSonar( void );
  void DumpOdometry( void );
  void LogPosition( void );

  void Update( Nimage* img );
  int UpdateSonar( Nimage* img ); 
  //int UpdateLaser( Nimage* img ); 
  int UpdateVision( Nimage* img ); 

  void CalculateRect( float x, float y, float a );
  void CalculateRect( void ){ CalculateRect( x, y, a ); };
  void StoreRect( void );
  void UnDraw( Nimage* img );
  void Draw( Nimage* img );
  int Move( Nimage* img );

  int HasMoved( void );

  void PublishVision( void );
  //void PublishLaser( void );
  void PublishSonar( void );
  void PublishPosition( void );
  void GetPositionCommands( void );

  int UpdateAndPublishPtz( void );
};

#endif
