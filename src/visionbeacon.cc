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
//  $Revision: 1.1.2.7 $
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
#include "visionbeacon.hh"


///////////////////////////////////////////////////////////////////////////
// Default constructor
//
CVisionBeacon::CVisionBeacon(CWorld *world, CObject *parent)
        : CObject(world, parent)
{
    m_channel = 0;
    m_radius = 0;
    
    // Set the initial map pose
    //
    m_map_px = m_map_py = m_map_pth = 0;
}


///////////////////////////////////////////////////////////////////////////
// Initialise object
//
bool CVisionBeacon::StartUp()
{
    /*
    if (!CObject::Startup(cfg))
        return false;

    cfg->begin_section(m_id);

    m_channel = cfg->ReadInt("channel", 0, "");
    m_radius = cfg->ReadDouble("radius", 0.20, "");
    
    cfg->end_section();

    // *** HACK
    // Assign arbitrary colors to channels
    //
    int c = 255 * m_channel / 32;
    m_color = RTK_RGB(c, 255 - c, 255 - c);

    #ifdef INCLUDE_RTK
        m_mouse_radius = m_radius * 1.5;
        m_draggable = (m_parent == m_world);
    #endif
    */
    return true;
}


///////////////////////////////////////////////////////////////////////////
// Update the laser data
//
void CVisionBeacon::Update()
{
    //RTK_TRACE0("updating laser beacon");
    ASSERT(m_world != NULL);

    // Undraw our old representation
    //
    m_world->SetRectangle(m_map_px, m_map_py, m_map_pth,
                       2 * m_radius, 2 * m_radius, layer_obstacle, 0);
    m_world->SetRectangle(m_map_px, m_map_py, m_map_pth,
                       2 * m_radius, 2 * m_radius, layer_laser, 0);
    m_world->SetRectangle(m_map_px, m_map_py, m_map_pth,
                       2 * m_radius, 2 * m_radius, layer_vision, 0);
    
    // Update our global pose
    //
    GetGlobalPose(m_map_px, m_map_py, m_map_pth);
    
    // Draw our new representation
    //
    m_world->SetRectangle(m_map_px, m_map_py, m_map_pth,
                          2 * m_radius, 2 * m_radius, layer_obstacle, 1);
    m_world->SetRectangle(m_map_px, m_map_py, m_map_pth,
                          2 * m_radius, 2 * m_radius, layer_laser, 1);
    m_world->SetRectangle(m_map_px, m_map_py, m_map_pth,
                          2 * m_radius, 2 * m_radius, layer_vision, m_channel);
}


#ifdef INCLUDE_RTK

///////////////////////////////////////////////////////////////////////////
// Process GUI update messages
//
void CVisionBeacon::OnUiUpdate(RtkUiDrawData *pData)
{
    CObject::OnUiUpdate(pData);

    pData->begin_section("global", "VisionBeacons");
    
    if (pData->draw_layer("", TRUE))
    {
        // *** HACK - color?
        pData->set_color(RTK_RGB(255, 0, 0));
        pData->ex_rectangle(m_map_px, m_map_py, m_map_pth, 2 * m_radius, 2 * m_radius);
    }

    pData->end_section();
}


///////////////////////////////////////////////////////////////////////////
// Process GUI mouse messages
//
void CVisionBeacon::OnUiMouse(RtkUiMouseData *pData)
{
    CObject::OnUiMouse(pData);
}

#endif



