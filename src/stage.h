#ifndef STG_H
#define STG_H
/*
 *  Stage : a multi-robot simulator.  
 * 
 *  Copyright (C) 2001-2003 Richard Vaughan, Andrew Howard and Brian
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
 * Author: Richard Vaughan vaughan@hrl.com 
 * Date: 1 June 2003
 *
 * CVS: $Id: stage.h,v 1.20 2003-10-13 08:37:00 rtv Exp $
 */

#ifdef __cplusplus
 extern "C" {
#endif 

   // for now I'm specifying packets in the native types. I may change
   // this in the future for cross-platform networks, but right now I 
   // want to keep things simple until everything works.
   //#include <stdint.h> // for the integer types (uint32_t, etc)
   
#include <stdio.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h> // for portable int types eg. uint32_t
#include <sys/types.h>
#include <sys/time.h>

#include "replace.h"

typedef struct timeval stg_timeval_t;
   
   // global stage configs
#define STG_TOKEN_MAX 32
#define STG_LISTENQ  128
#define STG_DEFAULT_SERVER_PORT 6601  // move to configure.in?
#define STG_HOSTNAME_MAX  128
#define STG_MAX_CONNECTIONS 128

#define STG_SERVER_GREETING 42   
#define STG_CLIENT_GREETING 142

   // client specifies type-of-service when connecting to the server
   typedef enum
     {
       STG_TOS_REQUESTREPLY=0,
       STG_TOS_SUBSCRIPTION
     } stg_tos_t;
   
   
   // all properties have unique id numbers and must be listed here
   typedef enum
     {
       // server properties
       STG_SERVER_TIME=1,
       STG_SERVER_WORLD_COUNT,
       STG_SERVER_CLIENT_COUNT,
       STG_SERVER_CREATE_WORLD,
       // world properties
       STG_WORLD_TIME,
       STG_WORLD_MODEL_COUNT,
       STG_WORLD_CREATE_MODEL,
       STG_WORLD_DESTROY,
       // model properties
       STG_MOD_DESTROY,
       STG_MOD_TIME,
       STG_MOD_CIRCLES,
       STG_MOD_COLOR,
       STG_MOD_COMMAND,
       STG_MOD_DATA,
       STG_MOD_GEOM,
       STG_MOD_IDARRETURN,
       STG_MOD_LASERRETURN,
       STG_MOD_NAME,
       STG_MOD_OBSTACLERETURN,
       STG_MOD_ORIGIN,
       STG_MOD_PARENT, 
       STG_MOD_PLAYERID,
       STG_MOD_POSE,
       STG_MOD_POWER,
       STG_MOD_PPM,
       STG_MOD_PUCKRETURN,
       STG_MOD_RANGEBOUNDS, 
       STG_MOD_INTERVAL,
       STG_MOD_RECTS,
       STG_MOD_SIZE,
       STG_MOD_SONARRETURN,
       STG_MOD_VELOCITY,
       STG_MOD_VISIONRETURN,
       STG_MOD_VOLTAGE,
       STG_MOD_RANGERS,
       STG_MOD_LASER_DATA,
       STG_MOD_NEIGHBORS,
       STG_MOD_NEIGHBORRETURN, // if non-zero, show up in neighbor sensor
       STG_MOD_NEIGHBORBOUNDS, // range bounds of neighbor sensor
       STG_MOD_BLINKENLIGHT,  // light blinking rate
       STG_MOD_NOSE,
       STG_MOD_LOS_MSG,
       STG_MOD_LOS_MSG_CONSUME,
       STG_MOD_MOUSE_MODE,
       STG_MOD_BORDER,        // if non-zero, add a bounding rectangle
       STG_MOD_MATRIX_RENDER, // if non-zero, render in the matrix
       STG_MOD_CREATE_WORLD,
       STG_MOD_DESTROY_WORLD,
       STG_MOD_WORLD_SIZE,
       STG_MOD_WORLD_GUI, 
       // remove?
       //STG_MOD_POSITION_ORIGIN, // see position.cc
       //STG_MOD_POSITION_ODOM,
       //STG_MOD_POSITION_MODE,
       //STG_MOD_POSITION_STEER,

       STG_MESSAGE_COUNT // THIS MUST BE THE LAST ENTRY
     } stg_prop_id_t;
  
// PROPERTY DEFINITIONS ///////////////////////////////////////////////

// a unique id for each entity equal to its position in the world's array
typedef int stg_id_t;

// this packet is exchanged as a handshake when connecting to Stage
typedef struct
{
  int code;
  pid_t pid;
} stg_greeting_t;


// used for specifying 3 axis positions
typedef struct
{
  double x, y, a;
} stg_pose_t;

typedef stg_pose_t stg_velocity_t; // same thing, different name

// used for rectangular sizes
typedef struct 
{
  double x, y;
} stg_size_t;

// Color type
typedef uint32_t stg_color_t;

// Possible laser return values
typedef enum 
  {
    LaserTransparent = 0,
    LaserVisible, 
    LaserBright,
  } stg_laser_return_t;

// Possible IDAR return values
typedef enum 
  {
    IDARTransparent=0,
    IDARReflect,
    IDARReceive
  } stg_idar_return_t;


// a number of milliseconds, used for example as the blinkenlight interval
#define STG_LIGHT_ON UINT_MAX
#define STG_LIGHT_OFF 0

typedef int stg_interval_ms_t;

typedef struct
{
  int enable;
  stg_interval_ms_t period_ms;
} stg_blinkenlight_t;

// Possible Gripper return values
typedef enum 
  {
    GripperDisabled = 0,
    GripperEnabled
  } stg_gripper_return_t;

typedef int stg_neighbor_return_t;

// any integer value other than this is a valid fiducial ID
// TODO - fix this up
#define FiducialNone 0

typedef enum
  {
    STG_NOREPLY = 0,
    STG_WANTREPLY,
    STG_ISREPLY
  } stg_reply_mode_t;

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
  double range;
  double error;
} stg_ranger_t;

