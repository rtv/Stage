///////////////////////////////////////////////////////////////////////////
//
// File: irdevice.hh
// Author: Richard Vaughan
// Date: 22 October 2001
// Desc: Simulates HRL's Infrared Data and Ranging System
//
// Copyright HRL Laboratories LLC, 2001
// Supported by DARPA
//
// ** This file is not covered by the GNU General Public License **
//
//  CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/include/irdevice.hh,v $
//  $Author: rtv $
//  $Revision: 1.4 $
//
///////////////////////////////////////////////////////////////////////////

#ifndef IDARDEVICE_HH
#define IDARDEVICE_HH

#include "entity.hh"

typedef struct 
{
  int from_port, from_type, from_index;
  int to_port, to_type, to_index;

  int direction; // 0-7
  int intensity;

} idar_message_t;

class CIDARDevice : public CEntity
{
private: 

  double m_max_range;

  int m_num_scanlines; // number or lines raytraced per sensor
  double m_angle_per_sensor; // angle subtended by 1 sensor
  double m_angle_per_scanline; // angle between scanlines in a single sensor

  idarrx_t recv; // record the most intense message received
 
public: 
  
  CIDARDevice(CWorld *world, CEntity *parent );
  //~CIDARDevice( void );

  // each sensor must transmit over a wide area (45 degrees on the
  // orginal robots), so we do several ray traces per sensor to make
  // sure we hit anything in the way. the irdevice is rendered large
  // enough in the matrix so that it can guarantee to be hit by at
  // least one scanline in the chord. this looks like a reasonable
  // solution compared to the huge cost of scanning the whole chord.
  
  // Update the device
  //
  virtual void Update( double sim_time );
  
  virtual bool ReceiveMessage( CEntity* sender,
			       unsigned char* mesg, int len, 
			       uint8_t intensity,
			       bool reflection );
  
  virtual uint8_t LookupIntensity( uint8_t transmit_intensity, 
				   double transmit_range, 
				   bool reflection );

  virtual void TransmitMessage( idartx_t* transmit );
  
#ifdef INCLUDE_RTK2
  // Initialise the rtk gui
  protected: virtual void RtkStartup();

  // Finalise the rtk gui
  protected: virtual void RtkShutdown();

  // Update the rtk gui
  protected: virtual void RtkUpdate();
  
  // For drawing the ir message & beams
  private: rtk_fig_t *data_fig;
  private: rtk_fig_t *rays_fig;

#endif

};


#endif

