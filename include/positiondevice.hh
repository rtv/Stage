///////////////////////////////////////////////////////////////////////////
//
// File: pioneermobiledevice.hh
// Author: Richard Vaughan, Andrew Howard
// Date: 5 Dec 2000
// Desc: Simulates the Pioneer robot base
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/include/positiondevice.hh,v $
//  $Author: vaughan $
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

#ifndef PIONEERMOBILEDEVICE_H
#define PIONEERMOBILEDEVICE_H

#include "stage.h"
#include "playerdevice.hh"


class CPositionDevice : public CEntity
{
    // Minimal constructor
    //
    public: CPositionDevice(CWorld *world, CEntity *parent );

    // Update the device
    //
    public: virtual void Update( double sim_time );

    // Extract command from the command buffer
    //
    private: void ParseCommandBuffer(player_position_cmd_t &command );
				    
    // Compose the reply packet
    //
    private: void ComposeData(player_position_data_t &position );

    // Check to see if the given pose will yield a collision
    //
    private: bool InCollision(double px, double py, double pth);

    // Render the object in the world rep
    //
    private: virtual void Map(bool render);

    // Timings
    //
    private: double m_last_time;

    // Current command and data buffers
    //
  //private: player_position_cmd_t m_command;
  // private: player_position_data_t m_data;
    
    // Commanded robot speed
    //
    private: double m_com_vr, m_com_vth;

    // Odometric pose
    //
    private: double m_odo_px, m_odo_py, m_odo_pth;

    // Last map pose (for unmapping)
    //
    private: double m_map_px, m_map_py, m_map_pth;

    public: unsigned char stall;
    public: int Move();

#ifdef INCLUDE_RTK

    // Process GUI update messages
    //
    public: virtual void OnUiUpdate(RtkUiDrawData *data);

    // Process GUI mouse messages
    //
    public: virtual void OnUiMouse(RtkUiMouseData *data);

    // Draw the pioneer chassis
    //
    public: void DrawChassis(RtkUiDrawData *data);
  
#endif
};

#endif
