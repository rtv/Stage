
#if HAVE_CONFIG_H
  #include <config.h>
#endif
#if HAVE_STRINGS_H
  #include <strings.h>
#endif

//#define MALLOC_CHECK_ 1

#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <termios.h>
#include <unistd.h>
#include <assert.h>

//#undef DEBUG 
#undef VERBOSE
#define DEBUG

#include "replace.h"
#include "sio.h"


// these vars are used only in this file (hence static)

static int server_port = STG_DEFAULT_SERVER_PORT;
static char server_host[STG_HOSTNAME_MAX] = "localhost";

// the master-server's listening socket
static struct pollfd listen_poll;

// the current connection details
static struct pollfd connection_polls[ STG_MAX_CONNECTIONS ];
static int sio_connection_count;

int SIOGetConnectionCount()
{
  return sio_connection_count;
}

///////////////////////////////////////////////////////////////////////////
// Print the usage string
void SIOPrintUsage( void )
{
  printf("\nUsage: client [options]\n"
	 "Options: <argument> [default]\n"
	 " -h\t\tPrint this message\n" 
	 " -h <hostname>\tRun as a client to a Stage server on hostname\n"
	 "See the Stage manual for details.\n"
	 "\nPart of the Player/Stage Project [http://playerstage.sourceforge.net].\n"
	 "Copyright 2000-2003 Richard Vaughan, Andrew Howard, Brian Gerkey and contributors\n"
	 "Released under the GNU General Public License"
	 " [http://www.gnu.org/copyleft/gpl.html].\n"
	 "\n"
	 );
}

// sends a buffer full of properties and places any replies in the
// recv buffer (if non-null)
int SIOPropertyUpdate( int con, double timestamp, 
		       stage_buffer_t* send, stage_buffer_t* recv  )
{
  if( send == NULL )
    { 
      PRINT_ERR( "send buffer is null" );
      return -1;
    }
  
  PRINT_DEBUG( "writing properties" );
  
  // send the properties
  assert( SIOWriteMessage( con, timestamp, STG_HDR_PROPS, send->data, send->len )
	  == 0 );
  
  PRINT_DEBUG( "reading replies" );

  stage_buffer_t* dbg = SIOCreateBuffer();
  
  // the next thing to arrive should be our reply
  stage_header_t hdr;

  hdr.len = 0;
  //while( hdr.len == 0 )
    {
      size_t hdrbytes = SIOReadPacket( con, &hdr, sizeof(hdr) ); 
      if( hdr.type != STG_HDR_PROPS )
	{
	  PRINT_ERR2( "unexpected header type %s (%d)", 
		      SIOHdrString(hdr.type), hdr.type );
	}
      
      //SIOBufferPacket( dbg, &hdr, sizeof(hdr) );
      //SIODebugBuffer( dbg );
      //SIOFreeBuffer( dbg );
      
      PRINT_DEBUG3( "read %d byte reply header type %s %d bytes  follow", 
		    	    hdrbytes, SIOHdrString(hdr.type), hdr.len ); 
    }

  // make room for the incoming properties and read them
  char* buf = calloc(hdr.len,1);
  size_t databytes = SIOReadPacket( con, buf, hdr.len );
  assert( databytes == hdr.len );
  
  // if the caller is interested in the relies, copy the data we
  // read into the receive buffer
  if( recv ) SIOBufferPacket( recv, buf, hdr.len );
  
  free( buf );

  return 0; // success
}


