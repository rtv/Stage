///////////////////////////////////////////////////////////////////////////
//
// File: ptzdevice.hh
// Author: Andrew Howard
// Date: 01 Dec 2000
// Desc: Simulates the Sony pan-tilt-zoom camera
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/include/ptzdevice.hh,v $
//  $Author: ahoward $
//  $Revision: 1.2.2.7 $
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

#ifndef PTZDEVICE_HH
#define PTZDEVICE_HH


#include "playerdevice.hh"


class CPtzDevice : public CPlayerDevice
{
    // Default constructor
    //
    public: CPtzDevice(CWorld *world, CEntity *parent, CPlayerRobot* robot);
    
    // Update the device
    //
    public: virtual void Update();

    // Get the pan/tilt/zoom values
    // The pan and tilt are angles (in radians)
    // The zoom is a the field of view (in radians)
    //
    public: void GetPTZ(double &pan, double &tilt, double &zoom);

    // Update times
    //
    private: double m_last_update, m_update_interval;
    
    // Physical limits
    //
    private: double m_pan_min, m_pan_max;
    private: double m_tilt_min, m_tilt_max;
    private: double m_zoom_min, m_zoom_max;
    private: double m_fov_min, m_fov_max;

    // Current camera settings
    //
    private: double m_pan, m_tilt, m_zoom;

#ifdef INCLUDE_RTK
    
    // Process GUI update messages
    //
    public: virtual void OnUiUpdate(RtkUiDrawData *pData);

    // Process GUI mouse messages
    //
    public: virtual void OnUiMouse(RtkUiMouseData *pData);
    
#endif

};

#endif




