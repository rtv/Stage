///////////////////////////////////////////////////////////////////////////
//
// File: entityfactory.cc
// Author: Andrew Howard
// Date: 05 Dec 2000
// Desc: Naked function for creating different types of objects
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/entityfactory.cc,v $
//  $Author: gerkey $
//  $Revision: 1.7 $
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
#include "puck.hh"
#include "visionbeacon.hh"
#include "usc_pioneer.hh"
#include "pioneermobiledevice.hh"
#include "laserdevice.hh"
#include "laserbeacondevice.hh"
#include "playerserver.hh"
#include "gripperdevice.hh"
#include "sonardevice.hh"
#include "ptzdevice.hh"
#include "visiondevice.hh"

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
    
    // Create puck
    //
    if (strcmp(type, "movable_object") == 0)
        return new CPuck(world, parent);
    
    // Create gripper
    //
    if (strcmp(type, "gripper_device") == 0)
        return new CGripperDevice(world, parent, NULL);
    
    // Create sonar
    //
    if (strcmp(type, "sonar_device") == 0)
        return new CSonarDevice(world, parent, NULL);
    
    // Create camera
    //
    if (strcmp(type, "ptz_device") == 0)
        return new CPtzDevice(world, parent, NULL);
    
    // Create vision device (ACTS)
    //
    if (strcmp(type, "vision_device") == 0)
        return new CVisionDevice(world, parent, NULL, NULL);


    // Create vision beacon
    //
    if (strcmp(type, "vision_beacon") == 0)
        return new CVisionBeacon(world, parent);

    // Create a USC pioneer robot (a pioneer loaded with gadgets)
    //
    if (strcmp(type, "usc_pioneer") == 0)
        return new CUscPioneer(world, parent);

    // Create a Player server
    //
    if (strcmp(type, "player_server") == 0)
        return new CPlayerServer(world, parent);

    // Create a pioneer
    //
    if (strcmp(type, "position_device") == 0)
        return new CPioneerMobileDevice(world, parent, NULL);

    // Create a laser
    //
    if (strcmp(type, "laser_device") == 0)
        return new CLaserDevice(world, parent, NULL);

    // Create a laser
    //
    if (strcmp(type, "laser_beacon_device") == 0)
        return new CLaserBeaconDevice(world, parent, NULL, NULL);

    return NULL;
}



