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

#include "objectfactory.hh"
#include "laserbeaconobject.hh"

// Player objects
//
#include "playerrobot.hh"
#include "pioneermobiledevice.hh"
#include "laserdevice.hh"


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
    if (strcmp(type, "robot") == 0)
        return new CPlayerRobot(world, parent);
    if (strcmp(type, "laser_beacon") == 0)
        return new CLaserBeaconObject(world, parent);

    // Create pioneer device
    //
    if (strcmp(type, "pioneer") == 0)
    {
        FIND_PLAYER_ROBOT();
        return new CPioneerMobileDevice(world, parent, robot,
                                        robot->playerIO + SPOSITION_DATA_START,
                                        SPOSITION_TOTAL_BUFFER_SIZE);
    }
    
    // Create laser device
    //
    if (strcmp(type, "laser") == 0)
    {
        FIND_PLAYER_ROBOT();
        return new CLaserDevice(world, parent, robot,
                                robot->playerIO + LASER_DATA_START,
                                LASER_TOTAL_BUFFER_SIZE);
    }
    
    return NULL;
}


