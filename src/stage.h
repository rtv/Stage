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
#define STAGE_TOKEN_MAX 64
#define STG_DEFAULT_SERVER_PORT 6601  // move to configure.in?
#define STG_HOSTNAME_MAX  128
#define STG_LISTENQ  128
#define STG_MAX_CONNECTIONS 128

// (currently) static memory allocation for getting and setting properties
//const int MAX_NUM_PROPERTIES = 30;
#define MAX_PROPERTY_DATA_LEN  20000

#define ENTITY_FIRST_PROPERTY 1

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

/*
 
  const int PROP_GENERIC = 1 << 16;
  const int PROP_LASER = 2 << 16;
  const int PROP_SONAR = 3 << 16;
  const int PROP_ = 1 << 16;
  const int PROP_LASER = 1 << 16;
  const int PROP_LASER = 1 << 16;


  PropLaserFov = PROP_LASER & 1
  PropLaserRes = PROP_LASER & 2



 */

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
typedef enum { StageModelPackets, 
	       StagePropertyPackets, 
	       StageCommand,
	       StageContinue,
} stage_header_type_t;


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
  // for _Packets types, this gives the number of packets to follow
  // for Command types, specifies the command
  // for Continue types, is not used.
  uint32_t data;   
} __attribute ((packed)) stage_header_t;

// COMMANDS - no packet follows; the header's data field is set to one
// of these
typedef enum { SAVEc = 1, LOADc, PAUSEc, DOWNLOADc, SUBSCRIBEc } stage_cmd_t;

// this allows us to set a property for an entity
// pretty much any data member of an entity can be set
// using the entity's ::SetProperty() method
typedef struct
{
  uint16_t id; // identify the entity
  //EntityProperty property; // identify the property
  uint16_t len; // the property uses this much data (to follow)
} __attribute ((packed)) stage_property_t;

// a client that receives this packet should create a new entity
typedef struct
{
  int32_t id;
  int32_t parent;
  char token[ STAGE_TOKEN_MAX ]; // a string from the library to
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

// pose changes are so common that we have a special message for them,
// rather than setting the [x,y,th] properties seperately. i've taken
// out the parent_id field for compactness - you must set that
// property directly - it doesn't happen too often anyway
typedef struct
{
  int16_t stage_id;
  // i changed the x and y to signed ints so that XS can handle negative
  // local coords -BPG
  int32_t x, y; // mm, mm 
  int16_t th; // degrees
} __attribute ((packed)) stage_pose_t;

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

#ifdef __cplusplus
  }
#endif 

#endif
