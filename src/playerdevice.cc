///////////////////////////////////////////////////////////////////////////
//
// File: playerdevice.cc
// Author: Andrew Howard
// Date: 28 Nov 2000
// Desc: Base class for all player devices
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/playerdevice.cc,v $
//  $Author: ahoward $
//  $Revision: 1.3 $
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
#include "playerdevice.hh"


///////////////////////////////////////////////////////////////////////////
// Minimal constructor
//
CPlayerDevice::CPlayerDevice(CRobot *robot, void *buffer, size_t data_len, size_t command_len, size_t config_len)
        : CDevice(robot)
{
    m_info = (PlayerStageInfo*) buffer;
    m_info_len = INFO_BUFFER_SIZE;
    
    m_data_buffer = (BYTE*) buffer + m_info_len;
    m_data_len = data_len;

    m_command_buffer = (BYTE*) m_data_buffer + data_len;
    m_command_len = command_len;

    m_config_buffer = (BYTE*) m_command_buffer + command_len;
    m_config_len = config_len;

    TRACE4("creating player device at addr: %p %p %p %p", m_info_buffer, m_data_buffer,
         m_command_buffer, m_config_buffer);
}


///////////////////////////////////////////////////////////////////////////
// Default startup -- doesnt do much
//
bool CPlayerDevice::Startup()
{
    if (!CDevice::Startup())
        return false;

    /* *** TESTING -- this doesnt work right now
    // Mark this device as available
    //
    m_world->LockShmem();
    m_info->available = 1;
    m_world->UnlockShmem();
    */
    
    return true;
}


///////////////////////////////////////////////////////////////////////////
// Default shutdown -- doesnt do much
//
bool CPlayerDevice::Shutdown()
{
    return true;
}


///////////////////////////////////////////////////////////////////////////
// See if the PlayerDevice is subscribed
//
bool CPlayerDevice::IsSubscribed()
{
    m_world->LockShmem();
    bool subscribed = m_info->subscribed;
    m_world->UnlockShmem();

    return subscribed;
}


///////////////////////////////////////////////////////////////////////////
// Write to the data buffer
//
size_t CPlayerDevice::PutData(void *data, size_t len)
{
    //if (len > m_data_len)
    //    MSG2("data len (%d) > buffer len (%d)", (int) len, (int) m_data_len);
    //ASSERT(len <= m_data_len);

    m_world->LockShmem();

    // Take the smallest number of bytes
    // This avoids an overflow of either buffer
    //
    len = min(len, m_data_len);

    // Copy the data (or as much as we were given)
    //
    memcpy(m_data_buffer, data, len);

    // Set data flag to indicate data is available
    //
    m_info->data_len = len;
    
    m_world->UnlockShmem();
    return len;
}


///////////////////////////////////////////////////////////////////////////
// Read from the command buffer
//
size_t CPlayerDevice::GetCommand(void *data, size_t len)
{   
    //if (len < m_command_len)
    //    MSG2("buffer len (%d) < command len (%d)", (int) len, (int) m_command_len);
    //ASSERT(len >= m_command_len);
    
    m_world->LockShmem();

    // See if there is a command
    //
    size_t command_len = m_info->command_len;
    ASSERT(command_len <= m_command_len);

    // Take the smallest number of bytes
    // This avoids an overflow of either buffer
    //
    len = min(len, command_len);

    // Copy the command
    //
    memcpy(data, m_command_buffer, len);
    
    m_world->UnlockShmem();
    return len;
}


///////////////////////////////////////////////////////////////////////////
// Read from the configuration buffer
//
size_t CPlayerDevice::GetConfig(void *data, size_t len)
{
    //if (len < m_config_len)
    //    MSG2("buffer len (%d) < config len (%d)", (int) len, (int) m_config_len);
    //ASSERT(len >= m_config_len);
    
    m_world->LockShmem();

    // See if there is a config
    //
    size_t config_len = m_info->config_len;
    ASSERT(config_len <= m_config_len);

    // Take the smallest number of bytes
    // This avoids an overflow of either buffer
    //
    len = min(len, config_len);

    // Copy the command
    //
    memcpy(data, m_config_buffer, len);
    
    m_world->UnlockShmem();
    return len;
}

