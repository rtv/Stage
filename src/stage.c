/*
 *  Stage : a multi-robot simulator.  
 * 
 *  Copyright (C) 2001-2003 Richard Vaughan, Andrew Howard and Brian
 *  Gerkey for the Player/Stage Project
 *  http://playerstage.sourceforge.net
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

/* File: stage.c
 * Desc: functions for interfacing with the Stage server
 * Author: Richard Vaughan vaughan@hrl.com 
 * Date: 1 June 2003
 *
 * CVS: $Id: stage.c,v 1.25 2003-10-22 19:51:02 rtv Exp $
 */

#define DEBUG

#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <signal.h>
#include <netinet/in.h>

#include "stage.h"

//////////////////////////////////////////////////////////////////////////
/*
 * FUNCTION DEFINITIONS
*/ 

// operations on properties -------------------------------------------------

// allocates a stg_property_t structure
stg_property_t* stg_property_create( void )
{
  stg_property_t* prop;
  
  prop = (stg_property_t*)calloc(sizeof(stg_property_t),1);

  assert( prop );
  
  //PRINT_WARN1( "created property at %p", prop );
  return prop;
}

// attaches the [len] bytes at [data] to the end of [prop],
// reallocating memory appropriately, and returning a pointer to the
// combined data.
stg_property_t* stg_property_attach_data( stg_property_t* prop, 
					  void* data, 
					  size_t len )
{
  assert( prop );
  
  //PRINT_DEBUG1( "start: prop at %p", prop );
  //stg_property_print( prop );

  if( data && len > 0 )
    {  
      size_t totalsize = sizeof(stg_property_t)+len;
      
      //PRINT_DEBUG1( "attaching %d bytes of data", len );
      
      assert( prop = (stg_property_t*)realloc(prop,totalsize) );
      
      /* record the amount of extra data allocated */
      prop->len = len;
      
      /* copy in the data */
      memcpy( prop->data, data, len );
    }
  else
    PRINT_WARN( "attaching no data" );

  //PRINT_DEBUG1( "result: prop at %p", prop );
  //stg_property_print( prop );
  
  return prop;
}

// deallocate the memory used by a property (uses free(3)).
void stg_property_free( stg_property_t* prop )
{
  //PRINT_WARN1( "freeing property at %p", prop );
  free( prop );
}

/*
      // server properties
    case STG_SERVER_TIME:  return "STG_MOD_SERVER_TIME"; break;  
    case STG_SERVER_WORLD_COUNT: return "STG_SERVER_WORLD_COUNT"; break;  
    case STG_SERVER_CLIENT_COUNT: return "STG_SERVER_CLIENT_COUNT"; break;  

      // world properties
    case STG_WORLD_TIME: return "STG_WORLD_TIME"; break;  
    case STG_WORLD_MODEL_COUNT:  return "STG_WORLD_MODEL_COUNT"; break;  
*/    


// returns a human readable desciption of the message type
const char* stg_message_string( stg_msg_type_t id )
{
  switch( id )
    {
    case STG_MSG_UNKNOWN: return "STG_MSG_UNKNOWN";
    case STG_MSG_NOOP: return "STG_MSG_NOOP";
    case STG_MSG_PAUSE: return "STG_MSG_PAUSE";
    case STG_MSG_RESUME: return "STG_MSG_RESUME";
    case STG_MSG_PROPERTY: return "STG_MSG_PROPERTY";
    case STG_MSG_PROPERTY_REQ: return "STG_MSG_PROPERTY_REQ";
    case STG_MSG_SAVE: return "STG_MSG_SAVE";
    case STG_MSG_LOAD: return "STG_MSG_LOAD";
    case STG_MSG_SUBSCRIBE: return "STG_MSG_SUBCRIBE";
    case STG_MSG_WORLD_CREATE: return "STG_MSG_WORLD_CREATE";
    case STG_MSG_WORLD_CREATE_REPLY: return "STG_MSG_WORLD_CREATE_REPLY";
    case STG_MSG_WORLD_DESTROY: return "STG_MSG_WORLD_DESTROY";
    case STG_MSG_MODEL_CREATE: return "STG_MSG_MODEL_CREATE";
    case STG_MSG_MODEL_CREATE_REPLY: return "STG_MSG_MODEL_CREATE_REPLY";
    case STG_MSG_MODEL_DESTROY: return "STG_MSG_MODEL_DESTROY";
    default:
      break;
    }
  return "<unknown>";
}

