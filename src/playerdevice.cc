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
//  $Revision: 1.2.2.6 $
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

#define ENABLE_TRACE 1

#include "world.hh"
#include "playerdevice.hh"


///////////////////////////////////////////////////////////////////////////
// Minimal constructor
//
CPlayerDevice::CPlayerDevice(CWorld *world, CObject *parent,
                             CPlayerRobot *robot, size_t offset, size_t buffer_len,
                             size_t data_len, size_t command_len, size_t config_len)
        : CObject(world, parent)
{
    m_robot = robot;

    ASSERT(data_len + command_len + config_len <= buffer_len);

    m_info = NULL;
    m_data_buffer = NULL;
    m_command_buffer = NULL;
    m_config_buffer = NULL;

    m_offset = offset;
    m_info_len = INFO_BUFFER_SIZE;
    m_data_len = data_len;
    m_command_len = command_len;
    m_config_len = config_len;
}

///////////////////////////////////////////////////////////////////////////
// Default startup -- doesnt do much
//
bool CPlayerDevice::Startup(RtkCfgFile *cfg)
{
    if (!CObject::Startup(cfg))
        return false;

    // Get a pointer to the shared memory area
    //
    BYTE *buffer = (BYTE*) m_robot->GetShmem();
    if (buffer == NULL)
    {
        ERROR("shared memory pointer == NULL; cannot start device");
        return false;
    }
    
    // Initialise pointer to buffers
    //
    m_info = (PlayerStageInfo*) (buffer + m_offset);
    m_data_buffer = (BYTE*) m_info + m_info_len;
    m_command_buffer = (BYTE*) m_data_buffer + m_data_len;
    m_config_buffer = (BYTE*) m_command_buffer + m_command_len;

    TRACE4("creating player device at addr: %p %p %p %p", m_info, m_data_buffer,
         m_command_buffer, m_config_buffer);
    
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
void CPlayerDevice::Shutdown()
{
    CObject::Shutdown();
}


///////////////////////////////////////////////////////////////////////////
// See if the PlayerDevice is subscribed
//
bool CPlayerDevice::IsSubscribed()
{
    m_robot->LockShmem();
    bool subscribed = m_info->subscribed;
    m_robot->UnlockShmem();
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

    m_robot->LockShmem();
    
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

    m_robot->UnlockShmem();
    
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
    
    m_robot->LockShmem();

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
    
    m_robot->UnlockShmem();
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
    
    m_robot->LockShmem();

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
    
    m_robot->UnlockShmem();
    return len;
}

