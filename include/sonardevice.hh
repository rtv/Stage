///////////////////////////////////////////////////////////////////////////
//
// File: sonardevice.hh
// Author: Richard Vaughan, Andrew Howard
// Date: 30 Nov 2000
// Desc: Simulates the pioneer sonars
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/include/sonardevice.hh,v $
//  $Author: ahoward $
//  $Revision: 1.1.2.1 $
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

#ifndef SONARDEVICE_HH
#define SONARDEVICE_HH

#include "playerdevice.hh"

#define SONARSAMPLES 16

class CSonarDevice : public CPlayerDevice
{
    // Default constructor
    //
    public: CSonarDevice(CWorld *world, CObject *parent, CPlayerRobot* robot);
    
    // Update the device
    //
    public: virtual void Update();

    // Get the pose of the sonar
    //
    private: void GetSonarPose(int s, double &px, double &py, double &pth);
    
    // Sonar timing settings
    //
    private: double updateInterval;
    private: double lastUpdate;
    
    // Maximum range of sonar in meters
    //
    private: double m_min_range;
    private: double m_max_range;

    // Array holding the sonar poses
    //
    private: int m_sonar_count;
    private: double m_sonar[SONARSAMPLES][3];
    
    // Array holding the sonar data
    //
    private: unsigned short m_range[SONARSAMPLES];
    
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

#else
    
    // Process GUI update messages
    //
    public: virtual void OnUiUpdate(RtkUiDrawData *pData);

    // Process GUI mouse messages
    //
    public: virtual void OnUiMouse(RtkUiMouseData *pData);

    // Draw the sonar scan
    //
    private: void DrawScan(RtkUiDrawData *pData);

    // Laser scan outline
    //
    private: int m_hit_count;
    private: double m_hit[SONARSAMPLES][2][2];
    
#endif    
};

#endif
