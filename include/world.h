/*************************************************************************
 * world.h - most of the header action is here 
 * RTV
 * $Id: world.h,v 1.3.2.1 2000-12-05 23:17:34 ahoward Exp $
 ************************************************************************/

#ifndef WORLD_H
#define WORLD_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <iomanip.h>
#include <termios.h>
#include <strstream.h>
#include <iostream.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fstream.h>

#include "robot.h"
#include "image.h"
#include "win.h"

#ifdef INCLUDE_RTK
#include "rtk-ui.hh"
#endif

// forward declaration
class CWorldWin;
class CObject;


// Enumerated type describing the layers in the world rep
//
enum EWorldLayer
{
    layer_obstacle = 0,
    layer_laser = 1
};


// various c function declarations - an untidy mix of c and c++ - yuk!
int InitNetworking( void );


class CWorld
{
public:
  CWorld( char* initFile );
  virtual ~CWorld();

  CWorldWin* win;

  int semKey, semid; // semaphore access for shared mem locking

  int LockShmem( void ); // protect shared mem areas
  int UnlockShmem( void );

  // data
  Nimage* bimg; // background image 
  Nimage* img; //foreground img;

  // Laser image
  // Store laser intensity data (beacons)
  //
  Nimage *m_laser_img;

  int width, height, depth; 
  int paused;

  unsigned char channels[256];
  char posFile[64];
  char bgFile[64];

  CRobot* bots;
  float pioneerWidth, pioneerLength;
  float localizationNoise;
  float sonarNoise;

  float maxAngularError; // percent error on turning odometry

  float ppm;

  int* hits;
  int population;

  int refreshBackground;

  double timeStep, timeNow, timeThen, timeBegan;

  double sonarInterval, laserInterval, visionInterval, ptzInterval; // seconds

  // methods
  int LoadVars( char* initFile);
  void Update( void );
  void Draw( void );
  void SavePos( void );
  //void LoadPos( void );

  void UpdateSonar( int b );
  void DumpSonar( void );
  void DumpOdometry( void );
  void GetUpdate( void );  

  CRobot* NearestRobot( float x, float y );

  ///////////////////////////////////////////////////////
  // Added new stuff here -- ahoward

  // Startup routine -- creates objects in the world
  //
  public: bool Startup();

  // Shutdown routine -- deletes objects in the world
  //
  public: void Shutdown();

  // A list of all the objects in the world
  //
  private: int m_object_count;
  private: CObject *m_object[256];

  // Get a cell from the world grid
  //
  public: BYTE GetCell(double px, double py, EWorldLayer layer);

  // Set a cell in the world grid
  //
  public: void SetCell(double px, double py, EWorldLayer layer, BYTE value);

  // Set a rectangle in the world grid
  //
  public: void SetRectangle(double px, double py, double pth,
                            double dx, double dy, EWorldLayer layer, BYTE value);
  

#ifdef INCLUDE_RTK

  // Process GUI update messages
  //
  public: virtual void OnUiUpdate(RtkUiDrawData *pData);

  // Process GUI mouse messages
  //
  public: virtual void OnUiMouse(RtkUiMouseData *pData);

  // Draw the background; i.e. things that dont move
  //
  private: void DrawBackground(RtkUiDrawData *pData);

  // Draw the laser layer
  //
  private: void DrawLayer(RtkUiDrawData *pData, EWorldLayer layer);
  
#endif
};

#endif




