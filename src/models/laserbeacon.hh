///////////////////////////////////////////////////////////////////////////
//
// File: laerbeacon.hh
// Author: Andrew Howard
// Date: 4 Dec 2000
// Desc: Simulates the laser beacons
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/models/laserbeacon.hh,v $
//  $Author: rtv $
//  $Revision: 1.1 $
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

  // a static named constructor - a pointer to this function is given
  // to the Library object and paired with a string.  When the string
  // is seen in the worldfile, this function is called to create an
  // instance of this entity
public: static CLaserBeacon* Creator( CWorld *world, CEntity *parent )
  { return( new CLaserBeacon( world, parent ) ); }

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
