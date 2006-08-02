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
 * Desc: Audio player driver
 * Author: Pooya Karimian
 * Date: 24 April 2006
 * CVS: $Id $
 */


#include "p_driver.h"

typedef char audio_msg_t[STG_AUDIO_MAX_STRING_LEN];


/** @addtogroup player
@par Audio interface
- PLAYER_OPAQUE_CMD
- PLAYER_OPAQUE_DATA_STATE
*/

extern "C" {
    int audio_init(stg_model_t * mod);
} InterfaceAudio::InterfaceAudio(player_devaddr_t addr,
				 StgDriver * driver,
				 ConfigFile * cf, int section)

:InterfaceModel(addr, driver, cf, section, audio_init)
{
// TODO: alwayson?
//  driver->alwayson = TRUE;    
}

void InterfaceAudio::Publish(void)
{

    stg_audio_data_t *sdata = (stg_audio_data_t *) mod->data;
    assert(sdata);

    // check if there's new data to publish
    if (sdata->recv[0]==0)
	return ;
	
    player_opaque_data_t pdata;
    memset(&pdata, 0, sizeof(pdata));
    // Translate the Stage-formatted sdata into the Player-formatted pdata

    audio_msg_t *audio_msg;
//    pdata.data_count = sizeof(audio_msg_t);
    pdata.data_count = strlen(sdata->recv)+1;
    audio_msg = reinterpret_cast < audio_msg_t * >(pdata.data);

    //sprintf(audioMsgStruct, "%s", sdata->recv);
    strncpy((char *)audio_msg, sdata->recv, STG_AUDIO_MAX_STRING_LEN);

    // clear received message once sent to the client
    sdata->recv[0]=0;
//  uint size = sizeof(pdata) - sizeof(pdata.data) + pdata.data_count;

    // Publish it
    this->driver->Publish(this->addr, NULL,
			  PLAYER_MSGTYPE_DATA,
			  PLAYER_OPAQUE_DATA_STATE,
			  (void *) &pdata, sizeof(pdata), NULL);

}

int InterfaceAudio::ProcessMessage(MessageQueue * resp_queue,
				   player_msghdr_t * hdr, void *data)
{
    // PROCESS INCOMING REQUESTS HERE
//  if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_CMD, 
    if (Message::MatchMessage(hdr, PLAYER_MSGTYPE_CMD,
			      PLAYER_OPAQUE_CMD, this->addr)) {
	if (hdr->size == sizeof(player_opaque_data_t)) {
	    player_opaque_data_t *pcmd = (player_opaque_data_t *) data;

	    // Pass it to stage:
	    stg_audio_cmd_t cmd;
//        cmd.cmd = STG_SPEECH_CMD_NOP;
//        cmd.string[0] = 0;

	    cmd.cmd = STG_AUDIO_CMD_SAY;
//        strncpy(cmd.string, pcmd->string, STG_AUDIO_MAX_STRING_LEN);
	    strncpy(cmd.string, (char *) pcmd->data,
		    STG_AUDIO_MAX_STRING_LEN);
	    cmd.string[STG_AUDIO_MAX_STRING_LEN - 1] = 0;

	    stg_model_set_cmd(this->mod, &cmd, sizeof(cmd));
	} else
	    PRINT_ERR2("wrong size audio command packet (%d/%d bytes)",
		       (int) hdr->size,
		       (int) sizeof(player_opaque_data_t));

	return 0;
    }
/*
  // is it a geometry request?  
  if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ, 
                           PLAYER_GRIPPER_REQ_GET_GEOM,
                           this->addr))
    {
      // TODO: get pose in top-level model's CS instead.
      
      stg_geom_t geom;
      stg_model_get_geom( this->mod, &geom );
      
      stg_pose_t pose;
      stg_model_get_pose( this->mod, &pose);
      
      player_gripper_geom_t pgeom;
      pgeom.pose.px = pose.x;
      pgeom.pose.py = pose.y;
      pgeom.pose.pa = pose.a;      
      pgeom.size.sw = geom.size.y;
      pgeom.size.sl = geom.size.x;
      
      this->driver->Publish(this->addr, resp_queue,
			    PLAYER_MSGTYPE_RESP_ACK, 
			    PLAYER_GRIPPER_REQ_GET_GEOM,
			    (void*)&pgeom, sizeof(pgeom), NULL);
      return(0);

      
    }
*/
    PRINT_WARN2("stage audio doesn't support message id:%d/%d",
		hdr->type, hdr->subtype);
    return -1;

}
