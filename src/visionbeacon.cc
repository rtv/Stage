///////////////////////////////////////////////////////////////////////////
//
// File: visionbeacon.cc
// Author: Andrew Howard
// Date: 6 Dec 2000
// Desc: Simulates the ACTS beacons
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/visionbeacon.cc,v $
//  $Author: rtv $
//  $Revision: 1.19 $
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

#include "world.hh"
#include "visionbeacon.hh"


///////////////////////////////////////////////////////////////////////////
// Default constructor
//
CVisionBeacon::CVisionBeacon(CWorld *world, CEntity *parent)
        : CEntity(world, parent)
{   
  m_stage_type = VisionBeaconType;

  vision_return = true;
  laser_return = LaserReflect;
  obstacle_return = true;
  sonar_return = true;

  // Set default shape and geometry
  this->shape = ShapeCircle;
  this->size_x = 0.20;
  this->size_y = 0.20;
}


///////////////////////////////////////////////////////////////////////////
// Update the laser data
//
void CVisionBeacon::Update( double sim_time )
{
  ASSERT(m_world != NULL);

  CEntity::Update(sim_time);
  
  double x, y, th;
  GetGlobalPose( x,y,th );

  ReMap(x, y, th);
}