int SIOCreateModels( int con, double timestamp, stage_model_t* models, int num_models )
{
  stage_buffer_t* request = SIOCreateBuffer();
  stage_buffer_t* reply = SIOCreateBuffer();
  
  
  // package the models as a property request to root
  SIOBufferProperty( request, 0, STG_PROP_ROOT_CREATE,
		     models, num_models * sizeof(stage_model_t), STG_WANTREPLY );
  
  // send the requests and read the reply
  // the reply buffer will contain the models with their ids filled in
  if( SIOPropertyUpdate( con, timestamp, request, reply ) == -1 )
    {
      PRINT_ERR( "failed to swap model requests with server" );
      return -1;
    }
  
  SIODebugBuffer( reply );
  
  PRINT_DEBUG2( "reply[0] type %s len %lu",
		SIOPropString(((stage_property_t*)(reply->data))->property),
		(unsigned long)((stage_property_t*)reply->data)->len );
  
  // the reply should be the same size as the request, as it should
  // contain the same models with their id set correctly
  assert( request->len == reply->len );
  
  stage_property_t* prop = (stage_property_t*)(reply->data);
  stage_model_t* newmodels = (stage_model_t*)(reply->data + sizeof(stage_property_t));
  
  PRINT_DEBUG1( "reply prop %s", SIOPropString(prop->property) );
  
  int m;
  for( m=0; m<num_models; m++ )
    PRINT_DEBUG3( "new model token %s id %d parent %d",
		  newmodels[m].token, newmodels[m].id, newmodels[m].parent_id );
  
  // copy the reply into the original buffer
  memcpy( models, newmodels, num_models * sizeof(stage_model_t) );  
  
  SIOFreeBuffer( request );
  SIOFreeBuffer( reply );
  
  return 0; // success
}

const char* SIOPropString( stage_prop_id_t id )
{
  switch( id )
    {
    case STG_PROP_ENTITY_CIRCLES: return "STG_PROP_ENTITY_CIRCLES"; break;
    case STG_PROP_ENTITY_COLOR: return "STG_PROP_ENTITY_COLOR"; break;
    case STG_PROP_ENTITY_COMMAND: return "STG_PROP_ENTITY_COMMAND"; break;
    case STG_PROP_ENTITY_DATA: return "STG_PROP_ENTITY_DATA"; break;
    case STG_PROP_ENTITY_IDARRETURN: return "STG_PROP_ENTITY_IDARRETURN"; break;
    case STG_PROP_ENTITY_LASERRETURN: return "STG_PROP_ENTITY_LASERRETURN"; break;
    case STG_PROP_ENTITY_NAME: return "STG_PROP_ENTITY_NAME"; break;
    case STG_PROP_ENTITY_OBSTACLERETURN: return "STG_PROP_ENTITY_OBSTACLERETURN";break;
    case STG_PROP_ENTITY_ORIGIN: return "STG_PROP_ENTITY_ORIGIN"; break;
    case STG_PROP_ENTITY_PLAYERID: return "STG_PROP_ENTITY_PLAYERID"; break;
    case STG_PROP_ENTITY_POSE: return "STG_PROP_ENTITY_POSE"; break;
    case STG_PROP_ENTITY_PPM: return "STG_PROP_ENTITY_PPM"; break;
    case STG_PROP_ENTITY_PUCKRETURN: return "STG_PROP_ENTITY_PUCKETURN"; break;
    case STG_PROP_ENTITY_RECTS: return "STG_PROP_ENTITY_RECTS"; break;
    case STG_PROP_ENTITY_SIZE: return "STG_PROP_ENTITY_SIZE"; break;
    case STG_PROP_ENTITY_SONARRETURN: return "STG_PROP_ENTITY_SONARRETURN"; break;
    case STG_PROP_ENTITY_SUBSCRIBE: return "STG_PROP_ENTITY_SUBSCRIBE"; break;
    case STG_PROP_ENTITY_UNSUBSCRIBE: return "STG_PROP_ENTITY_UNSUBSCRIBE"; break;
    case STG_PROP_ENTITY_VELOCITY: return "STG_PROP_ENTITY_VELOCITY"; break;
    case STG_PROP_ENTITY_VISIONRETURN: return "STG_PROP_ENTITY_VISIONRETURN"; break;
    case STG_PROP_ENTITY_VOLTAGE: return "STG_PROP_ENTITY_VOLTAGE"; break;
    case STG_PROP_ROOT_CREATE: return "STG_PROP_ROOT_CREATE"; break;
    case STG_PROP_ROOT_GUI: return "STG_PROP_ROOT_GUI"; break;
    case STG_PROP_SONAR_GEOM: return "STG_PROP_SONAR_GEOM"; break;
    case STG_PROP_SONAR_POWER: return "STG_PROP_SONAR_POWER"; break;
    case STG_PROP_SONAR_RANGEBOUNDS: return "STG_PROP_SONAR_RANGEBOUNDS";break;
    case STG_PROP_IDAR_TXRX: return "STG_PROP_IDAR_TXRX"; break;
    case STG_PROP_IDAR_TX: return "STG_PROP_IDAR_TX"; break;
    case STG_PROP_IDAR_RX: return "STG_PROP_IDAR_RX"; break;
    case STG_PROP_ENTITY_PARENT: return "STG_PROP_ENTITY_PARENT"; break;
    default:
      break;
    }
  return "unknown";
}

