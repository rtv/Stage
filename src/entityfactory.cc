///////////////////////////////////////////////////////////////////////////
//
// File: entityfactory.cc
// Author: Andrew Howard
// Date: 05 Dec 2000
// Desc: CWorld method for creating different types of objects
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/entityfactory.cc,v $
//  $Author: rtv $
//  $Revision: 1.18 $
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
#include "fixedobstacle.hh"
#include "boxobstacle.hh"
#include "laserbeacon.hh"
#include "puck.hh"
#include "visionbeacon.hh"
#include "sonardevice.hh"
#include "positiondevice.hh"
#include "omnipositiondevice.hh"
#include "laserdevice.hh"

#include "playerdevice.hh"
#include "miscdevice.hh"
#include "ptzdevice.hh"
#include "visiondevice.hh"
#include "laserbeacondevice.hh"
#include "broadcastdevice.hh"
#include "gripperdevice.hh"
#include "gpsdevice.hh"

#ifdef HRL_HEADERS
#include "irdevice.hh"
#include "descartesdevice.hh"
#endif

#include "world.hh"

/////////////////////////////////////////////////////////////////////////
// Create an object given a type
//
CEntity* CWorld::CreateObject(const char *type, CEntity *parent )
{
  if (strcmp(type, "obstacle") == 0 )
    return new CFixedObstacle(this, parent);

  if (strcmp(type, "position_device") == 0)
    return new CPositionDevice(this, parent );

  if (strcmp(type, "omni_position_device") == 0)
    return new COmniPositionDevice(this, parent );

  if (strcmp(type, "player_device") == 0)
    return new CPlayerDevice(this, parent );

  if (strcmp(type, "laser_device") == 0)
    return new CLaserDevice(this, parent );

  if (strcmp(type, "sonar_device") == 0)
    return new CSonarDevice(this, parent );

  if (strcmp(type, "misc_device") == 0)
    return new CMiscDevice(this, parent );

  if (strcmp(type, "ptz_device") == 0)
    return new CPtzDevice(this, parent );
  
  if (strcmp(type, "box") == 0 )
    return new CBoxObstacle(this, parent);
    
  if (strcmp(type, "laser_beacon") == 0)
    return new CLaserBeacon(this, parent);
   
  if (strcmp(type, "lbd_device") == 0)
    return new CLBDDevice(this, (CLaserDevice*)parent );

  if (strcmp(type, "vision_device") == 0)
    return new CVisionDevice(this, (CPtzDevice*)parent);
        
  if (strcmp(type, "vision_beacon") == 0)
    return new CVisionBeacon(this, parent);

  if (strcmp(type, "movable_object") == 0)
    return new CPuck(this, parent);

  if (strcmp(type, "gps_device") == 0)
    return new CGpsDevice(this, parent);

  if (strcmp(type, "gripper_device") == 0)
    return new CGripperDevice(this, parent);

  if (strcmp(type, "broadcast_device") == 0)
    return new CBroadcastDevice(this, parent);
    
  if (strcmp(type, "puck") == 0)
    return new CPuck(this, parent);

#ifdef HRL_HEADERS 
  // these the proprietary HRL devices - the device code cannot be distributed
  // so they are implemented in an external library
  if (strcmp(type, "idar_device") == 0)
    return new CIDARDevice(this, parent);

  if (strcmp(type, "descartes_device") == 0)
    return new CDescartesDevice(this, parent);
#endif

  return NULL;
}




