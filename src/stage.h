#ifndef STG_H
#define STG_H
/*
 *  Stage : a multi-robot simulator.  
 * 
 *  Copyright (C) 2001-2004 Richard Vaughan, Andrew Howard and Brian
 *  Gerkey for the Player/Stage Project
 *  http://playerstage.sourceforge.net
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

/* File: stage.h
 * Desc: packet types and functions for interfacing with the Stage server
 * Author: Richard Vaughan vaughan@sfu.ca 
 * Date: 1 June 2003
 *
 * CVS: $Id: stage.h,v 1.29 2004-04-24 01:21:28 rtv Exp $
 */

#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h> // for portable int types eg. uint32_t
#include <sys/types.h>
#include <sys/time.h>
#include <assert.h>

#include <glib.h>


#ifdef __cplusplus
 extern "C" {
#endif 

#include "config.h"
#include "replace.h"

#define STG_DEFAULT_SERVER_PORT 6600
#define STG_DEFAULT_SERVER_HOST "localhost"

#define STG_ID_STRING_MAX 64
#define STG_TOKEN_MAX 64
#define STG_SERVER_GREETING 42
#define STG_CLIENT_GREETING 44

// TODO - override from command line and world config request
#define STG_DEFAULT_WORLD_INTERVAL 0.1 // 10Hz

#define STG_HELLO 'S'

// Movement masks for figures  - these should match their RTK equivalents
#define STG_MOVE_TRANS (1 << 0)
#define STG_MOVE_ROT   (1 << 1)
#define STG_MOVE_SCALE (1 << 2)

// for now I'm specifying packets in the native types. I may change
// this in the future for cross-platform networks, but right now I 
// want to keep things simple until everything works.

// every packet sent between Stage and client has a header that
// specifies the type of message to follow. The comment shows the
// payload that follows each type of message.

typedef enum
  {
    STG_PROP_TIME, // double - time since startup in seconds
    STG_PROP_CIRCLES,
    STG_PROP_COLOR,
    STG_PROP_GEOM,
    STG_PROP_NAME, // ?
    STG_PROP_PARENT, 
    STG_PROP_PLAYERID,
    STG_PROP_POSE,
    STG_PROP_POWER,
    STG_PROP_PPM,
    STG_PROP_PUCKRETURN,
    STG_PROP_RANGEBOUNDS, 
    STG_PROP_INTERVAL,
    STG_PROP_RECTS,
    STG_PROP_LINES,
    STG_PROP_SIZE,
    STG_PROP_VELOCITY, 
    STG_PROP_LASERRETURN,
    STG_PROP_ORIGIN,
    STG_PROP_OBSTACLERETURN,
    STG_PROP_VISIONRETURN,
    STG_PROP_SONARRETURN, 
    STG_PROP_NEIGHBORRETURN,
    STG_PROP_VOLTAGE,
    STG_PROP_RANGERDATA,
    STG_PROP_RANGERCONFIG,
    STG_PROP_LASERDATA,
    STG_PROP_LASERCONFIG,
    STG_PROP_NEIGHBORS,
    STG_PROP_NEIGHBORBOUNDS, // range bounds of neighbor sensor
    STG_PROP_BLINKENLIGHT,  // light blinking rate
    STG_PROP_NOSE,
    STG_PROP_GRID,
    STG_PROP_LOSMSG,
    STG_PROP_LOSMSGCONSUME,
    STG_PROP_MOVEMASK,
    STG_PROP_BOUNDARY,        // if non-zero, add a bounding rectangle
    STG_PROP_MATRIXRENDER, // if non-zero, render in the matrix
    STG_PROP_COUNT // this must be the last entry
} stg_prop_type_t;

   
typedef uint16_t stg_msg_type_t;
   
#define STG_MSG_MASK_MAJOR 0xFF00 // major type is in first byte
#define STG_MSG_MASK_MINOR 0x00FF // minor type is in second byte
   
#define STG_MSG_SERVER 0x0100
#define STG_MSG_WORLD  0x0200
#define STG_MSG_MODEL  0x0300
#define STG_MSG_CLIENT 0x0400

#define STG_MSG_SERVER_SUBSCRIBE        (STG_MSG_SERVER | 1)
#define STG_MSG_SERVER_UNSUBSCRIBE      (STG_MSG_SERVER | 2)
#define STG_MSG_SERVER_WORLDCREATE      (STG_MSG_SERVER | 3)
#define STG_MSG_SERVER_WORLDDESTROY     (STG_MSG_SERVER | 4)

#define STG_MSG_WORLD_PAUSE             (STG_MSG_WORLD | 1)
#define STG_MSG_WORLD_RESUME            (STG_MSG_WORLD | 2)
#define STG_MSG_WORLD_RESTART           (STG_MSG_WORLD | 3)
#define STG_MSG_WORLD_SAVE              (STG_MSG_WORLD | 4) 
#define STG_MSG_WORLD_LOAD              (STG_MSG_WORLD | 5)
#define STG_MSG_WORLD_MODELCREATE       (STG_MSG_WORLD | 6)
#define STG_MSG_WORLD_MODELDESTROY      (STG_MSG_WORLD | 7)
#define STG_MSG_WORLD_INTERVALSIM       (STG_MSG_WORLD | 8)
#define STG_MSG_WORLD_INTERVALREAL      (STG_MSG_WORLD | 9)

#define STG_MSG_MODEL_PROPERTY          (STG_MSG_MODEL | 1)
#define STG_MSG_MODEL_SAVE              (STG_MSG_MODEL | 2)
#define STG_MSG_MODEL_LOAD              (STG_MSG_MODEL | 3)
    
#define STG_MSG_CLIENT_WORLDCREATEREPLY (STG_MSG_CLIENT | 1)
#define STG_MSG_CLIENT_MODELCREATEREPLY (STG_MSG_CLIENT | 2)
#define STG_MSG_CLIENT_PROPERTY         (STG_MSG_CLIENT | 3)
#define STG_MSG_CLIENT_CLOCK            (STG_MSG_CLIENT | 4)


typedef double stg_time_t;

typedef int stg_id_t;

typedef struct 
{
  double x, y;
} stg_size_t;
   
// each message in a packet starts with a standard message
// header. some messages may have no payload at all
typedef struct
{
  stg_msg_type_t type;
  size_t payload_len;
  double timestamp;
  char payload[0]; // named access to the end of the struct
} stg_msg_t;

typedef struct
{  
  stg_id_t world;
  stg_id_t model;
  stg_id_t prop;
  double timestamp; // the time at which this property was filled
  size_t datalen; // data size
  char data[]; // the data follows
} stg_prop_t;

typedef struct
{
  stg_id_t world;
  double simtime;
} stg_clock_t;

typedef struct
{
  stg_id_t world;
  stg_id_t model;
  stg_id_t prop;
  stg_time_t interval; // requested time between updates in seconds
} stg_sub_t;

typedef struct
{
  stg_id_t world;
  stg_id_t model;
  stg_id_t prop;
} stg_unsub_t;

typedef struct
{
  stg_id_t world;
  char token[ STG_TOKEN_MAX ];
} stg_createmodel_t;

typedef struct
{
  stg_id_t world;
  stg_id_t model;
} stg_destroymodel_t;

typedef struct
{
  char token[ STG_TOKEN_MAX ];
  stg_time_t interval_sim;
  stg_time_t interval_real;
  int ppm;
  //stg_size_t size;
} stg_createworld_t;

typedef struct
{
  stg_id_t world;
} stg_destroyworld_t;
  


stg_msg_t* stg_msg_create( stg_msg_type_t type, void* data, size_t len );
stg_msg_t* stg_msg_append( stg_msg_t* msg, void* data, size_t len );
void stg_msg_destroy( stg_msg_t* msg );


typedef struct
 {
   gpointer function;
   gpointer data; // passed into the function along with the property ptr
 } stg_callback_prop_t;


typedef struct 
{
  stg_id_t id;
  void* data;
  size_t len;
} stg_property_t;

stg_property_t* prop_create( stg_id_t id, void* data, size_t len );
void prop_destroy( stg_property_t* prop );


// this packet is exchanged as a handshake when connecting to Stage
typedef struct
{
  int code; // must be STG_SERVER_GREETING
} stg_greeting_t;

// after connection, the server sends one of these as the first packet.
typedef struct
{
  int code; // should be STG_CLIENT_GREETING
  char id_string[STG_ID_STRING_MAX]; // a string describing Stage
  int vmajor, vminor, vmicro; // Stage's version number.
} stg_connect_reply_t;



// PROPERTY-SPECIFIC DEFINITIONS -------------------------------------------

// COLOR ----------------------------------------------------------------

typedef uint32_t stg_color_t;

// Look up the color in the X11 database.  (i.e. transform color name to
// color value).  If the color is not found in the database, a bright
// red color will be returned instead.
stg_color_t stg_lookup_color(const char *name);

// POSITION -------------------------------------------------------------

// used for specifying 3 axis positions
typedef struct
{
  double x, y, a;
} stg_pose_t;

typedef enum
  { STG_POSITION_CONTROL_VELOCITY, STG_POSITION_CONTROL_POSITION }
stg_position_control_mode_t;

typedef enum
  { STG_POSITION_STEER_DIFFERENTIAL, STG_POSITION_STEER_INDEPENDENT }
stg_position_steer_mode_t;


// VELOCITY ------------------------------------------------------------

typedef stg_pose_t stg_velocity_t;

// BLINKENLIGHT ------------------------------------------------------------

// a number of milliseconds, used for example as the blinkenlight interval
#define STG_LIGHT_ON UINT_MAX
#define STG_LIGHT_OFF 0

typedef int stg_interval_ms_t;

typedef struct
{
  int enable;
  stg_interval_ms_t period_ms;
} stg_blinkenlight_t;

// GRIPPER ------------------------------------------------------------

// Possible Gripper return values
typedef enum 
  {
    GripperDisabled = 0,
    GripperEnabled
  } stg_gripper_return_t;

// FIDUCIAL ------------------------------------------------------------

typedef int stg_neighbor_return_t;

// any integer value other than this is a valid fiducial ID
// TODO - fix this up
#define FiducialNone 0

/* line-of-sight messaging packet */
#define STG_LOS_MSG_MAX_LEN 32

typedef struct
{
  int id;
  char bytes[STG_LOS_MSG_MAX_LEN];
  size_t len;
  int power;
  //  int consume;
} stg_los_msg_t;

// print the values in a message packet
void stg_los_msg_print( stg_los_msg_t* msg );

typedef struct
{
  stg_id_t id;
  double range;
  double bearing;
  double orientation;
  stg_size_t size;
} stg_neighbor_t;

// RANGER ------------------------------------------------------------

typedef struct
{
  double min;
  double max;
} stg_bounds_t;

typedef struct
{
  stg_pose_t pose;
  stg_size_t size;
  stg_bounds_t bounds_range;
  double fov;
} stg_ranger_config_t;

typedef struct
{
  double range;
  //double error;
} stg_ranger_sample_t;

// RECTS --------------------------------------------------------------

typedef struct
{
  //int toplx, toply, toprx, topry, botlx, botly, botrx, botry;
  double x, y, w, h;
} stg_rect_t;

typedef struct
{
  double x, y, a, w, h;
} stg_rotrect_t; // rotated rectangle

// specify a line from (x1,y1) to (x2,y2), all in meters
typedef struct
{
  double x1, y1, x2, y2;
} stg_line_t;

typedef struct
{
  int toplx, toply, toprx, topry, botlx, botly, botrx, botry;
} stg_corners_t;

// MOVEMASK ---------------------------------------------------------
   
typedef int stg_movemask_t;

typedef int stg_bool_t;

// GUI -------------------------------------------------------------------

// used for loading and saving GUI state
// prefer to expand this single packet with new fields rather
// than create extra GUI packets
typedef struct
{
  char token[ STG_TOKEN_MAX ]; // string identifying the GUI library
  double ppm;
  int width, height;
  double originx, originy;
  char showgrid;
  char showdata;
} stg_gui_config_t;


// an individual range reading
typedef double stg_range_t;

// LASER  ------------------------------------------------------------

// Possible laser return values
typedef enum 
  {
    LaserTransparent = 0,
    LaserVisible, 
    LaserBright,
  } stg_laser_return_t;

typedef struct
{
  uint32_t range; // mm
  char reflectance; 
} stg_laser_sample_t;

typedef struct
{
  stg_pose_t pose;
  stg_size_t size;
  double fov;
  double range_max;
  double range_min;
  int samples;
} stg_laser_config_t;


// end property typedefs -------------------------------------------------


//  FUNCTION DEFINITIONS


// utilities ---------------------------------------------------------------

// normalizes the set [rects] of [num] rectangles, so that they fit
// exactly in a unit square.
void stg_normalize_rects( stg_rotrect_t* rects, int num );

// returns an array of 4 * num_rects stg_line_t's
stg_line_t* stg_rects_to_lines( stg_rotrect_t* rects, int num_rects );
void stg_normalize_lines( stg_line_t* lines, int num );
void stg_scale_lines( stg_line_t* lines, int num, double xscale, double yscale );
void stg_translate_lines( stg_line_t* lines, int num, double xtrans, double ytrans );

// returns the real (wall-clock) time in seconds
stg_time_t stg_timenow( void );

// operations on properties -------------------------------------------------

// allocates a stg_property_t structure
//stg_property_t* prop_create( void );

int stg_property_attach_data( stg_property_t* prop, 
			      void* data, 
			      size_t len );

// deallocate the memory used by a property (uses free(3)).
void stg_property_free( gpointer prop );

// returns a human readable desciption of the property type [id]
const char* stg_property_string( stg_id_t id );

// compose a human readable string describing property [prop]
char* stg_property_sprint( char* str, stg_property_t* prop );

// print a human readable string describing property [prop] on [fp],
// an open file pointer (eg. stdout, stderr).
void stg_property_fprint( FILE* fp, stg_property_t* prop );

// print a human-readable property on stdout
void stg_property_print( stg_property_t* prop );
void stg_property_print_cb( gpointer key, gpointer value, gpointer user );

// attempt to read a packet from [fd]. returns the number of bytes
// read, or -1 on error (and errno is set as per read(2)).
ssize_t stg_fd_packet_read( int fd, void* buf, size_t len );

// attempt to write a packet on [fd]. returns the number of bytes
// written, or -1 on error (and errno is set as per write(2)).
ssize_t stg_fd_packet_write( int fd, void* buf, size_t len );

// read a message from the server. returns a full message - header and
// payload.  This call allocates memory for the message malloc(3), so
// caller should free the data using free(3),realloc(3), etc.  returns
// NULL on error.
stg_msg_t* stg_fd_msg_read( int fd );

// write a message out	  
int stg_fd_msg_write( int fd, stg_msg_t* msg );

// if stage wants to quit, this will return non-zero
int stg_quit_test( void );

// set stage's quit flag
void stg_quit_request( void );


void stg_err( char* err );


// debug ------------------------------------------------------------------

// returns a human readable desciption of the message type
const char* stg_message_string( stg_msg_type_t id );


typedef struct
{
  stg_id_t world;
  stg_id_t model;
  stg_id_t prop; 
} stg_target_t;

void stg_target_error( stg_target_t* tgt, char* errstr );
void stg_target_debug( stg_target_t* tgt, char* errstr );
void stg_target_warn( stg_target_t* tgt, char* errstr );
void stg_target_message( stg_target_t* tgt, char* errstr );

int stg_prop_set( stg_property_t* prop );

void stg_key_free( gpointer key );

void stg_property_destroy( gpointer prop );


// utility

// Some useful macros

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

//#define SUCCESS TRUE
//#define FAIL FALSE

#define MILLION 1000000L
#define BILLION 1000000000.0

#ifndef M_PI
#define M_PI        3.14159265358979323846
#endif

#ifndef TWOPI
#define TWOPI 2.0 * M_PI
#endif

#define STG_HELLO 'S'

// Convert radians to degrees
#ifndef RTOD
  #define RTOD(r) ((r) * 180.0 / M_PI)
#endif

// Convert degrees to radians
#ifndef DTOR
  #define DTOR(d) ((d) * M_PI / 180.0)
#endif
 
// Normalize angle to domain -pi, pi
#ifndef NORMALIZE
  #define NORMALIZE(z) atan2(sin(z), cos(z))
#endif

#define ASSERT(m) assert(m)

// Error macros
#define PRINT_ERR(m) printf( "\033[41merr\033[0m: "m" (%s %s)\n", __FILE__, __FUNCTION__)
#define PRINT_ERR1(m,a) printf( "\033[41merr\033[0m: "m" (%s %s)\n", a, __FILE__, __FUNCTION__)    
#define PRINT_ERR2(m,a,b) printf( "\033[41merr\033[0m: "m" (%s %s)\n", a, b, __FILE__, __FUNCTION__) 
#define PRINT_ERR3(m,a,b,c) printf( "\033[41merr\033[0m: "m" (%s %s)\n", a, b, c, __FILE__, __FUNCTION__)
#define PRINT_ERR4(m,a,b,c,d) printf( "\033[41merr\033[0m: "m" (%s %s)\n", a, b, c, d, __FILE__, __FUNCTION__)
#define PRINT_ERR5(m,a,b,c,d,e) printf( "\033[41merr\033[0m: "m" (%s %s)\n", a, b, c, d, e, __FILE__, __FUNCTION__)

// Warning macros
#define PRINT_WARN(m) printf( "\033[44mwarn\033[0m: "m" (%s %s)\n", __FILE__, __FUNCTION__)
#define PRINT_WARN1(m,a) printf( "\033[44mwarn\033[0m: "m" (%s %s)\n", a, __FILE__, __FUNCTION__)    
#define PRINT_WARN2(m,a,b) printf( "\033[44mwarn\033[0m: "m" (%s %s)\n", a, b, __FILE__, __FUNCTION__) 
#define PRINT_WARN3(m,a,b,c) printf( "\033[44mwarn\033[0m: "m" (%s %s)\n", a, b, c, __FILE__, __FUNCTION__)
#define PRINT_WARN4(m,a,b,c,d) printf( "\033[44mwarn\033[0m: "m" (%s %s)\n", a, b, c, d, __FILE__, __FUNCTION__)
#define PRINT_WARN5(m,a,b,c,d,e) printf( "\033[44mwarn\033[0m: "m" (%s %s)\n", a, b, c, d, e, __FILE__, __FUNCTION__)

// Message macros
#ifdef DEBUG
#define PRINT_MSG(m) printf( "stage: "m" (%s %s)\n", __FILE__, __FUNCTION__)
#define PRINT_MSG1(m,a) printf( "stage: "m" (%s %s)\n", a, __FILE__, __FUNCTION__)    
#define PRINT_MSG2(m,a,b) printf( "stage: "m" (%s %s)\n", a, b, __FILE__, __FUNCTION__) 
#define PRINT_MSG3(m,a,b,c) printf( "stage: "m" (%s %s)\n", a, b, c, __FILE__, __FUNCTION__)
#define PRINT_MSG4(m,a,b,c,d) printf( "stage: "m" (%s %s)\n", a, b, c, d, __FILE__, __FUNCTION__)
#define PRINT_MSG5(m,a,b,c,d,e) printf( "stage: "m" (%s %s)\n", a, b, c, d, e,__FILE__, __FUNCTION__)
#else
#define PRINT_MSG(m) printf( "stage: "m"\n" )
#define PRINT_MSG1(m,a) printf( "stage: "m"\n", a)
#define PRINT_MSG2(m,a,b) printf( "stage: "m"\n,", a, b )
#define PRINT_MSG3(m,a,b,c) printf( "stage: "m"\n", a, b, c )
#define PRINT_MSG4(m,a,b,c,d) printf( "stage: "m"\n", a, b, c, d )
#define PRINT_MSG5(m,a,b,c,d,e) printf( "stage: "m"\n", a, b, c, d, e )
#endif

// DEBUG macros
#ifdef DEBUG
#define PRINT_DEBUG(m) printf( "debug: "m" (%s %s)\n", __FILE__, __FUNCTION__)
#define PRINT_DEBUG1(m,a) printf( "debug: "m" (%s %s)\n", a, __FILE__, __FUNCTION__)    
#define PRINT_DEBUG2(m,a,b) printf( "debug: "m" (%s %s)\n", a, b, __FILE__, __FUNCTION__) 
#define PRINT_DEBUG3(m,a,b,c) printf( "debug: "m" (%s %s)\n", a, b, c, __FILE__, __FUNCTION__)
#define PRINT_DEBUG4(m,a,b,c,d) printf( "debug: "m" (%s %s)\n", a, b, c ,d, __FILE__, __FUNCTION__)
#define PRINT_DEBUG5(m,a,b,c,d,e) printf( "debug: "m" (%s %s)\n", a, b, c ,d, e, __FILE__, __FUNCTION__)
#else
#define PRINT_DEBUG(m)
#define PRINT_DEBUG1(m,a)
#define PRINT_DEBUG2(m,a,b)
#define PRINT_DEBUG3(m,a,b,c)
#define PRINT_DEBUG4(m,a,b,c,d)
#define PRINT_DEBUG5(m,a,b,c,d,e)
#endif

#ifdef __cplusplus
  }
#endif 



#endif
