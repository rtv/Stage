///////////////////////////////////////////////////////////////////////////
//
// File: miscdevice.cc
// Author: Andrew Howard
// Date: 5 Feb 2000
// Desc: Simulates misc P2OS stuff
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/models/miscdevice.cc,v $
//  $Author: rtv $
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

#include <stage.h>
#include "world.hh"
#include "miscdevice.hh"


// register this device type with the Library
CEntity misc_bootstrap( "misc", 
		       MiscType, 
		       (void*)&CMiscDevice::Creator ); 

///////////////////////////////////////////////////////////////////////////
// Default constructor
//
CMiscDevice::CMiscDevice(CWorld *world, CEntity *parent )
        : CPlayerEntity(world, parent )
{
  // set the Player IO sizes correctly for this type of Entity
  m_data_len    = sizeof( player_misc_data_t );
  m_command_len = 0;
  m_config_len  = 0;
  m_reply_len  = 0;
  
  m_player.code = PLAYER_MISC_CODE;
  this->stage_type = MiscType;
  this->color = ::LookupColor(MISC_COLOR);

  m_interval = 0.1;
}


///////////////////////////////////////////////////////////////////////////
// Update the misc data
//
void CMiscDevice::Update( double sim_time )
{
  CPlayerEntity::Update( sim_time ); // inherit debug output

  if(!Subscribed()) 
    return;
   
  // Check to see if it is time to update
  //  - if not, return right away.
  if( sim_time - m_last_update < m_interval) return;
  
  m_last_update = sim_time;
 
  m_data.frontbumpers = 0;
  m_data.rearbumpers = 0;
  m_data.voltage = 120;
  
  PutData( &m_data, sizeof(m_data) );
}











