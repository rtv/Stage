#ifndef STG_H
#define STG_H

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
#include <sys/poll.h> // for struct pollfd
   
   typedef struct timeval stg_timeval_t;
   
   // global stage configs
#define STG_TOKEN_MAX 32
#define STG_LISTENQ  128
#define STG_DEFAULT_SERVER_PORT 6601  // move to configure.in?
#define STG_HOSTNAME_MAX  128
#define STG_MAX_CONNECTIONS 128

#define STG_SERVER_GREETING 42   
#define STG_CLIENT_GREETING 142


   // each entity supports up to this many transducer poses
#define STG_TRANSDUCERS_MAX 500

   // all models have a unique type number
   typedef enum
     {
       STG_MODEL_GENERIC = 0,
       STG_MODEL_WALL,
       STG_MODEL_POSITION,
       STG_MODEL_LASER,
       STG_MODEL_SONAR,
       STG_MODEL_COUNT // THIS MUST BE THE LAST MODEL TYPE
     } stg_model_type_t;

  // all properties have unique id numbers and must be listed here
   // please stick to the syntax STG_PROP_<model>_<property>
   typedef enum
     {
       STG_PROP_ENTITY_CREATE=1, 
       STG_PROP_ENTITY_DESTROY,
       STG_PROP_ENTITY_CIRCLES,
       STG_PROP_ENTITY_COLOR,
       STG_PROP_ENTITY_COMMAND,
       STG_PROP_ENTITY_DATA,
       STG_PROP_ENTITY_GEOM,
       STG_PROP_ENTITY_IDARRETURN,
       STG_PROP_ENTITY_LASERRETURN,
       STG_PROP_ENTITY_NAME,
       STG_PROP_ENTITY_OBSTACLERETURN,
       STG_PROP_ENTITY_ORIGIN,
       STG_PROP_ENTITY_PARENT, 
       STG_PROP_ENTITY_PLAYERID,
       STG_PROP_ENTITY_POSE,
       STG_PROP_ENTITY_POWER,
       STG_PROP_ENTITY_PPM,
       STG_PROP_ENTITY_PUCKRETURN,
       STG_PROP_ENTITY_RANGEBOUNDS, 
       STG_PROP_ENTITY_RECTS,
       STG_PROP_ENTITY_SIZE,
       STG_PROP_ENTITY_SONARRETURN,
       STG_PROP_ENTITY_SUBSCRIPTION,
       STG_PROP_ENTITY_VELOCITY,
       STG_PROP_ENTITY_VISIONRETURN,
       STG_PROP_ENTITY_VOLTAGE,
       STG_PROP_ENTITY_TRANSDUCERS,
       STG_PROP_ENTITY_LASER_DATA,
       STG_PROP_ENTITY_NEIGHBORS,
       STG_PROP_ENTITY_NEIGHBORRETURN,
       STG_PROP_ENTITY_NEIGHBORBOUNDS,
       STG_PROP_ENTITY_BLINKENLIGHT,
       STG_PROP_ENTITY_NOSE,
       STG_PROP_IDAR_RX, // see idar.cc
       STG_PROP_IDAR_TX,
       STG_PROP_IDAR_TXRX,
       STG_PROP_WORLD_CREATE,
       STG_PROP_WORLD_DESTROY,
       STG_PROP_WORLD_SIZE,
       STG_PROP_WORLD_GUI, 
       STG_PROP_POSITION_ORIGIN, // see position.cc
       STG_PROP_POSITION_ODOM,
       STG_PROP_POSITION_MODE,
       STG_PROP_POSITION_STEER,
       STG_PROPERTY_COUNT // THIS MUST BE THE LAST ENTRY
     } stg_prop_id_t;
  
// PROPERTY DEFINITIONS ///////////////////////////////////////////////

// a unique id for each entity equal to its position in the world's array
typedef int stg_id_t;

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

// Possible blinkenlight values
typedef enum
  {
    LightNone = 0,
    LightOff,
    LightOn,
    LightBlinkSlow,
    LightBlinkMedium,
    LightBlinkFast,
    LightFlashOnce
  } stg_blinkenlight_t;

// Possible Gripper return values
typedef enum 
  {
    GripperDisabled = 0,
    GripperEnabled
  } stg_gripper_return_t;

