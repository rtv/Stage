///////////////////////////////////////////////////////////////////////////
//
// File: entityfactory.cc
// Author: Andrew Howard
// Date: 05 Dec 2000
// Desc: Naked function for creating different types of objects
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/entityfactory.cc,v $
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

#include <string.h>
#include "entityfactory.hh"
#include "boxobstacle.hh"
#include "laserbeacon.hh"
#include "visionbeacon.hh"
#include "usc_pioneer.hh"


/////////////////////////////////////////////////////////////////////////
// Create an object given a type
//
CEntity* CreateObject(const char *type, CWorld *world, CEntity *parent)
{
    // Create box obstacle
    //
    if (strcmp(type, "box_obstacle") == 0)
        return new CBoxObstacle(world, parent);
    
    // Create laser beacon
    //
    if (strcmp(type, "laser_beacon") == 0)
        return new CLaserBeacon(world, parent);

    // Create vision beacon
    //
    if (strcmp(type, "vision_beacon") == 0)
        return new CVisionBeacon(world, parent);

    // Create a USC pioneer robot
    //
    if (strcmp(type, "usc_pioneer") == 0)
        return new CUscPioneer(world, parent);

    return NULL;
}

