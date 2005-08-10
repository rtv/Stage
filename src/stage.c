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

#include "replace.h"
#include "stage_internal.h"

int _stg_quit = FALSE;
int _stg_disable_gui = FALSE;

/** @defgroup stage Stage

Stage provides a virtual world populated by mobile robots and sensors,
along with various objects for the robots to sense and manipulate. It
is usually used with the Player robot server, but it can also be used
as a stand-alone robot simulation library. Stage is part of the
Player/Stage Project [http://playerstage.sf.net].

<h2>Design</h2>

<p>Stage was designed with multi-agent systems in mind, so it provides
fairly simple, computationally cheap models of lots of devices rather
than attempting to emulate any device with great fidelity. This design
is intended to be useful compromise between conventional high-fidelity
robot simulations, the minimal simulations described by Jakobi [4], and
the grid-world simulations common in artificial life research [5]. We
intend Stage to be just realistic enough to enable users to move
controllers between Stage robots and real robots, while still being
fast enough to simulate large populations. We also intend Stage to be
comprehensible to undergraduate students, yet sophisticated enough for
professional reseachers.
  
<p>Player also contains several useful 'virtual devices'; including
some sensor pre-processing or sensor-integration algorithms that help
you to rapidly build powerful robot controllers. These are easy to use
with Stage.

<h2>Models</h2>

<p>Stage provides several sensor and actuator models, including sonar
or infrared rangers, scanning laser rangefinder, color-blob tracking,
fiducial tracking and mobile robot bases with odometric or global
localization.

<i>Note:
Some models from previous versions may not yet be available in this
release (e.g. gripper & puck), but we're working on them. Let us know which ones you need.
</i>

<h2>Credits</h2>

Stage was written by:

<ul>
<li>Richard Vaughan <tt>(vaughan@sfu.ca)</tt>
<li>Andrew Howard  <tt>(ahoward@robotics.usc.edu)</tt>
<li>Brian Gerkey <tt>(gerkey@robotics.stanford.edu)</tt>
<li>Reed Hedges <tt>(hedges@mobilerobots.com)</tt>
</ul>

Many patches and bug reports have been contributed by users around the
world.

Stage is part of the <a
href="http://playerstage.sourceforge.net">Player/Stage Project</a>, a
community effort to develop Free Software tools for robotics
research. Stage is free to use, modify and redistribute under the
terms of the <a href=http://www.gnu.org/licenses/licenses.html> GNU
General Public License</a>.


If you use Stage in your work, we'd appreciate a citation. At the time of writing, the most suitable reference is:

<ul>
<li>
Brian Gerkey, Richard T. Vaughan and Andrew Howard. "The
Player/Stage Project: Tools for Multi-Robot and Distributed Sensor
Systems"
Proceedings of the 11th International Conference on Advanced Robotics,
pages 317-323,
Coimbra, Portugal, June 2003 <A HREF="http://www.isr.uc.pt/icar03/">(ICAR'03)</A>.
<br>
<A HREF="http://robotics.stanford.edu/~gerkey/research/final_papers/icar03-player.ps.gz">[gzipped
postscript]</A> <A HREF="http://robotics.stanford.edu/~gerkey/research/final_papers/icar03-player.pdf">[pdf]</A>  
</ul>

<p>Please help us keep track of what's being used out there by
correctly naming the Player/Stage components you use. Player and Stage
used together are referred to as "the Player/Stage system" or just
"Player/Stage". When libstage is used without Player, it's just called
"Stage". When Player is used with its 3D ODE-based simulation backend,
Gazebo, it's called Player/Gazebo. Gazebo without Player is just
"Gazebo". All this software is part of the "Player/Stage Project".

Funding for Stage has been provided in part by:
<ul>
 <li>NSERC (Canada)
 <li>Simon Fraser University (Canada)
 <li>DARPA (USA)
 <li>NASA (USA)
 <li>NSF (USA)
</ul>

<p>The names "Player" and "Stage" were inspired by the lines:

<pre>
All the world's a stage
And all the men and women merely players 
</pre>

from <i>"As You Like It"</i> by William Shakespeare.


<hr>
<small>
Richard Vaughan <tt>(vaughan@sfu.ca)</tt> \$Id: stage.c,v 1.76 2005-08-10 22:57:52 rtv Exp $
</small>

*/

