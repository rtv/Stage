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
//  $Revision: 1.3.2.1 $
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
//#include "playerdevice.hh"

enum pioneer_shape_t { rectangle, circle };


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
  //private: void ParseCommandBuffer(player_position_cmd_t &command );
    private: void ParseCommandBuffer( void );
				    
    // Compose the reply packet
    //
  //private: void ComposeData(player_position_data_t &position );
    private: void ComposeData( void );

    // Check to see if the given pose will yield a collision
    //
    private: bool InCollision(double px, double py, double pth);

    // Render the object in the world rep
    //
    private: virtual void Map(bool render);

    // Load the object from an argument list
    //
    public: virtual bool Load(int argc, char **argv);

    // Save the object to an argument list
    //
    public: virtual bool Save(int &argc, char **argv);

    // Timings
    //
    private: double m_last_time;

private: pioneer_shape_t m_shape;
public: pioneer_shape_t GetShape( void ){ return m_shape; };
public: void SetShape( pioneer_shape_t );

    // Current command and data buffers
    //
  private: player_position_cmd_t m_command;
  private: player_position_data_t m_data;
    
    // Commanded robot speed
    //
  //private: double m_com_vr, m_com_vth;

    // Odometric pose
    //
    private: double m_odo_px, m_odo_py, m_odo_pth;

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
