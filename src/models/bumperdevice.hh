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
 * Desc: Simulates a set of binary bumper sensors that can be used as
 * bumpers and whiskers. bumpers are triggered by obstacles.
 * Author: Richard Vaughan 
 * Date: 30 Oct 2002 
 * CVS info: $Id: bumperdevice.hh,v 1.1 2002/10/27
 * 21:46:13 rtv Exp $
 */


#ifndef BUMPERDEVICE_HH
#define BUMPERDEVICE_HH

#include "playerdevice.hh"
#include "world.hh"
#include "library.hh"

/*  defines the params for a bumper */
/* this is a mirror of the player packet, but in stage native formats */
typedef struct 
{
  /* the local pose of a single bumper in meters */
  double x_offset, y_offset, th_offset;   
  /* length of the sensor in meters.  */
  double length; 
  /* radius of curvature - zero for straight lines */
  double radius; 
} stage_bumper_definition_t;


class CBumperDevice : public CPlayerEntity
{
  // Default constructor
  public: CBumperDevice( LibraryItem *libit, CWorld *world, CEntity *parent);
  
  // a static named constructor - a pointer to this function is given
  // to the Library object and paired with a string.  When the string
  // is seen in the worldfile, this function is called to create an
  // instance of this entity
public: static CBumperDevice* Creator( LibraryItem *libit,
				      CWorld *world, CEntity *parent )
  {
    return( new CBumperDevice( libit, world, parent ) );
  }
    
  // Startup routine
  public: virtual bool Startup();
  
  // Load the entity from the world file
  public: virtual bool Load(CWorldFile *worldfile, int section);

  // Update the device
  public: virtual void Update(double sim_time);

  // Process configuration requests.
  private: void UpdateConfig();
    
  public: int bumper_count;
  // Array holding the bumper configurations
public: stage_bumper_definition_t bumpers[ PLAYER_BUMPER_MAX_SAMPLES ];
  
  // a similer array, but with coords transformed for raytracing
  // and in native stage coords
public: stage_bumper_definition_t scanlines[  PLAYER_BUMPER_MAX_SAMPLES ];

#ifdef INCLUDE_RTK2

  // Initialise the rtk gui
  protected: virtual void RtkStartup();

  // Finalise the rtk gui
  protected: virtual void RtkShutdown();

  // Update the rtk gui
  protected: virtual void RtkUpdate();

protected: void RtkRenderBumper( rtk_fig_t* fig, int bumper );


  // For drawing the bumper beams
  private: rtk_fig_t *scan_fig;
#endif

};

#endif // BUMPERDEVICE_HH

