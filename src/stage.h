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
 * CVS: $Id: stage.h,v 1.24 2003-10-22 07:04:54 rtv Exp $
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

typedef double stg_time_t;
   
// global stage configs
#define STG_ID_STRING "Stage multi-robot simulator."

#define STG_TOKEN_MAX 32
#define STG_HOSTNAME_MAX  128
#define STG_ID_STRING_MAX 64

#define STG_SERVER_GREETING 42   
#define STG_CLIENT_GREETING 142

#define STG_DEFAULT_SERVER_PORT 6601  // move to configure.in?
#define STG_DEFAULT_WORLDFILE "default.world"
#define STG_DEFAULT_HOST "localhost"
   

// ENUMS ---------------------------------------------------------------

// specify the important constants for specifying message types,
// properties, etc.

// every packet sent between Stage and client has a header that
// specifies the type of message to follow. The comment shows the
// payload that follows each type of message
typedef enum
  {
    STG_MSG_UNKNOWN = 0,
    STG_MSG_NOOP, // no payload - just used to update the time
    STG_MSG_PAUSE, // no payload - stop the clock
    STG_MSG_RESUME, // no payload - start the clock
    STG_MSG_PROPERTY,  // stg_property_t plus variable length data
    STG_MSG_PROPERTY_REQ, // stg_property_req_t
    STG_MSG_SAVE, // stg_id_t 
    STG_MSG_LOAD, // stg_id_t
    STG_MSG_SUBSCRIBE, // stg_subscription_t
    STG_MSG_WORLD_CREATE, // stg_world_create_t
    STG_MSG_WORLD_CREATE_REPLY, //stg_world_create_reply_t
    STG_MSG_WORLD_DESTROY, // stg_id_t
    STG_MSG_MODEL_CREATE, // stg_model_create_t
    STG_MSG_MODEL_CREATE_REPLY, // stg_model_create_reply_t
    STG_MSG_MODEL_DESTROY, // stg_id_t
    STG_MSG_COUNT // this must be the last entry
  } stg_msg_type_t;
   
// all properties have unique id numbers and must be listed here

// model properties (TODO - the comment shows the type of each property, if
// appropriate )
typedef enum
  {
    //STG_MOD_NULL, // no payload - getting this just gives you a header
    STG_MOD_TIME, // double - time since startup in seconds
    STG_MOD_CHILDREN, // int - number of active children
    STG_MOD_STATUS, // stg_model_status_t
    STG_MOD_CIRCLES,
    STG_MOD_COLOR,
    STG_MOD_GEOM,
    STG_MOD_NAME,
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
    STG_MOD_VELOCITY, 
    STG_MOD_LASERRETURN,
    STG_MOD_ORIGIN,
    STG_MOD_OBSTACLERETURN,
    STG_MOD_VISIONRETURN,
    STG_MOD_SONARRETURN, 
    STG_MOD_NEIGHBORRETURN,
    STG_MOD_VOLTAGE,
    STG_MOD_RANGERS,
    STG_MOD_LASER_DATA,
    STG_MOD_NEIGHBORS,
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
    
    STG_MOD_PROP_COUNT // THIS MUST BE THE LAST ENTRY
  } stg_prop_id_t;
   
// specifies what reply (if any) is required when setting a property
typedef enum
  { 
    STG_PR_NONE=0, // no reply required
    STG_PR_POST, // reply with the property AFTER setting it
    STG_PR_PRE  // reply with the property BEFORE setting it
  } stg_prop_reply_t;
   
// a client keeps one of these for each model and world it creates 
typedef enum
  {
    STG_MS_UNKNOWN= 0,
    STG_MS_CREATED,
    STG_MS_DESTROYED,
    STG_MS_ERROR_UNKNOWN
  } stg_status_t;
   
// a client records the subscription status for each property of each
// model it creates
typedef enum
  {
    STG_SUB_UKNOWN=0,
    STG_SUB_SUBSCRIBED,
    STG_SUB_UNSUBSCRIBED,
    STG_SUB_ERROR_TIMEOUT,
    STG_SUB_ERROR_BADMATCH,
    STG_SUB_ERROR_UNKNOWN
  } stg_sub_status_t;
   
      
