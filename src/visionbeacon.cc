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
//  $Revision: 1.1.2.1 $
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

#define ENABLE_TRACE 1

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
bool CVisionBeacon::Startup(RtkCfgFile *cfg)
{
    if (!CObject::Startup(cfg))
        return false;

    cfg->BeginSection(m_id);

    m_channel = cfg->ReadInt("channel", 0, "");
    m_radius = cfg->ReadDouble("radius", 0.20, "");
    
    cfg->EndSection();

    #ifdef INCLUDE_RTK
        m_drag_radius = 0.20;
    #endif
    
    return true;
}


///////////////////////////////////////////////////////////////////////////
// Update the laser data
//
void CVisionBeacon::Update()
{
    //TRACE0("updating laser beacon");
    ASSERT(m_world != NULL);

    // Undraw our old representation
    //
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

    pData->BeginSection("global", "VisionBeacons");
    
    if (pData->DrawLayer("", TRUE))
    {
        pData->SetColor(m_channel);
        pData->ExRectangle(m_map_px, m_map_py, m_map_pth, 2 * m_radius, 2 * m_radius);
    }

    pData->EndSection();
}


///////////////////////////////////////////////////////////////////////////
// Process GUI mouse messages
//
void CVisionBeacon::OnUiMouse(RtkUiMouseData *pData)
{
    CObject::OnUiMouse(pData);
}

#endif



