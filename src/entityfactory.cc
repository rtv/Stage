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
//  $Revision: 1.29 $
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

// TO ADD NEW DEVICES 
// - ADD A STRING TO CWorld::StringFromType()
// - ADD A CONSTRUCTOR CALL TO CWorld::CreateEntity()


// PUT YOUR DEVICE'S NAME IN HERE
// THIS STRING IS THE DEVICE'S NAME IN THE WORLD FILE
// AND IS USED AS A DESCRIPTIVE NAME IN THE GUI
char* CWorld::StringFromType( StageType t )
{
  switch( t )
  {
    case NullType: return "None"; 
    case WallType: return "wall"; break;
    case PlayerType: return "player"; 
    case MiscType: return "misc"; 
    case PositionType: return "position"; 
    case SonarType: return "sonar"; 
    case LaserTurretType: return "laser"; 
    case VisionType: return "vision"; 
    case PtzType: return "ptz"; 
    case BoxType: return "box"; 
    case LaserBeaconType: return "laserbeacon"; 
    case LBDType: return "lbd"; 
      //REMOVE case VisionBeaconType: return "vision_beacon"; 
    case GripperType: return "gripper"; 
    case AudioType: return "audio"; 
    case BroadcastType: return "broadcast"; 
    case SpeechType: return "speech"; 
    case TruthType: return "truth"; 
    case GpsType: return "gps"; 
    case PuckType: return "puck"; 
    case OccupancyType: return "occupancy"; 
    case IDARType: return "idar";
    case DescartesType: return "descartes";
    case BpsType: return "bps";
  }	 
  return( "unknown" );
}

// CONSTRUCT YOUR DEVICE HERE
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
      //REMOVE case VisionBeaconType:
      //return new CVisionBeacon(this, parent);
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
    case BpsType:
      return new CBpsDevice(this, parent);
    default:
      PRINT_WARN1("unknown type %d", type);
      return(NULL);
  }

  // case AudioType:
  // case SpeechType:
  //VBDType // Vision Beacon Detector?
  //case IDARType, // HRL's Infrared Data And Ranging turret
  //case DescartesType, // HRL's customized Descartes robot platform
  //case TruthType:
  //case OccupancyType:
}

//////////////////////////////////////////////////////////////////////////
// find the type number of an entity by looking up it's name
StageType CWorld::TypeFromString( const char* ent_string )
{
  for( StageType t = NullType; t< NUMBER_OF_STAGE_TYPES; ((int)t)++ )
       if( strcmp( ent_string, StringFromType(t) ) == 0 )
	 return t;
  
  return NullType; // string not found
}

/////////////////////////////////////////////////////////////////////////
// Create an entity given it's string name (used by server to create entities).
CEntity* CWorld::CreateEntity(const char *type_str, CEntity *parent )
{
  StageType t = TypeFromString( type_str );

  return CreateEntity( t, parent ); 
}

