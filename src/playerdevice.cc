///////////////////////////////////////////////////////////////////////////
//
// File: playerserver.cc
// Author: Richard Vaughan, Andrew Howard
// Date: 6 Dec 2000
// Desc: Provides interface to Player.
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/playerdevice.cc,v $
//  $Author: ahoward $
//  $Revision: 1.23.2.1 $
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

#include <stage.h>
#include "world.hh"
#include "playerdevice.hh"

///////////////////////////////////////////////////////////////////////////
// Macros
//
//#define DEBUG
#undef DEBUG
#undef VERBOSE

//#define NO_PLAYER_SPAWN

///////////////////////////////////////////////////////////////////////////
// Default constructor
//
CPlayerDevice::CPlayerDevice(CWorld *world, CEntity *parent )
        : CEntity(world, parent)
{
  // set the Player IO sizes correctly for this type of Entity
  m_data_len    = sizeof( int ); //PLAYER_DATA_SIZE;
  m_command_len = 0; //PLAYER_COMMAND_SIZE;
  m_config_len  = 0; //PLAYER_CONFIG_SIZE;
     
  m_player_type = PLAYER_PLAYER_CODE; // from player's messages.h
  m_stage_type = PlayerType;
  m_color_desc = PLAYER_COLOR;

  m_size_x = 0.12; // estimated USC PlayerBox ("whitebox") size
  m_size_y = 0.12;

  m_interval = 1.0;

  player_pid = 0;
}


///////////////////////////////////////////////////////////////////////////
// Destructor
//
CPlayerDevice::~CPlayerDevice( void )
{
}


///////////////////////////////////////////////////////////////////////////
// Start the device
//
bool CPlayerDevice::SetupIOPointers( char* io )
{
  if( !CEntity::SetupIOPointers( io ) )
    return false;
  
    // Startup player
   //
  //if (!StartupPlayer( m_player_port )) return false;
  
  return true;
}


///////////////////////////////////////////////////////////////////////////
// Shutdown the devices
//
void CPlayerDevice::Shutdown()
{
    CEntity::Shutdown();
    // Shutdown player
    //
    ShutdownPlayer();
}


///////////////////////////////////////////////////////////////////////////
// Update the robot 
//
void CPlayerDevice::Update( double sim_time )
{
#ifdef DEBUG
  CEntity::Update( sim_time ); // inherit debug output
#endif
}


///////////////////////////////////////////////////////////////////////////
// Start player instance
//
int CPlayerDevice::StartupPlayer(int count)
{
  int fds[2];
  fds[0] = fds[1] = -1;
 
#ifdef DEBUG
  cout << "StartupPlayer()" << endl;
#endif

#ifndef NO_PLAYER_SPAWN
  
  // we'll pipe the list of ports to Player; create the pipe here
  if(pipe(fds) == -1)
  {
    perror("CPlayerDevice::StartupPlayer(): pipe() failed");
    return(-1);
  }

  // ----------------------------------------------------------------------
  // fork off a player process to handle robot I/O
  if( (player_pid = fork()) < 0 )
  {
    cerr << "fork error creating robots" << flush;
    return(-1);
  }
  else
  {
    if( player_pid == 0 ) // new child process
    {
      // pass in the number or ports (they will come in on the pipe)
      char portBuf[32];
      sprintf( portBuf, "%d", (int) count );

      // BPG
      // release controlling tty so Player doesn't get signals
      setpgrp();
      // GPB

      // redirect our standard in
      if(dup2(fds[0],0) == -1)
      {
        perror("CPlayerDevice::StartupPlayer(): dup2() failed");
        kill(getppid(),SIGINT);
        exit(-1);
      }

      // we assume Player is in the current path
      if( execlp( "player", "player",
                  "-ports", portBuf, 
                  "-stage", m_world->PlayerIOFilename(), 
                  (strlen(m_world->m_auth_key)) ? "-key" : NULL,
                  (strlen(m_world->m_auth_key)) ? m_world->m_auth_key : NULL,
                  NULL) < 0 )
      {
        cerr << "execlp failed: make sure Player can be found"
                " in the current path."
                << endl;
        if(!kill(getppid(),SIGINT))
          perror("shit! even kill() errored");
        exit(-1);
      }
    }
    else
    {
#ifdef DEBUG
      printf("forked player with pid: %d\n", player_pid);
#endif
    }

  }


#endif

#ifdef DEBUG
  cout << "StartupPlayer() done" << endl;
#endif
    return(fds[1]);
}


///////////////////////////////////////////////////////////////////////////
// Stop player instance
//
void CPlayerDevice::ShutdownPlayer()
{
  int wait_retval;
  // BPG
  if(player_pid)
  {
    if(kill(player_pid,SIGTERM))
        perror("CPlayerDevice::~CPlayerDevice(): kill() failed sending SIGINT to Player");
    if((wait_retval = waitpid(player_pid,NULL,0)) == -1)
        perror("CPlayerDevice::~CPlayerDevice(): waitpid() returned an error");
    /*
    else if(!wait_retval)
    {
      fputs("Warning: killing Player by force\n",stderr);
      if(kill(player_pid,SIGKILL))
        perror("CPlayerDevice::~CPlayerDevice(): kill() failed sending SIGKILL to Player");
    }
    */
    // GPB
  }

}

#ifdef INCLUDE_RTK

///////////////////////////////////////////////////////////////////////////
// Process GUI update messages
//
void CPlayerDevice::OnUiUpdate(RtkUiDrawData *data)
{
    CEntity::OnUiUpdate(data);
}


///////////////////////////////////////////////////////////////////////////
// Process GUI mouse messages
//
void CPlayerDevice::OnUiMouse(RtkUiMouseData *pData)
{
    CEntity::OnUiMouse(pData);;
}

#endif

