///////////////////////////////////////////////////////////////////////////
//
// File: laserbeacon.cc
// Author: Andrew Howard
// Date: 4 Dec 2000
// Desc: Simulates the Player CLaserDevice (the SICK laser)
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/laserbeacon.cc,v $
//  $Author: rtv $
//  $Revision: 1.25 $
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


// register this device type with the Library
CEntity laserbeacon_bootstrap( "laserbeacon", 
			       LaserBeaconType, 
			       (void*)&CLaserBeacon::Creator ); 

///////////////////////////////////////////////////////////////////////////
// Default constructor
//
CLaserBeacon::CLaserBeacon(CWorld *world, CEntity *parent)
        : CEntity(world, parent)
{
  this->stage_type = LaserBeaconType;
  this->color = ::LookupColor(LASERBEACON_COLOR);
  
  this->shape = ShapeRect;
  this->size_x = 0.05; 
  this->size_y = 0.3;     
  
  this->id = 0;
    
  // This object is visible to lasers
  // and is reflective
  this->laser_return = LaserBright;
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
  if ( strlen(this->name) == 0)
    snprintf( this->name, 64, "id %d", this->id);
  
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






