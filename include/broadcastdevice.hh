///////////////////////////////////////////////////////////////////////////
//
// File: broadcastdevice.hh
// Author: Andrew Howard
// Date: 5 Feb 2000
// Desc: Simulates the IP broadcast device
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/include/broadcastdevice.hh,v $
//  $Author: ahoward $
//  $Revision: 1.1.2.2 $
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

#ifndef BROADCASTDEVICE_HH
#define BROADCASTDEVICE_HH

#include "playerdevice.hh"

class CBroadcastDevice : public CPlayerDevice
{
    // Default constructor
    //
    public: CBroadcastDevice(CWorld *world, CObject *parent, CPlayerRobot *robot);
    
    // Update the device
    //
    public: virtual void Update();

    // Timing parameters
    //
    private: double m_last_update;
    private: double m_update_interval;
    
    // Index of last broadcast packet
    //
    private: int m_last_packet;

    // Buffers for storing data
    //
    private: player_broadcast_data_t m_data;
    private: player_broadcast_cmd_t m_cmd;
 
#ifdef INCLUDE_RTK
    
    // Process GUI update messages
    //
    public: virtual void OnUiUpdate(RtkUiDrawData *pData);
    
#endif    
};

#endif






