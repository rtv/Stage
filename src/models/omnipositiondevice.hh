///////////////////////////////////////////////////////////////////////////
//
// File: omniposition.hh
// Author: Andrew Howard
// Date: 19 Oct 2001
// Desc: Simulates an omni-robot
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/models/omnipositiondevice.hh,v $
//  $Author: rtv $
//  $Revision: 1.3 $
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

#include "stage.h"
#include "playerdevice.hh"

typedef enum{ VELOCITY_MODE, POSITION_MODE } stage_move_mode_t;

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

  // Move the device according to the current velocities
  private: void Move();

  // computes velocities that'll servo towards the goal position
  private: void PositionControl();

  // Process configuration requests.
  private: void UpdateConfig();

  // Extract command from the command buffer
  private: void ParseCommandBuffer( void );
				    
  // Compose the reply packet
  private: void ComposeData( void );

  // when false, Move() is never called and the robot doesn't move
  bool motors_enabled;
  
  // when true x, y, a axes work independently, when false, we model a
  // differential-steer robot (like a Pioneer 2 or a tank).
  bool enable_omnidrive; 

  // movement mode - VELOCITY_MODE or POSITION_MODE
  stage_move_mode_t move_mode;
  
  // Timings
  private: double last_time;

  // Current command and data buffers
  private: player_position_cmd_t command;
  private: player_position_data_t data;

  // Commanded velocities (for velocity control)
  private: double com_vx, com_vy, com_va;
    
  // Commanded pose (for position control)
  private: double com_px, com_py, com_pa;

  // Stall flag -- set if robot is stalled
  private: int stall;

  // Current odometry values
  private: double odo_px, odo_py, odo_pa;
};

#endif
