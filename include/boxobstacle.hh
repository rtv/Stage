///////////////////////////////////////////////////////////////////////////
//
// File: boxobstacle.hh
// Author: Andrew Howard
// Date: 18 Dec 2000
// Desc: Simulates box obstacles
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/include/boxobstacle.hh,v $
//  $Author: vaughan $
//  $Revision: 1.5.2.1 $
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

#ifndef BOXOBSTACLE_HH
#define BOXOBSTACLE_HH

#include "entity.hh"

class CBoxObstacle : public CEntity
{
    // Default constructor
    //
    public: CBoxObstacle(CWorld *world, CEntity *parent);

    // Load the object from an argument list
    //
    public: virtual bool Load(int argc, char **argv);

    // Save the object to an argument list
    //
    public: virtual bool Save(int &argc, char **argv);
    
  // Update the device
  //
public: virtual void Update( double sim_time );
  
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
