///////////////////////////////////////////////////////////////////////////
//
// File: laserbeacon.cc
// Author: Andrew Howard
// Date: 4 Dec 2000
// Desc: Simulates the Player CLaserDevice (the SICK laser)
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/laserbeacon.cc,v $
//  $Author: vaughan $
//  $Revision: 1.13 $
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

    m_player_port = 0; // not a player device
    m_player_index = 0;
    m_player_type = 0;

    m_stage_type = LaserBeaconType;
  
    m_beacon_id = 0;
    m_index = -1;

  m_beacon_id = 0;
  m_index = -1;
  
  m_interval = 0.2; // 5Hz update
  
  laser_return = LaserBright;
  
  // Set this flag to make the beacon transparent to lasesr
  //
  m_transparent = false;
  
  m_size_x = 0.05; // very thin!   
  m_size_y = 0.3;     
}


///////////////////////////////////////////////////////////////////////////
// Load the object from an argument list
//
bool CLaserBeacon::Load(int argc, char **argv)
{
    if (!CEntity::Load(argc, argv))
        return false;

    for (int i = 0; i < argc;)
    {
        // Extract id
        //
        if (strcmp(argv[i], "id") == 0 && i + 1 < argc)
        {
            if (m_name[0] == 0)
                strcpy(m_name, argv[i + 1]);
            m_beacon_id = atoi(argv[i + 1]);
            i += 2;
        }
        else if (strcmp(argv[i], "transparent") == 0)
        {
	  laser_return = 0;
	  i += 1;
        }
        else
            i++;
    }

#ifdef INCLUDE_RTK
    m_mouse_radius = (m_parent_object == NULL ? 0.2 : 0.0);
    m_draggable = (m_parent_object == NULL);
#endif
        
    return true;
}


///////////////////////////////////////////////////////////////////////////
// Save the object
//
bool CLaserBeacon::Save(int &argc, char **argv)
{
    if (!CEntity::Save(argc, argv))
        return false;

    // Save id
    //
    char id[32];
    snprintf(id, sizeof(id), "%d", (int) m_beacon_id);
    argv[argc++] = strdup("id");
    argv[argc++] = strdup(id);

    if (m_transparent)
        argv[argc++] = strdup("transparent");
    
    return true;
}


///////////////////////////////////////////////////////////////////////////
// Update the laser data
//
void CLaserBeacon::Update( double sim_time )
{
    ASSERT(m_world != NULL);

    // See if its time to update beacons
    //

    //if( sim_time - m_last_update < m_interval )
    //return;
    
    double x, y, th;
    GetGlobalPose( x,y,th );
    
    // if we've moved 
    if( (m_map_px != x) || (m_map_py != y) || (m_map_pth != th ) )
      {
	m_last_update = sim_time;
	
	// Undraw our old representation
	//
	if (!m_transparent)
	  {
	    m_world->matrix->mode = mode_unset;
	    m_world->SetRectangle( m_map_px, m_map_py, m_map_pth, 
				   m_size_x, m_size_y, this );
	  }
	
	m_map_px = x;
	m_map_py = y;
	m_map_pth = th;
	
	// Draw our new representation
	//
	if (!m_transparent)
	  {
	    m_world->matrix->mode = mode_set;
	    m_world->SetRectangle( m_map_px, m_map_py, m_map_pth, 
				   m_size_x, m_size_y, this );
	  }
	// CHOP THIS!
	//m_world->SetLaserBeacon(m_index, m_map_px, m_map_py, m_map_pth);
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
        data->set_color(m_color); 
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





