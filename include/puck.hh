///////////////////////////////////////////////////////////////////////////
//
// File: puck.hh
// Author: brian gerkey
// Date: 23 June 2001
// Desc: Simulates pucks
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/include/puck.hh,v $
//  $Author: inspectorg $
//  $Revision: 1.10 $
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

#ifndef PUCK_HH
#define PUCK_HH

#include "entity.hh"

class CPuck : public CEntity
{
  // Default constructor
  public: CPuck(CWorld *world, CEntity *parent);

  // Load the entity from the worldfile
  public: virtual bool Load(CWorldFile *worldfile, int section);

  // Startup routine
  public: virtual bool Startup();
    
  // Update the device
  public: virtual void Update( double sim_time );

  // Move the puck
  private: void Move();
    
  // Return diameter of puck
  public: double GetDiameter() { return(m_size_x); }

  // Timings (for movement)
  private: double m_last_time;

  // coefficient of "friction"
  private: double m_friction;

  // Vision return value when we are being held
  // Vision return value when we are not being held
  private: int vision_return_held;
  private: int vision_return_notheld;

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
