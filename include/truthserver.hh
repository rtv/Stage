
#ifndef _TRUTHSERVER_H
#define _TRUTHSERVER_H

#include "messages.h"
#include "stage_types.hh"

const int TRUTH_SERVER_PORT = 6001;
const int ENVIRONMENT_SERVER_PORT = 6002;


void* TruthServer( void* ); // defined in truthserver.cc
void* EnvServer( void* ); // defined in envserver.cc

typedef	struct sockaddr SA; // useful abbreviation

// packet for truth device
typedef struct
{
  int stage_id;
  StageType stage_type;

  player_id_t id;
  player_id_t parent;
  
  int16_t channel; // ACTS color channel (-1 = no channel)
  uint32_t x, y; // mm, mm
  uint16_t th, w, h; // degrees, mm, mm
  int16_t rotdx, rotdy; // offset the body's center of rotation; mm. 
} __attribute ((packed)) stage_truth_t;

#ifndef _XLIB_H_ // if we're not inclduing the X header

// borrow an X type for compatibility
typedef struct {
    short x, y;
} XPoint;

#endif // _XLIB_H

#endif // _TRUTHSERVER_H


