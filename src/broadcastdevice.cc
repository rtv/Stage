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
//  $Revision: 1.4 $
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
CBroadcastDevice::CBroadcastDevice(CWorld *world, CEntity *parent )
  : CEntity(world, parent )
{
  // set the Player IO sizes correctly for this type of Entity
  m_data_len    = sizeof( player_broadcast_data_t ); 
  m_command_len = sizeof( player_broadcast_cmd_t );
  m_config_len  = 0;//sizeof( player_broadcast_config_t );
 
  m_player_type = PLAYER_BROADCAST_CODE;
  m_stage_type= BroadcastType;

  m_last_update = 0;
  m_interval = 0.050;

  m_size_x = 0.1;
  m_size_y = 0.1;
}


///////////////////////////////////////////////////////////////////////////
// Startup routine
//
bool CBroadcastDevice::Startup()
{
    if (!CEntity::Startup())
        return false;

    // Register ourself as a broadcast device with the world
    m_world->AddBroadcastDevice(this);

    return true;
}


///////////////////////////////////////////////////////////////////////////
// Shutdown routine
//
void CBroadcastDevice::Shutdown()
{
    // Unregister ourself as a broadcast device
    m_world->RemoveBroadcastDevice(this);
}


///////////////////////////////////////////////////////////////////////////
// Update the broadcast data
void CBroadcastDevice::Update( double sim_time )
{
    ASSERT(m_world != NULL);
    
    // See if its time to update
    if( sim_time - m_last_update < m_interval )
        return;
    m_last_update = sim_time;

    // See if there is any data to send
    if (GetCommand(&this->cmd, sizeof(this->cmd)) > 0)
    {
        // Loop through all messages in the buffer
        // and send them to other broadcast devices.
        uint8_t *p = this->cmd.buffer;
        uint8_t *msg = NULL;
        uint16_t len = 0;
        while (true)
        {
            msg = p + sizeof(len);
            len = ntohs(*((uint16_t*) p));
            if (len == 0)
                break;
            this->SendMsg(msg, len);
            p += len;
        }
    }

    // Send back all data that has accumulated
    // Clear our queue
    PutData(&this->data, this->data.len + sizeof(this->data.len));
    memset(&this->data, 0, sizeof(this->data));
}


///////////////////////////////////////////////////////////////////////////
// Send a message
void CBroadcastDevice::SendMsg(uint8_t *msg, uint16_t len)
{
    // Send message to all registered broadcast devices
    for (int i = 0; true; i++)
    {
        CBroadcastDevice *device = m_world->GetBroadcastDevice(i);
        if (device == NULL)
            break;
        device->RecvMsg(msg, len);
    }
}


///////////////////////////////////////////////////////////////////////////
// Receive a message
void CBroadcastDevice::RecvMsg(uint8_t *msg, uint16_t len)
{
    // Check for overflow
    // Need to leave 2 bytes for end-of-list marker
    if (this->data.len + len + sizeof(len) + sizeof(len) > sizeof(this->data.buffer))
    {
        printf("Warning: broadcast packet overrun; packets have been discarded\n");
        return;
    }
    
    // Write message and message length
    memcpy(this->data.buffer + this->data.len, &len, sizeof(len));
    memcpy(this->data.buffer + this->data.len + sizeof(len), msg, len);
    this->data.len += sizeof(len) + len;
}
   









