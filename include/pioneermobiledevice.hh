/* 	$Id: pioneermobiledevice.hh,v 1.1.2.1 2000-12-05 23:17:34 ahoward Exp $	 */
// this device implements a differential-steer mobility base for the robot

#ifndef PIONEERMOBILEDEVICE_H
#define PIONEERMOBILEDEVICE_H

#include "offsets.h"
#include "playerdevice.hh"
#include "win.h"



class CPioneerMobileDevice : public CPlayerDevice
{
    public: CPioneerMobileDevice( CRobot* robot, double wwidth, double llength,
                                  void *buffer, size_t data_len, 
                                  size_t command_len, size_t config_len);

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

    // Commanded robot speed
    //
    private: double m_com_vr, m_com_vth;

    // Last map pose
    //
    private: double m_map_px, m_map_py, m_map_pth;
    
  public:
  unsigned char stall;

  double width, length,  halfWidth, halfLength;
  double xodom, yodom, aodom; 
    
  Rect rect, oldRect;
  // center pixel positions are used to draw the direction indicator 
  int centerx, oldCenterx, centery, oldCentery;
  
  void Update( void );
  
  void Stop( void );
  void HitObstacle( void );  
  
  void CalculateRect( float x, float y, float a );
  void CalculateRect( void )
    { CalculateRect( m_robot->x, m_robot->y, m_robot->a ); };
  void StoreRect( void );

  int Move();
  
    //*** REMOVE virtual bool MapDraw( void );
    //*** REMOVE virtual bool MapUnDraw( void );

  XPoint undrawPts[7];
  virtual bool GUIDraw( void );
  virtual bool GUIUnDraw( void );
  


 private:

  PlayerPositionCommand m_command;
  PlayerPositionData m_data;

  double m_update_interval, m_last_update;

  // RTK interface
  //
#ifdef INCLUDE_RTK
    
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
