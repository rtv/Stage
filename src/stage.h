#ifndef STAGE_H
#define STAGE_H

#include <stdint.h> // for the integer types (uint32_t, etc)

#if HAVE_CONFIG_H
  #include <config.h>
#endif
#if HAVE_STRINGS_H
  #include <strings.h>
#endif

#define STAGE_TOKEN_MAX 64

// the server reads a header to discover which type of data follows...
typedef enum { StageEntityPackets, 
	       StagePropertyPackets, 
	       StageCommand,
	       StageContinue,
} stage_header_type_t;

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
  //StageType type;
  char token[ STAGE_TOKEN_MAX ];
} __attribute ((packed)) stage_entity_t;

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

#endif
