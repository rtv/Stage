#ifndef STAGE_H
#define STAGE_H

#ifdef __cplusplus
 extern "C" {
#endif 
   
#include <stdint.h> // for the integer types (uint32_t, etc)
   
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
     STG_PROP_ENTITY_SIZE,
     STG_PROP_ENTITY_ORIGIN,
     STG_PROP_ENTITY_NAME,
     STG_PROP_ENTITY_COLOR,
     STG_PROP_ENTITY_SHAPE,
     STG_PROP_ENTITY_LASERRETURN,
     STG_PROP_ENTITY_SONARRETURN,
     STG_PROP_ENTITY_IDARRETURN,
     STG_PROP_ENTITY_OBSTACLERETURN,
     STG_PROP_ENTITY_VISIONRETURN,
     STG_PROP_ENTITY_PUCKRETURN,
     STG_PROP_ENTITY_PLAYERID,
     STG_PROP_ENTITY_COMMAND,
     STG_PROP_ENTITY_DATA,
     STG_PROP_ENTITY_CONFIG,
     STG_PROP_ENTITY_REPLY, 
     STG_PROPERTY_COUNT // THIS MUST BE THE LAST ENTRY
   } stage_prop_id_t;


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

// a unique id for each entity equal to its position in the world's array
typedef int stage_id_t;

// the server reads a header to discover which type of data follows...
typedef enum { 
  STG_HDR_CONTINUE, 
  STG_HDR_MODELS, 
  STG_HDR_PROPS, 
  STG_HDR_CMD,
} stage_header_type_t;

// COMMANDS - no packet follows; the header's data field is set to one
// of these
typedef enum {
  STG_CMD_SAVE, 
  STG_CMD_LOAD, 
  STG_CMD_PAUSE, 
  STG_CMD_UNPAUSE, 
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
  // the meaning of the data field varies with the message type:
  // for STG_HDR_MODELS, gives the number of stage_model_t packets to follow
  // for STG_HDR_CMD, specifies the command type
  // for STG_HDR_PROPS, specifies the number of bytes to follow, of contiguous
  // stage_property_t + data packet  pairs

  uint32_t sec; // at this many seconds
  uint32_t usec; // plus this many microseconds

  uint32_t data;   
} __attribute ((packed)) stage_header_t;

// this allows us to set a property for an entity
// pretty much any data member of an entity can be set
// using the entity's ::SetProperty() method
typedef struct
{
  uint16_t id; // identify the entity
  stage_prop_id_t property; // identify the property
  uint16_t len; // the property uses this much data (to follow)
} __attribute ((packed)) stage_property_t;

// a client that receives this packet should create a new entity
typedef struct
{
  int32_t id;
  int32_t parent_id;
  char token[ STG_TOKEN_MAX ]; // a string from the library to
 // identify the type of model
} __attribute ((packed)) stage_model_t;

// a client that receives this packet should create a matrix
typedef struct
{
  int32_t sizex;
  int32_t sizey;
} __attribute ((packed)) stage_matrix_t;

//TODO - this is out of date now
typedef struct
{
  int32_t sizex;
  int32_t sizey;
  double scale;
} __attribute ((packed)) stage_background_t;


// used for specifying 3 axis positions
typedef struct
{
  double x, y, a;
} stage_pose_t;

// used for rectangular sizes
typedef struct 
{
  double x, y;
} stage_size_t;

///////////////////////////////////////////////////////////////////////////
// Some useful macros

// Determine size of array
#ifndef ARRAYSIZE
#define ARRAYSIZE(x) (int) (sizeof(x) / sizeof(x[0]))
#endif

#define TRUE 1
#define FALSE 0

#define SUCCESS TRUE
#define FAIL FALSE

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
#define PRINT_ERR(m)         printf("\n\033[41mstage error\033[0m : %s : "m"\n", \
                                    __FILE__)
#define PRINT_ERR1(m, a)     printf("\n\033[41mstage error\033[0m : %s : "m"\n", \
                                    __FILE__, a)
#define PRINT_ERR2(m, a, b)  printf("\n\033[41mstage error\033[0m : %s : "m"\n", \
                                    __FILE__, a, b)

// Warning macros
#define PRINT_WARN(m)         printf("\n\033[44mstage warning\033[0m : %s %s "m"\n", \
                                     __FILE__, __FUNCTION__)
#define PRINT_WARN1(m, a)     printf("\n\033[44mstage warning\033[0m : %s %s "m"\n", \
                                     __FILE__, __FUNCTION__, a)
#define PRINT_WARN2(m, a, b)  printf("\n\033[44mstage warning\033[0m : %s %s "m"\n", \
                                     __FILE__, __FUNCTION__, a, b)
#define PRINT_WARN3(m, a, b, c) printf("\n\033[44mstage warning\033[0m : %s %s "m"\n", \
                                     __FILE__, __FUNCTION__, a, b, c)

// Message macros
#define PRINT_MSG(m) printf("stage msg : %s :\n  "m"\n", __FILE__)
#define PRINT_MSG1(m, a) printf("stage msg : %s :\n  "m"\n", __FILE__, a)
#define PRINT_MSG2(m, a, b) printf("stage msg : %s :\n  "m"\n", __FILE__, a, b)

// DEBUG macros
#ifdef DEBUG
#define PRINT_DEBUG(m)         printf("\r\033[42mstage debug\033[0m : %s %s\n  "m"\n", \
                                     __FILE__, __FUNCTION__)
#define PRINT_DEBUG1(m, a)     printf("\r\033[42mstage debug\033[0m : %s %s\n  "m"\n", \
                                     __FILE__, __FUNCTION__, a)
#define PRINT_DEBUG2(m, a, b)  printf("\r\033[42mstage debug\033[0m : %s %s\n  "m"\n", \
                                     __FILE__, __FUNCTION__, a, b)
#define PRINT_DEBUG3(m, a, b, c) printf("\r\033[42mstage debug\033[0m : %s %s\n  "m"\n", \
                                     __FILE__, __FUNCTION__, a, b, c)
#define PRINT_DEBUG4(m, a, b, c, d) printf("\r\033[42mstage debug\033[0m : %s %s\n  "m"\n", \
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

#ifdef __cplusplus
  }
#endif 

#endif
