// Author: Richard Vaughan

#include <FL/Fl_Shared_Image.H>

#include "stage.hh"
#include "config.h" // results of cmake's system configuration tests
#include "file_manager.hh"
using namespace Stg;

// static member - a vector command line arguments
std::vector<std::string> World::args;

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
						
  //if (!g_thread_supported ()) g_thread_init (NULL);
	
  RegisterModels();
	
  init_called = true;
	
  // ask FLTK to load support for various image formats
  fl_register_images();
}

bool Stg::InitDone()
{
  return init_called;
}

static inline uint8_t* pb_get_pixel( Fl_Shared_Image* img, int x, int y )
{
  uint8_t* pixels = (uint8_t*)(img->data()[0]);
  unsigned int index = (y * img->w() * img->d()) + (x * img->d());
  return( pixels + index );
}

// set all the pixels in a rectangle 
static inline void pb_set_rect( Fl_Shared_Image* pb, int x, int y, int width, int height, uint8_t val )
{
  int bytes_per_sample = 1;
  int num_samples = pb->d();

  int a, b;
  for( a = y; a < y+height; a++ )
	for( b = x; b < x+width; b++ )
	  {	
		// zeroing
		uint8_t* pix = pb_get_pixel( pb, b, a );
		memset( pix, val, num_samples * bytes_per_sample );
	  }
}  

// returns true if the value in the first channel is above threshold
static inline bool pb_pixel_is_set( Fl_Shared_Image* img, int x, int y, int threshold )
{
  uint8_t* pixel = pb_get_pixel( img,x,y );
  return( pixel[0] > threshold );
}

int Stg::stg_rotrects_from_image_file( const char* filename, 
									   stg_rotrect_t** rects, 
									   unsigned int* rect_count,
									   unsigned int* widthp, 
									   unsigned int* heightp )
{
  // TODO: make this a parameter
  const int threshold = 127;

  Fl_Shared_Image *img = Fl_Shared_Image::get(filename);
  if( img == NULL ) {
	std::cerr << "failed to open file: " << filename << std::endl;
	assert( img );
  }

  //printf( "loaded image %s w %d h %d d %d count %d ld %d\n", 
  //  filename, img->w(), img->h(), img->d(), img->count(), img->ld() );

  *rect_count = 0;
  size_t allocation_unit = 1000;
  size_t rects_allocated = allocation_unit;
  *rects = (stg_rotrect_t*)calloc( sizeof(stg_rotrect_t), rects_allocated );

  int img_width = img->w();
  int img_height = img->h();

  // if the caller wanted to know the dimensions
  if( widthp ) *widthp = img_width;
  if( heightp ) *heightp = img_height;

  for(int y = 0; y < img_height; y++)
	{
	  for(int x = 0; x < img_width; x++)
		{
		  // skip blank (white) pixels
		  if(  pb_pixel_is_set( img,x,y, threshold) )
			continue;

		  // a rectangle starts from this point
		  int startx = x;
		  int starty = y;
		  int height = img_height; // assume full height for starters

		  // grow the width - scan along the line until we hit an empty (white) pixel
		  for( ; x < img_width &&  ! pb_pixel_is_set(img,x,y,threshold); x++ )
			{
			  // handle horizontal cropping
			  //double ppx = x * sx;
			  //if (ppx < this->crop_ax || ppx > this->crop_bx)
			  //continue;

			  // look down to see how large a rectangle below we can make
			  int yy  = y;
			  while( ! pb_pixel_is_set(img,x,yy,threshold) && (yy < img_height-1) )
				{ 
				  // handle vertical cropping
				  //double ppy = (this->image->height - yy) * sy;
				  //if (ppy < this->crop_ay || ppy > this->crop_by)
				  //continue;

				  yy++;
				} 	      

			  // now yy is the depth of a line of non-zero pixels
			  // downward we store the smallest depth - that'll be the
			  // height of the rectangle
			  if( yy-y < height ) height = yy-y; // shrink the height to fit
			} 

		  // whiten the pixels we have used in this rect
		  pb_set_rect( img, startx, starty, x-startx, height, 0xFF );

		  // add this rectangle to the array
		  (*rect_count)++;

		  if( (*rect_count) > rects_allocated )
			{
			  rects_allocated = (*rect_count) + allocation_unit;

			  *rects = (stg_rotrect_t*)
				realloc( *rects, rects_allocated * sizeof(stg_rotrect_t) );
			}

		  //  y-invert all the rectangles because we're using conventional
		  // rather than graphics coordinates. this is much faster than
		  // inverting the original image.
		  stg_rotrect_t *latest = &(*rects)[(*rect_count)-1];
		  latest->pose.x = startx;
		  latest->pose.y = img_height-1 - (starty + height);
		  latest->pose.a = 0.0;
		  latest->size.x = x - startx;
		  latest->size.y = height;

		  assert( latest->pose.x >= 0 );
		  assert( latest->pose.y >= 0 );
		  assert( latest->pose.x <= img_width );
		  assert( latest->pose.y <= img_height);
		  //assert( latest->size.x > 0 );
		  //assert( latest->size.y > 0 );

		  // 			if( latest->size.x < 1  || latest->size.y < 1 )
		  // 			  printf( "p [%.2f %.2f] s [%.2f %.2f]\n", 
		  // 						 latest->pose.x, latest->pose.y, latest->size.x, latest->size.y );

		  //printf( "rect %d (%.2f %.2f %.2f %.2f %.2f\n", 
		  //  *rect_count, 
		  //  latest->x, latest->y, latest->a, latest->w, latest->h );

		}
	}

  if( img ) img->release(); // frees all image resources: the
  // constructor is not available

  return 0; // ok
}

// POINTS -----------------------------------------------------------

stg_point_t* Stg::stg_unit_square_points_create( void )
{
  stg_point_t * pts = new stg_point_t[4];

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
double Stg::constrain( double val, double minval, double maxval )
{
  if( val < minval )
	return minval;

  if( val > maxval )
	return maxval;

  return val;
}

