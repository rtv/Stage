///////////////////////////////////////////////////////////////////////////
//
// File: gpsdevice.hh
// Author: brian gerkey
// Date: 10 July 2001
// Desc: Simulates a generic GPS device
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/include/gpsdevice.hh,v $
//  $Author: gerkey $
//  $Revision: 1.1 $
//
///////////////////////////////////////////////////////////////////////////

#ifndef GPSDEVICE_HH
#define GPSDEVICE_HH

#include "playerdevice.hh"

class CGpsDevice : public CPlayerDevice
{
    // Default constructor
    //
    public: CGpsDevice(CWorld *world, CEntity *parent, CPlayerServer *server);
    
    // Update the device
    //
    public: virtual void Update();

    // Buffers for storing data
    //
    private: player_gps_data_t m_data;    
};

#endif






