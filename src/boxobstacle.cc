///////////////////////////////////////////////////////////////////////////
//
// File: boxobstacle.cc
// Author: Andrew Howard
// Date: 18 Dec 2000
// Desc: Simulates box obstacles
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/boxobstacle.cc,v $
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

#define ENABLE_RTK_TRACE 1

#include "world.hh"
#include "boxobstacle.hh"


///////////////////////////////////////////////////////////////////////////
// Default constructor
//
CBoxObstacle::CBoxObstacle(CWorld *world, CEntity *parent)
        : CEntity(world, parent)
{
  // Set default shape and geometry
  this->shape = ShapeRect;
  this->size_x = 1.0;
  this->size_y = 1.0;
  
  SetColor(BOX_COLOR);
  
  m_stage_type = BoxType;

  vision_return = true; 
  laser_return = LaserReflect;
  sonar_return = true;
  obstacle_return = true;
}


///////////////////////////////////////////////////////////////////////////
// Update the laser data
//
void CBoxObstacle::Update( double simtime )
{
  CEntity::Update(simtime);
  
  double x, y, th;
  GetGlobalPose( x,y,th );

  ReMap(x, y, th);
}