void SIOPackPose( stage_pose_t *pose, double x, double y, double a )
{
  pose->x = x;
  pose->y = y;
  pose->a = a;
}

void CatchSigPipe( int signo )
{
#ifdef VERBOSE
  puts( "** SIGPIPE! **" );
#endif
  exit( -1 );
}


void ClientCatchSigPipe( int signo )
{
#ifdef VERBOSE
  puts( "** SIGPIPE! **" );
#endif
}

void SIOTimeStamp( stage_header_t* hdr, double simtime )
{
  double seconds = floor( simtime );
  double usec = (simtime - seconds) * MILLION; 
  
  hdr->sec = (int)seconds; 
  hdr->usec = (int)usec;
}

void SIODebugBuffer( stage_buffer_t* buf )
{
  assert( buf );
  
  size_t c;
  
  PRINT_DEBUG3( "stage buffer at %p data: %p len: %d bytes: ", 
		buf, buf->data, buf->len );
  
  if( buf->len == 0 )
    printf( "<none>" );
  
  for( c=0; c<buf->len; c++ )
    {
      int v = buf->data[c];
      printf( "%x ", v);
    }
  
  puts( "" );
}



size_t SIOBufferPacket( stage_buffer_t* buf, void* data, size_t len )
{
  // make space for the new packet (this could be more efficient if
  // we grew the buffer in large chunks less frequently, but this
  // will do for now)
  size_t new_len = buf->len + len;
  
  buf->data = realloc( buf->data, new_len );
  
  // add the new packet to the end of the buffer
  memcpy( buf->data + buf->len, data, len );
  
  // remember how big the buffer has become
  buf->len = new_len;

  return len; // number of bytes added to buffer
}


size_t SIOWritePacket( int con, void* data, size_t len )
{
  size_t writecnt = 0;
  int thiswritecnt;
 
  //PRINT_DEBUG3( "writing packet on connection %d - %p %d bytes\n", 
  //	con, data, len );
  
  while(writecnt < len )
  {
    thiswritecnt = write( connection_polls[con].fd, 
			  data+writecnt, len-writecnt);
      
    // check for error on write
    if( thiswritecnt == -1 )
      return -1; // fail
      
    writecnt += thiswritecnt;
  }
  
  //PRINT_DEBUG2( "wrote %d/%d packet bytes\n", writecnt, len );
  
  return len; //success
}

// write out the contents of the buffer
size_t SIOWriteBuffer( int con, stage_buffer_t* buf )
{
  //printf( "writing buffer on connection %d - %p %d bytes\n", 
  //  con, buf->data, buf->len );
  
  return( SIOWritePacket( con, buf->data, buf->len ) );
}


