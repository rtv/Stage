///////////////////////////////////////////////////////////////////////////
//
// File: gpsdevice.cc
// Author: brian gerkey
// Date: 10 July 2001
// Desc: Simulates a generic GPS device
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/gpsdevice.cc,v $
//  $Author: gerkey $
//  $Revision: 1.14 $
//
///////////////////////////////////////////////////////////////////////////

#include <stage.h>
#include "world.hh"
#include "gpsdevice.hh"


///////////////////////////////////////////////////////////////////////////
// Default constructor
//
CGpsDevice::CGpsDevice(CWorld *world, CEntity *parent )
  : CEntity(world, parent )
{
  m_data_len    = sizeof( player_gps_data_t ); 
  m_command_len = 0;
  m_config_len  = 0;
  m_reply_len  = 0;

  m_player.code = PLAYER_GPS_CODE;
  this->stage_type = GpsType;
  this->color = ::LookupColor(GPS_COLOR);
  
  m_interval = 0.05; 
}


///////////////////////////////////////////////////////////////////////////
// Update the gps data
//
void CGpsDevice::Update( double sim_time )
{
  double px,py,pth;

  ASSERT(m_world != NULL);
    
  if(!Subscribed())
    return;
    
  if( sim_time - m_last_update < m_interval )
    return;
  m_last_update = sim_time;

  // Return global pose
  GetGlobalPose(px,py,pth);
  m_data.xpos = htonl((int)(px*1000.0));
  m_data.ypos = htonl((int)(py*1000.0));
  m_data.heading = htonl((int)(RTOD(NORMALIZE(pth))));
  PutData(&m_data, sizeof(m_data));
}


