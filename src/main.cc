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
//  $Revision: 1.33 $
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

//#define DEBUG

#include "world.hh"

///////////////////////////////////////////////////////////////////////////
// Local vars

// Quit signal
static bool quit = false;

// Pointer to the one-and-only instance of the world
// This really should be static
CWorld *world = NULL;

///////////////////////////////////////////////////////////////////////////
// Print the usage string
void PrintUsage( void )
{
  printf("\nUsage: stage [options] WORLDFILE\n"
	 "Options:\n"
	 " -xs\t\tDo not start the XS Graphical User Interface\n"
	 " +xs\t\tDo start the XS Graphical User Interface\n"
	 " -p\t\tDo not start Player\n"
	 " +p\t\tDo start Player\n"
	 " -v <float>\tSet the simulated time increment per cycle."
	 " -u <float>\tSet the desired real time per cycle. Default: 0.1 sec\n"
	 " Default: 0.1 sec\n"
	 " -l <filename>\tLog the position of all objects into the"
	 " named file.\n"
	 " -tp <portnum>\tSet the truth server port\n"
	 " -ep <portnum>\tSet the environment server port\n"
	 " -fast\t\tRun as fast as possible; don't try to match real time\n"
	 " -s\t\tSynchronize to an external client (experimental)\n"
#ifdef HRL_HEADERS
	 " -i\t\tSend IDAR messages to XS (hrlstage only)\n"
#endif
	 "Command-line options override any configuration file equivalents.\n"
	 "\n"
	 );
}


///////////////////////////////////////////////////////////////////////////
// Clean up and quit
void StageQuit( void )
{  
  puts( "\n** Stage quitting **" );

  // Stop the world
  world->Shutdown();
  
  // Destroy the world
  delete world;

  exit( 0 );
}

///////////////////////////////////////////////////////////////////////////
// Handle quit signals
void sig_quit(int signum)
{
  //printf( "SIGNAL %d\n", signum );
  quit = true;
}


///////////////////////////////////////////////////////////////////////////
// Program entry
//
int main(int argc, char **argv)
{
  // hello world
  printf("\n** Stage  v%s ** ", (char*) VERSION);
  
  // Create the world
  world = new CWorld();
  
  // Make sure we have a filename, at least
  if (argc < 2)
  {
    PrintUsage();
    exit(1);
  }
    
  // Load the world - the filename is the last argument
  // this may produce more startup output
  if (!world->Load(argv[argc - 1]))
  {
    printf("Stage: failed load\n");
    StageQuit();
  }

  // override default and config file values with command line options.
  // any options set will produce console output for reassurance
  if (!world->ParseCmdline(argc, argv))
  {
    printf("Stage: failed to parse command line\n");
    StageQuit();
  }

  puts( "" ); // end the startup output line
  fflush( stdout );
  
  // Start the world
  if (!world->Startup())
  {
    printf("Stage: failed startup\n");
    StageQuit();
  }

  // Register callback for quit (^C,^\) events
  // - sig_quit function raises the quit flag 
  signal(SIGINT, sig_quit );
  signal(SIGQUIT, sig_quit );
  signal(SIGTERM, sig_quit );
  signal(SIGHUP, sig_quit );

  //printf("running\n");

  // the main loop - it'll be interrupted by a signal
  while( !quit )
    world->Main();

  //printf("quiting\n");
  
  // clean up and exit
  StageQuit();
}




