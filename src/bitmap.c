
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

int stg_load_image( const char* filename, 
		    stg_rotrect_t** rects, 
		    int* rect_count,
		    int* widthp, int* heightp )
{
  g_type_init(); // glib GObject initialization

  GError* err = NULL;
  GdkPixbuf* pb = gdk_pixbuf_new_from_file( filename, &err );

  if( err )
    {
      fprintf( stderr, "\nError loading bitmap: %s\n", err->message );
      return 1; // error
    }
  
  // this should be ok as no error was reported
  assert( pb );
  
  
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
  
  // if the caller wanted to know the dimensions
  if( widthp ) *widthp = img_width;
  if( heightp ) *heightp = img_height;
  
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
	  latest->pose.x = startx;
	  latest->pose.y = starty;
	  latest->pose.a = 0.0;
	  latest->size.x = x - startx;
	  latest->size.y = height;
	  
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
      rect->pose.y = img_height - rect->pose.y;
      rect->size.y = -rect->size.y;
    }
  

  return 0; // ok
}


/* int stg_load_image_polygons( const char* filename,  */
/* 			     stg_polygon_t** polys,  */
/* 			     int* poly_count, */
/* 			     int* widthp, int* heightp ) */
/* { */
/*   g_type_init(); // glib GObject initialization */

/*   GError* err = NULL; */
/*   GdkPixbuf* pb = gdk_pixbuf_new_from_file( filename, &err ); */

/*   if( err ) */
/*     { */
/*       fprintf( stderr, "\nError loading bitmap: %s\n", err->message ); */
/*       return 1; // error */
/*     } */
  
/*   // this should be ok as no error was reported */
/*   assert( pb ); */
  
  
/* #ifdef DEBUG */
/*   printf( "image \"%s\" channels:%d bits:%d alpha:%d " */
/* 	  "width:%d height:%d rowstride:%d pixels:%p\n", */
	  
/* 	  filename, */
/* 	  gdk_pixbuf_get_n_channels(pb), */
/* 	  gdk_pixbuf_get_bits_per_sample(pb), */
/* 	  gdk_pixbuf_get_has_alpha(pb),	       */
/* 	  gdk_pixbuf_get_width(pb), */
/* 	  gdk_pixbuf_get_height(pb), */
/* 	  gdk_pixbuf_get_rowstride(pb), */
/* 	  gdk_pixbuf_get_pixels(pb) ); */
/* #endif */
  
/*   *ploy_count = 0; */
/*   *polys = NULL; */
  
/*   int img_width = gdk_pixbuf_get_width(pb); */
/*   int img_height = gdk_pixbuf_get_height(pb); */
  
/*   // if the caller wanted to know the dimensions */
/*   if( widthp ) *widthp = img_width; */
/*   if( heightp ) *heightp = img_height; */
  
/*   int y, x; */
/*   for(y = 0; y < img_height; y++) */
/*     { */
/*       for(x = 0; x < img_width; x++) */
/* 	{ */
/* 	  // skip blank pixels */
/* 	  if( ! pb_pixel_is_set( pb,x,y) ) */
/* 	    continue; */
	  
/* 	  // a rectangle starts from this point */
/* 	  int startx = x; */
/* 	  int starty = y; */
/* 	  int height = img_height; // assume full height for starters */
	  
/* 	  // grow the width - scan along the line until we hit an empty pixel */
/* 	  for( ; x < img_width &&  pb_pixel_is_set(pb,x,y); x++ ) */
/* 	    { */
/* 	      // handle horizontal cropping */
/* 	      //double ppx = x * sx;  */
/* 	      //if (ppx < this->crop_ax || ppx > this->crop_bx) */
/* 	      //continue; */
	      
/* 	      // look down to see how large a rectangle below we can make */
/* 	      int yy  = y; */
/* 	      while( pb_pixel_is_set(pb,x,yy) && (yy < img_height-1) ) */
/* 		{  */
/* 		  // handle vertical cropping */
/* 		  //double ppy = (this->image->height - yy) * sy; */
/* 		  //if (ppy < this->crop_ay || ppy > this->crop_by) */
/* 		  //continue; */
		  
/* 		  yy++;  */
/* 		} 	       */

