///////////////////////////////////////////////////////////////////////////
//
// File: playerserver.hh
// Author: Richard Vaughan, Andrew Howard
// Date: 6 Dec 2000
// Desc: Provides interface to Player server.
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/include/playerdevice.hh,v $
//  $Author: gerkey $
//  $Revision: 1.12 $
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

#ifndef ROBOT_H
#define ROBOT_H

#include <sys/types.h>
#include "entity.hh"

// Forward declare some of the classes we will use
//
class CDevice;
class CWorld;


class CPlayerDevice : public CEntity
{
    public: CPlayerDevice( CWorld* world, CEntity *parent );
    public: ~CPlayerDevice( void );
    
    // Start all the devices
    //
    public: virtual bool SetupIOPointers( char* io );

    // Shutdown the devices
    //
    public: virtual void Shutdown();

    // Update robot and all its devices
    //
    public: virtual void Update( double sim_time );
    
    // Start player
    //
    // made this public so it can be called from world.cc - BPG
    // changed this from returning bool to returning int, because now
    // it returs the fd that is piped into the child player - BPG
    public: int StartupPlayer(int count);

    // Stop player
    //
    private: void ShutdownPlayer();

    // Stuff needed to interface with player
    //
    private: caddr_t playerIO; // ptr to shared memory for player I/O
  //private: char tmpName[16]; // name of shared memory device in filesystem

  // PID of player process spawned by this object
    //
    private: pid_t player_pid;

#ifdef INCLUDE_RTK
    
    // Process GUI update messages
    //
    public: virtual void OnUiUpdate(RtkUiDrawData *pData);

    // Process GUI mouse messages
    //
    public: virtual void OnUiMouse(RtkUiMouseData *pData);

#endif
};

#endif





















