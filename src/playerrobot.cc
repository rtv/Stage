///////////////////////////////////////////////////////////////////////////
//
// File: playerrobot.cc
// Author: Richard Vaughan, Andrew Howard
// Date: 6 Dec 2000
// Desc: Provides interface to Player.
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/playerrobot.cc,v $
//  $Author: ahoward $
//  $Revision: 1.1.2.5 $
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

#define ENABLE_TRACE 0

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

#include <offsets.h>
#include "world.hh"


///////////////////////////////////////////////////////////////////////////
// Macros
//
#define SEMKEY 2000


///////////////////////////////////////////////////////////////////////////
// Default constructor
//
CPlayerRobot::CPlayerRobot(CWorld *world, CObject *parent)
        : CObject(world, parent)
{
    playerIO = NULL;

    #ifdef INCLUDE_RTK
        m_mouse_radius = 0.6;
        m_draggable = true;
        m_show_sensors = false;
    #endif
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
bool CPlayerRobot::Startup(RtkCfgFile *cfg)
{
    TRACE0("starting devices");

    if (!CObject::Startup(cfg))
        return false;
    
    cfg->BeginSection(m_id);
    
    // Get the port for this robot
    //
    int port = cfg->ReadInt("port", 6666, "");
    
    cfg->EndSection();

    // Create the lock object for the shared mem
    //
    if (!CreateShmemLock())
        return false;
    
    // Startup player
    // This will generate the memory map, so it must be done first
    //
    if (!StartupPlayer(port))
        return false;
    
    return true;
}


///////////////////////////////////////////////////////////////////////////
// Shutdown the devices
//
void CPlayerRobot::Shutdown()
{
    TRACE0("shutting down devices");

    // Shutdown player
    //
    ShutdownPlayer();

    CObject::Shutdown();
}


///////////////////////////////////////////////////////////////////////////
// Update the robot 
//
void CPlayerRobot::Update()
{
    CObject::Update();
}


///////////////////////////////////////////////////////////////////////////
// Start player instance
//
bool CPlayerRobot::StartupPlayer(int port)
{
    TRACE0("starting player");
    
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

    // test the memory space
    for( int t=0; t<64; t++ ) playerIO[t] = (unsigned char)t;

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
    TRACE0("stopping player");
    
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
    TRACE0("making semaphore");

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
        MSG( "Unable to create semaphore" );
        return false;
    }
    if( semctl( semid, 0, SETVAL, argument ) < 0 )
    {
        MSG( "Failed to set semaphore value" );
        return false;
    }
    return true;
}


///////////////////////////////////////////////////////////////////////////
// Lock the shared mem
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
      MSG1("lock failed return value = %d", (int) retval);
      return false;
  }
  return true;
}


///////////////////////////////////////////////////////////////////////////
// Unlock the shared mem
//
void CPlayerRobot::UnlockShmem( void )
{
  struct sembuf ops[1];

  ops[0].sem_num = 0;
  ops[0].sem_op = -1;
  ops[0].sem_flg = 0;

  int retval = semop( semid, ops, 1 );
  if (retval != 0)
      MSG1("unlock failed return value = %d", (int) retval);
}

#ifdef INCLUDE_RTK

///////////////////////////////////////////////////////////////////////////
// Process GUI update messages
//
void CPlayerRobot::OnUiUpdate(RtkUiDrawData *pData)
{
    CObject::OnUiUpdate(pData);
}


///////////////////////////////////////////////////////////////////////////
// Process GUI mouse messages
//
void CPlayerRobot::OnUiMouse(RtkUiMouseData *pData)
{
    CObject::OnUiMouse(pData);

    pData->BeginSection("global", "object");
    
    // Default process for any "move" modes
    //
    if (pData->UseMouseMode("move"))
    {
        if (IsMouseReady())
        {
            // Toggle sensor state on middle button
            //
            if (pData->IsButtonDown() && pData->WhichButton() == 2)
            {
                m_show_sensors = !m_show_sensors;
                TRACE1("sensors %d", (int) m_show_sensors);
            }
        }
    }

    pData->EndSection();
}

#endif
