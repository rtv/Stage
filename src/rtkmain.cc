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
//  $Revision: 1.1.2.1 $
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
// Call this function from the standard main to get an RTK interface
//
int rtkmain(int argc, char** argv, CWorld *world)
{
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
    
    // Create the simulator agent
    //
    RtkSimAgent *pSim = new RtkSimAgent(world);

    // Add agents to the application
    //
    pApp->AddAgent(pGlobalView);
    pApp->AddAgent(pPropertyView);
    pApp->AddAgent(pButtonView);
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



