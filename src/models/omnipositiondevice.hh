///////////////////////////////////////////////////////////////////////////
//
// File: omniposition.hh
// Author: Andrew Howard
// Date: 19 Oct 2001
// Desc: Simulates an omni-robot
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/models/omnipositiondevice.hh,v $
//  $Author: gerkey $
//  $Revision: 1.2.4.1 $
//
// Usage:
//  (empty)
//
// Theory of operation:
//  (empty)
//
// Known bugs:
//  (empty)
//
// Possible enhancements:
//  (empty)
//
///////////////////////////////////////////////////////////////////////////

#ifndef OMNIPOSITIONDEVICE_H
#define OMNIPOSITIONDEVICE_H

#include "stage1p3.h"
#include "playerdevice.hh"

class COmniPositionDevice : public CPlayerEntity
{
  // Minimal constructor
  public: COmniPositionDevice( LibraryItem *libit, CWorld *world, CEntity *parent);
    
  // a static named constructor - a pointer to this function is given
  // to the Library object and paired with a string.  When the string
  // is seen in the worldfile, this function is called to create an
  // instance of this entity
public: static COmniPositionDevice* Creator(  LibraryItem *libit, CWorld *world, CEntity *parent )
  { return( new COmniPositionDevice( libit, world, parent ) ); }

  // Startup routine
  public: virtual bool Startup();
  
  // Update the device
  public: virtual void Update( double sim_time );

  // Move the device
  private: void Move();

  // Extract command from the command buffer
  private: void ParseCommandBuffer( void );
				    
  // Compose the reply packet
  private: void ComposeData( void );

  // Timings
  private: double last_time;

  // Current command and data buffers
  private: player_position_cmd_t command;
  private: player_position_data_t data;

  // Commanded velocities
  private: double com_vx, com_vy, com_va;
    
  // Stall flag -- set if robot is stalled
  private: int stall;

  // Current odometry values
  private: double odo_px, odo_py, odo_pa;
};

#endif
