
#if HAVE_CONFIG_H
  #include <config.h>
#endif
#if HAVE_STRINGS_H
  #include <strings.h>
#endif


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

#include "replace.h"
#include "stageio.h"

// these vars are used only in this file (hence static)

static int server_port = STG_DEFAULT_SERVER_PORT;
static char server_host[STG_HOSTNAME_MAX] = "localhost";
static stage_buffer_t buf;

// the master-server's listening socket
static struct pollfd listen_poll;

// the current connection details
static struct pollfd connection_polls[ STG_MAX_CONNECTIONS ];
static int connection_count;


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

void TimeStamp( stage_header_t* hdr, double simtime )
{
  double seconds = floor( simtime );
  double usec = (simtime - seconds) * MILLION; 
  
  hdr->sec = (int)seconds; 
  hdr->usec = (int)usec;
}

void DebugBuffer( char* buf, size_t len )
{
  size_t c;
  
  printf( "Buffer @%p: ", buf );
  
  for( c=0; c<len; c++ )
    {
      int v = buf[c];
      printf( "%x ", v);
    }
  
  puts( "" );
}

// empty the buffer
void ClearBuffer( void )
{
  free( buf.data );
  memset( &buf, 0, sizeof(buf) );
}

// add a packet to a static queue, return a pointer to the buffer
stage_buffer_t* BufferPacket( char* packet, size_t len )
{
  static int init = 1;

  if( init ) // on the first call we nullify the static buffer
    {
      ClearBuffer();
      init = 0;
    }
  
  // if we request a length of -1, empty the buffer and return
  if( len == -1 )
    {
      free( buf.data );
      buf.data = NULL;
      buf.len = 0;
    }
  else
    {
      // make space for the new packet (this could be more efficient if
      // we grew the buffer in large chunks less frequently, but this
      // will do for now)
      buf.data = realloc( buf.data, buf.len + len );

      // add the new packet to the end of the buffer
      memcpy( buf.data + buf.len, packet, len );
      
      // remember how big the buffer has become
      buf.len += len;
    }
  
  return &buf;
}


size_t WritePacket( int fd, char* data, size_t len )
{
  size_t writecnt = 0;
  int thiswritecnt;
  
  //printf( "attempting to write a packet of %d bytes\n", len );

  while(writecnt < len )
  {
    thiswritecnt = write( fd, data+writecnt, len-writecnt);
      
    // check for error on write
    if( thiswritecnt == -1 )
      return -1; // fail
      
    writecnt += thiswritecnt;
  }

  //printf( "wrote %d/%d packet bytes\n", writecnt, len );

  return len; //success
}

// write out the contents of the buffer
int WriteBuffer( int fd, int clear )
{
  printf( "writing buffer - %d bytes\n", buf.len );

  if( WritePacket( fd, buf.data, buf.len ) != buf.len )
    return -1; // failure
  
  if( clear ) ClearBuffer();

  puts( "buffer write successful" );
  
  return 0; // success
}



