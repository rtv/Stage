
#ifndef _TRUTHSERVER_H
#define _TRUTHSERVER_H

#include "messages.h" // from player
#include "stage_types.hh"
#include <sys/poll.h>

const int DEFAULT_POSE_PORT = 6601;
const int DEFAULT_ENV_PORT = 6602;

// we usually use 1 or 2, so this should be plenty
const int MAX_POSE_CONNECTIONS = 100; 

// these can be modified in world_load.cc...
extern int global_truth_port;
extern int global_environment_port;

typedef	struct sockaddr SA; // useful abbreviation

typedef struct
{
  uint16_t width, height, ppm;
  uint32_t num_pixels;
  uint16_t num_objects;
} __attribute ((packed)) stage_header_t;

typedef struct
{
  char echo_request;
  int16_t stage_id;
  uint32_t x, y; // mm, mm
  int16_t th; // degrees
} __attribute ((packed)) stage_pose_t;

// packet for truth device
typedef struct
{
  int stage_id, parent_id;
  StageType stage_type;

  char hostname[ HOSTNAME_SIZE ];
  
  player_id_t id;
  
  // stage will echo these truths if this is true
  bool echo_request; 

  //int16_t channel; // ACTS color channel (-1 = no channel)
  uint32_t x, y; // mm, mm
  uint16_t w, h; // mm, mm  
  int16_t th; // degrees
  int16_t rotdx, rotdy; // offset the body's center of rotation; mm.
  uint16_t red, green, blue;
} __attribute ((packed)) stage_truth_t;


// COMMANDS can be sent to Stage over the truth channel
enum cmd_t { SAVEc = 1, LOADc, PAUSEc };


#ifndef _XLIB_H_ // if we're not inclduing the X header

// borrow an X type for compatibility
typedef struct {
    short x, y;
} XPoint;


#endif // _XLIB_H

#endif // _TRUTHSERVER_H


