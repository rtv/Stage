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
//  $Revision: 1.1 $
//
///////////////////////////////////////////////////////////////////////////

#ifndef IDARTURRETDEVICE_HH
#define IDARTURRETDEVICE_HH

#include "irdevice.hh"

class CIDARTurretDevice : public CEntity
{
private: 

  CIDARDevice* idars[ PLAYER_IDARTURRET_IDAR_COUNT ];
 
public: 
  
  CIDARTurretDevice(CWorld *world, CEntity *parent );
 
  virtual void Sync( void ); 
  virtual void Update( double sim_time );  

#ifdef INCLUDE_RTK2
  // Initialise the rtk gui
  protected: virtual void RtkStartup();

  // Finalise the rtk gui
  protected: virtual void RtkShutdown();

  // Update the rtk gui
  protected: virtual void RtkUpdate();
  
#endif
};


#endif

