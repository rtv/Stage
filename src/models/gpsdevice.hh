///////////////////////////////////////////////////////////////////////////
//
// File: gpsdevice.hh
// Author: brian gerkey
// Date: 10 July 2001
// Desc: Simulates a generic GPS device
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/models/gpsdevice.hh,v $
//  $Author: rtv $
//  $Revision: 1.2 $
//
///////////////////////////////////////////////////////////////////////////

#ifndef GPSDEVICE_HH
#define GPSDEVICE_HH

#include "playerdevice.hh"

class CGpsDevice : public CPlayerEntity
{
    // Default constructor
    //
    public: CGpsDevice( LibraryItem *libit, CWorld *world, CEntity *parent );
    
  // a static named constructor - a pointer to this function is given
  // to the Library object and paired with a string.  When the string
  // is seen in the worldfile, this function is called to create an
  // instance of this entity
public: static CGpsDevice* Creator(  LibraryItem *libit, CWorld *world, CEntity *parent )
  {
    return( new CGpsDevice( libit, world, parent ) );
  }
    
    // Startup routine
    public: virtual bool Startup();

    // Update the device
    //
    public: virtual void Update( double sim_time );

    // Buffers for storing data
    //
    private: player_gps_data_t m_data;    
};

#endif






