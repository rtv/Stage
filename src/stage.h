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
 * CVS: $Id: stage.h,v 1.65 2004-07-27 02:56:57 rtv Exp $
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
#include <pthread.h>
#include <semaphore.h>

#include <glib.h> // we use GLib's data structures extensively

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
#define STG_DEFAULT_WORLD_INTERVAL_MS 100 // 10Hz

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
    STG_PROP_MASS,
    STG_PROP_COLOR,
    STG_PROP_GEOM, 
    STG_PROP_NAME, // ?
    STG_PROP_PARENT, 
    STG_PROP_PLAYERID,
    STG_PROP_POSE,
    STG_PROP_POWER,
    STG_PROP_ENERGYCONFIG,
    STG_PROP_ENERGYDATA,
    STG_PROP_PUCKRETURN,
    STG_PROP_LINES,
    STG_PROP_VELOCITY, 
    STG_PROP_LASERRETURN,
    STG_PROP_OBSTACLERETURN,
    STG_PROP_VISIONRETURN,
    STG_PROP_RANGERRETURN, 
    STG_PROP_FIDUCIALRETURN,
    STG_PROP_RANGERDATA,
    STG_PROP_RANGERCONFIG,
    STG_PROP_FIDUCIALCONFIG,
    STG_PROP_FIDUCIALDATA,
    STG_PROP_BLOBCONFIG,
    STG_PROP_BLOBDATA,
    //STG_PROP_BLOBRETURN,
    STG_PROP_LASERDATA,
    STG_PROP_LASERCONFIG,
    STG_PROP_BLINKENLIGHT,  // light blinking rate
    STG_PROP_GUIFEATURES,
    //STG_PROP_LOSMSG,
    //STG_PROP_LOSMSGCONSUME,
    //STG_PROP_MOVEMASK,
    STG_PROP_MATRIXRENDER, // if non-zero, render in the matrix

    STG_PROP_COUNT // this must be the last entry (it's not a real
		   // property - it just counts 'em).
} stg_prop_type_t;


   // These events can happen regarding a property. Callback functions
   // can be installed to handle each event.
typedef enum
{
  STG_EVENT_STARTUP, // going from 0 to 1 subscription
  STG_EVENT_SHUTDOWN, // going from 1 to 0 subscriptions
  STG_EVENT_UPDATE, // called every simulation cycle
  STG_EVENT_SERVICE, // called when a subscription needs new data,
		     // before it generates a GET
  STG_EVENT_GET, // when someone requests the data 
  STG_EVENT_SET, // when someone sets the data

  STG_EVENT_COUNT // this must be the last entry (it's not a real
		  // event - it justs counts 'em).
} stg_event_t;

   
typedef uint16_t stg_msg_type_t;
   
#define STG_MSG_MASK_MAJOR 0xFF00 // major type is in first byte
#define STG_MSG_MASK_MINOR 0x00FF // minor type is in second byte
   
#define STG_MSG_SERVER      1 << 8
#define STG_MSG_WORLD       2 << 8
#define STG_MSG_MODEL       3 << 8
#define STG_MSG_CLIENT      4 << 8
   
// these messages are sent from a client to Stage
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

#define STG_MSG_MODEL_DELTA             (STG_MSG_MODEL | 1) // set a prop, no reply
#define STG_MSG_MODEL_PROPSET           (STG_MSG_MODEL | 2) // set a prop, ACK/NACK reply
#define STG_MSG_MODEL_PROPGET           (STG_MSG_MODEL | 3) // get a property
#define STG_MSG_MODEL_PROPGETSET        (STG_MSG_MODEL | 4) // set a prop, get prior state
#define STG_MSG_MODEL_PROPSETGET        (STG_MSG_MODEL | 5) // set a prop, get post state
#define STG_MSG_MODEL_SUBSCRIBE         (STG_MSG_MODEL | 6) // no reply
#define STG_MSG_MODEL_UNSUBSCRIBE       (STG_MSG_MODEL | 7) // no reply

