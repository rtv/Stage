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
//  $Revision: 1.17 $
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
char global_node_name[64];
bool g_log_output = false;
char g_log_filename[256];
char g_cmdline[256];

// the parameters controlling stage's update frequency
// and simulator timestep respectively (milliseconds)
int g_interval = 0, g_timestep = 0;

int g_truth_port = 0, g_env_port = 0;
// the number of stages running
int g_instance = 0;

// if this is set on the cmd line, take this string as an ID instead
// of the hostname 
char g_host_id[64];

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
  bool usage = false;

  for( int a=1; a<argc-1; a++ )
    {
      puts( argv[a] );

      if( strcmp( argv[a], "-l" ) == 0 )
	{
	  g_log_output = true;
	  strncpy( g_log_filename, argv[a+1], 255 );
	  printf( "[Logfile %s]", g_log_filename );

	  //store the command line for logging later
	  memset( g_cmdline, 0, sizeof(g_cmdline) );
	  
	  for( int g=0; g<argc; g++ )
	    {
	      strcat( g_cmdline, argv[g] );
	      strcat( g_cmdline, " " );
	    }

	  a++;
	}
      if( strcmp( argv[a], "-xs" ) == 0 )
	{
	  global_no_gui = true;
	  printf( "[No GUI]" );
	}
      else if( strcmp( argv[a], "+xs" ) == 0 )
	{
	  global_no_gui = false;
	  printf( "[GUI]" );
	}
      else if( strcmp( argv[a], "-u" ) == 0 )
	{
	  g_interval = (int)((1.0/atof(argv[a+1])) * 1000);
	  printf( "[Update %s Hz]", argv[a+1] );
	  a++;
	}
      else if( strcmp( argv[a], "-v" ) == 0 )
	{
	  g_timestep = (int)( g_interval * atof(argv[a+1]) );
	  printf( "[Virtual time %.2f]", atof(argv[a+1]) );
	  a++;
	}
      else if( strcmp( argv[a], "-tp" ) == 0 )
	{
	  g_truth_port = atoi(argv[a+1]);
	  printf( "[Truth %d]", g_truth_port );
	  a++;
	}
      else if( strcmp( argv[a], "-ep" ) == 0 )
	{
	  g_env_port = atoi(argv[a+1]);
	  printf( "[Env %d]", g_env_port );
	  a++;
	}
      else if( strcmp( argv[a], "-id" ) == 0 )
	{
	  memset( g_host_id, 0, 64 );
	  strncpy( g_host_id, argv[a+1], 64 );
	  printf( "[ID %s]", g_host_id ); fflush( stdout );
	  a++;
	}
      // arbitary node naming stuff
      //       else if( strcmp( argv[a], "-node" ) == 0 )
      //  	{
      //  	  strncpy( global_node_name, argv[a+1], 64 );
      //  	  printf( "[Node %s]", global_node_name );
      //  	}
      //else
      //usage = true;
    }

  if( usage )
    {

#ifdef INCLUDE_RTK
      printf("\nUsage: rtkstage [options] WORLDFILE\n"
	     "Options:\n"
	     " +xs\t\tExec the XS Graphical User Interface\n"
	     " -u <float>\tSet the update frequency in Hz. Default: 20\n"
	     " -v <float>\tSet ratio of simulated to real time. Default: 1.0\n"
	     );
#else
      printf("\nUsage: stage [options] WORLDFILE\n"
	     "Options:\n"
	     " -xs\t\tDon't start the XS Graphical User Interface\n"
	     " -u <float>\tSet the update frequency in Hz. Default: 20.0\n"
	     " -v <float>\tSet ratio of simulated to real time. Default: 1.0\n"
	     );
#endif

      return false;
    }
  
      // Extract the name of the file describing the world
      // - it's the last argument
      world_file = argv[ argc-1 ];
      
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

  memset( global_node_name, 0, 64 ); //zero out the node name
  
  // Parse the command line
  // this may produce more startup output on the first line
  if (!parse_cmdline(argc, argv))
    return 1;
  
    // Create the world
  //
  world = new CWorld();
  
  // Load the world
  // this may produce more startup output
  if (!world->Load(world_file))
    return 0;
  
  // override config file values with command line options
  if( g_truth_port )
    world->m_truth_port = g_truth_port;
  
  if( g_env_port )
    world->m_env_port = g_env_port;
  
  if( g_interval )
    world->m_timer_interval = g_interval;
  
  if( g_timestep )
    world->m_timestep = g_timestep;
  

    puts( "" ); // end the startup output line
    fflush( stdout );

    // Start the world
    //
    if (!world->Startup())
    {
        printf("Stage: aborting\n");
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


