#ifndef _SERVER_H
#define _SERVER_H

#include "stage.h"

struct _stg_world;
struct _stg_model;

typedef struct
{
  char* host;
  int port;
  
  stg_msec_t start_time;

  int quit; // set this to exit the server cleanly

  GHashTable* worlds; // indexed by id
  
  GArray* pfds; // array of struct pollfds. zeroth pollfd is for the
		// server. the rest are for connected clients
  
  GHashTable* clients; // table of connection_ts indexed by their
		       // file descriptor. each fd matches one in the
		       // pollfd array.

  // maps a world id on to the client that owns it. if the client dies
  //GHashTable* client_world_map;
  

} server_t;


// create a server 
server_t* server_create( int port );

int server_update_clients( server_t* server );
int server_poll( server_t* server );

void server_destroy( server_t* server );


void server_print( server_t* server );

double server_world_time( server_t* server, stg_id_t wid );

struct _world* server_get_world( server_t* server, stg_id_t wid );
struct _model* server_get_model( server_t* server, stg_id_t wid, stg_id_t mid );
int server_get_prop( server_t* server, 
		     stg_id_t wid, stg_id_t mid, stg_id_t pid,
		     void** data, size_t* len );

void server_update_worlds( server_t* server );
void server_update_subs( server_t* server );
int server_msg_dispatch( server_t* server, int fd, stg_msg_t* msg );
void server_remove_subs_of_model( server_t* server, stg_target_t* tgt );

int stg_target_set_data( server_t* server, 
			 stg_target_t* tgt, 
			 void* data, 
			 size_t len );

int server_package_parse( server_t* server, int fd, stg_package_t* pkg );

#endif
