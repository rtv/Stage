///////////////////////////////////////////////////////////////////////////
//
// File: laserdevice.cc
// Author: Andrew Howard
// Date: 28 Nov 2000
// Desc: Simulates the Player CLaserDevice (the SICK laser)
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/laserdevice.cc,v $
//  $Author: vaughan $
//  $Revision: 1.2 $
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

#include <math.h> // RTV - RH-7.0 compiler needs explicit declarations

#include "world.h"
#include "robot.h"
#include "laserdevice.hh"


///////////////////////////////////////////////////////////////////////////
// Default constructor
//
CLaserDevice::CLaserDevice(void *data_buffer, size_t data_len)
        : CDevice(data_buffer, data_len)
{
    m_update_interval = 0.05;
    m_last_update = 0;
    m_samples = 361;
}


///////////////////////////////////////////////////////////////////////////
// Update the laser data
//
bool CLaserDevice::Update()
{
    //TRACE0("updating laser data");
    ASSERT(m_robot != NULL);
    ASSERT(m_world != NULL);
    
    // Get a pointer to the bitmap
    //
    Nimage *img = m_world->img;

    // Check to see if it is time to update the laser
    //
    if (m_world->timeNow - m_last_update <= m_update_interval)
        return false;
    m_last_update = m_world->timeNow;

    // Make sure the data buffer is big enough
    //
    ASSERT(m_samples <= ARRAYSIZE(m_data));

    float startAngle = m_robot->a + M_PI / 2;
    float increment = -M_PI / m_samples;
    BYTE pixel = 0;

    float xx, yy = m_robot->y;

    int maxRange = (int)(8.0 * m_world->ppm); // 8m times pixels-per-meter
    int rng = 0;

    // Generate a complete set of samples
    //
    for(int s = 0; s < m_samples; s++)
    {
        float dist, dx, dy, angle;
        
        angle = startAngle + ( s * increment );
        
        dx = cos( angle );
        dy = sin( angle);
        
        xx = m_robot->x;
        yy = m_robot->y;
        rng = 0;
        
        pixel = img->get_pixel( (int)xx, (int)yy );
        
        // no need for bounds checking - get_pixel() does that internally
        //
        while( rng < maxRange && ( pixel == 0 || pixel == m_robot->color ) )
        {
            xx+=dx;
            yy+=dy;
            rng++;
            
            pixel = img->get_pixel( (int)xx, (int)yy );
        }
        
        // set laser value, scaled to current ppm
        // and converted to mm
        //
        float v = 1000 * rng / m_world->ppm;
        
        // Set the range
        // Swap the bytes while where at it
        //
        m_data[s] = htons((WORD16) v);
    }
    
    // Copy the laser data to the data buffer
    //
    PutData(m_data, m_samples * sizeof(WORD16));

    //~redrawLaser = true;
    return true;
}
