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
 * Desc: Simulates a noisy IR ring
 * Author: John Sweeney, copied from sonardevice.hh
 * Date: 22 Aug 2002
 * CVS info: $Id: irdevice.hh,v 1.1 2003-06-10 03:29:21 jazzfunk Exp $
 */

#ifndef _IRDEVICE_HH_
#define _IRDEVICE_HH_

#include "playerdevice.hh"
#include "library.hh"

#define IRSAMPLES PLAYER_IR_MAX_SAMPLES

#define IR_M_PARAM 0
#define IR_B_PARAM 1


class CIRDevice : public CPlayerEntity
{
public:
  // Default constructor
  CIRDevice(LibraryItem *libit, CWorld *world, CEntity *parent);

  // static named constructor, given to Library object with a string
  // when we see the string in the worldfile, call this function
  // to create an instance
  static CIRDevice *Creator(LibraryItem *libit, CWorld *world, CEntity *parent) {
    return new CIRDevice(libit, world, parent);
  }

  // startup routine
  virtual bool Startup();

  // Load the entity from the world file
  virtual bool Load(CWorldFile *worldfile, int section);

  // Update the device
  virtual void Update(double sim_time);

private:
  // Process configuration requests.
  void UpdateConfig();
    
#ifdef INCLUDE_RTK2
protected:
  // Initialise the rtk gui
  virtual void RtkStartup();

  // Finalise the rtk gui
  virtual void RtkShutdown();

  // Update the rtk gui
  virtual void RtkUpdate();
  
  // For drawing the sonar beams
  rtk_fig_t *scan_fig;
#endif

private:
  // Maximum range of sonar in meters
  double min_range;
  double max_range;
  
  // Array holding the sonar poses
  int ir_count;
  double irs[IRSAMPLES][3];

  // IR power state
  bool ir_power;

  // do we add noise?
  bool add_noise;

  // Arrays holding the regression parameters for
  // standard deviation estimation
  double sparams[2];

  // Structure holding the ir data
  player_ir_data_t data;

};

#endif

