/*************************************************************************
 * world.h - most of the header action is here 
 * RTV
 * $Id: world.h,v 1.1.1.1 2000-11-29 00:16:53 ahoward Exp $
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

//class ZoneRect
//{
//public:
// float x, y, w, h;
//};

// various c function declarations - an untidy mix of c and c++ - yuk!
int InitNetworking( void );


class CWorld
{
public:
  CWorld( char* initFile );

  int semKey, semid; // semaphore access for shared mem locking

  int LockShmem( void ); // protect shared mem areas
  int UnlockShmem( void );

  // data
  Nimage* bimg; // background image 
  Nimage* img; //foreground img;

  int width, height, depth; 
  int paused;

  unsigned char channels[256];
  char posFile[64];
  char bgFile[64];
  char zoneFile[64];

  CRobot* bots;
  float pioneerWidth, pioneerLength;
  float localizationNoise;
  float sonarNoise;

  float maxAngularError; // percent error on turning odometry

  float ppm;

  //int zc; // number of zones
  //ZoneRect zones[PROXIMITY_ZONES];

  int* hits;
  int population;

  int refreshBackground;

  double timeStep, timeNow, timeThen, timeBegan;
  int useLaser, useColor, useSonar; // deprecated - take these out

  double sonarInterval, laserInterval, visionInterval, ptzInterval; // seconds

  // methods
  void ToggleTrails( void );
  int LoadVars( char* initFile);
  int Connected( CRobot* r1, CRobot* r2 );
  void Update( void );
  void Draw( void );
  void SavePos( void );
  void LoadPos( void );
  //void SaveZones( void );
  //void LoadZones( void );
  void UpdateSonar( int b );
  void DumpSonar( void );
  void DumpOdometry( void );
  void GetUpdate( void );  

  CRobot* NearestRobot( float x, float y );

};

#endif




