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
//  $Revision: 1.9.2.15 $
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

class CLaserDevice : public CPlayerDevice
{
    // Default constructor
    //
    public: CLaserDevice(CWorld *world, CEntity *parent, CPlayerServer* server);

    // Update the device
    //
    public: virtual void Update();

    // Check to see if the configuration has changed
    //
    private: bool CheckConfig();

    // Generate scan data
    //
    private: bool GenerateScanData(player_laser_data_t *data);

    // Draw ourselves into the world rep
    //
    private: void Map(bool render);
    
    // Laser timing settings
    //
    private: double m_update_rate;
    private: double m_last_update;
    
    // Maximum range of sample in meters
    //
    private: double m_max_range;
    
    // Laser configuration parameters
    //
    private: double m_scan_res;
    private: double m_scan_min;
    private: double m_scan_max;
    private: int m_scan_count;
    private: bool m_intensity;

    // Size of laser in laser rep
    //
    private: double m_map_dx, m_map_dy;
    
    // The laser's last mapped pose
    //
    private: double m_map_px, m_map_py, m_map_pth;

#ifdef INCLUDE_XGUI
    // draw/undraw in X gui
    //
    public: virtual ExportData* ImportExportData( ImportData* imp );

    // export the laser hit points
   private: ExportLaserData expLaser;
#endif

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






