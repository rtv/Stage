///////////////////////////////////////////////////////////////////////////
//
// File: broadcastdevice.hh
// Author: Andrew Howard
// Date: 5 Feb 2000
// Desc: Simulates the IP broadcast device
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/include/broadcastdevice.hh,v $
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

#ifndef BROADCASTDEVICE_HH
#define BROADCASTDEVICE_HH

#include "playerdevice.hh"

class CBroadcastDevice : public CEntity
{
    // Default constructor
    //
    public: CBroadcastDevice(CWorld *world, CEntity *parent );

    // Startup routine
    //
    public: virtual bool StartUp();

    // Shutdown routine
    //
    public: virtual void Shutdown();

    // Update the device
    //
    public: virtual void Update( double sim_time );

    // Buffers for storing data
    //
    private: size_t m_cmd_len;
    private: player_broadcast_cmd_t m_cmd;
    private: size_t m_data_len;
    private: player_broadcast_data_t m_data;   
};

#endif






