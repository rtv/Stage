#ifndef STAGE_H
#define STAGE_H

#ifdef __cplusplus
 extern "C" {
#endif 

   // for now I'm specifying packets in the native types. I may change
   // this in the future for cross-platform networks, but right now I 
   // want to keep things simple until everything works.
   //#include <stdint.h> // for the integer types (uint32_t, etc)
   
#if HAVE_CONFIG_H
#include <config.h>
#endif
#if HAVE_STRINGS_H
  #include <strings.h>
#endif
   
#include <unistd.h>
   
#define DEBUG
   
   // global stage configs
#define STG_TOKEN_MAX 64
#define STG_LISTENQ  128
#define STG_DEFAULT_SERVER_PORT 6601  // move to configure.in?
#define STG_HOSTNAME_MAX  128
#define STG_MAX_CONNECTIONS 128
   
#define STG_SONAR_MAX_SAMPLES 32

   // (currently) static memory allocation for getting and setting properties
   //const int MAX_NUM_PROPERTIES = 30;
#define MAX_PROPERTY_DATA_LEN  20000
   
#define ENTITY_FIRST_PROPERTY 1
   
   // all properties have unique id numbers and must be listed here
   // please stick to the syntax STG_PROP_<model>_<property>
   typedef enum
   {
     STG_PROP_ENTITY_PARENT,
     STG_PROP_ENTITY_POSE,
     STG_PROP_ENTITY_VELOCITY,
     STG_PROP_ENTITY_SIZE,
     STG_PROP_ENTITY_ORIGIN,
     STG_PROP_ENTITY_NAME,
     STG_PROP_ENTITY_COLOR,
     STG_PROP_ENTITY_SUBSCRIBE,
     STG_PROP_ENTITY_UNSUBSCRIBE,     
     STG_PROP_ENTITY_VOLTAGE,
     STG_PROP_ENTITY_LASERRETURN,
     STG_PROP_ENTITY_SONARRETURN,
     STG_PROP_ENTITY_IDARRETURN,
     STG_PROP_ENTITY_OBSTACLERETURN,
     STG_PROP_ENTITY_VISIONRETURN,
     STG_PROP_ENTITY_PUCKRETURN,
     STG_PROP_ENTITY_PLAYERID,
     STG_PROP_ENTITY_PPM,
     STG_PROP_ENTITY_RECTS,
     STG_PROP_ENTITY_CIRCLES,
     STG_PROP_ENTITY_COMMAND,
     STG_PROP_ENTITY_DATA,
     STG_PROP_ENTITY_CONFIG,
     STG_PROPERTY_COUNT // THIS MUST BE THE LAST ENTRY
   } stage_prop_id_t;


// PROPERTY DEFINITIONS ///////////////////////////////////////////////

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
// TODO - fix this up
#define FiducialNone 0

// image types ////////////////////////////////////////////////////////

unsigned int RGB( int r, int g, int b );

typedef struct 
{
  int x, y;
} stage_point_t;

typedef struct
{
  int toplx, toply, toprx, topry, botlx, botly, botrx, botry;
} stage_rect_t;

typedef struct
{
  double x, y, a, w, h;
} stage_rotrect_t; // rotated rectangle

// a unique id for each entity equal to its position in the world's array
typedef int stage_id_t;

// the server reads a header to discover which type of data follows...
typedef enum { 
  STG_HDR_CONTINUE,  // marks the end of a  transaction
  STG_HDR_MODELS, // requests for new models follow
  STG_HDR_PROPS, // model property settings follow
  STG_HDR_CMD, // a command to the server (save, load, pause, quit, etc)
  STG_HDR_GUI // a GUI configuration packet follows
} stage_header_type_t;

// COMMANDS - no packet follows; the header's data field is set to one
// of these
typedef enum {
  STG_CMD_SAVE, 
  STG_CMD_LOAD, 
  STG_CMD_PAUSE, 
  STG_CMD_UNPAUSE, 
  STG_CMD_QUIT,
} stage_cmd_t;


// returned by BufferPacket()
typedef struct
{
  char* data;
  size_t len;
} stage_buffer_t;

typedef struct
{
  stage_header_type_t type; // see enum above
  unsigned int sec; // at this many seconds
  unsigned int usec; // plus this many microseconds

  size_t len; // this many bytes of data follow (for CMDs this is
 // actually the command number instead)
} __attribute ((packed)) stage_header_t;

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
} stage_gui_config_t;


// this allows us to set a property for an entity
// pretty much any data member of an entity can be set
// using the entity's ::SetProperty() method
typedef struct
{
  int id; // identify the entity
  stage_prop_id_t property; // identify the property
  size_t len; // the property uses this much data (to follow)
} __attribute ((packed)) stage_property_t;

// a client that receives this packet should create a new entity
typedef struct
{
  int id;
  int parent_id;
  char token[ STG_TOKEN_MAX ]; // a string from the library to
 // identify the type of model
} stage_model_t;

// used for specifying 3 axis positions
typedef struct
{
  double x, y, a;
} stage_pose_t;

typedef stage_pose_t stage_velocity_t; // same thing, different name

// used for rectangular sizes
typedef struct 
{
  double x, y;
} stage_size_t;

typedef struct
{
  double x, y, a;
  double xdot, ydot, adot;
  // double xdotdot, ydotdot, adotdot; // useful?
  char mask; // each of the first 6 bits controls which of the commands we are using
} stage_position_data_t;


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

#ifndef M_PI
#define M_PI        3.14159265358979323846
#endif

#ifndef TWOPI
#define TWOPI 2.0 * M_PI
#endif

#define STAGE_HELLO 'S'

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
#define PRINT_MSG4(m,a,b,c,d) printf( "stage: "m" (%s %s)\n", a, b, c, __FILE__, __FUNCTION__)
#else
#define PRINT_MSG(m) printf( "stage: "m"\n" )
#define PRINT_MSG1(m,a) printf( "stage: "m"\n")
#define PRINT_MSG2(m,a,b) printf( "stage: "m" \n" )
#define PRINT_MSG3(m,a,b,c) printf( "stage: "m" \n" )
#define PRINT_MSG4(m,a,b,c,d) printf( "stage: "m" \n" )
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

#ifdef __cplusplus
  }
#endif 

#endif