// an individual range reading
typedef double stg_range_t;

typedef struct
{
  stg_id_t id;
  double range;
  double bearing;
  double orientation;
  stg_size_t size;
} stg_neighbor_t;

typedef struct
{
  double range;
  double reflectance;
} stg_laser_sample_t;

#define STG_LASER_SAMPLES_MAX 361

typedef struct
{
  stg_pose_t pose;
  stg_size_t size;
  double angle_min;
  double angle_max;
  double range_max;
  double range_min;
  double sample_count;
  stg_laser_sample_t samples[STG_LASER_SAMPLES_MAX];
} stg_laser_data_t;

typedef struct
{
  int id;
  char name[STG_TOKEN_MAX+6]; // extra space for an instance number
  double width, height; // the dimensions of the world in meters
  double resolution; // the size of a pixel in meters
} stg_world_create_t;

// image types ////////////////////////////////////////////////////////

unsigned int RGB( int r, int g, int b );

typedef struct 
{
  int x, y;
} stg_point_t;

typedef struct
{
  //int toplx, toply, toprx, topry, botlx, botly, botrx, botry;
  double x, y, w, h;
} stg_rect_t;

typedef struct
{
  double x, y, a, w, h;
} stg_rotrect_t; // rotated rectangle

typedef struct
{
  int toplx, toply, toprx, topry, botlx, botly, botrx, botry;
} stg_corners_t;

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

typedef enum {
  STG_NOOP = 0,
  STG_COMMAND,
  STG_ACK,
  STG_NACK,
  STG_SUBSCRIBE,
  STG_UNSUBSCRIBE,
  STG_SET,
  STG_GET,
  STG_SETGET,
  STG_GETSET
} stg_prop_action_t;

// this allows us to set a property for an entity
// pretty much any data member of an entity can be set
// using the entity's ::SetProperty() method
typedef struct
{
  stg_prop_id_t property; // identify the property
  double timestamp;
  int id; // identify the entity or world if appropriate
  stg_prop_action_t action; // determines property set, get, or both
  size_t len; // the property uses this much data (to follow)
  char data[0]; // allows us named entry to the end of the struct
} stg_property_t;


