///////////////////////////////////////////////////////////////////////////
//
// File: playerplayerrobot.hhh
// Author: Richard Vaughan, Andrew Howard
// Date: 6 Dec 2000
// Desc: Provides interface to Player.
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/include/playerrobot.hh,v $
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

#ifndef ROBOT_H
#define ROBOT_H

// For CObject
//
#include "object.hh"

// Forward declare some of the classes we will use
//
class CDevice;
class CWorld;


class CPlayerRobot : public CObject
{
    public: CPlayerRobot( CWorld* world, CObject *parent);
    public: ~CPlayerRobot( void );
    
    // Update robot and all its devices
    //
    public: virtual void Update();

    // Start all the devices
    //
    public: virtual bool Startup(RtkCfgFile *cfg);

    // Shutdown the devices
    //
    public: virtual void Shutdown();

    // Start player
    //
    private: bool StartupPlayer(int port);

    // Stop player
    //
    private: void ShutdownPlayer();

    // Create a single semaphore to sync access to the shared memory segments
    //
    private: bool CreateShmemLock();

    // Lock the shared mem area
    //
    public: bool LockShmem( void );

    // Unlokc the shared mem area
    //
    public: bool UnlockShmem( void );

    // position, position at t-1, and position origin variables
    // *** REMOVE ahoward private: float x, y, a, oldx, oldy, olda, xorigin, yorigin, aorigin;

    // *** HACK these should be moved. ahoward
    public: unsigned char color; // unique ID and value drawn into world bitmap
    public: unsigned char channel; // the apparent color of this robot in ACTS

    // *** HACK -- playerIO should be private
    // Stuff needed to interface with player
    //
    public: caddr_t playerIO; // ptr to shared memory for player I/O
    private: char tmpName[16]; // name of shared memory device in filesystem
    private: int semKey, semid; // semaphore access for shared mem locking

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













