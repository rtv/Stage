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


#include "robot.h"
#include "laserbeaconobject.hh"



/////////////////////////////////////////////////////////////////////////
// Create an object given a type
//
CObject* CreateObject(const char *type, CWorld *world, CObject *parent)
{
    if (strcmp(type, "robot") == 0)
        return new CRobot(world, parent);
    if (strcmp(type, "laser_beacon") == 0)
        return new CLaserBeaconObject(world, parent);
    return NULL;
}


