///////////////////////////////////////////////////////////////////////////
//
// File: miscdevice.cc
// Author: Andrew Howard
// Date: 5 Feb 2000
// Desc: Simulates misc P2OS stuff
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/miscdevice.cc,v $
//  $Author: ahoward $
//  $Revision: 1.5.2.1 $
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


///////////////////////////////////////////////////////////////////////////
// Default constructor
//
CMiscDevice::CMiscDevice(CWorld *world, CEntity *parent )
        : CEntity(world, parent )
{
  // set the Player IO sizes correctly for this type of Entity
  m_data_len    = sizeof( player_misc_data_t );
  m_command_len = 0;//sizeof( misc_command_buffer_t );
  m_config_len  = 0;//sizeof( misc_config_buffer_t );
  
  m_player_type = PLAYER_MISC_CODE;
  m_stage_type = MiscType;
  m_color_desc = MISC_COLOR;

  m_interval = 0.1;
}


///////////////////////////////////////////////////////////////////////////
// Update the misc data
//
void CMiscDevice::Update( double sim_time )
{
  CEntity::Update( sim_time ); // inherit debug output

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