size_t SIOReadPacket( int con, void* buf, size_t len )
{
  //printf( "attempting to read a packet of max %d bytes\n", len );

  assert( buf ); // data destination must be good

  int recv = 0;
  // read a header so we know what's coming
  while( recv < (int)len )
    {
    //printf( "Reading on connection %d\n", con );
    
    /* read will block until it has some bytes to return */
    size_t r = read( connection_polls[con].fd, buf+recv,  len - recv );
    
    if( r == -1 ) // ERROR
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
  
  //printf( "read %d/%d bytes\n", recv, len );

  return recv; // the number of bytes read
}

const char* SIOHdrString( stage_header_type_t type )
{
  switch( type )
    {
    case STG_HDR_CONTINUE: return "STG_HDR_CONTINUE";
    case STG_HDR_MODEL: return "STG_HDR_MODEL";
    case STG_HDR_PROPS: return "STG_HDR_PROPS";
    case STG_HDR_CMD: return "STG_HDR_CMD";
    case STG_HDR_GUI: return "STG_HDR_GUI";
    default: return "unknown";
    }
}

/// COMPOUND FUNCTIONS
int SIOWriteMessage( int con, double simtime, stage_header_type_t type,  
		     void* data, size_t len )
{
  //PRINT_DEBUG3( "con %d: type: %s bytes: %d", con, SIOHdrString(type), len );

  // create a header
  stage_header_t hdr;
  hdr.type = type;
  hdr.len = len;
  SIOTimeStamp( &hdr, simtime );
  
  stage_buffer_t* outbuf = SIOCreateBuffer();
  assert( outbuf );
  
  // buffer the header, then the data
  SIOBufferPacket( outbuf, &hdr, sizeof(hdr) );
  SIOBufferPacket( outbuf, data, len );
  
  //SIODebugBuffer( outbuf );

  if( con == -1 ) // if ALL CONNECTIONS was requested
    {
      int c;
      for( c=0; c < sio_connection_count; c++ )
	{
	  // then write buffer out on the connection
	  if( SIOWriteBuffer( c, outbuf ) != outbuf->len )
	    {
	      puts( "Failed to write model requests" );
	      SIOFreeBuffer( outbuf );
	      return -1; // fail
	    }
	  //PRINT_DEBUG1( "ok on con %d", c);
	}
    }
  else // a specific connection was requested
    {
      // then write buffer out on the connection
      if( SIOWriteBuffer( con, outbuf ) != outbuf->len )
	{
	  puts( "Failed to write model requests" );
	  SIOFreeBuffer( outbuf );
	  return -1; // fail
	}
      //PRINT_DEBUG1( "ok on con %d", con);
    }
  
  SIOFreeBuffer( outbuf );
  return 0; //success
}

int SIOReadData( int con, 
		 size_t datalen, 
		 size_t recordlen, 
		 stg_data_callback_t callback )
{	       
  char* buf = calloc(datalen,1);
  assert(buf);
  
  double num_packets = (double)datalen / (double)recordlen;
  //PRINT_DEBUG3( "attempting to read %d bytes (%.2f records) on con %d", 
  //	datalen, num_packets, con );
  
  size_t bytes_read;
  if( (bytes_read = SIOReadPacket( con, buf,datalen )) != datalen )
    {
      PRINT_WARN2( "short read %d/%d bytes", bytes_read, datalen );
      free( buf );
      return -1; // fail
    }
 
  stage_buffer_t* replies = SIOCreateBuffer();

  // handle each packet as an individual request
  char* record;
  for( record = buf; record < buf + datalen; record += recordlen )
    {
      //printf( "Passing packet to callback %p\n", callback );
      
      // CALL THE CALLBACK WITH THIS SINGLE config
      if( callback )
	(*callback)( con, record, recordlen, replies );
    }
  
  free( buf );
  
  // send the replies
  //assert( SIOWriteMessage( con, 0.0,
  //		   STG_HDR_PROPS, replies->data, replies->len )
  //  == 0 );
  
  SIOFreeBuffer( replies );

  return 0;
}

// read a buffer len bytes long from fd, then call the property handling callback
// for each property packed into the buffer
int SIOReadProperties( int con, size_t len, stg_data_callback_t callback )
{
  if( len == 0 )
    {
      //PRINT_WARN( "Reading ZERO properties" );
      return 0; // success
    }

  // allocate space for the data
  char* prop_buf = (char*)calloc( len, 1 );
  assert( prop_buf );
  
  // read in the right number of bytes
  size_t bytes_read;
  if( (bytes_read = SIOReadPacket( con, prop_buf, len )) != len )
    {
      free( prop_buf );
      return -1; // fail
    }
  

  // create a buffer for replies
  stage_buffer_t* replies = SIOCreateBuffer();

  stage_buffer_t* this_reply;

  // the first property header is at the start of the buffer
  char* prop_header = prop_buf;
  
  // iterate through the buffer
  while( prop_header < (prop_buf + len) )
    {
      // parse the header
      stage_property_t* prop = (stage_property_t*)prop_header;
      
      //PRINT_DEBUG2( "Read property id %d len %d\n", prop->id, prop->len );
      
      // CALL THE CALLBACK WITH THIS SINGLE PROPERTY
      //PRINT_DEBUG2( "calling callback for property %s, reply %d",
      //	    SIOPropString(prop->property), prop->reply );
      
      // if this prop requests a reply
      if( prop->reply == STG_WANTREPLY )
	{
	  PRINT_DEBUG2( "reply requested for ent %d prop %s", 
			prop->id, SIOPropString(prop->property) );
	  
	  this_reply = replies; // we sent a pointer to the reply buffer
	}
      else
	this_reply = NULL; // if not, we send a null pointer
      
      if( callback )
	(*callback)( con, (char*)prop,  sizeof(stage_property_t)+prop->len,
		     this_reply );

      // move the pointer past this record in the buffer
      prop_header += sizeof(stage_property_t) + prop->len;
    }

  //PRINT_DEBUG( "reply buffer" );
  //SIODebugBuffer( replies );
  //PRINT_DEBUG1( "writing %d bytes of props",  replies->len );
  
  // send the replies
  assert( SIOWriteMessage( con, 0.0,
			   STG_HDR_PROPS, replies->data, replies->len )
	  == 0 );
  
  SIOFreeBuffer( replies );
  
  // free the memory
  free( prop_buf );


  return 0; // success
}


// returns an connection number of a poll structure attached to the
// Stage server
int SIOInitClient( int argc, char** argv )
{
  int con = 0; // the connection number

  sio_connection_count = 0;

  int a;
  // parse out the hostname - that's all we need just here
  // (the parent stageio object gets the port)
  for( a=1; a<argc; a++ )
  {
    // get the hostname, overriding the default
    if( strcmp( argv[a], "-c" ) == 0 )
    {
      // if -c is the last argument the cmd line is bad
      if( a == argc-1 )
	{
	  SIOPrintUsage();
	  return -1;
	}
     
      // the next argument is the hostname
      strncpy( server_host, argv[a+1], STG_HOSTNAME_MAX);
      
      a++;
    }
    
  }
  
  // reassuring console output
  //printf( "[Connecting to %s:%d]", server_host, server_port );
  //puts( "" );

  // connect to the remote server and download the world data
  
  // get the IP of our host
  struct hostent* info = gethostbyname( server_host );
  
  if( info )
    { // make sure this looks like a regular internet address
      assert( info->h_length == 4 );
      assert( info->h_addrtype == AF_INET );
    }
  else
    {
      PRINT_ERR1( "failed to resolve IP for remote host\"%s\"\n", 
		  server_host );
      return -1;
    }
  struct sockaddr_in servaddr;
  
  /* open socket for network I/O */
  connection_polls[con].fd = socket(AF_INET, SOCK_STREAM, 0);
  connection_polls[con].events = POLLIN; // notify me when data is available
  
  // switch on the re-use-address option
  const int on = 1;
  setsockopt( connection_polls[con].fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on) );

  //  printf( "POLLFD = %d\n", m_pose_connections[con].fd );
  
  if( connection_polls[con].fd < 0 )
    {
      PRINT_ERR1( "Error opening network socket for connection", con );
      fflush( stdout );
      return -1;
    }
  
  /* setup our server address (type, IP address and port) */
  bzero(&servaddr, sizeof(servaddr)); /* initialize */
  servaddr.sin_family = AF_INET;   /* internet address space */
  servaddr.sin_port = htons( server_port ); /*our command port */ 
  memcpy(&(servaddr.sin_addr), info->h_addr_list[0], info->h_length);
  
  if( connect( connection_polls[con].fd, 
               (struct sockaddr*)&servaddr, sizeof( servaddr) ) == -1 )
  {
    PRINT_ERR2( "Connection failed on %s:%d ", 
		info->h_addr_list[0], server_port ); 
    perror( "" );
    fflush( stdout );
    return -1;
  }
  // send the connection type byte - we want an asynchronous connection
  
  char c = STAGE_HELLO;
  int r;
  if( (r = write( connection_polls[con].fd, &c, 1 )) < 1 )
    {
      PRINT_ERR( "failed to write STAGE_HELLO greeting byte to server.\n" );
      if( r < 0 ) perror( "error on write" );
    return -1;
  }
  
  // a client has just the one connection (for now)
  sio_connection_count = 1;

  return con;
}  