// returns a human readable desciption of the property type [id]
const char* stg_property_string( stg_prop_id_t id )
{
  switch( id )
    {
      //case STG_MOD_NULL: return "STG_MOD_NULL"; break;
    case STG_MOD_CHILDREN: return "STG_MOD_CHILDREN"; break;
    case STG_MOD_STATUS: return "STG_MOD_STATUSTIME"; break;
    case STG_MOD_TIME: return "STG_MOD_TIME"; break;
    case STG_MOD_CIRCLES: return "STG_MOD_CIRCLES"; break;
    case STG_MOD_COLOR: return "STG_MOD_COLOR"; break;
    case STG_MOD_GEOM: return "STG_MOD_GEOM"; break;
    case STG_MOD_LASERRETURN: return "STG_MOD_LASERRETURN"; break;
    case STG_MOD_NAME: return "STG_MOD_NAME"; break;
    case STG_MOD_OBSTACLERETURN: return "STG_MOD_OBSTACLERETURN";break;
    case STG_MOD_ORIGIN: return "STG_MOD_ORIGIN"; break;
    case STG_MOD_PARENT: return "STG_MOD_PARENT"; break;
    case STG_MOD_PLAYERID: return "STG_MOD_PLAYERID"; break;
    case STG_MOD_POSE: return "STG_MOD_POSE"; break;
    case STG_MOD_POWER: return "STG_MOD_SONAR_POWER"; break;
    case STG_MOD_PPM: return "STG_MOD_PPM"; break;
    case STG_MOD_PUCKRETURN: return "STG_MOD_PUCKETURN"; break;
    case STG_MOD_RANGEBOUNDS: return "STG_MOD_RANGEBOUNDS";break;
    case STG_MOD_RECTS: return "STG_MOD_RECTS"; break;
    case STG_MOD_SIZE: return "STG_MOD_SIZE"; break;
    case STG_MOD_SONARRETURN: return "STG_MOD_SONARRETURN"; break;
    case STG_MOD_NEIGHBORRETURN: return "STG_MOD_NEIGHBORRETURN"; break;
    case STG_MOD_NEIGHBORBOUNDS: return "STG_MOD_NEIGHBORBOUNDS"; break;
    case STG_MOD_NEIGHBORS: return "STG_MOD_NEIGHBORS"; break;
    case STG_MOD_VELOCITY: return "STG_MOD_VELOCITY"; break;
    case STG_MOD_VISIONRETURN: return "STG_MOD_VISIONRETURN"; break;
    case STG_MOD_VOLTAGE: return "STG_MOD_VOLTAGE"; break;
    case STG_MOD_RANGERS: return "STG_MOD_RANGERS";break;
    case STG_MOD_LASER_DATA: return "STG_MOD_LASER_DATA";break;
    case STG_MOD_BLINKENLIGHT: return "STG_MOD_BLINKENLIGHT";break;
    case STG_MOD_NOSE: return "STG_MOD_NOSE";break;
    case STG_MOD_BORDER: return "STG_MOD_BORDER";break;
    case STG_MOD_LOS_MSG: return "STG_MOD_LOS_MSG";break;
    case STG_MOD_LOS_MSG_CONSUME: return "STG_MOD_LOS_MSG_CONSUME";break;
    case STG_MOD_MOUSE_MODE: return "STG_MOD_MOUSE_MODE";break;
    case STG_MOD_MATRIX_RENDER: return "STG_MOD_MATRIX_RENDER";break;
    case STG_MOD_INTERVAL: return "STG_MOD_INTERVAL";break;
	
      // todo
      //case STG_MOD_POSITION_ODOM: return "STG_MOD_POSITION_ODOM"; break;
      //case STG_MOD_POSITION_MODE: return "STG_MOD_POSITION_MODE"; break;
      //case STG_MOD_POSITION_STEER: return "STG_MOD_POSITION_STEER"; break;

    default:
      break;
    }
  return "<unknown>";
}


// compose a human readable string desrcribing property [prop]
char* stg_property_sprint( char* str, stg_property_t* prop )
{
  sprintf( str, 
	   "Property @ %p - id: %d type: %s bytes: %d\n",
	   prop, 
	   prop->id, 
	   //prop->timestamp, 
	   stg_property_string(prop->type), 
	   prop->len );
  
  return str;
}


