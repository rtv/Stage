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
//  $Revision: 1.10 $
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

#define ENABLE_RTK_TRACE 0

#include <sys/time.h>
#include "world.hh"
#include "playerserver.hh"
#include "playerdevice.hh"


///////////////////////////////////////////////////////////////////////////
// Minimal constructor
//
CPlayerDevice::CPlayerDevice(CWorld *world, CEntity *parent,
                             CPlayerServer *server, size_t offset, size_t buffer_len,
                             size_t data_len, size_t command_len, size_t config_len)
        : CEntity(world, parent)
{
    m_port = -1;
    m_server = server;

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
// Load the object from an argument list
//
bool CPlayerDevice::Load(int argc, char **argv)
{
    if (!CEntity::Load(argc, argv))
        return false;

    // Set the default port number
    // We take our parents port number by default.
    //
    // *** TODO

    for (int i = 0; i < argc;)
    {
        if (strcmp(argv[i], "port") == 0 && i + 1 < argc)
        {
            m_port = atoi(argv[i + 1]);
            i += 2;
        }
        else
        {
            PLAYER_MSG1("unrecognized token [%s]", argv[i]);
            i += 1;
        }
    }
    return true;
}


/////////////////////////////////////////////////////////////////////////
// Save the object
//
bool CPlayerDevice::Save(int &argc, char **argv)
{
    if (!CEntity::Save(argc, argv))
        return false;

    // Save port
    //
    char port[32];
    snprintf(port, sizeof(port), "%d", m_port);
    argv[argc++] = strdup("port");
    argv[argc++] = strdup(port);

    return true;
}


///////////////////////////////////////////////////////////////////////////
// Default startup -- doesnt do much
//
bool CPlayerDevice::Startup()
{
    if (!CEntity::Startup())
        return false;

    // Find our server based on the port number
    //
    if (m_server == NULL)
    {
        m_server = m_world->FindServer(m_port);
        if (m_server == NULL)
        {
            printf("player server [%d] not found; cannot start device\n", (int) m_port);
            return false;
        }
    }

    // Get a pointer to the shared memory area
    //
    uint8_t *buffer = (uint8_t*) m_server->GetShmem();
    if (buffer == NULL)
    {
        printf("shared memory pointer == NULL; cannot start device\n");
        return false;
    }
    
    // Initialise pointer to buffers
    //
    m_info = (player_stage_info_t*) (buffer + m_offset);
    m_data_buffer = (uint8_t*) m_info + m_info_len;
    m_command_buffer = (uint8_t*) m_data_buffer + m_data_len;
    m_config_buffer = (uint8_t*) m_command_buffer + m_command_len;

    //printf("creating player device at addr: %p %p %p %p\n", m_info, m_data_buffer,
    //       m_command_buffer, m_config_buffer);
    
    // Mark this device as available
    //
    m_server->LockShmem();
    m_info->available = 1;
    m_server->UnlockShmem();
    
    return true;
}


///////////////////////////////////////////////////////////////////////////
// Default shutdown -- doesnt do much
//
void CPlayerDevice::Shutdown()
{
  // Mark this device as unavailable
  //
  // but only if our server actually exists - BPG
  if(m_server)
  {
    m_server->LockShmem();
    m_info->available = 0;
    m_server->UnlockShmem();
  }
    
    CEntity::Shutdown();
}


///////////////////////////////////////////////////////////////////////////
// See if the PlayerDevice is subscribed
//
bool CPlayerDevice::IsSubscribed()
{
    m_server->LockShmem();
    bool subscribed = m_info->subscribed;
    m_server->UnlockShmem();
    return subscribed;
}


///////////////////////////////////////////////////////////////////////////
// Write to the data buffer
//
size_t CPlayerDevice::PutData(void *data, size_t len,
                              uint32_t time_sec, uint32_t time_usec)
{
    struct timeval curr;
    gettimeofday(&curr,NULL);
    //if (len > m_data_len)
    //    RTK_MSG2("data len (%d) > buffer len (%d)", (int) len, (int) m_data_len);
    //ASSERT(len <= m_data_len);

    m_server->LockShmem();
    
    // Take the smallest number of bytes
    // This avoids an overflow of either buffer
    //
    len = min(len, m_data_len);

    // Copy the data (or as much as we were given)
    //
    memcpy(m_data_buffer, data, len);

    // Set the timestamp
    //
    if (time_sec == 0 && time_usec == 0)
    {
        timeval curr;
        gettimeofday(&curr, NULL);
        m_info->data_timestamp_sec = curr.tv_sec;
        m_info->data_timestamp_usec = curr.tv_usec;
    }
    else
    {
        m_info->data_timestamp_sec = time_sec;
        m_info->data_timestamp_usec = time_usec;
    }

    // Set data flag to indicate data is available
    //
    m_info->data_len = len;

    m_server->UnlockShmem();
    
    return len;
}


///////////////////////////////////////////////////////////////////////////
// Read from the data buffer
// Returns the number of bytes copied
//
size_t CPlayerDevice::GetData(void *data, size_t len,
                              uint32_t *time_sec, uint32_t *time_usec)
{
    m_server->LockShmem();
    
    // Take the smallest number of bytes
    // This avoids an overflow of either buffer
    //
    len = min(len, m_data_len);

    // Copy the data (or as much as we were given)
    //
    memcpy(data, m_data_buffer, len);

    // Copy the timestamp
    //
    if (time_sec != NULL)
        *time_sec = m_info->data_timestamp_sec;
    if (time_usec != NULL)
        *time_usec = m_info->data_timestamp_usec;

    m_server->UnlockShmem();
    
    return len;
}


///////////////////////////////////////////////////////////////////////////
// Read from the command buffer
//
size_t CPlayerDevice::GetCommand(void *data, size_t len)
{   
    //if (len < m_command_len)
    //    RTK_MSG2("buffer len (%d) < command len (%d)", (int) len, (int) m_command_len);
    //ASSERT(len >= m_command_len);
    
    m_server->LockShmem();

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
    
    m_server->UnlockShmem();
    return len;
}


///////////////////////////////////////////////////////////////////////////
// Read from the configuration buffer
//
size_t CPlayerDevice::GetConfig(void *data, size_t len)
{
    //if (len < m_config_len)
    //    RTK_MSG2("buffer len (%d) < config len (%d)", (int) len, (int) m_config_len);
    //ASSERT(len >= m_config_len);
    
    m_server->LockShmem();

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

    // Reset the config len to indicate that we have consumed this message
    //
    m_info->config_len = 0;
    
    m_server->UnlockShmem();
    return len;
}

