#ifndef _CONNECTION_H
#define _CONNECTION_H

#include "stage.h"
#include "server.h"

typedef struct _connection
{
  char* host; // client name
  int port;   // connection port

  int fd; // file descriptor. it matches one contained in the server's
	  // array of struct pollfds
  
  GPtrArray* subs; // array of subscriptions made by this client
  
  GArray* worlds_owned; // list of ids of the worlds owned by this
  // client, so we can destroy them if this client disconnects. TODO -
  // implement a message that removes a world from this list to create
  // worlds that persist when the connection dies
  
  struct _server* server; // the server that created this connection

  GByteArray* outbuf;
  
  // etc.
  void* userdata; // hook for random data
} connection_t;



void stg_connection_print( connection_t* cli );
void stg_connection_print_cb( gpointer key, gpointer value, gpointer user );
connection_t* stg_connection_create( struct _server* server );

void stg_connection_world_own( connection_t* con, stg_id_t wid );

// close the connection and free the memory allocated
void stg_connection_destroy( connection_t* con );


ssize_t stg_connection_write( connection_t* con, void* data, size_t len );
ssize_t stg_connection_write_msg( connection_t* con, 
				  stg_msg_type_t type, void* data, size_t len );

//size_t stg_connection_read( connection_t* con, void* buf, size_t len);
//stg_msg_t* stg_connection_read_msg( connection_t* con );

void stg_connection_sub_update( connection_t* con );
void stg_connection_sub_update_cb( gpointer key, gpointer value, gpointer user );

#endif
