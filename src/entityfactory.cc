///////////////////////////////////////////////////////////////////////////
//
// File: entityfactory.cc
// Author: Andrew Howard
// Date: 05 Dec 2000
// Desc: CWorld method for creating different types of objects
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/entityfactory.cc,v $
//  $Author: inspectorg $
//  $Revision: 1.23 $
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
#include "bpsdevice.hh"
#include "gripperdevice.hh"
#include "gpsdevice.hh"
#include "motedevice.hh"

#ifdef HRL_HEADERS
#include "irdevice.hh"
#include "descartesdevice.hh"
#endif

#include "world.hh"


//  const char** object_types =
//  { 
//    "mote_device", 
//    "obstacle", 
//    "position_device", 
//    "omni_position_device", 
//    "player_device",
//    "laser_device",
//    "sonar_device",
//    "misc_device",
//    "ptz_device",
//    "box",
//    "laser_beacon",
//    "lbd_device",
//    "vision_device",
//    "vision_beacon",
//    "movable_object",
//    "gps_device",
//    "gripper_device", 
//    "puck" 
//  };

/////////////////////////////////////////////////////////////////////////
// Create an object given a type
//
CEntity* CWorld::CreateObject( StageType type, CEntity *parent )
{ 
  switch( type )
    {
    case NullType:
      puts( "Stage Warning: create null device type request. Ignored." );
      return NULL;
    case WallType:
      return new CFixedObstacle(this, parent);     
    case PlayerType:
      return new CPlayerDevice(this, parent );
    case MiscType: 
      return new CMiscDevice(this, parent );
    case RectRobotType:
    case RoundRobotType:
      return new CPositionDevice(this, parent );
    case SonarType:
      return new CSonarDevice(this, parent );
    case LaserTurretType:
      return new CLaserDevice(this, parent );
    case VisionType:
      return new CVisionDevice(this, (CPtzDevice*)parent);
    case PtzType:
      return new CPtzDevice(this, parent );
    case BoxType: 
      return new CBoxObstacle(this, parent);
    case LaserBeaconType:
      return new CLaserBeacon(this, parent);
      // temporarily broken
      //case LBDType: // Laser Beacon Detector 
      //return new CLBDDevice(this, (CLaserDevice*)parent );
    case VisionBeaconType:
      return new CVisionBeacon(this, parent);
    case GripperType:
      return new CGripperDevice(this, parent);
    case GpsType:
      return new CGpsDevice(this, parent);
    case PuckType:
      return new CPuck(this, parent);
    case BroadcastType:
      return new CBroadcastDevice(this, parent);
    case OmniPositionType:
      return new COmniPositionDevice(this, parent );
    case MoteType:
      return new CMoteDevice(this, parent );

    default:
      printf( "Stage Warning: CreateObject() Unknown type %d", type );
    }

  // case AudioType:
  // case SpeechType:
  //VBDType // Vision Beacon Detector?
  //case IDARType, // HRL's Infrared Data And Ranging turret
  //case DescartesType, // HRL's customized Descartes robot platform
  //case TruthType:
  //case OccupancyType:
}

/////////////////////////////////////////////////////////////////////////
// Create an object given a type
//
CEntity* CWorld::CreateObject(const char *type, CEntity *parent )
{ 
  if (strcmp(type, "mote") == 0)
    return new CMoteDevice(this, parent );

  if (strcmp(type, "obstacle") == 0 )
    return new CFixedObstacle(this, parent);

  if (strcmp(type, "position") == 0)
    return new CPositionDevice(this, parent );

  if (strcmp(type, "omni_position") == 0)
    return new COmniPositionDevice(this, parent );

  if (strcmp(type, "player") == 0)
    return new CPlayerDevice(this, parent );

  if (strcmp(type, "laser") == 0)
    return new CLaserDevice(this, parent );

  if (strcmp(type, "sonar") == 0)
    return new CSonarDevice(this, parent );

  if (strcmp(type, "misc") == 0)
    return new CMiscDevice(this, parent );

  if (strcmp(type, "ptz") == 0)
    return new CPtzDevice(this, parent );
  
  if (strcmp(type, "box") == 0 )
    return new CBoxObstacle(this, parent);
    
  if (strcmp(type, "laser_beacon") == 0)
    return new CLaserBeacon(this, parent);
   
  if (strcmp(type, "lbd") == 0)
    return new CLBDDevice(this, (CLaserDevice*)parent );

  if (strcmp(type, "vision") == 0)
    return new CVisionDevice(this, (CPtzDevice*)parent);
        
  if (strcmp(type, "vision_beacon") == 0)
    return new CVisionBeacon(this, parent);

  if (strcmp(type, "movable_object") == 0)
    return new CPuck(this, parent);

  if (strcmp(type, "gps") == 0)
    return new CGpsDevice(this, parent);

  if (strcmp(type, "gripper") == 0)
    return new CGripperDevice(this, parent);

  if (strcmp(type, "broadcast") == 0)
    return new CBroadcastDevice(this, parent);

  if (strcmp(type, "bps") == 0)
    return new CBpsDevice(this, parent);
    
  if (strcmp(type, "puck") == 0)
    return new CPuck(this, parent);

#ifdef HRL_HEADERS 
  // these the proprietary HRL devices - the device code cannot be distributed
  // so they are implemented in an external library
  if (strcmp(type, "idar") == 0)
    return new CIDARDevice(this, parent);

  if (strcmp(type, "descartes") == 0)
    return new CDescartesDevice(this, parent);
#endif

  return NULL;
}




