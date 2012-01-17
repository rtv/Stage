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

	  rotrect_t latest;
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

double direction( double a )
{
  if( a == 0.0 )
    return 0;
  else
    return sgn(a);
}

int Stg::polys_from_image_file( const std::string& filename, 
				std::vector<std::vector<point_t> >& polys )
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
  
  // a set of previously seen directed edges, The key is a 4-element vector
  // [x1,y1,x2,y2].
  std::set<std::vector<uint32_t> > edges;

  for(unsigned int y = 0; y < height; y++)
    {
      for(unsigned int x = 0; x < width; x++)
	{
	  // skip blank (white) pixels
	  if(  pixel_is_set( pixels, width, depth, x, y, threshold) )
	    continue;
	  
	  // generate the four directed edges for this pixel	  
	  std::vector<uint32_t> edge[4];
	  
	  for( int i=0; i<4; i++ )
	    edge[i].resize(4);
	  
	  edge[0][0] = x+0;
	  edge[0][1] = y+0;
	  edge[0][2] = x+1;
	  edge[0][3] = y+0;
	  
	  edge[1][0] = x+1;
	  edge[1][1] = y+0;
	  edge[1][2] = x+1;
	  edge[1][3] = y+1;
	  
	  edge[2][0] = x+1;
	  edge[2][1] = y+1;
	  edge[2][2] = x+0;
	  edge[2][3] = y+1;

	  edge[3][0] = x+0;
	  edge[3][1] = y+1;
	  edge[3][2] = x+0;
	  edge[3][3] = y+0;
	  
	  // put them in the set of the inverse edge does not exist
	  for( int i=0; i<4; i++ )
	    {
	      // the same edge with the opposite direction
	      std::vector<uint32_t> inv(4);
	      inv[0] = edge[i][2];
	      inv[1] = edge[i][3];
	      inv[2] = edge[i][0];
	      inv[3] = edge[i][1];
	      
	      std::set<std::vector<uint32_t> >::iterator it = 
		edges.find( inv );
	      
	      if( it == edges.end() ) // inverse not found
		edges.insert( edge[i] ); // add the new edge
	      else // inverse found! delete it
		edges.erase( it );
	    }
	}
    }
    
  std::multimap<point_t,point_t> mmap;
  
  FOR_EACH( it, edges )
    {
      // fill a multimap with start-point / end-point pairs.
      std::pair<point_t,point_t> p( point_t( (*it)[0], (*it)[1] ),
				    point_t( (*it)[2], (*it)[3] ));
      mmap.insert( p );
    }

  for( std::multimap<point_t,point_t>::iterator seedit = mmap.begin();
       seedit != mmap.end();
       seedit = mmap.begin() )       
    {
      std::vector<point_t> poly;
      
      while( seedit != mmap.end() ) 
	{
	  // invert y axis and add the new point to the poly
	  point_t pt = seedit->first;
	  pt.y = -pt.y;
	  
	  // can this vector simply extend the previous one?
	  size_t psize = poly.size();
	  if( psize > 2 ) // need at least two points already
	    {
	      // find the direction of the vector descrived by the two previous points
	      double ldx = direction( poly[psize-1].x - poly[psize-2].x );
	      double ldy = direction( poly[psize-1].y - poly[psize-2].y );
	      
	      // find the direction of the vector described by the new point and the previous point
	      double ndx = direction( pt.x - poly[psize-1].x );
	      double ndy = direction( pt.y - poly[psize-1].y );
	      
	      // if the direction is the same, we can replace the
	      // previous point with this one, rather than adding a
	      // new point
	      if( ldx == ndx && ldy == ndy )
		poly[psize-1] = pt;	      
	      else
		poly.push_back( pt );      
	    }
	  else
	    poly.push_back( pt ); 
	  
	  point_t next = seedit->second;
	  mmap.erase( seedit );
	  
	  seedit = mmap.find( next );      
	} 
      
      polys.push_back( poly );
    }
  
  if( img ) img->release(); // frees all resources for this image
  return 0; // ok
}

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

