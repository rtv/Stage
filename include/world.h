/*************************************************************************
 * world.h - most of the header action is here 
 * RTV
 * $Id: world.h,v 1.2 2000-12-01 00:20:52 vaughan Exp $
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

// forward declaration
class CWorldWin;

// various c function declarations - an untidy mix of c and c++ - yuk!
int InitNetworking( void );


class CWorld
{
public:
  CWorld( char* initFile );

  CWorldWin* win;

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

};

#endif




