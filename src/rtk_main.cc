///////////////////////////////////////////////////////////////////////////
//
// File: rtk_main.cc
// Author: Andrew Howard
// Date: 4 Dec 2000
// Desc: Program entry point when using RTK
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/rtk_main.cc,v $
//  $Author: ahoward $
//  $Revision: 1.1.2.2 $
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

#include "../VERSION"
#include "world.hh"
#include "rtk_ui.hh"


///////////////////////////////////////////////////////////////////////////
// Static vars

// Name of world file
//
char *world_file;


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
    world_file = argv[1];
    return true;
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
    CWorld *world = new CWorld;

    // Initialise the RTK interface
    //
    world->InitRtk(RtkAgent::get_default_router());
    
    // Load the world
    //
    if (!world->Load(world_file))
        return 0;
    
    // Open and start agents
    //
    if (!app->open_agents())
        exit(1);
    
    // Start the world
    //
    if (!world->Startup())
        return 0;

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



