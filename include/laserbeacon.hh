///////////////////////////////////////////////////////////////////////////
//
// File: laerbeacon.hh
// Author: Andrew Howard
// Date: 4 Dec 2000
// Desc: Simulates the laser beacons
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/include/laserbeacon.hh,v $
//  $Author: ahoward $
//  $Revision: 1.8.2.2 $
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

#ifndef LASERBEACON_HH
#define LASERBEACON_HH

#include "entity.hh"

class CLaserBeacon : public CEntity
{
  // Default constructor
  public: CLaserBeacon(CWorld *world, CEntity *parent);

  // Load the entity from the worldfile
  public: virtual bool Load(CWorldFile *worldfile, int section);

  // Update the device
  public: virtual void Update( double sim_time );

  // Beacon id
  public: int id;

  // TESTING
  private: int map_px, map_py, map_pth;

#ifdef INCLUDE_RTK
    
  // Process GUI update messages
  public: virtual void OnUiUpdate(RtkUiDrawData *data);

  // Process GUI mouse messages
  public: virtual void OnUiMouse(RtkUiMouseData *data);

#endif
};

#endif
