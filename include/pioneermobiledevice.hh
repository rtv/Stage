///////////////////////////////////////////////////////////////////////////
//
// File: pioneermobiledevice.hh
// Author: Richard Vaughan, Andrew Howard
// Date: 5 Dec 2000
// Desc: Simulates the Pioneer robot base
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/include/pioneermobiledevice.hh,v $
//  $Author: ahoward $
//  $Revision: 1.1.2.7 $
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


class CPioneerMobileDevice : public CPlayerDevice
{
    // Minimal constructor
    //
    public: CPioneerMobileDevice(CWorld *world, CObject *parent, CPlayerRobot* robot);

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

    // Render the object in the world rep
    //
    private: bool Map(bool render);

    // Timings
    //
    private: double m_last_time;

    // Robot dimensions
    //
    private: double m_width, m_length;

    // Current command and data buffers
    //
    private: player_position_cmd_t m_command;
    private: player_position_data_t m_data;
    
    // Commanded robot speed
    //
    private: double m_com_vr, m_com_vth;

    // Odometric pose
    //
    private: double m_odo_px, m_odo_py, m_odo_pth;

    // Last map pose (for unmapping)
    //
    private: double m_map_px, m_map_py, m_map_pth;

    
  public:
  unsigned char stall;

  int Move();
  
 private:

#ifndef INCLUDE_RTK

        
  Rect rect, oldRect;
  // center pixel positions are used to draw the direction indicator 
  int centerx, oldCenterx, centery, oldCentery;
  
  // *** REMOVE ahoward void HitObstacle( void );  
  
  void CalculateRect( float x, float y, float a );
  void CalculateRect( void )
    { CalculateRect( m_robot->x, m_robot->y, m_robot->a ); };
  void StoreRect( void );
  XPoint undrawPts[7];
  virtual bool GUIDraw( void );
  virtual bool GUIUnDraw( void );

#else
    
    // Process GUI update messages
    //
    public: virtual void OnUiUpdate(RtkUiDrawData *pData);

    // Process GUI mouse messages
    //
    public: virtual void OnUiMouse(RtkUiMouseData *pData);

    // Draw the pioneer chassis
    //
    public: void DrawChassis(RtkUiDrawData *pData);
  
#endif
};

#endif
