#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <signal.h>

#if HAVE_CONFIG_H
  #include <config.h>
#endif

//#define DEBUG
#include "stage.h"


const char* stg_model_string( stg_model_type_t mod )
{
  switch( mod )
    {
    case STG_MODEL_GENERIC: return "generic"; break;
    case STG_MODEL_WALL: return "wall"; break;
    case STG_MODEL_POSITION: return "position"; break;
    case STG_MODEL_LASER: return "laser"; break;
    case STG_MODEL_SONAR: return "sonar"; break;
    default: break;
    }
  return "unknown"; 
} 

const char* stg_property_string( stg_prop_id_t id )
{
  switch( id )
    {
    case STG_PROP_ENTITY_CREATE: return "STG_PROP_ENTITY_CREATE"; break;
    case STG_PROP_ENTITY_DESTROY: return "STG_PROP_ENTITY_DESTROY"; break;
    case STG_PROP_ENTITY_CIRCLES: return "STG_PROP_ENTITY_CIRCLES"; break;
    case STG_PROP_ENTITY_COLOR: return "STG_PROP_ENTITY_COLOR"; break;
    case STG_PROP_ENTITY_COMMAND: return "STG_PROP_ENTITY_COMMAND"; break;
    case STG_PROP_ENTITY_DATA: return "STG_PROP_ENTITY_DATA"; break;
    case STG_PROP_ENTITY_GEOM: return "STG_PROP_ENTITY_GEOM"; break;
    case STG_PROP_ENTITY_IDARRETURN: return "STG_PROP_ENTITY_IDARRETURN"; break;
    case STG_PROP_ENTITY_LASERRETURN: return "STG_PROP_ENTITY_LASERRETURN"; break;
    case STG_PROP_ENTITY_NAME: return "STG_PROP_ENTITY_NAME"; break;
    case STG_PROP_ENTITY_OBSTACLERETURN: return "STG_PROP_ENTITY_OBSTACLERETURN";break;
    case STG_PROP_ENTITY_ORIGIN: return "STG_PROP_ENTITY_ORIGIN"; break;
    case STG_PROP_ENTITY_PARENT: return "STG_PROP_ENTITY_PARENT"; break;
    case STG_PROP_ENTITY_PLAYERID: return "STG_PROP_ENTITY_PLAYERID"; break;
    case STG_PROP_ENTITY_POSE: return "STG_PROP_ENTITY_POSE"; break;
    case STG_PROP_ENTITY_POWER: return "STG_PROP_SONAR_POWER"; break;
    case STG_PROP_ENTITY_PPM: return "STG_PROP_ENTITY_PPM"; break;
    case STG_PROP_ENTITY_PUCKRETURN: return "STG_PROP_ENTITY_PUCKETURN"; break;
    case STG_PROP_ENTITY_RANGEBOUNDS: return "STG_PROP_ENTITY_RANGEBOUNDS";break;
    case STG_PROP_ENTITY_RECTS: return "STG_PROP_ENTITY_RECTS"; break;
    case STG_PROP_ENTITY_SIZE: return "STG_PROP_ENTITY_SIZE"; break;
    case STG_PROP_ENTITY_SONARRETURN: return "STG_PROP_ENTITY_SONARRETURN"; break;
    case STG_PROP_ENTITY_NEIGHBORRETURN: return "STG_PROP_ENTITY_NEIGHBORRETURN"; break;
    case STG_PROP_ENTITY_NEIGHBORBOUNDS: return "STG_PROP_ENTITY_NEIGHBORBOUNDS"; break;
    case STG_PROP_ENTITY_NEIGHBORS: return "STG_PROP_ENTITY_NEIGHBORS"; break;
    case STG_PROP_ENTITY_SUBSCRIPTION: return "STG_PROP_ENTITY_SUBSCRIPTION"; break;
    case STG_PROP_ENTITY_VELOCITY: return "STG_PROP_ENTITY_VELOCITY"; break;
    case STG_PROP_ENTITY_VISIONRETURN: return "STG_PROP_ENTITY_VISIONRETURN"; break;
    case STG_PROP_ENTITY_VOLTAGE: return "STG_PROP_ENTITY_VOLTAGE"; break;
    case STG_PROP_ENTITY_TRANSDUCERS: return "STG_PROP_ENTITY_TRANSDUCERS";break;
    case STG_PROP_ENTITY_LASER_DATA: return "STG_PROP_ENTITY_LASER_DATA";break;
    case STG_PROP_ENTITY_BLINKENLIGHT: return "STG_PROP_ENTITY_BLINKENLIGHT";break;
    case STG_PROP_ENTITY_NOSE: return "STG_PROP_ENTITY_NOSE";break;

      // remove these
    case STG_PROP_IDAR_RX: return "STG_PROP_IDAR_RX"; break;
    case STG_PROP_IDAR_TX: return "STG_PROP_IDAR_TX"; break;
    case STG_PROP_IDAR_TXRX: return "STG_PROP_IDAR_TXRX"; break;
    case STG_PROP_POSITION_ORIGIN: return "STG_PROP_POSITION_ORIGIN"; break;
    case STG_PROP_POSITION_ODOM: return "STG_PROP_POSITION_ODOM"; break;
    case STG_PROP_POSITION_MODE: return "STG_PROP_POSITION_MODE"; break;
    case STG_PROP_POSITION_STEER: return "STG_PROP_POSITION_STEER"; break;

    default:
      break;
    }
  return "unknown";
}

