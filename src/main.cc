///////////////////////////////////////////////////////////////////////////
//
// File: main.cc
// Author: Andrew Howard
// Date: 4 Dec 2000
// Desc: Program entry point when not using a GUI
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/main.cc,v $
//  $Author: vaughan $
//  $Revision: 1.24 $
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

#include "world.hh"
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


#define PIDFILENAME "stage.pid"

///////////////////////////////////////////////////////////////////////////
// Static vars

// Name of world file
//
char *world_file;

// Quit signal
//
bool quit = false;

//if this is true, exit() displays usage hints
extern bool usage; // defined in world_load.cc 


CWorld *world = 0;

///////////////////////////////////////////////////////////////////////////
// Handle quit signals
//
void sig_quit(int signum)
{
  unlink(PIDFILENAME);
  
  quit = true;
}

void PrintUsage( void )
{
  printf("\nUsage: stage [options] WORLDFILE\n"
	 "Options:\n"
	 " -xs\t\tDon't start the XS Graphical User Interface\n"
	 " -u <float>\tSet the desired real time per cycle. Default: 0.1\n"
	 " -v <float>\tSet the simulated time increment per cycle."
	 " Default: 0.1\n"
	 " -l <filename>\tLog the position of all objects into the"
	 " named file.\n"
	 " -tp <portnum>\tSet the truth server port\n"
	 " -ep <portnum>\tSet the environment server port\n"
	 " -fast\t\tRun as fast as possible; don't try to match real time\n"
	 " -s\t\tSynchronize to an external client (experimental)\n"
	 "Command-line options override any configuration file equivalents.\n"
	 "\n"
	 );
}

void StageQuit( void )
{  
  if( world )
    { 
      puts( "\nStage shutdown... " );
      world->Shutdown();
      delete world;
      puts( "...done." );
    }
  
  if( usage )
    PrintUsage();
}




///////////////////////////////////////////////////////////////////////////
// Program entry
//
int main(int argc, char **argv)
{
  // hello world
  printf("\n** Stage  v%s ** ", (char*) VERSION);
  
  // record our pid in the filesystem 
  FILE* pidfile;
  if((pidfile = fopen(PIDFILENAME, "w+")))
    {
      fprintf(pidfile,"%d\n", getpid());
      fclose(pidfile);
    }
  
  // Create the world
  //
  world = new CWorld();
    
  // check to see if the world file exists
  int fd = open( argv[argc-1], O_RDONLY );
  if( fd < 1 )
    {
      perror( "Stage: failed to open configuration file" );
      PrintUsage();
      exit( -1 );
    }
  
  close( fd );

  printf( "argv[argc-1]: %s\n", argv[ argc-1 ] );

  // Load the world - the filename is the last argument
  // this may produce more startup output
  if (!world->Load( argv[ argc-1 ] ))
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
  //  this splits of another thread which runs the simulation
  if (!world->Startup())
    {
      printf("Stage: failed startup\n");
      StageQuit();
    }
  
  // Register callback for quit (^C,^\) events
  //
  signal(SIGINT, sig_quit);
  signal(SIGQUIT, sig_quit);
  signal(SIGTERM, sig_quit);
  signal(SIGHUP, sig_quit);
  
  // register callback for any call of exit(3) 
  if( atexit( StageQuit ) == -1 )
    {
      printf("Stage: failed to register exit callback\n");
      StageQuit();
    }
  
  // Wait for a signal
  //
  while (!quit)
    pause();
  
  // exit will call the StageQuit callback function
  exit( 1 );
}





