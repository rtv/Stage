/*
 *  Player - One Hell of a Robot Server
 *  Copyright (C) 2004, 2005 Richard Vaughan
 *                      
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
 * Desc: Speech player driver
 * Author: Pooya Karimian
 * Date: 21 March 2006
 * CVS: $Id$
 */


#include "p_driver.h"
using namespace Stg;

/** @addtogroup player
@par Speech interface
- PLAYER_SPEECH_CMD_SAY
*/


InterfaceSpeech::InterfaceSpeech( player_devaddr_t addr, 
                              StgDriver* driver,
                              ConfigFile* cf,
                              int section )
  
  : InterfaceModel( addr, driver, cf, section, "" )
{
  // nothing to do
}

void InterfaceSpeech::Publish( void )
{
/*
  speech_data_t* sdata = (speech_data_t*)mod->data;
  assert(sdata);

  player_speech_cmd_t pdata;
  memset( &pdata, 0, sizeof(pdata) );

  // Translate the Stage-formatted sdata into the Player-formatted pdata
  
  // Publish it
  this->driver->Publish(this->addr, NULL,
			PLAYER_MSGTYPE_DATA,
			PLAYER_SPEECH_CMD_SAY,
			(void*)&pdata, sizeof(pdata), NULL);
*/			
}

int InterfaceSpeech::ProcessMessage( QueuePointer & resp_queue,
									 player_msghdr_t* hdr,
									 void* data )
{
// PROCESS INCOMING REQUESTS HERE
	if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_CMD, 
				   PLAYER_SPEECH_CMD_SAY, 
				   this->addr))
	{
//		if( hdr->size == sizeof(player_speech_cmd_t) )
//		{
			player_speech_cmd_t* pcmd = (player_speech_cmd_t*)data;

			// Pass it to stage:

			mod->Say( pcmd->string );

			return( 0 );
//		}
//		else
//		{
//			PRINT_ERR2( "wrong size speech command packet (%d/%d bytes)",
//					  (int)hdr->size, (int)sizeof(player_speech_cmd_t) );
//
//			return( -1 );
//		}
	}

	PRINT_WARN2( "stage speech doesn't support message id:%d/%d",
				 hdr->type, hdr->subtype );
	return ( -1 );
}