// a client that receives this packet should create a new entity
// and return a single int identifier
typedef struct
{
  stg_id_t id; // Stage chooses an ID on SET and returns it in GET
  stg_id_t parent_id; // -1 specifies a top-level model
  char name[STG_TOKEN_MAX]; // a decsriptive name
  //  char token[STG_TOKEN_MAX]; // the token used in the world file
  char color[STG_TOKEN_MAX];
} stg_entity_create_t;


typedef enum
  { STG_POSITION_CONTROL_VELOCITY, STG_POSITION_CONTROL_POSITION }
stg_position_control_mode_t;

typedef enum
  { STG_POSITION_STEER_DIFFERENTIAL, STG_POSITION_STEER_INDEPENDENT }
stg_position_steer_mode_t;

typedef struct
{
  double x, y, a;
  double xdot, ydot, adot;
  // double xdotdot, ydotdot, adotdot; // useful?
  char mask; // each of the first 6 bits controls which of the commands we are using
} stg_position_data_t;

typedef stg_position_data_t stg_position_cmd_t;

typedef struct
{
  int id; // the object we're subscribing to
  stg_prop_id_t prop; // the property we're subscribing to
} stg_subscription_t;

typedef struct {
  char host[STG_HOSTNAME_MAX];
  int port;
  stg_timeval_t time;
  struct pollfd pollfd;
  stg_tos_t tos;

  //stg_subscription_t* subs;
  //int num_subs; 
} stg_client_t;

typedef struct
{
  stg_id_t stage_id;
  char name[STG_TOKEN_MAX];
} stg_name_id_t;  




//////////////////////////////////////////////////////////////////////////
/*
 * FUNCTION DEFINITIONS
*/ 
// print a human-readable property on stdout
void stg_property_print( stg_property_t* prop );
stg_property_t* stg_property_read( stg_client_t* cli );
ssize_t stg_property_write( stg_client_t* cli, stg_property_t* prop );
stg_property_t* stg_property_create( void );
stg_property_t* stg_property_attach_data( stg_property_t* prop, void* data, size_t len );
void stg_property_free( stg_property_t* prop );
void stg_property_fprint( FILE* fp, stg_property_t* prop );

stg_client_t* stg_client_create( char* host, int port, stg_tos_t tos );

void stg_client_free( stg_client_t* cli );
size_t stg_packet_read_fd( int fd, void* buf, size_t len );
stg_property_t* stg_property_read_fd( int fd );
ssize_t stg_property_write_fd( int fd, stg_property_t* prop );
void stg_catch_pipe( int signo );
const char* stg_property_string( stg_prop_id_t id );

int stg_property_subscribe( stg_client_t* cli, stg_subscription_t* sub );



// set a property of the model with the given id. 
// returns 0 on success, else -1.
int stg_set_property( stg_client_t* cli,
		      int id, 
		      stg_prop_id_t type,
		      void* data, 
		      size_t len );

// gets the requested data from the server, allocating memory for the packet.
// caller must free() the data
// returns 0 on success, else -1.
int stg_get_property( stg_client_t* cli,
		      int id, 
		      stg_prop_id_t type,
		      void **data, 
		      size_t *len );

// sets a property, then gets the same property. This is effectively
// an atomic operation in stage.
int stg_setget_property( stg_client_t* cli,
			 int id, 
			 stg_prop_id_t type,
			 void *set_data,
			 size_t set_len,
			 void **get_data, 
			 size_t *get_len );

// gets a property, then sets the same property
// (useful if you want to reset the property to it's initial state later).
int stg_getset_property( stg_client_t* cli,
			 int id, 
			 stg_prop_id_t type,
			 void *set_data,
			 size_t set_len,
			 void **get_data, 
			 size_t *get_len );


stg_property_t* stg_send_property( stg_client_t* cli,
				   int id, 
				   stg_prop_id_t type,
				   stg_prop_action_t action,
				   void* data, 
				   size_t len );

