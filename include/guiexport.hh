/*************************************************************************
 * guiexport.hh - data types for exporting data to an external GUI
 * RTV
 * $Id: guiexport.hh,v 1.12 2002-06-04 06:35:07 rtv Exp $
 ************************************************************************/

#include <stage.h> // for some player data types and sizes
//#include "xs.hh"

#ifndef GUIEXPORT_H
#define GUIEXPORT_H

//#include "pioneermobiledevice.hh" // for definition of pioneer_shape_t

#define LABELSIZE 64
#define SONARSAMPLES PLAYER_NUM_SONAR_SAMPLES 
#define LASERSAMPLES PLAYER_NUM_LASER_SAMPLES 

#define MAXBEACONS 100

// for now i have a MAX_OBJECTS maximum to the number of objects
// that can be processed by the GUI. should make this dynamic one day
#define MAXEXPORTOBJECTS 10000;

// forward definition
class CEntity;

typedef struct  
{
  double x, y;
} DPoint;


enum ExportObjectType 
{ 
  player_o,
  sonar_o,
  misc_o,
  laserbeacondetector_o,
  vision_o,
  ptz_o, 
  generic_o, 
  laserturret_o, 
  pioneer_o, 
  uscpioneer_o,		  
  box_o, 
  laserbeacon_o,
  puck_o,
  gripper_o,
  gps_o,
};

//  typedef struct  
//  {
//    double x, y;
//  } DPoint;

//  typedef struct  
//  {
//    double x, y, th;
//  } DTriple;

//  typedef struct
//  {
//    double toplx, toply, toprx, topry, botlx, botly, botrx, botry;
//  } DRect;

typedef struct  
{
  double x, y, th;
  int id;
} BeaconData;


// structure to hold an object's exportable data
// i.e. sufficient to render it in an external GUI
// each CObject (CThing) has one of these when XGui is enabled.
typedef struct
{
  int objectType;
  CEntity* objectId;
  double x, y, th;
  double width, height;
  char* data;
  char label[LABELSIZE];
} ExportData;

typedef struct
{
  DPoint hitPts[LASERSAMPLES];
  int hitCount;
} ExportLaserData;

typedef struct
{
  DPoint hitPts[SONARSAMPLES];
  int hitCount;
} ExportSonarData;

typedef struct
{
  BeaconData beacons[MAXBEACONS];
  int beaconCount;
} ExportLaserBeaconDetectorData;

typedef struct
{
  double pan, tilt, zoom;
} ExportPtzData;

typedef struct
{
  bool paddles_open;
  bool paddles_closed;
  bool lift_up;
  bool lift_down;
  bool have_puck;
  double paddle_width;
  double paddle_height;
} ExportGripperData;

typedef struct
{
  int shape;
  double radius;
} ExportPositionData;



// for now the import type can be the same as the export type.
// might need to change this later, so i'll declare a type for it.
typedef  ExportData ImportData;

#endif






