///////////////////////////////////////////////////////////////////////////
//
// File: playerdevice.hh
// Author: Andrew Howard
// Date: 28 Nov 2000
// Desc: Base class for all devices
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/include/playerdevice.hh,v $
//  $Author: ahoward $
//  $Revision: 1.1.2.6 $
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

#ifndef PLAYERDEVICE_HH
#define PLAYERDEVICE_HH

// For size_t
//
#include <stddef.h>

// For base class
//
#include "object.hh"

// For all the lengths
//
#include <offsets.h>


////////////////////////////////////////////////////////////////////////////////
// Base class for all player devices
//
class CPlayerDevice : public CObject
{
    // Minimal constructor
    // This is an abstract class and *cannot* be instantiated directly
    // buffer points to a single buffer containing the data, command and configuration buffers.
    //
    protected: CPlayerDevice(CWorld *world, CObject *parent, 
                             CPlayerRobot *robot, size_t offset, size_t buffer_len,
                             size_t data_len, size_t command_len, size_t config_len);
    
    // Initialise the device
    //
    public: virtual bool Startup(RtkCfgFile *cfg);

    // Close the device
    //
    public: virtual void Shutdown();

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

    // Pointer to player robot
    //
    protected: CPlayerRobot *m_robot;

    // Offset info shared memory
    //
    private: size_t m_offset;
    
    // Pointer to shared info buffers
    //
    private: PlayerStageInfo *m_info;
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
