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
 * Author: Andrew Howard, Richard Vaughan
 * Date: 12 Mar 2001
 * CVS: $Id: stage_types.hh,v 1.9.2.1 2002-12-10 00:28:18 rtv Exp $
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
// CONSTANTS - please don't use #defines here!

// this is the default port bound by Stage's server
const int DEFAULT_POSE_PORT = 6601;

// we usually use 1 or 2, so this should be plenty
// TODO - make this dynamic
const int MAX_POSE_CONNECTIONS = 100; 

// this is the root filename for stage devices
// this is appended with the user name and instance number
// so in practice you get something like /tmp/stageIO.vaughan.0
// currently only the zero instance is supported

// this line causes the compiler to complain about multiple
// definitions of IOFILENAME. why? - rtv 
// const char* IOFILENAME = "/tmp/stageIO";
// i'll stick to the macro...
#define IOFILENAME "/tmp/stageIO"

// the max size of an entity's worldfile token
const size_t STAGE_TOKEN_MAX = 48;

const size_t STAGE_WORLD_FILENAME_MAX = 512;

///////////////////////////////////////////////////////////////////////////
// Global variables

// raising this causes Stage to exit the main loop and die nicely
// exception throwing would be better style...
extern bool quit;



///////////////////////////////////////////////////////////////////////////
// Useful stage types

// ENTITY TYPE DEFINITIONS /////////////////////////////////////////////////////////

// a unique id for each entity equal to its position in the world's array
typedef int stage_id_t;

// Color type
typedef uint32_t StageColor;

/* StageType is deprecated and has been removed */

// these properties can be set with the SetProperty() method

const int ENTITY_FIRST_PROPERTY = 1;

enum EntityProperty
{
  PropParent = ENTITY_FIRST_PROPERTY, 
  PropSizeX, 
  PropSizeY, 
  PropPoseX, 
  PropPoseY, 
  PropPoseTh, 
  PropOriginX, 
  PropOriginY, 
  PropName,
  PropColor, 
  PropShape, 
  PropLaserReturn,
  PropSonarReturn,
  PropIdarReturn, 
  PropObstacleReturn, 
  PropVisionReturn, 
  PropPuckReturn,
  PropPlayerId,
  PropPlayerSubscriptions,
  PropCommand,
  PropData,
  PropConfig,
  PropReply,
  ENTITY_LAST_PROPERTY // this must be the final property - we use it
 // as a count of the number of properties.
};

// (currently) static memory allocation for getting and setting properties
//const int MAX_NUM_PROPERTIES = 30;
const int MAX_PROPERTY_DATA_LEN = 20000;


class CEntity;
class CWorld;
class LibraryItem;
// pointer to a function that returns a new  entity
// given a world and a parent
typedef CEntity*(*CreatorFunctionPtr)( LibraryItem *libit, CWorld *world, CEntity *parent );

typedef struct libitem
{
  const char* token;
  //StageType type;
  const char* colorstr;
  CreatorFunctionPtr fp;
} libitem_t;


// PROPERTY DEFINITIONS ///////////////////////////////////////////////

// Shapes for entities
enum StageShape
{
  ShapeNone = 0,
  ShapeCircle,
  ShapeRect
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

// Possible Gripper return values
enum GripperReturn
{
  GripperDisabled = 0,
  GripperEnabled
};

// any integer value other than this is a valid fiducial ID
const int FiducialNone = 0;

// image types ////////////////////////////////////////////////////////

unsigned int RGB( int r, int g, int b );

typedef struct Point
{
  int x, y;
};

typedef struct Rect
{
  int toplx, toply, toprx, topry, botlx, botly, botrx, botry;
};


///////////////////////////////////////////////////////////////////////////
// Some useful macros

// Determine size of array
#ifndef ARRAYSIZE
#define ARRAYSIZE(x) (int) (sizeof(x) / sizeof(x[0]))
#endif

// size of char arrays for hostnames
#define HOSTNAME_SIZE 256
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

// these are lifted from ahoward's rtk2 library code
// Append an item to a linked list
#define STAGE_LIST_APPEND(head, item) \
item->prev = head;\
item->next = NULL;\
if (head == NULL)\
    head = item;\
else\
{\
    while (item->prev->next)\
       item->prev = item->prev->next;\
    item->prev->next = item;\
}

// Remove an item from a linked list
#define STAGE_LIST_REMOVE(head, item) \
if (item->prev)\
    item->prev->next = item->next;\
if (item->next)\
    item->next->prev = item->prev;\
if (item == head)\
    head = item->next;

#endif