// these types are sent from Stage to a client
#define STG_MSG_CLIENT_DELTA            (STG_MSG_CLIENT | 1)
#define STG_MSG_CLIENT_REPLY            (STG_MSG_CLIENT | 2)
#define STG_MSG_CLIENT_CLOCK            (STG_MSG_CLIENT | 3)
#define STG_MSG_CLIENT_SAVE             (STG_MSG_CLIENT | 4)
#define STG_MSG_CLIENT_LOAD             (STG_MSG_CLIENT | 5)
#define STG_MSG_CLIENT_QUIT             (STG_MSG_CLIENT | 6)

#define STG_ACK  1
#define STG_NACK 0


// Basic self-describing measurement types. All packets with real
// measurements are specified in these terms so changing types here
// should work throughout the code If you change these, be sure to
// change the byte-ordering macros below accordingly.
   typedef int stg_id_t;
   typedef double stg_meters_t;
   typedef double stg_radians_t;
   typedef unsigned long stg_msec_t;
   typedef double stg_kg_t; // Kilograms (mass)
   typedef double stg_joules_t; // Joules (energy)
   typedef double stg_watts_t; // Watts (Joules per second) (energy expenditure)
   typedef int stg_bool_t;


#define HTON_M(m) htonl(m)   // byte ordering for METERS
#define NTOH_M(m) ntohl(m)
#define HTON_RAD(r) htonl(r) // byte ordering for RADIANS
#define NTOH_RAD(r) ntohl(r)
#define HTON_SEC(s) htonl(s) // byte ordering for SECONDS
#define NTOH_SEC(s) ntohl(s)
#define HTON_SEC(s) htonl(s) // byte ordering for SECONDS
#define NTOH_SEC(s) ntohl(s)



typedef struct 
{
  stg_meters_t x, y;
} stg_size_t;
   
// each message in a packet starts with a standard message
// header. some messages may have no payload at all
typedef struct
{
  stg_msg_type_t type;
  size_t payload_len;
  char payload[0]; // named access to the end of the struct
} stg_msg_t;

typedef struct
{  
  stg_id_t world;
  stg_id_t model;
  stg_id_t prop;
  stg_msec_t timestamp; // the time at which this property was filled
  size_t datalen; // data size
  char data[]; // the data follows
} stg_prop_t;

typedef struct
{
  stg_id_t world;
  stg_msec_t simtime;
} stg_clock_t;

typedef struct
{
  stg_id_t world;
  stg_id_t model;
  stg_id_t prop;
  stg_msec_t interval; // requested time between updates in seconds
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
  stg_id_t parent; // server-side ID of the parent
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
  stg_msec_t interval_sim;
  stg_msec_t interval_real;
  int ppm;
} stg_createworld_t;

typedef struct
{
  stg_id_t world;
} stg_destroyworld_t;
  


stg_msg_t* stg_msg_create( stg_msg_type_t type, void* data, size_t len );
stg_msg_t* stg_msg_append( stg_msg_t* msg, void* data, size_t len );
void stg_msg_destroy( stg_msg_t* msg );

void stg_buffer_append( GByteArray* buf, void* bytes, size_t len );
void stg_buffer_append_msg( GByteArray* buf, stg_msg_t* msg );
void stg_buffer_prepend( GByteArray* buf, void* bytes, size_t len );
void stg_buffer_clear( GByteArray* buf );

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
  stg_meters_t x, y, a;
  stg_bool_t stall;
} stg_pose_t;

typedef struct
{
  stg_pose_t pose;
  stg_size_t size;
} stg_geom_t;

void stg_print_geom( stg_geom_t* geom );

typedef enum
  { STG_POSITION_CONTROL_VELOCITY, STG_POSITION_CONTROL_POSITION }
stg_position_control_mode_t;

typedef enum
  { STG_POSITION_STEER_DIFFERENTIAL, STG_POSITION_STEER_INDEPENDENT }
stg_position_steer_mode_t;


//  --------------------------------------------------------------

