// sonardevice.h - RTV
// $Id: sonardevice.h,v 1.1 2000-12-01 02:57:18 vaughan Exp $

#ifndef SONARDEVICE_HH
#define SONARDEVICE_HH

#include "device.hh"
#include "robot.h"

// forward decl.
//class CWorld;

class CSonarDevice : public CDevice
{
    // Default constructor
    //
    public: CSonarDevice(CRobot* rr, void *buffer, size_t data_len, 
			 size_t command_len, size_t config_len);
    
    // Update the device
    //
    public: virtual bool Update();

    // draw myself on the window
    virtual bool GUIDraw();
    virtual bool GUIUnDraw();
    
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
    
    // private: CWorld* world;

};

#endif
