#ifndef _SERVER_H
#define _SERVER_H

#include "stage.h"

struct _stg_world;
struct _stg_model;

typedef struct
{
  char* host;
  int port;
  
  //struct timeval start_time;
  stg_time_t start_time;

  int quit; // set this to exit the server cleanly

  GHashTable* worlds; // indexed by id
  
  GArray* pfds; // array of struct pollfds. zeroth pollfd is for the
		// server. the rest are for connected clients
  
  GHashTable* clients; // table of stg_connection_ts indexed by their
		       // file descriptor. each fd matches one in the
		       // pollfd array.

} stg_server_t;


// create a server 
stg_server_t* server_create( int port );

int server_update_clients( stg_server_t* server );
int server_poll( stg_server_t* server );

void server_destroy( stg_server_t* server );


void server_print( stg_server_t* server );

double server_world_time( stg_server_t* server, stg_id_t wid );

struct _stg_world* server_get_world( stg_server_t* server, stg_id_t wid );
struct _stg_model* server_get_model( stg_server_t* server, stg_id_t wid, stg_id_t mid );
int server_get_prop( stg_server_t* server, 
		     stg_id_t wid, stg_id_t mid, stg_id_t pid,
		     void** data, size_t* len );

void server_update_worlds( stg_server_t* server );
void server_update_subs( stg_server_t* server );
int server_msg_dispatch( stg_server_t* server, int fd, stg_msg_t* msg );
void server_remove_subs_of_model( stg_server_t* server, stg_target_t* tgt );

int stg_target_set_data( stg_server_t* server, 
			 stg_target_t* tgt, 
			 void* data, 
			 size_t len );


#endif
