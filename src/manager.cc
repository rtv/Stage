/*##########################################################################
# manager.cc - implements the Stage Manager for syncing distributed Stages
# $Id: manager.cc,v 1.10 2001-09-27 00:02:29 gerkey Exp $
*#########################################################################*/

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


void Activity( int reading )
{
  static int t = 0, s = 0;

  char b,c;
  
  // if there was activity, spin the active ticker
  // else spin the idle ticker
  ( reading > 0 ) ? t++ : s++;

  switch( t )
    {
    case 0: c = '|'; break;
    case 1: c = '/'; break;
    case 2: c = '-'; break;
    case 3: c = '\\'; 
      t = 0;
      break;
    }    

  switch( s )
    {
    case 0: b = '|'; break;
    case 1: b = '/'; break;
    case 2: b = '-'; break;
    case 3: b = '\\'; 
      s = 0;
      break;
    }    

     
    char* buf = new char[ stages+1 ];
    memset( buf, 0, stages+1 );
  
    for( int c=0; c<stages; c++ )
//      // is this one ready to read?
      (servers[c].revents & POLLIN) ?  buf[c] = '!' : buf[c] = '.';
  
      printf( "\rActivity: %c %c [%s]", b, c, buf);
 
      //printf( "\rActivity: %c", c );

  delete [] buf;

 fflush( stdout );
}

int main(int argc, char **argv)
{
  // hello world
  printf("** Manager v%s ** ", (char*) VERSION);

  /* client vars */
  int v;
  //int b, r, w; /* return values of various system calls */ 

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

      printf( "\nConnecting to %s on port %d... ", hostname, DEFAULT_TRUTH_PORT );

      struct hostent* entp;
      if( (entp = gethostbyname( hostname )) == NULL)
	{
	  fprintf(stderr, "unknown host \"%s\" (argument %d). Quitting\n",
		  hostname,s );
	  return(-1);
	}

      char* ip = argv[s+1];
      
      /* setup our server address (type, IP address and port) */
      bzero(&servaddr, sizeof(servaddr)); /* initialize */
      servaddr.sin_family = AF_INET;   /* internet address space */
      servaddr.sin_port = htons( DEFAULT_TRUTH_PORT ); /*our command port */ 
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

  struct timeval start_tv;
  //struct timeval tv;
  gettimeofday( &start_tv, 0 );
  
  //double seconds;

  while( 1 )
    {

      int num_to_read = 0;
      
      // let's try this poll(2) thing (with a timeout so we can be sure it is running)
      if((num_to_read = poll( servers, stages, 100)) == -1)
	{
	  perror( "poll failed");
	  return(-1);
	}
 
      //printf("poll returned %d to-read\n", num_to_read);
      // call the corresponding Read() for each one that's ready
      
      Activity( num_to_read );


      if( num_to_read > 0 ) for(int i=0; i<stages; i++)
	{
	  // is this one ready to read?
	  if(servers[i].revents & POLLIN)
	    {
	      
	      //printf("reading from: %d 0x%x\n", i,servers[i].events);

	      int r = 0;
	      
	      while( r < (int)sizeof(truth) )
		{
		  int v=0;
		  if( (v = read( servers[i].fd, ((char*)&truth)+r, sizeof(truth)-r )) < 1 )
		    {
		      cout << "Read error (Stage dead?). quitting. " << endl;
		      exit( -1 );
		    }
		  r+=v;
		}
	      
	      // foward the packet to the other servers 
	      for(int j=0; j<stages; j++)
              {
		if( j != i ) 
                {
                  unsigned int writecnt = 0;
                  int thiswritecnt;
                  while(writecnt < sizeof(truth))
                  {
                    thiswritecnt = write( servers[j].fd, 
                                          ((char*)&truth)+writecnt, 
                                          sizeof(truth)-writecnt);
                    assert(thiswritecnt >= 0);
                    writecnt += thiswritecnt;
                  }
                }
              }
	    }
	}     
    }
}







