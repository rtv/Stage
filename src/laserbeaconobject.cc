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
#include "world.h"
#include "robot.h"
#include "laserbeaconobject.hh"


///////////////////////////////////////////////////////////////////////////
// Default constructor
//
CLaserBeaconObject::CLaserBeaconObject(CWorld *world, CObject *parent, double dx, double dy)
        : CObject(world, parent)
{
    // Position relative to parent object
    //
    SetPose(dx, dy, 0);

    m_gx = m_gy = m_gth = 0;
}


///////////////////////////////////////////////////////////////////////////
// Update the laser data
//
void CLaserBeaconObject::Update()
{
    //TRACE0("updating laser data");
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

    /* *** remove
    // Get a pointer to the bitmap in which laser beacon data is stored
    //
    Nimage *img = m_world->m_laser_img;

    // Compute current position in world image coords
    //
    int ix = (int) (m_gx * m_world->ppm);
    int iy = (int) (m_gy * m_world->ppm);

    // Undraw our old representation
    //
    img->set_pixel(ix, iy, 0);

    // Compute the new position in world coords
    //
    double px = m_lx;
    double py = m_ly;
    double pth = m_lth;
    LocalToGlobal(px, py, pth);
    
    // Compute position in world image coords
    //
    ix = (int) (px * m_world->ppm);
    iy = (int) (py * m_world->ppm);

    // Redraw our new representation
    //
    img->set_pixel(ix, iy, 1);

    // Update our position in world coords
    //
    m_gx = px;
    m_gy = py;
    m_gth = pth;
    */
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



