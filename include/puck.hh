///////////////////////////////////////////////////////////////////////////
//
// File: puck.hh
// Author: brian gerkey
// Date: 23 June 2001
// Desc: Simulates pucks
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/include/puck.hh,v $
//  $Author: gerkey $
//  $Revision: 1.4 $
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

#ifndef PUCK_HH
#define PUCK_HH

#include "entity.hh"

class CPuck : public CEntity
{
    // Default constructor
    //
    public: CPuck(CWorld *world, CEntity *parent);

    // Load the object from an argument list
    //
    public: virtual bool Load(int argc, char **argv);

    // Save the object to an argument list
    //
    public: virtual bool Save(int &argc, char **argv);

    // Startup routine
    //
    public: virtual bool Startup();
    
    // Update the device
    //
    public: virtual void Update();

    // Set the puck's speed
    // 
    public: void SetSpeed(double vr) { m_com_vr = vr; }

    // Move the puck
    // 
    private: void Move();

    // Check to see if the given pose will yield a collision
    //
    private: bool InCollision(double px, double py, double pth);

    ///////////////////////////////////////////////////////////////////////////
    // Check to see if the given pose will yield a collision with a movable 
    // object (e.g., robot, puck)
    //
    private: CEntity* InCollisionWithMovableObject(double px, 
                                               double py, 
                                               double pth);
    
    // puck channel (corresponds to ACTS channel)
    //
    private: int m_channel;

    // Return diameter of puck
    //
    public: double GetDiameter() { return(exp.width); }

    // Current mapped pose
    //
    private: double m_map_px, m_map_py, m_map_pth;

    // Timings (for movement)
    //
    private: double m_last_time;

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
