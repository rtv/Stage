///////////////////////////////////////////////////////////////////////////
//
// File: gpsdevice.hh
// Author: brian gerkey
// Date: 10 July 2001
// Desc: Simulates a generic GPS device
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/include/gpsdevice.hh,v $
//  $Author: inspectorg $
//  $Revision: 1.3 $
//
///////////////////////////////////////////////////////////////////////////

#ifndef GPSDEVICE_HH
#define GPSDEVICE_HH

#include "entity.hh"

class CGpsDevice : public CEntity
{
    // Default constructor
    //
    public: CGpsDevice(CWorld *world, CEntity *parent );
    
    // Update the device
    //
    public: virtual void Update( double sim_time );

    // Buffers for storing data
    //
    private: player_gps_data_t m_data;    
};

#endif






