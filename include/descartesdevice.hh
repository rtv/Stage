///////////////////////////////////////////////////////////////////////////
//
// File: pioneermobiledevice.hh
// Author: Richard Vaughan, Andrew Howard
// Date: 5 Dec 2000
// Desc: Simulates the Pioneer robot base
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/include/descartesdevice.hh,v $
//  $Author: rtv $
//  $Revision: 1.1 $
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

#ifndef DESCARTESDEVICE_H
#define DESCARTESDEVICE_H

#include "positiondevice.hh"

class CDescartesDevice : public CPositionDevice
{
  // Minimal constructor
  //
 public: CDescartesDevice(CWorld *world, CEntity *parent );
 
 // Control speed and turn rate to achieve desired heading and speed
 //
 public: void Servo( void );
 
// Update the device
 //
 public: virtual void Update( double sim_time );
  
  // override the parent's method in order to set the bumpers
public: virtual CEntity* TestCollision(double px, double py, double pth);
 
 // Extract command from the command buffer
 //
 //private: void ParseCommandBuffer(player_position_cmd_t &command );
 private: void ParseCommandBuffer( void );
 
 // Compose the reply packet
 //
 //private: void ComposeData(player_position_data_t &position );
 private: void ComposeData( void );
 
  
  // odometric pose estimate
  //protected: double m_odo_px, m_odo_py, m_odo_pth;

 // Current command and data buffers
 // - these hide the positiondevice buffers with the same names
 private: player_descartes_cmd_t m_command;
 private: player_descartes_data_t m_data;
 
private: double lastxpos, lastypos;

  // these are demanded by the user and set in ParseCommandBuffer()
  // Servo() tweaks speed and turnrate to achieve these
private: 
  double goalspeed, goalheading, goaldistance, distance_travelled;
  
  bool left_bumper, right_bumper;

  double lastcommandtime; // the time in seconds of the last command
};

#endif
