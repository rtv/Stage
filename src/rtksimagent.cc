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
//  $Revision: 1.1.2.7 $
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
    UiForceUpdateSource::Init(m_pMsgRouter);
    UiDrawSink::Init(m_pMsgRouter);
    UiMouseSink::Init(m_pMsgRouter);
    UiPropertySink::Init(m_pMsgRouter);
    UiButtonSink::Init(m_pMsgRouter);

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
	StartThread(-10);
	return TRUE;
}


///////////////////////////////////////////////////////////////////////////
// Stop any threads/timers for this module
//
bool RtkSimAgent::Stop()
{	
	// Stop the module thread
	//
	StopThread();
	return TRUE;
}


///////////////////////////////////////////////////////////////////////////
// Main agent loop
//
void RtkSimAgent::Main()
{ 
    // Update the GUI now.
    //
    SendUiForceUpdate(NULL);
    
    while (!Quit())
    {
        // Update at 10Hz
        //
        Sleep(100);
        
        // Update the GUI now
        //
        SendUiForceUpdate(NULL);
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
    pData->BeginSection("global", "object");
    pData->MouseMode("move");
    pData->EndSection();
        
    // Pass on the mouse mode to objects in the world
    //
    m_pWorld->OnUiMouse(pData);
}


///////////////////////////////////////////////////////////////////////////
// UI property message handler
//
void RtkSimAgent::OnUiProperty(RtkUiPropertyData* pData)
{
    pData->BeginSection("default", "world");
    m_pWorld->OnUiProperty(pData);
    pData->EndSection();
}


///////////////////////////////////////////////////////////////////////////
// UI button message handler
//
void RtkSimAgent::OnUiButton(RtkUiButtonData *pData)
{
}