size_t ReadPacket( int fd, char* buf, size_t len )
{
  //printf( "attempting to read a packet of max %d bytes\n", len );

  assert( buf ); // data destination must be good

  int recv = 0;
  // read a header so we know what's coming
  while( recv < (int)len )
  {
    // printf( "Reading on %d\n", fd );
    
    /* read will block until it has some bytes to return */
    size_t r = read( fd, buf+recv,  len - recv );
      
    if( r == -1 ) // ERROR
    {
      if( errno != EINTR )
	    {
	      printf( "ReadPacket: read returned %d\n", r );
	      perror( "code" );
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
 
size_t ReadHeader( int fd, stage_header_t* hdr  )
{
  return ReadPacket( fd, (char*)hdr, sizeof(stage_header_t) );
}

stage_buffer_t* BufferHeader( stage_header_t* hdr )
{
  return BufferPacket( (char*)hdr, sizeof(stage_header_t) );
}

size_t WriteHeader( int fd, stage_header_t* hdr )
{      
  return WritePacket( fd, (char*)hdr, sizeof(stage_header_t) );
}

int WriteCommand( int fd, double timestamp, stage_cmd_t cmd )
{
  stage_header_t hdr;
  hdr.type = STG_HDR_CMD;
  hdr.data = cmd;

  TimeStamp( &hdr, timestamp );
  
  return( WriteHeader( fd, &hdr ) == sizeof(hdr) ? 0 : -1 );
}

int BufferCommand( double timestamp, stage_cmd_t cmd )
{
  stage_header_t hdr;
  hdr.type = STG_HDR_CMD;
  hdr.data = cmd;
  
  TimeStamp( &hdr, timestamp );
  
  return( BufferHeader( &hdr ) ? 0 : -1 );
}

/// COMPOUND FUNCTIONS

int WriteModels( int fd, double simtime, stage_model_t* ent, int count )
{
  // create a header  - we're sending 'count' model creation requests
  stage_header_t hdr;
  hdr.type = STG_HDR_MODELS;
  hdr.data = count;

  TimeStamp( &hdr, simtime );
  
  // this is one way to do it
  //WriteHeader( fd, StageModelPackets, count );
  //WriteModels( fd, ent, 1 );
  
  // this is the same thing but buffered so only one write happens:
  // first add the header and model requests to the buffer
  BufferHeader( &hdr );
  BufferPacket( (char*)ent, count * sizeof(stage_model_t) );
  // then write and clear the buffer
  if( WriteBuffer( fd, 1 ) == -1 )
    {
      puts( "Failed to write model requests" );
      return -1;
    }
  return 0; //success
}

int WriteProperties( int fd, double simtime, char* data, size_t data_len )
{
  stage_header_t hdr;
  
  hdr.type = STG_HDR_PROPS;
  hdr.data = data_len; // no dirty properties so far
  
  TimeStamp( &hdr, simtime );

  // buffer the header
  BufferHeader( &hdr );  
  // buffer the data (possibly zero bytes)
  BufferPacket( data, data_len );
  
  // write and clear the buffer onto this connection's fd
  if( WriteBuffer(  fd,  1 ) == -1 )
    {
      printf( "Failed to write properties" );	  
      return -1; // fail
    }
  return 0; //success
}


int ReadModels( int fd, int num )
{
  size_t bytes_expected = num * sizeof(stage_model_t);
  stage_model_t* models =  (stage_model_t*)malloc(bytes_expected);
  
  size_t bytes_read;
  if( (bytes_read = ReadPacket( fd, (char*)models, num*sizeof(stage_model_t) )) 
      != bytes_expected )
    {
      free( models );
      return -1; // fail
    }
  
  // handle each packet as an individual request
  int m;
  for( m=0; m<num; m++ )
    {
      // CALL THE MODEL CREATION CALLBACK WITH A SINGLE MODEL REQUEST
      
      printf( "Creating model %d %s parent %d\n",
	      models[m].id, models[m].token, models[m].parent_id );
      
      //if( CreateEntityFromLibrary( &(models[m])) == -1 )
      //printf( "enitity create failed!\n" );
      
    }
  
  free( models );
  return 0;
}

// read a buffer len bytes long from fd, then call the property handling callback
// for each property packed into the buffer
int ReadProperties( int fd, size_t len )
{
  // allocate space for the data
  char* prop_buf = malloc( len );
  assert( prop_buf );
  
  // read in the right number of bytes
  size_t bytes_read;
  if( (bytes_read = ReadPacket( fd, prop_buf, len )) != len )
    {
      free( prop_buf );
      return -1; // fail
    }
  
  // the first property header is at the start of the buffer
  char* prop_header = prop_buf;
  
  // iterate through the buffer
  while( prop_header < (prop_buf + len) )
    {
      // parse the header
      stage_property_t* prop = (stage_property_t*)prop_header;
      
      PRINT_DEBUG2( "Read property id %d len %d\n", prop->id, prop->len );
      
      // CALL THE CALLBACK WITH THIS SINGLE PROPERTY
      
      // move the pointer past this record in the buffer
      prop_header += sizeof(stage_property_t) + prop->len;
    }
  
  return 0; // success
}


// returns a file descriptor attatched to the Stage server
int InitStageClient( int argc, char** argv )
{
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
  printf( "[Connecting to %s:%d]", server_host, server_port );
  puts( "" );

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
  connection_polls[0].fd = socket(AF_INET, SOCK_STREAM, 0);
  connection_polls[0].events = POLLIN; // notify me when data is available
  
  //  printf( "POLLFD = %d\n", m_pose_connections[0].fd );
  
  if( connection_polls[0].fd < 0 )
    {
      printf( "Error opening network socket\n" );
      fflush( stdout );
      return -1;
    }
  
  /* setup our server address (type, IP address and port) */
  bzero(&servaddr, sizeof(servaddr)); /* initialize */
  servaddr.sin_family = AF_INET;   /* internet address space */
  servaddr.sin_port = htons( server_port ); /*our command port */ 
  memcpy(&(servaddr.sin_addr), info->h_addr_list[0], info->h_length);
  
  if( connect( connection_polls[0].fd, 
               (struct sockaddr*)&servaddr, sizeof( servaddr) ) == -1 )
  {
    printf( "Can't find a Stage server on %s. Quitting.\n", 
            info->h_addr_list[0] ); 
    perror( "" );
    fflush( stdout );
    return -1;
  }
  // send the connection type byte - we want an asynchronous connection
  
  char c = STAGE_SYNC;
  int r;
  if( (r = write( connection_polls[0].fd, &c, 1 )) < 1 )
    {
      printf( "XS: failed to write STAGE_SYNC byte to Stage. Quitting\n" );
    if( r < 0 ) perror( "error on write" );
    return -1;
  }
  
  // a client has just the one connection (for now)
  connection_count = 1;

  return connection_polls[0].fd;
}  


int ServiceConnections( void )
{
  // read stuff until we get a continue message on each channel
#ifdef VERBOSE
  PRINT_DEBUG( "Start" );
#endif    

  // we want poll to block until it is interrupted by a timer signal,
  // so we give it a time-out of -1.
  int timeout = -1;
  int readable = 0;
  int syncs = 0;  
  
  // we loop on a poll until have a sync from all clients or we run
  // out of connections
  while(  (connection_count > 0) && (syncs < connection_count) ) 
    {
      // use poll to see which pose connections have data
      if((readable = 
	  poll( connection_polls,
		connection_count,
		timeout )) == -1) 
	{
	  PRINT_ERR( "poll(2) returned error)");	  
	  return -1; // fail
	}
      
      if( readable > 0 )
	{
	  int t;
	  for( t=0; t<connection_count; t++ )// all the connections
	    {
	      short revents = connection_polls[t].revents;
	      int confd = connection_polls[t].fd;
	      
	      if( revents & POLLIN )// data available
		{ 
		  //printf( "poll() says data available "
		  //  "(POLLIN) on connection %d\n", t );
		  
		  int hdrbytes;
		  stage_header_t hdr;
		  
		  hdrbytes = ReadHeader( confd, &hdr); 
		  
		  if( hdrbytes < (int)sizeof(hdr) )
		    {
		      //printf( "Failed to read header on connection %d "
		      //      "(%d/%d bytes).\n"
		      //      "Connection closed",
		      //      t, hdrbytes, sizeof(hdr) );
		      
		      DestroyConnection( t ); // zap this connection
		    }
		  else // find out the type of packet to follow
		    {  // and handle it
		      switch( hdr.type )
			{
			case STG_HDR_MODELS:
			  PRINT_DEBUG2( "Header: STG_HDR_MODELS (%d) on %d", 
					hdr.data, confd );
			  ReadModels( confd, hdr.data );
			  break;
			  
			case STG_HDR_PROPS: // some poses are coming in 
			  PRINT_DEBUG2( "Header: STG_HDR_PROPS (%d) on %d", 
					hdr.data, confd );
			  ReadProperties( confd, hdr.data );
			  break;
			  
			case STG_HDR_CMD:
			  PRINT_DEBUG2( "Header: STG_HDR_CMD (%d) on %d",
					hdr.data, confd );
			  
			  switch( hdr.data ) // contains the command type
			    {
			      // client is ready for Stage to do an update
			    case STG_CMD_CONTINUE: 
			      PRINT_DEBUG2( "Command: STG_CMD_CONTINUE (%d) on %d", 
					    hdr.data, confd );
			      syncs++;
			      
			      printf( "received %d of %d required continues\n",
				      syncs, connection_count );
			      break;

			    case STG_CMD_SAVE:
			    case STG_CMD_LOAD:
			    case STG_CMD_PAUSE:
			    case STG_CMD_UNPAUSE:
			      PRINT_WARN2( "Command (%d) not yet implemented "
					   "on connection %d",
					   hdr.data, t );
			      break;
						      
			    default:
			      PRINT_WARN2( "Unknown command (%d) on connection %d",
					   hdr.data, t );
			      break;
			    }
			  break;			  
			  
			default:
			  PRINT_WARN2( "Unknown header type (%d) on %d",
				       hdr.type, t  );
			}
		    }
		}
	      // if poll reported some badness on this fd
	      else if( !revents & EINTR ) //
		{
		  printf( "Stage: connection %d seems bad\n", t );
		  DestroyConnection( t ); // zap this connection
		}    
	    }
	}
    }    
  
  //#ifdef VERBOSE
  PRINT_DEBUG( "End");
  //#endif

  return 0;
}

int InitServer( int argc, char** argv )
{ 
  PRINT_DEBUG( "Start" );
  
  PRINT_DEBUG( "Resolving hostname" );

  //////////////////////////////////////////////////////////////////////
  // FIGURE OUT THE DEFAULT FULL HOST NAME & ADDRESS
  
  // maintain a connection to the nameserver - speeds up lookups
  sethostent( TRUE );
  
  PRINT_DEBUG( "Getting host information for localhost" );
  struct hostent* info = gethostbyname( "localhost" );
  assert( info );
  struct in_addr current_hostaddr;
  
  // make sure this looks like a regular internet address
  assert( info->h_length == 4 );
  assert( info->h_addrtype == AF_INET );
  
  // copy the address out
  memcpy( &current_hostaddr.s_addr, info->h_addr_list[0], 4 ); 

  ////////////////////////////////////////////////////////////////////
  PRINT_DEBUG( "Setting up connection server" );

  listen_poll.fd = socket(AF_INET, SOCK_STREAM, 0);
  listen_poll.events = POLLIN; // notify me when a connection request happens
  
  struct sockaddr_in servaddr;  
  bzero(&servaddr, sizeof(servaddr));
  
  servaddr.sin_family      = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port        = htons(server_port);
  
  // switch on the re-use-address option
  const char on = 1;
  setsockopt( listen_poll.fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on) );
  
  if( bind(listen_poll.fd, (struct sockaddr*) &servaddr, sizeof(servaddr) )  < 0 )
  {
    perror("failed bind");

    printf( "Port %d is in use. Quitting (but try again in a few seconds).", 
	    server_port );
    return -1; // fail
  }
  
  // catch signals generated by socket closures
  signal( SIGPIPE, CatchSigPipe );
  
  // listen for requests on this socket
  // we poll it in ListenForPoseConnections()
  assert( listen( listen_poll.fd, STG_LISTENQ) == 0 );


  PRINT_DEBUG( "End" );

  return 0; //success
}


int ParseCmdLine( int argc, char** argv )
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

int AcceptConnections( void )
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
      //DirtyEntities( m_pose_connection_count );
      
      
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
      
      // if this is a syncronized connection, increase the sync counter 
      switch( b )
	{
	case STAGE_SYNC: 
#ifdef VERBOSE      
	  printf( "\nStage: connection accepted (id: %d fd: %d)\n", 
		  connection_count, connfd );
	  fflush( stdout );
#endif            
	  retval = 0; //success
	  break;
	  
	default: printf( "Stage: unknown sync on %d. Closing connection\n",
			 connfd  );
	  close( connfd );
	  retval = -1; // fail
	  break;
	}
      
   
      if( retval != -1 ) // if successful
	{
	  connection_polls[ connection_count ].fd = connfd;
	  connection_polls[ connection_count ].events = POLLIN;
	  connection_count++;
	}
    }

  return retval;
}

void DestroyConnection( int con )
{

  PRINT_DEBUG1( "Closing connection %d", con );

  close( connection_polls[con].fd );
  
  connection_count--;
  
  // shift the rest of the array 1 place left
  int p;
  for( p=con; p<connection_count; p++ )
    {
      // the pollfd array
      memcpy( &(connection_polls[p]), 
	      &(connection_polls[p+1]),
	      sizeof( struct pollfd ) );
    }      
  
  PRINT_DEBUG1( "remaining connections %d", connection_count );
}

int ReportResults( double simtime )
{
  int t;
  for( t=0; t<connection_count; t++ )// all the connections
    {      
      // TODO figure out how many dirty props are on this connection, buffer them
      // together, and write 'em out (instead of NULL here)
      if( WriteProperties( connection_polls[t].fd, simtime, NULL, 0 ) == -1 )
	return -1; // fail
    }
  
  return 0; // success
}

