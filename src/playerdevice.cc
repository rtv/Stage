///////////////////////////////////////////////////////////////////////////
//
// File: playerdevice.cc
// Author: Andrew Howard
// Date: 28 Nov 2000
// Desc: Base class for all player devices
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/playerdevice.cc,v $
//  $Author: gerkey $
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

#include "world.h"
#include "playerdevice.hh"
#include <sys/time.h>
#include <unistd.h>


///////////////////////////////////////////////////////////////////////////
// Minimal constructor
//
CPlayerDevice::CPlayerDevice(CRobot *robot, void *buffer, size_t data_len, size_t command_len, size_t config_len)
        : CDevice(robot)
{
    m_info = (player_stage_info_t*) buffer;
    m_info_len = INFO_BUFFER_SIZE;
    
    m_data_buffer = (uint8_t*) buffer + m_info_len;
    m_data_len = data_len;

    m_command_buffer = (uint8_t*) m_data_buffer + data_len;
    m_command_len = command_len;

    m_config_buffer = (uint8_t*) m_command_buffer + command_len;
    m_config_len = config_len;

    PLAYER_TRACE4("creating player device at addr: %p %p %p %p", m_info, m_data_buffer,
         m_command_buffer, m_config_buffer);
}


///////////////////////////////////////////////////////////////////////////
// Default startup -- doesnt do much
//
bool CPlayerDevice::Startup()
{
    if (!CDevice::Startup())
        return false;

    // Mark this device as available
    //
    m_world->LockShmem();
    m_info->available = 1;
    m_world->UnlockShmem();
    
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
size_t CPlayerDevice::PutData(void *data, size_t len, uint64_t timestamp)
{
    struct timeval curr;
    gettimeofday(&curr,NULL);
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

    if(!timestamp)
      m_info->data_timestamp = 
              htonll((uint64_t)((((uint64_t)curr.tv_sec) * 1000000) + 
                                      (uint64_t)curr.tv_usec));
    else
      m_info->data_timestamp = htonll(timestamp);
    //
    //printf("CPlayerDevice::PutData: set timestamp to: %Lu\n",
                    //ntohll(m_info->data_timestamp));
    
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

