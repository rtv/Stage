///////////////////////////////////////////////////////////////////////////
//
// File: rtkmain.cc
// Author: Andrew Howard
// Date: 4 Dec 2000
// Desc: Creates an RTK GUI
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/rtkmain.cc,v $
//  $Author: ahoward $
//  $Revision: 1.1.2.3 $
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
#include "rtk-ui.hh"


///////////////////////////////////////////////////////////////////////////
// Version string
//
#define VERSION "0.8.3 advanced"


///////////////////////////////////////////////////////////////////////////
// Static vars

// Name of world file
//
char *world_file;


///////////////////////////////////////////////////////////////////////////
// Command line parsing
//
bool ParseCmdLine(int argc, char** argv)
{
    if (argc < 2)
    {
        printf("Usage: rtk-stage WORLDFILE\n");
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
int main(int argc, char** argv)
{
    // Parse the command line
    //
    if (!ParseCmdLine(argc, argv))
        return 1;
    
    // Create the application
    //
    RtkUiApp *app = new RtkUiApp(&argc, &argv);

    // Split the application into left and right panes
    //
    RtkUiPane *left_pane, *right_pane;
    app->GetRootPane()->SplitHorz(&left_pane, &right_pane); 

    // Split the right pane into top and bottom panes
    //
    RtkUiPane *top_pane, *bot_pane;
    right_pane->SplitVert(&top_pane, &bot_pane);

    // Craete some views
    //
    app->AddAgent(new RtkUiScrollView(left_pane, "global"));
    app->AddAgent(new RtkUiPropertyView(top_pane, "default"));
    app->AddAgent(new RtkUiButtonView(bot_pane, "default"));

    // Create the world
    //
    CWorld *world = new CWorld;

    // Initialise the RTK interface
    //
    world->InitRtk(RtkAgent::GetDefaultRouter());

    // Load the world
    //
    world->Load(world_file);
    
    // Open and start agents
    //
    if (!app->OpenAgents())
        exit(1);
    
    // Start the world
    //
    world->Startup();

    // Do message loop
    //   
    app->Main();

    // Stop the world
    //
    world->Shutdown();

    // Shut everything down
    //
    app->CloseAgents();

    // Destroy the world
    //
    delete world;

    return 0;
}



