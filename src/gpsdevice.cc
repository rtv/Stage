///////////////////////////////////////////////////////////////////////////
//
// File: gpsdevice.cc
// Author: brian gerkey
// Date: 10 July 2001
// Desc: Simulates a generic GPS device
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/gpsdevice.cc,v $
//  $Author: vaughan $
//  $Revision: 1.2 $
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

  m_size_x = 0.1;
  m_size_y = 0.1;

  m_player_type = PLAYER_GPS_CODE;
  m_stage_type = GpsType;

  m_interval = 0.05;
}


///////////////////////////////////////////////////////////////////////////
// Update the gps data
//
void CGpsDevice::Update( double sim_time )
{
    ASSERT(m_world != NULL);
    
    if( Subscribed() < 1)
      return;
    
    if( sim_time - m_last_update < m_interval )
      return;
    
    m_last_update = sim_time;
    

    double px,py,pth;
    GetGlobalPose(px,py,pth);

    m_data.xpos = htonl((int)(px*1000.0));
    m_data.ypos = htonl((int)(py*1000.0));
   
    PutData(&m_data, sizeof(m_data));
}