// standard energy consumption of some devices in W.
//
// The MOTIONKG value is a hack to approximate cost of motion, as
// Stage does not yet have an acceleration model.
//
#define STG_ENERGY_COST_LASER 20.0 // 20 Watts! (LMS200 - from SICK web site)
#define STG_ENERGY_COST_FIDUCIAL 10.0 // 10 Watts
#define STG_ENERGY_COST_RANGER 0.5 // 500mW (estimate)
#define STG_ENERGY_COST_MOTIONKG 10.0 // 10 Watts per KG when moving 
#define STG_ENERGY_COST_BLOB 4.0 // 4W (estimate)

typedef struct
{
  stg_joules_t joules; // current energy stored in Joules/1000
  stg_watts_t watts; // current power expenditure in mW (mJoules/sec)
  int charging; // 1 if we are receiving energy, -1 if we are
		// supplying energy, 0 if we are neither charging nor
		// supplying energy.
  stg_meters_t range; // the range that our charging probe hit a charger
} stg_energy_data_t;

typedef struct
{
  stg_joules_t capacity; // maximum energy we can store (we start fully charged)
  stg_meters_t probe_range; // the length of our recharge probe
  //stg_pose_t probe_pose; // TODO - the origin of our probe

  stg_watts_t give_rate; // give this many Watts to a probe that hits me (possibly 0)
  
  stg_watts_t trickle_rate; // this much energy is consumed or
			    // received by this device per second as a
			    // baseline trickle. Positive values mean
			    // that the device is just burning energy
			    // stayting alive, which is appropriate
			    // for most devices. Negative values mean
			    // that the device is receiving energy
			    // from the environment, simulating a
			    // solar cell or some other ambient energy
			    // collector

} stg_energy_config_t;


// VELOCITY ------------------------------------------------------------

typedef stg_pose_t stg_velocity_t;

// BLINKENLIGHT ------------------------------------------------------------

// a number of milliseconds, used for example as the blinkenlight interval
#define STG_LIGHT_ON UINT_MAX
#define STG_LIGHT_OFF 0

//typedef int stg_interval_ms_t;

typedef struct
{
  int enable;
  stg_msec_t period;
} stg_blinkenlight_t;

// GRIPPER ------------------------------------------------------------

// Possible Gripper return values
typedef enum 
  {
    GripperDisabled = 0,
    GripperEnabled
  } stg_gripper_return_t;

// RANGER ------------------------------------------------------------

typedef struct
{
  stg_meters_t min, max;
} stg_bounds_t;

typedef struct
{
  stg_pose_t pose;
  stg_size_t size;
  stg_bounds_t bounds_range;
  stg_radians_t fov;
} stg_ranger_config_t;

typedef struct
{
  stg_meters_t range;
  //double error; // TODO
} stg_ranger_sample_t;

// RECTS --------------------------------------------------------------

typedef struct
{
  stg_meters_t x, y, w, h;
} stg_rect_t;

typedef struct
{
  stg_meters_t x, y;
  stg_radians_t a;
  stg_meters_t w, h;
} stg_rotrect_t; // rotated rectangle

// specify a line from (x1,y1) to (x2,y2), all in meters
typedef struct
{
  stg_meters_t x1, y1, x2, y2;
} stg_line_t;

typedef struct
{
  int toplx, toply, toprx, topry, botlx, botly, botrx, botry;
} stg_corners_t;

typedef struct
{
  stg_meters_t x, y;
} stg_point_t;

// MOVEMASK ---------------------------------------------------------
   
typedef int stg_movemask_t;

typedef struct
{
  uint8_t nose;
  uint8_t grid;
  uint8_t boundary;
  stg_movemask_t movemask;
} stg_guifeatures_t;


// GUI -------------------------------------------------------------------

// used for loading and saving GUI state
// prefer to expand this single packet with new fields rather
// than create extra GUI packets
typedef struct
{
  char token[ STG_TOKEN_MAX ]; // string identifying the GUI library
  double ppm;
  int width, height;
  stg_meters_t originx, originy;
  char showgrid;
  char showdata;
} stg_gui_config_t;


