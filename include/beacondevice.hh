///////////////////////////////////////////////////////////////////////////
//
// File: beacondevice.hh
// Author: Andrew Howard
// Date: 12 Jan 2000
// Desc: Simulates the laser-based beacon detector
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/include/beacondevice.hh,v $
//  $Author: ahoward $
//  $Revision: 1.1.2.2 $
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

#ifndef BEACONDEVICE_HH
#define BEACONDEVICE_HH

#include "playerdevice.hh"

// Forward declarations
//
class CLaserDevice;


class CBeaconDevice : public CPlayerDevice
{
    // Default constructor
    //
    public: CBeaconDevice(CWorld *world, CObject *parent,
                          CPlayerRobot *robot, CLaserDevice *laser);
    
    // Update the device
    //
    public: virtual void Update();

    // Pointer to laser used as souce of data
    //
    private: CLaserDevice *m_laser;
    
    // Beacon detector timing settings
    //
    private: double m_update_interval;
    private: double m_last_update;

    // Detection parameters
    //
    private: double m_max_range;
 
#ifdef INCLUDE_RTK
    
    // Process GUI update messages
    //
    public: virtual void OnUiUpdate(RtkUiDrawData *pData);

    // List of points at which beacon was detected
    //
    private: int m_hit_count;
    private: double m_hit[16][2];
    
#endif    
};

#endif