// print a human-readable property on [fp], an open file pointer
// (eg. stdout, stderr).
void stg_property_fprint( FILE* fp, stg_property_t* prop )
{
  char str[1000];
  stg_property_sprint( str, prop );
  fprintf( fp, "%s", str );
}

// print a human-readable property on stdout
void stg_property_print( stg_property_t* prop )
{
  stg_property_fprint( stdout, prop );
}

void stg_catch_pipe( int signum )
{
  PRINT_WARN1( "caught SIG_PIPE (%d)", signum );
}

// operations on clients ----------------------------------------------------

// create a new client, connected to the Stage server at [host]:[port].
stg_client_t* stg_client_create( const char* host, int port )
{
  if( host == NULL )
    host = "localhost";

  if( port < 1 )
    port = STG_DEFAULT_SERVER_PORT;

  stg_client_t *cli = (stg_client_t*)malloc( sizeof( stg_client_t ) );
  assert( cli );
  
  memset( cli, 0, sizeof(*cli) );

  strncpy( cli->host, host, STG_HOSTNAME_MAX );
  cli->port = port;
  
  // get the IP of our host
  struct hostent* info = gethostbyname(  host );
  
  if( info )
    { // make sure this looks like a regular internet address
      assert( info->h_length == 4 );
      assert( info->h_addrtype == AF_INET );
    }
  else
    {
      PRINT_ERR1( "failed to resolve IP for remote host\"%s\"", 
		  host );
      return NULL;
    }
  struct sockaddr_in servaddr;
  
  /* open socket for network I/O */
  cli->pollfd.fd = socket(AF_INET, SOCK_STREAM, 0);
  cli->pollfd.events = POLLIN; // notify me when data is available
  
  // switch on the re-use-address option
  const int on = 1;
  setsockopt( cli->pollfd.fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on) );
  
  if( cli->pollfd.fd < 0 )
    {
      puts( "Error opening network socket for connection"  );
      fflush( stdout );
      return NULL;
    }
  
  /* setup our server address (type, IP address and port) */
  bzero(&servaddr, sizeof(servaddr)); /* initialize */
  servaddr.sin_family = AF_INET;   /* internet address space */
  servaddr.sin_port = htons( port ); /*our command port */ 
  memcpy(&(servaddr.sin_addr), info->h_addr_list[0], info->h_length);
 
  if( signal( SIGPIPE, stg_catch_pipe ) == SIG_ERR )
    {
      PRINT_ERR( "error setting pipe signal catcher" );
      exit( -1 );
    }
 
  if( connect( cli->pollfd.fd, 
               (struct sockaddr*)&servaddr, sizeof( servaddr) ) == -1 )
    {
      PRINT_ERR2( "Connection failed on %s:%d",
		  info->h_addr_list[0], port ); 
      PRINT_ERR( "Did you forget to start Stage?");
      perror( "" );
      fflush( stdout );
    return NULL;
    }
  
  // send the greeting
  stg_greeting_t greeting;
  greeting.code = STG_SERVER_GREETING;
  
  int r;
  if( (r = write( cli->pollfd.fd, &greeting, sizeof(greeting) )) < 1 )
    {
      PRINT_ERR( "failed to write STG_SERVER_GREETING to server." );
      if( r < 0 ) perror( "error on write" );
      return NULL;
    }

  stg_connect_reply_t reply;
  // wait for the reply
  if( (r = read( cli->pollfd.fd, &reply, sizeof(reply) )) < 1 )
    {
      PRINT_ERR( "failed to READ STG_CLIENT_GREETING from server." );
      if( r < 0 ) perror( "error on read" );
      return NULL;
    }
  
  if( reply.code != STG_CLIENT_GREETING ) 
    PRINT_ERR1( "received bad reply from server (%d)", greeting.code );
  else
    PRINT_DEBUG1( "received good reply from server (%d)", greeting.code );

  // announce the connection on the console
  printf( "Server: \"%s\" (v.%d.%d.%d)\n",
	  reply.id_string, reply.vmajor, reply.vminor, reply.vbugfix );
  
  return cli;
}  


