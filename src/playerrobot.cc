///////////////////////////////////////////////////////////////////////////
//
// File: playerrobot.cc
// Author: Richard Vaughan, Andrew Howard
// Date: 6 Dec 2000
// Desc: Provides interface to Player.
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/playerrobot.cc,v $
//  $Author: vaughan $
//  $Revision: 1.1.2.15 $
//
// Usage:
//  (empty)
//
// Theory of operation:
//  (empty)
//
// Known bugs:
//  - All robots are creating a semaphore with the same key.  This doesnt
//    appear to be a problem, but should be investigates. ahoward
//
// Possible enhancements:
//  (empty)
//
///////////////////////////////////////////////////////////////////////////

#include <errno.h>
#include <fcntl.h>
#include <iomanip.h>
#include <iostream.h>
#include <math.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strstream.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <iomanip.h>
#include <sys/mman.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/sem.h>

#define RTK_ENABLE_TRACE 0

#include <stage.h>
#include "world.hh"


///////////////////////////////////////////////////////////////////////////
// Macros
//
#define SEMKEY 2000


///////////////////////////////////////////////////////////////////////////
// Default constructor
//
CPlayerRobot::CPlayerRobot(CWorld *world, CEntity *parent)
        : CEntity(world, parent)
{
    m_port = 6665;
    playerIO = NULL;

    exp.objectType = player_o;
    exp.data = (char*)0;
    strcpy( exp.label, "Player Robot" );
}


///////////////////////////////////////////////////////////////////////////
// Destructor
//
CPlayerRobot::~CPlayerRobot( void )
{
}


///////////////////////////////////////////////////////////////////////////
// Start all the devices
//
bool CPlayerRobot::Startup()
{
    if (!CEntity::Startup())
        return false;
 
    // Create the lock object for the shared mem
    //
    if (!CreateShmemLock())
        return false;
    
    // Startup player
    // This will generate the memory map, so it must be done first
    //
    if (!StartupPlayer(m_port))
        return false;

    return true;
}


///////////////////////////////////////////////////////////////////////////
// Shutdown the devices
//
void CPlayerRobot::Shutdown()
{
    // Shutdown player
    //
    ShutdownPlayer();

    CEntity::Shutdown();
}


///////////////////////////////////////////////////////////////////////////
// Update the robot 
//
void CPlayerRobot::Update()
{
    CEntity::Update();
}


///////////////////////////////////////////////////////////////////////////
// Start player instance
//
bool CPlayerRobot::StartupPlayer(int port)
{
    // -- create the memory map for IPC with Player --------------------------

    // amount of memory to reserve per robot for Player IO
    size_t areaSize = TOTAL_SHARED_MEMORY_BUFFER_SIZE;

    // make a unique temporary file for shared mem
    strcpy( tmpName, "/tmp/playerIO.XXXXXX" );
    int tfd = mkstemp( tmpName );

    // make the file the right size
    if( ftruncate( tfd, areaSize ) < 0 )
    {
        perror( "Failed to set file size" );
        return false;
    }

    playerIO = (caddr_t)mmap( NULL, areaSize,
                              PROT_READ | PROT_WRITE, MAP_SHARED, tfd, (off_t) 0);
    if (playerIO == MAP_FAILED )
    {
        perror( "Failed to map memory" );
        return false;
    }

    close( tfd ); // can close fd once mapped

    //cout << "Mapped area: " << (unsigned int)playerIO << endl;

    // Initialise entire space
    //
    memset(playerIO, 0, areaSize);
    
    // test the memory space
    // Player will try to read this data
    //
    for( int t=0; t < TEST_TOTAL_BUFFER_SIZE; t++ )
        playerIO[TEST_DATA_START + t] = (unsigned char) t;

    // ----------------------------------------------------------------------
    // fork off a player process to handle robot I/O
    if( (player_pid = fork()) < 0 )
    {
        cerr << "fork error creating robots" << flush;
        return false;
    }
    else
    {
        // BPG
        //if( pid == 0 ) // new child process
        // GPB
        if( player_pid == 0 ) // new child process
        {
            // create player port number for command line
            char portBuf[32];
            sprintf( portBuf, "%d", (int) port );

            // BPG
            // release controlling tty so Player doesn't get signals
            setpgrp();
            // GPB

            // we assume Player is in the current path
            if( execlp( "player", "player",
                        "-gp", portBuf, "-stage", tmpName, (char*) 0) < 0 )
            {
                cerr << "execlp failed: make sure Player can be found"
                    " in the current path."
                     << endl;
                return false;
            }
        }
        //else
        //printf("forked player with pid: %d\n", player_pid);
    }
    return true;
}


///////////////////////////////////////////////////////////////////////////
// Stop player instance
//
void CPlayerRobot::ShutdownPlayer()
{
    // BPG
    if(kill(player_pid,SIGINT))
        perror("CPlayerRobot::~CPlayerRobot(): kill() failed sending SIGINT to Player");
    if(waitpid(player_pid,NULL,0) == -1)
        perror("CPlayerRobot::~CPlayerRobot(): waitpid() returned an error");
    // GPB

    // delete the playerIO.xxxxxx file
    remove( tmpName );
}


///////////////////////////////////////////////////////////////////////////
// Create a single semaphore to sync access to the shared memory segments
//
bool CPlayerRobot::CreateShmemLock()
{
    semKey = SEMKEY;

    union semun
    {
        int val;
        struct semid_ds *buf;
        ushort *array;
    } argument;

    argument.val = 0; // initial semaphore value
    semid = semget( semKey, 1, 0666 | IPC_CREAT );

    if( semid < 0 ) // semget failed
    {
        printf( "Unable to create semaphore\n" );
        return false;
    }
    if( semctl( semid, 0, SETVAL, argument ) < 0 )
    {
        printf( "Failed to set semaphore value\n" );
        return false;
    }
    return true;
}


///////////////////////////////////////////////////////////////////////////
// lock the shared mem
// Returns a pointer to the memory
//
bool CPlayerRobot::LockShmem( void )
{
  struct sembuf ops[1];

  ops[0].sem_num = 0;
  ops[0].sem_op = 1;
  ops[0].sem_flg = 0;

  int retval = semop( semid, ops, 1 );
  if (retval != 0)
  {
      printf("lock failed return value = %d\n", (int) retval);
      return false;
  }
  return true;
}


///////////////////////////////////////////////////////////////////////////
// unlock the shared mem
//
void CPlayerRobot::UnlockShmem( void )
{
  struct sembuf ops[1];

  ops[0].sem_num = 0;
  ops[0].sem_op = -1;
  ops[0].sem_flg = 0;

  int retval = semop( semid, ops, 1 );
  if (retval != 0)
      printf("unlock failed return value = %d\n", (int) retval);
}

#ifdef INCLUDE_RTK

///////////////////////////////////////////////////////////////////////////
// Process GUI update messages
//
void CPlayerRobot::OnUiUpdate(RtkUiDrawData *pData)
{
    CEntity::OnUiUpdate(pData);
}


///////////////////////////////////////////////////////////////////////////
// Process GUI mouse messages
//
void CPlayerRobot::OnUiMouse(RtkUiMouseData *pData)
{
 CEntity::OnUiMouse(pData);;
}

#endif
