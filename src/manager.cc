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
#include <netdb.h>     /* for gethostbyname(3) */
#include <sys/poll.h>

#include "truthserver.hh"

#define VERSION "0.1"

#define VERBOSE

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
struct pollfd* servers = 0;


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
     
  printf( "\rActivity: %c ", c ); fflush( stdout );
}

int main(int argc, char **argv)
{
  // hello world
  printf("** Manager v%s ** ", (char*) VERSION);

  /* client vars */
  int v, b, r, w; /* return values of various system calls */ 

  stages = argc - 1;

  // one fd for each stage
  servers = new struct pollfd[ stages ];
      
  for( int s=0; s<stages; s++ )
    {

      /* open socket for network I/O */
      servers[s].fd = socket(AF_INET, SOCK_STREAM, 0);
      servers[s].events = POLLIN; // notify me when data arrives
      
      if( servers[s].fd < 0 )
	{
	  printf( "Error opening network socket. Exiting\n" );
	  fflush( stdout );
	  exit( -1 );
	}
      
      struct sockaddr_in servaddr;
      
      char* hostname = argv[s+1];

      printf( "\nConnecting to %s on port %d... ", hostname, TRUTH_SERVER_PORT );

      struct hostent* entp;
      if( (entp = gethostbyname( hostname )) == NULL)
	{
	  fprintf(stderr, "unknown host \"%s\" (argument %s). Quitting\n",
		  hostname );
	  return(-1);
	}

      char* ip = argv[s+1];
      
      /* setup our server address (type, IP address and port) */
      bzero(&servaddr, sizeof(servaddr)); /* initialize */
      servaddr.sin_family = AF_INET;   /* internet address space */
      servaddr.sin_port = htons( TRUTH_SERVER_PORT ); /*our command port */ 
      // hostname
      memcpy(&(servaddr.sin_addr), entp->h_addr_list[0], entp->h_length);

      v = connect( servers[s].fd, (sockaddr*)&servaddr, sizeof( servaddr) );
      
      if( v < 0 )
	{
	  printf( "\nCan't find a Stage truth server on %s. Quitting.\n", 
		  ip ); 
	  fflush( stdout );
	  exit( -1 );
	}
      else
	printf( " OK." );
    }

  puts( "" );

  stage_truth_t truth;

  struct timeval start_tv, tv;
  gettimeofday( &start_tv, 0 );
  
  double seconds;

  while( 1 )
    {

      int num_to_read = 0;
      
      // let's try this poll(2) thing (with no timeout)
      if((num_to_read = poll( servers, stages, -1)) == -1)
	{
	  perror( "poll failed");
	  return(-1);
	}
 
      //printf("poll returned %d to-read\n", num_to_read);
      // call the corresponding Read() for each one that's ready
      
      if( num_to_read > 0 ) for(int i=0; i<stages; i++)
	{
	  // is this one ready to read?
	  if(servers[i].revents & POLLIN)
	    {
	      
	      //printf("reading from: %d 0x%x\n", i,servers[i].events);

	      r = read( servers[i].fd, &truth, sizeof(truth) );
	      
	      if( r > 0 )
		{
		  Activity();
		  
		  // foward the packet to the other servers 
		  for(int j=0; j<stages; j++)
		    if( j != i ) 
		      v = write( servers[j].fd, &truth, sizeof(truth) );
		}
	      else
		{
		  cout << "Read error (Arena dead?). quitting. " << endl;
		  exit( -1 );
		}
	    }
	  
	}     
    }
}







