///////////////////////////////////////////////////////////////////////////
//
// File: rtk_main.cc
// Author: Andrew Howard
// Date: 4 Dec 2000
// Desc: Program entry point when using RTK
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/rtk_main.cc,v $
//  $Author: vaughan $
//  $Revision: 1.7 $
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
#include "rtk_ui.hh"


///////////////////////////////////////////////////////////////////////////
// Static vars

// Name of world file
//
char *world_file;

CWorld *world = 0;

///////////////////////////////////////////////////////////////////////////
// Parse the command line
//
bool parse_cmdline(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("Usage: rtk_stage WORLDFILE\n");
        return false;
    }

    // Extract the name of the file describing the world
    //
    world_file = argv[argc-1];
    return true;
}

void PrintUsage( void )
{
  printf("\nUsage: rtkstage [options] WORLDFILE\n"
	 "Options:\n"
	 " +xs\t\tExec the XS Graphical User Interface\n"
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


///////////////////////////////////////////////////////////////////////////
// Program entry
//
int main(int argc, char **argv)
{
    printf("** rtk_stage %s **\n", (char*) VERSION);
    
    // Parse the command line
    //
    if (!parse_cmdline(argc, argv))
      return 1;
    
    // Create the application
    //
    RtkUiApp *app = new RtkUiApp(&argc, &argv);
    
    // Split the application into left and right panes
    //
    RtkUiPane *left_pane, *right_pane;
    app->get_root_pane()->split_horz(&left_pane, &right_pane); 

    // Split the right pane into top and bottom panes
    //
    RtkUiPane *top_pane, *bot_pane;
    right_pane->split_vert(&top_pane, &bot_pane);

    // Craete some views
    //
    app->add_agent(new RtkUiScrollView(left_pane, "global"));
    app->add_agent(new RtkUiPropertyView(top_pane, "default"));
    app->add_agent(new RtkUiButtonView(bot_pane, "default"));

    // Create the world
    //
    world = new CWorld;

    // Initialise the RTK interface
    //
    world->InitRtk(RtkAgent::get_default_router());
    
    // Load the world
    //
    if (!world->Load(world_file))
        return 0;

    // override default and config file values with command line options.
    // any options set will produce console output for reassurance
    if (!world->ParseCmdline(argc, argv))
      return 0;

    
    // Open and start agents
    //
    if (!app->open_agents())
        exit(1);
    
    // Start the world
    //
    if (!world->Startup())
    {
      // shutdown first to make sure Players are killed
      world->Shutdown();
      return 0;
    }
    
    // Do message loop
    //   
    app->main();

    // Stop the world
    //
    world->Shutdown();
    
    // Stop agents
    //
    app->close_agents();

    // Destroy the world
    //
    delete world;

    return 0;
}



