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
//  $Revision: 1.16.2.5 $

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

#include "playerdevice.hh"
#include "laserbeacon.hh"

#include <slist.h> // STL

typedef std::slist< int > LaserBeaconList; 

class CLaserDevice : public CEntity
{
    // Default constructor
    //
    public: CLaserDevice(CWorld *world, CEntity *parent );

    // Load the object from an argument list
    //
    public: virtual bool Load(int argc, char **argv);

    // Save the object to an argument list
    //
    public: virtual bool Save(int &argc, char **argv);

    // Update the device
    //
    public: virtual void Update( double sim_time );

    // Check to see if the configuration has changed
    //
    private: bool CheckConfig();

    // Generate scan data
    //
    private: bool GenerateScanData(player_laser_data_t *data);

    public: LaserBeaconList m_visible_beacons;

    // Draw ourselves into the world rep
    //
  //private: virtual void Map(bool render);
    
    // Laser timing settings
    //
  //private: double m_update_rate;
    
    // Maximum range of sample in meters
    //
    private: double m_max_range;

    // Set this flag to make the laser transparent to other lasers
    //
    private: bool m_transparent;
    
    // Laser configuration parameters
    //
    private: double m_scan_res;
    private: double m_scan_min;
    private: double m_scan_max;
    private: int m_scan_count;
    private: bool m_intensity;

    // storage for exporting the laser hit points
  //private: ExportLaserData expLaser;

#ifdef INCLUDE_RTK
    
    // Process GUI update messages
    //
    public: virtual void OnUiUpdate(RtkUiDrawData *data);

    // Process GUI mouse messages
    //
    public: virtual void OnUiMouse(RtkUiMouseData *data);

    // Draw the laser turret
    //
    private: void DrawTurret(RtkUiDrawData *data);

    // Draw the laser scan
    //
    private: void DrawScan(RtkUiDrawData *data);

    // Laser scan outline
    //
    private: int m_hit_count;
    private: double m_hit[512][2];
    
#endif    
};

#endif















