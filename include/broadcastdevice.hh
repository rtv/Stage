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
//  $Revision: 1.6 $
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
    public: CBroadcastDevice(CWorld *world, CEntity *parent );

    // Startup routine
    public: virtual bool Startup();

    // Shutdown routine
    public: virtual void Shutdown();

    // Update the device
    public: virtual void Update( double sim_time );

    // Send a message
    private: void SendMsg(uint8_t *msg, uint16_t len);

    // Receive a message
    private: void RecvMsg(uint8_t *msg, uint16_t len);
    
    // Buffers for storing data
    private: player_broadcast_cmd_t cmd;
    private: player_broadcast_data_t data;   
};

#endif