// read stuff until we get a continue message on each channel
int SIOServiceConnections(   stg_connection_callback_t lostconnection_callback,
			     stg_data_callback_t cmd_callback, 
			     stg_data_callback_t model_callback,
			     stg_data_callback_t prop_callback,
			     stg_data_callback_t gui_callback )
{
  // we want poll to block until it is interrupted by a timer signal,
  // so we give it a time-out of -1.
  int timeout = -1;
  int readable = 0;
  int syncs = 0;  
  
  // we loop on a poll until have a sync from all clients or we run
  // out of connections
  while(  (sio_connection_count > 0) && (syncs < sio_connection_count) ) 
    {
      // use poll to see which pose connections have data
      if((readable = 
	  poll( connection_polls,
		sio_connection_count,
		timeout )) == -1) 
	{
	  PRINT_ERR( "poll(2) returned error)");
	  perror("");
	  return -1; // fail
	}
      
      if( readable > 0 )
	{
	  int t;
	  for( t=0; t<sio_connection_count; t++ )// all the connections
	    {
	      short revents = connection_polls[t].revents;
	      //int confd = connection_polls[t].fd;
	      
	      if( revents & POLLIN )// data available
		{ 
		  //printf( "poll() says data available "
		  //  "(POLLIN) on connection %d\n", t );
		  
		  int hdrbytes;
		  stage_header_t hdr;
		  
		  hdrbytes = SIOReadPacket( t, &hdr, sizeof(hdr) ); 
		  
		  if( hdrbytes < (int)sizeof(hdr) )
		    {
		      PRINT_DEBUG1( "con %d: failed header read. closing.",t );
		      SIODestroyConnection( t ); // zap this connection
		      (*lostconnection_callback)( t );
		    }
		  else // find out the type of packet to follow
		    {  // and handle it
		      //PRINT_DEBUG2( "con %d header reports %d bytes follow",
		      //	    t, hdr.len );
			
		      //PRINT_DEBUG2( "con %d time %.4f header received",
		      //	    t, (double)hdr.sec + (double)(hdr.usec) / MILLION );

			switch( hdr.type )
			  {
			  case STG_HDR_GUI: // a gui config packet is coming in 
			    //PRINT_DEBUG1( "STG_HDR_GUI on %d", t );
			    SIOReadData( t, hdr.len, 
					 sizeof(stage_gui_config_t), 
					 gui_callback );
			    break;
			    
			  case STG_HDR_MODEL:
			    PRINT_DEBUG1( "STG_HDR_MODEL on %d", t );
			    SIOReadData( t, hdr.len, 
					 sizeof(stage_model_t), 
					 model_callback );
			    break;
			    
			  case STG_HDR_PROPS: // some poses are coming in 
			    //PRINT_DEBUG1( "STG_HDR_PROPS on %d", t );
			    SIOReadProperties( t, hdr.len, prop_callback );
			    break;
			    
			    
			  case STG_HDR_CMD:
			    //PRINT_DEBUG1( "STG_HDR_CMD on %d", t );
			    if( cmd_callback )
			      (*cmd_callback)( t, &(hdr.len), 1, NULL );
			    break;			  
			    
			  case STG_HDR_CONTINUE: 
			    //PRINT_DEBUG1( "STG_HDR_CONTINUE on %d", t );
			    syncs++;
			    break;
			    
			  default:
			    PRINT_WARN2( "Unknown header type (%d) on %d",
					 hdr.type, t  );
			  }
		    }
		}
	      // if poll reported some badness on this connection
	      else if( !revents & EINTR ) //
		{
		  PRINT_MSG1( "connection %d seems bad", t );
		  SIODestroyConnection( t ); // zap this connection
		}    
	    }
	}
    }    
  
#ifdef VERBOSE
  PRINT_DEBUG( "End");
#endif

  return 0;
}


