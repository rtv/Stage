///////////////////////////////////////////////////////////////////////////
//
// File: playerserver.hh
// Author: Richard Vaughan, Andrew Howard
// Date: 6 Dec 2000
// Desc: Provides interface to Player server.
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/include/playerdevice.hh,v $
//  $Author: inspectorg $
//  $Revision: 1.14 $
//
// Usage:
//  (empty)
//
// Theory of operation:
//  (empty)
//
// Known bugs:
//  (empty)
//
// Possible enhancements:
//  (empty)
//
///////////////////////////////////////////////////////////////////////////

#ifndef ROBOT_H
#define ROBOT_H

#include <sys/types.h>
#include "entity.hh"

// Forward declare some of the classes we will use
//
class CDevice;
class CWorld;

class CPlayerDevice : public CEntity
{
  public: CPlayerDevice( CWorld* world, CEntity *parent );
  public: ~CPlayerDevice( void );
    
  // Start all the devices
  // REMOVE public: virtual bool SetupIOPointers( char* io );

  // Update robot and all its devices
  public: virtual void Update( double sim_time );
    
  // Start the device
  public: virtual bool Startup( void ); 
  
  // Finalize object
  public: virtual void Shutdown();

  private: caddr_t playerIO; // ptr to shared memory for player I/O

#ifdef INCLUDE_RTK
    
  // Process GUI update messages
  //
  public: virtual void OnUiUpdate(RtkUiDrawData *pData);

  // Process GUI mouse messages
  //
  public: virtual void OnUiMouse(RtkUiMouseData *pData);

#endif
};

#endif





