// LASER  ------------------------------------------------------------

// Possible laser return values
typedef enum 
  {
    LaserTransparent = 0,
    LaserVisible, // 1
    LaserBright, // 2
  } stg_laser_return_t;

#define STG_DEFAULT_LASERRETURN LaserVisible

typedef struct
{
  uint32_t range; // mm
  char reflectance; 
} stg_laser_sample_t;

typedef struct
{
  stg_geom_t geom;
  stg_radians_t fov;
  stg_meters_t range_max;
  stg_meters_t range_min;
  int samples;
} stg_laser_config_t;


// print human-readable version of the struct
void stg_print_laser_config( stg_laser_config_t* slc );

// BlobFinder ------------------------------------------------------------

#define STG_BLOBFINDER_CHANNELS_MAX 16

typedef struct
{
  int channel_count; // 0 to STG_BLOBFINDER_CHANNELS_MAX
  stg_color_t channels[STG_BLOBFINDER_CHANNELS_MAX];
  int scan_width;
  int scan_height;
  stg_meters_t range_max;
  stg_radians_t pan, tilt, zoom;
} stg_blobfinder_config_t;

typedef struct
{
  int channel;
  stg_color_t color;
  int xpos, ypos;   // all values are in pixels
  //int width, height;
  int left, top, right, bottom;
  int area;
  stg_meters_t range;
} stg_blobfinder_blob_t;


// fiducial finder ------------------------------------------------------

// any integer value other than this is a valid fiducial ID
// TODO - fix this up
#define FiducialNone 0

// TODO - line-of-sight messaging
/* line-of-sight messaging packet */
/* 
#define STG_LOS_MSG_MAX_LEN 32
   
typedef struct
{
  int id;
  char bytes[STG_LOS_MSG_MAX_LEN];
  size_t len;
  int power; // transmit power or received power
  //  int consume;
} stg_los_msg_t;

// print the values in a message packet
void stg_los_msg_print( stg_los_msg_t* msg );
*/

typedef struct
{
  stg_meters_t max_range_anon;
  stg_meters_t max_range_id;
  stg_meters_t min_range;
  stg_radians_t fov; // field of view 
  stg_radians_t heading; // center of field of view

} stg_fiducial_config_t;


typedef struct
{
  stg_meters_t range; // range to the target
  stg_radians_t bearing; // bearing to the target 
  stg_pose_t geom; // size and relative angle of the target
  int id; // the identifier of the target, or -1 if none can be detected.

} stg_fiducial_t;

// end property typedefs -------------------------------------------------


// SOME DEFAULT VALUES FOR PROPERTIES -----------------------------------

#define STG_DEFAULT_MASS 10.0 
#define STG_DEFAULT_POSEX 0.0
#define STG_DEFAULT_POSEY 0.0
#define STG_DEFAULT_POSEA 0.0
#define STG_DEFAULT_ORIGINX 0.0
#define STG_DEFAULT_ORIGINY 0.0
#define STG_DEFAULT_ORIGINA 0.0
#define STG_DEFAULT_SIZEX 0.4
#define STG_DEFAULT_SIZEY 0.4
#define STG_DEFAULT_OBSTACLERETURN TRUE
#define STG_DEFAULT_LASERRETURN LaserVisible
#define STG_DEFAULT_RANGERRETURN TRUE
#define STG_DEFAULT_COLOR (0xFF0000)
#define STG_DEFAULT_MOVEMASK (STG_MOVE_TRANS | STG_MOVE_ROT)
#define STG_DEFAULT_NOSE TRUE
#define STG_DEFAULT_GRID FALSE
#define STG_DEFAULT_BOUNDARY FALSE

#define STG_DEFAULT_ENERGY_CAPACITY 1000.0
#define STG_DEFAULT_ENERGY_CHARGEENABLE 1
#define STG_DEFAULT_ENERGY_PROBERANGE 0.0
#define STG_DEFAULT_ENERGY_GIVERATE 0.0
#define STG_DEFAULT_ENERGY_TRICKLERATE 0.1

