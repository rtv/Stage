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
//  $Revision: 1.25 $
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
#include "truthdevice.hh"

#ifdef HRL_HEADERS
#include "irdevice.hh"
#include "descartesdevice.hh"
#endif

#include "world.hh"


/////////////////////////////////////////////////////////////////////////
// Create an entity given a type (used by server to create entities).
CEntity* CWorld::CreateEntity(const char *type, CEntity *parent )
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

  if (strcmp(type, "truth") == 0)
    return new CTruthDevice(this, parent);

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


/////////////////////////////////////////////////////////////////////////
// Create an entity given a type (used by client to create entities).
CEntity* CWorld::CreateEntity( StageType type, CEntity *parent)
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
    case PositionType:
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
    case LBDType:
      return new CLBDDevice(this, (CLaserDevice*)parent );
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
    case TruthType:
      return new CTruthDevice(this, parent );

    default:
      PRINT_WARN1("unknown type %d", type);
  }

  // case AudioType:
  // case SpeechType:
  //VBDType // Vision Beacon Detector?
  //case IDARType, // HRL's Infrared Data And Ranging turret
  //case DescartesType, // HRL's customized Descartes robot platform
  //case TruthType:
  //case OccupancyType:
}