// any integer value other than this is a valid fiducial ID
// TODO - fix this up
#define FiducialNone 0

typedef enum
  {
    STG_NOREPLY = 0,
    STG_WANTREPLY,
    STG_ISREPLY
  } stg_reply_mode_t;

// IDAR types ////////////////////////////////////////////////////////////

#define IDARBUFLEN 16   // idar message max in bytes
#define RAYS_PER_SENSOR 5 // resolution

typedef struct
{
  uint8_t mesg[IDARBUFLEN];
  uint8_t len; //0-IDARBUFLEN
  uint8_t intensity;
} stg_idar_tx_t;

typedef struct
{
  uint8_t mesg[IDARBUFLEN];
  uint8_t len; //0-255
  uint8_t intensity; //0-255
  uint8_t reflection; // true/false
  uint32_t timestamp_sec;
  uint32_t timestamp_usec;
  uint16_t range; // mm
} stg_idar_rx_t; 

// line-of-sight communications packet

#define STG_MSG_MAX 32

typedef struct
{
  stg_pose_t pose;
  stg_size_t size;
  double range_min;
  double range_max;
  double range;
  double intensity_send;
  double intensity_receive;
  unsigned char data_send[STG_MSG_MAX];
  unsigned char data_recv[STG_MSG_MAX];
} stg_transducer_t;

typedef struct
{
  stg_id_t id;
  double range;
  double bearing;
  double orientation;
  stg_size_t size;
  unsigned char msg[STG_MSG_MAX];
} stg_neighbor_t;

typedef struct
{
  double min;
  double max;
} stg_bounds_t;

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
  char name[STG_TOKEN_MAX];
  char token[STG_TOKEN_MAX];
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
  stg_rect_t* rects;
  int rect_count;
} stg_rect_array_t;

typedef struct
{
  stg_rotrect_t* rects;
  int rect_count;
} stg_rotrect_array_t;

typedef struct
{
  int toplx, toply, toprx, topry, botlx, botly, botrx, botry;
} stg_corners_t;

// the server reads a header to discover which type of data follows...
typedef enum { 
  STG_HDR_CONTINUE,  // marks the end of a  transaction
  STG_HDR_PROPS, // model property settings follow
  STG_HDR_CMD, // a command to the server (save, load, pause, quit, etc)
} stg_header_type_t;

// COMMANDS - no packet follows; the header's data field is set to one
// of these
typedef enum {
  STG_CMD_SAVE, 
  STG_CMD_LOAD, 
  STG_CMD_PAUSE, 
  STG_CMD_UNPAUSE, 
  STG_CMD_QUIT,
} stg_cmd_t;


// returned by BufferPacket()
typedef struct
{
  char* data;
  size_t len;
} stg_buffer_t;

typedef struct
{
  stg_header_type_t type; // see enum above
  double timestamp;
  size_t len; // this many bytes of data follow (for CMDs this is
 // actually the command number instead)
} stg_header_t;

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
  char showsubscribedonly;
} stg_gui_config_t;

