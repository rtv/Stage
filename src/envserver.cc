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

extern  CWorld* world;

const int LISTENQ = 128;
//const long int MILLION = 1000000L;

bool g_wait_for_env_server = true;

extern void CatchSigPipe( int signo );

static void * EnvWriter( void* arg )
{
#ifdef VERBOSE
  printf( "Stage: EnvWriter thread\n" );
  fflush( stdout );
#endif

  int* connfd = (int*)arg;

  sigblock(SIGINT);
  sigblock(SIGQUIT);
  sigblock(SIGHUP);

  pthread_detach(pthread_self());

  stage_env_header_t hdr;

  hdr.width = (uint16_t)world->matrix->width;
  hdr.height = (uint16_t)world->matrix->height;
  hdr.ppm = (uint16_t)world->ppm;
  hdr.num_objects = (uint16_t)world->GetObjectCount();
  hdr.num_pixels = 0; // we'll count them below

  for( int x = 0; x < world->matrix->width; x++ )
    for( int y = 0; y < world->matrix->height; y++ )
      if( world->matrix->is_type( x, y, WallType ) )
	hdr.num_pixels++;
  
  int errorretval = -1;

  // send the header to the connected client
  if( write( *connfd, &hdr, sizeof(hdr) ) < 0 )
    {
      perror( "EnvWriter failed writing header" );
      close( *connfd );
      pthread_exit( (void*)&errorretval );
    }

  // while the client is reading that, we'll process he pixels

  // allocate storage for the filled pixels
  XPoint* pixels = new XPoint[ hdr.num_pixels ];
  
  memset( pixels, 0, hdr.num_pixels * sizeof( XPoint ) );

  int store = 0;
  // iterate through again, this time recording the pixel's details
  for( int x = 0; x < world->matrix->width; x++ )
    for( int y = 0; y < world->matrix->height; y++ )
      {
	if( world->matrix->is_type( x, y, WallType ) )
	  {
	    pixels[store].x = x;
	    pixels[store].y = y;
	    
	    store++;
	  }
      }
  
  
  // send the pixels
  int send = hdr.num_pixels*sizeof(XPoint);
  int sent = 0;

#ifdef VERBOSE      
      printf( "Stage: EnvWriter sending %d pixels (%d bytes)\n", 
	      hdr.num_pixels, send );
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

  stage_truth_t truth;

  for( int n=0; n<hdr.num_objects; n++ )
    {
      world->GetObject(n)->ComposeTruth( &truth, n );
      
      // send the truth
      send = sizeof(stage_truth_t);
      sent = 0;
      
#ifdef VERBOSE      
      printf( "Stage: EnvWriter sending %d truths (%d bytes)\n", 
	      1, send );
      fflush( stdout );
#endif
      
      while( sent < send )
	{
	  int b = write( *connfd, &truth, send - sent );
	  
	  if( b < 0 )
	    {
	      perror( "EnvWriter failed writing truths" );
	      close( *connfd );
	      pthread_exit( (void*)&errorretval );
	    }
	  
	  sent += b;
	  
#ifdef VERBOSE      
	  printf( "Stage: EnvWriter sent %d/%d bytes\n", sent, send );
	  fflush( stdout );
#endif
	  
	}
    }
  
  //close( *connfd );

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
  servaddr.sin_port        = htons( world->m_env_port);
  
  // switch on the re-use-address option
  const int on = 1;
  setsockopt( listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on) );

  if( bind(listenfd, (SA *) &servaddr, sizeof(servaddr) )  < 0 )
      {
	cout << "Port " << world->m_env_port 
	     << " is in use. Quitting (but try again in a few seconds)." 
	     <<endl;
	exit( -1 );
      }
 
  listen(listenfd, LISTENQ);

  g_wait_for_env_server = false;
  
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

