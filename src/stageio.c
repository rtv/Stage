
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
#include "stage.h"


// these vars are used only in this file (hence static)
static int server_port = STG_DEFAULT_SERVER_PORT;
static char server_host[STG_HOSTNAME_MAX] = "localhost";
static struct pollfd connection_poll;
static stage_buffer_t buf;

///////////////////////////////////////////////////////////////////////////
// Print the usage string
void StageClientPrintUsage( void )
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
void ClientCatchSigPipe( int signo )
{
#ifdef VERBOSE
  puts( "** SIGPIPE! **" );
#endif
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
  if( WritePacket( fd, buf.data, buf.len ) != buf.len )
    return -1; // failure
  
  if( clear ) ClearBuffer();
  
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

int ReadHeader( int fd, stage_header_t* hdr  )
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

size_t WriteCommand( int fd, stage_cmd_t cmd )
{
  stage_header_t hdr;
  hdr.type = StageCommand;
  hdr.data = cmd;

  return WriteHeader( fd, &hdr );
}

stage_buffer_t* BufferCommand( stage_cmd_t cmd )
{
  stage_header_t hdr;
  hdr.type = StageCommand;
  hdr.data = cmd;
  
  return BufferHeader( &hdr );
}

size_t ReadModels( int fd, stage_model_t* models, int num )
{
  return ReadPacket( fd, (char*)models, num *  sizeof(stage_model_t) );
}

size_t WriteModels( int fd, stage_model_t* models, int num )
{
  return WritePacket( fd, (char*)models, num*sizeof(stage_model_t) );
}

stage_buffer_t* BufferModels( stage_model_t* models, int num )
{
  return BufferPacket( (char*)models, num*sizeof(stage_model_t) );
}


/// COMPOUND FUNCTIONS

int CreateModels( int fd, stage_model_t* ent, int count )
{
  int retval = 0;

  // create a header  - we're sending 'count' model creation requests
  stage_header_t hdr;
  hdr.type = StageModelPackets;
  hdr.data = count;
  
  // this is one way to do it
  //WriteHeader( fd, StageModelPackets, count );
  //WriteModels( fd, ent, 1 );
  
  // this is the same thing but buffered so only one write happens:
  // first add the header and model requests to the buffer
  BufferHeader( &hdr );
  BufferModels( ent, count );
  // then write and clear the buffer
  retval = WriteBuffer( fd, 1 );

  return retval;
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
	  StageClientPrintUsage();
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
  connection_poll.fd = socket(AF_INET, SOCK_STREAM, 0);
  connection_poll.events = POLLIN; // notify me when data is available
  
  //  printf( "POLLFD = %d\n", m_pose_connections[0].fd );
  
  if( connection_poll.fd < 0 )
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
  
  if( connect( connection_poll.fd, 
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
  if( (r = write( connection_poll.fd, &c, 1 )) < 1 )
    {
      printf( "XS: failed to write STAGE_SYNC byte to Stage. Quitting\n" );
    if( r < 0 ) perror( "error on write" );
    return -1;
  }

  return connection_poll.fd;
}  