typedef enum {
  STG_NOOP = 0,
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


typedef enum { STG_SUBSCRIBE, STG_UNSUBSCRIBE } stg_subscription_action_t;

typedef struct
{
  stg_prop_id_t property;
  stg_subscription_action_t action;
} stg_subscription_t;

// a client that receives this packet should create a new entity
// and return a single int identifier
typedef struct
{
  stg_id_t id; // Stage chooses an ID on SET and returns it in GET
  stg_id_t parent_id;
  char name[STG_TOKEN_MAX]; // a decsriptive name
  char token[STG_TOKEN_MAX]; // the token used in the world file
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


typedef struct {
  char host[STG_HOSTNAME_MAX];
  int port;
  struct pollfd pollfd;
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

stg_client_t* stg_client_create( char* host, int port );

void stg_client_free( stg_client_t* cli );
size_t stg_packet_read_fd( int fd, void* buf, size_t len );
stg_property_t* stg_property_read_fd( int fd );
ssize_t stg_property_write_fd( int fd, stg_property_t* prop );
void stg_catch_pipe( int signo );
const char* stg_property_string( stg_prop_id_t id );
const char* stg_model_string( stg_model_type_t id );
stg_model_type_t stg_model_type_from_string( char* str );
stg_id_t stg_model_create( stg_client_t* cli, stg_entity_create_t* ent );
int stg_model_destroy( stg_client_t* cli, stg_id_t id );

stg_rotrect_array_t* stg_rotrect_array_create( void );
void stg_rotrect_array_free( stg_rotrect_array_t* r );
// add a rectangle to the end of the array, allocating memory 
stg_rotrect_array_t* stg_rotrect_array_append( stg_rotrect_array_t* array, 
					 stg_rotrect_t* rect );

stg_property_t* stg_send_property( stg_client_t* cli,
				   int id, 
				   stg_prop_id_t type,
				   stg_prop_action_t action,
				   void* data, 
				   size_t len );
stg_id_t stg_model_create( stg_client_t* cli, stg_entity_create_t* ent );
int stg_model_destroy( stg_client_t* cli, stg_id_t id );
stg_id_t stg_world_create( stg_client_t* cli, stg_world_create_t* world );

int stg_model_set_neighbor_return( stg_client_t* cli, stg_id_t id, 
				    int *val );

int stg_model_set_laser_return(stg_client_t* cli, stg_id_t id, 
			       stg_laser_return_t *val);

// SET/GET PROPERTIES
int stg_model_set_size( stg_client_t* cli, stg_id_t id, stg_size_t* sz );
int stg_model_get_size( stg_client_t* cli, stg_id_t id, stg_size_t* sz );

int stg_model_set_velocity(stg_client_t* cli, stg_id_t id, stg_velocity_t* sz);
int stg_model_get_velocity(stg_client_t* cli, stg_id_t id, stg_velocity_t* sz);

int stg_model_set_pose( stg_client_t* cli, stg_id_t id, stg_pose_t* sz);
int stg_model_get_pose( stg_client_t* cli, stg_id_t id, stg_pose_t* sz );

int stg_model_set_origin( stg_client_t* cli, stg_id_t id, stg_pose_t* org );
int stg_model_get_origin( stg_client_t* cli, stg_id_t id, stg_pose_t* org );

int stg_model_subscribe( stg_client_t* cli, stg_id_t id, stg_prop_id_t prop);
int stg_model_unsubscribe( stg_client_t* cli, stg_id_t id, stg_prop_id_t prop);

int stg_model_set_neighbor_bounds( stg_client_t* cli, stg_id_t id, 
				   stg_bounds_t* data );
int stg_model_get_neighbor_bounds( stg_client_t* cli, stg_id_t id, 
				   stg_bounds_t* data );

int stg_model_set_rects(  stg_client_t* cli, stg_id_t id, 
			  stg_rotrect_array_t* rects );

int stg_model_set_transducers( stg_client_t* cli, stg_id_t id, 
			       stg_transducer_t* trans, int count );
int stg_model_get_transducers( stg_client_t* cli, stg_id_t id, 
			       stg_transducer_t** trans, int* count );

int stg_model_set_laser_data( stg_client_t* cli, stg_id_t id, 
			      stg_laser_data_t* data );

int stg_model_get_laser_data( stg_client_t* cli, stg_id_t id, 
			      stg_laser_data_t* data );

int stg_model_get_neighbors( stg_client_t* cli, stg_id_t id, 
			     stg_neighbor_t** neighbors, int *neighbor_count );

int stg_model_set_light( stg_client_t* cli, stg_id_t id, 
			 stg_blinkenlight_t *val);
int stg_model_get_light( stg_client_t* cli, stg_id_t id, 
			 stg_blinkenlight_t *val);

typedef int stg_nose_t;

int stg_model_set_nose( stg_client_t* cli, stg_id_t id, 
			 stg_nose_t *val);
int stg_model_get_nose( stg_client_t* cli, stg_id_t id, 
			 stg_nose_t *val);

//int stg_model_get_rects(  stg_client_t* cli, stg_id_t id, 
//		  stg_rotrect_array_t* rects );


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
#define RTOD(r) ((r) * 180 / M_PI)

// Convert degrees to radians
#define DTOR(d) ((d) * M_PI / 180)

// Normalize angle to domain -pi, pi
#define NORMALIZE(z) atan2(sin(z), cos(z))

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