stg_model_type_t
stg_model_type_from_string( char* str )
{
  stg_model_type_t mod;
 
  if( strcmp( stg_model_string(STG_MODEL_POSITION), str )==0)
    mod = STG_MODEL_POSITION;
  
  else if( strcmp( stg_model_string(STG_MODEL_WALL), str )==0)
    mod = STG_MODEL_WALL;
  
  else if( strcmp( stg_model_string(STG_MODEL_LASER), str )==0)
    mod = STG_MODEL_LASER;
  
  //else if( strcmp( stg_model_string(STG_MODEL_SONAR), str )==0)
  //mod = STG_MODEL_SONAR;
  
  else
    { 
      PRINT_WARN1( "non-specific model created from token \"%s\"", str );
      mod = STG_MODEL_GENERIC;
    }

  return mod;
}


stg_property_t* stg_property_create( void )
{
  stg_property_t* prop;
  
  prop = (stg_property_t*)malloc(sizeof(stg_property_t));

  assert( prop );
  
  memset( prop, 0, sizeof(stg_property_t) ); 
  
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
    {  size_t totalsize = sizeof(stg_property_t)+len;
    
    PRINT_DEBUG1( "attaching %d bytes of data", len );
    
    prop = (stg_property_t*)realloc(prop,totalsize);
    assert( prop );
    
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

  size_t propsize = sizeof(stg_property_t);
  
  PRINT_DEBUG1( "reading a property header of %d bytes", propsize );

  // read a header
  size_t result = stg_packet_read_fd( fd, prop, sizeof(*prop) );
  
  PRINT_DEBUG2( "read a property header of %d/%d bytes", 
		result, propsize );

  if( result == sizeof(*prop) )
    {	
      // see if the property has any data to follow
      if( prop->len > 0 )
	{
	  char* data = NULL;
	  assert( data = (char*)malloc(prop->len) );
	  
	  // read all the data
	  result = stg_packet_read_fd( fd, data, prop->len );
	  
	  PRINT_DEBUG2( "read property data of %d/%d bytes", 
			result, prop->len );

	  // copy the data into the property
	  prop = stg_property_attach_data( prop, data, prop->len );
	}
    } 
  else
    {
      PRINT_ERR2( "short read of property header (%d/%d bytes)", 
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
  return stg_property_read_fd( cli->pollfd.fd );
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
stg_client_t* stg_client_create( char* host, int port )
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
      PRINT_ERR2( "Connection failed on %s:%d ", 
		  info->h_addr_list[0], port ); 
      perror( "" );
      fflush( stdout );
    return NULL;
    }
  
  // send the greeting
  int c = STG_SERVER_GREETING;
  int r;
  if( (r = write( cli->pollfd.fd, &c, sizeof(c) )) < 1 )
    {
      PRINT_ERR( "failed to write STG_SERVER_GREETING to server.\n" );
      if( r < 0 ) perror( "error on write" );
      return NULL;
    }

  // wait for the reply
  if( (r = read( cli->pollfd.fd, &c, sizeof(c) )) < 1 )
    {
      PRINT_ERR( "failed to READ STG_CLIENT_GREETING from server.\n" );
      if( r < 0 ) perror( "error on read" );
      return NULL;
    }

  if( c != STG_CLIENT_GREETING ) 
    PRINT_ERR1( "received bad reply from server (%d)", c );
  else
    PRINT_MSG1( "received good reply from server (%d)", c );

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
  return( stg_property_read( cli ) );  
}  

stg_id_t stg_model_create( stg_client_t* cli, stg_entity_create_t* ent )
{
  stg_property_t* reply = stg_send_property( cli, -1, 
					     STG_PROP_ENTITY_CREATE, 
					     STG_SETGET,
					     ent, sizeof(stg_entity_create_t) );
  stg_id_t returned_id = reply->id;
  stg_property_free( reply );
  
  return returned_id;
}

int stg_model_destroy( stg_client_t* cli, stg_id_t id )
{
  stg_property_t* reply = stg_send_property( cli, id,
					     STG_PROP_ENTITY_DESTROY, 
					     STG_SETGET,
					     NULL, 0 );
  stg_id_t returned_id = reply->id;
  stg_property_free( reply );
  
  return( returned_id == -1 ?  0 : -1 );
}

int stg_model_set_size( stg_client_t* cli, stg_id_t id, stg_size_t* sz )
{
  stg_property_t* reply = stg_send_property( cli, id,
					     STG_PROP_ENTITY_SIZE, 
					     STG_SETGET,
					     sz, sizeof(stg_size_t) );
  memcpy( sz, reply->data, sizeof(stg_size_t) );
  stg_property_free( reply );
  
  return 0;
}

int stg_model_get_size( stg_client_t* cli, stg_id_t id, stg_size_t* sz )
{
  stg_property_t* reply = stg_send_property( cli, id,
					     STG_PROP_ENTITY_SIZE, 
					     STG_GET,
					     NULL, 0 );
  memcpy( sz, reply->data, sizeof(stg_size_t) );
  stg_property_free( reply );
  
  return 0;
}
  
int stg_model_set_velocity( stg_client_t* cli, stg_id_t id, stg_velocity_t* sz )
{
  stg_property_t* reply = stg_send_property( cli, id,
					     STG_PROP_ENTITY_VELOCITY, 
					     STG_SETGET,
					     sz, sizeof(stg_velocity_t) );
  memcpy( sz, reply->data, sizeof(stg_velocity_t) );
  stg_property_free( reply );
  
  return 0;
}

int stg_model_get_velocity( stg_client_t* cli, stg_id_t id, stg_velocity_t* sz )
{
  stg_property_t* reply = stg_send_property( cli, id,
					     STG_PROP_ENTITY_VELOCITY, 
					     STG_GET,
					     NULL, 0 );
  memcpy( sz, reply->data, sizeof(stg_velocity_t) );
  stg_property_free( reply );
  
  return 0;
}

int stg_model_set_pose( stg_client_t* cli, stg_id_t id, stg_pose_t* sz )
{
  stg_property_t* reply = stg_send_property( cli, id,
					     STG_PROP_ENTITY_POSE, 
					     STG_SETGET,
					     sz, sizeof(stg_pose_t) );
  memcpy( sz, reply->data, sizeof(stg_pose_t) );
  stg_property_free( reply );
  
  return 0;
}

int stg_model_get_pose( stg_client_t* cli, stg_id_t id, stg_pose_t* sz )
{
  stg_property_t* reply = stg_send_property( cli, id,
					     STG_PROP_ENTITY_POSE, 
					     STG_GET,
					     NULL, 0 );
  
  memcpy( sz, reply->data, sizeof(stg_pose_t) );
  stg_property_free( reply );
  
  return 0;
}

int stg_model_set_origin( stg_client_t* cli, stg_id_t id, stg_pose_t* org )
{
  stg_property_t* reply = stg_send_property( cli, id,
					     STG_PROP_ENTITY_ORIGIN, 
					     STG_SETGET,
					     org, sizeof(stg_pose_t) );
  
  memcpy( org, reply->data, sizeof(stg_pose_t) );
  stg_property_free( reply );
  
  return 0;
}

int stg_model_get_origin( stg_client_t* cli, stg_id_t id, stg_pose_t* org )
{
  stg_property_t* reply = stg_send_property( cli, id,
					     STG_PROP_ENTITY_ORIGIN, 
					     STG_GET,
					     NULL, 0 );
  
  memcpy( org, reply->data, sizeof(stg_pose_t) );
  stg_property_free( reply );
  
  return 0;
}

int stg_model_set_rects(  stg_client_t* cli, stg_id_t id, 
			  stg_rotrect_array_t* rects )
{
  stg_property_t* reply = stg_send_property( cli, id,
					     STG_PROP_ENTITY_RECTS, 
					     STG_SETGET,
					     rects->rects, 
					     rects->rect_count*
					     sizeof(stg_rotrect_t) );

  // infer the size of rectangles in the data from the size
  memcpy( rects->rects, reply->data, reply->len );
  rects->rect_count = reply->len / sizeof(stg_rotrect_t);
  stg_property_free( reply );
  
  return 0;
}
  

int stg_model_set_transducers( stg_client_t* cli, stg_id_t id, 
			       stg_transducer_t* trans, int count )
{
  stg_property_t* reply = stg_send_property( cli, id, 
					     STG_PROP_ENTITY_TRANSDUCERS, 
					     STG_SETGET,
					     trans, 
					     count *
					     sizeof(stg_transducer_t) );
  
  stg_property_free( reply );
  return 0;
}

int stg_model_get_transducers( stg_client_t* cli, stg_id_t id, 
			       stg_transducer_t** trans, int* count )
{
  stg_property_t* reply = stg_send_property( cli, id, 
					     STG_PROP_ENTITY_TRANSDUCERS, 
					     STG_GET,
					     NULL, 0 ); 
  
  // infer the number of transducers from the data len
  *count = reply->len / sizeof(stg_transducer_t);
  *trans = (stg_transducer_t*)malloc( reply->len ); 
  memcpy( *trans, reply->data, reply->len );
  stg_property_free( reply );
  return 0;
}

int stg_model_set_neighbor_return( stg_client_t* cli, stg_id_t id, 
				    int *val )
{
  stg_property_t* reply = stg_send_property( cli, id, 
					     STG_PROP_ENTITY_NEIGHBORRETURN, 
					     STG_SETGET,
					     val, sizeof(int) ); 
  
  memcpy( val, reply->data, sizeof(int) );
  stg_property_free( reply );
  return 0;
}

int stg_model_set_light( stg_client_t* cli, stg_id_t id, 
			 stg_blinkenlight_t *val)
{
  stg_property_t* reply = stg_send_property( cli, id, 
					     STG_PROP_ENTITY_BLINKENLIGHT,
					     STG_SETGET,
					     val,sizeof(stg_blinkenlight_t));
 
  memcpy( val, reply->data, sizeof(stg_blinkenlight_t) );
  stg_property_free( reply );
  return 0;
}

int stg_model_get_light( stg_client_t* cli, stg_id_t id, 
			 stg_blinkenlight_t *val)
{
  stg_property_t* reply = stg_send_property( cli, id, 
					     STG_PROP_ENTITY_BLINKENLIGHT,
					     STG_GET,
					     NULL, 0 );
 
  memcpy( val, reply->data, sizeof(stg_blinkenlight_t) );
  stg_property_free( reply );
  return 0;
}

int stg_model_set_nose( stg_client_t* cli, stg_id_t id, 
			 stg_nose_t *val)
{
  stg_property_t* reply = stg_send_property( cli, id, 
					     STG_PROP_ENTITY_NOSE,
					     STG_SETGET,
					     val,sizeof(stg_nose_t));
 
  memcpy( val, reply->data, sizeof(stg_nose_t) );
  stg_property_free( reply );
  return 0;
}

int stg_model_get_nose( stg_client_t* cli, stg_id_t id, 
			 stg_nose_t *val)
{
  stg_property_t* reply = stg_send_property( cli, id, 
					     STG_PROP_ENTITY_NOSE,
					     STG_GET,
					     NULL, 0 );
 
  memcpy( val, reply->data, sizeof(stg_nose_t) );
  stg_property_free( reply );
  return 0;
}

int stg_model_set_laser_return( stg_client_t* cli, stg_id_t id, 
				stg_laser_return_t *val)
{
  stg_property_t* reply = stg_send_property( cli, id, 
					     STG_PROP_ENTITY_LASERRETURN, 
					     STG_SETGET,
					     val, sizeof(stg_laser_return_t) ); 
  
  memcpy( val, reply->data, sizeof(stg_laser_return_t) );
  stg_property_free( reply );
  return 0;
}

int stg_model_set_laser_data( stg_client_t* cli, stg_id_t id, 
			      stg_laser_data_t* data )
{
  stg_property_t* reply = stg_send_property( cli, id, 
					     STG_PROP_ENTITY_LASER_DATA, 
					     STG_GET,
					     data, sizeof(stg_laser_data_t) ); 
  
  memcpy( data, reply->data, sizeof(stg_laser_data_t) );
  stg_property_free( reply );
  return 0;
}

int stg_model_get_laser_data( stg_client_t* cli, stg_id_t id, 
			      stg_laser_data_t* data )
{
  stg_property_t* reply = stg_send_property( cli, id, 
					     STG_PROP_ENTITY_LASER_DATA, 
					     STG_GET,
					     NULL, 0 ); 
  
  memcpy( data, reply->data, sizeof(stg_laser_data_t) );
  stg_property_free( reply );
  return 0;
}

int stg_model_set_neighbor_bounds( stg_client_t* cli, stg_id_t id, 
				   stg_bounds_t* data )
{
  stg_property_t* reply = stg_send_property( cli, id, 
					     STG_PROP_ENTITY_NEIGHBORBOUNDS, 
					     STG_SETGET,
					     data, sizeof(stg_bounds_t) ); 
  
  memcpy( data, reply->data, sizeof(stg_bounds_t) );
  stg_property_free( reply );
  return 0;
}

int stg_model_get_neighbor_bounds( stg_client_t* cli, stg_id_t id, 
				   stg_bounds_t* data )
{
  stg_property_t* reply = stg_send_property( cli, id, 
					     STG_PROP_ENTITY_NEIGHBORBOUNDS, 
					     STG_GET,
					     NULL, 0 ); 
  
  memcpy( data, reply->data, sizeof(stg_bounds_t) );
  stg_property_free( reply );
  return 0;
}

// returns an array of neighbors. user must free the array after use
int stg_model_get_neighbors( stg_client_t* cli, stg_id_t id, 
			     stg_neighbor_t** neighbors, int *neighbor_count )
{
  stg_property_t* reply = stg_send_property( cli, id, 
					     STG_PROP_ENTITY_NEIGHBORS, 
					     STG_GET,
					     NULL, 0 ); 
  
  *neighbors = (stg_neighbor_t*)malloc( reply->len );
  memcpy( *neighbors, reply->data, reply->len);
  *neighbor_count = reply->len / sizeof(stg_neighbor_t);
  
  stg_property_free( reply );
  return 0;
}

stg_id_t stg_world_create( stg_client_t* cli, stg_world_create_t* world )
{
  stg_property_t* reply = stg_send_property( cli, -1,
					     STG_PROP_WORLD_CREATE, 
					     STG_SETGET,
					     world, sizeof(stg_world_create_t) );  
  stg_id_t returned_id = reply->id;
  stg_property_free( reply );
  
  return returned_id;
}

int stg_model_subscribe( stg_client_t* cli, stg_id_t id, stg_prop_id_t prop )
{
  stg_subscription_t sub;
  sub.property = prop;
  sub.action = STG_SUBSCRIBE;

  stg_property_t* reply = stg_send_property( cli, id,
					     STG_PROP_ENTITY_SUBSCRIPTION, 
					     STG_SET,
					     &sub, sizeof(sub) );
  return 0;
}

int stg_model_unsubscribe( stg_client_t* cli, stg_id_t id, stg_prop_id_t prop )
{
  stg_subscription_t unsub;
  unsub.property = prop;
  unsub.action = STG_UNSUBSCRIBE;

  stg_property_t* reply = stg_send_property( cli, id,
					     STG_PROP_ENTITY_SUBSCRIPTION, 
					     STG_SET,
					     &unsub, sizeof(unsub) );
  return 0;
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

// RECTANGLES /////////////////////////////////////////////////////////
/*
stg_rect_array_t* stg_rect_array_create( void )
{
  stg_rect_array_t *r = (stg_rect_array_t*)calloc(sizeof(stg_rect_array_t),1);
  assert(r);
  return r;
}

// add a rectangle to the end of the array, allocating memory 
stg_rect_array_t* stg_rect_array_append( stg_rect_array_t* array, 
					 stg_rect_t* rect )
{
  if( array == NULL ) return NULL;
  if( rect == NULL ) return array;
  
  // grow the array by 1 rect
  array->rect_count++;
  array = (stg_rect_array_t*)realloc( array->rects, 
				      array->rect_count * sizeof(stg_rect_t) );
  
  // copy the new rect into the end of the array
  memcpy( array->rects + array->rect_count-1 * sizeof(stg_rect_t), 
	  rect, 
	  sizeof(stg_rect_t) );
  
  return array;
}
    
void stg_rect_array_free( stg_rect_array_t* r )
{
  free( r->rects );
  free( r );
}

*/

// ROTATED ROTRECTANGLES ///////////////////////////////////////////////

stg_rotrect_array_t* stg_rotrect_array_create( void )
{
  stg_rotrect_array_t *r = (stg_rotrect_array_t*)calloc(sizeof(stg_rotrect_array_t),1);
  assert(r);
  return r;
}

// add a rotrectangle to the end of the array, allocating memory 
stg_rotrect_array_t* stg_rotrect_array_append( stg_rotrect_array_t* array, 
					       stg_rotrect_t* rotrect )
{
  if( rotrect == NULL ) return array;
  
  if( array == NULL )
    array = stg_rotrect_array_create();
  
  // grow the array by 1 rotrect
  int numrects = array->rect_count + 1;
  array->rects = (stg_rotrect_t*)realloc( array->rects, 
					  numrects * sizeof(stg_rotrect_t) );
  
  array->rect_count = numrects;
  // copy the new rotrect into the end of the array
  memcpy( &array->rects[numrects-1], rotrect, sizeof(stg_rotrect_t) );
  
  return array;
}
    
void stg_rotrect_array_free( stg_rotrect_array_t* r )
{
  free( r->rects );
  free( r );
}
