///////////////////////////////////////////////////////////////////////////
//
// File: laserbeacon.cc
// Author: Andrew Howard
// Date: 4 Dec 2000
// Desc: Simulates the Player CLaserDevice (the SICK laser)
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/laserbeacon.cc,v $
//  $Author: inspectorg $
//  $Revision: 1.17 $
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

  // This is not a player device
  m_player_port = 0; 
  m_player_index = 0;
  m_player_type = 0;

  m_size_x = 0.05; // very thin!   
  m_size_y = 0.3;     

  m_color_desc = LASERBEACON_COLOR;
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
    
  // if we've moved 
  if( (m_map_px != x) || (m_map_py != y) || (m_map_pth != th ) )
  {
    m_last_update = sim_time;
	
    // Undraw our old representation
    m_world->SetRectangle( m_map_px, m_map_py, m_map_pth, 
                             m_size_x, m_size_y, this, false);
	
    m_map_px = x;
    m_map_py = y;
    m_map_pth = th;
	
    // Draw our new representation
    m_world->SetRectangle( m_map_px, m_map_py, m_map_pth, 
                           m_size_x, m_size_y, this, true);
  }
}


#ifdef INCLUDE_RTK

///////////////////////////////////////////////////////////////////////////
// Process GUI update messages
//
void CLaserBeacon::OnUiUpdate(RtkUiDrawData *data)
{
    CEntity::OnUiUpdate(data);

    data->begin_section("global", "");
    
    if (data->draw_layer("laser_beacon", true))
    {
        double sx = m_size_x;
        double sy = m_size_y;        
        double ox, oy, oth;
        GetGlobalPose(ox, oy, oth);
        data->set_color(RTK_RGB(m_color.red, m_color.green, m_color.blue));
        data->ex_rectangle(ox, oy, oth, sx, sy);

        // Draw in a little arrow showing our orientation
        if (IsMouseReady())
        {
            double dx = 0.20 * cos(oth);
            double dy = 0.20 * sin(oth);
            data->ex_arrow(ox, oy, ox + dx, oy + dy, 0, 0.05);
        }
    }

    data->end_section();
}


///////////////////////////////////////////////////////////////////////////
// Process GUI mouse messages
//
void CLaserBeacon::OnUiMouse(RtkUiMouseData *data)
{
    CEntity::OnUiMouse(data);
}

#endif





