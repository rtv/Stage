///////////////////////////////////////////////////////////////////////////
//
// File: bpsdevice.hh
// Author: Brian Gerkey
// Date: 22 May 2002
// Desc: Simulates the BPS device
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/models/bpsdevice.hh,v $
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

#ifndef BPSDEVICE_HH
#define BPSDEVICE_HH

#include "playerdevice.hh"

class CBpsDevice : public CPlayerEntity
{
    // Default constructor
    public: CBpsDevice(CWorld *world, CEntity *parent );

  // a static named constructor - a pointer to this function is given
  // to the Library object and paired with a string.  When the string
  // is seen in the worldfile, this function is called to create an
  // instance of this entity
public: static CBpsDevice* Creator( CWorld *world, CEntity *parent )
  { return( new CBpsDevice( world, parent ) ); }

    // Startup routine
    public: virtual bool Startup();

    // Update the device
    public: virtual void Update( double sim_time );
};

#endif






