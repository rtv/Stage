///////////////////////////////////////////////////////////////////////////
//
// File: laserbeacon.cc
// Author: Andrew Howard
// Date: 4 Dec 2000
// Desc: Simulates the Player CLaserDevice (the SICK laser)
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/laserbeacon.cc,v $
//  $Author: gerkey $
//  $Revision: 1.20 $
//
// Usage:
//  This object acts a both a simple laser reflector and a more complex
//  laser beacon with an id.
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

//#define ENABLE_RTK_TRACE 1

#include "world.hh"
#include "laserbeacon.hh"
#include "laserdevice.hh" // for the laser return values


///////////////////////////////////////////////////////////////////////////
// Default constructor
//
CLaserBeacon::CLaserBeacon(CWorld *world, CEntity *parent)
        : CEntity(world, parent)
{
  // set the Player IO sizes correctly for this type of Entity
  m_data_len    = 0;
  m_command_len = 0;
  m_config_len  = 0;
  m_reply_len  = 0;

  // This is not a player device
  m_player_port = 0; 
  m_player_index = 0;
  m_player_type = 0;

  // Set default shape and geometry
  this->shape = ShapeRect;
  this->size_x = 0.05; 
  this->size_y = 0.3;     

  SetColor(LASERBEACON_COLOR);
  m_stage_type = LaserBeaconType;
  
  this->id = 0;
    
  // This object is visible to lasers
  // and is reflective
  this->laser_return = LaserBright1;
}


///////////////////////////////////////////////////////////////////////////
// Load the entity from the worldfile
bool CLaserBeacon::Load(CWorldFile *worldfile, int section)
{
  if (!CEntity::Load(worldfile, section))
    return false;

  // Read the beacon id
  this->id = worldfile->ReadInt(section, "id", 0);

  // Use the beacon id as a name if there is none
  // This will leak memory, but I dont really care.
  if (!this->name || strlen(this->name) == 0)
  {
    char *name = (char*) malloc(64);
    snprintf(name, 64, "id %d", this->id);
    this->name = name;
  }
  
  return true;
}


///////////////////////////////////////////////////////////////////////////
// Update the laser data
//
void CLaserBeacon::Update( double sim_time )
{
  CEntity::Update(sim_time);
    
  double x, y, th;
  GetGlobalPose( x,y,th );

  ReMap(x, y, th);
}