// PROPERTY DEFINITIONS ///////////////////////////////////////////////

// a unique id for each entity equal to its position in the world's array
typedef int stg_id_t;

// this packet is exchanged as a handshake when connecting to Stage
typedef struct
{
  int code; // must be STG_SERVER_GREETING
} stg_greeting_t;

// after connection, the server sends one of these as the first packet.
typedef struct
{
  int code; // will be STG_CLIENT_GREETING
  char id_string[STG_ID_STRING_MAX]; // a string describing Stage
  int vmajor, vminor, vbugfix; // Stage's version number.
} stg_connect_reply_t;

// this packet begins all post-connection comms [type] indicates the
// type of the data that follows
typedef struct
{
  stg_msg_type_t type;
  size_t len;
  char payload[0]; // named access to the following bytes (neat trick).
} stg_header_t;

typedef struct
{
  int key; // a key that is returned in the reply to identify this request
  char name[STG_TOKEN_MAX+6]; // extra space for an instance number
  double width, height; // the dimensions of the world in meters
  double resolution; // the size of a pixel in meters
} stg_world_create_t;

typedef struct
{
  stg_id_t id; // the id of the world, or zero if creation failed
  int key; // a key that is returned in the reply to identify this request
} stg_world_create_reply_t;

// used to request a model's property 
typedef struct
{
  stg_id_t id;
  stg_prop_id_t type;
} stg_property_req_t;

// image types ////////////////////////////////////////////////////////

unsigned int RGB( int r, int g, int b );

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

// basic client, world, model and property structures ------------------
// these are the core strcutures maintained by a client program

// this allows us to set a property for a Stage model
// (CEntity). Pretty much any data member can be set using the
// entity's ::SetProperty() method
typedef struct
{
  stg_id_t id; // identify the model
  stg_prop_id_t type; // identify the property
  stg_prop_reply_t reply; // specifies the kind of reply requested
  //stg_time_t timestamp;
  size_t len; // the property uses this much data (to follow)
  char data[0]; // allows us named entry to the end of the struct
} stg_property_t;



// a client that receives this packet should create a new entity
// and return a single int identifier
typedef struct
{
  stg_id_t world_id; // Stage chooses an ID on SET and returns it in GET
  stg_id_t parent_id; // -1 specifies a top-level model
  int key; // a key that is returned in the reply tp identify this request
  char name[STG_TOKEN_MAX]; // a decsriptive name
  //  char token[STG_TOKEN_MAX]; // the token used in the world file
  char color[STG_TOKEN_MAX];
} stg_model_create_t;

typedef struct
{
  int key; // the key from the creation request
  stg_id_t id; // the id of the created model, or zero if creation failed 
} stg_model_create_reply_t;


typedef struct
{
  int id; // the object concerned
  stg_prop_id_t prop; // the property concerned
  stg_sub_status_t status; // the current status of the subscription
} stg_subscription_t;

struct _stg_client; // foward declare

typedef struct
{
  stg_id_t id;
  char name[STG_TOKEN_MAX];
  int section; // worldfile section
  int key;

  struct _stg_client* client;
  stg_status_t status;
  stg_property_t* props[STG_MOD_PROP_COUNT];
  stg_sub_status_t subs[STG_MOD_PROP_COUNT];   
} stg_model_t;  


typedef struct
{
  stg_id_t id; // Stage's identifier for this world
  char name[STG_TOKEN_MAX];
  stg_time_t time; // the latest time received from the server
  int key; // a user-defined key supplied when creating the world
  stg_status_t status; // current status of the world
} stg_world_t;

typedef struct _stg_client 
{
  char host[STG_HOSTNAME_MAX];
  int port; 

  struct pollfd pollfd; // asynchronous channel for subscriptions
  struct pollfd pollfd_sync; // synchronous channel (remove?)
  
  stg_world_t* world; // a single world (might extend this to multiple
		      // worlds one day)
  stg_model_t** models; // an array of model pointers
  int model_count; // number of models in the array
  
} stg_client_t;


//////////////////////////////////////////////////////////////////////////
/*
 * FUNCTION DEFINITIONS
*/ 


// operations on properties -------------------------------------------------

