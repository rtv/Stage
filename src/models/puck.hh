///////////////////////////////////////////////////////////////////////////
//
// File: puck.hh
// Author: brian gerkey
// Date: 23 June 2001
// Desc: Simulates pucks
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/models/puck.hh,v $
//  $Author: rtv $
//  $Revision: 1.2.6.1 $
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
  public: CPuck( LibraryItem *libit, int id, CEntity *parent);

  // a static named constructor - a pointer to this function is given
  // to the Library object and paired with a string.  When the string
  // is seen in the worldfile, this function is called to create an
  // instance of this entity
public: static CPuck* Creator(  LibraryItem *libit, int id, CEntity *parent )
  { return( new CPuck( libit, id, parent ) ); }


  // Update the device
  public: virtual void Update( double sim_time );

  // Move the puck
  private: void Move();
    
  // Return diameter of puck
  public: double GetDiameter() { return(this->size_x); }

  // Timings (for movement)
  private: double m_last_time;

  // coefficient of "friction"
  private: double m_friction;

  // Vision return value when we are being held
  // Vision return value when we are not being held
  private: int vision_return_held;
  private: int vision_return_notheld;
};

#endif
