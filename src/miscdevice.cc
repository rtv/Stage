///////////////////////////////////////////////////////////////////////////
//
// File: miscdevice.cc
// Author: Andrew Howard
// Date: 5 Feb 2000
// Desc: Simulates misc P2OS stuff
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/miscdevice.cc,v $
//  $Author: vaughan $
//  $Revision: 1.1.2.4 $
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

#ifdef INCLUDE_XGUI
  exp.objectType = misc_o;
  exp.objectId = this;
  exp.data = (char*)&expMisc;  
#endif

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

#ifdef INCLUDE_XGUI
////////////////////////////////////////////////////////////////////////////
// compose and return the export data structure for external rendering
// return null if we're not exporting data right now.
ExportData* CMiscDevice::ImportExportData( ImportData* imp )
{
  //CObject::ImportExportData( imp );
 if( imp ) // if there is some imported data
    SetGlobalPose( imp->x, imp->y, imp->th ); // move to the suggested place
  
  if( !exporting ) return 0;

  // fill in the exp structure
  // exp.type, exp.id are set in the constructor
  GetGlobalPose( exp.x, exp.y, exp.th );
  
  //exp.width = m_size_x;
  //exp.height = m_size_y;

   expMisc.frontbumpers = 0;
   expMisc.rearbumpers = 0;
   expMisc.voltage = 120;

  return &exp;
}

#endif












