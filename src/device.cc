///////////////////////////////////////////////////////////////////////////
//
// File: device.cc
// Author: Andrew Howard
// Date: 28 Nov 2000
// Desc: Base class for all devices
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/device.cc,v $
//  $Author: ahoward $
//  $Revision: 1.1 $
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

#include "world.h"
#include "device.hh"


///////////////////////////////////////////////////////////////////////////
// Minimal constructor
//
CDevice::CDevice(void *data_buffer, size_t data_len)
{
    m_data_buffer = data_buffer;
    m_data_len = data_len;  
}


///////////////////////////////////////////////////////////////////////////
// Default open -- doesnt do much
//
bool CDevice::Open(CRobot *robot, CWorld *world)
{
    m_robot = robot;
    m_world = world;
    return true;
}


///////////////////////////////////////////////////////////////////////////
// Default close -- doesnt do much
//
bool CDevice::Close()
{
    m_robot = NULL;
    m_world = NULL;
    return true;
}


///////////////////////////////////////////////////////////////////////////
// Write to the data buffer
//
void CDevice::PutData(void *data, size_t len)
{
    if (len > m_data_len)
        MSG2("data len (%d) > buffer len (%d)", (int) len, (int) m_data_len);
    ASSERT(len <= m_data_len);

    // Copy the data
    //
    m_world->LockShmem();
    memcpy(m_data_buffer, data, len);
    m_world->UnlockShmem();
}


///////////////////////////////////////////////////////////////////////////
// Read from the command buffer
//
void CDevice::GetCommand(void *data, size_t len)
{
}


///////////////////////////////////////////////////////////////////////////
// Read from the configuration buffer
//
void CDevice::GetConfig(void *data, size_t len)
{
}

