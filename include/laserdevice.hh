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
//  $Revision: 1.9.2.6 $
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


#define LASERSAMPLES 361


class CLaserDevice : public CPlayerDevice
{
    // Default constructor
    //
    public: CLaserDevice(CWorld *world, CObject *parent, CPlayerRobot* robot);

    // Initialise the device
    //
    public: virtual bool Startup(RtkCfgFile *cfg);
    
    // Update the device
    //
    public: virtual void Update();

    // Check to see if the configuration has changed
    //
    private: bool CheckConfig();

    // Generate scan data
    //
    private: bool UpdateScanData();

    // Draw ourselves into the world rep
    //
    private: void Map(bool render);
    
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
    private: int m_sample_density;
    private: bool m_intensity;

    // Array holding the laser data
    //
    private: uint16_t m_data[512];

    // Size of laser in laser rep
    //
    private: double m_map_dx, m_map_dy;
    
    // The laser's last mapped pose
    //
    private: double m_map_px, m_map_py, m_map_pth;

#ifndef INCLUDE_RTK
    
    // draw myself on the window
    public: virtual bool GUIDraw();
    public: virtual bool GUIUnDraw();
    
    // storage for the GUI rendering
    private: XPoint hitPts[ LASERSAMPLES + 1 ];
    private: XPoint oldHitPts[ LASERSAMPLES + 1];
    private: int undrawRequired;

#else
    
    // Process GUI update messages
    //
    public: virtual void OnUiUpdate(RtkUiDrawData *pData);

    // Process GUI mouse messages
    //
    public: virtual void OnUiMouse(RtkUiMouseData *pData);

    // Draw the laser turret
    //
    private: void DrawTurret(RtkUiDrawData *pData);

    // Draw the laser scan
    //
    private: void DrawScan(RtkUiDrawData *pData);

    // Laser scan outline
    //
    private: int m_hit_count;
    private: double m_hit[LASERSAMPLES][2];
    
#endif    
};

#endif






