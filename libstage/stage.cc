// Author: Richard Vaughan

#include <FL/Fl_Shared_Image.H>

#include "stage.hh"
#include "config.h" // results of cmake's system configuration tests
#include "file_manager.hh"
using namespace Stg;

static bool init_called = false;

const char* Stg::Version()
{ 
  return VERSION; 
}


void Stg::Init( int* argc, char** argv[] )
{
  PRINT_DEBUG( "Stg::Init()" );
  
  // copy the command line args for controllers to inspect
  World::args.clear();
  for( int i=0; i<*argc; i++ )
    World::args.push_back( (*argv)[i] ); 

  // seed the RNG 
  srand48( time(NULL) );

  if(!setlocale(LC_ALL,"POSIX"))
    PRINT_WARN("Failed to setlocale(); config file may not be parse correctly\n" );
						
  RegisterModels();
	
  // ask FLTK to load support for various image formats
  fl_register_images();

  init_called = true;
}

bool Stg::InitDone()
{
  return init_called;
}

const Color Color::blue( 0,0,1 );
const Color Color::red( 1,0,0 );
const Color Color::green( 0,1,0 );
const Color Color::yellow( 1,1,0 );
const Color Color::magenta( 1,0,1 );
const Color Color::cyan( 0,1,1 );		


// static inline uint8_t* pb_get_pixel( Fl_Shared_Image* img, 
// 				     const unsigned int x, 
// 				     const unsigned int y )
// {
//   uint8_t* pixels = (uint8_t*)(img->data()[0]);
//   const unsigned int index = (y * img->w() * img->d()) + (x * img->d());
//   return( pixels + index );
// }

 // // returns true if the value in the first channel is above threshold
// static inline bool pb_pixel_is_set( Fl_Shared_Image* img, 
// 				    const unsigned int x, 
// 				    const unsigned int y, 
// 				    const unsigned int threshold )
// {
//   return( pb_get_pixel( img,x,y )[0] > threshold );
// }

// set all the pixels in a rectangle 
static inline void pb_set_rect( Fl_Shared_Image* pb, 
				const unsigned int x, const unsigned int y, 
				const unsigned int rwidth, const unsigned int rheight, 
				const uint8_t val )
{
  const unsigned int bytes_per_sample = 1;
  const unsigned int depth = pb->d();
  const unsigned int width = pb->w();
  
  for( unsigned int a = y; a < y+rheight; a++ )
    {	
      // zeroing
      //uint8_t* pix = pb_get_pixel( pb, x, a );
      uint8_t* pix = (uint8_t*)(pb->data()[0] + (a*width*depth) + x*depth);
      memset( pix, val, rwidth * depth * bytes_per_sample );
    }
}  


static inline bool pixel_is_set( uint8_t* pixels,
				 const unsigned int width,
				 const unsigned int depth,
				 const unsigned int x, 
				 const unsigned int y,
				 uint8_t threshold )
{
  return( (pixels + (y*width*depth) + x*depth)[0] > threshold );
}

int Stg::rotrects_from_image_file( const std::string& filename, 
				   std::vector<rotrect_t>& rects )
{
  // TODO: make this a parameter
  const int threshold = 127;
  
  Fl_Shared_Image *img = Fl_Shared_Image::get(filename.c_str());
  if( img == NULL ) 
    {
      std::cerr << "failed to open file: " << filename << std::endl;

      assert( img ); // easy access to this point in debugger     
      exit(-1); 
    }

  //printf( "loaded image %s w %d h %d d %d count %d ld %d\n", 
  //  filename, img->w(), img->h(), img->d(), img->count(), img->ld() );

  const unsigned int width = img->w();
  const unsigned height = img->h();
  const unsigned int depth = img->d();
  uint8_t* pixels = (uint8_t*)img->data()[0];

  for(unsigned int y = 0; y < height; y++)
    {
      for(unsigned int x = 0; x < width; x++)
	{
	  // skip blank (white) pixels
	  if(  pixel_is_set( pixels, width, depth, x, y, threshold) )
	    continue;

	  // a rectangle starts from this point
	  const unsigned int startx = x;
	  const unsigned int starty = y;
	  unsigned int rheight = height; // assume full height for starters

	  // grow the width - scan along the line until we hit an empty (white) pixel
	    for( ; x < width &&  ! pixel_is_set( pixels, width, depth, x, y, threshold); x++ )
	    {
	      // look down to see how large a rectangle below we can make
	      unsigned int yy  = y;
	      //while( ! pb_pixel_is_set(img,x,yy,threshold) && (yy < height-1) )
	      while( !  pixel_is_set( pixels, width, depth, x, yy, threshold) && (yy < height-1) )
		  yy++;

	      // now yy is the depth of a line of non-zero pixels
	      // downward we store the smallest depth - that'll be the
	      // height of the rectangle
	      if( yy-y < rheight ) rheight = yy-y; // shrink the height to fit
	    } 

	  // whiten the pixels we have used in this rect
	  pb_set_rect( img, startx, starty, x-startx, rheight, 0xFF );

	  //  y-invert all the rectangles because we're using conventional
	  // rather than graphics coordinates. this is much faster than
	  // inverting the original image.

	  rotrect_t latest;// = &(*rects)[(*rect_count)-1];
	  latest.pose.x = startx;
	  latest.pose.y = height-1 - (starty + rheight);
	  latest.pose.a = 0.0;
	  latest.size.x = x - startx;
	  latest.size.y = rheight;

	  assert( latest.pose.x >= 0 );
	  assert( latest.pose.y >= 0 );
	  assert( latest.pose.x <= width );
	  assert( latest.pose.y <= height);

	  rects.push_back( latest );

	  //printf( "rect %d (%.2f %.2f %.2f %.2f %.2f\n", 
	  //  *rect_count, 
	  //  latest->x, latest->y, latest->a, latest->w, latest->h );

	}
    }

  if( img ) img->release(); // frees all resources for this image
  return 0; // ok
}

/*
private static void Floodfill(byte[,] vals, point_int_t q, byte SEED_COLOR, byte COLOR)
{
  int h = vals.GetLength(0);
  int w = vals.GetLength(1);

  if (q.Y < 0 || q.Y > h - 1 || q.X < 0 || q.X > w - 1)
    return;

  std::stack<Point> stack = new Stack<Point>();
  stack.Push(q);
  while (stack.Count > 0)
    {
      Point p = stack.Pop();
      int x = p.X;
      int y = p.Y;
      if (y < 0 || y > h - 1 || x < 0 || x > w - 1)
	continue;
      byte val = vals[y, x];
      if (val == SEED_COLOR)
        {
	  vals[y, x] = COLOR;
	  stack.Push(new Point(x + 1, y));
	  stack.Push(new Point(x - 1, y));
	  stack.Push(new Point(x, y + 1));
	  stack.Push(new Point(x, y - 1));
        }
    }
}
*/

// POINTS -----------------------------------------------------------

point_t* Stg::unit_square_points_create( void )
{
  point_t * pts = new point_t[4];

  pts[0].x = 0;
  pts[0].y = 0;
  pts[1].x = 1;
  pts[1].y = 0;
  pts[2].x = 1;
  pts[2].y = 1;
  pts[3].x = 0;
  pts[3].y = 1;

  return pts;
}

// return a value based on val, but limited minval <= val >= maxval  
double Stg::constrain( double val, const double minval, const double maxval )
{
  if( val < minval ) return minval;
  if( val > maxval ) return maxval;
  return val;
}

