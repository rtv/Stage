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

#include "world.hh"
#include "rtk-ui.hh"
#include "rtksimagent.hh"


///////////////////////////////////////////////////////////////////////////
// Version string
//
#define VERSION "0.8.1 advanced"


///////////////////////////////////////////////////////////////////////////
// Naked declarations

// Parse the command line
//
bool ParseCmdLine(int argc, char** argv);

// Name of world file
//
char *m_world_file;


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
    RtkUiApp *pApp = new RtkUiApp(&argc, &argv);

    // Split the application into left and right panes
    //
    RtkUiPane *pLeftPane, *pRightPane;
    pApp->GetRootPane()->SplitHorz(&pLeftPane, &pRightPane); 

    // Split the right pane into top and bottom panes
    //
    RtkUiPane *pTopPane, *pBotPane;
    pRightPane->SplitVert(&pTopPane, &pBotPane);

    // Create some views
    //
    RtkUiScrollView *pGlobalView = new RtkUiScrollView(pLeftPane, "global");
    RtkUiPropertyView *pPropertyView = new RtkUiPropertyView(pTopPane, "default");
    RtkUiButtonView *pButtonView = new RtkUiButtonView(pRightPane, "default");

    // Add agents to the application
    //
    pApp->AddAgent(pGlobalView);
    pApp->AddAgent(pPropertyView);
    pApp->AddAgent(pButtonView);
    
    // Create the simulator agent
    //
    RtkSimAgent *pSim = new RtkSimAgent(m_world_file);
    pApp->AddAgent(pSim);
    
    // Open and start agents
    //
    if (!pApp->OpenAgents())
        exit(1);

    // Do message loop
    //   
    pApp->Main();

    // Shut everything down
    //
    pApp->CloseAgents();

    return 0;
}


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
    m_world_file = argv[1];
    return true;
}

