///////////////////////////////////////////////////////////////////////////
//
// File: fixedobstacle.hh
// Author: Andrew Howard
// Date: 29 Dec 2001
// Desc: Simulates fixed obstacles
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/include/fixedobstacle.hh,v $
//  $Author: rtv $
//  $Revision: 1.4 $
//
///////////////////////////////////////////////////////////////////////////

#ifndef FIXEDOBSTACLE_HH
#define FIXEDOBSTACLE_HH

#include "entity.hh"

// Some forward declarations
class Nimage;

class CFixedObstacle : public CEntity
{
  // Default constructor
  public: CFixedObstacle(CWorld *world, CEntity *parent);

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

  void BuildQuadTree( uint8_t color, int x1, int y1, int x2, int y2 );
};

#endif
