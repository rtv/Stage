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
//  $Revision: 1.2.2.3 $
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
    public: CPtzDevice(CPlayerRobot* robot,
                       void *buffer, size_t data_len, 
                       size_t command_len, size_t config_len);
    
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

#ifndef INCLUDE_RTK
    
public: 
  bool GUIDraw( void );
  bool GUIUnDraw( void );
  
  bool undrawRequired;
  XPoint drawPts[4];
  XPoint unDrawPts[4];

#endif

};

#endif




