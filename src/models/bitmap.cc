///////////////////////////////////////////////////////////////////////////
//
// File: fixedobstacle.cc
// Author: Andrew Howard
// Date: 29 Dec 2001
// Desc: Simulates fixed obstacles
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/models/bitmap.cc,v $
//  $Author: inspectorg $
//  $Revision: 1.4 $
//
///////////////////////////////////////////////////////////////////////////

#include <float.h>
#include "image.hh"
#include "world.hh"
#include "bitmap.hh"

//#define DEBUG

///////////////////////////////////////////////////////////////////////////
// Default constructor
CBitmap::CBitmap(LibraryItem* libit, CWorld *world, CEntity *parent)
    : CEntity(libit, world, parent)
{
  vision_return = true;
  laser_return = LaserVisible;
  sonar_return = true;
  obstacle_return = true;
  puck_return = false; // we trade velocities with pucks
  idar_return = IDARReflect;

  this->crop_ax = -DBL_MAX;
  this->crop_ay = -DBL_MAX;
  this->crop_bx = +DBL_MAX;
  this->crop_by = +DBL_MAX;
 
  this->filename = NULL;
  this->scale = 1.0 / m_world->ppm;
  this->image = NULL;

#ifdef INCLUDE_RTK2
  // We cant move fixed obstacles (yet!)
  this->movemask = 0;
  // TODO - add Update() method so we can move bitmaps around
  //this->movemask = RTK_MOVE_TRANS | RTK_MOVE_ROT;
#endif
}

///////////////////////////////////////////////////////////////////////////
// Load the entity from the worldfile
bool CBitmap::Load(CWorldFile *worldfile, int section)
{
  PRINT_DEBUG2( "Loading from %p, %d\n", worldfile, section );

  if (!CEntity::Load(worldfile, section))
    return false;

  // Get the name of the image file to load.
  this->filename = worldfile->ReadFilename(section, "file", NULL);
  if (!this->filename || strlen(this->filename) == 0)
  {
    PRINT_ERR("empty filename");
    return false;
  }

  // Get the scale of the image;
  // i.e. the width/length of each pixel in m.
  // If no scale is specified, use the world resolution.
  this->scale = worldfile->ReadLength(section, "scale", 0 );
  
  if( this->scale != 0 )
    PRINT_WARN("worldfile bitmap keyword 'scale' is deprecated,"
               " please use 'resolution <meters per pixel>' instead");
  
  // Get the scale of the image;
  // i.e. the width/length of each pixel in m.
  // If no scale is specified, use the world resolution.
  this->scale = worldfile->ReadLength(section, "resolution", this->scale );
  if (this->scale == 0)
  {
    this->scale = 1.0 / m_world->ppm;
    PRINT_WARN2("\n\t- no resolution specified for image [%s]; using world default of %.2f",
                this->filename, this->scale );
  }


  // Get the crop region;
  // i.e. the bit of the image we are interested in
  this->crop_ax = worldfile->ReadTupleLength(section, "crop", 0, -DBL_MAX);
  this->crop_ay = worldfile->ReadTupleLength(section, "crop", 1, -DBL_MAX);
  this->crop_bx = worldfile->ReadTupleLength(section, "crop", 2, +DBL_MAX);
  this->crop_by = worldfile->ReadTupleLength(section, "crop", 3, +DBL_MAX);

  // Create and load the image here (we need to know its size)
  // Try to guess the file type from the extension.
  assert( this->image = new Nimage );

  int len = strlen(this->filename);
  if (strcmp(&(this->filename[len - 4]), ".fig") == 0)
  {
    /* REINSTATE someday
       if (!img.load_fig(this->filename, this->ppm, this->scale))
       return false;
    */
    return false;
  } 
  else if( strcmp( &(this->filename[ len - 7 ]), ".pnm.gz" ) == 0 )
  {
    if (!this->image->load_pnm_gz(this->filename))
      return false;
  }
  else 
  {
    if (!this->image->load_pnm(this->filename))
      return false;
  }

  // Compute the object size based on the image
  this->size_x = this->scale * this->image->width;
  this->size_y = this->scale * this->image->height;

  // is the pose explicitly set?
  // if the pose was NOT set in the worldfile
  if( (worldfile->ReadTupleLength( section, "pose", 0, DBL_MAX ) == DBL_MAX) )
  {
    // we shift so the global origin is at the bottom left corner of the image
    this->SetGlobalPose( this->size_x/2.0, size_y/2.0, 0);    

    // record this position as the initial pose se we don't try to save it later
    this->GetPose( init_px, init_py, init_pth );
  }

  // draw a border around the image
  //this->image->draw_box(0,0,this->image->width-1, this->image->height-1, 255 );
 
  // scan the image into a vector of rectangle descriptions
  double sx = this->scale;
  double sy = this->scale;

  // Draw the image into the matrix (and GUI if compiled in) 

  // RTV - this box-drawing algorithm compresses hospital.world from
  // 104,000+ pixels to 5,757 rectangles. it's not perfect but pretty
  // darn good with bitmaps featuring lots of horizontal and vertical
  // lines - such as most worlds. Also combined matrix & gui
  // rendering loops.  hospital.pnm.gz now loads more than twice as
  // fast and redraws waaaaaay faster. yay!
  
  for (int y = 0; y < this->image->height; y++)
  {
    for (int x = 0; x < this->image->width; x++)
    {
      //m_world->Ticker();

      if (this->image->get_pixel(x, y) == 0)
        continue;
	  
      // a rectangle starts from this point
      int startx = x;
      int starty = this->image->height - y;
      int height = this->image->height; // assume full height for starters
	  
      // grow the width - scan along the line until we hit an empty pixel
      for( ;  this->image->get_pixel( x, y ) > 0; x++ )
	    {
	      // handle horizontal cropping
	      double ppx = x * sx; 
	      if (ppx < this->crop_ax || ppx > this->crop_bx)
          continue;
	      
	      // look down to see how large a rectangle below we can make
	      int yy  = y;
	      while( (this->image->get_pixel( x, yy ) > 0 ) 
               && (yy < this->image->height) )
        { 
          // handle vertical cropping
          double ppy = (this->image->height - yy) * sy;
          if (ppy < this->crop_ay || ppy > this->crop_by)
            continue;
		  
          yy++; 
        } 	      
	      // now yy is the depth of a line of non-zero pixels
	      // downward we store the smallest depth - that'll be the
	      // height of the rectangle
	      if( yy-y < height ) height = yy-y; // shrink the height to fit
	    } 
	  
      int width = x - startx;
	  
      // delete the pixels we have used in this rect
      this->image->fast_fill_rect( startx, y, width, height, 0 );
	  
      double px = ((startx + (width/2.0) + 0.5 ) * sx) - size_x/2.0;
      double py = ((starty - (height/2.0) - 0.5 ) * sy) - size_y/2.0;
      //double pth = 0;
      double pw = width * sx;
      double ph = height * sy;
	  
      // store the rectangles for drawing into the GUI later
      bitmap_rectangle_t r;
      r.x = px;
      r.y = py;
      r.w = pw;
      r.h = ph;
      bitmap_rects.push_back( r );
	  
      // create a matrix rectangle in global coordinates
      //this->LocalToGlobal( px, py, pth );
      //m_world->SetRectangle( px, py, pth, pw, ph, this, true);
    }
  }

  return true;
}