int SIOParseCmdLine( int argc, char** argv )
{
  int a;
  
  for( a=1; a<argc-1; a++ )
    {
      /*      // FAST MODE - run as fast as possible - don't attempt t match real time */
      /*      if((strcmp( argv[a], "--fast" ) == 0 ) ||  */
      /*         (strcmp( argv[a], "-f" ) == 0)) */
      /*      { */
      /*        m_real_timestep = 0.0; */
      /*        printf( "[Fast]" ); */
      /*      } */
      
      /*      // set the stop time */
      /*      if(!strcmp(argv[a], "-t")) */
      /*        { */
      /*  	m_stoptime = atoi(argv[++a]); */
      /*  	printf("[Stop time: %d]",m_stoptime); */
      /*        } */
      
      /*      // START WITH CLOCK STOPPED */
      /*      if( strcmp( argv[a], "-s" ) == 0 ) */
      /*        { */
      /*  	this->start_disabled = true; */
       /*  	printf( "[Clock stopped (start with SIGUSR1)]" ); */
       /*        } */
     }

   return 0; // success
 }


 void SIODestroyConnection( int con )
 {

   PRINT_DEBUG1( "Closing connection %d", con );

   close( connection_polls[con].fd );

   sio_connection_count--;

   // shift the rest of the array 1 place left
   int p;
   for( p=con; p<sio_connection_count; p++ )
     {
       // the pollfd array
       memcpy( &(connection_polls[p]), 
	       &(connection_polls[p+1]),
	       sizeof( struct pollfd ) );
     }      

   PRINT_DEBUG1( "remaining connections %d", sio_connection_count );
 }