#define STG_DEFAULT_LASER_POSEX 0.0
#define STG_DEFAULT_LASER_POSEY 0.0
#define STG_DEFAULT_LASER_POSEA 0.0
#define STG_DEFAULT_LASER_SIZEX 0.15
#define STG_DEFAULT_LASER_SIZEY 0.15
#define STG_DEFAULT_LASER_MINRANGE 0.0
#define STG_DEFAULT_LASER_MAXRANGE 8.0
#define STG_DEFAULT_LASER_FOV M_PI
#define STG_DEFAULT_LASER_SAMPLES 180

#define STG_DEFAULT_BLOB_CHANNELCOUNT 6
#define STG_DEFAULT_BLOB_SCANWIDTH 80
#define STG_DEFAULT_BLOB_SCANHEIGHT 60 
#define STG_DEFAULT_BLOB_RANGEMAX 8.0 
#define STG_DEFAULT_BLOB_PAN 0.0
#define STG_DEFAULT_BLOB_TILT 0.0
#define STG_DEFAULT_BLOB_ZOOM DTOR(60)
 
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
stg_msec_t stg_timenow( void );

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

// read a message from the fd
stg_msg_t* stg_read_msg( int fd );



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
//int stg_fd_msg_write( int fd, stg_msg_t* msg );
int stg_fd_msg_write( int fd, 
		      stg_msg_type_t type, 
		      void* data, size_t datalen );

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
void stg_print_target( stg_target_t* tgt );

// TOKEN -----------------------------------------------------------------------
// token stuff for parsing worldfiles

#define CFG_OPEN '('
#define CFG_CLOSE ')'
#define STR_OPEN '\"'
#define STR_CLOSE '\"'
#define TPL_OPEN '['
#define TPL_CLOSE ']'


typedef enum {
  STG_T_NUM = 0,
  STG_T_BOOLEAN,
  STG_T_MODELPROP,
  STG_T_WORLDPROP,
  STG_T_NAME,
  STG_T_STRING,
  STG_T_KEYWORD,
  STG_T_CFG_OPEN,
  STG_T_CFG_CLOSE,
  STG_T_TPL_OPEN,
  STG_T_TPL_CLOSE,
} stg_token_type_t;


typedef struct _stg_token {
  char* token; // the text of the token 
  stg_token_type_t type;
  unsigned int line; // the line on which the token appears 
  
  struct _stg_token* next; // linked list support
  struct _stg_token* child; // tree support
  
} stg_token_t;

stg_token_t* stg_token_next( stg_token_t* tokens );
// print <token> formatted for a human reader, with a string <prefix>
void stg_token_print( char* prefix,  stg_token_t* token );
// print a token array suitable for human reader
void stg_tokens_print( stg_token_t* tokens );
void stg_tokens_free( stg_token_t* tokens );

// create a new token structure from the arguments
stg_token_t* stg_token_create( const char* token, stg_token_type_t type, int line );

// add a token to a list
stg_token_t* stg_token_append( stg_token_t* head, 
			       char* token, stg_token_type_t type, int line );

const char* stg_token_type_string( stg_token_type_t type );


// Bitmap loading -------------------------------------------------------

// load <filename>, an image format understood by gdk-pixbuf, and
// return a set of rectangles that approximate the image. Caller must
// free the array of rectangles.
int stg_load_image( const char* filename, stg_rotrect_t** rects, int* rect_count );


// Stage Client ---------------------------------------------------------

#define STG_DEFAULT_WORLDFILE "default.world"

typedef void (*stg_client_callback_t)(void);

