///////////////////////////////////////////////////////////////////////////
//
// File: LaserBeaconDevice.hh
// Author: Andrew Howard
// Date: 28 Nov 2000
// Desc: Simulates the laser beacons
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/include/laserbeacondevice.hh,v $
//  $Author: ahoward $
//  $Revision: 1.1 $
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

#ifndef LASERBEACONDEVICE_HH
#define LASERBEACONDEVICE_HH

#include "device.hh"

class CLaserBeaconDevice : public CDevice
{
    // Default constructor
    // Requires the position of the beacon relative to the parent object
    //
    public: CLaserBeaconDevice(CRobot* robot, double dx, double dy);
    
    // Update the device
    //
    public: virtual bool Update();

    // Position wrt parent object
    //
    private: double m_dx, m_dy;

    // Position wrt world
    //
    private: double m_px, m_py;
};

#endif
