///////////////////////////////////////////////////////////////////////////
//
// File: bitmap.hh
// Author: Andrew Howard
// Date: 29 Dec 2001
// Desc: Simulates fixed obstacles
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/models/bitmap.hh,v $
//  $Author: rtv $
//  $Revision: 1.2.6.2 $
//
///////////////////////////////////////////////////////////////////////////

#ifndef BITMAP_HH
#define BITMAP_HH

#include <vector>
#include "entity.hh"

// Some forward declarations
class Nimage;

typedef struct
{
  double x, y, w, h;
} bitmap_rectangle_t;

class CBitmap : public CEntity
{
  // Default constructor
  public: CBitmap( LibraryItem *libit, int id, CEntity *parent);

  // a static named constructor - a pointer to this function is given
  // to the Library object and paired with a string.  When the string
  // is seen in the worldfile, this function is called to create an
  // instance of this entity
  public: static CBitmap* Creator( LibraryItem *libit, 
				   int id, CEntity *parent )
  {
    return( new CBitmap( libit, id, parent ) );
  }

private:
  stage_rotrect_t* rects;
  int rect_count;
  double rects_max_x, rects_max_y; // the upper bounds on rectangle positions
  
  // bool controls whether rects are added to or removed from the matrix
  void RenderRects(  bool render );

public:
  virtual int SetProperty( int con, stage_prop_id_t property, 
			   void* value, size_t len );
  
#ifdef INCLUDE_RTK2  
  void RtkStartup();
#endif

};

#endif
