
#ifndef _TRUTHSERVER_H
#define _TRUTHSERVER_H

#include "messages.h" // from player
#include "stage_types.hh"
#include <sys/poll.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

const int DEFAULT_POSE_PORT = 6601;
const int DEFAULT_ENV_PORT = 6602;

// we usually use 1 or 2, so this should be plenty
const int MAX_POSE_CONNECTIONS = 100; 

// these can be modified in world_load.cc...
extern int global_truth_port;
extern int global_environment_port;

typedef	struct sockaddr SA; // useful abbreviation

enum MessageType { PosePacket, TruthPacket , 
		   Continue, ContinueTime, Command };

// every message on the truth channel uses this header before the data packets
// defined below 
typedef struct
{
  MessageType type;
  // meaning of the data field varies with the message type:
  // for XPackets types, this gives the number of packets to follow
  // for Command types, gives the command number
  // for Continue types, is not used.
  uint32_t data;   
} __attribute ((packed)) stage_truth_header_t;


typedef struct
{
  uint16_t width, height, ppm;
  uint32_t num_pixels;
  uint16_t num_objects;
} __attribute ((packed)) stage_env_header_t;

typedef struct
{
  char echo_request;
  int16_t stage_id;
  // i changed the x and y to signed ints so that XS can handle negative
  // local coords -BPG
  int32_t x, y; // mm, mm 
  int16_t th; // degrees
  int16_t parent_id;
} __attribute ((packed)) stage_pose_t;

// packet for truth device
typedef struct
{
  int stage_id, parent_id;
  StageType stage_type;
  //StageShape stage_shape; // TODO
  struct in_addr hostaddr; // the IP of the host - just 32 bits
  player_id_t id;  
  bool echo_request;   // stage will echo these truths if this is true
  uint32_t x, y; // mm, mm
  uint16_t w, h; // mm, mm  
  int16_t th; // degrees
  int16_t rotdx, rotdy; // offset the body's center of rotation; mm.
  uint16_t red, green, blue;
} __attribute ((packed)) stage_truth_t;


// COMMANDS can be sent to Stage over the truth channel
enum cmd_t { SAVEc = 1, LOADc, PAUSEc, DOWNLOADc };

#endif // _TRUTHSERVER_H