// shut down a client and free its memory
void stg_client_free( stg_client_t* cli )
{
  if( cli->pollfd.fd > 0 ) close( cli->pollfd.fd );
  free( cli );
}

// ask the client to check for incoming data. Incoming messages will
// update the client data strutures appropriately. The socket is
// polled just once per call.
void stg_client_read( stg_client_t* cli )
{
  // sxee if any data is pending
  poll( &cli->pollfd, 1, 0 ); // don't block
  
  // TODO - loop here?
  if( cli->pollfd.revents & POLLIN )
    {
      //puts( "POLLIN" );

      stg_header_t* hdr = stg_fd_msg_read(cli->pollfd.fd );
      
      if( hdr == NULL )
	{
	  PRINT_ERR1( "error reading message on fd %d", 
		      cli->pollfd.fd  );
	  // todo - close client nicely
	  PRINT_DEBUG( "close client nicely" );
	  exit(-1);
	}
      
  
      switch( hdr->type )
	{
	case STG_MSG_SAVE:
	  puts( "SAVE" );
	  //stg_client_save( cli );
	  break;
	  
	case STG_MSG_SUBSCRIBE:
	  puts( "SUBSCRIBE" );
	  //stg_client_handle_subscription( cli );
	  break;
	  
	case STG_MSG_PROPERTY:
	  puts( "PROPERTY" );
	  {
	    // copy the property out of the message so we can
	    // free the header & property independently
	    stg_property_t* tmpprop = (stg_property_t*)hdr->payload;
	    size_t plen = sizeof(stg_property_t) + tmpprop->len;
	    stg_property_t* prop = calloc( plen, 1 );
	    memcpy( prop, tmpprop, plen );

	    // TODO - this doesn't seem to b triggered when Stage dies. what
	    // is up with that?
	    if( prop == NULL )
	      {
		PRINT_ERR( "failed to read a property "
			     "(I would quit nicely here if only i knew how)" );
		// have a callback in the client here
		//Interrupt(0); // kills Player dead.
		exit(-1);
	      }
	    
	    PRINT_DEBUG3( "received property [%d:%s] "
			  "data %d bytes", //, time %.3f seconds",
			  prop->id, 
			  stg_property_string(prop->type), 
			  prop->len
			  //prop->timestamp 
			  );
	    
	    if( prop->id == cli->world->id )
	      stg_world_property_set( cli->world, prop );
	    else
	      {
		// find the incoming stage id in the model array and
		// poke in the data. TODO - use a hash table instead
		// of an array so we avoid this search
		stg_model_t* target = NULL;
		
		int i;
		for( i=0; i< cli->model_count; i++ )
		  {
		    if( cli->models[i]->id == prop->id )
		      {
			target = cli->models[i];
			//printf( " - target is section %d\n", i );
			break;
		      }
		  }
		
		if( target == NULL )
		  PRINT_ERR1( "received property for unknown model (%d)",
			      prop->id );
		else
		  {
		    //int y=0;
		    //for( y=0; y<STG_MOD_PROP_COUNT; y++ )
		    //if( target->props[y] )
		    //PRINT_WARN4( "model %p id %d prop %s %p\n",
		    //	     target, 
		    //	     target->id, 
		    //	     stg_property_string(y),
		    //	     target->props[y] );
		    
		    PRINT_DEBUG2( "deleting prop %d:%s",
				  target->id, stg_property_string(prop->type));
		    stg_model_property_delete( target, prop->type );
		    target->props[prop->type] = prop;
		  }
	      }
	  } 
	  break;
	  
	default:
	  PRINT_WARN1( "unhandled message type %s", 
		       stg_message_string(hdr->type) );
	  break;
	}

      free(hdr);
    }
}



