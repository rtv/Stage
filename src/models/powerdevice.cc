/*
 *  Stage : a multi-robot simulator.
 *  Copyright (C) 2001, 2002 Richard Vaughan, Andrew Howard and Brian Gerkey.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */
/*
 * Desc: Simulates a sonar ring.
 * Author: Andrew Howard, Richard Vaughan
 * Date: 28 Nov 2000
 * CVS info: $Id: powerdevice.cc,v 1.2.8.1 2004-11-11 00:35:54 gerkey Exp $
 */

#include <math.h>
#include "world.hh"
#include "powerdevice.hh"

#include "world.hh"

// constructor
CPowerDevice::CPowerDevice(LibraryItem* libit,CWorld *world, CEntity *parent )
  : CPlayerEntity(libit,world, parent )    
{
  // set the Player IO sizes correctly for this type of Entity
  m_data_len    = sizeof( player_power_data_t );
  m_command_len = 0;
  m_config_len  = 1;
  m_reply_len  = 1;
  
  m_player.code = PLAYER_POWER_CODE; // from player's messages.h

  this->charge = 120; // volts/10  

  // once per second is fine for battery charge
  this->m_interval = 1.0; 
}

///////////////////////////////////////////////////////////////////////////
// Startup routine
//
bool CPowerDevice::Startup()
{
  if (!CPlayerEntity::Startup())
    return false;

  return true;
}

///////////////////////////////////////////////////////////////////////////
// Update the sonar data
void CPowerDevice::Update( double sim_time ) 
{
  CPlayerEntity::Update( sim_time );

  // dump out if noone is subscribed
  if(!Subscribed())
    return;

  // Check to see if it is time to update
  //  - if not, return right away.
  if( sim_time - m_last_update < m_interval)
    return;
  m_last_update = sim_time;

#if 0
  // Process configuration requests
  UpdateConfig();
#endif
  
  // TODO - have a configurable power consumption model in here.
  // for now I just return the same voltage all the time

  // pack the charge data in the buffer
  player_power_data_t powdata;
  powdata.charge = htons( charge );
  
  // and publish it
  PutData( &powdata, sizeof(powdata) );

  return;
}

#if 0
///////////////////////////////////////////////////////////////////////////
// Process configuration requests.

void CPowerDevice::UpdateConfig()
{
  int len;
  void* client;  
  player_power_config_t conf;
  
  while (true)
    {
      len = GetConfig(&client, &conf, sizeof(conf));
      if (len <= 0)
	break;
      
      switch( conf.subtype )
	{
	case PLAYER_MAIN_POWER_REQ:
	  // we got a request for the power data
	  
	  // pack the charge data in the buffer and reply with the data attached
	  player_power_data_t powdata;
	  powdata.charge = htons( charge );

	  PutReply(client, PLAYER_MSGTYPE_RESP_ACK, NULL, &powdata, sizeof(powdata));
	  
	  break;
	  
	default:
	  PRINT_WARN1("invalid power device configuration request [%c]", conf.subtype );
	  PutReply(client, PLAYER_MSGTYPE_RESP_NACK);
	  break;
	}
    }
}
#endif