// SET/GET PROPERTIES
typedef int stg_nose_t;
typedef int stg_border_t;
typedef int stg_mouse_mode_t;
typedef int stg_matrix_render_t;

// print the values in a message packet
void stg_los_msg_print( stg_los_msg_t* msg );

// Look up the color in a database.  (i.e. transform color name to
// color value).  If the color is not found in the database, a bright
// red color will be returned instead.
stg_color_t stg_lookup_color(const char *name);


///////////////////////////////////////////////////////////////////////////
// Some useful macros

// Determine size of array
#ifndef ARRAYSIZE
#define ARRAYSIZE(x) (int) (sizeof(x) / sizeof(x[0]))
#endif

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
#define PRINT_ERR4(m,a,b,c,d) printf( "\033[41merr\033[0m: "m" (%s %s)\n", a, b, c, __FILE__, __FUNCTION__)

// Warning macros
#define PRINT_WARN(m) printf( "\033[44mwarn\033[0m: "m" (%s %s)\n", __FILE__, __FUNCTION__)
#define PRINT_WARN1(m,a) printf( "\033[44mwarn\033[0m: "m" (%s %s)\n", a, __FILE__, __FUNCTION__)    
#define PRINT_WARN2(m,a,b) printf( "\033[44mwarn\033[0m: "m" (%s %s)\n", a, b, __FILE__, __FUNCTION__) 
#define PRINT_WARN3(m,a,b,c) printf( "\033[44mwarn\033[0m: "m" (%s %s)\n", a, b, c, __FILE__, __FUNCTION__)
#define PRINT_WARN4(m,a,b,c,d) printf( "\033[44mwarn\033[0m: "m" (%s %s)\n", a, b, c, __FILE__, __FUNCTION__)

// Message macros
#ifdef DEBUG
#define PRINT_MSG(m) printf( "stage: "m" (%s %s)\n", __FILE__, __FUNCTION__)
#define PRINT_MSG1(m,a) printf( "stage: "m" (%s %s)\n", a, __FILE__, __FUNCTION__)    
#define PRINT_MSG2(m,a,b) printf( "stage: "m" (%s %s)\n", a, b, __FILE__, __FUNCTION__) 
#define PRINT_MSG3(m,a,b,c) printf( "stage: "m" (%s %s)\n", a, b, c, __FILE__, __FUNCTION__)
#define PRINT_MSG4(m,a,b,c,d) printf( "stage: "m" (%s %s)\n", a, b, c, d, __FILE__, __FUNCTION__)
#else
#define PRINT_MSG(m) printf( "stage: "m"\n" )
#define PRINT_MSG1(m,a) printf( "stage: "m"\n", a)
#define PRINT_MSG2(m,a,b) printf( "stage: "m"\n,", a, b )
#define PRINT_MSG3(m,a,b,c) printf( "stage: "m"\n", a, b, c )
#define PRINT_MSG4(m,a,b,c,d) printf( "stage: "m"\n", a, b, c, d )
#endif

// DEBUG macros
#ifdef DEBUG
#define PRINT_DEBUG(m) printf( "debug: "m" (%s %s)\n", __FILE__, __FUNCTION__)
#define PRINT_DEBUG1(m,a) printf( "debug: "m" (%s %s)\n", a, __FILE__, __FUNCTION__)    
#define PRINT_DEBUG2(m,a,b) printf( "debug: "m" (%s %s)\n", a, b, __FILE__, __FUNCTION__) 
#define PRINT_DEBUG3(m,a,b,c) printf( "debug: "m" (%s %s)\n", a, b, c, __FILE__, __FUNCTION__)
#define PRINT_DEBUG4(m,a,b,c,d) printf( "debug: "m" (%s %s)\n", a, b, c ,d, __FILE__, __FUNCTION__)
#else
#define PRINT_DEBUG(m)
#define PRINT_DEBUG1(m,a)
#define PRINT_DEBUG2(m,a,b)
#define PRINT_DEBUG3(m,a,b,c)
#define PRINT_DEBUG4(m,a,b,c,d)
#endif

#ifdef __cplusplus
  }
#endif 



#endif