/* 	      // now yy is the depth of a line of non-zero pixels */
/* 	      // downward we store the smallest depth - that'll be the */
/* 	      // height of the rectangle */
/* 	      if( yy-y < height ) height = yy-y; // shrink the height to fit */
/* 	    }  */
	  
/* 	  // delete the pixels we have used in this rect */
/* 	  pb_zero_rect( pb, startx, starty, x-startx, height ); */
	  
/* 	  // add this rectangle to the array */
/* 	  (*poly_count)++; */
/* 	  *polys = (stg_polygo_t*) */
/* 	    realloc( *polys, *poly_count * sizeof(stg_polygon_t) ); */

/* 	  // stash the 4 points */
/* 	  stg_point_t* pts = (stg_point_t*)calloc( 4, sizeof(stg_point_t) ); */
/* 	  pts[0].x = startx; */
/* 	  pts[0].y = starty; */
/* 	  pts[1].x = x;  */
/* 	  pts[1].y = starty; */
/* 	  pts[2].x = x; */
/* 	  pts[2].y = starty + height; */
/* 	  pts[3].x = startx; */
/* 	  pts[3].y = starty + height; */
	  
/* 	  // and put the points into the polygon	   */
/* 	  stg_polygon_t latest = (*polys)[(*poly_count)-1]; */
/* 	  latest->points = pts; */
/* 	  latest->point_count = 4; */
	  	  
/* 	  //printf( "rect %d (%.2f %.2f %.2f %.2f %.2f\n",  */
/* 	  //  *rect_count,  */
/* 	  //  latest->x, latest->y, latest->a, latest->w, latest->h );  */
	  
/* 	} */
/*     } */
  
/*   // free the image data */
/*   gdk_pixbuf_unref( pb ); */

/*   // now y-invert all the rectangles because we're using conventional */
/*   // rather than graphics coordinates. this is much faster than */
/*   // inverting the original image. */
/*   int r; */
/*   for( r=0; r< *rect_count; r++ ) */
/*     { */
/*       stg_rotrect_t *rect = &(*rects)[r];  */
/*       rect->pose.y = img_height - rect->pose.y; */
/*       rect->size.y = -rect->size.y; */
/*     } */
  

/*   return 0; // ok */
/* } */


/// return an array of [count] polygons. Caller must free() the space.
stg_polygon_t* stg_polygons_create( int count )
{
  stg_polygon_t* polys = (stg_polygon_t*)calloc( count, sizeof(stg_polygon_t));
  
  // each polygon contains an array of points
  int p;
  for( p=0; p<count; p++ )
    polys[p].points = g_array_new( FALSE, TRUE, sizeof(stg_point_t));

  return polys;
}

/// destroy an array of polygons
void stg_polygons_destroy( stg_polygon_t* p, size_t count )
{
  int c;
  for( c=0; c<count; c++ )
    if( p[c].points )
      g_array_free( p[c].points, TRUE );
  
  free( p );      
}

/// return a single polygon structure. Caller  must free() the space.
stg_polygon_t* stg_polygon_create( void )
{
  return stg_polygons_create( 1 );
}

/// destroy a single polygon
void stg_polygon_destroy( stg_polygon_t* p )
{
  stg_polygons_destroy( p, 1 );
}

stg_point_t* stg_points_create( size_t count )
{
  return( (stg_point_t*)calloc( count, sizeof(stg_point_t)));
}

stg_point_t* stg_point_create( void )
{
  return stg_points_create(1);
}

void stg_points_destroy( stg_point_t* pts )
{
  free( pts );
}

void stg_point_destroy( stg_point_t* pt )
{
  free( pt );
}

/// Copies [count] points from [pts] into polygon [poly], allocating
/// memory if mecessary. Any previous points in [poly] are
/// overwritten.
void stg_polygon_set_points( stg_polygon_t* poly, stg_point_t* pts, size_t count )
{
  assert( poly );
  
  g_array_set_size( poly->points, 0 );
  g_array_append_vals( poly->points, pts, count );
}

/// Appends [count] points from [pts] to the point list of polygin
/// [poly], allocating memory if mecessary.
void stg_polygon_append_points( stg_polygon_t* poly, stg_point_t* pts, size_t count )
{
  assert( poly );
  g_array_append_vals( poly->points, pts, count );
}
