///////////////////////////////////////////////////////////////////////////
//
// File: laerbeacon.hh
// Author: Andrew Howard
// Date: 4 Dec 2000
// Desc: Simulates the laser beacons
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/include/laserbeacon.hh,v $
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

#ifndef LASERBEACON_HH
#define LASERBEACON_HH

#include "object.hh"

class CLaserBeacon : public CObject
{
    // Default constructor
    //
    public: CLaserBeacon(CWorld *world, CObject *parent);

    // Initialise the object from an argument list
    //
    public: virtual bool Init(int argc, char **argv);
    
    // Startup routine
    //
    public: virtual bool Startup();
    
    // Update the device
    //
    public: virtual void Update();

    // Beacon id
    //
    private: int m_beacon_id;

    // Current mapped pose
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
