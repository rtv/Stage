///////////////////////////////////////////////////////////////////////////
//
// File: visionbeacon.cc
// Author: Andrew Howard
// Date: 6 Dec 2000
// Desc: Simulates the ACTS beacons
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/visionbeacon.cc,v $
//  $Author: ahoward $
//  $Revision: 1.15.2.1 $
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
  laser_return = LaserSomething;
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
// Load the object from an argument list
//
bool CVisionBeacon::Load(int argc, char **argv)
{
  if (!CEntity::Load(argc, argv))
    return false;

  //printf( "beacon %p channel: %d\n", this, channel_return );

  for (int i = 0; i < argc;)
  {
    // Extract radius
    //
    if (strcmp(argv[i], "radius") == 0 && i + 1 < argc)
    {
      m_radius = atof(argv[i + 1]);
      i += 2;

	    m_size_x = m_size_y = 2.0 * m_radius;
    }

    // RTV - this'd report error before any subclass had a chance
    // Syntax error
    else
    {
      PLAYER_MSG1("unrecognized token [%s]", argv[i]);
      i += 1;
    }
  }

  return true;
}


///////////////////////////////////////////////////////////////////////////
// Save the object
//
bool CVisionBeacon::Save(int &argc, char **argv)
{
    if (!CEntity::Save(argc, argv))
        return false;

    // Save radius
    //
    char z[128];
    snprintf(z, sizeof(z), "%0.2f", (double) m_radius);
    argv[argc++] = strdup("radius");
    argv[argc++] = strdup(z);

    return true;
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
    //
    m_world->matrix->mode = mode_unset;
    m_world->SetCircle(m_map_px, m_map_py, m_radius, this );
	
    m_map_px = x;
    m_map_py = y;
    m_map_pth = th;
	
    // Draw our new representation
    //
    m_world->matrix->mode = mode_set;
    m_world->SetCircle(m_map_px, m_map_py, m_radius, this );
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



