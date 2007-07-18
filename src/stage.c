#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <assert.h>
#include <unistd.h>
#include <math.h>
#include <glib.h>
#include <locale.h>

#include <gdk-pixbuf/gdk-pixbuf.h>

//#define DEBUG

//#include "replace.h"
#include "stage_internal.h"

int _stg_quit = FALSE;
int _stg_disable_gui = FALSE;

GHashTable* global_typetable = NULL;
GHashTable* stg_create_typetable( void );

int stg_init( int argc, char** argv )
{
  g_type_init(); // glib GObject initialization

  if( ! _stg_disable_gui )
    {
      // TODO - don't start the GUI if it was disabled
      //puts( "GUI_STARTUP" );
      gui_startup( &argc, &argv );

      //int debug_gtk_argc = 2;
      //char* debug_gtk_argv[2];
      //debug_gtk_argv[0] = argv[0];
      //debug_gtk_argv[1] = "--g-fatal-warnings";
      //gui_startup( &debug_gtk_argc, &debug_gtk_argv );
    }

  // this forces use of decimal points in the config file rather than
  // euro-style commas. Do this after gui_startup() as GTK messes with
  // locale.
  if(!setlocale(LC_ALL,"POSIX"))
    fputs("Warning: failed to setlocale(); config file may not be parse correctly\n", stderr);


  // load the type table into a hash table
  global_typetable = stg_create_typetable();

  return 0; // ok
}

const char* stg_version_string( void )
{
  return PACKAGE_STRING;
}

/* const char* stg_model_type_string( stg_model_type_t type ) */
/* { */
/*   switch( type ) */
/*     { */
/*     case STG_MODEL_BASIC: return "model"; */
/*     case STG_MODEL_LASER: return "laser"; */
/*     case STG_MODEL_POSITION: return "position"; */
/*     case STG_MODEL_BLOB: return "blobfinder"; */
/*     case STG_MODEL_FIDUCIAL: return "fiducial"; */
/*     case STG_MODEL_RANGER: return "ranger"; */
/*       //case STG_MODEL_TEST: return "test"; */
/*     case STG_MODEL_GRIPPER: return "gripper"; */
/*     default: */
/*       break; */
/*     }   */
/*   return "<unknown type>"; */
/* } */

void stg_print_err( const char* err )
{
  printf( "Stage error: %s\n", err );
  _stg_quit = TRUE;
}


void stg_print_geom( stg_geom_t* geom )
{
  printf( "geom pose: (%.2f,%.2f,%.2f) size: [%.2f,%.2f]\n",
	  geom->pose.x,
	  geom->pose.y,
	  geom->pose.a,
	  geom->size.x,
	  geom->size.y );
}


void stg_print_pose( stg_pose_t* pose )
{
  printf( "pose [x:%.3f y:%.3f a:%.3f]\n",
	  pose->x, pose->y, pose->a );
}

void stg_print_velocity( stg_velocity_t* vel )
{
  printf( "velocity [x:%.3f y:%.3f a:%.3f]\n",
	  vel->x, vel->y, vel->a );
}

stg_msec_t stg_realtime( void )
{
  struct timeval tv;
  gettimeofday( &tv, NULL );  
  stg_msec_t timenow = (stg_msec_t)( tv.tv_sec*1000 + tv.tv_usec/1000 );
  return timenow;
}

stg_msec_t stg_realtime_since_start( void )
{
  static stg_msec_t starttime = 0;  
  stg_msec_t timenow = stg_realtime();
  
  if( starttime == 0 )
    starttime = timenow;
    
  return( timenow - starttime );
}


// if stage wants to quit, this will return non-zero
int stg_quit_test( void )
{
  return _stg_quit;
}

void stg_quit_request( void )
{
  _stg_quit = 1;
}

void stg_quit_cancel( void )
{
  _stg_quit = 0;
}

// Look up the color in a database.  (i.e. transform color name to
// color value).  If the color is not found in the database, a bright
// red color will be returned instead.
stg_color_t stg_lookup_color(const char *name)
{
  FILE *file;
  const char *filename;
  
  if( name == NULL ) // no string?
    return 0; // black
  
  if( strcmp( name, "" ) == 0 ) // empty string?
    return 0; // black

  filename = COLOR_DATABASE;
  file = fopen(filename, "r");
  if (!file)
  {
    PRINT_ERR2("unable to open color database %s : %s",
               filename, strerror(errno));
    fclose(file);
    return 0xFFFFFF;
  }
  
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
      
    // Read the name
    char* nname = line + chars_matched;

    // If the name matches
    if (strcmp(nname, name) == 0)
    {
      fclose(file);
      return ((r << 16) | (g << 8) | b);
    }
  }
  PRINT_WARN1("unable to find color [%s]; using default (red)", name);
  fclose(file);
  return 0xFF0000;
}



