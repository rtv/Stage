///////////////////////////////////////////////////////////////////////////
//
// File: playerserver.cc
// Author: Richard Vaughan, Andrew Howard
// Date: 6 Dec 2000
// Desc: Provides interface to Player.
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/playerdevice.cc,v $
//  $Author: gerkey $
//  $Revision: 1.29 $
//
// Usage:
//  (empty)
//
// Theory of operation:
//  (empty)
//
// Known bugs:

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


///////////////////////////////////////////////////////////////////////////
// Default constructor
CPlayerDevice::CPlayerDevice(CWorld *world, CEntity *parent )
        : CEntity(world, parent)
{
  // set the Player IO sizes correctly for this type of Entity
  m_data_len    = sizeof( int ); //PLAYER_DATA_SIZE;
  m_command_len = 0; //PLAYER_COMMAND_SIZE;
  m_config_len  = 0; //PLAYER_CONFIG_SIZE;
     
  m_player.code = PLAYER_PLAYER_CODE; // from player's messages.h
  m_stage_type = PlayerType;

  SetColor(PLAYER_COLOR);

  this->shape = ShapeRect;
  this->size_x = 0.12; // estimated USC PlayerBox ("whitebox") size
  this->size_y = 0.12;

  m_interval = 1.0;
}


///////////////////////////////////////////////////////////////////////////
// Destructor
CPlayerDevice::~CPlayerDevice( void )
{
}


///////////////////////////////////////////////////////////////////////////
// Setyp the object
bool CPlayerDevice::Startup()
{
    return CEntity::Startup();
}


///////////////////////////////////////////////////////////////////////////
// Shutdown the devices
void CPlayerDevice::Shutdown()
{
    CEntity::Shutdown();
}


/* REMOVE
///////////////////////////////////////////////////////////////////////////
// Start the device
bool CPlayerDevice::SetupIOPointers( char* io )
{
  if( !CEntity::SetupIOPointers( io ) )
    return false;
  
  return true;
}
*/


///////////////////////////////////////////////////////////////////////////
// Update the robot 
//
void CPlayerDevice::Update( double sim_time )
{
#ifdef DEBUG
  CEntity::Update( sim_time ); // inherit debug output
#endif
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

