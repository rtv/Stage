///////////////////////////////////////////////////////////////////////////
//
// File: boxobstacle.hh
// Author: Andrew Howard
// Date: 18 Dec 2000
// Desc: Simulates box obstacles
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/models/box.hh,v $
//  $Author: rtv $
//  $Revision: 1.2 $
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

#ifndef BOX_HH
#define BOX_HH

#include "entity.hh"

class CBox : public CEntity
{
  // Default constructor
  public: CBox( LibraryItem *libit, CWorld *world, CEntity *parent);

  // a static named constructor - a pointer to this function is given
  // to the Library object and paired with a string.  When the string
  // is seen in the worldfile, this function is called to create an
  // instance of this entity
public: static CBox* Creator( LibraryItem *libit,  CWorld *world, CEntity *parent )
  {
    return( new CBox( libit, world, parent ) );
  }
  // Update the device
  public: virtual void Update(double sim_time);
};

#endif