// send a message of [len] bytes at [data] of type [type] on open file
// descriptor [fd] constructing a header appropriately. returns 0 on
// success, else positive error code
int stg_fd_msg_write( int fd, stg_msg_type_t type, 
		      void *data, size_t len )
{  
  stg_header_t hdr;
  hdr.type = type;
  hdr.len = len;
  
  //PRINT_DEBUG3( "writing %d bytes header + %d bytes data on fd %d", 
  //	sizeof(hdr), len, fd  );
  
  ssize_t res = stg_fd_packet_write( fd, &hdr, sizeof(hdr) );

  if( res < 0 )
    {
      PRINT_ERR( "header write error" ); 
      perror("");
      return 1;
    }
  
  if( res != sizeof(hdr) )
    {
      PRINT_ERR2( "header write size wrong (%d/%d) bytes",
		  res, sizeof(hdr) );
      return 2;
    }
  
  res =  stg_fd_packet_write( fd, data, len );
  
  if( res < 0 )
    {
      PRINT_ERR( "data write error" );
      perror("");
      return 1;
    }
  
  if( res != len )
    {
      PRINT_ERR2( "data write size wrong (%d/%d) bytes",
		  res, sizeof(hdr) );
      return 2;
    }
  
  return 0; // ok
}


// read a message from the server. returns a full message - header and
// payload.  This call allocates memory for the message malloc(3), so
// caller should free the data using free(3),realloc(3), etc.  returns
// NULL on error.
stg_header_t* stg_fd_msg_read( int fd )
{
  //PRINT_DEBUG( "msg read" );

  // read a header
  stg_header_t* hdr = (stg_header_t*)calloc( sizeof(stg_header_t), 1);
  
  if( stg_fd_header_read( fd, hdr ) )
    {
      PRINT_ERR( "failed to read a message header" );
      free( hdr ); // remove the dud data
      return NULL;
    }
    
  //PRINT_DEBUG2( "read msg type %s with %d bytes to follow", 
  //	stg_message_string(hdr->type), hdr->len );
  
  if( hdr->len > 0 ) // if there is some data to follow
    {
      // allocate enough memory for it
      hdr = (stg_header_t*)realloc( hdr, sizeof(stg_header_t) + hdr->len );
      
      //PRINT_DEBUG1( "reading %d byte message payload", hdr->len );
      
      // read the message
      if( stg_fd_packet_read( fd, hdr->payload, hdr->len ) != hdr->len )
	{
	  PRINT_ERR( "failed to read a message payload" );
	  free( hdr ); // remove the dud data
	  return NULL;
	}
    }

  return hdr; 
}

// read a message from the open file descriptor [fd]. When you know
// which message is coming, this is easier to use than
// stg_fd_msg_read(). Returns 0 on success, else positive error code
int stg_fd_msg_read_fixed( int fd, stg_msg_type_t type, 
			   void *data, size_t len )
{ 
  PRINT_DEBUG1( "msg read fixed (type %s)", stg_message_string(type) );

  // read a header
  stg_header_t hdr; 
  
  if( stg_fd_packet_read( fd, &hdr, sizeof(stg_header_t) ) 
      != sizeof(stg_header_t) )
    {
      PRINT_ERR( "failed to read a message header" );
      return 1;
    }
    
  if( hdr.len > 0 ) // if there is some data to follow
    {
      if( hdr.type != type )
	{
	  
	  PRINT_ERR2( "wrong message type (%d/%d)",
		      hdr.type, type );
	  return 1;
	}
      
      if( hdr.len != len )
	{
	  PRINT_ERR2( "wrong message length (got %d, expected %d)",
		      hdr.len, len );
	  return 2;
	}

      // read the message
      if( stg_fd_packet_read( fd, data, len ) )
	{
	  PRINT_ERR( "failed to read a message payload" );
	  return 3;
	}
    }
  return 0; // ok
}


//stg_property_t* stg_client_property_read( stg_client_t* cli );

// returns 0 on success, else positive error code
//stg_sub_result_t stg_client_property_subscribe( stg_client_t* cli, 
//					int model_index, 
//					stg_prop_id_t data );

// returns 0 on success, else positive error code
//stg_sub_result_t stg_client_property_unsubscribe( stg_client_t* cli, 
//					  int model_index, 
//					  stg_prop_id_t data );

void stg_model_property_wait( stg_model_t* model, stg_prop_id_t datatype )
{
  PRINT_DEBUG2( "waiting for property %d:%s", 
	       model->id, stg_property_string(datatype) );

  // wait until a property of this type shows up in the buffer
  while( model->props[datatype] == 0 )
    {
      PRINT_DEBUG2("waiting until %d %s shows up", 
		   model->id, stg_property_string(datatype) );
      
      stg_client_read( model->client );
      
      usleep(100);
    }

  PRINT_DEBUG( "data arrived" );
}