//////////////////////////////////////////////////////////////////////////
// scale an array of rectangles so they fit in a unit square
void stg_lines_normalize( stg_line_t* lines, int num )
{
  // assuming the rectangles fit in a square +/- one billion units
  double minx, miny, maxx, maxy;
  minx = miny = BILLION;
  maxx = maxy = -BILLION;
  
  int l;
  for( l=0; l<num; l++ )
    {
      // find the bounding rectangle
      if( lines[l].x1 < minx ) minx = lines[l].x1;
      if( lines[l].y1 < miny ) miny = lines[l].y1;      
      if( lines[l].x1 > maxx ) maxx = lines[l].x1;      
      if( lines[l].y1 > maxy ) maxy = lines[l].y1;
      if( lines[l].x2 < minx ) minx = lines[l].x2;
      if( lines[l].y2 < miny ) miny = lines[l].y2;      
      if( lines[l].x2 > maxx ) maxx = lines[l].x2;      
      if( lines[l].y2 > maxy ) maxy = lines[l].y2;
    }
  
  // now normalize all lengths so that the lines all fit inside
  // rectangle from 0,0 to 1,1
  double scalex = maxx - minx;
  double scaley = maxy - miny;

  for( l=0; l<num; l++ )
    { 
      lines[l].x1 = (lines[l].x1 - minx) / scalex;
      lines[l].y1 = (lines[l].y1 - miny) / scaley;
      lines[l].x2 = (lines[l].x2 - minx) / scalex;
      lines[l].y2 = (lines[l].y2 - miny) / scaley;
    }
}

void stg_lines_scale( stg_line_t* lines, int num, double xscale, double yscale )
{
  int l;
  for( l=0; l<num; l++ )
    {
      lines[l].x1 *= xscale;
      lines[l].y1 *= yscale;
      lines[l].x2 *= xscale;
      lines[l].y2 *= yscale;
    }
}

void stg_lines_translate( stg_line_t* lines, int num, double xtrans, double ytrans )
{
  int l;
  for( l=0; l<num; l++ )
    {
      lines[l].x1 += xtrans;
      lines[l].y1 += ytrans;
      lines[l].x2 += xtrans;
      lines[l].y2 += ytrans;
    }
}

//////////////////////////////////////////////////////////////////////////
// scale an array of rectangles so they fit in a unit square
void stg_rotrects_normalize( stg_rotrect_t* rects, int num )
{
  // assuming the rectangles fit in a square +/- one billion units
  double minx, miny, maxx, maxy;
  minx = miny = BILLION;
  maxx = maxy = -BILLION;
  
  int r;
  for( r=0; r<num; r++ )
    {
      // test the origin of the rect
      if( rects[r].pose.x < minx ) minx = rects[r].pose.x;
      if( rects[r].pose.y < miny ) miny = rects[r].pose.y;      
      if( rects[r].pose.x > maxx ) maxx = rects[r].pose.x;      
      if( rects[r].pose.y > maxy ) maxy = rects[r].pose.y;

      // test the extremes of the rect
      if( (rects[r].pose.x+rects[r].size.x)  < minx ) 
	minx = (rects[r].pose.x+rects[r].size.x);
      
      if( (rects[r].pose.y+rects[r].size.y)  < miny ) 
	miny = (rects[r].pose.y+rects[r].size.y);
      
      if( (rects[r].pose.x+rects[r].size.x)  > maxx ) 
	maxx = (rects[r].pose.x+rects[r].size.x);
      
      if( (rects[r].pose.y+rects[r].size.y)  > maxy ) 
	maxy = (rects[r].pose.y+rects[r].size.y);
    }
  
  // now normalize all lengths so that the rects all fit inside
  // rectangle from 0,0 to 1,1
  double scalex = maxx - minx;
  double scaley = maxy - miny;

  for( r=0; r<num; r++ )
    { 
      rects[r].pose.x = (rects[r].pose.x - minx) / scalex;
      rects[r].pose.y = (rects[r].pose.y - miny) / scaley;
      rects[r].size.x = rects[r].size.x / scalex;
      rects[r].size.y = rects[r].size.y / scaley;
    }
}	

// returns an array of 4 * num_rects stg_line_t's
stg_line_t* stg_rotrects_to_lines( stg_rotrect_t* rects, int num_rects )
{
  // convert rects to an array of lines
  int num_lines = 4 * num_rects;
  stg_line_t* lines = (stg_line_t*)calloc( sizeof(stg_line_t), num_lines );
  
  int r;
  for( r=0; r<num_rects; r++ )
    {
      lines[4*r].x1 = rects[r].pose.x;
      lines[4*r].y1 = rects[r].pose.y;
      lines[4*r].x2 = rects[r].pose.x + rects[r].size.x;
      lines[4*r].y2 = rects[r].pose.y;
      
      lines[4*r+1].x1 = rects[r].pose.x + rects[r].size.x;;
      lines[4*r+1].y1 = rects[r].pose.y;
      lines[4*r+1].x2 = rects[r].pose.x + rects[r].size.x;
      lines[4*r+1].y2 = rects[r].pose.y + rects[r].size.y;
      
      lines[4*r+2].x1 = rects[r].pose.x + rects[r].size.x;;
      lines[4*r+2].y1 = rects[r].pose.y + rects[r].size.y;;
      lines[4*r+2].x2 = rects[r].pose.x;
      lines[4*r+2].y2 = rects[r].pose.y + rects[r].size.y;
      
      lines[4*r+3].x1 = rects[r].pose.x;
      lines[4*r+3].y1 = rects[r].pose.y + rects[r].size.y;
      lines[4*r+3].x2 = rects[r].pose.x;
      lines[4*r+3].y2 = rects[r].pose.y;
    }
  
  return lines;
}