typedef struct
{
  char* host; // Stage is here
  int port;   // 

  struct pollfd pfd; // contains a socket connected to Stage
 
  stg_id_t next_id; // next available local model ID
  
  stg_msec_t stagetime; // current time in the simulator

  GHashTable* worlds_id_server; // contains stg_world_ts indexed by server-side id
  GHashTable* worlds_id; // contains stg_world_ts indexed by client-side id
  GHashTable* worlds_name; // the worlds indexed by namex
  
  // mutex and condition variable used to signal that a reply has been
  // recieved to a previous request
  //pthread_mutex_t models_mutex;
  //pthread_mutex_t reply_mutex;
  //sem_t reply_sem;
  //pthread_cond_t reply_cond;
  //pthread_t* thread;
  
  // buffer to hold the reply of a request/reply sequence
  GByteArray* reply;
  int reply_ready;

  stg_client_callback_t callback_save;
  stg_client_callback_t callback_load;
  

} stg_client_t;

typedef struct
{
  stg_client_t* client; // the client that created this world
  
  int id_client; // client-side id
  stg_id_t id_server; // server-side id
  stg_token_t* token;  
  double ppm;
  stg_msec_t interval_sim;
  stg_msec_t interval_real;
  
  int section; // worldfile index
  
  GHashTable* models_id_server;
  GHashTable* models_id;   // the models index by client-side id
  GHashTable* models_name; // the models indexed by name
  GHashTable* models_section; // the models indexed by worldfile section
} stg_world_t;

typedef struct _stg_model
{
  int id_client; // client-side if of this object
  stg_id_t id_server; // server-side id of this object
  
  struct _stg_model* parent; 
  stg_world_t* world; 
  
  int section; // worldfile index

  stg_token_t* token;

  GHashTable* props;
  
} stg_model_t;

#define STG_PACKAGE_KEY 12345

typedef struct
{
  int key; // must be STG_PACKAGE_KEY - a version-dependent constant value
  struct timeval timestamp; // real-time timestamp for performance
			    // measurements
  size_t payload_len;
  char payload[0]; // named access to the end of the struct
} stg_package_t;


// Stage Client functions. Most client programs will call all of these
// in this order.


// create a Stage Client.
stg_client_t* stg_client_create( void );


// THESE ARE IMPLEMENTED IN STAGECPP.CC
// because they use the C++ worldfile class (for now)
stg_world_t* stg_client_worldfile_load( stg_client_t* client, 
					char* worldfile_path );
void stg_client_save( stg_client_t* cli, stg_id_t world_id );
void stg_client_load( stg_client_t* cli, stg_id_t world_id );
// END THESE

// read a package: a complete set of Stage deltas. If no package is
// available, return NULL. If an error occurred, return NULL and set
// err > 0
stg_package_t* stg_client_read_package( stg_client_t* cli, 
					int sleep, // poll()'s sleep ms
					int* err );

// break the package into individual messages and handle them
int stg_client_package_parse( stg_client_t* cli, stg_package_t* pkg );

//void stg_client_thread( void* data );
//int stg_client_thread_start( stg_client_t* cli );

//int stg_client_write_msg( stg_client_t* cli, 
//		  stg_msg_type_t type, 
//		  void* data, size_t datalen );

// read and parse a package from the server
int stg_client_read( stg_client_t* cli, int sleeptime );


// load a worldfile into the client
// returns zero on success, else an error code.
//int stg_client_load( stg_client_t* client, char* worldfilename );

// connect to a Stage server at [host]:[port]. If [host] and [port]
// are not specified, client uses it's current values, which have
// sensible defaults.  returns zero on success, else an error code.
int stg_client_connect( stg_client_t* client, const char* host, const int port );

// ask a connected Stage server to create simulations of all our objects
void stg_client_push( stg_client_t* client );


void stg_client_handle_message( stg_client_t* cli, stg_msg_t* msg );

// remove all our objects from from the server
void stg_client_pull( stg_client_t* client );

void stg_client_install_save( stg_client_t* cli, stg_client_callback_t cb );
void stg_client_install_load( stg_client_t* cli, stg_client_callback_t cb );


// destroy a Stage client
void stg_client_destroy( stg_client_t* client );


// add a new world to a client, based on a token
stg_world_t* stg_client_createworld( stg_client_t* client, 
				     int section,
				     stg_token_t* token,
				     double ppm, 
				     stg_msec_t interval_sim, 
				     stg_msec_t interval_real  );

