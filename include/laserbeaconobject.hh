///////////////////////////////////////////////////////////////////////////
//
// File: laerbeaconobject.hh
// Author: Andrew Howard
// Date: 4 Dec 2000
// Desc: Simulates the laser beacons
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/include/laserbeaconobject.hh,v $
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

#ifndef LASERBEACONOBJECT_HH
#define LASERBEACONOBJECT_HH

#include "object.hh"

class CLaserBeaconObject : public CObject
{
    // Default constructor
    // Requires the position of the beacon relative to the parent object
    //
    public: CLaserBeaconObject(CWorld *world, CObject *parent);
    
    // Update the device
    //
    public: virtual void Update();

    // Current pose
    //
    private: double m_gx, m_gy, m_gth;

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