// sets [result] to the pose of [p2] in [p1]'s coordinate system
void stg_pose_sum( stg_pose_t* result, stg_pose_t* p1, stg_pose_t* p2 )
{
  double cosa = cos(p1->a);
  double sina = sin(p1->a);
  
  double tx = p1->x + p2->x * cosa - p2->y * sina;
  double ty = p1->y + p2->x * sina + p2->y * cosa;
  double ta = p1->a + p2->a;
  
  result->x = tx;
  result->y = ty;
  result->a = ta;
}


// pb_* functions are only used inside this file

guchar* pb_get_pixel( GdkPixbuf* pb, int x, int y )
{
  guchar* pixels = gdk_pixbuf_get_pixels(pb);
  int rs = gdk_pixbuf_get_rowstride(pb);
  int ch = gdk_pixbuf_get_n_channels(pb);
  return( pixels + y * rs + x * ch );
}

void pb_set_pixel( GdkPixbuf* pb, int x, int y, uint8_t val )
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
      memset( pix, val, num_samples * bytes_per_sample );
    }
  else
    PRINT_WARN4( "pb_set_pixel coordinate %d,%d out of range (image dimensions %d by %d)", x, y, width, height );
}

// set all the pixels in a rectangle 
void pb_set_rect( GdkPixbuf* pb, int x, int y, int width, int height, uint8_t val )
{
  //int pbwidth = gdk_pixbuf_get_width(pb);
  //int pbheight = gdk_pixbuf_get_height(pb);
  int bytes_per_sample = gdk_pixbuf_get_bits_per_sample (pb) / 8;
  int num_samples = gdk_pixbuf_get_n_channels(pb);

  int a, b;
  for( a = y; a < y+height; a++ )
    for( b = x; b < x+width; b++ )
      {	
	// zeroing
	guchar* pix = pb_get_pixel( pb, b, a );
	memset( pix, val, num_samples * bytes_per_sample );
      }
}  

// returns TRUE if any channel in the pixel is non-zero
gboolean pb_pixel_is_set( GdkPixbuf* pb, int x, int y, int threshold )
{
  guchar* pixel = pb_get_pixel( pb,x,y );
  //int channels = gdk_pixbuf_get_n_channels(pb);
  //int i;
  //for( i=0; i<channels; i++ )
  //if( pixel[i] ) return TRUE;
  if( pixel[0] > threshold ) return TRUE; // just use the red channel for now

  return FALSE;
}

int stg_rotrects_from_image_file( const char* filename, 
				  stg_rotrect_t** rects, 
				  unsigned int* rect_count,
				  unsigned int* widthp, 
				  unsigned int* heightp )
{
  // TODO: make this a parameter
  const int threshold = 127;

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
  size_t allocation_unit = 1000;
  size_t rects_allocated = allocation_unit;
  *rects = (stg_rotrect_t*)malloc( rects_allocated * sizeof(stg_rotrect_t) );
  
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
	  // skip blank (white) pixels
	  if(  pb_pixel_is_set( pb,x,y, threshold) )
	    continue;
	  
	  // a rectangle starts from this point
	  int startx = x;
	  int starty = y;
	  int height = img_height; // assume full height for starters
	  
	  // grow the width - scan along the line until we hit an empty (white) pixel
	  for( ; x < img_width &&  ! pb_pixel_is_set(pb,x,y,threshold); x++ )
	    {
	      // handle horizontal cropping
	      //double ppx = x * sx; 
	      //if (ppx < this->crop_ax || ppx > this->crop_bx)
	      //continue;
	      
	      // look down to see how large a rectangle below we can make
	      int yy  = y;
	      while( ! pb_pixel_is_set(pb,x,yy,threshold) && (yy < img_height-1) )
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
	  pb_set_rect( pb, startx, starty, x-startx, height, 0xFF );
	  
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
	  latest->pose.y = img_height - (starty + height);
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
  return 0; // ok
}


void print_pointer( void* p, char* separator )
{
  printf( "%p%s", p, separator) ;
}


// POINTS -----------------------------------------------------------

stg_point_t* stg_points_create( size_t count )
{
  return( (stg_point_t*)g_new( stg_point_t, count ));
}

void stg_points_destroy( stg_point_t* pts )
{
  g_free( pts );
}


stg_point_t* stg_unit_square_points_create( void )
{
  stg_point_t * pts = stg_points_create( 4 );
  
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


// CALLBACKS -------------------------------------------------------

stg_cb_t* cb_create( stg_model_callback_t callback, void* arg )
{
  stg_cb_t* cb = (stg_cb_t*)malloc(sizeof(stg_cb_t));
  cb->callback = callback;
  cb->arg = arg;
  return cb;
}

void cb_destroy( stg_cb_t* cb )
{
  free( cb );
}

