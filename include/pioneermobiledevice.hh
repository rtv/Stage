/* 	$Id: pioneermobiledevice.hh,v 1.1.2.3 2000-12-06 21:48:32 ahoward Exp $	 */
// this device implements a differential-steer mobility base for the robot

#ifndef PIONEERMOBILEDEVICE_H
#define PIONEERMOBILEDEVICE_H

#include "offsets.h"
#include "playerdevice.hh"
#include "win.h"



class CPioneerMobileDevice : public CPlayerDevice
{
    // Minimal constructor
    //
    public: CPioneerMobileDevice(CWorld *world, CObject *parent,
                                 CPlayerRobot* robot, 
                                 void *buffer, size_t buffer_len);

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
    private: double m_update_interval, m_last_update;

    // Current command and data buffers
    //
    private: PlayerPositionCommand m_command;
    private: PlayerPositionData m_data;
    
    // Commanded robot speed
    //
    private: double m_com_vr, m_com_vth;

    // Last map pose (for unmapping)
    //
    private: double m_map_px, m_map_py, m_map_pth;

    
  public:
  unsigned char stall;

  double width, length,  halfWidth, halfLength;
  double xodom, yodom, aodom; 
    
  Rect rect, oldRect;
  // center pixel positions are used to draw the direction indicator 
  int centerx, oldCenterx, centery, oldCentery;
  
    // *** REMOVE ahoward void HitObstacle( void );  
  
  int Move();
  
 private:

#ifndef INCLUDE_RTK

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
