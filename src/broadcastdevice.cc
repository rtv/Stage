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
//  $Revision: 1.1.2.2 $
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

#define ENABLE_RTK_TRACE 1

#include <stage.h>
#include "world.hh"
#include "playerrobot.hh"
#include "broadcastdevice.hh"


///////////////////////////////////////////////////////////////////////////
// Default constructor
//
CBroadcastDevice::CBroadcastDevice(CWorld *world, CObject *parent, CPlayerRobot *robot)
        : CPlayerDevice(world, parent, robot,
                        BROADCAST_DATA_START,
                        BROADCAST_TOTAL_BUFFER_SIZE,
                        BROADCAST_DATA_BUFFER_SIZE,
                        BROADCAST_COMMAND_BUFFER_SIZE,
                        BROADCAST_CONFIG_BUFFER_SIZE)
{
    m_last_packet = 0;
    m_last_update = 0;
    m_update_interval = 0.050;
}


///////////////////////////////////////////////////////////////////////////
// Update the broadcast data
//
void CBroadcastDevice::Update()
{
    ASSERT(m_robot != NULL);
    ASSERT(m_world != NULL);
    
    // Check to see if it is time to update the laser
    //
    if (m_world->GetTime() - m_last_update <= m_update_interval)
        return;
    m_last_update = m_world->GetTime();
    
    // See if there is any data to broadcast
    //
    size_t send_len = GetCommand(&m_cmd, sizeof(m_cmd));
    if (send_len > 0)
        m_world->PutBroadcast((uint8_t*) &m_cmd, send_len);

    // See if there is any data to read
    //
    size_t recv_len = m_world->GetBroadcast(&m_last_packet, (uint8_t*) &m_data, sizeof(m_data));
    if (recv_len > 0)
        PutData(&m_data, sizeof(m_data));
}



#ifdef INCLUDE_RTK

///////////////////////////////////////////////////////////////////////////
// Process GUI update messages
//
void CBroadcastDevice::OnUiUpdate(RtkUiDrawData *pData)
{
    // Draw our children
    //
    CObject::OnUiUpdate(pData);
}

#endif












