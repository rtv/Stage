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
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/idardevice.hh,v $
//  $Author: gerkey $
//  $Revision: 1.3 $
//
///////////////////////////////////////////////////////////////////////////

#ifndef IDARDEVICE_HH
#define IDARDEVICE_HH

#include "playerdevice.hh"

typedef struct 
{
  int from_port, from_type, from_index;
  int to_port, to_type, to_index;

  int direction; // 0-7
  int intensity;

} idar_message_t;

class CIdarDevice : public CPlayerEntity
{
private: 

  double m_max_range;

  int m_num_scanlines; // number or lines raytraced per sensor
  double m_angle_per_sensor; // angle subtended by 1 sensor
  double m_angle_per_scanline; // angle between scanlines in a single sensor

  idarrx_t recv; // record the most intense message received

public: 
  
  CIdarDevice(CWorld *world, CEntity *parent );
  //~CIdarDevice( void );

  // a static named constructor - a pointer to this function is given
  // to the Library object and paired with a string.  When the string
  // is seen in the worldfile, this function is called to create an
  // instance of this entity
public: static CIdarDevice* Creator( CWorld *world, CEntity *parent )
  { return( new CIdarDevice( world, parent ) ); }

  // Startup routine
  public: virtual bool Startup();

  // each sensor must transmit over a wide area (45 degrees on the
  // orginal robots), so we do several ray traces per sensor to make
  // sure we hit anything in the way. the irdevice is rendered large
  // enough in the matrix so that it can guarantee to be hit by at
  // least one scanline in the chord. this looks like a reasonable
  // solution compared to the huge cost of scanning the whole chord.

  virtual void Sync( void );
  
  // Update the device
  //
  virtual void Update( double sim_time );
  
  virtual bool ReceiveMessage( CEntity* sender,
			       unsigned char* mesg, int len, 
			       uint8_t intensity, bool reflection );
  
  virtual uint8_t LookupIntensity( uint8_t transmit_intensity, 
				   double transmit_range, 
				   bool reflection );

  virtual void TransmitMessage( idartx_t* transmit );
  
  // wipe the current message (and clear figure if rtk is used)
  virtual void ClearMessage( void );
  virtual void CopyMessage( idarrx_t* msg ){ memcpy( msg, &recv, sizeof(recv)); }; 
  virtual void CopyAndClearMessage( idarrx_t* msg ){ CopyMessage(msg); ClearMessage(); };    

#ifdef INCLUDE_RTK2
  // Initialise the rtk gui
  public: virtual void RtkStartup();

  // Finalise the rtk gui
  public: virtual void RtkShutdown();

  // Update the rtk gui
 public: virtual void RtkUpdate();
  
  // For drawing the ir message & beams
  private: rtk_fig_t *data_fig;
  private: rtk_fig_t *rays_fig;

#endif

};


#endif

