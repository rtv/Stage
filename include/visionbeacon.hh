///////////////////////////////////////////////////////////////////////////
//
// File: visionbeacon.hh
// Author: Andrew Howard
// Date: 6 Dec 2000
// Desc: Simulates ACTS vision beacons
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/include/visionbeacon.hh,v $
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

#ifndef VISIONBEACON_HH
#define VISIONBEACON_HH

#include "object.hh"

class CVisionBeacon : public CObject
{
    // Default constructor
    //
    public: CVisionBeacon(CWorld *world, CObject *parent);

    // Initialise object
    //
    public: virtual bool Startup(RtkCfgFile *cfg);
    
    // Update the device
    //
    public: virtual void Update();

    // Beacon radius
    //
    private: double m_radius;
    
    // Beacon channel (corresponds to ACTS channel)
    //
    private: int m_channel;

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
