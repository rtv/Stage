/* 	$Id: pioneermobiledevice.h,v 1.1 2000-11-30 02:50:13 vaughan Exp $	 */
// this device implements a differential-steer mobility base for the robot

#ifdef PIONEERMOBILEDEVICE_H
#define PIONEERMOBILEDEVICE_H

#include "device.hh"
#include "robot.h"

class CPioneerMobileDevice : public CDevice
{
  
 public:
  
  CPioneerMobileDevice( CRobot* rr, void *buffer, size_t data_len, 
			size_t command_len, size_t config_len);
  
  float width, length, speed, turnRate;
  float halfWidth, halfLength;
  
  float xodom;
  float yodom;
  float aodom; 
  
  unsigned char stall;
  
  Rect rect, oldRect;
  // center pixel positions are used to draw the direction indicator 
  int centerx, oldCenterx, centery, oldCentery;
  
  
  void Stop( void );
  void Move( void );
  void HitObstacle( void );  
  
  void CalculateRect( float x, float y, float a );
  void CalculateRect( void ){ CalculateRect( x, y, a ); };
  void StoreRect( void );
  void UnDraw( Nimage* img );
  void Draw( Nimage* img );
  int Move( Nimage* img );
  
  int HasMoved( void );
  
 private:
  BYTE commands[ P2OS_COMMAND_BUFFER_SIZE ];
  BYTE data[ P2OS_DATA_BUFFER_SIZE ]; 
};

#endif
