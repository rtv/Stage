///////////////////////////////////////////////////////////////////////////
//
// File: device.hh
// Author: Andrew Howard
// Date: 28 Nov 2000
// Desc: Base class for all devices
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/include/device.hh,v $
//  $Author: gerkey $
//  $Revision: 1.8 $
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
//#include "rtk-types.hh"


// Forward declare the world and robot classes
//
class CWorld;
class CRobot;
class CWorldWindow;


//////////////////////////////////////////////////////////////////////////////
// Base class for all simulated devices
//
class CDevice
{
    // Minimal constructor
    // This is an abstract class and *cannot* be instantiated directly
    // buffer points to a single buffer containing the data, command 
    // and configuration buffers.
    //
    protected: CDevice(CRobot* robot);
        
    // Initialise the device
    //
    public: virtual bool Startup();

    // Close the device
    //
    public: virtual bool Shutdown();

    // Update the device
    // This is pure virtual and *must* be overloaded
    //
    public: virtual bool Update() = 0;

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
    protected: CWorld *m_world;
};


#endif