int stg_init( int argc, char** argv )
{
  g_type_init(); // glib GObject initialization

  if( ! _stg_disable_gui )
    {
      // TODO - don't start the GUI if it was disabled
      //puts( "GUI_STARTUP" );x
      gui_startup( &argc, &argv );
    }

  // this forces use of decimal points in the config file rather than
  // euro-style commas. Do this after gui_startup() as GTK messes with
  // locale.
  if(!setlocale(LC_ALL,"POSIX"))
    fputs("Warning: failed to setlocale(); config file may not be parse correctly\n", stderr);

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

void stg_err( const char* err )
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

void stg_print_laser_config( stg_laser_config_t* slc )
{
  printf( "slc fov: %.2f  range_min: %.2f range_max: %.2f samples: %d\n",
	  slc->fov,
	  slc->range_min,
	  slc->range_max,
	  slc->samples );
}


stg_msec_t stg_timenow( void )
{
  struct timeval tv;
  static stg_msec_t starttime = 0;
  
  gettimeofday( &tv, NULL );
  
  stg_msec_t timenow = (stg_msec_t)( tv.tv_sec*1000 + tv.tv_usec/1000 );
  
  
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

/// converts an array of rectangles into an array of polygons
stg_polygon_t* stg_polygons_from_rotrects( stg_rotrect_t* rects, size_t count )
{
  stg_polygon_t* polys = stg_polygons_create( count );
  stg_point_t pts[4];
  
  size_t r;
  for( r=0; r<count; r++ )
    {  
      pts[0].x = rects[r].pose.x;
      pts[0].y = rects[r].pose.y;
      pts[1].x = rects[r].pose.x + rects[r].size.x;
      pts[1].y = rects[r].pose.y;
      pts[2].x = rects[r].pose.x + rects[r].size.x;
      pts[2].y = rects[r].pose.y + rects[r].size.y;
      pts[3].x = rects[r].pose.x;
      pts[3].y = rects[r].pose.y + rects[r].size.y;
      
      // copy these points in the polygon
      stg_polygon_set_points( &polys[r], pts, 4 );
    }
  
  return polys;
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


stg_polygon_t* stg_polygons_from_image_file(  const char* filename, 
					     size_t* count )
{
  stg_rotrect_t* rects = NULL;
  int rect_count = 0;

  if( stg_rotrects_from_image_file( filename,  
				    &rects,
				    &rect_count,
				    NULL, NULL ) )
    {
      PRINT_ERR1( "failed to load rects from image file \"%s\"",
		  filename );      
      return NULL;
    }

  //printf( "found %d rects\n", rect_count );
  // else

  *count = (size_t)rect_count;
  return stg_polygons_from_rotrects( rects, rect_count );
}

int stg_rotrects_from_image_file( const char* filename, 
				  stg_rotrect_t** rects, 
				  int* rect_count,
				  int* widthp, int* heightp )
{
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

// POINTS -----------------------------------------------------------

stg_point_t* stg_points_create( size_t count )
{
  return( (stg_point_t*)calloc( count, sizeof(stg_point_t)));
}

void stg_points_destroy( stg_point_t* pts )
{
  free( pts );
}

// POLYGONS -----------------------------------------------------------

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

stg_polygon_t* stg_unit_polygon_create( void )
{
  stg_point_t pts[4];
  pts[0].x = 0;
  pts[0].y = 0;
  pts[1].x = 1;
  pts[1].y = 0;
  pts[2].x = 1;
  pts[2].y = 1;
  pts[3].x = 0;
  pts[3].y = 1;  
  
  stg_polygon_t* poly = stg_polygons_create(1);
  stg_polygon_set_points( poly, pts, 4 );  
  return poly;
}

//////////////////////////////////////////////////////////////////////////
// scale an array of polygons so they fit in a rectangle of size
// [width] by [height], with the origin in the center of the rectangle
void stg_polygons_normalize( stg_polygon_t* polys, int num, 
			     double width, double height )
{
  // assuming the rectangles fit in a square +/- one billion units
  double minx, miny, maxx, maxy;
  minx = miny = BILLION;
  maxx = maxy = -BILLION;
  
  int l;
  for( l=0; l<num; l++ ) // examine all the polygons
    {
      // examine all the points in the polygon
      int p;
      for( p=0; p<polys[l].points->len; p++ )
	{
	  stg_point_t* pt = &g_array_index( polys[l].points, stg_point_t, p);
	  if( pt->x < minx ) minx = pt->x;
	  if( pt->y < miny ) miny = pt->y;
	  if( pt->x > maxx ) maxx = pt->x;
	  if( pt->y > maxy ) maxy = pt->y;	  
	}      
    }
  
  // now normalize all lengths so that the lines all fit inside
  // the specified rectangle
  double scalex = (maxx - minx);
  double scaley = (maxy - miny);
  
  for( l=0; l<num; l++ ) // scale each polygon
    { 
      // scale all the points in the polygon
      int p;
      for( p=0; p<polys[l].points->len; p++ )
	{
	  stg_point_t* pt = &g_array_index( polys[l].points, stg_point_t, p);
	  
	  pt->x = ((pt->x - minx) / scalex * width) - width/2.0;
	  pt->y = ((pt->y - miny) / scaley * height) - height/2.0;
	}
    }
}

void stg_polygon_print( stg_polygon_t* poly )
{
  printf( "polygon: %d pts : ", poly->points->len );
  
  int i;
  for(i=0;i<poly->points->len;i++)
    {
      stg_point_t* pt = &g_array_index( poly->points, stg_point_t, i );
      printf( "(%.2f,%.2f) ", pt->x, pt->y );
    }
  puts("");
}

void stg_polygons_print( stg_polygon_t* polys, unsigned int count )
{
  printf( "polygon array (%d polys)\n", count );
  
  int i;
  for( i=0; i<count; i++ )
    {
      printf( "[%d] ", i ); 
      stg_polygon_print( &polys[i] );
    }
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

/// Appends [count] points from [pts] to the point list of polygon
/// [poly], allocating memory if mecessary.
void stg_polygon_append_points( stg_polygon_t* poly, stg_point_t* pts, size_t count )
{
  assert( poly );
  g_array_append_vals( poly->points, pts, count );
}
