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
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/models/idar.hh,v $
//  $Author: rtv $
//  $Revision: 1.1.2.1 $
//
///////////////////////////////////////////////////////////////////////////

#ifndef IDAR_HH
#define IDAR_HH

#include "entity.hh"

#include <string.h> // for memcpy(3)

/*************************************************************************/

/*************************************************************************/
/*
 * IDARTurret device - a collection of IDARs with a combined interface
 */

#define PLAYER_IDARTURRET_IDAR_COUNT 8

// define structures to get and receive messages from a collection of
// IDARs in one go.

typedef struct
{
  stage_idar_rx_t rx[ PLAYER_IDARTURRET_IDAR_COUNT ];
} __attribute__ ((packed)) player_idarturret_reply_t;

typedef  struct
{
  stage_idar_tx_t tx[ PLAYER_IDARTURRET_IDAR_COUNT ];
} __attribute__ ((packed)) player_idarturret_config_t;

class CIdarModel : public CEntity
{
private: 
  int m_num_scanlines; // number or lines raytraced per sensor
  double m_angle_per_sensor; // angle subtended by 1 sensor
  double m_angle_per_scanline; // angle between scanlines in a single sensor

public: 
  
  CIdarModel(int id, char* token, char* color, CEntity *parent);

  // a static named constructor - a pointer to this function is given
  // to the Library object and paired with a string.  When the string
  // is seen in the worldfile, this function is called to create an
  // instance of this entity
public: static CIdarModel* Creator( int id, char* token, 
				    char* color, CEntity *parent )
  { return( new CIdarModel( id, token, color, parent ) ); };
  
  // override the CEntity::Property to handle IDAR specific messages
  virtual int Property( int con, stage_prop_id_t property, 
			void* value, size_t len, stage_buffer_t* reply );

  // each sensor must transmit over a wide area (45 degrees on the
  // orginal robots), so we do several ray traces per sensor to make
  // sure we hit anything in the way. the irdevice is rendered large
  // enough in the matrix so that it can guarantee to be hit by at
  // least one scanline in the chord. this looks like a reasonable
  // solution compared to the huge cost of scanning the whole chord.

  
  virtual bool ReceiveMessage( CEntity* sender,
			       unsigned char* mesg, int len, 
			       uint8_t intensity, bool reflection );
  
  virtual uint8_t LookupIntensity( uint8_t transmit_intensity, 
				   double transmit_range, 
				   bool reflection );
  
  virtual void TransmitMessage( stage_idar_tx_t* transmit );
  
#ifdef INCLUDE_RTK2
  // Initialise the rtk gui
  public: virtual int RtkStartup();

  // Finalise the rtk gui
  public: virtual void RtkShutdown();

  // flash up triangles to show we are RXTXing
  void RtkShowReceived( void );
  void RtkShowSent( void );

  // For drawing the ir message & beams
  private: rtk_fig_t *data_fig;
  private: rtk_fig_t *rays_fig;
  
private: rtk_fig_t *incoming_figs[STG_TRANSDUCERS_MAX];
private: rtk_fig_t *outgoing_figs[STG_TRANSDUCERS_MAX];
#endif

};


#endif

