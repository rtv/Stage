///////////////////////////////////////////////////////////////////////////
//
// File: visionbeacon.hh
// Author: Andrew Howard
// Date: 6 Dec 2000
// Desc: Simulates ACTS vision beacons
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/include/visionbeacon.hh,v $
//  $Author: gerkey $
//  $Revision: 1.5 $
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

#ifndef VISIONBEACON_HH
#define VISIONBEACON_HH

#include "entity.hh"

class CVisionBeacon : public CEntity
{
    // Default constructor
    //
    public: CVisionBeacon(CWorld *world, CEntity *parent);

    // Load the object from an argument list
    //
    public: virtual bool Load(int argc, char **argv);

    // Save the object to an argument list
    //
    public: virtual bool Save(int &argc, char **argv);
    
    // Update the device
    //
    public: virtual void Update();

    // Beacon radius
    //
    private: double m_radius;

    // Layers to render to
    //
    private: bool m_render_obstacle, m_render_laser;
    
    // Beacon channel (corresponds to ACTS channel)
    //
    private: int m_channel;

    // Current pose
    //
    private: double m_map_px, m_map_py, m_map_pth;

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
