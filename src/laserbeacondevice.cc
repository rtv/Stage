///////////////////////////////////////////////////////////////////////////
//
// File: laserbeacondevice.cc
// Author: Andrew Howard
// Date: 28 Nov 2000
// Desc: Simulates the Player CLaserDevice (the SICK laser)
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/laserbeacondevice.cc,v $
//  $Author: ahoward $
//  $Revision: 1.2.2.1 $
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
#include "laserbeacondevice.hh"


///////////////////////////////////////////////////////////////////////////
// Default constructor
//
CLaserBeaconDevice::CLaserBeaconDevice(CRobot *robot, double dx, double dy)
        : CDevice(robot)
{
    // Position relative to parent object
    //
    m_dx = dx;
    m_dy = dy;

    // Position relative to world
    //
    m_px = -1;
    m_py = -1;
}


///////////////////////////////////////////////////////////////////////////
// Update the laser data
//
void CLaserBeaconDevice::Update()
{
    //TRACE0("updating laser data");
    ASSERT(m_robot != NULL);
    ASSERT(m_world != NULL);
    
    // Get a pointer to the bitmap in which laser beacon data is stored
    //
    Nimage *img = m_world->m_laser_img;

    // Get robot pose
    //
    double robotx = m_robot->x / m_world->ppm;
    double roboty = m_robot->y / m_world->ppm;

    // Compute position in world coords
    //
    double px = robotx + m_dx * cos(m_robot->a) - m_dy * sin(m_robot->a);
    double py = roboty + m_dx * sin(m_robot->a) + m_dy * cos(m_robot->a);

    // Compute position in world image coords
    //
    int ix = (int) (m_px * m_world->ppm);
    int iy = (int) (m_py * m_world->ppm);

    // Undraw our old representation
    //
    img->set_pixel(ix, iy, 0);

    // Compute position in world image coords
    //
    ix = (int) (px * m_world->ppm);
    iy = (int) (py * m_world->ppm);

    // Redraw our new representation
    //
    img->set_pixel(ix, iy, 1);

    // Update our position in world coords
    //
    m_px = px;
    m_py = py;
}






