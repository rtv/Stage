
#if HAVE_CONFIG_H
  #include <config.h>
#endif
#if HAVE_STRINGS_H
  #include <strings.h>
#endif

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
#include <sys/poll.h>
#include <assert.h>

#define DEBUG
#define VERBOSE
//#undef DEBUG

#include "server.h"
#include "stage_macros.h"
#include "stageio.h"

int server_port = STG_DEFAULT_SERVER_PORT;
char server_host[STG_HOSTNAME_MAX] = "localhost";
struct pollfd connection_poll;

///////////////////////////////////////////////////////////////////////////
// Print the usage string
void PrintUsage( void )
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
	  PrintUsage();
	  return FAIL;
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
      return FAIL;
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
      return FAIL;
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
    return FAIL;
  }
  // send the connection type byte - we want an asynchronous connection
  
  char c = STAGE_SYNC;
  int r;
  if( (r = write( connection_poll.fd, &c, 1 )) < 1 )
    {
      printf( "XS: failed to write STAGE_SYNC byte to Stage. Quitting\n" );
    if( r < 0 ) perror( "error on write" );
    return FAIL;
  }

  return SUCCESS;
}  

int main( int argc, char** argv )
{
  InitStageClient( argc, argv);

  
  while( 1 )
    {
      WriteCommand( connection_poll.fd, SUBSCRIBEc );
      WriteHeader( connection_poll.fd, StageContinue, 0 ); 
      sleep( 1 );
    }
}
