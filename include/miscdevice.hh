///////////////////////////////////////////////////////////////////////////
//
// File: miscdevice.hh
// Author: Andrew Howard
// Date: 6 Feb 2000
// Desc: Simulates the misc P2OS services
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/include/miscdevice.hh,v $
//  $Author: vaughan $
//  $Revision: 1.3 $
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

class CMiscDevice : public CEntity
{
    // Default constructor
    //
    public: CMiscDevice(CWorld *world, CEntity *parent );
    
    // Update the device
    //
    public: virtual void Update( double sim_time );

    // Buffers for storing data
    //
    private: player_misc_data_t m_data;    
};

#endif






