///////////////////////////////////////////////////////////////////////////
//
// File: laserbeacon.cc
// Author: Andrew Howard
// Date: 4 Dec 2000
// Desc: Simulates the Player CLaserDevice (the SICK laser)
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/laserbeacon.cc,v $
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

#include <math.h> // RTV - RH-7.0 compiler needs explicit declarations
#include "world.hh"
#include "laserbeacon.hh"


///////////////////////////////////////////////////////////////////////////
// Default constructor
//
CLaserBeacon::CLaserBeacon(CWorld *world, CObject *parent)
        : CObject(world, parent)
{
    // Set the initial map pose
    //
    m_gx = m_gy = m_gth = 0;

    #ifdef INCLUDE_RTK
        m_mouse_radius = (m_parent == m_world ? 0.2 : 0.0);
        m_draggable = (m_parent == m_world);
    #endif
}


///////////////////////////////////////////////////////////////////////////
// Update the laser data
//
void CLaserBeacon::Update()
{
    //TRACE0("updating laser beacon");
    ASSERT(m_world != NULL);

    // Undraw our old representation
    //
    m_world->SetCell(m_gx, m_gy, layer_laser, 0);
    
    // Update our global pose
    //
    GetGlobalPose(m_gx, m_gy, m_gth);
    
    // Draw our new representation
    // Values > 1 are interpreted as intensities
    //
    m_world->SetCell(m_gx, m_gy, layer_laser, 2);
}


#ifdef INCLUDE_RTK

///////////////////////////////////////////////////////////////////////////
// Process GUI update messages
//
void CLaserBeacon::OnUiUpdate(RtkUiDrawData *pData)
{
    CObject::OnUiUpdate(pData);

    pData->BeginSection("global", "laserbeacons");
    
    if (pData->DrawLayer("", TRUE))
    {
        double r = 0.05;
        pData->SetColor(RTK_RGB(0, 0, 255));
        pData->Rectangle(m_gx - r, m_gy - r, m_gx + r, m_gy + r);
    }

    pData->EndSection();
}


///////////////////////////////////////////////////////////////////////////
// Process GUI mouse messages
//
void CLaserBeacon::OnUiMouse(RtkUiMouseData *pData)
{
    CObject::OnUiMouse(pData);
}

#endif



