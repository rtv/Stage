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
//  $Revision: 1.1.2.6 $
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
RtkSimAgent::RtkSimAgent(const char *pszWorldFile)
{
    m_strWorldFile = pszWorldFile;
    m_pWorld = NULL;
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

    m_pWorld = new CWorld;    

    // Start the world
    //
    RtkCfgFile oWorldCfg;
    if (!oWorldCfg.Open(CSTR(m_strWorldFile)))
        return false;

    // Create all the objects in the world
    //
    if (!m_pWorld->CreateChildren(&oWorldCfg))
        return false;

    // Start all the objects in the world
    //
    if (!m_pWorld->Startup(&oWorldCfg))
        return false;
    if (!m_pWorld->StartupChildren(&oWorldCfg))
        return false;
    
    oWorldCfg.Close();
    
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
    // Stop the objects
    //
    m_pWorld->ShutdownChildren();
    m_pWorld->Shutdown();
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
	StartThread(-1);
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
    //RTK_TRACE("RtkSimAgent::Main\n");
  
    // Update the GUI now.
    //
    SendUiForceUpdate(NULL);
    
    while (!Quit())
    {
        // Update the world
        //
        for (int i = 0; i < 3; i++)
        {
            // *** HACK -- sleep time?
            //
            Sleep(25);
            m_pWorld->Update();
            m_pWorld->UpdateChildren();
        }

        //RTK_TRACE1("time %d", (int) GetTime());
        
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
    m_pWorld->OnUiUpdate(pData);
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

