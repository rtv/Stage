///////////////////////////////////////////////////////////////////////////
//
// File: device.hh
// Author: Andrew Howard
// Date: 28 Nov 2000
// Desc: Base class for all devices
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/include/device.hh,v $
//  $Author: ahoward $
//  $Revision: 1.7.2.2 $
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

#ifndef DEVICE_HH
#define DEVICE_HH

// For type sizes and message macros
//
#include "rtk-types.hh"


// Forward declare the world and robot classes
//
class CWorld;
class CRobot;
class CWorldWindow;

// For base class
//
#include "object.hh"


//////////////////////////////////////////////////////////////////////////////
// Base class for all simulated devices
//
class CDevice : public CObject
{
    // Minimal constructor
    //
    protected: CDevice(CRobot* robot);

    /* *** REMOVE ahoward
    // Initialise the device
    //
    public: virtual bool Startup(RtkCfgFile *cfg);

    // Close the device
    //
    public: virtual bool Shutdown();

    // Update the device
    // This is pure virtual and *must* be overloaded
    //
    public: virtual bool Update() = 0;
    */

    // Generic bitmap draw method - most devices don't draw themselves
    //
    public: virtual bool MapDraw(){ return true; }; //do nowt 

    // Generic bitmap undraw method - most devices don't undraw themselves
    //
    public: virtual bool MapUnDraw(){ return true; }; //do nowt 

    // Generic GUI draw method - most devices don't draw themselves
    //
    public: virtual bool GUIDraw(){ return true; }; //do nowt 

    // Generic GUI undraw method - most devices don't undraw themselves
    //
    public: virtual bool GUIUnDraw(){ return true; }; //do nowt 

    // Pointer to the robot and world objects
    //
    protected: CRobot *m_robot;
    // *** REMOVE ahoward protected: CWorld *m_world;
};


#endif
