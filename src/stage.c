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

/* File: stage.cc
 * Desc: functions for interfacing with the Stage server
 * Author: Richard Vaughan vaughan@hrl.com 
 * Date: 1 June 2003
 *
 * CVS: $Id: stage.c,v 1.22 2003-10-16 02:05:14 rtv Exp $
 */

#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <signal.h>

#if HAVE_CONFIG_H
  #include <config.h>
#endif

//#define DEBUG
#include "stage.h"
//#include <sys/types.h>
//#include <sys/socket.h>

#include <netinet/in.h>


const char* stg_property_string( stg_prop_id_t id )
{
  switch( id )
    {
      // server properties
    case STG_SERVER_TIME:  return "STG_MOD_SERVER_TIME"; break;  
    case STG_SERVER_WORLD_COUNT: return "STG_SERVER_WORLD_COUNT"; break;  
    case STG_SERVER_CLIENT_COUNT: return "STG_SERVER_CLIENT_COUNT"; break;  
    case STG_SERVER_CREATE_WORLD: return "STG_SERVER_CREATE_WORLD"; break;

      // world properties
    case STG_WORLD_TIME: return "STG_WORLD_TIME"; break;  
    case STG_WORLD_MODEL_COUNT:  return "STG_WORLD_MODEL_COUNT"; break;  
    case STG_WORLD_CREATE_MODEL: return "STG_WORLD_CREATE_MODEL"; break;
    case STG_WORLD_DESTROY: return "STG_WORLD_DESTROY"; break;
    case STG_WORLD_SAVE: return "STG_WORLD_SAVE"; break;
    case STG_WORLD_LOAD: return "STG_WORLD_LOAD"; break;
      
      // model properties
    case STG_MOD_DESTROY: return "STG_MOD_DESTROY"; break;
    case STG_MOD_CIRCLES: return "STG_MOD_CIRCLES"; break;
    case STG_MOD_COLOR: return "STG_MOD_COLOR"; break;
    case STG_MOD_COMMAND: return "STG_MOD_COMMAND"; break;
    case STG_MOD_DATA: return "STG_MOD_DATA"; break;
    case STG_MOD_GEOM: return "STG_MOD_GEOM"; break;
    case STG_MOD_IDARRETURN: return "STG_MOD_IDARRETURN"; break;
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
    case STG_MOD_TIME: return "STG_MOD_TIME";break;
	
      // todo
      //case STG_MOD_POSITION_ODOM: return "STG_MOD_POSITION_ODOM"; break;
      //case STG_MOD_POSITION_MODE: return "STG_MOD_POSITION_MODE"; break;
      //case STG_MOD_POSITION_STEER: return "STG_MOD_POSITION_STEER"; break;

    default:
      break;
    }
  return "unknown";
}

stg_property_t* stg_property_create( void )
{
  stg_property_t* prop;
  
  prop = (stg_property_t*)calloc(sizeof(stg_property_t),1);

  assert( prop );
  
  //PRINT_WARN1( "created property at %p", prop );
  return prop;
}

void stg_property_free( stg_property_t* prop )
{
  //PRINT_WARN1( "freeing property at %p", prop );
  free( prop );
}

// lengthen prop by len bytes and copy in the data.
stg_property_t* stg_property_attach_data( stg_property_t* prop,
					  void* data, size_t len )
{
  assert( prop );
  
  PRINT_DEBUG1( "start: prop at %p", prop );
  //stg_property_print( prop );

  if( data && len > 0 )
    {  
      size_t totalsize = sizeof(stg_property_t)+len;
      
      PRINT_DEBUG1( "attaching %d bytes of data", len );
      
      assert( prop = (stg_property_t*)realloc(prop,totalsize) );
      
      /* record the amount of extra data allocated */
      prop->len = len;
      
      /* copy in the data */
      memcpy( prop->data, data, len );
    }
  else
    PRINT_DEBUG( "no data or zero length" );
  
  PRINT_DEBUG1( "result: prop at %p", prop );
  //stg_property_print( prop );
  
  return prop;
}


