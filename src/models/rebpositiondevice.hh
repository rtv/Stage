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
 * Desc: Simulates the UMass UBot, a differential robot.  Adapted from positiondevice.cc.
 * Author: John Sweeney
 * Date: 2002
 * CVS info: $Id: rebpositiondevice.hh,v 1.1 2003-06-10 03:29:21 jazzfunk Exp $
 */

#ifndef REBPOSITIONDEVICE_H
#define REBPOSITIONDEVICE_H

#include "stage.h"
#include "playerdevice.hh"

class CREBPositionDevice : public CPlayerEntity
{
public:
  // Minimal constructor
  CREBPositionDevice(LibraryItem *libit, CWorld *world, CEntity *parent );
  
  // a static named constructor - a pointer to this function is given
  // to the Library object and paired with a string.  When the string
  // is seen in the worldfile, this function is called to create an
  // instance of this entity
  static CREBPositionDevice* Creator( LibraryItem *libit, CWorld *world, CEntity *parent ) {
    return( new CREBPositionDevice( libit, world, parent ) );
  }
    
  // Startup routine
  virtual bool Startup();

  // Update the device
  virtual void Update( double sim_time );

  double GetX() { return odo_px; }
  double GetY() { return odo_py; }
  double GetTheta() { return odo_pth; }
  
protected: 
  // Move the robot
  int Move(); 
  
private:
  // Process configuration requests.
  void UpdateConfig();
  
  // Extract command from the command buffer
  void UpdateCommand();
				    
  // Compose the reply packet
  void UpdateData();

  // MEMBER DATA
protected:
  // Commanded robot speed
  double com_vr, com_vth;

  // Odometric pose
  double odo_px, odo_py, odo_pth;
  
  // Stall flag set if robot is in collision
  unsigned char stall;

  // are we in velocity or position mode?
  bool position_mode; 

  // are we doing heading PD controller or direct?
  bool heading_pd_control;

  bool motors_enabled;

private:
  // Timings
  double last_time;

  // Current command and data buffers
  player_position_cmd_t cmd;
  player_position_data_t data;

#ifdef INCLUDE_RTK2
  // Initialise the rtk gui
  protected: virtual void RtkStartup();
#endif

};

#endif