int SIOInitServer( int argc, char** argv )
{ 
  //PRINT_DEBUG( "Start" );
  sio_connection_count = 0;
  //////////////////////////////////////////////////////////////////////
  // FIGURE OUT THE DEFAULT FULL HOST NAME & ADDRESS
  
  // maintain a connection to the nameserver - speeds up lookups
  sethostent( TRUE );
  
  //PRINT_DEBUG( "Getting host information for localhost" );
  struct hostent* info = gethostbyname( "localhost" );
  assert( info );
  struct in_addr current_hostaddr;
  
  // make sure this looks like a regular internet address
  assert( info->h_length == 4 );
  assert( info->h_addrtype == AF_INET );
  
  // copy the address out
  memcpy( &current_hostaddr.s_addr, info->h_addr_list[0], 4 ); 

  ////////////////////////////////////////////////////////////////////
  //PRINT_DEBUG( "Setting up connection server" );

  listen_poll.fd = socket(AF_INET, SOCK_STREAM, 0);
  listen_poll.events = POLLIN; // notify me when a connection request happens
  
  struct sockaddr_in servaddr;  
  bzero(&servaddr, sizeof(servaddr));
  
  servaddr.sin_family      = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port        = htons(server_port);
  
  // switch on the re-use-address option
  const int on = 1;
  setsockopt( listen_poll.fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on) );
  
  if( bind(listen_poll.fd, (struct sockaddr*) &servaddr, sizeof(servaddr) )  < 0 )
  {
    perror("failed bind");

    PRINT_ERR1( "Port %d is in use", 
	    server_port );
    return -1; // fail
  }
  
  // catch signals generated by socket closures
  signal( SIGPIPE, CatchSigPipe );
  
  // listen for requests on this socket
  // we poll it in ListenForPoseConnections()
  assert( listen( listen_poll.fd, STG_LISTENQ) == 0 );


  //PRINT_DEBUG( "End" );

  return 0; //success
}