// Set a property of a remote model 
int stg_model_property_set( stg_model_t* model, stg_property_t* prop )    
{  
  
  PRINT_DEBUG4( "property set %d:%s %d bytes reply %d",
		prop->id,
		stg_property_string(prop->type), 
		prop->len, 
		prop->reply );
  
  //PRINT_DEBUG2( "writing %d bytes on fd %d",
  //       sizeof(stg_property_t)+prop->len,  model->client->pollfd.fd );

  return stg_fd_msg_write( model->client->pollfd.fd, STG_MSG_PROPERTY,
			   prop, sizeof(stg_property_t)+prop->len );
}   

// Set a property of a model. Constructs a stg_property_t from the
// individual arguments then calls stg_client_property_set(). Returns
// 0 on success, else positive error code
int stg_model_property_set_ex( stg_model_t* model, // the Stage model  
			       stg_time_t timestamp,
			       stg_prop_id_t type, // the property 
			       stg_prop_reply_t reply, // the desired reply
			       void* data, // the new contents
			       size_t len ) // the size of the new contents
{
  PRINT_DEBUG4( "property set ex %p %.3f %d %d",
		model, timestamp, type, reply );

  stg_property_t* prop =  stg_property_create();
  prop->id = model->id;
  //prop->timestamp = timestamp;
  prop->type = type;
  prop->reply = reply;
  
  prop = stg_property_attach_data( prop, data, len);  

  PRINT_DEBUG2( "calling stg_model_property_set( %p, %p )",
		model, prop );
  
  stg_model_property_set( model, prop );
  stg_property_free( prop );
  
  // todo - error checking
  return 0;
}  


// requests a model property from the server, to be sent
// asynchonously. Returns immediately after sending the request. To
// get the data, either (i) call stg_client_read() repeatedly until
// the data shows up. To guarantee getting new data this way, either
// delete the local cached version before making the request, or
// inspect the property timestamps; or (ii) call stg_client_msg_read()
// until you see the data you want.
void stg_model_property_req( stg_model_t* model, stg_prop_id_t type )
{
  stg_property_req_t req;
  req.id = model->id;
  req.type = type;
  
  stg_fd_msg_write( model->client->pollfd.fd, 
		    STG_MSG_PROPERTY_REQ,
		    &req, sizeof(req) );
}  

// delete the indicated property, request a new copy, and read until
// it arrives
void stg_model_property_refresh( stg_model_t* model, stg_prop_id_t prop )
{
  stg_model_property_delete( model, prop );
  stg_model_property_req( model, prop );
  stg_model_property_wait( model, prop );
}


/* TODO

// ask the server for a property, then attempt to read it from the
// incoming socket. Assumes there is no asyncronous traffic on the
// socket (so don't use this after a starting a subscription, for
// example).
stg_property_t* stg_client_property_get_sync( stg_client_t* cli,
					      int id, 
					      stg_prop_id_t type );

// set a property of the model, composing a stg_property_t from the
// arguments. read and return an immediate reply, assuming there is no
// asyncronhous data on the same channel. returns the reply, possibly
// NULL.
stg_property_t*
stg_client_property_set_ex_sync( stg_client_t* cli, 
				 stg_id_t id, // the Stage mode
				 stg_prop_id_t type, // the property 
				 void* data, // the new contents
				 size_t len ); // the size of the new contents

*/

// Functions for accessing the client proxy's cache of properties.
// it's good idea to use these interfaces to get a model's data rather
// than accessng the stg_model_t structure directly, as the structure
// may change in future.

// operations on models ----------------------------------------------------

// it's good idea to use these interfaces to get a model's data rather
// than accessng the stg_model_t structure directly, as the structure
// may change in future.

// returns a pointer to the property + payload
stg_property_t* stg_model_property( stg_model_t* model, stg_prop_id_t prop )
{
  assert( model );
  assert( model->props );

  return model->props[prop];
}

// frees and zeros the model's property
void stg_model_property_delete( stg_model_t* model, stg_prop_id_t prop )
{
  assert( model );
  assert( model->props );
  
  if( model->props[prop] )
    {
      free(model->props[prop]);
      model->props[prop] = NULL;
    }
}

// returns the payload of the property without the property header stuff.
void* stg_model_property_data( stg_model_t* model, stg_prop_id_t prop )
{
  assert( model );
  assert( model->props );
  assert( model->props[prop] );

  return model->props[prop]->data;
}

