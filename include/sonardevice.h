// sonardevice.h - RTV
// $Id: sonardevice.h,v 1.4.2.2 2000-12-06 05:13:42 ahoward Exp $

#ifndef SONARDEVICE_HH
#define SONARDEVICE_HH

#include "playerdevice.hh"
#include "robot.h"
#include <X11/Xlib.h> // for XPoint

#define SONARSAMPLES 16

class CSonarDevice : public CPlayerDevice
{
    // Default constructor
    //
    public: CSonarDevice(CRobot* rr, void *buffer, size_t data_len, 
			 size_t command_len, size_t config_len);
    
    // Update the device
    //
    public: virtual void Update();
    
    // Sonar timing settings
    //
    private: double updateInterval;
    private: double lastUpdate;
    
    // Maximum range of sample in meters
    //
    private: double m_max_range;
    
    // Array holding the sonar data
    //
    private: unsigned short sonar[SONARSAMPLES];
    
#ifndef INCLUDE_RTK

    // draw myself on the window
    virtual bool GUIDraw();
    virtual bool GUIUnDraw();
    
    // store the sonar hit points if we want to render them in the
    // GUI window
    private: XPoint hitPts[ SONARSAMPLES+1 ];
    private: XPoint oldHitPts[ SONARSAMPLES+1 ];
    private: int undrawRequired;
    
    // is GUI drawing enabled for this device?
    //private: bool GUIrender;

#endif    
};

#endif
