///////////////////////////////////////////////////////////////////////////
//
// File: irturretdevice.hh
// Author: Richard Vaughan
// Date: 22 October 2001
// Desc: Provides a single interface to a collection of IDARs
//
//
//  CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/idarturretdevice.hh,v $
//  $Author: rtv $
//  $Revision: 1.3 $
//
///////////////////////////////////////////////////////////////////////////

#ifndef IDARTURRETDEVICE_HH
#define IDARTURRETDEVICE_HH

#include "idardevice.hh"

class CIdarTurretDevice : public CPlayerEntity
{
private: 

  CIdarDevice* idars[ PLAYER_IDARTURRET_IDAR_COUNT ];
 
public: 
  
  CIdarTurretDevice(CWorld *world, CEntity *parent );
 
  // a static named constructor - a pointer to this function is given
  // to the Library object and paired with a string.  When the string
  // is seen in the worldfile, this function is called to create an
  // instance of this entity
public: static CIdarTurretDevice* Creator( CWorld *world, CEntity *parent )
  { return( new CIdarTurretDevice( world, parent ) ); }

  virtual void Sync( void ); 
  virtual void Update( double sim_time );  


#ifdef INCLUDE_RTK2

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

