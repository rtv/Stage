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

#include "truthserver.hh"

#define VERBOSE

#define STAGE_IP "127.0.0.1"

void PrintStageTruth( stage_truth_t &truth, double seconds )
{
  printf( "Time: %.3f ID: %d (%4d,%d,%d)\tPID:(%4d,%d,%d)\tpose: [%d,%d,%d]\tsize: [%d,%d]\n", 
	  seconds,
	  truth.stage_id,
	  truth.id.port, 
	  truth.id.type, 
	  truth.id.index,
	  truth.parent.port, 
	  truth.parent.type, 
	  truth.parent.index,
	  truth.x, truth.y, truth.th,
	  truth.w, truth.h );
  
  fflush( stdout );
}


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
  servaddr.sin_port = htons( TRUTH_SERVER_PORT ); /*our command port */ 
  inet_pton(AF_INET, STAGE_IP, &servaddr.sin_addr); /* the arena IP */

  v = connect( sockfd, (sockaddr*)&servaddr, sizeof( servaddr) );
    
  if( v < 0 )
    {
      printf( "Can't find a Stage truth server on %s. Quitting.\n", 
	      STAGE_IP ); 
      fflush( stdout );
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
  
  stage_truth_t truth;

  struct timeval start_tv, tv;
  gettimeofday( &start_tv, 0 );
  
  double seconds;

  while( 1 )
    {	      
      /* read will block until it has some bytes to return */
      r = read( sockfd, &truth, sizeof(truth) );
      
      if( r > 0 )
	{
	  gettimeofday( &tv, 0 );

	  seconds = tv.tv_sec - start_tv.tv_sec;
	  seconds += (tv.tv_usec - start_tv.tv_usec) / 1000000.0;

	  PrintStageTruth( truth, seconds );
	  fflush( stdout );
	}
      else
	{
	  cout << "Read error (Arena dead?). quitting. " << endl;
	  exit( -1 );
	}
    }
    
    exit(0);
}








