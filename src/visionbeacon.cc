///////////////////////////////////////////////////////////////////////////
//
// File: visionbeacon.cc
// Author: Andrew Howard
// Date: 6 Dec 2000
// Desc: Simulates the ACTS beacons
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/visionbeacon.cc,v $
//  $Author: inspectorg $
//  $Revision: 1.16 $
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

  vision_return = true;
  laser_return = LaserReflect;
  obstacle_return = true;
  sonar_return = true;

  m_player_port = 0; // not a player device
  m_player_type = 0;
  m_player_index = 0;
    
  m_radius = 0.1; // our beacons are 10cm radius
  m_size_x = m_radius * 2.0;
  m_size_y = m_radius * 2.0;

  // Set the initial map pose
  //
  m_map_px = m_map_py = m_map_pth = 0;
}


///////////////////////////////////////////////////////////////////////////
// Update the laser data
//
void CVisionBeacon::Update( double sim_time )
{
  ASSERT(m_world != NULL);
    
  double x, y, th;
  GetGlobalPose( x,y,th );
    
  // if we've moved 
  //if( (m_map_px != x) || (m_map_py != y) || (m_map_pth != th ) )
  {
    m_last_update = sim_time;
	
    // Undraw our old representation
    m_world->SetCircle(m_map_px, m_map_py, m_radius, this, false);
	
    m_map_px = x;
    m_map_py = y;
    m_map_pth = th;
	
    // Draw our new representation
    m_world->SetCircle(m_map_px, m_map_py, m_radius, this, true);
  }
}


#ifdef INCLUDE_RTK

///////////////////////////////////////////////////////////////////////////
// Process GUI update messages
//
void CVisionBeacon::OnUiUpdate(RtkUiDrawData *data)
{
    CEntity::OnUiUpdate(data);

    data->begin_section("global", "");
    
    if (data->draw_layer("vision_beacon", true))
    {
      data->set_color(RTK_RGB(m_color.red, m_color.green, m_color.blue));
      data->ellipse(m_map_px-m_radius, m_map_py-m_radius, 
                    m_map_px+m_radius,m_map_py+m_radius);
    }

    data->end_section();
}


///////////////////////////////////////////////////////////////////////////
// Process GUI mouse messages
//
void CVisionBeacon::OnUiMouse(RtkUiMouseData *data)
{
    CEntity::OnUiMouse(data);
}

#endif