int SIOAcceptConnections( void )
{
  int readable = 0;
  int retval = 0; // 0: success, -1: failure
  
  // poll for connection requests with a very fast timeout
  if((readable = poll( &listen_poll, 1, 0 )) == -1)
    {
      if( errno != EINTR ) // timer interrupts are OK
	{ 
	  perror( "Stage warning: poll error (not EINTR)");
	  return -1; // fail
	}
    }
  
  // if the socket had a request
  if( readable && (listen_poll.revents & POLLIN ) ) 
    {
      // set up a socket for this connection
      struct sockaddr_in cliaddr;  
      bzero(&cliaddr, sizeof(cliaddr));
#if PLAYER_SOLARIS
      int clilen;
#else
      socklen_t clilen;
#endif
      
      clilen  = sizeof(cliaddr);
      int connfd = 0;
      
      connfd = accept( listen_poll.fd, (struct sockaddr*) &cliaddr, &clilen);
      
      
      // set the dirty flag for all entities on this connection
      //DirtyEntities( m_pose_sio_connection_count );
      
      
      // determine the type of connection, sync or async, by reading
      // the first byte
      char b = 0;
      int r = 0;
      
      if( (r = read( connfd, &b, 1 )) < 1 ) 
	{
	  puts( "failed to read sync type byte. Quitting\n" );
	  if( r < 0 ) perror( "read error" );
	  return -1; // fail
	}

      PRINT_DEBUG1( "received greeting %c", b );
      
      // if this is a syncronized connection, increase the sync counter 
      switch( b )
	{
	case STAGE_HELLO: 
	  PRINT_MSG2( "connection accepted (id: %d fd: %d)", 
		      sio_connection_count, connfd );
	  fflush( stdout );
	  
	  retval = 0; //success
	  break;
	  
	default: 
	  PRINT_ERR1( "Stage: bad greeting on %d. Closing connection\n",
			connfd  );
	  close( connfd );
	  retval = -1; // fail
	  break;
	}
      
   
      if( retval != -1 ) // if successful
	{
	  connection_polls[ sio_connection_count ].fd = connfd;
	  connection_polls[ sio_connection_count ].events = POLLIN;
	  sio_connection_count++;
	}
    }

  return retval;
}

// pack a property into the bundle, handling memory management, etc.
int SIOBufferProperty( stage_buffer_t* bundle,
		       int id,
		       stage_prop_id_t type,
		       void* data, size_t len, stage_reply_mode_t mode )
{
  assert( bundle );
  
  // construct 
  stage_property_t prop;
  prop.id = id;
  prop.property = type;
  prop.len = len;
  prop.reply = mode;

  //PRINT_DEBUG2( "buffering %lu byte header + %lu bytes data", 
  //	(unsigned long)sizeof(prop), (unsigned long)len );

  // buffer the header for this property
  SIOBufferPacket( bundle, &prop, sizeof(prop) );
  // buffer the property data after the header
  SIOBufferPacket( bundle, data, len );
    
  return 0;
}

void SIOClearBuffer( stage_buffer_t* buf )
{
  if( buf->data ) 
    {
      free( buf->data );
      buf->data = NULL;
    }
  
  buf->len = 0;
}

void SIOFreeBuffer( stage_buffer_t* bundle )
{
  if( bundle )
    {
      //printf( "Freeing stage_buffer_t at %p\n", bundle->data); 
      // SIODebugBuffer( bundle );
 
      SIOClearBuffer( bundle );
      free( bundle );
    }
}

stage_buffer_t* SIOCreateBuffer( void )
{
  //printf( "Allocating stage_buffer_t ... " ); fflush( stdout );

  // create and zero a buffer structure
  stage_buffer_t* buf = (stage_buffer_t*)malloc( sizeof(stage_buffer_t) );
  
  assert( buf );
  memset( buf, 0, sizeof(stage_buffer_t) );

  //printf( "done at %p\n", buf );
  //SIODebugBuffer( buf );
  
  return buf;
}
    


 

