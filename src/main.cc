/*************************************************************************
 * main.cc   
 * RTV
 * $Id: main.cc,v 1.2.2.2 2000-12-06 03:57:22 ahoward Exp $
 ************************************************************************/

#include <X11/Xlib.h>
#include <assert.h>
#include <fcntl.h>
#include <fstream.h>
#include <iomanip.h>
#include <math.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strstream.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <termios.h>
#include <time.h>

#include "world.hh" 
#include "win.h"

// For RTK user interface
//
#ifdef INCLUDE_RTK
#include "rtkmain.hh"
#endif

#define VERSION "0.8.1 advanced"

//#define DEBUG
//#define VERBOSE

//static long int MILLION = 1000000L;

extern void TimerHandler( int val );

int showWindow = true;
int doQuit = false;
double quitTime = 0;

CWorld* world;
//CWorldWin* win;

void HandleCommandLine( int argc, char** argv )
{
  for( int n=0; n < argc; n++ )
    {
      if( strcmp( argv[n], "-w" ) == 0 )
	showWindow = false;
      if( strcmp( argv[n], "+w" ) == 0 )
	showWindow = true;
      if( strcmp( argv[n], "-window" ) == 0 )
	showWindow = false;
      if( strcmp( argv[n], "+window" ) == 0 )
	showWindow = true;
    }      
}

int main( int argc, char** argv )
{
  cout << "** Stage v" << VERSION << " ** " << flush;

  // the last argument specifies the initfile. is the file readable? 
  char* initFile = argv[argc-1];

  ifstream init( initFile );
  
  if( !init )
    {
      cout << "Cannot open world file: " << initFile 
	   << "\nUsage: stage [options] <worldfile>" 
	   << "\nStage exiting." << endl;
      return -1;
    }
  else
    init.close();
  
  // create the world first - later inits dereference `world'
  world = new CWorld( initFile ); 
  
  // read command line args - these may override the initfile
  HandleCommandLine( argc, argv );
  
#ifndef INCLUDE_RTK
  
  // create the window, unless we switched off graphics
  if( showWindow ) world->win = new CWorldWin( world, initFile );
  else world->win = NULL;

  // -- other system inits ---------------------------------------------
  srand48( time(NULL) ); // init random number generator
 
  //cout << "NET" << endl;
  InitNetworking(); // spawns Position and GUI server threads

  //install signal handler for timing
  if( signal( SIGALRM, &TimerHandler ) == SIG_ERR )
    {
      cout << "Failed to install signal handler" << endl;
      exit( -1 );
    }
  
  //start timer
  struct itimerval tick;
  tick.it_value.tv_sec = tick.it_interval.tv_sec = 0;
  tick.it_value.tv_usec = tick.it_interval.tv_usec = 25000; // 0.025 seconds
  
  if( setitimer( ITIMER_REAL, &tick, 0 ) == -1 )
  {
    cout << "failed to set timer" << endl;;
    exit( -1 );
  }

  cout << (char)0x07 << flush; // beep!

  // Start the objects
  //
  world->Startup();
  
  // -- Main loop -------------------------------------------------------
  // Stage will perform a whole world update each time round this loop.
  // To avoid hogging the processor, we spend most of the time asleep,
  // waking up every 20ms or so to do an update
  while( !doQuit )
    {
      world->Update();

      sleep( 1000 ); // go to sleep until a timer event occurs
    }

  // Stop the objects
  //
  world->Shutdown();

#else

  // Run the RTK interface
  //
  rtkmain(argc, argv, world);
  
#endif
}  








