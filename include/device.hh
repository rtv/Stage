///////////////////////////////////////////////////////////////////////////
//
// File: device.hh
// Author: Andrew Howard
// Date: 28 Nov 2000
// Desc: Base class for all devices
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/include/device.hh,v $
//  $Author: vaughan $
//  $Revision: 1.6 $
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

// For size_t
//
#include <stddef.h>

//////////////////////////////////////////////////////////////////////////////
// Define length-specific data types
// For type sizes
#include "rtk-types.hh"


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
    // Default constructor
    // This is an abstract class and *cannot* be instantiated directly
    // buffer points to a single buffer containing the data, command 
    // and configuration buffers.
    //
    protected: CDevice(CRobot* rr, void *buffer, 
		       size_t data_len, size_t command_len, size_t config_len);
        
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

    // See if the device is subscribed
    //
    public: bool IsSubscribed();
    
    // Write to the data buffer
    // Returns the number of bytes copied
    //
    protected: size_t PutData(void *data, size_t len);

    // Read from the command buffer
    // Returns the number of bytes copied
    //
    protected: size_t GetCommand(void *data, size_t len);

    // Read from the configuration buffer
    // Returns the number of bytes copied
    //
    protected: size_t GetConfig(void *data, size_t len);

    // Pointer to the robot and world objects
    //
    protected: CRobot *m_robot;
    protected: CWorld *m_world;

    // Pointer to shared info buffers
    //
    private: BYTE *m_info_buffer;
    private: size_t m_info_len;

    // Pointer to shared data buffers
    //
    private: void *m_data_buffer;
    private: size_t m_data_len;

    // Pointer to shared command buffers
    //
    private: void *m_command_buffer;
    private: size_t m_command_len;

    // Pointer to shared config buffers
    //
    private: void *m_config_buffer;
    private: size_t m_config_len;
};


#endif
