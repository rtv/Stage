///////////////////////////////////////////////////////////////////////////
//
// File: RtkSimAgent.cpp
// Author: Andrew Howard
// Date: 13 Nov 2000
// Desc: Simple viewing agent for Player
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/rtksimagent.cc,v $
//  $Author: ahoward $
//  $Revision: 1.1.2.8 $
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

#define ENABLE_RTK_TRACE 1

#include <math.h>
#include "world.hh"
#include "rtksimagent.hh"


///////////////////////////////////////////////////////////////////////////
// Default constructor
//
RtkSimAgent::RtkSimAgent(CWorld *world)
{
    m_pWorld = world;
}


///////////////////////////////////////////////////////////////////////////
// Destructor
//
RtkSimAgent::~RtkSimAgent()
{
}


///////////////////////////////////////////////////////////////////////////
// Cfgtialisation
//
bool RtkSimAgent::Open(RtkCfgFile* pCfgFile)
{
    if (!RtkAgent::Open(pCfgFile))
        return FALSE;

    // Initialise messages
    //
    UiForceUpdateSource::init(m_msg_router);
    UiDrawSink::init(m_msg_router);
    UiMouseSink::init(m_msg_router);
    UiPropertySink::init(m_msg_router);
    UiButtonSink::init(m_msg_router);

    return TRUE;
}


///////////////////////////////////////////////////////////////////////////
// Close function
//
void RtkSimAgent::Close()
{
}


///////////////////////////////////////////////////////////////////////////
// Save function
//
void RtkSimAgent::Save(RtkCfgFile* pCfgFile)
{
}


///////////////////////////////////////////////////////////////////////////
// Start any threads/timers for this module
//
bool RtkSimAgent::Start()
{
	// Start the module thread
	//
	start_thread(-10);
	return TRUE;
}


///////////////////////////////////////////////////////////////////////////
// Stop any threads/timers for this module
//
bool RtkSimAgent::Stop()
{	
	// Stop the module thread
	//
	stop_thread();
	return TRUE;
}


///////////////////////////////////////////////////////////////////////////
// Main agent loop
//
void RtkSimAgent::Main()
{ 
    // Update the GUI now.
    //
    send_ui_force_update(NULL);
    
    while (!quit())
    {
        // Update at 10Hz
        //
        sleep(100);
        
        // Update the GUI now
        //
        send_ui_force_update(NULL);
    }

}


///////////////////////////////////////////////////////////////////////////
// UI draw message handler
//
void RtkSimAgent::OnUiDraw(RtkUiDrawData* pData)
{
    m_pWorld->OnUiDraw(pData);
}


///////////////////////////////////////////////////////////////////////////
// UI mouse message handler
//
void RtkSimAgent::OnUiMouse(RtkUiMouseData* pData)
{
    // Create a mouse mode.
    // Note that we dont actually use it here
    //
    pData->begin_section("global", "object");
    pData->mouse_mode("move");
    pData->end_section();
        
    // Pass on the mouse mode to objects in the world
    //
    m_pWorld->OnUiMouse(pData);
}


///////////////////////////////////////////////////////////////////////////
// UI property message handler
//
void RtkSimAgent::OnUiProperty(RtkUiPropertyData* pData)
{
    pData->begin_section("default", "world");
    m_pWorld->OnUiProperty(pData);
    pData->end_section();
}


///////////////////////////////////////////////////////////////////////////
// UI button message handler
//
void RtkSimAgent::OnUiButton(RtkUiButtonData *pData)
{
}

