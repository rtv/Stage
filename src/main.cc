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
//  $Revision: 1.12 $
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


///////////////////////////////////////////////////////////////////////////
// Static vars

// Name of world file
//
char *world_file;

// Quit signal
//
bool quit = false;

CWorld *world = 0;

bool global_no_gui = false;

///////////////////////////////////////////////////////////////////////////
// Handle quit signals
//
void sig_quit(int signum)
{
  quit = true;
}

///////////////////////////////////////////////////////////////////////////
// Parse the command line
//
bool parse_cmdline(int argc, char **argv)
{
  //for( int g=0; g<argc; g++ )
  //printf( "\nargv[%d] : %s", g, argv[g] );
  //puts( "" );

  bool usage = false;

    // Extract the name of the file describing the world
    // - it's the last argument
  if( argc == 2 )
    world_file = argv[1];
  else if( argc == 3 )
    {
      world_file = argv[2];
      
      if( strcmp( argv[1], "-xs" ) == 0 )
	{
	  global_no_gui = true;
	  printf( "[No GUI]" );
	}
      else if( strcmp( argv[1], "+xs" ) == 0 )
	{
	  global_no_gui = false;
	  printf( "[GUI]" );
	}
	else
	  usage = true;
    }
  else
    usage = true;
    

      if( usage )
	{
	  printf("\nUsage: stage [+/-xs] WORLDFILE\nOptions:\n"
		 " +/-xs\tEnable the XS Graphical User Interface (xs must be in the $PATH)\n" );
	  return false;
	}
    
    printf( "[%s]", world_file );
    return true;
}


///////////////////////////////////////////////////////////////////////////
// Program entry
//
int main(int argc, char **argv)
{
  // hello world
  printf("** Stage  v%s ** ", (char*) VERSION);
  
  // Parse the command line
  // this may produce more startup output on the first line
  if (!parse_cmdline(argc, argv))
    return 1;
  
    // Create the world
    //
    world = new CWorld;
    
    // Load the world
    // this may produce more startup output
    if (!world->Load(world_file))
        return 0;
    
    puts( "" ); // end the startup output line
    fflush( stdout );

    // Start the world
    //
    if (!world->Startup())
    {
        printf("aborting\n");
        world->Shutdown();
        return 1;
    }

    // Register callback for quit (^C,^\) events
    //
    signal(SIGINT, sig_quit);
    signal(SIGQUIT, sig_quit);
    signal(SIGHUP, sig_quit);

    // Wait for a signal
    //
    while (!quit)
      pause();

    // Stop the world
    //
    world->Shutdown();

    // Destroy the world
    //
    delete world;

    return 0;
}


