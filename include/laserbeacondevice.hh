///////////////////////////////////////////////////////////////////////////
//
// File: laserbeacondevice.hh
// Author: Andrew Howard
// Date: 12 Jan 2000
// Desc: Simulates the laser-based beacon detector
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/include/laserbeacondevice.hh,v $
//  $Author: ahoward $
//  $Revision: 1.1.2.4 $
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

#include "playerdevice.hh"

// Forward declarations
//
class CLaserDevice;


class CLaserBeaconDevice : public CPlayerDevice
{
    // Default constructor
    //
    public: CLaserBeaconDevice(CWorld *world, CObject *parent,
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
    private: double m_max_anon_range;
    private: double m_max_id_range;
 
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