void stg_world_destroy( stg_world_t* world );
int stg_world_pull( stg_world_t* world );

void stg_world_resume( stg_world_t* world );
void stg_world_pause( stg_world_t* world );

stg_model_t* stg_world_model_name_lookup( stg_world_t* world, 
					  char* modelname );

// add a new model to a world, based on a parent, section and token
stg_model_t* stg_world_createmodel( stg_world_t* world, 
				    stg_model_t* parent, 
				    int section, 
				    stg_token_t* token );

void stg_model_destroy( stg_model_t* model );
int stg_model_pull( stg_model_t* model );
void stg_model_attach_prop( stg_model_t* mod, stg_property_t* prop );
void stg_model_prop_with_data( stg_model_t* mod, 
			       stg_id_t type, void* data, size_t len );
stg_property_t* stg_model_get_prop_cached( stg_model_t* mod, stg_id_t propid );

/******************/

int stg_model_prop_get( stg_model_t* mod, stg_id_t propid, void* data, size_t len);
int stg_model_prop_get_var( stg_model_t* mod, stg_id_t propid, void** data, size_t* len);

int stg_model_prop_delta( stg_model_t* mod, stg_id_t prop, void* data, size_t len );

// set property [propid] with [len] bytes at [data]
int stg_model_prop_set( stg_model_t* mod, stg_id_t propid, void* data, size_t len);

// set property [propid] with [len] bytes at [data], wait for a reply
// of [reply_len] bytes and copy it into [reply].
int stg_model_prop_set_reply( stg_model_t* mod, stg_id_t propid, 
			      void* data, size_t data_len,
			      void* reply, size_t reply_len );

// set property [propid] with [len] bytes at [data], wait for a
// reply. At exit, [reply] is a malloc'ed buffer that contains the
// reply data [reply_len] contains the allocated buffer size.
int stg_model_prop_set_reply_var( stg_model_t* mod, stg_id_t propid, 
				  void* data, size_t data_len,
				  void** reply, size_t* reply_len );
/******************/


int stg_model_subscribe( stg_model_t* mod, stg_id_t prop, stg_msec_t interval );
int stg_model_unsubscribe( stg_model_t* mod, stg_id_t prop );

int stg_model_request_reply( stg_model_t* mod, void* req, size_t req_len,
			     void** rep, size_t* rep_len );

void stg_world_push( stg_world_t* model );
void stg_model_push( stg_model_t* model );
void stg_world_push_cb( gpointer key, gpointer value, gpointer user );
void stg_model_push_cb( gpointer key, gpointer value, gpointer user );


// sends a property to the serverx
int stg_model_property_set( stg_model_t* mod, stg_id_t prop, void* data, size_t len );


stg_id_t stg_world_new(  stg_client_t* cli, char* token, 
			 stg_meters_t width, stg_meters_t height, int ppm, 
			 stg_msec_t interval_sim, stg_msec_t interval_real  );

stg_id_t stg_model_new(  stg_client_t* cli, 
			 stg_id_t world,
			 char* token );

int stg_client_property_set( stg_client_t* cli, stg_id_t world, stg_id_t model, 
		      stg_id_t prop, void* data, size_t len );


stg_model_t* stg_client_get_model( stg_client_t* cli, 
				   const char* wname, const char* mname );

stg_model_t* stg_client_get_model_serverside( stg_client_t* cli, stg_id_t wid, stg_id_t mid );

// functions for parsing worldfiles 
//stg_token_t* stg_tokenize( FILE* wf );
//stg_world_t* sc_load_worldblock( stg_client_t* cli, stg_token_t** tokensptr );
//stg_model_t* sc_load_modelblock( stg_world_t* world, stg_model_t* parent, 
//				stg_token_t** tokensptr );

// ---------------------------------------------------------------------------


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

//#define MIN(X,Y) ( x<y ? x : y )
//#define MAX(X,Y) ( x>y ? x : y )

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
