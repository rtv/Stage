///////////////////////////////////////////////////////////////////////////
//
// File: boxobstacle.cc
// Author: Andrew Howard
// Date: 18 Dec 2000
// Desc: Simulates box obstacles
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/boxobstacle.cc,v $
//  $Author: ahoward $
//  $Revision: 1.13.2.1 $
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
  // set the Player IO sizes correctly for this type of Entity
  m_data_len    = 0;
  m_command_len = 0;
  m_config_len  = 0;
  
  m_player_port = 0; // not a player device
  m_player_type = 0;
  m_player_index = 0;
  
  m_size_x = 1.0;
  m_size_y = 1.0;
  
  m_color_desc = BOX_COLOR;
  
  m_stage_type = BoxType;

  vision_return = true; 
  laser_return = LaserSomething;
  sonar_return = true;
  obstacle_return = true;

  // Set the initial map pose
  //
  m_map_px = m_map_py = m_map_pth = 0;
}


///////////////////////////////////////////////////////////////////////////
// Load the object from an argument list
//
bool CBoxObstacle::Load(int argc, char **argv)
{
    if (!CEntity::Load(argc, argv))
        return false;

    for (int i = 0; i < argc;)
    {
        // Extract box size
        //
        if (strcmp(argv[i], "size") == 0 && i + 2 < argc)
        {
            m_size_x = atof(argv[i + 1]);
            m_size_y = atof(argv[i + 2]);
            i += 3;
        }
        else
            i++;
    }

    return true;
}


///////////////////////////////////////////////////////////////////////////
// Save the object
//
bool CBoxObstacle::Save(int &argc, char **argv)
{
    if (!CEntity::Save(argc, argv))
        return false;
    
    // Convert to strings
    //
    char sx[32];
    snprintf(sx, sizeof(sx), "%.2f", m_size_x);
    char sy[32];
    snprintf(sy, sizeof(sy), "%.2f", m_size_y);

    // Add to argument list
    //
    argv[argc++] = strdup("size");
    argv[argc++] = strdup(sx);
    argv[argc++] = strdup(sy);
        
    return true;
}


///////////////////////////////////////////////////////////////////////////
// Update the laser data
//
void CBoxObstacle::Update( double simtime )
{
  ASSERT(m_world);
  
  // See if its time to recalculate boxes
  //
  //if( simtime - m_last_update < m_interval )
  //return;
    
  double x, y, th;
  GetGlobalPose( x,y,th );

  m_last_update = simtime;
  
  // if we've moved 
  if( (m_map_px != x) || (m_map_py != y) || (m_map_pth != th ) )
    {    
      // Undraw our old representation
      m_world->matrix->SetMode( mode_unset );
      m_world->SetRectangle( m_map_px, m_map_py, m_map_pth,
			     m_size_x, m_size_y, this );
      
      m_map_px = x; // update the render positions
      m_map_py = y;
      m_map_pth = th;
      
      // Draw our new representation
      m_world->matrix->SetMode( mode_set );
      m_world->SetRectangle( m_map_px, m_map_py, m_map_pth,
			     m_size_x, m_size_y, this );
    }
}

#ifdef INCLUDE_RTK

///////////////////////////////////////////////////////////////////////////
// Process GUI update messages
void CBoxObstacle::OnUiUpdate(RtkUiDrawData *data)
{
    CEntity::OnUiUpdate(data);

    data->begin_section("global", "");
    
    if (data->draw_layer("box_obstacle", true))
    {
        double ox, oy, oth;
        GetGlobalPose(ox, oy, oth);
        data->set_color(RTK_RGB(m_color.red, m_color.green, m_color.blue));
        data->ex_rectangle(ox, oy, oth, m_size_x, m_size_y);
    }

    data->end_section();
}


///////////////////////////////////////////////////////////////////////////
// Process GUI mouse messages
//
void CBoxObstacle::OnUiMouse(RtkUiMouseData *data)
{
    CEntity::OnUiMouse(data);
}

#endif




