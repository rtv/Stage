///////////////////////////////////////////////////////////////////////////
//
// File: visionbeacon.cc
// Author: Andrew Howard
// Date: 6 Dec 2000
// Desc: Simulates the ACTS beacons
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/visionbeacon.cc,v $
//  $Author: vaughan $
//  $Revision: 1.6 $
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
  m_radius = 0.1; // our beacons are 10cm radius
    
    m_stage_type = VisionBeaconType;

    m_channel = 0; // visible by default on ACTS ch.0
    m_radius = 0;

    m_render_obstacle = true;
    m_render_laser = true;
    
    // Set the initial map pose
    //
    m_map_px = m_map_py = m_map_pth = 0;

    m_size_x = m_radius;
    m_size_y = m_radius;
}


///////////////////////////////////////////////////////////////////////////
// Load the object from an argument list
//
bool CVisionBeacon::Load(int argc, char **argv)
{
    if (!CEntity::Load(argc, argv))
        return false;

    //printf( "beacon %p channel: %d\n", this, m_channel );

    for (int i = 0; i < argc;)
    {
        // Extract radius
        //
        if (strcmp(argv[i], "radius") == 0 && i + 1 < argc)
        {
            m_radius = atof(argv[i + 1]);
            i += 2;

	    m_size_x = m_size_y = m_radius;
        }

        // See which layers to render
        //
        else if (strcmp(argv[i], "norender") == 0 && i + 1 < argc)
        {
            if (strcmp(argv[i + 1], "obstacle") == 0)
                m_render_obstacle = false;
            else if (strcmp(argv[i + 1], "laser") == 0)
                m_render_laser = false;
            else
                PLAYER_MSG2("unrecognized token [%s %s]", argv[i], argv[i + 1]);
            i += 2;
        }

	// RTV - this'd report error before any subclass had a chance
        // Syntax error
        else
        {
	  PLAYER_MSG1("unrecognized token [%s]", argv[i]);
	  i += 1;
        }
    }

#ifdef INCLUDE_RTK
    m_mouse_radius = (m_parent_object == NULL ? 1.414 * m_radius : 0.0);
    m_draggable = (m_parent_object == NULL);
#endif
        
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

    // Save render settings
    //
    if (!m_render_obstacle)
    {
        argv[argc++] = strdup("norender");
        argv[argc++] = strdup("obstacle");
    }
    if (!m_render_laser)
    {
        argv[argc++] = strdup("norender");
        argv[argc++] = strdup("laser");
    }
    return true;
}


///////////////////////////////////////////////////////////////////////////
// Update the laser data
//
void CVisionBeacon::Update( double sim_time )
{
    ASSERT(m_world != NULL);

    // Dont update anything if we are not subscribed
    //
    //if( Subscribed() < 1 )
    //return;
    
    ASSERT(m_world != NULL);
    
    // See if its time to recalculate vision
    //
    if( sim_time - m_last_update < m_interval )
        return;

    m_last_update = sim_time;


    // Undraw our old representation
    //
    if (m_render_obstacle)
    {
        m_world->SetRectangle(m_map_px, m_map_py, m_map_pth,
                              2 * m_radius, 2 * m_radius, layer_obstacle, 0);
    }
    if (m_render_laser)
    {
        m_world->SetRectangle(m_map_px, m_map_py, m_map_pth,
                       2 * m_radius, 2 * m_radius, layer_laser, 0);
    }
    m_world->SetRectangle(m_map_px, m_map_py, m_map_pth,
                       2 * m_radius, 2 * m_radius, layer_vision, 0);
    
    // Update our global pose
    //
    GetGlobalPose(m_map_px, m_map_py, m_map_pth);
    
    // Draw our new representation
    //
    if (m_render_obstacle)
    {
        m_world->SetRectangle(m_map_px, m_map_py, m_map_pth,
                              2 * m_radius, 2 * m_radius, layer_obstacle, 1);
    }
    if (m_render_laser)
    {
        m_world->SetRectangle(m_map_px, m_map_py, m_map_pth,
                              2 * m_radius, 2 * m_radius, layer_laser, 1);
    }
    m_world->SetRectangle(m_map_px, m_map_py, m_map_pth,
                          2 * m_radius, 2 * m_radius, layer_vision, m_channel+1);
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
        data->set_color(m_color);
        data->ex_rectangle(m_map_px, m_map_py, m_map_pth, 2 * m_radius, 2 * m_radius);
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



