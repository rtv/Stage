///////////////////////////////////////////////////////////////////////////
//
// File: RtkSimAgent.hh
// Author: Andrew Howard
// Date: 4 Dec 2000
// Desc: Simple viewing agent for Player
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/include/rtksimagent.hh,v $
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



#include "rtk-agent.hh"
#include "rtk-ui.hh"

// Need a pointer to the world
//
#include "world.hh"


class RtkSimAgent :
    public RtkAgent,
    public UiForceUpdateSource,
    public UiDrawSink,
    public UiMouseSink,
    public UiPropertySink,
    public UiButtonSink
{
    // Default constructor
	//
	public: RtkSimAgent(const char *pszWorldFile);

	// Destructor
	//
	public: virtual ~RtkSimAgent();

	// Cfgtialisation
	//
	public: virtual BOOL Open(RtkCfgFile *pCfgFile);

	// Close function
	//
	public: virtual void Close();

	// Save function
	//
	public: virtual void Save(RtkCfgFile *pCfgFile);

    // Start any threads/timers for this module
	//
	public: virtual BOOL Start();
	
	// Stop any threads/timers for this module
	//
	public: virtual BOOL Stop();

    // Main agent loop
    //
    public: virtual void Main();

	// UI draw message handler
	//
	public: virtual void OnUiDraw(RtkUiDrawData *pData);
    
	// UI mouse message handler
	//
	public: virtual void OnUiMouse(RtkUiMouseData *pData);
    
	// UI property message handler
	//
	public: virtual void OnUiProperty(RtkUiPropertyData *pData);

    // UI button message handler
	//
	public: virtual void OnUiButton(RtkUiButtonData *pData);

    // Name of file containing world spec
    //
    private: RtkString m_strWorldFile;

    // Pointer to world
    //
    private: CWorld *m_pWorld;
};





