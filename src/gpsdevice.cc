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
//  $Revision: 1.1 $
//
///////////////////////////////////////////////////////////////////////////

#include <stage.h>
#include "world.hh"
#include "gpsdevice.hh"


///////////////////////////////////////////////////////////////////////////
// Default constructor
//
CGpsDevice::CGpsDevice(CWorld *world, CEntity *parent, CPlayerServer *server)
        : CPlayerDevice(world, parent, server,
                        GPS_DATA_START,
                        GPS_TOTAL_BUFFER_SIZE,
                        GPS_DATA_BUFFER_SIZE,
                        GPS_COMMAND_BUFFER_SIZE,
                        GPS_CONFIG_BUFFER_SIZE)
{


  exp.objectType = gps_o;
  exp.objectId = this;
  exp.data = (char*)&m_data;  
  strcpy( exp.label, "GPS" );
}


///////////////////////////////////////////////////////////////////////////
// Update the gps data
//
void CGpsDevice::Update()
{
    ASSERT(m_server != NULL);
    ASSERT(m_world != NULL);
    
    double px,py,pth;
    GetGlobalPose(px,py,pth);

    m_data.xpos = htonl((int)(px*1000.0));
    m_data.ypos = htonl((int)(py*1000.0));
   
    PutData(&m_data, sizeof(m_data));
}


