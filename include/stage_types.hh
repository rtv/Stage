///////////////////////////////////////////////////////////////////////////
//
// File: stage_types.hh
// Author: Andrew Howard
// Date: 12 Mar 2001
// Desc: Types and macros for stage
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/include/stage_types.hh,v $
//  $Author: vaughan $
//  $Revision: 1.14 $
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

// Message macros
//
#define PRINT_MSG(m) printf("stage msg : %s :\n  "m"\n", __FILE__)
#define PRINT_MSG1(m, a) printf("stage msg : %s :\n  "m"\n", __FILE__, a)
#define PRINT_MSG2(m, a, b) printf("stage msg : %s :\n  "m"\n", __FILE__, a, b)

// color type
typedef struct
{
  unsigned short red, green, blue;
} StageColor;

// definition of stage object type codes
// similar to player types, but not exactly, as different robots
// can appear to stage as identical position devices, for example.
enum StageType {
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
DescartesType // HRL's customized Descarte robot platform
};

#endif
