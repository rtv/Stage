///////////////////////////////////////////////////////////////////////////
//
// File: stage_types.hh
// Author: Andrew Howard
// Date: 12 Mar 2001
// Desc: Types and macros for stage
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/include/stage_types.hh,v $
//  $Author: gsibley $
//  $Revision: 1.19 $
//
// Usage:
//  (empty)
//
// Theory of operation:
//  (empty)
//
// Known bugs:
//  (empty)
//
// Possible enhancements:
//  (empty)
//
///////////////////////////////////////////////////////////////////////////

#ifndef STAGE_TYPES_HH
#define STAGE_TYPES_HH

#include <stddef.h>
#include <assert.h>
#include <math.h>

///////////////////////////////////////////////////////////////////////////
// Some useful macros

// Determine size of array
//
#ifndef ARRAYSIZE
#define ARRAYSIZE(x) (int) (sizeof(x) / sizeof(x[0]))
#endif

// size of char arrays for hostnames
#define HOSTNAME_SIZE 32
#define MILLION 1000000L

#ifndef M_PI
	#define M_PI        3.14159265358979323846
#endif

#ifndef TWOPI
	#define TWOPI        6.2831853
#endif

#define STAGE_SYNC 0
#define STAGE_ASYNC 1

// Convert radians to degrees
//
#define RTOD(r) ((r) * 180 / M_PI)

// Convert degrees to radians
//
#define DTOR(d) ((d) * M_PI / 180)

// Normalize angle to domain -pi, pi
//
#define NORMALIZE(z) atan2(sin(z), cos(z))

#define ASSERT(m) assert(m)

// Error macros
#define PRINT_ERR(m)         printf("\rstage error : %s %s\n  "m"\n", \
                                    __FILE__, __FUNCTION__)
#define PRINT_ERR1(m, a)     printf("\rstage error : %s %s\n  "m"\n", \
                                    __FILE__, __FUNCTION__, a)
#define PRINT_ERR2(m, a, b)  printf("\rstage error : %s %s\n  "m"\n", \
                                    __FILE__, __FUNCTION__, a, b)

// Warning macros
#define PRINT_WARN(m)         printf("\rstage warning : %s %s\n  "m"\n", \
                                     __FILE__, __FUNCTION__)
#define PRINT_WARN1(m, a)     printf("\rstage warning : %s %s\n  "m"\n", \
                                     __FILE__, __FUNCTION__, a)
#define PRINT_WARN2(m, a, b)  printf("\rstage warning : %s %s\n  "m"\n", \
                                     __FILE__, __FUNCTION__, a, b)
#define PRINT_WARN3(m, a, b, c) printf("\rstage warning : %s %s\n  "m"\n", \
                                     __FILE__, __FUNCTION__, a, b, c)

// Message macros
#define PRINT_MSG(m) printf("stage msg : %s :\n  "m"\n", __FILE__)
#define PRINT_MSG1(m, a) printf("stage msg : %s :\n  "m"\n", __FILE__, a)
#define PRINT_MSG2(m, a, b) printf("stage msg : %s :\n  "m"\n", __FILE__, a, b)

// DEBUG macros
#ifdef DEBUG
#define PRINT_DEBUG(m)         printf("\rstage debug : %s %s\n  "m"\n", \
                                     __FILE__, __FUNCTION__)
#define PRINT_DEBUG1(m, a)     printf("\rstage debug : %s %s\n  "m"\n", \
                                     __FILE__, __FUNCTION__, a)
#define PRINT_DEBUG2(m, a, b)  printf("\rstage debug : %s %s\n  "m"\n", \
                                     __FILE__, __FUNCTION__, a, b)
#define PRINT_DEBUG3(m, a, b, c) printf("\rstage debug : %s %s\n  "m"\n", \
                                     __FILE__, __FUNCTION__, a, b, c)
#else
#define PRINT_DEBUG(m)
#define PRINT_DEBUG1(m, a)
#define PRINT_DEBUG2(m, a, b)
#define PRINT_DEBUG3(m, a, b, c)
#endif



// color type
typedef struct
{
  unsigned short red, green, blue;
} StageColor;


// Shapes for entities
enum StageShape
{
  ShapeNone = 0,
  ShapeCircle,
  ShapeRect
};

// definition of stage object type codes
// similar to player types, but not exactly, as different robots
// can appear to stage as identical position devices, for example.
enum StageType
{
  NullType = 0,
  WallType,
  PlayerType, 
  MiscType, 
  RectRobotType,
  RoundRobotType,
  SonarType,
  LaserTurretType,
  VisionType,
  PtzType,
  BoxType,
  LaserBeaconType,
  LBDType, // Laser Beacon Detector
  VisionBeaconType,
//VBDType // Vision Beacon Detector?
  GripperType, 
  GpsType,
  PuckType,
  BroadcastType,
  AudioType,
  SpeechType,
  TruthType,
  OccupancyType,
  IDARType, // HRL's Infrared Data And Ranging turret
  DescartesType, // HRL's customized Descartes robot platform
  OmniPositionType,
  MoteType
};


// Possible laser return values
enum LaserReturn
{
  LaserTransparent = 0,
  LaserReflect, 
  LaserBright1,
  LaserBright2,
  LaserBright3,
  LaserBright4
};


// Possible IDAR return values
enum IDARReturn
{
  IDARTransparent=0,
  IDARReflect,
  IDARReceive
};

#endif










