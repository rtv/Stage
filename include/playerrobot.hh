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
//  $Revision: 1.1.2.8 $
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
#include "object.hh"

// Forward declare some of the classes we will use
//
class CDevice;
class CWorld;


class CPlayerRobot : public CObject
{
    public: CPlayerRobot( CWorld* world, CObject *parent);
    public: ~CPlayerRobot( void );
    
    // Start all the devices
    //
    public: virtual bool Startup();

    // Shutdown the devices
    //
    public: virtual void Shutdown();

    // Update robot and all its devices
    //
    public: virtual void Update();
    
    // Start player
    //
    private: bool StartupPlayer(int port);

    // Stop player
    //
    private: void ShutdownPlayer();

    // Create a single semaphore to sync access to the shared memory segments
    //
    private: bool CreateShmemLock();

    // Get a pointer to shared mem area
    //
    public: void* GetShmem() {return playerIO;};
    
    // lock the shared mem area
    //
    public: bool LockShmem( void );

    // Unlokc the shared mem area
    //
    public: void UnlockShmem( void );

    // *** HACK these should be moved. ahoward
    public: unsigned char color; // unique ID and value drawn into world bitmap
    public: unsigned char channel; // the apparent color of this robot in ACTS

    // Port number for player
    //
    public: int m_port;

    // Stuff needed to interface with player
    //
    private: caddr_t playerIO; // ptr to shared memory for player I/O
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





