// allocates a stg_property_t structure
stg_property_t* stg_property_create( void );

// attaches the [len] bytes at [data] to the end of [prop],
// reallocating memory appropriately, and returning a pointer to the
// combined data.
stg_property_t* stg_property_attach_data( stg_property_t* prop, 
					  void* data, 
					  size_t len );

// deallocate the memory used by a property (uses free(3)).
void stg_property_free( stg_property_t* prop );

// returns a human readable desciption of the property type [id]
const char* stg_property_string( stg_prop_id_t id );

// compose a human readable string desrcribing property [prop]
char* stg_property_sprint( char* str, stg_property_t* prop );

// print a human-readable property on [fp], an open file pointer
// (eg. stdout, stderr).
void stg_property_fprint( FILE* fp, stg_property_t* prop );

// print a human-readable property on stdout
void stg_property_print( stg_property_t* prop );


// operations on worlds (stg_world_t) --------------------------------------

// get a property of the indicated type, returns NULL on failure
stg_property_t* stg_world_property_get(stg_world_t* world, stg_prop_id_t type);

// set a property of a world
void stg_world_property_set( stg_world_t* world, stg_property_t* prop );

// operations on clients ----------------------------------------------------

// create a new client, connected to the Stage server at [host]:[port].
stg_client_t* stg_client_create( const char* host, int port );

// shut down a client and free its memory
void stg_client_free( stg_client_t* cli );

// ask the client to check for incoming data. Incoming messages will
// update the client data strutures appropriately. The socket is
// polled just once per call.
void stg_client_read( stg_client_t* cli );

// returns the model with matching [id] or NULL if none is found
stg_model_t* stg_client_model_lookup( stg_id_t id );

// operations on models ----------------------------------------------------

// frees and zeros the model's property
void stg_model_property_delete( stg_model_t* model, stg_prop_id_t prop );

// it's good idea to use these interfaces to get a model's data rather
// than accessng the stg_model_t structure directly, as the structure
// may change in future.

// returns a pointer to the property + payload
stg_property_t* stg_model_property( stg_model_t* model, stg_prop_id_t prop );

// returns the payload of the property without the property header stuff.
void* stg_model_property_data( stg_model_t* model, stg_prop_id_t prop );

// loops on stg_client_read() until data is available for the
// indicated property
void stg_model_property_wait( stg_model_t* model, stg_prop_id_t datatype );

// requests a model property from the server, to be sent
// asynchonously. Returns immediately after sending the request. To
// get the data, either (i) call stg_client_read() repeatedly until
// the data shows up. To guarantee getting new data this way, either
// delete the local cached version before making the request, or
// inspect the property timestamps; or (ii) call stg_client_msg_read()
// until you see the data you want.
void stg_model_property_req( stg_model_t* model,  stg_prop_id_t type );

// ask the server for a property, then attempt to read it from the
// incoming socket. Assumes there is no asyncronous traffic on the
// socket (so don't use this after a starting a subscription, for
// example).
stg_property_t* stg_model_property_get_sync( stg_model_t* model, 
					     stg_prop_id_t type );

// set a property of the model, composing a stg_property_t from the
// arguments. read and return an immediate reply, assuming there is no
// asyncronhous data on the same channel. returns the reply, possibly
// NULL.
stg_property_t*
stg_model_property_set_ex_sync( stg_model_t* model,
				stg_time_t timestamp,
				stg_prop_id_t type, // the property 
				void* data, // the new contents
				size_t len ); // the size of the new contents

// Set a property of a remote model 
int stg_model_property_set( stg_model_t* model, stg_property_t* prop ); 

// Set a property of a remote model. Constructs a stg_property_t from
// the individual arguments then calls
// stg_client_property_set(). Returns 0 on success, else positive
// error code
int stg_model_property_set_ex( stg_model_t* model,
			       stg_time_t timestamp,
			       stg_prop_id_t type, // the property 
			       stg_prop_reply_t reply, // the desired reply
			       void* data, // the new contents
			       size_t len ); // the size of the new contents

// Functions for accessing a model's cache of properties.  it's good
// idea to use these interfaces to get a model's data rather than
// accessng the stg_model_t structure directly, as the structure may
// change in future.

