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

int stages = 0;
int* servers = 0;


void Activity( void )
{
  static int t = 0;

  char c;
  
  switch( t++ )
    {
    case 0: c = '|'; break;
    case 1: c = '/'; break;
    case 2: c = '-'; break;
    case 3: c = '\\'; 
      t = 0;
      break;
    }    
     
  printf( " %c\r", c ); fflush( stdout );
}

int main(int argc, char **argv)
{
  /* client vars */
  int b, r, w; /* return values of various system calls */ 
  struct sockaddr_in me; /* this client's net address structure */

  int v; // used for various return values;;

  stages = argc - 1;

  // one fd for each stage
  servers = new int[ stages ];
      
  
  for( int s=0; s<stages; s++ )
    {
      
      /* open socket for network I/O */
      servers[s] = socket(AF_INET, SOCK_STREAM, 0);
      
      if( servers[s] < 0 )
	{
	  printf( "Error opening network socket. Exiting\n" );
	  fflush( stdout );
	  exit( -1 );
	}
      
      struct sockaddr_in servaddr;
      
      char* ip = argv[s+1];
      
      /* setup our server address (type, IP address and port) */
      bzero(&servaddr, sizeof(servaddr)); /* initialize */
      servaddr.sin_family = AF_INET;   /* internet address space */
      servaddr.sin_port = htons( TRUTH_SERVER_PORT ); /*our command port */ 
      inet_pton(AF_INET, ip, &servaddr.sin_addr); /* the arena IP */
      
      v = connect( servers[s], (sockaddr*)&servaddr, sizeof( servaddr) );
      
      if( v < 0 )
	{
	  printf( "Can't find a Stage truth server on %s. Quitting.\n", 
		  ip ); 
	  fflush( stdout );
	  exit( -1 );
	}
#ifdef VERBOSE
      else
	puts( " OK." );
#endif
    }


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
      r = read( servers[0], &truth, sizeof(truth) );
      
      if( r > 0 )
	{
	  //gettimeofday( &tv, 0 );
	  
	  //seconds = tv.tv_sec - start_tv.tv_sec;
	  //seconds += (tv.tv_usec - start_tv.tv_usec) / 1000000.0;
	  
	  //PrintStageTruth( truth, seconds );
	  //fflush( stdout );

	  Activity();

	  // send the packet to the other servers 
	  v = write( servers[1], &truth, sizeof(truth) );
	}
      else
	{
	  cout << "Read error (Arena dead?). quitting. " << endl;
	  exit( -1 );
	}
    }
    
    exit(0);
}








