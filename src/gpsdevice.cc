///////////////////////////////////////////////////////////////////////////
//
// File: gpsdevice.cc
// Author: brian gerkey
// Date: 10 July 2001
// Desc: Simulates a generic GPS device
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/gpsdevice.cc,v $
//  $Author: rtv $
//  $Revision: 1.10 $
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
  //m_config_len  = sizeof( player_gps_config_t );
  m_config_len  = 1;
  m_reply_len  = 1;

  m_player.type = PLAYER_GPS_CODE;
  m_stage_type = GpsType;

  SetColor(GPS_COLOR);
  
  m_interval = 0.05; 
}


///////////////////////////////////////////////////////////////////////////
// Update the gps data
//
void CGpsDevice::Update( double sim_time )
{
  double px,py,pth;
  void* client;

  ASSERT(m_world != NULL);
    
  if(!Subscribed())
    return;
    
  if( sim_time - m_last_update < m_interval )
    return;
  m_last_update = sim_time;

  // Check for teleportation requests
  // Move the devices parent, if it has one.
  // Should probably move the top-level object that the
  // device is attached to.
  player_gps_config_t config;
  if(GetConfig(&client,&config, sizeof(config)) > 0)
  {
    px = ntohl(config.xpos)/1000.0;
    py = ntohl(config.ypos)/1000.0;
    pth = ntohl(config.heading)*M_PI/180;
    if (m_parent_object)
    {
      m_parent_object->SetGlobalPose(px, py, pth);
      m_parent_object->SetDirty(1);
    }
    else
      SetGlobalPose(px, py, pth);

    PutReply(client, PLAYER_MSGTYPE_RESP_ACK, NULL, NULL, 0);
  }

  // Return global pose
  GetGlobalPose(px,py,pth);
  m_data.xpos = htonl((int)(px*1000.0));
  m_data.ypos = htonl((int)(py*1000.0));
  m_data.heading = htonl((int)(pth*180/M_PI));
  PutData(&m_data, sizeof(m_data));
}


