///////////////////////////////////////////////////////////////////////////
//
// File: omniposition.hh
// Author: Andrew Howard
// Date: 19 Oct 2001
// Desc: Simulates an omni-robot
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/include/omnipositiondevice.hh,v $
//  $Author: ahoward $
//  $Revision: 1.1.2.2 $
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

class COmniPositionDevice : public CEntity
{
  // Minimal constructor
  public: COmniPositionDevice(CWorld *world, CEntity *parent);
    
  // Update the device
  public: virtual void Update( double sim_time );

  // Move the device
  private: void Move();

  // Extract command from the command buffer
  private: void ParseCommandBuffer( void );
				    
  // Compose the reply packet
  private: void ComposeData( void );

  // Check to see if the given pose will yield a collision
  private: bool InCollision(double px, double py, double pa);

  // Render the object in the world rep
  private: virtual void Map(double px, double py, double pa, bool render);

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

#ifdef INCLUDE_RTK

  // Process GUI update messages
  public: virtual void OnUiUpdate(RtkUiDrawData *data);

  // Process GUI mouse messages
  public: virtual void OnUiMouse(RtkUiMouseData *data);

  // Draw the chassis
  public: void DrawChassis(RtkUiDrawData *data);
  
#endif
};

#endif