char* stg_property_sprint( char* str, stg_property_t* prop )
{
  sprintf( str, 
	   "Property @ %p - id: %d time: %.6f prop: %d data bytes: %d\n",
	   prop, prop->id, prop->timestamp, prop->property, prop->len );

  return str;
}

void stg_property_fprint( FILE* fp, stg_property_t* prop )
{
  char str[1000];
  stg_property_sprint( str, prop );
  fprintf( fp, "%s", str );
}

void stg_property_print( stg_property_t* prop )
{
  stg_property_fprint( stdout, prop );
}

size_t stg_packet_read_fd( int fd, void* buf, size_t len )
{
  PRINT_DEBUG1( "attempting to read a packet of max %d bytes", len );
  
  assert( fd > 0 );
  assert( buf ); // data destination must be good
  
  size_t recv = 0;
  // read a header so we know what's coming
  while( recv < len )
    {
      PRINT_DEBUG3( "reading %d bytes from fd %d into %p", 
		    len - recv, fd,  ((char*)buf)+recv );

      /* read will block until it has some bytes to return */
      size_t r = 0;
      if( (r = read( fd, ((char*)buf)+recv,  len - recv )) < 0 )
	{
	  if( errno != EINTR )
	    {
	      PRINT_ERR1( "ReadPacket: read returned %d", r );
	      perror( "system reports" );
	      break;
	    }
	}
      else if( r == 0 ) // EOF
	break; 
      else
	recv += r;
    }      
  
  PRINT_DEBUG2( "read %d/%d bytes", recv, len );
  
  return recv; // the number of bytes read
}

size_t stg_packet_read( stg_client_t* cli, void* buf, size_t len )
{
  return stg_packet_read_fd( cli->pollfd.fd, buf, len );
}
  
void stg_catch_pipe( int signo )
{
  puts( "caught SIGPIPE" );
}

ssize_t stg_packet_write_fd( int fd, void* data, size_t len )
{
  size_t writecnt = 0;
  ssize_t thiswritecnt;
  
  PRINT_DEBUG3( "writing packet on fd %d - %p %d bytes\n", 
		fd, data, len );
  
  while(writecnt < len )
  {
    thiswritecnt = write( fd, ((char*)data)+writecnt, len-writecnt);
      
    // check for error on write
    if( thiswritecnt == -1 )
      return -1; // fail
      
    writecnt += thiswritecnt;
  }
  
  PRINT_DEBUG2( "wrote %d/%d packet bytes\n", writecnt, len );
  
  return len; //success
}

size_t stg_packet_write( stg_client_t* cli, void* data, size_t len )
{
  return stg_packet_write_fd( cli->pollfd.fd, data, len );
}

// return a stg_property_t read from the channel. caller must free the 
// property. On failure the channel is closed and NULL returned.
stg_property_t* stg_property_read_fd( int fd )
{
  stg_property_t* prop = stg_property_create();

#ifdef DEBUG
  size_t propsize = sizeof(stg_property_t);
  PRINT_DEBUG1( "reading a property header of %d bytes", propsize );
#endif

  // read a header
  size_t result = stg_packet_read_fd( fd, prop, sizeof(*prop) );
  
  PRINT_DEBUG2( "read a property header of %d/%d bytes", 
		result, propsize );

  if( result == sizeof(*prop) )
    {	
      // see if the property has any data to follow
      if( prop->len > 0 )
	{
	  void* data = NULL;
	  assert( data = malloc(prop->len) );
	  
	  // read all the data
	  result = stg_packet_read_fd( fd, data, prop->len );
	  
	  PRINT_DEBUG2( "read property data of %d/%d bytes", 
			result, prop->len );

	  // copy the data into the property
	  prop = stg_property_attach_data( prop, data, prop->len );

	  free( data );
	}
    } 
  else
    {
      PRINT_DEBUG2( "short read of property header (%d/%d bytes)", 
		    result, propsize );
      
      stg_property_free(prop);
      prop = NULL; // indicates failure
    }
  return prop; 
}

