///////////////////////////////////////////////////////////////////////////
//
// File: createobject.cc
// Author: Andrew Howard
// Date: 05 Dec 2000
// Desc: Naked C function for creating different types of objects
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/objectfactory.cc,v $
//  $Author: ahoward $
//  $Revision: 1.1.2.10 $
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

#include "objectfactory.hh"

// Obstacles
//
#include "boxobstacle.hh"

// Beacons
//
#include "laserbeacon.hh"
#include "visionbeacon.hh"

// Player devices
//
#include "playerrobot.hh"
#include "pioneermobiledevice.hh"
#include "sonardevice.hh"
#include "laserdevice.hh"
#include "ptzdevice.hh"
#include "visiondevice.hh"
#include "beacondevice.hh"
#include "broadcastdevice.hh"


/////////////////////////////////////////////////////////////////////////
// Useful macros

#define FIND_PLAYER_ROBOT() CPlayerRobot *robot = (CPlayerRobot*) \
                            parent->FindAncestor(typeid(CPlayerRobot)); \
                            if (robot == NULL) \
                            { \
                                MSG("player device does not have player robot as ancestor;" \
                                    "device ignored"); \
                                return NULL; \
                            } 

/////////////////////////////////////////////////////////////////////////
// Create an object given a type
// Additional arguments can be passed in through argv.
//
CObject* CreateObject(const char *type, CWorld *world, CObject *parent)
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

    // Create player robot
    //
    if (strcmp(type, "player_robot") == 0)
        return new CPlayerRobot(world, parent);
        
    // Create pioneer device
    //
    if (strcmp(type, "pioneer") == 0)
    {
        FIND_PLAYER_ROBOT();
        return new CPioneerMobileDevice(world, parent, robot);
    }
    
    // Create sonar device
    //
    if (strcmp(type, "sonar") == 0)
    {
        FIND_PLAYER_ROBOT();
        return new CSonarDevice(world, parent, robot);
    }

    // Create laser device
    //
    if (strcmp(type, "laser") == 0)
    {
        FIND_PLAYER_ROBOT();
        return new CLaserDevice(world, parent, robot);
    }
    
    // Create ptz camera device
    //
    if (strcmp(type, "ptzcamera") == 0)
    {
        FIND_PLAYER_ROBOT();
        return new CPtzDevice(world, parent, robot);
    }
    
    // Create vision device
    //
    if (strcmp(type, "vision") == 0)
    {
        FIND_PLAYER_ROBOT();
        CPtzDevice *ptz = (CPtzDevice*) parent->FindAncestor(typeid(CPtzDevice));
        if (ptz == NULL)
        {
            MSG("vision device requires ptz camera as ancestor; ignoring");
            return NULL;
        }
        return new CVisionDevice(world, parent, robot, ptz);
    }
    
    // Create beacon detector device
    //
    if (strcmp(type, "beacon") == 0)
    {
        FIND_PLAYER_ROBOT();
        CLaserDevice *laser = (CLaserDevice*) parent->FindAncestor(typeid(CLaserDevice));
        if (laser == NULL)
        {
            MSG("beacon device requires laser as ancestor; ignoring");
            return NULL;
        }      
        return new CBeaconDevice(world, parent, robot, laser);
    }

    // Create broadcast device
    //
    if (strcmp(type, "broadcast") == 0)
    {
        FIND_PLAYER_ROBOT();
        return new CBroadcastDevice(world, parent, robot);
    }

    return NULL;
}




