///////////////////////////////////////////////////////////////////////////
//
// File: laserdevice.hh
// Author: Andrew Howard
// Date: 28 Nov 2000
// Desc: Simulates the Player CLaserDevice (the SICK laser)
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/include/laserdevice.hh,v $
//  $Author: vaughan $
//  $Revision: 1.9 $
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

#ifndef LASERDEVICE_HH
#define LASERDEVICE_HH

#include <X11/Xlib.h> // for XPoint structure

#include "playerdevice.hh"


class CLaserDevice : public CPlayerDevice
{
    // Default constructor
    //
    public: CLaserDevice(CRobot* robot, void *buffer, size_t data_len, 
			 size_t command_len, size_t config_len);
    
    // Update the device
    //
    public: virtual bool Update();

    // Check to see if the configuration has changed
    //
    private: bool CheckConfig();

    // Laser timing settings
    //
    private: double m_update_interval;
    private: double m_last_update;
    
    // Maximum range of sample in meters
    //
    private: double m_max_range;
    
    // Laser configuration parameters
    //
    private: int m_min_segment;
    private: int m_max_segment;
    private: int m_samples;
    private: bool m_intensity;

    // Array holding the laser data
    //
    private: UINT16 m_data[512];
    
    // draw myself on the window
    virtual bool GUIDraw();
    virtual bool GUIUnDraw();
    
    // storage for the GUI rendering
     private: XPoint hitPts[ LASERSAMPLES + 1 ];
     private: XPoint oldHitPts[ LASERSAMPLES + 1];
     private: int undrawRequired;
};

#endif




