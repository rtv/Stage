///////////////////////////////////////////////////////////////////////////
//
// File: visionbeacon.hh
// Author: Andrew Howard
// Date: 6 Dec 2000
// Desc: Simulates ACTS vision beacons
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/visionbeacon.hh,v $
//  $Author: rtv $
//  $Revision: 1.1 $
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

#include "entity.hh"

class CVisionBeacon : public CEntity
{
    // Default constructor
    public: CVisionBeacon(CWorld *world, CEntity *parent);

    // Update the device
    public: virtual void Update( double sim_time );
};

#endif
