///////////////////////////////////////////////////////////////////////////
//
// File: position.hh
// Author: Richard Vaughan, Andrew Howard
// Date: 19 Oct 2001
// Desc: Simulates a mobile robot with odometry and stall sensor. Supports
//       differential and omnidirectional drive modes.
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/models/positiondevice.hh,v $
//  $Author: inspectorg $
//  $Revision: 1.6 $
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

#ifndef POSITIONDEVICE_H
#define POSITIONDEVICE_H

#include "stage.h"
#include "playerdevice.hh"

// control mode
typedef enum{ VELOCITY_CONTROL_MODE, POSITION_CONTROL_MODE } 
stage_position_control_mode_t;

// drive mode
typedef enum{ DIFF_DRIVE_MODE, OMNI_DRIVE_MODE } 
stage_position_drive_mode_t;

class CPositionDevice : public CPlayerEntity
{
  // Minimal constructor
  public: CPositionDevice( LibraryItem *libit, CWorld *world, CEntity *parent);
    
  // a static named constructor - a pointer to this function is given
  // to the Library object and paired with a string.  When the string
  // is seen in the worldfile, this function is called to create an
  // instance of this entity
  public: static CPositionDevice* Creator(  LibraryItem *libit, CWorld *world, CEntity *parent )
    { return( new CPositionDevice( libit, world, parent ) ); }

  // load settings from worldfile
  bool Load(CWorldFile *worldfile, int section);

  // Startup routine
  public: virtual bool Startup();
  
  // Update the device
  public: virtual void Update( double sim_time );

  // Move the device according to the current velocities
  protected: void Move();

  // computes velocities that'll servo towards the goal position
  private: void PositionControl();

  // Process configuration requests.
  private: void UpdateConfig();

  // Extract command from the command buffer
  private: void ParseCommandBuffer( player_position_cmd_t* command );
				    
  // Compose the reply packet
  private: void ComposeData( player_position_data_t* data );

  // when false, Move() is never called and the robot doesn't move
  bool motors_enabled;
  
  // OMNI_MOVE_MODE = Omnidirectional drive - x y & th axes are
  // independent.  DIFF_MOVE_MODE = Differential-drive - x & th axes
  // are coupled, y axis disabled (like a Pioneer 2 or a tank).
  stage_position_drive_mode_t  drive_mode; 
  
  // movement mode - VELOCITY_CONTROL_MODE or POSITION_CONTROL_MODE
  stage_position_control_mode_t control_mode;

  // Odometric biases
  private: double odo_ex, odo_ey, odo_ea;
  
  // Timings
  private: double last_time;

  // Current command and data buffers
  //private: player_position_cmd_t command;
  //private: player_position_data_t data;

  // Commanded velocities (for velocity control)
  protected: double com_vx, com_vy, com_va;
    
  // Commanded pose (for position control)
  protected: double com_px, com_py, com_pa;

  // Stall flag -- set if robot is stalled
  protected: int stall;

  // Current odometry values
  protected: double odo_px, odo_py, odo_pa;

#ifdef INCLUDE_RTK2
  // Initialise the rtk gui
  protected: virtual void RtkStartup();
#endif

};

#endif
