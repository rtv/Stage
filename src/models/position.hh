///////////////////////////////////////////////////////////////////////////
//
// File: position.hh
// Author: Richard Vaughan, Andrew Howard
// Date: 19 Oct 2001
// Desc: Simulates a mobile robot with odometry and stall sensor. Supports
//       differential and omnidirectional drive modes.
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/models/position.hh,v $
//  $Author: rtv $
//  $Revision: 1.1.2.1 $
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

#include "entity.hh"

class CPositionModel : public CEntity
{
  // Minimal constructor
public: CPositionModel( int id, char* token, char* color,  CEntity *parent);
  
  // a static named constructor - a pointer to this function is given
  // to the Library object and paired with a string.  When the string
  // is seen in the worldfile, this function is called to create an
  // instance of this entity
public: static CPositionModel* Creator(  int id, char* token, 
					  char* color, CEntity *parent )
  { return( new CPositionModel( id, token, color, parent ) ); }
  
  // Update the device
  public: virtual int Update();

private: 
  // computes velocities that'll servo towards the goal position
  void PositionControl();
  
  // set the current velocities to the commanded velocities, subject
  // to the current steering rules
  void VelocityControl();
  
  // Process configuration requests.
private: int Property( int con, stage_prop_id_t property, 
		       void* value, size_t len, stage_buffer_t* reply );
  
  // STG_POSITION_STEER_DIFFERENTIAL or  STG_POSITION_STEER_INDEPENDENT
  stage_position_steer_mode_t  steer_mode; 
  
  //  STG_POSITION_CONTROL_VELOCITY or  STG_POSITION_CONTROL_POSITION
  stage_position_control_mode_t control_mode;
  
  // Timings
  private: double last_time;

  // Current command and data buffers
  //private: player_position_cmd_t command;
  //private: player_position_data_t data;

  // Commanded velocities (for velocity control)
  private: double com_vx, com_vy, com_va;
    
  // Commanded pose (for position control)
  private: double com_px, com_py, com_pa;

  // Stall flag -- set if robot is stalled
  private: int stall;

  // Current odometry values
  private: double odo_px, odo_py, odo_pa;

#ifdef INCLUDE_RTK2
  // Initialise the rtk gui
  protected: virtual int RtkStartup();
#endif

};

#endif
