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
    public: CLaserBeaconDevice(CWorld *world, CEntity *parent,
                               CPlayerServer *server, CLaserDevice *laser);

    // Load the object from an argument list
    //
    public: virtual bool Load(int argc, char **argv);

    // Save the object to an argument list
    //
    public: virtual bool Save(int &argc, char **argv);
    
    // Update the device
    //
    public: virtual void Update();

    // Pointer to laser used as souce of data
    //
    private: CLaserDevice *m_laser;

    // Time of last update
    //
    private: uint32_t m_time_sec, m_time_usec;

    // Detection parameters
    //
    private: double m_max_anon_range;
    private: double m_max_id_range;
    
#ifdef INCLUDE_RTK
    
    // Process GUI update messages
    //
    public: virtual void OnUiUpdate(RtkUiDrawData *pData);

#endif

    private:  ExportLaserBeaconDetectorData expBeacon; 
};

#endif






