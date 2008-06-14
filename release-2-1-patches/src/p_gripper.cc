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
 * Desc: A plugin driver for Player that gives access to Stage devices.
 * Author: Richard Vaughan
 * Date: 10 December 2004
 * CVS: $Id: p_gripper.cc,v 1.10 2007-09-11 01:14:19 rtv Exp $
 */


#include "p_driver.h"

//#include "playerclient.h" // for the dumb pioneer gripper command defines

/** @addtogroup player
@par Gripper interface
- PLAYER_GRIPPER_DATA_STATE
- PLAYER_GRIPPER_CMD_OPEN
- PLAYER_GRIPPER_CMD_CLOSE
- PLAYER_GRIPPER_REQ_GET_GEOM
*/

extern "C" {
int gripper_init( stg_model_t* mod );
}

InterfaceGripper::InterfaceGripper( player_devaddr_t addr,
				    StgDriver* driver,
				    ConfigFile* cf,
				    int section )

  : InterfaceModel( addr, driver, cf, section, gripper_init )
{
  // nothing to do
}

void InterfaceGripper::Publish( void )
{
  stg_gripper_data_t* sdata = (stg_gripper_data_t*)mod->data;
  assert(sdata);

  player_gripper_data_t pdata;
  memset( &pdata, 0, sizeof(pdata) );

  // set the proper bits
  pdata.beams = 0;
  pdata.beams |=  sdata->outer_break_beam ? 0x01 : 0x00;
  pdata.beams |=  sdata->inner_break_beam ? 0x02 : 0x00;
//   pdata.beams |=  sdata->outer_break_beam ? 0x04 : 0x00;
//   pdata.beams |=  sdata->inner_break_beam ? 0x08 : 0x00;

  if (sdata->paddles == STG_GRIPPER_PADDLE_OPEN)
    pdata.state = PLAYER_GRIPPER_STATE_OPEN;
  else if (sdata->paddles == STG_GRIPPER_PADDLE_CLOSED)
    pdata.state = PLAYER_GRIPPER_STATE_CLOSED;
  else if (sdata->paddles == STG_GRIPPER_PADDLE_OPENING ||
           sdata->paddles == STG_GRIPPER_PADDLE_CLOSING)
    pdata.state = PLAYER_GRIPPER_STATE_MOVING;

//   pdata.state = 0;
//   pdata.state |= (sdata->paddles == STG_GRIPPER_PADDLE_OPEN) ? 0x01 : 0x00;
//   pdata.state |= (sdata->paddles == STG_GRIPPER_PADDLE_CLOSED) ? 0x02 : 0x00;
//   pdata.state |= ((sdata->paddles == STG_GRIPPER_PADDLE_OPENING) ||
// 		  (sdata->paddles == STG_GRIPPER_PADDLE_CLOSING))  ? 0x04 : 0x00;
//
//   pdata.state |= (sdata->lift == STG_GRIPPER_LIFT_UP) ? 0x10 : 0x00;
//   pdata.state |= (sdata->lift == STG_GRIPPER_LIFT_DOWN) ? 0x20 : 0x00;
//   pdata.state |= ((sdata->lift == STG_GRIPPER_LIFT_UPPING) ||
// 		  (sdata->lift == STG_GRIPPER_LIFT_DOWNING))  ? 0x040 : 0x00;

  //pdata.state |= sdata->lift_error ? 0x80 : 0x00;
  //pdata.state |= sdata->gripper_error ? 0x08 : 0x00;

  //this->driver->PutData( this->id, &pdata, sizeof(pdata), NULL);

  // Write data
  this->driver->Publish(this->addr,
			PLAYER_MSGTYPE_DATA,
			PLAYER_GRIPPER_DATA_STATE,
			(void*)&pdata, sizeof(pdata), NULL);

}

int InterfaceGripper::ProcessMessage(QueuePointer &resp_queue,
				     player_msghdr_t* hdr,
                                      void* data)
{
  // Is it a new motor command?
  if (Message::MatchMessage (hdr, PLAYER_MSGTYPE_CMD, PLAYER_GRIPPER_CMD_OPEN, this->addr))
  {
    stg_gripper_cmd_t cmd;
    cmd.cmd = STG_GRIPPER_CMD_OPEN;
    cmd.arg = 0;
    stg_model_set_cmd( this->mod, &cmd, sizeof(cmd));
    return 0;
  }
  else if (Message::MatchMessage (hdr, PLAYER_MSGTYPE_CMD, PLAYER_GRIPPER_CMD_CLOSE, this->addr))
  {
    stg_gripper_cmd_t cmd;
    cmd.cmd = STG_GRIPPER_CMD_CLOSE;
    cmd.arg = 0;
    stg_model_set_cmd( this->mod, &cmd, sizeof(cmd));
    return 0;
  }
  else if (Message::MatchMessage (hdr, PLAYER_MSGTYPE_CMD, -1, this->addr))
  {
    PRINT_WARN1( "Stage: player gripper command %d is not implemented\n", hdr->subtype );
    return 0;
  }

/*  if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_CMD,
                           PLAYER_GRIPPER_CMD_STATE,
                           this->addr))
    {
      if( hdr->size == sizeof(player_gripper_cmd_t) )
	{
	  player_gripper_cmd_t* pcmd = (player_gripper_cmd_t*)data;

	  // Pass it to stage:
	  stg_gripper_cmd_t cmd;
	  cmd.cmd = STG_GRIPPER_CMD_NOP;
	  cmd.arg = 0;

	  switch( pcmd->cmd )
	    {
	    case GRIPclose: cmd.cmd = STG_GRIPPER_CMD_CLOSE; break;
	    case GRIPopen:  cmd.cmd = STG_GRIPPER_CMD_OPEN; break;
	    case LIFTup:    cmd.cmd = STG_GRIPPER_CMD_UP; break;
	    case LIFTdown:  cmd.cmd = STG_GRIPPER_CMD_DOWN; break;

	    default:
	      PRINT_WARN1( "Stage: player gripper command %d is not implemented\n",
			   pcmd->cmd );
	    }

	  //cmd.cmd  = pcmd->cmd;
	  cmd.arg = pcmd->arg;

	  stg_model_set_cmd( this->mod, &cmd, sizeof(cmd));
	}
      else
	PRINT_ERR2( "wrong size gripper command packet (%d/%d bytes)",
		    (int)hdr->size, (int)sizeof(player_gripper_cmd_t) );

      return 0;
    }*/

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
      pgeom.pose.pz = 0.0f;
      pgeom.pose.proll = 0.0f;
      pgeom.pose.ppitch = 0.0f;
      pgeom.pose.pyaw = pose.a;
      pgeom.outer_size.sw = geom.size.x;
      pgeom.outer_size.sl = geom.size.y;
      pgeom.inner_size.sw = geom.size.x;
      pgeom.inner_size.sl = geom.size.y;
      pgeom.num_beams = 2;
      pgeom.capacity = 0;

      this->driver->Publish(this->addr, resp_queue,
			    PLAYER_MSGTYPE_RESP_ACK,
			    PLAYER_GRIPPER_REQ_GET_GEOM,
			    (void*)&pgeom, sizeof(pgeom), NULL);
      return(0);


    }

  PRINT_WARN2( "stage gripper doesn't support message id:%d/%d",
	       hdr->type, hdr->subtype );
  return -1;
}

