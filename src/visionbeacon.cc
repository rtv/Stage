///////////////////////////////////////////////////////////////////////////
//
// File: visionbeacon.cc
// Author: Andrew Howard
// Date: 6 Dec 2000
// Desc: Simulates the ACTS beacons
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/visionbeacon.cc,v $
//  $Author: gerkey $
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

#include "world.hh"
#include "visionbeacon.hh"


///////////////////////////////////////////////////////////////////////////
// Default constructor
//
CVisionBeacon::CVisionBeacon(CWorld *world, CEntity *parent)
        : CEntity(world, parent)
{   
  m_stage_type = VisionBeaconType;

  // set the Player IO sizes correctly for this type of Entity
  m_data_len    = 0;
  m_command_len = 0;
  m_config_len  = 0;
  m_reply_len  = 0;

  vision_return = true;
  laser_return = LaserReflect;
  obstacle_return = true;
  sonar_return = true;

  m_player_port = 0; // not a player device
  m_player_type = 0;
  m_player_index = 0;

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

