///////////////////////////////////////////////////////////////////////////
//
// File: bpsdevice.cc
// Author: Brian Gerkey
// Date: 22 May 2002
// Desc: Simulates the bps device
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/bpsdevice.cc,v $
//  $Author: gerkey $
//  $Revision: 1.3 $
//
// Usage:
//  (empty)
//
// Theory of operation:
//  This is really just a place-holder; the real work is done by the
//  real BPS device in Player.
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
#include "bpsdevice.hh"


///////////////////////////////////////////////////////////////////////////
// Default constructor
//
CBpsDevice::CBpsDevice(CWorld *world, CEntity *parent )
  : CEntity(world, parent )
{
    // set the Player IO sizes correctly for this type of Entity
    m_data_len    = sizeof( player_bps_data_t ); 
    m_command_len = 0;
    m_config_len  = 1;
    m_reply_len  = 1;
 
    m_player.code = PLAYER_BPS_CODE;
    m_stage_type = BpsType;
}


///////////////////////////////////////////////////////////////////////////
// Startup routine
//
bool CBpsDevice::Startup()
{
    if (!CEntity::Startup())
        return false;
    return true;
}


///////////////////////////////////////////////////////////////////////////
// Update the broadcast data
void CBpsDevice::Update( double sim_time )
{
}