bool ColorInRectangle( Nimage* image, uint8_t color, int x1, int y1, int x2, int y2 )
{
  // look in the rectangle for a pixel this color
  for( int p=x1; p<x2; p++ )
    for( int q=y1; q<y2; q++ )
      if( image->get_pixel( p, q ) == color ) return true; // found one!
 
  // we didn't find a pixel that color
  return false;
}


#ifdef INCLUDE_RTK2

void CBitmap::BuildQuadTree( uint8_t color, int x1, int y1, int x2, int y2 )
{
  //printf( "QT: %d  %d,%d %d,%d\n", color, x1,y1,x2,y2 );
  
  if( ColorInRectangle( this->image, color, x1, y1, x2, y2 ) )
    {
      int width = x2-x1;
      int height = y2-y1;

      // split the rectangle along its longest axis
      if( width > height )
	{
	  if( width <= 1 ) return;

	  int xsplit = x1 + (x2-x1)/2;
	  BuildQuadTree( color, x1, y1, xsplit, y2 );
	  BuildQuadTree( color, xsplit, y1, x2, y2 );	  
	}
      else
	{
	  if( height <=1 ) return;

	  int ysplit = y1 + (y2-y1)/2;
	  BuildQuadTree( color, x1, y1, x2, ysplit );
	  BuildQuadTree( color, x1, ysplit, x2, y2 );	  	  
	}
    }
  else     // add this rect to the figure
    {
      double width = (double)(x2-x1) * this->scale;
      double height =  (double)(y2-y1) * this->scale;
      double px = (double)x1 * this->scale + width/2.0;
      double py = (double)(this->image->height - y1) * this->scale - height/2.0;

      //printf( "rect: %.2f,%.2f  %.2f,%.2f\n", px, py, width, height );
      
      // create a rectangle 
      rtk_fig_rectangle( this->fig, px, py, 0.0, width, height, false);
    }
}
#endif

///////////////////////////////////////////////////////////////////////////
// Initialise object by Copying image into matrix
bool CBitmap::Startup()
{
  if (!CEntity::Startup())
    {
      PRINT_DEBUG( "CEntity::Startup() failed" );
      return false;
    }

  if( !this->image )
    {
      PRINT_DEBUG( "bitmap has no image" );
      //rtk_fig_clear( this->fig );
      return true;
    }

  // draw the add the rectangles we pre-computed in Load() into the matrix
  for(
      std::vector<bitmap_rectangle_t>::iterator it = bitmap_rects.begin();
      it != bitmap_rects.end();
      it++ )  
    {
      double th = 0;
      this->LocalToGlobal( it->x, it->y, th );
      m_world->SetRectangle( it->x, it->y, th, it->w, it->h, this, true);
      //rtk_fig_rectangle(this->fig, it->x, it->y, 0, it->w, it->h, true ); 
    }

  return true;
}


///////////////////////////////////////////////////////////////////////////
// Finalize object
void CBitmap::Shutdown()
{
  CEntity::Shutdown();
}


#ifdef INCLUDE_RTK2

void CBitmap::RtkStartup()
{
  CEntity::RtkStartup();

  // bitmaps don't need labels
  //rtk_fig_clear( this->label );
 
  rtk_fig_origin( this->fig, local_px, local_py, local_pth );
  rtk_fig_color_rgb32(this->fig, this->color);
  
  // add the figure we pre-computed in Startup() above  
  for(
      std::vector<bitmap_rectangle_t>::iterator it = bitmap_rects.begin();
      it != bitmap_rects.end();
      it++ )  
    rtk_fig_rectangle(this->fig, it->x, it->y, 0, it->w, it->h, true ); 
}

#endif


