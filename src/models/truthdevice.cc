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
 * Desc: A device for getting the true pose of things.
 * Author: Andrew Howard
 * Date: 6 Jun 2002
 * CVS info: $Id: truthdevice.cc,v 1.2.4.1.2.3 2004-11-15 16:42:13 gerkey Exp $
 */

#include "world.hh"
#include "truthdevice.hh"

///////////////////////////////////////////////////////////////////////////
// Default constructor
CTruthDevice::CTruthDevice(LibraryItem* libit,CWorld *world, CEntity *parent)
    : CPlayerEntity(libit,world, parent)
{
  m_data_len = sizeof(player_truth_data_t);
  m_command_len = 0;
  m_config_len  = 10;
  m_reply_len  = 10;

  m_player.code = PLAYER_TRUTH_CODE;
}

///////////////////////////////////////////////////////////////////////////
// Startup routine
//
bool CTruthDevice::Startup()
{
  if (!CPlayerEntity::Startup())
    return false;

  return true;
}

///////////////////////////////////////////////////////////////////////////
// Update the truth device
void CTruthDevice::Update(double sim_time)
{
  if (!Subscribed())
    return;

  UpdateConfig();
  UpdateData();
}


///////////////////////////////////////////////////////////////////////////
// Update the truth device
void CTruthDevice::UpdateConfig()
{
  int len;
  double px, py, pa;
  void* client;

  player_truth_pose_t config;
  if ((len = GetConfig(&client, &config, sizeof(config))) > 0)
  {
    switch (config.subtype)
      {
      case PLAYER_TRUTH_GET_POSE:
	
        GetGlobalPose(px, py, pa);
        config.pos[0] = htonl((int)rint((px-((m_world->GetWidth()/2.0)/m_world->ppm))*1e3));
        config.pos[1] = htonl((int)rint((py-((m_world->GetHeight()/2.0)/m_world->ppm))*1e3));
        config.pos[2] = 0;
        config.rot[0] = 0;
        config.rot[1] = 0;
        config.rot[2] = htonl((int)rint(NORMALIZE(pa)*1e3));
	
        PutReply(client, PLAYER_MSGTYPE_RESP_ACK, NULL, &config, sizeof(config));
        break;


      case PLAYER_TRUTH_SET_POSE_ON_ROOT:	
      case PLAYER_TRUTH_SET_POSE:
	
	// if it's to be put on root, reparent this model
	if( config.subtype == PLAYER_TRUTH_SET_POSE_ON_ROOT )
	  m_parent_entity->SetParent( m_world->root );

	// in either case, we move the model's pose
        if (len < (int)sizeof(config))
        {
          PRINT_WARN2("unexpected packet len (%d < %d)", len, sizeof(config));
          break;
        }

        px = ((int)ntohl(config.pos[0]))/1e3 + 
                ((m_world->GetWidth()/2.0)/m_world->ppm);
        py = ((int)ntohl(config.pos[1]))/1e3 +
                ((m_world->GetHeight()/2.0)/m_world->ppm);
        pa = ((int)ntohl(config.rot[2]))*1e3;

        // Move our parent.  Should possibly move the top level
        // ancestor.
        if (m_parent_entity)
        {
          m_parent_entity->SetGlobalPose(px, py, pa);
          m_parent_entity->SetDirty(1);
        }
        else
        {
          SetGlobalPose(px, py, pa);
          SetDirty(1);
        }

        PutReply(client, PLAYER_MSGTYPE_RESP_ACK);
        break;

      default:
        PRINT_WARN1("unrecognized request [%d]", config.subtype);
        break;
    }
  }
}


///////////////////////////////////////////////////////////////////////////
// Update data
void CTruthDevice::UpdateData()
{
  double px, py, pa;

  player_truth_data_t data;

  GetGlobalPose(px, py, pa);
  data.pos[0] = htonl((int)rint((px-((m_world->GetWidth()/2.0)/m_world->ppm))*1e3));
  data.pos[1] = htonl((int)rint((py-((m_world->GetHeight()/2.0)/m_world->ppm))*1e3));
  data.pos[2] = 0;
  data.rot[0] = 0;
  data.rot[1] = 0;
  data.rot[2] = htonl((int)rint(NORMALIZE(pa)*1e3));
  
  PutData(&data, sizeof(data));
}
