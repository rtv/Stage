/*************************************************************************
 * server.cc - implements the position & GUI servers, plus signal handling
 * RTV
 * $Id: server.cc,v 1.3 2000-12-08 09:08:11 vaughan Exp $
 ************************************************************************/

// YUK this file is all in C and implements the PositionServer and GuiServer
// that run as separate threads alongside the simulation

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <iomanip.h>
#include <iostream.h>
#include <math.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strstream.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>
#include <values.h>

//#define DEBUG
//#define VERBOSE

#include "world.h"
#include "ports.h"

#define	SA	struct sockaddr

typedef struct Pos
{
  int id;
  float x, y, a;
};


extern  CWorld* world;

const int LISTENQ = 128;
const long int MILLION = 1000000L;

extern int doQuit;
extern int coords;


float UnitGaussianNoiseAboutZero( void )
{
  // from NRinC p289
  static int iset = 0;
  static float gset;
  float fac, rsq, v1, v2;

  if( iset==0 )
    {
      do
	{
	  v1 = 2.0 * drand48() - 1.0;
	  v2 = 2.0 * drand48() - 1.0;
	  rsq = v1*v1+v2*v2;
	} while( rsq >= 1.0 || rsq == 0.0 );

      fac = sqrt( -2.0 * log(rsq)/rsq );

      gset = v1*fac;
      iset = 1;
      return v2*fac;
    }
  else
    {
      iset = 0;
      return gset;
    }
}


void TimerHandler( int val )
{
  // do nothing! just wake from sleep
  //cout << "WAKE!" << flush;
}  

void CatchSigPipe( int signo )
{
#ifdef VERBOSE
  cout << "** SIGPIPE! **" << endl;
#endif

  // SIGPIPE likely to be because X window was destroyed
  // so we shut down gracefully with SIGINT
  raise( SIGINT );

}

void CatchSigHup( int signo )
{
#ifdef VERBOSE
  cout << "** SIGHUP! **" << endl;
#endif
}

void CatchSigInt( int signo )
{
#ifdef VERBOSE
  cout << "** SIGINT! **" << endl;
#endif

  // shutdown stuff moved to destructors - RTV

  delete world;

  cout << "Stage quitting." << endl;
  exit(0);
}


static void * RunGUI( void * )
{ 
  //cout << "GuiServer1... " << endl;;

  pthread_detach(pthread_self());
  
  // BPG
   sigblock(SIGINT);
  // GPB

  int ifsock=0, bar=0;
  struct sockaddr_in ifserver;
  struct sockaddr_in foo;
  
  ifsock = socket(AF_INET, SOCK_STREAM, 0);
  if (ifsock < 0) 
    {
      perror("opening stream socket");
      exit(1);
    }
  
  ifserver.sin_family = AF_INET;
  ifserver.sin_addr.s_addr = INADDR_ANY;
  ifserver.sin_port = htons( GUIInPort ); // well-know port 50020

  // switch on the re-use-address option
  const int on = 1;
  setsockopt( ifsock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on) );

  if (bind(ifsock, (struct sockaddr*)&ifserver, sizeof(ifserver))) 
    {
      perror("binding stream socket");
      exit(1);
    }
   
  listen( ifsock, 128 );

  //cout << "\t\tok." << endl;
 
  char buf[50];

  while( 1 )
    {
      
      bzero(buf, sizeof(buf));
      
      unsigned int k = sizeof(foo);   
  
      //cout << "Waiting for connecyion" << endl;
      // the thread will block on accept until a connect request is pending
      bar = accept( ifsock, (struct sockaddr*)&foo, &k );
      
      if( bar > 0 )
	{
	  int rval = 0;
	  rval = read( bar, buf, sizeof(buf));
	  if( rval > 0 )
	    {
	      //cout << endl << "recv: " << buf << '\r' << endl;;
	    
	      switch( buf[0] )
		{
		case 'X': //its a reset command
		  break;
		  
		case 't':  // its a toggle
		  
		  if( strcmp( &buf[1], "pause" ) == 0 )
		    world->paused = !world->paused;
		  
		  else if( strcmp( &buf[1], "quit" ) == 0 )
		    doQuit = true;
		  
		  else if( strcmp( &buf[1], "showsonar" ) == 0 )
		    {if( world->win ) 
		      world->win->showSensors = !world->win->showSensors;}
		  
		  else if( strcmp( &buf[1], "drawmode" ) == 0 )
		    {if( world->win ) 
		      world->win->drawMode = !world->win->drawMode;}

		  /* else if( strcmp( &buf[1], "savezones" ) == 0 )
		    world->SaveZones();

		  else if( strcmp( &buf[1], "loadzones" ) == 0 )
		    world->LoadZones();
		  */

		  else if( strcmp( &buf[1], "savepos" ) == 0 )
		    world->SavePos();
		  
		  else if( strcmp( &buf[1], "loadpos" ) == 0 )
		    {
		      ifstream in( world->posFile );
		      
		      for( CRobot* r = world->bots; r; r=r->next )
			{
			  double xpos, ypos, theta;
			  
			  in >> xpos >> ypos >> theta;
			  
#ifdef DEBUG
			  cout << "read: " <<  xpos << ' ' << ypos 
			       << ' ' << theta << endl;
#endif
			  
			  r->ResetPosition( xpos, ypos, theta );
			}
  
		      in.close(); // finished with the position file 
		    }
		}
	    }
	}
      
      close( bar );
      bar = 0;
    } 
}