// frees and zeros the model's property
void stg_model_property_delete( stg_model_t* model, stg_prop_id_t prop);

// returns a pointer to the property + payload
stg_property_t* stg_model_property( stg_model_t* model, stg_prop_id_t prop);

// returns the payload of the property without the property header stuff.
void* stg_model_property_data( stg_model_t* model, stg_prop_id_t prop );

// deletes the indicated property, requests a new copy, and read until
// it arrives
void stg_model_property_refresh( stg_model_t* model, stg_prop_id_t prop );

// operations on file descriptors ----------------------------------------

// internal low-level functions. user code should probably use one of
// the interafce functions instead.

// attempt to read a packet from [fd]. returns the number of bytes
// read, or -1 on error (and errno is set as per read(2)).
ssize_t stg_fd_packet_read( int fd, void* buf, size_t len );

// attempt to write a packet on [fd]. returns the number of bytes
// written, or -1 on error (and errno is set as per write(2)).
ssize_t stg_fd_packet_write( int fd, void* buf, size_t len );

// attempt to read a header packet from open file descriptor [fd]
// returns 0 on success, else a positive error code.
int stg_fd_header_read( int fd, stg_header_t *hdr );

// send a message on open file descriptor [fd], constructing a header
// appropriately returns 0 on success, else positive error code
int stg_fd_msg_write( int fd, stg_msg_type_t type, 
		      void *data, size_t len );

// read a message from the open file descriptor [fd]. A message is
// returned in [data], its type in [type] and its size in [len]. This
// call allocates memory for [data] using malloc(3), so caller should
// free the data using free(3),realloc(3), etc.  Returns 0 on success,
// else positive error code
//int stg_fd_msg_read( int fd, stg_msg_type_t* type, 
//	     void **data, size_t *len );

// read a message from the server. returns a full message - header and
// payload.  This call allocates memory for the message malloc(3), so
// caller should free the data using free(3),realloc(3), etc.  returns
// NULL on error.
stg_header_t* stg_fd_msg_read( int fd );

// read a message from the open file descriptor [fd]. When you know
// which message is coming, this is easier to use than
// stg_fd_msg_read(). Returns 0 on success, else positive error code
int stg_fd_msg_read_fixed( int fd, stg_msg_type_t type, 
			   void *data, size_t len );

// Attempt to read a stg_property_t and subsequent data from the open
// file descriptor [fd]. Caller must free the returned property with
// free(3). Returns a property or NULL on failure.
//stg_property_t* stg_fd_property_read( int fd );

//stg_property_t* stg_fd_property_read( int fd );
//ssize_t stg_fd_property_write( int fd, stg_property_t* prop );
//stg_property_t* stg_fd_property_read( int fd );

// miscellaneous  ---------------------------------------------------------

void stg_catch_pipe( int signo );

// debug ------------------------------------------------------------------

// returns a human readable desciption of the message type
const char* stg_message_string( stg_msg_type_t id );


// PROPERTY-SPECIFIC DEFINITIONS -------------------------------------------

// simple boolean properties
typedef int stg_nose_t;
typedef int stg_border_t;
typedef int stg_mouse_mode_t;
typedef int stg_matrix_render_t;

// COLOR ----------------------------------------------------------------

typedef uint32_t stg_color_t;

// Look up the color in a database.  (i.e. transform color name to
// color value).  If the color is not found in the database, a bright
// red color will be returned instead.
stg_color_t stg_lookup_color(const char *name);

// POSITION -------------------------------------------------------------

// used for specifying 3 axis positions
typedef struct
{
  double x, y, a;
} stg_pose_t;

// VELOCITY ------------------------------------------------------------

typedef stg_pose_t stg_velocity_t;

// SIZE ----------------------------------------------------------------

// used for various rectangular sizes
typedef struct 
{
  double x, y;
} stg_size_t;

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
  double range;
  double error;
} stg_ranger_t;

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
  double range;
  char reflectance;
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
// TODO - remove this static buffer
  stg_laser_sample_t samples[STG_LASER_SAMPLES_MAX];
} stg_laser_data_t;

// POSITION ------------------------------------------------------------

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
