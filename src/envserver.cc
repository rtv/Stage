#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <iomanip.h>
#include <termios.h>
#include <strstream.h>
#include <iostream.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <pthread.h>
#include <math.h>
#include <sys/time.h>
#include <unistd.h>

#include <queue>

//#define DEBUG
//#define VERBOSE

#include "world.hh"
#include "truthserver.hh"
//#include "comms.h"
//#include "crums.h"

extern  CWorld* world;

const int LISTENQ = 128;
const long int MILLION = 1000000L;

extern void CatchSigPipe( int signo );

int global_environment_port = ENVIRONMENT_SERVER_PORT;

static void * EnvWriter( void* arg )
{
#ifdef VERBOSE
  printf( "Stage: EnvWriter thread (socket %d)\n", *connfd );
  fflush( stdout );
#endif

  int* connfd = (int*)arg;

  sigblock(SIGINT);
  sigblock(SIGQUIT);
  sigblock(SIGHUP);

  pthread_detach(pthread_self());

  player_occupancy_data_t og;

  og.width = (uint16_t)world->m_bimg->width;
  og.height = (uint16_t)world->m_bimg->height;
  og.ppm = (uint16_t)world->ppm;
  og.num_pixels = 0; // we'll count them below

  // count the filled pixels
  unsigned int total_pixels = world->m_bimg->width * world->m_bimg->height;
  
  for( unsigned int index = 0; index < total_pixels; index++ )
    if( world->m_bimg->get_pixel( index) != 0 )
      og.num_pixels++;

  int errorretval = -1;

  // send the header to the connected client
  if( write( *connfd, &og, sizeof(og) ) < 0 )
    {
      perror( "EnvWriter failed writing header" );
      close( *connfd );
      pthread_exit( (void*)&errorretval );
    }

  // while the client is reading that, we'll process he pixels

  // allocate storage for the filled pixels
  XPoint* pixels = new XPoint[ og.num_pixels ];
  
  memset( pixels, 0, og.num_pixels * sizeof( XPoint ) );

  int store = 0;
  // iterate through again, this time recording the pixel's details
  for( int x = 0; x < world->m_bimg->width; x++ )
    for( int y = 0; y < world->m_bimg->height; y++ )
      {
	unsigned int val = world->m_bimg->get_pixel( x, y );
	
	if( val != 0 ) // it's not a background pixel
	  {
	    pixels[store].x = x;
	    pixels[store].y = y;
	    
	    store++;
	  }
      }
  
  
  // send the pixels

  int send = og.num_pixels*sizeof(XPoint);
  int sent = 0;

#ifdef VERBOSE      
      printf( "Stage: EnvWriter sending %d pixels (%d bytes)\n", 
	      og.num_pixels, send );
      fflush( stdout );
#endif

  while( sent < send )
    {
      int b = write( *connfd, pixels, send - sent );
  
      if( b < 0 )
	{
	  perror( "EnvWriter failed writing pixels" );
	  close( *connfd );
	  pthread_exit( (void*)&errorretval );
	}
      
      sent += b;

#ifdef VERBOSE      
      printf( "Stage: EnvWriter sent %d/%d bytes\n", sent, send );
      fflush( stdout );
#endif
      
    }

  close( *connfd );

#ifdef VERBOSE
  puts( "Stage: EnvWriter thread exit" );
  fflush( stdout );
#endif

  pthread_exit( 0 ); 
}	


void* EnvServer( void* )
{
  sigblock(SIGINT);
  sigblock(SIGQUIT);
  sigblock(SIGHUP);

  pthread_detach(pthread_self());
    
  int listenfd;

  int* connfd = new int(0);
  //pid_t childpid;
  socklen_t clilen;
  struct sockaddr_in	cliaddr, servaddr;
  
  listenfd = socket(AF_INET, SOCK_STREAM, 0);
  
  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family      = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port        = htons(global_environment_port);
  
  // switch on the re-use-address option
  const int on = 1;
  setsockopt( listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on) );

  if( bind(listenfd, (SA *) &servaddr, sizeof(servaddr) )  < 0 )
      {
	cout << "Port " << global_environment_port 
	     << " is in use. Quitting (but try again in a few seconds)." 
	     <<endl;
	exit( -1 );
      }
 
  listen(listenfd, LISTENQ);
  
  pthread_t tid_dummy;
  
  while( 1 ) 
    {
      clilen = sizeof(cliaddr);
      *connfd = accept(listenfd, (SA *) &cliaddr, &clilen);

#ifdef VERBOSE
  printf( "Stage: EnvWriter connection accepted (socket %d)\n", *connfd );
  fflush( stdout );
#endif
      // start a thread to handle the connection
      pthread_create(&tid_dummy, NULL, &EnvWriter, connfd ); 
    }
}

