// Author: Richard Vaughan

#include <errno.h>
#include <FL/Fl_Shared_Image.H>

#include "stage.hh"
#include "config.h" // results of cmake's system configuration tests
#include "file_manager.hh"
using namespace Stg;

static bool init_called = false;

stg_typetable_entry_t Stg::typetable[MODEL_TYPE_COUNT];


const char* Stg::Version()
{ 
  return VERSION; 
}

void Stg::Init( int* argc, char** argv[] )
{
	PRINT_DEBUG( "Stg::Init()" );

	// seed the RNG 
	srand48( time(NULL) );

	if(!setlocale(LC_ALL,"POSIX"))
		PRINT_WARN("Failed to setlocale(); config file may not be parse correctly\n" );
						
	if (!g_thread_supported ()) g_thread_init (NULL);

	//g_thread_init( NULL );

	RegisterModels();

	init_called = true;

	// ask FLTK to load support for various image formats
	fl_register_images();
}

bool Stg::InitDone()
{
	return init_called;
}

void Stg::RegisterModel( stg_model_type_t type, 
								 const char* name, 
								 stg_creator_t creator )
{
  Stg::typetable[ type ].token = name;
  Stg::typetable[ type ].creator = creator;
}
  


void Stg::stg_print_err( const char* err )
{
	printf( "Stage error: %s\n", err );
	//_stg_quit = TRUE;
}


void Stg::stg_print_velocity( const Velocity& vel )
{
	printf( "velocity [x:%.3f y:%.3f a:%.3f]\n",
			vel.x, vel.y, vel.a );
}


// Look up the color in a database.  (i.e. transform color name to
// color value).  If the color is not found in the database, a bright
// red color will be returned instead.
stg_color_t Stg::stg_lookup_color(const char *name)
{
	if( name == NULL ) // no string?
		return 0; // black

	if( strcmp( name, "" ) == 0 ) // empty string?
		return 0; // black

	static FILE *file = NULL;
	static GHashTable* table = g_hash_table_new( g_str_hash, g_str_equal );

	if( file == NULL )
	{
		std::string rgbFile = FileManager::findFile( "rgb.txt" );
		file = fopen( rgbFile.c_str(), "r" );
		
		if( file == NULL )
		{
			PRINT_ERR1("unable to open color database: %s "
					   "(try adding rgb.txt's location to your STAGEPATH)",
					   strerror(errno));
			exit(0);
		}

		PRINT_DEBUG( "Success!" );

		// load the file into the hash table       
		while (TRUE)
		{
			char line[1024];
			if (!fgets(line, sizeof(line), file))
				break;

			// it's a macro or comment line - ignore the line
			if (line[0] == '!' || line[0] == '#' || line[0] == '%') 
				continue;

			// Trim the trailing space
			while (strchr(" \t\n", line[strlen(line)-1]))
				line[strlen(line)-1] = 0;

			// Read the color
			int r, g, b;
			int chars_matched = 0;
			sscanf( line, "%d %d %d %n", &r, &g, &b, &chars_matched );


			stg_color_t* col = new stg_color_t;
			*col = ( 0xFF000000 | (r << 16) | (g << 8) | b);

			// Read the name
			char* colorname = strdup( line + chars_matched );

			// map the name to the color in the table
			g_hash_table_insert( table, (gpointer)colorname, (gpointer)col );
		}
		fclose(file);
	}

	// look up the colorname in the database  
	stg_color_t* found = (stg_color_t*)g_hash_table_lookup( table, name );

	if( found )
		return *found;
	else
		return (stg_color_t)0;
}

static guchar* pb_get_pixel( Fl_Shared_Image* img, int x, int y )
{
  guchar* pixels = (guchar*)(img->data()[0]);
  unsigned int index = (y * img->w() * img->d()) + (x * img->d());
  return( pixels + index );
}

// set all the pixels in a rectangle 
static void pb_set_rect( Fl_Shared_Image* pb, int x, int y, int width, int height, uint8_t val )
{
	int bytes_per_sample = 1;
	int num_samples = pb->d();

	int a, b;
	for( a = y; a < y+height; a++ )
		for( b = x; b < x+width; b++ )
		{	
			// zeroing
			guchar* pix = pb_get_pixel( pb, b, a );
			memset( pix, val, num_samples * bytes_per_sample );
		}
}  

// returns true if the value in the first channel is above threshold
static gboolean pb_pixel_is_set( Fl_Shared_Image* img, int x, int y, int threshold )
{
	guchar* pixel = pb_get_pixel( img,x,y );
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


stg_color_t Stg::stg_color_pack( double r, double g, double b, double a )
{
	stg_color_t col=0;
	col += (stg_color_t)((1.0-a)*255.0)<<24;
	col += (stg_color_t)(r*255.0)<<16;
	col += (stg_color_t)(g*255.0)<<8;
	col += (stg_color_t)(b*255.0);

	return col;
}

void Stg::stg_color_unpack( stg_color_t col, 
		double* r, double* g, double* b, double* a )
{
	if(a) *a = 1.0 - (((col & 0xFF000000) >> 24) / 255.0);
	if(r) *r = ((col & 0x00FF0000) >> 16) / 255.0;
	if(g) *g = ((col & 0x0000FF00) >> 8)  / 255.0;
	if(b) *b = ((col & 0x000000FF) >> 0)  / 255.0;
}


// DRAW INTERFACE

using namespace Draw;

draw_t* Draw::create( type_t type,
		vertex_t* verts,
		size_t vert_count )
{
	size_t vert_mem_size = vert_count * sizeof(vertex_t);

	// allocate space for the draw structure and the vertex data behind it
	draw_t* d = (draw_t*)
		g_malloc( sizeof(draw_t) + vert_mem_size );

	d->type = type;
	d->vert_count = vert_count;

	// copy the vertex data behind the draw structure
	memcpy( d->verts, verts, vert_mem_size );

	return d;
}

void Draw::destroy( draw_t* d )
{
	g_free( d );
}
