///////////////////////////////////////////////////////////////////////////
//
// File: laserdevice.cc
// Author: Andrew Howard
// Date: 28 Nov 2000
// Desc: Simulates the Player CLaserDevice (the SICK laser)
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/laserdevice.cc,v $
//  $Author: ahoward $
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

#include <math.h> // RTV - RH-7.0 compiler needs explicit declarations

#include "world.h"
#include "robot.h"
#include "laserdevice.hh"


///////////////////////////////////////////////////////////////////////////
// Default constructor
//
CLaserDevice::CLaserDevice(void *buffer, size_t data_len, size_t command_len, size_t config_len)
        : CDevice(buffer, data_len, command_len, config_len)
{
    m_update_interval = 0.2; // RTV - speed to match the real laser
    m_last_update = 0;

    m_min_segment = 0;
    m_max_segment = 361;
    m_intensity = false;
  
    m_samples = 361;
    m_max_range = 8.0; // meters - this could be dynamic one day
                     // but this matches the default laser setup.
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

    // Check to see if the configuration has changed
    //
    CheckConfig();

    // Make sure the data buffer is big enough
    //
    ASSERT(m_samples <= ARRAYSIZE(m_data));

    // calculate the angle of the first laser beam
    double startAngle = m_robot->a + M_PI_2;
    double increment = -M_PI / m_samples; // and the difference between beams
    
    int maxRange = (int)( m_max_range * m_world->ppm); //scaled and quantized
    int rng = 0;

    //look up a couple of indrect variables to avoid doing it in the loop
    BYTE myColor = m_robot->color;
    double ppm = m_world->ppm; // pixels per metre

    double robotx = m_robot->x;
    double roboty = m_robot->y;

    float v = 0; // computed range value
 
    double dist, dx, dy, angle, pixelx, pixely;
    
    BYTE pixel, sidePixel;

    // Generate a complete set of samples
    //
    for(int s = 0; s < m_samples; s++)
    {
        // Allow for partial scans
        //
        if (s < m_min_segment || s > m_max_segment)
        {
            m_data[s] = 0;
            continue;
        }
        
        angle = startAngle + ( s * increment );
        
        dx = cos( angle );
        dy = sin( angle);

        // initialize variables that'll be interated over
        pixelx = robotx;
        pixely = roboty;

        rng = pixel = sidePixel = 0;

        // no need for bounds checking - get_pixel() does that internally
        //
        while( rng < maxRange 
	       && ( pixel == 0 || pixel == myColor ) 
	       && ( sidePixel == 0 || sidePixel == myColor ) )
        {
	    // we have to check 2 pixels to prevent sliding through
	    // 1-pixel gaps in jaggies
            pixel = img->get_pixel( (int)pixelx, (int)pixely );
	    sidePixel = img->get_pixel( (int)pixelx + 1, (int)pixely );

            pixelx += dx;
            pixely += dy;
            rng++;            
        }
        
        // set laser value, scaled to current ppm
        // and converted to mm
        //
        v = 1000.0 * rng / ppm;
        
        // Set the range
        // Swap the bytes while we're at it
        //
        m_data[s] = htons((UINT16) v);
    }
    
    // Copy the laser data to the data buffer
    //
    PutData(m_data, m_samples * sizeof(UINT16));

    //~redrawLaser = true;
    return true;
}


///////////////////////////////////////////////////////////////////////////
// Check to see if the configuration has changed
//
bool CLaserDevice::CheckConfig()
{
    BYTE config[5];

    if (GetConfig(config, sizeof(config)) == 0)
        return false;  
    
    m_min_segment = ntohs(MAKEUINT16(config[0], config[1]));
    m_max_segment = ntohs(MAKEUINT16(config[2], config[3]));
    m_intensity = (bool) config[4];
    MSG3("new scan range [%d %d], intensity [%d]",
         (int) m_min_segment, (int) m_max_segment, (int) m_intensity);

    // *** HACK -- change the update rate based on the scan size
    // This is a guess only
    //
    m_update_interval = 0.2 * (m_max_segment - m_min_segment + 1) / 361;
        
    return true;
}







