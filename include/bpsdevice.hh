///////////////////////////////////////////////////////////////////////////
//
// File: bpsdevice.hh
// Author: Brian Gerkey
// Date: 22 May 2002
// Desc: Simulates the BPS device
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/include/bpsdevice.hh,v $
//  $Author: gerkey $
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

class CBpsDevice : public CEntity
{
    // Default constructor
    public: CBpsDevice(CWorld *world, CEntity *parent );

    // Startup routine
    public: virtual bool Startup();

    // Update the device
    public: virtual void Update( double sim_time );
};

#endif






