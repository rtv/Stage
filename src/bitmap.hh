///////////////////////////////////////////////////////////////////////////
//
// File: bitmap.hh
// Author: Andrew Howard
// Date: 29 Dec 2001
// Desc: Simulates fixed obstacles
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/bitmap.hh,v $
//  $Author: rtv $
//  $Revision: 1.2 $
//
///////////////////////////////////////////////////////////////////////////

#ifndef BITMAP_HH
#define BITMAP_HH

#include "entity.hh"
#include <vector>

// Some forward declarations
class Nimage;

class CBitmap : public CEntity
{
  // Default constructor
  public: CBitmap(CWorld *world, CEntity *parent);

  // a static named constructor - a pointer to this function is given
  // to the Library object and paired with a string.  When the string
  // is seen in the worldfile, this function is called to create an
  // instance of this entity
  public: static CBitmap* Creator( CWorld *world, CEntity *parent )
  {
    return( new CBitmap( world, parent ) );
  }

  // Load the entity from the worldfile
  public: virtual bool Load(CWorldFile *worldfile, int section);

  // Initialise object
  public: virtual bool Startup( void ); 
  
  // Finalize object
  public: virtual void Shutdown();

  // Name of file containg image
  private: const char *filename;

  // Scale of the image (m/pixel)
  public: double scale;

  // Crop region (m)
  private: double crop_ax, crop_ay, crop_bx, crop_by;

  // The image representing the environment
  public: Nimage *image;

#ifdef INCLUDE_RTK2
  typedef struct
  {
    double x, y, w, h;
  } bitmap_rectangle_t;
  
  vector<bitmap_rectangle_t> bitmap_rects;

  void RtkStartup();
#endif

  void BuildQuadTree( uint8_t color, int x1, int y1, int x2, int y2 );
};

#endif
