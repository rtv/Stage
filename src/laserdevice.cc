///////////////////////////////////////////////////////////////////////////
//
// File: laserdevice.cc
// Author: Andrew Howard
// Date: 28 Nov 2000
// Desc: Simulates the Player CLaserDevice (the SICK laser)
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/laserdevice.cc,v $
//  $Author: gerkey $
//  $Revision: 1.15 $
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

#define ENABLE_TRACE 0

#include <math.h> // RTV - RH-7.0 compiler needs explicit declarations
#include "world.h"
#include "robot.h"
#include "laserdevice.hh"


///////////////////////////////////////////////////////////////////////////
// Default constructor
//
CLaserDevice::CLaserDevice(CRobot* rr, void *buffer, size_t data_len, 
			   size_t command_len, size_t config_len)
        : CPlayerDevice(rr, buffer, data_len, command_len, config_len)
{
    m_update_interval = 0.2; // RTV - speed to match the real laser
    m_last_update = 0;

    m_min_segment = 0;
    m_max_segment = 361;
    m_intensity = false;
  
    m_samples = 361;
    m_max_range = 8.0; // meters - this could be dynamic one day
                     // but this matches the default laser setup.

    sampleIncrement = 3; // specify the degree of under-sampling
                     // the ray tracing will only compute every sampleIncrement
                     // sample for speed-up versus lost resolution. 
                     // must be > 0. I reckon 3 is a good default - RTV
                     // if you need the best possible quality, set this to 1. 

    undrawRequired = false;
}


///////////////////////////////////////////////////////////////////////////
// Update the laser data
//
bool CLaserDevice::Update()
{
    //TRACE0("updating laser data");
    ASSERT(m_robot != NULL);
    ASSERT(m_world != NULL);

    // Dont update anything if we are not subscribed
    //
    if (!IsSubscribed())
        return true;
    
    // Get pointers to the various bitmaps
    //
    Nimage *img = m_world->img;
    Nimage *laser_img = m_world->m_laser_img;

    ASSERT(img != NULL);
    ASSERT(laser_img != NULL);

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
    uint8_t myColor = m_robot->color;
    double ppm = m_world->ppm; // pixels per metre

    double robotx = m_robot->x;
    double roboty = m_robot->y;
 
    double dist, dx, dy, angle, pixelx, pixely;
    
    uint8_t occupied, intensity;

    // Generate a complete set of samples
    //
    for(int s = 0; s < m_samples; s+=sampleIncrement)
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

        // Scan through the entire allowed range
        //
        for (rng = 0; rng < maxRange; rng++)
        {
            // Look for laser beacons
            //
            if (m_intensity)
            {
                intensity = laser_img->get_pixel((int) pixelx, (int) pixely);
                if (intensity != 0)
                    break;
            }

            // Look for occupied cells
            // we have to check 2 pixels to prevent sliding through
            // 1-pixel gaps in jaggies
            //
            occupied = img->get_pixel( (int)pixelx, (int)pixely );
            if (occupied != 0 && occupied != myColor)
                break;
            occupied = img->get_pixel( (int)pixelx + 1, (int)pixely );
            if (occupied != 0 && occupied != myColor)
                break;

            // Compute the next pixel to inspect
            //
            pixelx += dx;
            pixely += dy;
        }
                
        // set laser value, scaled to current ppm
        // and converted to mm
        //
        uint16_t v = (uint16_t) (1000.0 * rng / ppm);

        // Add in the intensity values in the top 3 bits
        //
        if (m_intensity)
            v = v | (((uint16_t) intensity) << 13);
        
        // Set the range
        // Swap the bytes while we're at it
        //
	if( sampleIncrement == 1 ) // we're not undersampling 
	  {
	    m_data[s] = htons( v );
	    
	    // store the hit points if we need to draw them on the GUI
	    if( m_robot->showDeviceDetail )
	      {
		hitPts[s].x = (int)pixelx;
		hitPts[s].y = (int)pixely;
	      }
	  }
	else // we're undersampling so fill in intervening results too
 	  { 
	    unsigned short u = htons(v);
	    // fill in all the elements we've under-sampled
	    for( int d=0; (d < sampleIncrement) && (s+d < m_samples); d++ )
	      {
		m_data[s+d] = u;
		
		// store the hit points if we need to draw them on the GUI
		if( m_robot->showDeviceDetail )
		  {
		    hitPts[s+d].x = (int)pixelx;
		    hitPts[s+d].y = (int)pixely;
		  }
	      }
	  }
    }
    
    // Copy the laser data to the data buffer
    //
    PutData(m_data, m_samples * sizeof(uint16_t));

    return true;
}


///////////////////////////////////////////////////////////////////////////
// Check to see if the configuration has changed
//
bool CLaserDevice::CheckConfig()
{
    uint8_t config[5];

    if (GetConfig(config, sizeof(config)) == 0)
        return false;  
    
    m_min_segment = ntohs(MAKEUINT16(config[0], config[1]));
    m_max_segment = ntohs(MAKEUINT16(config[2], config[3]));
    m_intensity = (bool) config[4];
    PLAYER_MSG3("new scan range [%d %d], intensity [%d]",
         (int) m_min_segment, (int) m_max_segment, (int) m_intensity);

    // *** HACK -- change the update rate based on the scan size
    // This is a guess only
    //
    m_update_interval = 0.2 * (m_max_segment - m_min_segment + 1) / 361;
        
    return true;
}

bool CLaserDevice::GUIDraw()
{ 
  // dump out if noone is subscribed or the device
  if( !IsSubscribed() || !m_robot->showDeviceDetail ) return true;

  // replicate the first point at the end in order to draw a closed polygon
  hitPts[m_samples].x = hitPts[0].x;
  hitPts[m_samples].y = hitPts[0].y;

  m_world->win->SetForeground( m_world->win->RobotDrawColor( m_robot) );
  m_world->win->DrawLines( hitPts, m_samples+1 );
    
  memcpy( oldHitPts, hitPts, sizeof( XPoint ) * (m_samples+1) );
  
  undrawRequired = true;

  return true; 
};  

bool CLaserDevice::GUIUnDraw()
{ 
  if( undrawRequired )
  {
    m_world->win->SetForeground( m_world->win->RobotDrawColor( m_robot) );
    m_world->win->DrawLines( oldHitPts, m_samples+1 );

    undrawRequired = false;
  }   
  return true; 
};