// return a stg_property_t read from the client. caller must free the 
// property. On failure the channel is closed and NULL returned.
stg_property_t* stg_property_read( stg_client_t* cli )
{
  stg_property_t* prop = stg_property_read_fd( cli->pollfd.fd );

  // the property contains the latest Stage time - poke it into
  // the client
  if( prop )
    memcpy( &cli->time, &prop->timestamp, sizeof(stg_timeval_t) );
  
  return prop;
}

// returns 0 on success, else -1
ssize_t stg_property_write_fd( int fd, stg_property_t* prop )
{
  assert( fd > 0 );
  size_t packetlen =  sizeof(stg_property_t) + prop->len;
  ssize_t result = stg_packet_write_fd( fd, prop, packetlen );
  
  if( result == 1 )
    perror( "error attempting to write a property" );
  else if( result == 0 )
    PRINT_WARN( "attempting to write a property wrote zero bytes" );
  
  return result;
}

ssize_t stg_property_write( stg_client_t* cli, stg_property_t* prop )
{
  return( stg_property_write_fd( cli->pollfd.fd, prop ) ); 
}

void stg_client_free( stg_client_t* cli )
{
  close( cli->pollfd.fd );
}

// returns a pointer to an allocated poll structure attached to the
// Stage server - use stg_destroy_client() to free the connection
stg_client_t* stg_client_create( char* host, int port, stg_tos_t tos )
{
  if( host == NULL )
    host = "localhost";

  if( port < 1 )
    port = STG_DEFAULT_SERVER_PORT;

  stg_client_t *cli = (stg_client_t*)malloc( sizeof( stg_client_t ) );
  assert( cli );
  
  memset( cli, 0, sizeof(*cli) );

  // get the IP of our host
  struct hostent* info = gethostbyname(  host );
  
  if( info )
    { // make sure this looks like a regular internet address
      assert( info->h_length == 4 );
      assert( info->h_addrtype == AF_INET );
    }
  else
    {
      PRINT_ERR1( "failed to resolve IP for remote host\"%s\"\n", 
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
  greeting.pid = getpid();
  
  int r;
  if( (r = write( cli->pollfd.fd, &greeting, sizeof(greeting) )) < 1 )
    {
      PRINT_ERR( "failed to write STG_SERVER_GREETING to server.\n" );
      if( r < 0 ) perror( "error on write" );
      return NULL;
    }

  //stg_tos_t tos;
  if( (r = write( cli->pollfd.fd, &tos, sizeof(tos) )) < 1 )
    {
      PRINT_ERR( "failed to write type-of-service specifier to server.\n" );
      if( r < 0 ) perror( "error on write" );
      return NULL;
    }
  
  // wait for the reply
  if( (r = read( cli->pollfd.fd, &greeting, sizeof(greeting) )) < 1 )
    {
      PRINT_ERR( "failed to READ STG_CLIENT_GREETING from server.\n" );
      if( r < 0 ) perror( "error on read" );
      return NULL;
    }
  
  if( greeting.code != STG_CLIENT_GREETING ) 
    PRINT_ERR1( "received bad reply from server (%d)", greeting.code );
  else
    PRINT_DEBUG1( "received good reply from server (%d)", greeting.code );

  return cli;
}  

stg_property_t* stg_send_property( stg_client_t* cli,
				   int id, 
				   stg_prop_id_t type,
				   stg_prop_action_t action,
				   void* data, 
				   size_t len )
{
  stg_property_t* prop =  stg_property_create();
  prop->id = id;
  prop->timestamp = 1.0;
  prop->property = type;
  prop->action = action;
  
  prop = stg_property_attach_data( prop, data, len);  
  stg_property_write( cli, prop );
  stg_property_free( prop );

  return( stg_property_read( cli ));  
}  

int stg_exchange_property( stg_client_t* cli,
			   int id, 
			   stg_prop_id_t type,
			   stg_prop_action_t action,
			   void *set_data,
			   size_t set_len,
			   void **get_data, 
			   size_t *get_len )
{
  stg_property_t* reply = 
    stg_send_property( cli, id, type, action, set_data, set_len );
  
  // initially zero the reply data
  if( get_data && get_len )
    {
      *get_data = NULL;
      *get_len = 0;
    }
  
  if( reply == NULL ) // fail
      return -1;
  
  // if some data was contained in the reply, allocate space for it
  // and return it in the get_data pointer
  if( reply->len > 0 && get_data && get_len )
    {
      *get_data = calloc( reply->len, 1 );
      memcpy( *get_data, reply->data, reply->len );
      *get_len = reply->len;
    }

  stg_property_free(reply);
  return 0;
}

// set a property of the model with the given id. 
// returns 0 on success, else -1.
int stg_set_property( stg_client_t* cli,
		      int id, 
		      stg_prop_id_t type,
		      void* data, 
		      size_t len )
{
  return stg_exchange_property( cli, id, type, STG_SET, 
				data, len, NULL, NULL );
}

// gets the requested data from the server, allocating memory for the packet.
// caller must free() the data
// returns 0 on success, else -1.
int stg_get_property( stg_client_t* cli,
		      int id, 
		      stg_prop_id_t type,
		      void **data, 
		      size_t *len )
{
  return stg_exchange_property( cli, id, type, STG_GET, 
				NULL, 0, data, len );
}

// sends the set_data packet and gets the get_data packet requested
// data from the server, allocating memory for the packet.  caller
// must free() the get_data. returns 0 on success, else -1.
int stg_setget_property( stg_client_t* cli,
			 int id, 
			 stg_prop_id_t type,
			 void *set_data,
			 size_t set_len,
			 void **get_data, 
			 size_t *get_len )
{
  return stg_exchange_property( cli, id, type, STG_SETGET, 
				set_data, set_len, get_data, get_len );
}

// sends the set_data packet and gets the get_data packet requested
// data from the server, allocating memory for the packet.  caller
// must free() the get_data. returns 0 on success, else -1.
int stg_getset_property( stg_client_t* cli,
			 int id, 
			 stg_prop_id_t type,
			 void *set_data,
			 size_t set_len,
			 void **get_data, 
			 size_t *get_len )
{
  return stg_exchange_property( cli, id, type, STG_GETSET, 
				set_data, set_len, get_data, get_len );
}

void stg_los_msg_print( stg_los_msg_t* msg )
{
  printf( "Mesg - id: %d power: %d len: %d bytes: %s\n",
	  msg->id, msg->power, msg->len, msg->bytes );
}


stg_id_t stg_world_create( stg_client_t* cli, stg_world_create_t* world )
{
  stg_property_t* reply = stg_send_property( cli, -1,
					     STG_SERVER_CREATE_WORLD, 
					     STG_COMMAND,
					     world, 
					     sizeof(stg_world_create_t));  
  
  if( !(reply && (reply->action == STG_ACK) ))
    {
      PRINT_ERR( "stage1p4: create world failed" );
      exit(-1);
    }
  
  stg_id_t returned_id = reply->id;
  stg_property_free( reply );
  
  return returned_id;
}


int stg_property_subscribe( stg_client_t* cli, 
			    stg_subscription_t* sub )
{
  assert( cli );
  assert( sub );

  ssize_t res = write( cli->pollfd.fd, sub, sizeof(stg_subscription_t)); 
  
  if( res < 0 ) 
    perror("write subscription failed");
  if( res < sizeof(stg_subscription_t)) 
    PRINT_ERR2("write subscription wrote %d/%d bytes", 
	       res, sizeof(stg_subscription_t) );
  
  return 0;//ok
}






///////////////////////////////////////////////////////////////////////////
// Look up the color in a data based (transform color name -> color value).
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
