///////////////////////////////////////////////////////////////////////////
//
// File: main.cc
// Author: Andrew Howard
// Date: 4 Dec 2000
// Desc: Program entry point when not using a GUI
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/main.cc,v $
//  $Author: rtv $
//  $Revision: 1.38 $
//
// Usage:
//  (empty)
//
// Theory of operation:
//  (empty)
//
// Known bugs:
//  (empty)
//
// Possible enhancements:
//  (empty)
//
///////////////////////////////////////////////////////////////////////////

#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <netdb.h> // for gethostbyname(3)

//#define DEBUG

#include "server.hh"

///////////////////////////////////////////////////////////////////////////
// Local vars

// Quit signal
bool quit = false;
//static bool quit = false;

// Pointer to the one-and-only instance of the world
// This really should be static
CWorld *world = NULL;

///////////////////////////////////////////////////////////////////////////
// Print the usage string
void PrintUsage( void )
{
  printf("\nUsage: stage [options] WORLDFILE\n"
	 "Options: <argument> [default]\n"
	 " -s\t\tRun as a Stage server (default)"
	 " -c\t\tRun as a client to a Stage server on localhost"
	 " -c <hostname>\t\tRun as a client to a Stage server on hostname"
	 " -g\t\tDo not start the X GUI\n"
	 " +g\t\tDo start the X GUI\n"
	 " -p\t\tDo not start Player\n"
	 " +p\t\tDo start Player\n"
	 " -v <float>\tSet the simulated time increment per cycle [0.1sec].\n"
	 " -u <float>\tSet the desired real time per cycle [0.1 sec].\n"
	 " -l <filename>\tLog the position of all objects into the"
	 " named file.\n"
	 " -p <portnum>\tSet the server port\n"
	 " -f\t\tRun as fast as possible; don't try to match real time\n"
	 //" -r <IP:port>\tSend sensor data to this address in RTP format\n"
	 //#ifdef HRL_HEADERS
	 //" -i\t\tSend IDAR messages to XS (hrlstage only)\n"
	 //#endif
	 "Command-line options override any configuration file equivalents.\n"
	 "\n"
	 );
}

///////////////////////////////////////////////////////////////////////////
// Clean up and quit
void StageQuit( void )
{  
  puts( "\n** Stage quitting **" );
  
  if( world )  
    {
      world->Shutdown();  // Stop the world
      delete world;       // Destroy the world
    }
  exit( 0 );
}

///////////////////////////////////////////////////////////////////////////
// Handle quit signals
void sig_quit(int signum)
{
  PRINT_DEBUG1( "SIGNAL %d\n", signum );
  quit = true;
}


///////////////////////////////////////////////////////////////////////////
// Program entry
//
int main(int argc, char **argv)
{  
  // hello world
  printf("\n** Stage  v%s ** ", (char*) VERSION);

#ifdef INCLUDE_RTK2
  // Initialise rtk if we are using it
  rtk_init(&argc, &argv);
#endif
  
  world = NULL;
  
  // CStageServer and CStageClient are subclasses of CStageIO and CWorld
  // constructing them does most of the startup work.
  
  // check the command line for the '-c' option that makes this a client
  for( int a=1; a<argc; a++ )
    {
      PRINT_DEBUG2( "argv[%d] = %s\n", a, argv[a] );
      
      if( strcmp( argv[a], "-c" ) == 0 )
	{
	  printf( "[Client]" );
	  assert( world = new CStageClient( argc, argv ) );
	}
    }
  
  // if we're not a client, we must be a server
  if( world == NULL )
    assert( world = new CStageServer( argc, argv ) );
  
  // a world constructor may have raised the quit flag
  // (this would be more elegantly implemented with an exception..)
  if( quit )
    {
      puts( "Stage: failed to create a world. Quitting." );
      exit( 0 );
    }

  // startup is (externally) identical for client and server, but they
  // do slightly different things inside.
  // post-constructor startup is required to exploit object polymorphism
  if (!world->Startup())
    {
      puts("Stage: failed to startup world. Quitting.");
      StageQuit();
    }
  
  puts( "" ); // end the startup output line
  
  // Register callback for quit (^C,^\) events
  // - sig_quit function raises the quit flag 
  signal(SIGINT, sig_quit );
  signal(SIGQUIT, sig_quit );
  signal(SIGTERM, sig_quit );
  signal(SIGHUP, sig_quit );
  
  // the main loop - it'll be interrupted by a signal
  while( !quit )
    {
      world->Update(); // update the simulation
    
      //sleep( 10 );
      usleep( 500000 );
    }

  // clean up and exit
  StageQuit();
}




