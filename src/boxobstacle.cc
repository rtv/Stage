///////////////////////////////////////////////////////////////////////////
//
// File: boxobstacle.cc
// Author: Andrew Howard
// Date: 18 Dec 2000
// Desc: Simulates box obstacles
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/boxobstacle.cc,v $
//  $Author: vaughan $
//  $Revision: 1.3 $
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
  
  m_stage_type = BoxType;

  m_size_x = 1.0;
  m_size_y = 1.0;
  
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

#ifdef INCLUDE_RTK
    m_mouse_radius = max(m_size_x, m_size_y) * 0.6;
    m_draggable = (m_parent_object == NULL);
#endif
    
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
  //cout << "UPDATE BOX" << endl;
  if( Subscribed() ) // i.e. our truth has been poked
    {
      ASSERT(m_world);
      
      // Undraw our old representation
      //
      m_world->SetRectangle(m_map_px, m_map_py, m_map_pth,
			    m_size_x, m_size_y, layer_obstacle, 0);
      m_world->SetRectangle(m_map_px, m_map_py, m_map_pth,
			    m_size_x, m_size_y, layer_laser, 0);
      
      // Update our global pose
      //
      GetGlobalPose(m_map_px, m_map_py, m_map_pth);
      
      // Draw our new representation
      //
      m_world->SetRectangle(m_map_px, m_map_py, m_map_pth,
			    m_size_x, m_size_y, layer_obstacle, 1);
      m_world->SetRectangle(m_map_px, m_map_py, m_map_pth,
                          m_size_x, m_size_y, layer_laser, 1);
    }
}

#ifdef INCLUDE_RTK

///////////////////////////////////////////////////////////////////////////
// Process GUI update messages
//
void CBoxObstacle::OnUiUpdate(RtkUiDrawData *pData)
{
    CEntity::OnUiUpdate(pData);

    pData->begin_section("global", "");
    
    if (pData->draw_layer("box_obstacle", true))
    {
        double ox, oy, oth;
        GetGlobalPose(ox, oy, oth);
        pData->set_color(m_color);
        pData->ex_rectangle(ox, oy, oth, m_size_x, m_size_y);
    }

    pData->end_section();
}


///////////////////////////////////////////////////////////////////////////
// Process GUI mouse messages
//
void CBoxObstacle::OnUiMouse(RtkUiMouseData *pData)
{
    CEntity::OnUiMouse(pData);
}

#endif




