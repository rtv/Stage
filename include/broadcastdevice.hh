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
//  $Revision: 1.2 $
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
    public: CBroadcastDevice(CWorld *world, CEntity *parent, CPlayerServer *server);

    // Startup routine
    //
    public: virtual bool StartUp();

    // Shutdown routine
    //
    public: virtual void Shutdown();

    // Update the device
    //
    public: virtual void Update();

    // Timing parameters
    //
    private: double m_last_update;
    private: double m_update_interval;

    // Buffers for storing data
    //
    private: size_t m_cmd_len;
    private: player_broadcast_cmd_t m_cmd;
    private: size_t m_data_len;
    private: player_broadcast_data_t m_data;   
};

#endif






