
#include <stdlib.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include "stage.h"

// pb_* functions are only used inside this file

guchar* pb_get_pixel( GdkPixbuf* pb, int x, int y )
{
  guchar* pixels = gdk_pixbuf_get_pixels(pb);
  int rs = gdk_pixbuf_get_rowstride(pb);
  int ch = gdk_pixbuf_get_n_channels(pb);
  return( pixels + y * rs + x * ch );
}

void pb_zero_pixel( GdkPixbuf* pb, int x, int y )
{
  // bounds checking
  int width = gdk_pixbuf_get_width(pb);
  int height = gdk_pixbuf_get_height(pb);
  if( x >=0 && x < width && y >= 0 && y < height )
    {
      // zeroing
      guchar* pix = pb_get_pixel( pb, x, y );
      int bytes_per_sample = gdk_pixbuf_get_bits_per_sample (pb) / 8;
      int num_samples = gdk_pixbuf_get_n_channels(pb);
      memset( pix, 0, num_samples * bytes_per_sample );
    }
  else
    PRINT_WARN4( "zero pixel %d,%d out of range (image dimensions %d by %d)", x, y, width, height );
}

// zero all the pixels in a rectangle 
void pb_zero_rect( GdkPixbuf* pb, int x, int y, int width, int height )
{
  //todo - this could be faster - improve it if it gets used a lot)
  int a, b;
  for( a = y; a < y+height; a++ )
    for( b = x; b < x+width; b++ )
      pb_zero_pixel( pb,b,a );
}  

// returns TRUE if any channel in the pixel is non-zero
gboolean pb_pixel_is_set( GdkPixbuf* pb, int x, int y )
{
  guchar* pixel = pb_get_pixel( pb,x,y );
  //int channels = gdk_pixbuf_get_n_channels(pb);

  //int i;
  //for( i=0; i<channels; i++ )
  //if( pixel[i] ) return TRUE;
  if( pixel[0] ) return TRUE; // just use the red channel for now

  return FALSE;
}

int stg_load_image( const char* filename, stg_rotrect_t** rects, int* rect_count )
{
  g_type_init(); // glib GObject initialization

  GError* err = NULL;
  GdkPixbuf* pb = gdk_pixbuf_new_from_file( filename, &err );

  if( pb == NULL )
    {
      // TODO - use the GError properly here
      PRINT_ERR( "error loading world bitmap" );
      return 1; // error
    }

#ifdef DEBUG
  printf( "image \"%s\" channels:%d bits:%d alpha:%d "
	  "width:%d height:%d rowstride:%d pixels:%p\n",
	  
	  filename,
	  gdk_pixbuf_get_n_channels(pb),
	  gdk_pixbuf_get_bits_per_sample(pb),
	  gdk_pixbuf_get_has_alpha(pb),	      
	  gdk_pixbuf_get_width(pb),
	  gdk_pixbuf_get_height(pb),
	  gdk_pixbuf_get_rowstride(pb),
	  gdk_pixbuf_get_pixels(pb) );
#endif

  *rect_count = 0;
  *rects = NULL;
  
  int img_width = gdk_pixbuf_get_width(pb);
  int img_height = gdk_pixbuf_get_height(pb);
  
  int y, x;
  for(y = 0; y < img_height; y++)
    {
      for(x = 0; x < img_width; x++)
	{
	  // skip blank pixels
	  if( ! pb_pixel_is_set( pb,x,y) )
	    continue;
	  
	  // a rectangle starts from this point
	  int startx = x;
	  int starty = y;
	  int height = img_height; // assume full height for starters
	  
	  // grow the width - scan along the line until we hit an empty pixel
	  for( ; x < img_width &&  pb_pixel_is_set(pb,x,y); x++ )
	    {
	      // handle horizontal cropping
	      //double ppx = x * sx; 
	      //if (ppx < this->crop_ax || ppx > this->crop_bx)
	      //continue;
	      
	      // look down to see how large a rectangle below we can make
	      int yy  = y;
	      while( pb_pixel_is_set(pb,x,yy) && (yy < img_height-1) )
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
	  
	  // delete the pixels we have used in this rect
	  pb_zero_rect( pb, startx, starty, x-startx, height );
	  
	  // add this rectangle to the array
	  (*rect_count)++;
	  *rects = (stg_rotrect_t*)
	    realloc( *rects, *rect_count * sizeof(stg_rotrect_t) );
	  
	  stg_rotrect_t *latest = &(*rects)[(*rect_count)-1];
	  latest->x = startx;
	  latest->y = starty;
	  latest->a = 0.0;
	  latest->w = x - startx;
	  latest->h = height;
	  
	  //printf( "rect %d (%.2f %.2f %.2f %.2f %.2f\n", 
	  //  *rect_count, 
	  //  latest->x, latest->y, latest->a, latest->w, latest->h ); 
	  
	}
    }
  
  // free the image data
  gdk_pixbuf_unref( pb );

  // now y-invert all the rectangles because we're using conventional
  // rather than graphics coordinates. this is much faster than
  // inverting the original image.
  int r;
  for( r=0; r< *rect_count; r++ )
    {
      stg_rotrect_t *rect = &(*rects)[r]; 
      rect->y = img_height - rect->y;
      rect->h = -rect->h;
    }
  

  return 0; // ok
}

