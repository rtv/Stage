///////////////////////////////////////////////////////////////////////////
//
// File: miscdevice.hh
// Author: Andrew Howard
// Date: 6 Feb 2000
// Desc: Simulates the misc P2OS services
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/include/miscdevice.hh,v $
//  $Author: rtv $
//  $Revision: 1.7 $
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

#ifndef MISCDEVICE_HH
#define MISCDEVICE_HH

#include "playerdevice.hh"

class CMiscDevice : public CPlayerEntity
{
    // Default constructor
    //
    public: CMiscDevice(CWorld *world, CEntity *parent );
    
  // a static named constructor - a pointer to this function is given
  // to the Library object and paired with a string.  When the string
  // is seen in the worldfile, this function is called to create an
  // instance of this entity
public: static CMiscDevice* Creator( CWorld *world, CEntity *parent )
  { return( new CMiscDevice( world, parent ) ); }

    // Update the device
    //
    public: virtual void Update( double sim_time );

    // Buffers for storing data
    //
    private: player_misc_data_t m_data;    
};

#endif






