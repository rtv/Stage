/*************************************************************************
 * posreader.cc - a demo client for the position server
 * RTV
 * $Id: posreader.cc,v 1.1.1.1.2.1 2001-02-06 03:42:51 ahoward Exp $
 ************************************************************************/


#include <sys/types.h>	/* basic system data types */
#include <sys/socket.h>	/* basic socket definitions */
#include <netinet/in.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <math.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>
#include <pthread.h>
#include <signal.h>
#include <sys/time.h>
#include <iostream.h>
#include <values.h>

#include "ports.h"

//#define VERBOSE

int main(int argc, char **argv)
{
  /* client vars */
  int b, r, w; /* return values of various system calls */ 
  int listenfd; /* file descriptor for network I/O */
  struct sockaddr_in me; /* this client's net address structure */

  int v; // used for various return values;;
      
  /* open socket for network I/O */
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  
  if( sockfd < 0 )
    {
      printf( "Error opening network socket. Exiting\n" );
      fflush( stdout );
      exit( -1 );
    }


  struct sockaddr_in servaddr;

  /* setup our server address (type, IP address and port) */
  bzero(&servaddr, sizeof(servaddr)); /* initialize */
  servaddr.sin_family = AF_INET;   /* internet address space */
  servaddr.sin_port = htons( POSITION_REQUEST_PORT ); /*our command port */ 
  inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr); /* the Stage IP */

  v = connect( sockfd, (sockaddr*)&servaddr, sizeof( servaddr) );
    
  if( v < 0 )
    {
      puts( "Can't find an Stage position server on localhost. Quitting.\n"); 
      exit( -1 );
    }
#ifdef VERBOSE
  else

    puts( " OK." );
#endif

/*-----------------------------------------------------------*/
      
    /* report good setup and go into receive loop */
#ifdef VERBOSE
  puts( " OK." );    
  puts( "Running " );
  fflush( stdout );
#endif
 
 
  char* buffer = new char[MAXRTK_MSG];
  float xpos, ypos, heading;
  int* id;

  while( 1 )
    {	      
      /* read will block until it has some bytes to return */
      r = read( sockfd, buffer, MAXRTK_MSG );
      
      if( r > 0 )
	{
	  buffer[r] = 0; // add a string terminator
	  
	  char* first = buffer;
	  char* last = (buffer+r) - 16; 
	  
	  for( char* b=first; b <= last; b+=16 )
	    {
	      /* parse the received data  */
	      id = *(int**)b;
	      xpos = *(float*)(b+4);
	      ypos = *(float*)(b+8);
	      heading = *(float*)(b+12);  
	   	      
	      printf( "0x%x %.2f %.2f %.2f\n", id, xpos, ypos, heading );
	      fflush( stdout );
	    }
	  // blank line
	  puts( "" );
	}
      else
	{
	  cout << "Read error (Stage dead?). quitting. " << endl;
	  exit( -1 );
	}
    }
    
    /* shouldn't ever get here, but just to be tidy */

      delete[]  buffer;
      
    exit(0);
}








