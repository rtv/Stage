///////////////////////////////////////////////////////////////////////////
//
// File: positiondevice.hh
// Author: Richard Vaughan, Andrew Howard
// Date: 5 Dec 2000
// Desc: Simulates the Pioneer robot base
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/include/positiondevice.hh,v $
//  $Author: inspectorg $
//  $Revision: 1.7 $
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

class CPositionDevice : public CEntity
{
  // Minimal constructor
  public: CPositionDevice(CWorld *world, CEntity *parent );

  // Update the device
  public: virtual void Update( double sim_time );

  // Extract command from the command buffer
  //private: void ParseCommandBuffer(player_position_cmd_t &command );
  private: void ParseCommandBuffer( void );
				    
  // Compose the reply packet
  //private: void ComposeData(player_position_data_t &position );
  private: void ComposeData( void );

  // Move the robot
  private: int Move();

  // Timings
  private: double m_last_time;

  // Current command and data buffers
  private: player_position_cmd_t m_command;
  private: player_position_data_t m_data;
    
  // Commanded robot speed
  private: double m_com_vr, m_com_vth;

  // Odometric pose
  protected: double m_odo_px, m_odo_py, m_odo_pth;
  protected: unsigned char stall;
};

#endif
