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
//  $Revision: 1.2 $
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
CMiscDevice::CMiscDevice(CWorld *world, CEntity *parent, CPlayerServer *server)
        : CPlayerDevice(world, parent, server,
                        MISC_DATA_START,
                        MISC_TOTAL_BUFFER_SIZE,
                        MISC_DATA_BUFFER_SIZE,
                        MISC_COMMAND_BUFFER_SIZE,
                        MISC_CONFIG_BUFFER_SIZE)
{


  exp.objectType = misc_o;
  exp.objectId = this;
  exp.data = (char*)&m_data;  
  strcpy( exp.label, "Pioneer Misc" );
}


///////////////////////////////////////////////////////////////////////////
// Update the misc data
//
void CMiscDevice::Update()
{
    ASSERT(m_server != NULL);
    ASSERT(m_world != NULL);
    
    m_data.frontbumpers = 0;
    m_data.rearbumpers = 0;
    m_data.voltage = 120;
   
    PutData(&m_data, sizeof(m_data));
}











