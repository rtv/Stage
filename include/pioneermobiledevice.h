/* 	$Id: pioneermobiledevice.h,v 1.7 2001-01-13 02:45:54 gerkey Exp $	 */
// this device implements a differential-steer mobility base for the robot

#ifndef PIONEERMOBILEDEVICE_H
#define PIONEERMOBILEDEVICE_H

#include "stage.h"
#include "playerdevice.hh"
#include "win.h"



class CPioneerMobileDevice : public CPlayerDevice
{
 public:
  
  CPioneerMobileDevice( CRobot* robot, double wwidth, double llength,
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
  
  void CalculateRect( double x, double y, double a );
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

  //*** remove. ahoward BYTE commands[ POSITION_COMMAND_BUFFER_SIZE ];
  //BYTE data[ PDATA_BUFFER_SIZE ];
  player_position_cmd_t m_command;
  player_position_data_t m_data;

  double m_update_interval, m_last_update;
};

#endif
