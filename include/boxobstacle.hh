///////////////////////////////////////////////////////////////////////////
//
// File: boxobstacle.hh
// Author: Andrew Howard
// Date: 18 Dec 2000
// Desc: Simulates box obstacles
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/include/boxobstacle.hh,v $
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

#ifndef BOXOBSTACLE_HH
#define BOXOBSTACLE_HH

#include "object.hh"

class CBoxObstacle : public CObject
{
    // Default constructor
    //
    public: CBoxObstacle(CWorld *world, CObject *parent);

    // Initialise object
    //
    public: virtual bool Startup(RtkCfgFile *cfg);
    
    // Update the device
    //
    public: virtual void Update();

    // Box dimensions
    //
    private: double m_size_x, m_size_y;
    
    // Current pose
    //
    private: double m_map_px, m_map_py, m_map_pth;

#ifdef INCLUDE_RTK
    
    // Process GUI update messages
    //
    public: virtual void OnUiUpdate(RtkUiDrawData *pData);

    // Process GUI mouse messages
    //
    public: virtual void OnUiMouse(RtkUiMouseData *pData);

#endif
};

#endif
