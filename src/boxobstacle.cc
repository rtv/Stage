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
//  $Revision: 1.1.2.2 $
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
CBoxObstacle::CBoxObstacle(CWorld *world, CObject *parent)
        : CObject(world, parent)
{
    m_size_x = 0;
    m_size_y = 0;
        
    // Set the initial map pose
    //
    m_map_px = m_map_py = m_map_pth = 0;
}


///////////////////////////////////////////////////////////////////////////
// Initialise object
//
bool CBoxObstacle::Startup(RtkCfgFile *cfg)
{
    if (!CObject::Startup(cfg))
        return false;

    cfg->BeginSection(m_id);

    m_size_x = cfg->ReadDouble("size_x", 1.00, "");
    m_size_y = cfg->ReadDouble("size_y", 1.00, "");
    
    cfg->EndSection();

    #ifdef INCLUDE_RTK
        m_mouse_radius = max(m_size_x, m_size_y) * 0.6;
        m_draggable = (m_parent == m_world);
    #endif
    
    return true;
}


///////////////////////////////////////////////////////////////////////////
// Update the laser data
//
void CBoxObstacle::Update()
{
    //RTK_TRACE0("updating laser beacon");
    ASSERT(m_world != NULL);

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


#ifdef INCLUDE_RTK

///////////////////////////////////////////////////////////////////////////
// Process GUI update messages
//
void CBoxObstacle::OnUiUpdate(RtkUiDrawData *pData)
{
    CObject::OnUiUpdate(pData);

    pData->BeginSection("global", "BoxObstacles");
    
    if (pData->DrawLayer("", TRUE))
    {
        pData->SetColor(RTK_RGB(128, 128, 255));
        pData->ExRectangle(m_map_px, m_map_py, m_map_pth, m_size_x, m_size_y);
    }

    pData->EndSection();
}


///////////////////////////////////////////////////////////////////////////
// Process GUI mouse messages
//
void CBoxObstacle::OnUiMouse(RtkUiMouseData *pData)
{
    CObject::OnUiMouse(pData);
}

#endif



