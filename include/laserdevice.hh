///////////////////////////////////////////////////////////////////////////
//
// File: laserdevice.hh
// Author: Andrew Howard
// Date: 28 Nov 2000
// Desc: Simulates the Player CLaserDevice (the SICK laser)
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/include/laserdevice.hh,v $
//  $Author: ahoward $
//  $Revision: 1.3 $
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

#include "device.hh"

class CLaserDevice : public CDevice
{
    // Default constructor
    //
    public: CLaserDevice(void *buffer, size_t data_len, size_t command_len, size_t config_len);
    
    // Update the device
    //
    public: virtual bool Update();

    // Laser timing settings
    //
    private: double m_update_interval;
    private: double m_last_update;

    // Number of laser samples
    //
    private: int m_samples;

    // Maximum range of sample in meters
    //
    private: double m_max_range;
    
    // Array holding the laser data
    //
    private: WORD16 m_data[512];
};

#endif
