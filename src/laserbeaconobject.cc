///////////////////////////////////////////////////////////////////////////
//
// File: laserbeaconobject.cc
// Author: Andrew Howard
// Date: 4 Dec 2000
// Desc: Simulates the Player CLaserDevice (the SICK laser)
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/laserbeaconobject.cc,v $
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

#define ENABLE_TRACE 1

#include <math.h> // RTV - RH-7.0 compiler needs explicit declarations
#include "world.hh"
#include "robot.h"
#include "laserbeaconobject.hh"


///////////////////////////////////////////////////////////////////////////
// Default constructor
//
CLaserBeaconObject::CLaserBeaconObject(CWorld *world, CObject *parent)
        : CObject(world, parent)
{
    // Position relative to parent object
    //
    SetPose(0, 0, 0);

    // Set the initial map pose
    //
    m_gx = m_gy = m_gth = 0;
}


///////////////////////////////////////////////////////////////////////////
// Update the laser data
//
void CLaserBeaconObject::Update()
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
    //
    m_world->SetCell(m_gx, m_gy, layer_laser, 1);
}


#ifdef INCLUDE_RTK

///////////////////////////////////////////////////////////////////////////
// Process GUI update messages
//
void CLaserBeaconObject::OnUiUpdate(RtkUiDrawData *pData)
{
    CObject::OnUiUpdate(pData);

    pData->BeginSection("global", "laserbeacons");
    
    if (pData->DrawLayer("", TRUE))
    {
        double r = 0.05;
        pData->SetColor(RGB(0, 0, 255));
        pData->Rectangle(m_gx - r, m_gy - r, m_gx + r, m_gy + r);
    }

    pData->EndSection();
}


///////////////////////////////////////////////////////////////////////////
// Process GUI mouse messages
//
void CLaserBeaconObject::OnUiMouse(RtkUiMouseData *pData)
{
    CObject::OnUiMouse(pData);
}

#endif



