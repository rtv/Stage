/* 	$Id: pioneermobiledevice.h,v 1.2 2000-12-01 00:20:52 vaughan Exp $	 */
// this device implements a differential-steer mobility base for the robot

#ifndef PIONEERMOBILEDEVICE_H
#define PIONEERMOBILEDEVICE_H

#include "device.hh"
#include "robot.h"
#include "world.h"
#include "win.h"

class CPioneerMobileDevice : public CDevice
{
 public:
  
  CPioneerMobileDevice( CRobot* rr, double wwidth, double llength,
			void *buffer, size_t data_len, 
			size_t command_len, size_t config_len);
  
  double speed, turnRate;

  unsigned char stall;

  double width, length,  halfWidth, halfLength;
  double xodom, yodom, aodom; 
    
  Rect rect, oldRect;
  // center pixel positions are used to draw the direction indicator 
  int centerx, oldCenterx, centery, oldCentery;
  
  bool Update( void );
  
  void Stop( void );
  void HitObstacle( void );  
  
  void CalculateRect( float x, float y, float a );
  void CalculateRect( void )
    { CalculateRect( m_robot->x, m_robot->y, m_robot->a ); };
  void StoreRect( void );

  int Move();
  
  virtual bool MapDraw( void );
  virtual bool MapUnDraw( void );

  XPoint undrawPts[7];
  virtual bool GUIDraw( void );
  virtual bool GUIUnDraw( void );
  
  void ComposeData();
  void ParseCommandBuffer();

 private:
  BYTE commands[ P2OS_COMMAND_BUFFER_SIZE ];
  BYTE data[ P2OS_DATA_BUFFER_SIZE ]; 

  double m_update_interval, m_last_update;

  CRobot* m_robot;
  CWorld* m_world;
};

#endif
