///////////////////////////////////////////////////////////////////////////
//
// File: irturretdevice.hh
// Author: Richard Vaughan
// Date: 22 October 2001
// Desc: Provides a single interface to a collection of IDARs
//
//
//  CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/include/idarturretdevice.hh,v $
//  $Author: rtv $
//  $Revision: 1.4 $
//
///////////////////////////////////////////////////////////////////////////

#ifndef IDARTURRETDEVICE_HH
#define IDARTURRETDEVICE_HH

#include "irdevice.hh"

class CIDARTurretDevice : public CPlayerEntity
{
private: 

  CIDARDevice* idars[ PLAYER_IDARTURRET_IDAR_COUNT ];
 
public: 
  
  CIDARTurretDevice(CWorld *world, CEntity *parent );
 
  // a static named constructor - a pointer to this function is given
  // to the Library object and paired with a string.  When the string
  // is seen in the worldfile, this function is called to create an
  // instance of this entity
public: static CIDARTurretDevice* Creator( CWorld *world, CEntity *parent )
  { return( new CIDARTurretDevice( world, parent ) ); }

  virtual void Sync( void ); 
  virtual void Update( double sim_time );  


#ifdef INCLUDE_RTK2
  virtual void RenderMessages( player_idarturret_reply_t* rep );

  rtk_fig_t* data_fig;
  // Initialise the rtk gui
  protected: virtual void RtkStartup();

  // Finalise the rtk gui
  protected: virtual void RtkShutdown();

  // Update the rtk gui
  protected: virtual void RtkUpdate();
  
#endif
};


#endif

