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
 * Desc: Simulates a differential mobile robot.
 * Author: Andrew Howard, Richard Vaughan
 * Date: 5 Dec 2000
 * CVS info: $Id: positiondevice.hh,v 1.2 2002-08-30 18:17:28 rtv Exp $
 */

#ifndef POSITIONDEVICE_H
#define POSITIONDEVICE_H

#include "stage.h"
#include "playerdevice.hh"

class CPositionDevice : public CPlayerEntity
{
  // Minimal constructor
  public: CPositionDevice(CWorld *world, CEntity *parent );

  // a static named constructor - a pointer to this function is given
  // to the Library object and paired with a string.  When the string
  // is seen in the worldfile, this function is called to create an
  // instance of this entity
public: static CPositionDevice* Creator( CWorld *world, CEntity *parent )
  {
    return( new CPositionDevice( world, parent ) );
  }

  // Update the device
  public: virtual void Update( double sim_time );

  // Process configuration requests.
  private: void UpdateConfig();

  // Extract command from the command buffer
  private: void UpdateCommand();
				    
  // Compose the reply packet
  private: void UpdateData();

  // Move the robot
  protected: int Move(); 

  // Timings
  private: double last_time;

  // Current command and data buffers
  private: player_position_cmd_t cmd;
  private: player_position_data_t data;
    
  // Commanded robot speed
  protected: double com_vr, com_vth;

  // Odometric pose
  protected: double odo_px, odo_py, odo_pth;

  // Stall flag set if robot is in collision
  protected: unsigned char stall;

#ifdef INCLUDE_RTK2
  // Initialise the rtk gui
  protected: virtual void RtkStartup();
#endif

};

#endif