static void * PositionWriter( void* connfd )
{
  pthread_detach(pthread_self());
  
  // BPG
  sigblock(SIGINT);
  // GPB

  int fd = *(int*)connfd;

#ifdef VERBOSE
  cout << "PositionWriter started" << endl;
#endif

  unsigned char* sendline = new unsigned char[MAXPOSITIONSTRING];
  
  //install signal handler for timing
  if( signal( SIGALRM, &TimerHandler ) == SIG_ERR )
    {
      cout << "Failed to install signal handler" << endl;
      exit( -1 );
    }
    
  //start timer
  struct itimerval tick;
  tick.it_value.tv_sec = tick.it_interval.tv_sec = 0; 
  tick.it_value.tv_usec = tick.it_interval.tv_usec = 500000; // 0.5 seconds
   
  if( setitimer( ITIMER_REAL, &tick, 0 ) == -1 )
    {
      cout << "failed to set timer" << endl;;
      exit( -1 );
    }

#ifdef VERBOSE
  cout << "PositionWriter running" << endl;
#endif

  int v = 0;
  while( 1 ) 
    {
      // compile a string containing the positions of all the robots
      int len = 0;
      
      for( CRobot* r=world->bots; r; r=r->next )
	{
	  *(CRobot**)(sendline+len) = r;
	  *(float*)(sendline+len+4)   = (float)r->x/world->ppm;
	  *(float*)(sendline+len+8) = (float)r->y/world->ppm;
	  *(float*)(sendline+len+12) = (float)r->a;

	  len += 16;
	}
      
      sendline[len] = 0; //terminator

      // send the string to the connected client
      v = write( fd, sendline, len );

      if( v < 0 )
	{
#ifdef VERBOSE
	  cout << "Closing position client." << endl;
#endif
	  close( fd );
	  pthread_exit( 0 );
	}
      
      sleep( 100 );
    }
}	
 

static void* PositionServer( void* )
{ 
  //cout << "PositionServer... " << endl;;

  pthread_detach(pthread_self());

  //cout << "detatched... " << endl;;
  
  // BPG
  sigblock(SIGINT);
  // GPB
  
  
  CRobot* r;
  int listenfd;

  int* connfd = new int(0);
  pid_t childpid;
  socklen_t clilen;
  struct sockaddr_in	cliaddr, servaddr;
  
  listenfd = socket(AF_INET, SOCK_STREAM, 0);
  
  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family      = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port        = htons(POSITION_REQUEST_PORT);
  
  // switch on the re-use-address option
  const int on = 1;
  setsockopt( listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on) );

  if( bind(listenfd, (SA *) &servaddr, sizeof(servaddr) )  < 0 )
      {
	cout << "Port " << POSITION_REQUEST_PORT 
	     << " is in use. Quitting (but try again in a few seconds)." 
	     <<endl;
	exit( -1 );
      }
 
  listen(listenfd, LISTENQ);
  
  pthread_t tid_dummy;
  
  //cout << "\tok." << endl;

  while( 1 ) 
    {
      clilen = sizeof(cliaddr);
      *connfd = accept(listenfd, (SA *) &cliaddr, &clilen);
#ifdef VERBOSE      
      cout << "PositionServer connection accepted on " << connfd <<  endl;
#endif            
      // start a thread to handle the connection
      pthread_create(&tid_dummy, NULL, &PositionWriter, connfd ); 
    }
}


int InitNetworking( void )
{
  signal( SIGPIPE, CatchSigPipe );
  signal( SIGHUP, CatchSigHup );
  signal( SIGINT, CatchSigInt );

  pthread_t tid_dummy;
  
  // spawn a thread to handle GUI network interface

  //cout << "Running GUI" << endl;
 
 
   if( 0 != pthread_create(&tid_dummy, NULL, &RunGUI, (void *)NULL ) )
    {
      cout << "Failed to start GUI thread. Exiting." << endl;
      exit( -1 );
    }
  
  // spawn a thread to handle Position server requests
  if( 0 != pthread_create(&tid_dummy, NULL, &PositionServer, (void *)NULL ) )
    {
      cout << "Failed to start PositionServer thread. Exiting." << endl;
      exit( -1 );
    }
  

  return 1;
}
 






































