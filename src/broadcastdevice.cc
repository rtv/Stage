///////////////////////////////////////////////////////////////////////////
//
// File: broadcastdevice.cc
// Author: Andrew Howard
// Date: 12 Jan 2000
// Desc: Simulates the laser-based broadcast detector
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/broadcastdevice.cc,v $
//  $Author: ahoward $
//  $Revision: 1.2 $
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

#include <stage.h>
#include "world.hh"
#include "broadcastdevice.hh"


///////////////////////////////////////////////////////////////////////////
// Default constructor
//
CBroadcastDevice::CBroadcastDevice(CWorld *world, CEntity *parent, CPlayerServer *server)
        : CPlayerDevice(world, parent, server,
                        BROADCAST_DATA_START,
                        BROADCAST_TOTAL_BUFFER_SIZE,
                        BROADCAST_DATA_BUFFER_SIZE,
                        BROADCAST_COMMAND_BUFFER_SIZE,
                        BROADCAST_CONFIG_BUFFER_SIZE)
{
    m_last_update = 0;
    m_update_interval = 0.050;
}


///////////////////////////////////////////////////////////////////////////
// Startup routine
//
bool CBroadcastDevice::StartUp()
{
    /*
    if (!CPlayerDevice::Startup(cfg))
        return false;

    // Register ourself as a broadcast device with the world
    //
    m_world->AddBroadcastDevice(this);
    */
    return true;
}


///////////////////////////////////////////////////////////////////////////
// Shutdown routine
//
void CBroadcastDevice::Shutdown()
{
    // Unregister ourself as a broadcast device
    //
    m_world->RemoveBroadcastDevice(this);
}


///////////////////////////////////////////////////////////////////////////
// Update the broadcast data
//
void CBroadcastDevice::Update()
{
    ASSERT(m_server != NULL);
    ASSERT(m_world != NULL);
    
    // Check to see if it is time to update the laser
    //
    if (m_world->GetTime() - m_last_update <= m_update_interval)
        return;
    m_last_update = m_world->GetTime();
    
    // See if there is any data to send
    //
    m_cmd_len = GetCommand(&m_cmd, sizeof(m_cmd));
    if (m_cmd_len > 0)
    {
        // Send message to all registered broadcast devices
        //
        for (int i = 0; true; i++)
        {
            CBroadcastDevice *device = m_world->GetBroadcastDevice(i);
            if (device == NULL)
                break;

            // Check for overflow
            //
            if (device->m_data_len + m_cmd_len + 2 > sizeof(m_data))
            {
                printf("Warning: broadcast packet overrun; packets have been discarded\n");
                break;
            }
                
            memcpy(device->m_data.buffer + device->m_data_len, &m_cmd, m_cmd_len);
            device->m_data_len += m_cmd_len;
        }
    }

    // Send back all data that has accumulated
    //
    PutData(&m_data, m_data_len + 2);
    memset(&m_data, 0, sizeof(m_data));
    m_data_len = 0;
}













