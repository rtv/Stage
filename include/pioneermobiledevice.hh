///////////////////////////////////////////////////////////////////////////
//
// File: pioneermobiledevice.hh
// Author: Richard Vaughan, Andrew Howard
// Date: 5 Dec 2000
// Desc: Simulates the Pioneer robot base
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/include/pioneermobiledevice.hh,v $
//  $Author: gerkey $
//  $Revision: 1.8 $
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

enum pioneer_shape_t
{
  rectangle,
  circle
};


class CPioneerMobileDevice : public CPlayerDevice
{
    // Minimal constructor
    //
    public: CPioneerMobileDevice(CWorld *world, CEntity *parent, CPlayerServer* server);

    ///////////////////////////////////////////////////////////////////////////
    // Load the object from an argument list
    //
    bool Load(int argc, char **argv);
   
    // Update the device
    //
    public: virtual void Update();

    // Extract command from the command buffer
    //
    private: void ParseCommandBuffer();

    // Compose the reply packet
    //
    private: void ComposeData();

    // Check to see if the given pose will yield a collision
    //
    private: bool InCollision(double px, double py, double pth);

    // Get and set shape parameter
    public: pioneer_shape_t GetShape() { return(m_shape); };
    public: void SetShape(pioneer_shape_t shape);

    // Render the object in the world rep
    //
    private: bool Map(bool render);

    // Timings
    //
    private: double m_last_time;

    ////////////////////////
    // Robot dimensions

    // Rectangular robot:
    private: double m_size_x, m_size_y, m_offset_x;

    // Circular robot:
    private: double m_radius;
    
    // structure for exporting pioneer-specific data to a GUI
    private: ExportPositionData expPosition; 

    public: double GetRadius() { return(m_radius); }

    ////////////////////////

    // Rectangular robot:

    // Current command and data buffers
    //
    private: player_position_cmd_t m_command;
    private: player_position_data_t m_data;
    
    // Our shape
    //
    private: pioneer_shape_t m_shape;


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
