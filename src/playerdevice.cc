/*
 *  Stage : a multi-robot simulator.
 *  Copyright (C) 2001, 2002 Richard Vaughan, Andrew Howard and Brian Gerkey.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */
/*
 * Desc: Player device; doesnt do much anymore.
 * Author: Richard Vaughan, Andrew Howard
 * Date: 7 Dec 2000
 * CVS info: $Id: playerdevice.cc,v 1.30 2002-06-07 06:30:52 inspectorg Exp $
 */

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
  m_data_len = sizeof( int ); //PLAYER_DATA_SIZE;
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


///////////////////////////////////////////////////////////////////////////
// Update the robot 
//
void CPlayerDevice::Update( double sim_time )
{
#ifdef DEBUG
  CEntity::Update( sim_time ); // inherit debug output
#endif
}