// operations on worlds (stg_world_t) --------------------------------------

// get a property of the indicated type, returns NULL on failure
stg_property_t* stg_world_property_get(stg_world_t* world, stg_prop_id_t type)
{
  assert( world );

  stg_property_t* prop = stg_property_create();

  prop->id = world->id;
  prop->type = type;

  switch( prop->type )
    {
    case STG_MOD_TIME:
      prop = stg_property_attach_data(prop, &world->time, sizeof(world->time));
      break;

    default:
      PRINT_WARN1( "getting world property %s not implemented",
		  stg_property_string( prop->type ) );
      break;
    }

  return prop;
}

// set a property of a world
void stg_world_property_set( stg_world_t* world, stg_property_t* prop )
{
  //PRINT_DEBUG3( "setting world %p property %s with %d bytes of data", 
  //	world, stg_property_string(prop->type), prop->len );
  
  switch( prop->type )
    {
    case STG_MOD_TIME:
      assert( prop->len == sizeof(stg_time_t) );
      //PRINT_DEBUG1( "prop says the world time is %.3f", 
      //	    *(stg_time_t*)prop->data );
      
      world->time =  *(stg_time_t*)prop->data;
      //PRINT_DEBUG1( "setting time. time now %.3f seconds",
      //	    world->time );
      break;
      
    default:
      PRINT_WARN2( "setting world property %s not implemented"
		   " (%d bytes of data)",
		   stg_property_string(prop->type), 
		   prop->len );
      break;
    }
}

// operations on file descriptors ----------------------------------------

// internal low-level functions. user code should probably use one of
// the interafce functions instead.

// attempt to read a packet from [fd]. returns the number of bytes
// read, or -1 on error (and errno is set as per read(2)).
ssize_t stg_fd_packet_read( int fd, void* buf, size_t len )
{
  //PRINT_DEBUG1( "attempting to read a packet of max %d bytes", len );
  
  assert( fd > 0 );
  assert( buf ); // data destination must be good
  
  size_t recv = 0;
  while( recv < len )
    {
      //PRINT_DEBUG3( "reading %d bytes from fd %d into %p", 
      //	    len - recv, fd,  ((char*)buf)+recv );

      /* read will block until it has some bytes to return */
      size_t r = 0;
      if( (r = read( fd, ((char*)buf)+recv,  len - recv )) < 0 )
	{
	  if( errno != EINTR )
	    {
	      PRINT_ERR1( "ReadPacket: read returned %d", r );
	      perror( "system reports" );
	      return -1;
	    }
	}
      else if( r == 0 ) // EOF
	break; 
      else
	recv += r;
    }      
  
  //PRINT_DEBUG2( "read %d/%d bytes", recv, len );
  
  return recv; // the number of bytes read
}
// attempt to write a packet on [fd]. returns the number of bytes
// written, or -1 on error (and errno is set as per write(2)).
ssize_t stg_fd_packet_write( int fd, void* data, size_t len )
{
  size_t writecnt = 0;
  ssize_t thiswritecnt;
  
  //PRINT_DEBUG3( "writing packet on fd %d - %p %d bytes", 
  //	fd, data, len );
  
  while(writecnt < len )
  {
    thiswritecnt = write( fd, ((char*)data)+writecnt, len-writecnt);
      
    // check for error on write
    if( thiswritecnt == -1 )
      return -1; // fail
      
    writecnt += thiswritecnt;
  }
  
  //PRINT_DEBUG2( "wrote %d/%d packet bytes", writecnt, len );
  
  return len; //success
}

// attempt to read a header packet from the client
// returns 0 on success, else a positive error code.
int stg_fd_header_read( int fd, stg_header_t *hdr )
{
  //PRINT_DEBUG( "attempting to read a header" );

  assert(hdr);
  
  ssize_t res = stg_fd_packet_read( fd, hdr, sizeof(stg_header_t) );
  
  if( res < 0 )
    {
      PRINT_ERR1( "failed to read a message header on %d", fd );
      perror( "system error: ");
      return 1;
    }
  
  if( res != sizeof(stg_header_t ) )
    {
      PRINT_ERR3( "failed to read a header on fd %d (%d/%d bytes)",
		  fd, res, sizeof(stg_header_t) ); 
      return 2;
    }
  
  //PRINT_DEBUG2( "read a header of type %s with %d bytes to follow",
  //       stg_message_string(hdr->type), hdr->len );
  
  
  if( !(hdr->type < STG_MSG_COUNT &&  hdr->type > 0) )
    {
      PRINT_ERR1( "header has invalid type (%d)", hdr->type );
      return 3; // error
    }
  
  return 0; // ok
}

