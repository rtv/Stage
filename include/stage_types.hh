/*
 *  Stage : a multi-robot simulator.
 *  Copyright (C) 2001, 2002 Richard Vaughan, Andrew Howard and Brian Gerkey.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */
/*
 * Desc: Shared types, constants, etc
 * Author: Andrew Howard
 * Date: 12 Mar 2001
 * CVS: $Id: stage_types.hh,v 1.29 2002-08-21 01:30:49 gerkey Exp $
 */

#ifndef STAGE_TYPES_HH
#define STAGE_TYPES_HH

#if HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stddef.h>
#include <assert.h>
#include <math.h>

// get player's message structures
#include "player.h" 

///////////////////////////////////////////////////////////////////////////
// CONSTANTS
const int DEFAULT_POSE_PORT = 6601;

// we usually use 1 or 2, so this should be plenty
const int MAX_POSE_CONNECTIONS = 100; 

// this is the root filename for stage devices
// this is appended with the user name and instance number
// so in practice you get something like /tmp/stageIO.vaughan.0
// currently only the zero instance is supported in player
#define IOFILENAME "/tmp/stageIO"


///////////////////////////////////////////////////////////////////////////
// Global variables

// raising this causes Stage to exit the main loop and die nicely
// exception throwing would be better style...
extern bool quit;


///////////////////////////////////////////////////////////////////////////
// Some useful macros

// Determine size of array
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
#define TWOPI 2 * M_PI
#endif

#define STAGE_SYNC 0
#define STAGE_ASYNC 1

// Convert radians to degrees
#define RTOD(r) ((r) * 180 / M_PI)

// Convert degrees to radians
#define DTOR(d) ((d) * M_PI / 180)

// Normalize angle to domain -pi, pi
#define NORMALIZE(z) atan2(sin(z), cos(z))

#define ASSERT(m) assert(m)

// Error macros
#define PRINT_ERR(m)         printf("\nstage error : %s : "m"\n", \
                                    __FILE__)
#define PRINT_ERR1(m, a)     printf("\nstage error : %s : "m"\n", \
                                    __FILE__, a)
#define PRINT_ERR2(m, a, b)  printf("\nstage error : %s : "m"\n", \
                                    __FILE__, a, b)

// Warning macros
#define PRINT_WARN(m)         printf("\nstage warning : %s %s "m"\n", \
                                     __FILE__, __FUNCTION__)
#define PRINT_WARN1(m, a)     printf("\nstage warning : %s %s "m"\n", \
                                     __FILE__, __FUNCTION__, a)
#define PRINT_WARN2(m, a, b)  printf("\nstage warning : %s %s "m"\n", \
                                     __FILE__, __FUNCTION__, a, b)
#define PRINT_WARN3(m, a, b, c) printf("\nstage warning : %s %s "m"\n", \
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
#define PRINT_DEBUG4(m, a, b, c, d) printf("\rstage debug : %s %s\n  "m"\n", \
                                     __FILE__, __FUNCTION__, a, b, c, d)
#else
#define PRINT_DEBUG(m)
#define PRINT_DEBUG1(m, a)
#define PRINT_DEBUG2(m, a, b)
#define PRINT_DEBUG3(m, a, b, c)
#define PRINT_DEBUG4(m, a, b, c, d)
#endif


///////////////////////////////////////////////////////////////////////////
// Useful stage types

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
  PositionType,
  SonarType,
  LaserTurretType,
  VisionType,
  PtzType,
  BoxType,
  LaserBeaconType,
  LBDType, // Laser Beacon Detector
  VisionBeaconType,
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
  MoteType,
  BpsType,
  IDARTurretType,
  NUMBER_OF_STAGE_TYPES // THIS MUST BE LAST - put yours before this.
};


// Possible laser return values
enum LaserReturn
{
  LaserTransparent = 0,
  LaserVisible, 
  LaserBright,
};


// Possible IDAR return values
enum IDARReturn
{
  IDARTransparent=0,
  IDARReflect,
  IDARReceive
};

#endif










