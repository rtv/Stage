///////////////////////////////////////////////////////////////////////////
//
// File: entityfactory.cc
// Author: Andrew Howard
// Date: 05 Dec 2000
// Desc: Naked function for creating different types of objects
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/entityfactory.cc,v $
//  $Author: vaughan $
//  $Revision: 1.11 $
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
#include "sonardevice.hh"
#include "positiondevice.hh"
#include "laserdevice.hh"

#include "playerdevice.hh"
#include "miscdevice.hh"
#include "ptzdevice.hh"
#include "visiondevice.hh"
#include "laserbeacondevice.hh"
#include "broadcastdevice.hh"
#include "gripperdevice.hh"
#include "gpsdevice.hh"
//#include "truthdevice.hh"
//#include "occupancydevice.hh"


/////////////////////////////////////////////////////////////////////////
// Create an object given a type
//
CEntity* CreateObject(const char *type, CWorld *world, CEntity *parent)
{
    if (strcmp(type, "position_device") == 0)
        return new CPositionDevice(world, parent );

    if (strcmp(type, "player_device") == 0)
        return new CPlayerDevice(world, parent );

    if (strcmp(type, "laser_device") == 0)
        return new CLaserDevice(world, parent );

    if (strcmp(type, "sonar_device") == 0)
        return new CSonarDevice(world, parent );

    if (strcmp(type, "misc_device") == 0)
        return new CMiscDevice(world, parent );

    if (strcmp(type, "ptz_device") == 0)
        return new CPtzDevice(world, parent );

    if (strcmp(type, "box") == 0 )
      return new CBoxObstacle(world, parent);
    
    if (strcmp(type, "laser_beacon") == 0)
      return new CLaserBeacon(world, parent);
   
    if (strcmp(type, "lbd_device") == 0)
      return new CLBDDevice(world, (CLaserDevice*)parent );

    if (strcmp(type, "vision_device") == 0)
      return new CVisionDevice(world, (CPtzDevice*)parent );
        
    if (strcmp(type, "vision_beacon") == 0)
      return new CVisionBeacon(world, parent);

    if (strcmp(type, "movable_object") == 0)
      return new CPuck(world, parent);

    if (strcmp(type, "gps_device") == 0)
      return new CGpsDevice(world, parent);

    if (strcmp(type, "gripper_device") == 0)
      return new CGripperDevice(world, parent);

    if (strcmp(type, "puck") == 0)
      return new CPuck(world, parent);

    // TODO - devices in various stages of brokeness
    
    if (strcmp(type, "broadcast_device") == 0)
      return new CBroadcastDevice(world, parent);
    
    // these devices are probably abandonded, replaced by
    // truth and env servers

    //if (strcmp(type, "truth_device") == 0)
    //  return new CTruthDevice(world, parent );

    //if (strcmp(type, "occupancy_device") == 0)
    //  return new COccupancyDevice(world, parent );

    return NULL;
}