// Attempt to read a stg_property_t and subsequent data from the open
// file descriptor [fd]. Caller must free the returned property with
// free(3). Returns a property or NULL on failure.
stg_property_t* stg_fd_property_read( int fd )
{
  stg_property_t* prop = stg_property_create();

  size_t result = stg_fd_packet_read( fd, prop, sizeof(*prop) );
  
  //PRINT_DEBUG2( "read a property header of %d/%d bytes", 
  //	result, propsize );

  if( result == sizeof(*prop) )
    {	
      // see if the property has any data to follow
      if( prop->len > 0 )
	{
	  void* data = NULL;
	  assert( data = malloc(prop->len) );
	  
	  // read all the data
	  result = stg_fd_packet_read( fd, data, prop->len );
	  
	  PRINT_DEBUG2( "read property data of %d/%d bytes", 
			result, prop->len );
	  
	  // copy the data into the property
	  prop = stg_property_attach_data( prop, data, prop->len );

	  free( data );
	}
    } 
  else
    {
      PRINT_WARN2( "short read of property header (%d/%d bytes)", 
		   result, sizeof(stg_property_t) );
      
      stg_property_free(prop);
      prop = NULL; // indicates failure
    }
  return prop; 
}

// attempts to write property [prop] on open file descriptor
// [fd]. returns the number of bytes written, or -1 on failure
// (setting errno as per write(2)).
ssize_t stg_fd_property_write( int fd, stg_property_t* prop )
{
  assert( fd > 0 );
  size_t packetlen =  sizeof(stg_property_t) + prop->len;
  ssize_t result = stg_fd_packet_write( fd, prop, packetlen );
  
  if( result == 1 )
    perror( "error attempting to write a property" );
  else if( result == 0 )
    PRINT_WARN( "attempting to write a property wrote zero bytes" );
  
  return result;
}


//stg_property_t* stg_fd_property_read( int fd );
//ssize_t stg_fd_property_write( int fd, stg_property_t* prop );
//stg_property_t* stg_fd_property_read( int fd );

// miscellaneous  ---------------------------------------------------------

// Look up the color in a database.  (i.e. transform color name to
// color value).  If the color is not found in the database, a bright
// red color will be returned instead.
stg_color_t stg_lookup_color(const char *name)
{
  FILE *file;
  const char *filename;
  
  if( name == NULL ) // no string?
    return 0; // black
  
  if( strcmp( name, "" ) == 0 ) // empty string?
    return 0; // black

  filename = COLOR_DATABASE;
  file = fopen(filename, "r");
  if (!file)
  {
    PRINT_ERR2("unable to open color database %s : %s",
               filename, strerror(errno));
    fclose(file);
    return 0xFFFFFF;
  }
  
  while (TRUE)
  {
    char line[1024];
    if (!fgets(line, sizeof(line), file))
      break;

    // it's a macro or comment line - ignore the line
    if (line[0] == '!' || line[0] == '#' || line[0] == '%') 
      continue;

    // Trim the trailing space
    while (strchr(" \t\n", line[strlen(line)-1]))
      line[strlen(line)-1] = 0;

    // Read the color
    int r, g, b;
    int chars_matched = 0;
    sscanf( line, "%d %d %d %n", &r, &g, &b, &chars_matched );
      
    // Read the name
    char* nname = line + chars_matched;

    // If the name matches
    if (strcmp(nname, name) == 0)
    {
      fclose(file);
      return ((r << 16) | (g << 8) | b);
    }
  }
  PRINT_WARN1("unable to find color [%s]; using default (red)", name);
  fclose(file);
  return 0xFF0000;
}

// print the values in a message packet
void stg_los_msg_print( stg_los_msg_t* msg )
{
  printf( "LosMesg - id: %d power: %d len: %d bytes: %s\n",
	  msg->id, msg->power, msg->len, msg->bytes );
}


