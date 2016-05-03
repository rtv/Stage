#ifndef STG_H
#define STG_H
/*
 *  Stage : a multi-robot simulator. Part of the Player Project.
 *
 *  Copyright (C) 2001-2009 Richard Vaughan, Brian Gerkey, Andrew
 *  Howard, Toby Collett, Reed Hedges, Alex Couture-Beil, Jeremy
 *  Asher, Pooya Karimian
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

/** \file stage.hh
 *  Desc: External header file for the Stage library
 *  Author: Richard Vaughan (vaughan@sfu.ca)
 *  Date: 1 June 2003
 *  SVN: $Id$
 */

// C libs
#include <unistd.h>
#include <stdint.h> // for portable int types eg. uint32_t
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <libgen.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <pthread.h>
#include <arpa/inet.h>

// C++ libs
#include <cmath>
#include <iostream>
#include <vector>
#include <list>
#include <map>
#include <set>
#include <queue>
#include <algorithm>
#include <iterator>



// FLTK Gui includes
#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Gl_Window.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Window.H>
#include <FL/fl_draw.H>
#include <FL/gl.h> // FLTK takes care of platform-specific GL stuff
// except GLU
#ifdef __APPLE__
#include <OpenGL/glu.h>
#else
#include <GL/glu.h>
#endif

#include <config.h>
#ifdef HAVE_BOOST_RANDOM
#include <boost/random.hpp>
#endif


/** @brief The Stage library uses its own namespace */
namespace Stg
{
  // forward declare
  class Block;
  class Canvas;
  class Cell;
  class Worldfile;
  class World;
  class WorldGui;
  class Model;
  class OptionsDlg;
  class Camera;
  class FileManager;
  class Option;

  typedef Model* (*creator_t)( World*, Model*, const std::string& type );

#ifdef HAVE_BOOST_RANDOM
  /** Define some types for the base boost random number generator. */
  typedef boost::mt19937 BaseBoostGenerator;
  typedef boost::mt19937 * BaseBoostGeneratorPtr;
#endif

  /** Initialize the Stage library. Stage will parse the argument
      array looking for parameters in the conventional way. */
  void Init( int* argc, char** argv[] );

  /** returns true iff Stg::Init() has been called. */
  bool InitDone();

  /** returns a human readable string indicating the libstage version
      number. */
  const char* Version();

  /** Copyright string */
  const char COPYRIGHT[] =
    "Copyright Richard Vaughan and contributors 2000-2009";

  /** Author string */
  const char AUTHORS[] =
    "Richard Vaughan, Brian Gerkey, Andrew Howard, Reed Hedges, Pooya Karimian, Toby Collett, Jeremy Asher, Alex Couture-Beil and contributors.";

  /** Project website string */
  const char WEBSITE[] = "http://playerstage.org";

  /** Project description string */
  const char DESCRIPTION[] =
    "Robot simulation library\nPart of the Player Project";

  /** Project distribution license string */
  const char LICENSE[] =
    "Stage robot simulation library\n"					\
    "Copyright (C) 2000-2009 Richard Vaughan and contributors\n"	\
    "Part of the Player Project [http://playerstage.org]\n"		\
    "\n"								\
    "This program is free software; you can redistribute it and/or\n"	\
    "modify it under the terms of the GNU General Public License\n"	\
    "as published by the Free Software Foundation; either version 2\n"	\
    "of the License, or (at your option) any later version.\n"		\
    "\n"								\
    "This program is distributed in the hope that it will be useful,\n"	\
    "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"	\
    "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"	\
    "GNU General Public License for more details.\n"			\
    "\n"								\
    "You should have received a copy of the GNU General Public License\n" \
    "along with this program; if not, write to the Free Software\n"	\
    "Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.\n" \
    "\n"								\
    "The text of the license may also be available online at\n"		\
    "http://www.gnu.org/licenses/old-licenses/gpl-2.0.html\n";

  /** Convenient constant */
  const double thousand = 1e3;

  /** Convenient constant */
  const double million = 1e6;

  /** Convenient constant */
  const double billion = 1e9;

  /** convert an angle in radians to degrees. */
  inline double rtod( double r ){ return( r*180.0/M_PI ); }

  /** convert an angle in degrees to radians. */
  inline double dtor( double d ){ return( d*M_PI/180.0 ); }

  /** Normalize an angle to within +/_ M_PI. */
  inline double normalize( double a )
  {
    while( a < -M_PI ) a += 2.0*M_PI;
    while( a >  M_PI ) a -= 2.0*M_PI;
    return a;
  };

  /** take binary sign of a, either -1, or 1 if >= 0 */
  inline int sgn( int a){ return( a<0 ? -1 : 1); }

  /** take binary sign of a, either -1.0, or 1.0 if >= 0. */
  inline double sgn( double a){ return( a<0 ? -1.0 : 1.0); }

  /** any integer value other than this is a valid fiducial ID */
  enum { FiducialNone = 0 };

  /** Value that uniquely identifies a model */
  typedef uint32_t id_t;

  /** Metres: floating point unit of distance */
  typedef double meters_t;

  /** Radians: unit of angle */
  typedef double radians_t;

  /** time structure */
  typedef struct timeval time_t;

  /** Milliseconds: unit of (short) time */
  typedef unsigned long msec_t;

  /** Microseconds: unit of (very short) time */
  typedef uint64_t usec_t;

  /** Kilograms: unit of mass */
  typedef double kg_t; // Kilograms (mass)

  /** Joules: unit of energy */
  typedef double joules_t;

  /** Watts: unit of power (energy/time) */
  typedef double watts_t;

  class Color
  {
  public:
    double r,g,b,a;

    Color( double r, double g, double b, double a=1.0 );

    /** Look up the color in the X11-style database. If the color is
	not found in the database, a cheerful red color will be used
	instead.  */
    Color( const std::string& name );

    Color();

    bool operator!=( const Color& other ) const;
    bool operator==( const Color& other ) const;
    static Color RandomColor();
    void Print( const char* prefix ) const;

    /** convenient constants */
    static const Color blue, red, green, yellow, magenta, cyan;

    const Color& Load( Worldfile* wf, int entity );

    void GLSet( void ) { glColor4f( r,g,b,a ); }
  };

  /** specify a rectangular size */
  class Size
  {
  public:
    meters_t x, y, z;

    Size( meters_t x,
	  meters_t y,
	  meters_t z )
      : x(x), y(y), z(z)
    {/*empty*/}

    /** default constructor uses default non-zero values */
    Size() : x( 0.4 ), y( 0.4 ), z( 1.0 )
    {/*empty*/}

    Size& Load( Worldfile* wf, int section, const char* keyword );
    void Save( Worldfile* wf, int section, const char* keyword ) const;

    void Zero() { x=y=z=0.0; }
  };

  /** Specify a 3 axis position, in x, y and heading. */
  class Pose
  {
  public:
    meters_t x, y, z;///< location in 3 axes
    radians_t a;///< rotation about the z axis.

    Pose( meters_t x,
	  meters_t y,
	  meters_t z,
	  radians_t a )
      : x(x), y(y), z(z), a(a)
    { /*empty*/ }

    Pose() : x(0.0), y(0.0), z(0.0), a(0.0)
    { /*empty*/ }

    virtual ~Pose(){};

    /** return a random pose within the bounding rectangle, with z=0 and
	angle random */
    static Pose Random( meters_t xmin, meters_t xmax,
			meters_t ymin, meters_t ymax )
    {
      return Pose( xmin + drand48() * (xmax-xmin),
		   ymin + drand48() * (ymax-ymin),
		   0,
		   normalize( drand48() * (2.0 * M_PI) ));
    }

    /** Print pose in human-readable format on stdout
	@param prefix Character string to prepend to pose output
    */
    virtual void Print( const char* prefix ) const
    {
      printf( "%s pose [x:%.3f y:%.3f z:%.3f a:%.3f]\n",
	      prefix, x,y,z,a );
    }

    std::string String() const
    {
      char buf[256];
      snprintf( buf, 256, "[ %.3f %.3f %.3f %.3f ]",
		x,y,z,a );
      return std::string(buf);
    }

    /* returns true iff all components of the velocity are zero. */
    bool IsZero() const
    { return( !(x || y || z || a )); };

    /** Set the pose to zero [0,0,0,0] */
    void Zero()
    { x=y=z=a=0.0; }

    Pose& Load( Worldfile* wf, int section, const char* keyword );
    void Save( Worldfile* wf, int section, const char* keyword );

    inline Pose operator+( const Pose& p ) const
    {
      const double cosa = cos(a);
      const double sina = sin(a);

      return Pose( x + p.x * cosa - p.y * sina,
		   y + p.x * sina + p.y * cosa,
		   z + p.z,
		   normalize(a + p.a) );
    }

    // a < b iff a is closer to the origin than b
    bool operator<( const Pose& p ) const
    {
	  //return( hypot( y, x ) < hypot( otHer.y, other.x ));
	 // just compare the squared values to avoid the sqrt()
      return( (y*y+x*x) < (p.y*p.y + p.x*p.x ));
    }

    bool operator==( const Pose& other ) const
    {
      return(  x==other.x &&
	       y==other.y &&
	       z==other.z &&
	       a==other.a );
    }

    bool operator!=( const Pose& other ) const
    {
      return(  x!=other.x ||
	       y!=other.y ||
	       z!=other.z ||
	       a!=other.a );
    }

	   meters_t Distance( const Pose& other ) const
	   {
	     return hypot( x-other.x, y-other.y );
	   }
  };


  class RaytraceResult
  {
  public:
    Pose pose;
    Model* mod;
    Color color;
    meters_t range;

    RaytraceResult() : pose(), mod(NULL), color(), range(0.0) {}
    RaytraceResult( const Pose& pose, Model* mod, const Color& color, const meters_t range)
       : pose(pose), mod(mod), color(color), range(range) {}

  };


  /** Specify a 4 axis velocity: 3D vector in [x, y, z], plus rotation
      about Z (yaw).*/
  class Velocity : public Pose
  {
  public:
    /** @param x velocity vector component along X axis (forward speed), in meters per second.
	@param y velocity vector component along Y axis (sideways speed), in meters per second.
	@param z velocity vector component along Z axis (vertical speed), in meters per second.
	@param a rotational velocity around Z axis (yaw), in radians per second.
    */
    Velocity( double x,
	      double y,
	      double z,
	      double a ) :
      Pose( x, y, z, a )
    { /*empty*/ }

    Velocity()
    { /*empty*/ }


    Velocity& Load( Worldfile* wf, int section, const char* keyword )
    {
       Pose::Load( wf, section, keyword );
       return *this;
    }

    /** Print velocity in human-readable format on stdout, with a
	prefix string

	@param prefix Character string to prepend to output, or NULL.
    */
    virtual void Print( const char* prefix ) const
    {
      if( prefix )
	printf( "%s", prefix );

      printf( "velocity [x:%.3f y:%.3f z:%3.f a:%.3f]\n",
	      x,y,z,a );
    }
  };


  /** Specify an object's basic geometry: position and rectangular
      size.  */
  class Geom
  {
  public:
    Pose pose;///< position
    Size size;///< extent

    /** Print geometry in human-readable format on stdout, with a
	prefix string

	@param prefix Character string to prepend to output, or NULL.
    */
    void Print( const char* prefix ) const
    {
      if( prefix )
	printf( "%s", prefix );

      printf( "geom pose: (%.2f,%.2f,%.2f) size: [%.2f,%.2f]\n",
	      pose.x,
	      pose.y,
	      pose.a,
	      size.x,
	      size.y );
    }

    /** Default constructor. Members pose and size use their default constructors. */
    Geom() : pose(), size() {}

    /** construct from a prior pose and size */
    Geom( const Pose& p, const Size& s ) : pose(p), size(s) {}

    void Zero()
    {
      pose.Zero();
      size.Zero();
    }
  };

  /** Bound a range of values, from min to max. min and max are initialized to zero. */
  class Bounds
  {
  public:
    /// smallest value in range, initially zero
    double min;
    /// largest value in range, initially zero
    double max;

    Bounds() : min(0), max(0) { /* empty*/  }
    Bounds( double min, double max ) : min(min), max(max) { /* empty*/  }

    Bounds& Load( Worldfile* wf, int section, const char* keyword );

    // returns value, but no smaller than min and no larger than max.
    double Constrain( double value );
  };

  /** Define a three-dimensional bounding box, initialized to zero */
  class bounds3d_t
  {
  public:
    /// volume extent along x axis, intially zero
    Bounds x;
    /// volume extent along y axis, initially zero
    Bounds y;
    /// volume extent along z axis, initially zero
    Bounds z;

    bounds3d_t() : x(), y(), z() {}
    bounds3d_t( const Bounds& x, const Bounds& y, const Bounds& z)
      : x(x), y(y), z(z) {}
  };

  /** Define a field-of-view: an angle and range bounds */
  typedef struct
  {
    Bounds range; ///< min and max range of sensor
    radians_t angle; ///< width of viewing angle of sensor
  } fov_t;

  /** Define a point on a 2d plane */
  class point_t
  {
  public:
    meters_t x, y;
    point_t( meters_t x, meters_t y ) : x(x), y(y){}
    point_t() : x(0.0), y(0.0){}

    bool operator+=( const point_t& other )
    { return ((x += other.x) && (y += other.y) ); }

    /** required to put these in sorted containers like std::map */
    bool operator<( const point_t& other ) const
    {
      if( x < other.x ) return true;
      if( other.x < x ) return false;
      return y < other.y;
    }

    bool operator==( const point_t& other ) const
    { return ((x == other.x) && (y == other.y) ); }
  };

  /** Define a point in 3d space */
  class point3_t
  {
  public:
    meters_t x,y,z;
    point3_t( meters_t x, meters_t y, meters_t z )
      : x(x), y(y), z(z) {}

    point3_t() : x(0.0), y(0.0), z(0.0) {}
  };

  /** Define an integer point on the 2d plane */
  class point_int_t
  {
  public:
    int x,y;
    point_int_t( int x, int y ) : x(x), y(y){}
    point_int_t() : x(0), y(0){}

    /** required to put these in sorted containers like std::map */
    bool operator<( const point_int_t& other ) const
    {
      if( x < other.x ) return true;
      if( other.x < x ) return false;
      return y < other.y;
    }

    bool operator==( const point_int_t& other ) const
    { return ((x == other.x) && (y == other.y) ); }
  };

  /** create an array of 4 points containing the corners of a unit
      square.  */
  point_t* unit_square_points_create();

  /** Convenient OpenGL drawing routines, used by visualization
      code. */
  namespace Gl
  {
    void pose_shift( const Pose &pose );
    void pose_inverse_shift( const Pose &pose );
    void coord_shift( double x, double y, double z, double a  );
    void draw_grid( bounds3d_t vol );
    /** Render a string at [x,y,z] in the current color */
    void draw_string( float x, float y, float z, const char *string);
    void draw_string_multiline( float x, float y, float w, float h,
				const char *string, Fl_Align align );
    void draw_speech_bubble( float x, float y, float z, const char* str );
    void draw_octagon( float w, float h, float m );
    void draw_octagon( float x, float y, float w, float h, float m );
    void draw_vector( double x, double y, double z );
    void draw_origin( double len );
    void draw_array( float x, float y, float w, float h,
		     float* data, size_t len, size_t offset,
		     float min, float max );
    void draw_array( float x, float y, float w, float h,
		     float* data, size_t len, size_t offset );
    /** Draws a rectangle with center at x,y, with sides of length dx,dy */
    void draw_centered_rect( float x, float y, float dx, float dy );
  } // namespace Gl

  void RegisterModels();


  /** Abstract class for adding visualizations to models. Visualize must be overloaded, and is then called in the models local coord system */
  class Visualizer {
  private:
    const std::string menu_name;
    const std::string worldfile_name;

  public:
    Visualizer( const std::string& menu_name,
		const std::string& worldfile_name )
      : menu_name( menu_name ),
	worldfile_name( worldfile_name )
    { }

    virtual ~Visualizer( void ) { }
    virtual void Visualize( Model* mod, Camera* cam ) = 0;

    const std::string& GetMenuName() { return menu_name; }
    const std::string& GetWorldfileName() { return worldfile_name; }
  };


  /** Define a callback function type that can be attached to a
      record within a model and called whenever the record is set.*/
  typedef int(*model_callback_t)(Model* mod, void* user );

  typedef int(*world_callback_t)(World* world, void* user );

  // return val, or minval if val < minval, or maxval if val > maxval
  double constrain( double val, double minval, double maxval );

  typedef struct
  {
    int enabled;
    Pose pose;
    meters_t size; ///< rendered as a sphere with this diameter
    Color color;
    msec_t period; ///< duration of a complete cycle
    double duty_cycle; ///< mark/space ratio
  } blinkenlight_t;


  /** Defines a rectangle of [size] located at [pose] */
  typedef struct
  {
    Pose pose;
    Size size;
  } rotrect_t; // rotated rectangle

  /** load the image file [filename] and convert it to a vector of polygons
   */
  int polys_from_image_file( const std::string& filename,
			     std::vector<std::vector<point_t> >& polys );


  /** matching function should return true iff the candidate block is
      stops the ray, false if the block transmits the ray
  */
  typedef bool (*ray_test_func_t)(Model* candidate,
				  Model* finder,
				  const void* arg );

  // STL container iterator macros - __typeof is a gcc extension, so
  // this could be an issue one day.
#define VAR(V,init) __typeof(init) V=(init)

  //#define FOR_EACH(I,C) for(VAR(I,(C).begin());I!=(C).end();++I)

  // NOTE:
  // this version assumes the container is not modified in the loop,
  // which I think is true everywhere it is used in Stage
#define FOR_EACH(I,C) for(VAR(I,(C).begin()),ite=(C).end();(I)!=ite;++(I))

  /** wrapper for Erase-Remove method of removing all instances of thing from container
   */
  template <class T, class C>
  void EraseAll( T thing, C& cont )
  { cont.erase( std::remove( cont.begin(), cont.end(), thing ), cont.end() ); }

  // Error macros - output goes to stderr
#define PRINT_ERR(m) fprintf( stderr, "\033[41merr\033[0m: "m" (%s %s)\n", __FILE__, __FUNCTION__)
#define PRINT_ERR1(m,a) fprintf( stderr, "\033[41merr\033[0m: "m" (%s %s)\n", a, __FILE__, __FUNCTION__)
#define PRINT_ERR2(m,a,b) fprintf( stderr, "\033[41merr\033[0m: "m" (%s %s)\n", a, b, __FILE__, __FUNCTION__)
#define PRINT_ERR3(m,a,b,c) fprintf( stderr, "\033[41merr\033[0m: "m" (%s %s)\n", a, b, c, __FILE__, __FUNCTION__)
#define PRINT_ERR4(m,a,b,c,d) fprintf( stderr, "\033[41merr\033[0m: "m" (%s %s)\n", a, b, c, d, __FILE__, __FUNCTION__)
#define PRINT_ERR5(m,a,b,c,d,e) fprintf( stderr, "\033[41merr\033[0m: "m" (%s %s)\n", a, b, c, d, e, __FILE__, __FUNCTION__)

  // Warning macros
#define PRINT_WARN(m) printf( "\033[44mwarn\033[0m: "m" (%s %s)\n", __FILE__, __FUNCTION__)
#define PRINT_WARN1(m,a) printf( "\033[44mwarn\033[0m: "m" (%s %s)\n", a, __FILE__, __FUNCTION__)
#define PRINT_WARN2(m,a,b) printf( "\033[44mwarn\033[0m: "m" (%s %s)\n", a, b, __FILE__, __FUNCTION__)
#define PRINT_WARN3(m,a,b,c) printf( "\033[44mwarn\033[0m: "m" (%s %s)\n", a, b, c, __FILE__, __FUNCTION__)
#define PRINT_WARN4(m,a,b,c,d) printf( "\033[44mwarn\033[0m: "m" (%s %s)\n", a, b, c, d, __FILE__, __FUNCTION__)
#define PRINT_WARN5(m,a,b,c,d,e) printf( "\033[44mwarn\033[0m: "m" (%s %s)\n", a, b, c, d, e, __FILE__, __FUNCTION__)

  // Message macros
#ifdef DEBUG
#define PRINT_MSG(m) printf( "Stage: "m" (%s %s)\n", __FILE__, __FUNCTION__)
#define PRINT_MSG1(m,a) printf( "Stage: "m" (%s %s)\n", a, __FILE__, __FUNCTION__)
#define PRINT_MSG2(m,a,b) printf( "Stage: "m" (%s %s)\n", a, b, __FILE__, __FUNCTION__)
#define PRINT_MSG3(m,a,b,c) printf( "Stage: "m" (%s %s)\n", a, b, c, __FILE__, __FUNCTION__)
#define PRINT_MSG4(m,a,b,c,d) printf( "Stage: "m" (%s %s)\n", a, b, c, d, __FILE__, __FUNCTION__)
#define PRINT_MSG5(m,a,b,c,d,e) printf( "Stage: "m" (%s %s)\n", a, b, c, d, e,__FILE__, __FUNCTION__)
#else
#define PRINT_MSG(m) printf( "Stage: "m"\n" )
#define PRINT_MSG1(m,a) printf( "Stage: "m"\n", a)
#define PRINT_MSG2(m,a,b) printf( "Stage: "m"\n,", a, b )
#define PRINT_MSG3(m,a,b,c) printf( "Stage: "m"\n", a, b, c )
#define PRINT_MSG4(m,a,b,c,d) printf( "Stage: "m"\n", a, b, c, d )
#define PRINT_MSG5(m,a,b,c,d,e) printf( "Stage: "m"\n", a, b, c, d, e )
#endif

  // DEBUG macros
#ifdef DEBUG
#define PRINT_DEBUG(m) printf( "debug: "m" (%s %s)\n", __FILE__, __FUNCTION__)
#define PRINT_DEBUG1(m,a) printf( "debug: "m" (%s %s)\n", a, __FILE__, __FUNCTION__)
#define PRINT_DEBUG2(m,a,b) printf( "debug: "m" (%s %s)\n", a, b, __FILE__, __FUNCTION__)
#define PRINT_DEBUG3(m,a,b,c) printf( "debug: "m" (%s %s)\n", a, b, c, __FILE__, __FUNCTION__)
#define PRINT_DEBUG4(m,a,b,c,d) printf( "debug: "m" (%s %s)\n", a, b, c ,d, __FILE__, __FUNCTION__)
#define PRINT_DEBUG5(m,a,b,c,d,e) printf( "debug: "m" (%s %s)\n", a, b, c ,d, e, __FILE__, __FUNCTION__)
#else
#define PRINT_DEBUG(m)
#define PRINT_DEBUG1(m,a)
#define PRINT_DEBUG2(m,a,b)
#define PRINT_DEBUG3(m,a,b,c)
#define PRINT_DEBUG4(m,a,b,c,d)
#define PRINT_DEBUG5(m,a,b,c,d,e)
#endif

  class Block;
  class Model;


  // ANCESTOR CLASS
  /** Base class for Model and World */
  class Ancestor
  {
    friend class Canvas; // allow Canvas access to our private members

  protected:

    /** array contains the number of each type of child model */
    std::map<std::string,unsigned int> child_type_counts;

    std::vector<Model*> children;

    bool debug;

    /** A key-value database for users to associate arbitrary things with this object. */
    std::map<std::string,void*> props;

    std::string token;

    Ancestor& Load( Worldfile* wf, int section );
    void Save( Worldfile* wf, int section );

  public:
    Ancestor();
    virtual ~Ancestor();

    /** get the children of the this element */
    std::vector<Model*>& GetChildren(){ return children;}

    /** recursively call func( model, arg ) for each descendant */
    void ForEachDescendant( model_callback_t func, void* arg );

    virtual void AddChild( Model* mod );
    virtual void RemoveChild( Model* mod );
    virtual Pose GetGlobalPose() const;

    const char* Token() const { return token.c_str(); }
    const std::string& TokenStr() const { return token; }

    virtual void SetToken( const std::string& str )
    {
      //printf( "Ancestor::SetToken( %s )\n", str.c_str() );

      if( str.size() > 0 )
	token = str;
      else
	PRINT_ERR( "Ancestor::SetToken() called with zero length string. Ignored." );
    }

    /** A key-value database for users to associate arbitrary things with this model. */
    void SetProperty( std::string& key, void* value ){ props[ key ] = value; }

    /** A key-value database for users to associate arbitrary things with this model. */
    void* GetProperty( std::string& key )
    {
      std::map<std::string,void*>::iterator it = props.find( key );
      return( it == props.end() ? NULL : it->second );
    }
  };


  class Ray
  {
  public:
    Ray( const Model* mod, const Pose& origin, const meters_t range, const ray_test_func_t func, const void* arg, const bool ztest ) :
      mod(mod), origin(origin), range(range), func(func), arg(arg), ztest(ztest)
    {}

    Ray() : mod(NULL), origin(0,0,0,0), range(0), func(NULL), arg(NULL), ztest(true)
    {}

    const Model* mod;
    Pose origin;
    meters_t range;
    ray_test_func_t func;
    const void* arg;
    bool ztest;
  };


  // defined in stage_internal.hh
  class Region;
  class SuperRegion;
  class BlockGroup;
  class PowerPack;

  class LogEntry
  {
    usec_t timestamp;
    Model* mod;
    Pose pose;

  public:
    LogEntry( usec_t timestamp, Model* mod );

    /** A log of all LogEntries ever created */
    static std::vector<LogEntry> log;

    /** Returns the number of logged entries */
    static size_t Count(){ return log.size(); }

    /** Clear the log */
    static void Clear(){ log.clear(); }

    /** Print the log on stdout */
    static void Print();
  };

  class CtrlArgs
  {
  public:
    std::string worldfile;
    std::string cmdline;

    CtrlArgs( std::string w, std::string c ) : worldfile(w), cmdline(c) {}
  };

  class ModelPosition;

  /// %World class
  class World : public Ancestor
  {
  public:
    friend class Block;
    friend class Model; // allow access to private members
    friend class ModelFiducial;
    friend class Canvas;
    friend class WorkerThread;
    friend class ModelWifi;

  public:
    /** contains the command line arguments passed to Stg::Init(), so
	that controllers can read them. */
    static std::vector<std::string> args;
    static std::string ctrlargs;

  private:

    static std::set<World*> world_set; ///< all the worlds that exist
    static bool quit_all; ///< quit all worlds ASAP
    static void UpdateCb( World* world);
    static unsigned int next_id; ///<initially zero, used to allocate unique sequential world ids

    bool destroy;
    bool dirty; ///< iff true, a gui redraw would be required

    /** Pointers to all the models in this world. */
    std::set<Model*> models;

    /** pointers to the models that make up the world, indexed by name. */
    std::map<std::string, Model*> models_by_name;

    /** pointers to the models that make up the world, indexed by worldfile entry index */
    std::map<int,Model*> models_by_wfentity;

    /** Keep a list of all models with detectable fiducials. This
	avoids searching the whole world for fiducials. */
    std::vector<Model*> models_with_fiducials;

    struct ltx
    {
      bool operator()(const Model* a, const Model* b) const;
    };

    struct lty
    {
      bool operator()(const Model* a, const Model* b) const;
    };

    /** maintain a set of models with fiducials sorted by pose.x, for
	quickly finding nearby fidcucials */
    std::set<Model*,ltx> models_with_fiducials_byx;

    /** maintain a set of models with fiducials sorted by pose.y, for
	quickly finding nearby fidcucials */
    std::set<Model*,lty> models_with_fiducials_byy;

    /** Add a model to the set of models with non-zero fiducials, if not already there. */
    void FiducialInsert( Model* mod )
    {
      FiducialErase( mod ); // make sure it's not there already
      models_with_fiducials.push_back( mod );
    }

    /** Remove a model from the set of models with non-zero fiducials, if it exists. */
    void FiducialErase( Model* mod )
    {
      EraseAll( mod, models_with_fiducials );
    }

    /** Keep a list of all models with WIFI.  This avoids searching the whole
        world for models with WIFI */
    std::vector<Model*> models_with_wifi;
    int num_models_with_wifi; // quick fix because of iterator problems

  public:
    /** Add a model to the set of models with Wifi, if not already there. */
    void WifiInsert( Model* mod )
    {
      if ( mod )
      {
        //Try to find the model in the vector
        std::vector<Model*>::iterator location_iterator = std::find( models_with_wifi.begin(), models_with_wifi.end(), mod );

        //Add it to the vec if it's not already there
        if ( location_iterator == models_with_wifi.end() ){
          models_with_wifi.push_back(mod);
          ++num_models_with_wifi;
        }
      }
    }//WifiInsert

    /** Remove a model from the list of models with wifi, if it exists. */
    void WifiErase( Model* mod )
    {
      if ( mod ) {
        EraseAll( mod, models_with_wifi);
        --num_models_with_wifi;
      }
    }//WifiErase

  private:



    double ppm; ///< the resolution of the world model in pixels per meter
    bool quit; ///< quit this world ASAP
    bool show_clock; ///< iff true, print the sim time on stdout
    unsigned int show_clock_interval; ///< updates between clock outputs

    //--- thread sync ----
    pthread_mutex_t sync_mutex; ///< protect the worker thread management stuff
    unsigned int threads_working; ///< the number of worker threads not yet finished
    pthread_cond_t threads_start_cond; ///< signalled to unblock worker threads
    pthread_cond_t threads_done_cond; ///< signalled by last worker thread to unblock main thread
    int total_subs; ///< the total number of subscriptions to all models
    unsigned int worker_threads; ///< the number of worker threads to use

  protected:

    std::list<std::pair<world_callback_t,void*> > cb_list; ///< List of callback functions and arguments
    bounds3d_t extent; ///< Describes the 3D volume of the world
    bool graphics;///< true iff we have a GUI

    std::set<Option*> option_table; ///< GUI options (toggles) registered by models
    std::list<PowerPack*> powerpack_list; ///< List of all the powerpacks attached to models in the world
    /** World::quit is set true when this simulation time is reached */
    usec_t quit_time;
    std::list<float*> ray_list;///< List of rays traced for debug visualization
    usec_t sim_time; ///< the current sim time in this world in microseconds
    std::map<point_int_t,SuperRegion*> superregions;

    uint64_t updates; ///< the number of simulated time steps executed so far
    Worldfile* wf; ///< If set, points to the worldfile used to create this world

    void CallUpdateCallbacks(); ///< Call all calbacks in cb_list, removing any that return true;

  public:

    uint64_t UpdateCount(){ return updates; }

    bool paused; ///< if true, the simulation is stopped

    virtual void Start(){ paused = false; };
    virtual void Stop(){ paused = true; };
    virtual void TogglePause(){ paused ? Start() : Stop(); };

    bool Paused() const { return( paused ); };

    /** Force the GUI to redraw the world, even if paused. This
	imlementation does nothing, but can be overridden by
	subclasses. */
    virtual void Redraw( void ){ }; // does nothing

    std::vector<point_int_t> rt_cells;
    std::vector<point_int_t> rt_candidate_cells;

    static const int DEFAULT_PPM = 50;  // default resolution in pixels per meter

    /** Attach a callback function, to be called with the argument at
	the end of a complete update step */
    void AddUpdateCallback( world_callback_t cb, void* user );

    /** Remove a callback function. Any argument data passed to
	AddUpdateCallback is not automatically freed. */
    int RemoveUpdateCallback( world_callback_t cb, void* user );

    /** Log the state of a Model */
    void Log( Model* mod );

    /** hint that the world needs to be redrawn if a GUI is attached */
    void NeedRedraw(){ dirty = true; };

    /** Special model for the floor of the world */
    Model* ground;

    /** Get human readable string that describes the current simulation
	time. */
    virtual std::string ClockString( void ) const;

    Model* CreateModel( Model* parent, const std::string& typestr );

    void LoadModel(      Worldfile* wf, int entity );
    void LoadBlock(      Worldfile* wf, int entity );
    void LoadBlockGroup( Worldfile* wf, int entity );
    void LoadSensor(     Worldfile* wf, int entity );

    virtual Model* RecentlySelectedModel() const { return NULL; }

    /** Add the block to every raytrace bitmap cell that intersects
	the edges of the polygon.*/
    void MapPoly( const std::vector<point_int_t>& poly,
		  Block* block,
		  unsigned int layer );

    SuperRegion* AddSuperRegion( const point_int_t& coord );
    SuperRegion* GetSuperRegion( const point_int_t& org );
    SuperRegion* GetSuperRegionCreate( const point_int_t& org );

    /** convert a distance in meters to a distance in world occupancy
	grid pixels */
    int32_t MetersToPixels( meters_t x ) const
    { return (int32_t)floor(x * ppm); };

    /** Return the bitmap coordinates corresponding to the location in meters. */
    point_int_t MetersToPixels( const point_t& pt ) const
    { return point_int_t( MetersToPixels(pt.x), MetersToPixels(pt.y)); };

    // dummy implementations to be overloaded by GUI subclasses
    virtual void PushColor( Color col )
    { /* do nothing */  (void)col; };
    virtual void PushColor( double r, double g, double b, double a )
    { /* do nothing */ (void)r; (void)g; (void)b; (void)a; };

    virtual void PopColor(){ /* do nothing */  };

    SuperRegion* CreateSuperRegion( point_int_t origin );
    void DestroySuperRegion( SuperRegion* sr );

    /** trace a ray. */
    RaytraceResult Raytrace( const Ray& ray );

    RaytraceResult Raytrace( const Pose& pose,
			     const meters_t range,
			     const ray_test_func_t func,
			     const Model* finder,
			     const void* arg,
			     const bool ztest );

    void Raytrace( const Pose &gpose, // global pose
		   const meters_t range,
		   const radians_t fov,
		   const ray_test_func_t func,
		   const Model* model,
		   const void* arg,
		   const bool ztest,
		   std::vector<RaytraceResult>& results );

    //Raytracing specific to the WIFI model.
    meters_t RaytraceWifi( const Pose &gpose,
              const meters_t range,
              const ray_test_func_t func,
              const Model* mod,
              const Model* target,
              const void* arg,
              const bool ztest );

    meters_t RaytraceWifi( const Ray& r,
                               const Model* target );

    /** Enlarge the bounding volume to include this point */
    inline void Extend( point3_t pt );

    virtual void AddModel( Model* mod );
    virtual void RemoveModel( Model* mod );

    void AddModelName( Model* mod, const std::string& name );

    void AddPowerPack( PowerPack* pp );
    void RemovePowerPack( PowerPack* pp );

    void ClearRays();

    /** store rays traced for debugging purposes */
    void RecordRay( double x1, double y1, double x2, double y2 );

    /** Returns true iff the current time is greater than the time we
	should quit */
    bool PastQuitTime();

    static void* update_thread_entry( std::pair<World*,int>* info );

    class Event
    {
    public:

      Event( usec_t time, Model* mod, model_callback_t cb, void* arg )
	: time(time), mod(mod), cb(cb), arg(arg) {}

      usec_t time; ///< time that event occurs
      Model* mod; ///< model to pass into callback
      model_callback_t cb;
      void* arg;

      /** order by time. Break ties by value of Model*, then cb*.
	  @param event to compare with this one. */
      bool operator<( const Event& other ) const;
    };

    /** Queue of pending simulation events for the main thread to handle. */
    std::vector<std::priority_queue<Event> > event_queues;

    /** Queue of pending simulation events for the main thread to handle. */
    std::vector<std::queue<Model*> > pending_update_callbacks;

    /** Create a new simulation event to be handled in the future.

	@param queue_num Specify which queue the event should be on. The main
	thread is 0.

	@param delay The time from now until the event occurs, in
	microseconds.

	@param mod The model that should have its Update() method
	called at the specified time.
    */
    void Enqueue( unsigned int queue_num, usec_t delay, Model* mod, model_callback_t cb, void* arg )
    {  event_queues[queue_num].push( Event( sim_time + delay, mod, cb, arg ) ); }

    /** Set of models that require energy calculations at each World::Update(). */
    std::set<Model*> active_energy;
    void EnableEnergy( Model* m ) { active_energy.insert( m ); };
    void DisableEnergy( Model* m ) { active_energy.erase( m ); };

    /** Set of models that require their positions to be recalculated at each World::Update(). */
    std::set<ModelPosition*> active_velocity;

    /** The amount of simulated time to run for each call to Update() */
    usec_t sim_interval;

    // debug instrumentation - making sure the number of update callbacks
    // in each thread is consistent with the number that have been
    // registered globally
    int update_cb_count;

    /** consume events from the queue up to and including the current sim_time */
    void ConsumeQueue( unsigned int queue_num );

    /** returns an event queue index number for a model to use for
	updates */
    unsigned int GetEventQueue( Model* mod ) const;

  public:
    /** returns true when time to quit, false otherwise */
    static bool UpdateAll();

  /** run all worlds.
   *  If only non-gui worlds were created, UpdateAll() is
   *  repeatedly called.
   *  To simulate a gui world only a single gui world may
   *  have been created. This world is then simulated.
   */
  static void Run();

    World( const std::string& name = "MyWorld",
	   double ppm = DEFAULT_PPM );

    virtual ~World();

    /** Returns the current simulated time in this world, in microseconds. */
    usec_t SimTimeNow(void) const { return sim_time; }

    /** Returns a pointer to the currently-open worlddfile object, or
	NULL if there is none. */
    Worldfile* GetWorldFile()	{ return wf; };

    /** Returns true iff this World implements a GUI. The base World
	class returns false, but subclasses can override this
	behaviour. */
    virtual bool IsGUI() const { return false; }

    /** Open the file at the specified location, create a Worldfile
	object, read the file and configure the world from the
	contents, creating models as necessary. The created object
	persists, and can be retrieved later with
	World::GetWorldFile(). */
    virtual void Load( const std::string& worldfile_path );

    virtual void UnLoad();

    virtual void Reload();

    /** Save the current world state into a worldfile with the given
	filename.  @param Filename to save as. */
    virtual bool Save( const char* filename );

    /** Run one simulation timestep. Advances the simulation clock,
	executes all simulation updates due at the current time, then
	queues up future events. */
    virtual bool Update(void);

    /** Returns true iff either the local or global quit flag was set,
	which usually happens because someone called Quit() or
	QuitAll(). */
    bool TestQuit() const { return( quit || quit_all );  }

    /** Request the world quits simulation before the next timestep. */
    void Quit(){ quit = true; }

    /** Requests all worlds quit simulation before the next timestep. */
    void QuitAll(){ quit_all = true; }

    /** Cancel a local quit request. */
    void CancelQuit(){ quit = false; }

    /** Cancel a global quit request. */
    void CancelQuitAll(){ quit_all = false; }

    void TryCharge( PowerPack* pp, const Pose& pose );

    /** Get the resolution in pixels-per-metre of the underlying
	discrete raytracing model */
    double Resolution() const { return ppm; };

    /** Returns a pointer to the model identified by name, or NULL if
	nonexistent */
    Model* GetModel( const std::string& name ) const;

    /** Returns a const reference to the set of models in the world. */
    const std::set<Model*> GetAllModels() const { return models; };

    /** Return the 3D bounding box of the world, in meters */
    const bounds3d_t& GetExtent() const { return extent; };

    /** Return the number of times the world has been updated. */
    uint64_t GetUpdateCount() const { return updates; }

    /// Register an Option for pickup by the GUI
    void RegisterOption( Option* opt );

    /// Control printing time to stdout
    void ShowClock( bool enable ){ show_clock = enable; };

    /** Return the floor model */
    Model* GetGround() {return ground;};

  };


  class Block
  {
    friend class BlockGroup;
    friend class Model;
    friend class SuperRegion;
    friend class World;
    friend class Canvas;
    friend class Cell;
  public:

    /** Block Constructor. A model's body is a list of these
	blocks. The point data is copied, so pts can safely be freed
	after constructing the block.*/
    Block( BlockGroup* group,
	   const std::vector<point_t>& pts,
	   const Bounds& zrange );

    /** A from-file  constructor */
    Block( BlockGroup* group, Worldfile* wf, int entity);

    ~Block();

    /** render the block into the world's raytrace data structure */
    void Map( unsigned int layer );

    /** remove the block from the world's raytracing data structure */
    void UnMap( unsigned int layer );

    /** draw the block in OpenGL as a solid single color */
    void DrawSolid(bool topview);

    /** draw the projection of the block onto the z=0 plane	*/
    void DrawFootPrint();

    /** Translate all points in the block by the indicated amounts */
    void Translate( double x, double y );

    /** Return the center of the block on the X axis */
    double CenterX();

    /** Return the center of the block on the Y axis */
    double CenterY();

    /** Set the center of the block on the X axis */
    void SetCenterX( double y );

    /** Set the center of the block on the Y axis */
    void SetCenterY( double y );

    /** Set the center of the block */
    void SetCenter( double x, double y);

    /** Set the extent in Z of the block */
    void SetZ( double min, double max );

    void AppendTouchingModels( std::set<Model*>& touchers );

    /** Returns the first model that shares a bitmap cell with this model */
    Model* TestCollision();

    void Load( Worldfile* wf, int entity );

    void Rasterize( uint8_t* data,
		    unsigned int width, unsigned int height,
		    meters_t cellwidth, meters_t cellheight );

    BlockGroup* group; ///< The BlockGroup to which this Block belongs.
  private:
    std::vector<point_t> pts; ///< points defining a polygon.
    Bounds local_z; ///<  z extent in local coords.
    Bounds global_z; ///< z extent in global coordinates.

    /** record the cells into which this block has been rendered so we
	can remove them very quickly. One vector for each of the two
	bitmap layers.*/
    std::vector<Cell*> rendered_cells[2];

    void DrawTop();
    void DrawSides();
  };


  class BlockGroup
  {
    friend class Model;
    friend class Block;
    friend class World;
    friend class SuperRegion;

  private:
    std::vector<Block> blocks; ///< Contains the blocks in this group.
    int displaylist; ///< OpenGL displaylist that renders this blockgroup.

  public:    Model& mod;

  private:
    void AppendBlock( const Block& block );

    void CalcSize();
    void Clear() ; /** deletes all blocks from the group */

    void AppendTouchingModels( std::set<Model*>& touchers );

    /** Returns a pointer to the first model detected to be colliding
	with a block in this group, or NULL, if none are detected. */
    Model* TestCollision();

    /** Renders all blocks into the bitmap at the indicated layer.*/
    void Map( unsigned int layer );
    /** Removes all blocks from the bitmap at the indicated layer.*/
    void UnMap( unsigned int layer );

    /** Interpret the bitmap file as a set of rectangles and add them
	as blocks to this group.*/
    void LoadBitmap( const std::string& bitmapfile, Worldfile *wf );

    /** Add a new block decribed by a worldfile entry. */
    void LoadBlock( Worldfile* wf, int entity );

    /** Render the blockgroup as a bitmap image. */
    void Rasterize( uint8_t* data,
		    unsigned int width, unsigned int height,
		    meters_t cellwidth, meters_t cellheight );

    /** Draw the block in OpenGL as a solid single color. */
    void DrawSolid( const Geom &geom);

    /** Re-create the display list for drawing this blockgroup. This
	is required whenever a member block or the owning model
	changes its appearance.*/
    void BuildDisplayList();

    /** Draw the blockgroup from the cached displaylist. */
    void CallDisplayList();

  public:
    BlockGroup( Model& mod );
    ~BlockGroup();

    uint32_t GetCount() const { return blocks.size(); };
    const Block& GetBlock( unsigned int index ) const { return blocks[index]; };
    Block& GetBlockMutable( unsigned int index ) { return blocks[index]; };

    /** Return the extremal points of all member blocks in all three axes. */
    bounds3d_t BoundingBox() const;

    /** Draw the projection of the block group onto the z=0 plane. */
    void DrawFootPrint( const Geom &geom);
  };

  class Camera
  {
  protected:
    double _pitch; //left-right (about y)
    double _yaw; //up-down (about x)
    double _x, _y, _z;

  public:
    Camera() : _pitch( 0 ), _yaw( 0 ), _x( 0 ), _y( 0 ), _z( 0 ) { }
    virtual ~Camera() { }

    virtual void Draw( void ) const = 0;
    virtual void SetProjection( void ) const = 0;

    double yaw( void ) const { return _yaw; }
    double pitch( void ) const { return _pitch; }

    double x( void ) const { return _x; }
    double y( void ) const { return _y; }
    double z( void ) const { return _z; }

    virtual void reset() = 0;
    virtual void Load( Worldfile* wf, int sec ) = 0;

    //TODO data should be passed in somehow else. (at least min/max stuff)
    //virtual void SetProjection( float pixels_width, float pixels_height, float y_min, float y_max ) const = 0;
  };

  class PerspectiveCamera : public Camera
  {
  private:
    double _z_near;
    double _z_far;
    double _vert_fov;
    double _horiz_fov;
    double _aspect;

  public:
    PerspectiveCamera( void );

    virtual void Draw( void ) const;
    virtual void SetProjection( void ) const;
    //void SetProjection( double aspect ) const;
    void update( void );

    void strafe( double amount );
    void forward( double amount );

    void setPose( double x, double y, double z ) { _x = x; _y = y; _z = z; }
    void addPose( double x, double y, double z ) { _x += x; _y += y; _z += z; if( _z < 0.1 ) _z = 0.1; }
    void move( double x, double y, double z );
    void setFov( double horiz_fov, double vert_fov ) { _horiz_fov = horiz_fov; _vert_fov = vert_fov; }
    ///update vertical fov based on window aspect and current horizontal fov
    void setAspect( double aspect ) { _aspect = aspect; }
    void setYaw( double yaw ) { _yaw = yaw; }
    double horizFov( void ) const { return _horiz_fov; }
    double vertFov( void ) const { return _vert_fov; }
    void addYaw( double yaw ) { _yaw += yaw; }
    void setPitch( double pitch ) { _pitch = pitch; }
    void addPitch( double pitch ) { _pitch += pitch; if( _pitch < 0 ) _pitch = 0; else if( _pitch > 180 ) _pitch = 180; }

    double realDistance( double z_buf_val ) const {
      return _z_near * _z_far / ( _z_far - z_buf_val * ( _z_far - _z_near ) );
    }
    void scroll( double dy ) { _z += dy; }
    double nearClip( void ) const { return _z_near; }
    double farClip( void ) const { return _z_far; }
    void setClip( double near, double far ) { _z_far = far; _z_near = near; }

    void reset() { setPitch( 70 ); setYaw( 0 ); }

    void Load( Worldfile* wf, int sec );
    void Save( Worldfile* wf, int sec );
  };

  class OrthoCamera : public Camera
  {
  private:
    double _scale;
    double _pixels_width;
    double _pixels_height;
    double _y_min;
    double _y_max;

  public:
    OrthoCamera( void ) :
		_scale( 15 ),
		_pixels_width(0),
		_pixels_height(0),
		_y_min(0),
		_y_max(0)
	 { }

    virtual void Draw() const;

    virtual void SetProjection( double pixels_width,
										  double pixels_height,
										  double y_min,
										  double y_max );

    virtual void SetProjection( void ) const;

    void move( double x, double y );

    void setYaw( double yaw ) { _yaw = yaw;	}

    void setPitch( double pitch ) { _pitch = pitch; }

    void addYaw( double yaw ) { _yaw += yaw;	}

    void addPitch( double pitch ) {
      _pitch += pitch;
      if( _pitch > 90 )
	_pitch = 90;
      else if( _pitch < 0 )
	_pitch = 0;
    }

    void setScale( double scale ) { _scale = scale; }
    void setPose( double x, double y) { _x = x; _y = y; }

    void scale( double scale, double shift_x = 0, double h = 0, double shift_y = 0, double w = 0 );
    void reset( void ) { _pitch = _yaw = 0; }

    double scale() const { return _scale; }

    void Load( Worldfile* wf, int sec );
    void Save( Worldfile* wf, int sec );
  };


  /** Extends World to implement an FLTK / OpenGL graphical user
      interface.
  */
  class WorldGui : public World, public Fl_Window
  {
    friend class Canvas;
    friend class ModelCamera;
    friend class Model;
    friend class Option;

  private:

    Canvas* canvas;
    std::vector<Option*> drawOptions;
    FileManager* fileMan; ///< Used to load and save worldfiles
    std::vector<usec_t> interval_log;

    /** Stage attempts to run this many times faster than real
	time. If -1, Stage runs as fast as possible. */
    double speedup;

    Fl_Menu_Bar* mbar;
    OptionsDlg* oDlg;
    bool pause_time;

    /** The amount of real time elapsed between $timing_interval
	timesteps. */
    usec_t real_time_interval;

    /** The current real time in microseconds. */
    usec_t real_time_now;

    /** The last recorded real time, sampled every $timing_interval
	updates. */
    usec_t real_time_recorded;

    /** Number of updates between measuring elapsed real time. */
    uint64_t timing_interval;

    // static callback functions
    static void windowCb( Fl_Widget* w, WorldGui* wg );
    static void fileLoadCb( Fl_Widget* w, WorldGui* wg );
    static void fileSaveCb( Fl_Widget* w, WorldGui* wg );
    static void fileSaveAsCb( Fl_Widget* w, WorldGui* wg );
    static void fileExitCb( Fl_Widget* w, WorldGui* wg );
    static void viewOptionsCb( OptionsDlg* oDlg, WorldGui* wg );
    static void optionsDlgCb( OptionsDlg* oDlg, WorldGui* wg );
    static void helpAboutCb( Fl_Widget* w, WorldGui* wg );
    static void pauseCb( Fl_Widget* w, WorldGui* wg );
    static void onceCb( Fl_Widget* w, WorldGui* wg );
    static void fasterCb( Fl_Widget* w, WorldGui* wg );
    static void slowerCb( Fl_Widget* w, WorldGui* wg );
    static void realtimeCb( Fl_Widget* w, WorldGui* wg );
    static void fasttimeCb( Fl_Widget* w, WorldGui* wg );
    static void resetViewCb( Fl_Widget* w, WorldGui* wg );
    static void moreHelptCb( Fl_Widget* w, WorldGui* wg );

    // GUI functions
    bool saveAsDialog();
    bool closeWindowQuery();

    virtual void AddModel( Model* mod );

    void SetTimeouts();


  protected:

    virtual void PushColor( Color col );
    virtual void PushColor( double r, double g, double b, double a );
    virtual void PopColor();

    void DrawOccupancy() const;
    void DrawVoxels() const;

  public:

    WorldGui(int W,int H,const char*L=0);
    ~WorldGui();

    /** Forces the window to be redrawn, even if paused.*/
    virtual void Redraw( void );

    virtual std::string ClockString() const;
    virtual bool Update();
    virtual void Load( const std::string& filename );
    virtual void UnLoad();
    virtual bool Save( const char* filename );
    virtual bool IsGUI() const { return true; };
    virtual Model* RecentlySelectedModel() const;

    virtual void Start();
    virtual void Stop();

    usec_t RealTimeNow(void) const;

    void DrawBoundingBoxTree();

    Canvas* GetCanvas( void ) const { return canvas; }

    /** show the window - need to call this if you don't Load(). */
    void Show();

    /** Get human readable string that describes the current global energy state. */
    std::string EnergyString( void ) const;
    virtual void RemoveChild( Model* mod );

    bool IsTopView();
  };


  class StripPlotVis : public Visualizer
  {
  private:

    Model* mod;
    float* data;
    size_t len;
    size_t count;
    unsigned int index;
    float x,y,w,h,min,max;
    Color fgcolor, bgcolor;

  public:
    StripPlotVis( float x, float y, float w, float h,
		  size_t len,
		  Color fgcolor, Color bgcolor,
		  const char* name, const char* wfname );
    virtual ~StripPlotVis();
    virtual void Visualize( Model* mod, Camera* cam );
    void AppendValue( float value );
  };


  class PowerPack
  {
    friend class WorldGui;
    friend class Canvas;

  protected:

    class DissipationVis : public Visualizer
    {
    private:
      unsigned int columns, rows;
      meters_t width, height;

      std::vector<joules_t> cells;

      joules_t peak_value;
      double cellsize;

      static joules_t global_peak_value;

    public:
      DissipationVis( meters_t width,
		      meters_t height,
		      meters_t cellsize );

      virtual ~DissipationVis();
      virtual void Visualize( Model* mod, Camera* cam );

      void Accumulate( meters_t x, meters_t y, joules_t amount );
    } event_vis;


    StripPlotVis output_vis;
    StripPlotVis stored_vis;

    /** The model that owns this object */
    Model* mod;

    /** Energy stored */
    joules_t stored;

    /** Energy capacity */
    joules_t capacity;

    /** TRUE iff the device is receiving energy */
    bool charging;

    /** Energy dissipated */
    joules_t dissipated;

    // these are used to visualize the power draw
    usec_t last_time;
    joules_t last_joules;
    watts_t last_watts;

  public:
    static joules_t global_stored;
    static joules_t global_capacity;
    static joules_t global_dissipated;
    static joules_t global_input;

  public:
    PowerPack( Model* mod );
    ~PowerPack();

    /** OpenGL visualization of the powerpack state */
    void Visualize( Camera* cam );

    /** Returns the energy capacity minus the current amount stored */
    joules_t RemainingCapacity() const;

    /** Add to the energy store */
    void Add( joules_t j );

    /** Subtract from the energy store */
    void Subtract( joules_t j );

    /** Transfer some stored energy to another power pack */
    void TransferTo( PowerPack* dest, joules_t amount );

    double ProportionRemaining() const
    { return( stored / capacity ); }

    /** Print human-readable status on stdout, prefixed with the
	argument string, or NULL */
    void Print( const char* prefix ) const
    {
      if( prefix )
	printf( "%s", prefix );

      printf( "PowerPack %.2f/%.2f J\n", stored, capacity );
    }

    joules_t GetStored() const;
    joules_t GetCapacity() const;
    joules_t GetDissipated() const;
    void SetCapacity( joules_t j );
    void SetStored( joules_t j );

    /** Returns true iff the device received energy at the last timestep */
    bool GetCharging() const { return charging; }

    void ChargeStart(){ charging = true; }
    void ChargeStop(){ charging = false; }

    /** Lose energy as work or heat */
    void Dissipate( joules_t j );

    /** Lose energy as work or heat, and record the event */
    void Dissipate( joules_t j, const Pose& p );
  };


  /// %Model class
  class Model : public Ancestor
  {
    friend class Ancestor;
    friend class World;
    friend class World::Event;
    friend class WorldGui;
    friend class Canvas;
    friend class Block;
    friend class Region;
    friend class BlockGroup;
    friend class PowerPack;
    friend class Ray;
    friend class ModelFiducial;

  private:
    /** the number of models instatiated - used to assign unique sequential IDs */
    static uint32_t count;
    static std::map<id_t,Model*> modelsbyid;

    /** records if this model has been mapped into the world bitmap*/
    bool mapped;

    std::vector<Option*> drawOptions;
    const std::vector<Option*>& getOptions() const { return drawOptions; }

  protected:

    /** If true, the model always has at least one subscription, so
	always runs. Defaults to false. */
    bool alwayson;

    BlockGroup blockgroup;

    /** Iff true, 4 thin blocks are automatically added to the model,
	forming a solid boundary around the bounding box of the
	model. */
    int boundary;

    /** container for a callback function and a single argument, so
	they can be stored together in a list with a single pointer. */
  public:
    class cb_t
    {
    public:
      model_callback_t callback;
      void* arg;

      cb_t( model_callback_t cb, void* arg )
	: callback(cb), arg(arg) {}

      cb_t( world_callback_t cb, void* arg )
	: callback(NULL), arg(arg) { (void)cb; }

      cb_t() : callback(NULL), arg(NULL) {}

      /** for placing in a sorted container */
      bool operator<( const cb_t& other ) const
      {
	if( callback == other.callback )
	  return( arg < other.arg );
	//else
	return ((void*)(callback)) < ((void*)(other.callback));
      }

      /** for searching in a sorted container */
      bool operator==( const cb_t& other ) const
      { return( callback == other.callback);  }
    };

    class Flag
    {
    private:
      Color color;
      double size;
      int displaylist;

    public:
      void SetColor( const Color& col );
      void SetSize( double sz );

      Color GetColor(){ return color; }
      double GetSize(){ return size; }

      Flag( const Color& color, double size );
      Flag* Nibble( double portion );

      /** Draw the flag in OpenGl. Takes a quadric parameter to save
	  creating the quadric for each flag */
      void Draw(  GLUquadric* quadric );
    };

    typedef enum {
      CB_FLAGDECR,
      CB_FLAGINCR,
      CB_GEOM,
      CB_INIT,
      CB_LOAD,
      CB_PARENT,
      CB_POSE,
      CB_SAVE,
      CB_SHUTDOWN,
      CB_STARTUP,
      CB_UPDATE,
      CB_VELOCITY,
      //CB_POSTUPDATE,
      __CB_TYPE_COUNT // must be the last entry: counts the number of types
    } callback_type_t;

  protected:
    /** A list of callback functions can be attached to any
	address. When Model::CallCallbacks( void*) is called, the
	callbacks are called.*/
    std::vector<std::set<cb_t> > callbacks;

    /** Default color of the model's blocks.*/
    Color color;

    /** This can be set to indicate that the model has new data that
	may be of interest to users. This allows polling the model
	instead of adding a data callback. */
    bool data_fresh;

    /** If set true, Update() is not called on this model. Useful
	e.g. for temporarily disabling updates when dragging models
	with the mouse.*/
    bool disabled;

    /** Container for Visualizers attached to this model. */
    std::list<Visualizer*> cv_list;

    /** Container for flags attached to this model. */
    std::list<Flag*> flag_list;

    /** Model the interaction between the model's blocks and the
	surface they touch. @todo primitive at the moment */
    double friction;

    /** Specifies the the size of this model's bounding box, and the
	offset of its local coordinate system wrt that its parent. */
    Geom geom;

    /** Records model state and functionality in the GUI, if used */
    class GuiState
    {
    public:
      bool grid;
      bool move;
      bool nose;
      bool outline;

      GuiState();
      GuiState& Load( Worldfile* wf, int wf_entity );
    } gui;

    bool has_default_block;

    /** unique process-wide identifier for this model */
    uint32_t id;
    usec_t interval; ///< time between updates in usec
    usec_t interval_energy; ///< time between updates of powerpack in usec
    usec_t last_update; ///< time of last update in us
    bool log_state; ///< iff true, model state is logged
    meters_t map_resolution;
    kg_t mass;

    /** Pointer to the parent of this model, possibly NULL. */
    Model* parent;

    /** The pose of the model in it's parents coordinate frame, or the
	global coordinate frame is the parent is NULL. */
    Pose pose;

    /** Optional attached PowerPack, defaults to NULL */
    PowerPack* power_pack;

    /** list of powerpacks that this model is currently charging,
	initially NULL. */
    std::list<PowerPack*> pps_charging;

    /** Visualize the most recent rasterization operation performed by this model */
    class RasterVis : public Visualizer
    {
    private:
      uint8_t* data;
      unsigned int width, height;
      meters_t cellwidth, cellheight;
      std::vector<point_t> pts;

    public:
      RasterVis();
      virtual ~RasterVis( void ){}
      virtual void Visualize( Model* mod, Camera* cam );

      void SetData( uint8_t* data,
		    unsigned int width,
		    unsigned int height,
		    meters_t cellwidth,
		    meters_t cellheight );

      int subs;     //< the number of subscriptions to this model
      int used;     //< the number of connections to this model

      void AddPoint( meters_t x, meters_t y );
      void ClearPts();

    } rastervis;

    bool rebuild_displaylist; ///< iff true, regenerate block display list before redraw
    std::string say_string;   ///< if non-empty, this string is displayed in the GUI

    bool stack_children; ///< whether child models should be stacked on top of this model or not

    bool stall; ///< Set to true iff the model collided with something else
    int subs;    ///< the number of subscriptions to this model
    /** Thread safety flag. Iff true, Update() may be called in
	parallel with other models. Defaults to false for
	safety. Derived classes can set it true in their constructor to
	allow parallel Updates(). */
    bool thread_safe;

    /** Cache of recent poses, used to draw the trail. */
    class TrailItem
    {
    public:
      usec_t time;
      Pose pose;
      Color color;

      TrailItem()
	: time(0), pose(), color(){}

      //TrailItem( usec_t time, Pose pose, Color color )
      //: time(time), pose(pose), color(color){}
    };

    /** a ring buffer for storing recent poses */
    std::vector<TrailItem> trail;

    /** current position in the ring buffer */
    unsigned int trail_index;

    /** The maxiumum length of the trail drawn. Default is 20, but can
	be set in the world file using the trail_length model
	property. */
    static unsigned int trail_length;

    /** Number of world updates between trail records. */
    static uint64_t trail_interval;

    /** Record the current pose in our trail. Delete the trail head if it is full. */
    void UpdateTrail();

    //model_type_t type;
    const std::string type;
    /** The index into the world's vector of event queues. Initially
	-1, to indicate that it is not on a list yet. */
    unsigned int event_queue_num;
    bool used;   ///< TRUE iff this model has been returned by GetUnusedModelOfType()

    watts_t watts;///< power consumed by this model

    /** If >0, this model can transfer energy to models that have
	watts_take >0 */
    watts_t watts_give;

    /** If >0, this model can transfer energy from models that have
	watts_give >0 */
    watts_t watts_take;

    Worldfile* wf;
    int wf_entity;
    World* world; // pointer to the world in which this model exists
    WorldGui* world_gui; //pointer to the GUI world - NULL if running in non-gui mode

  public:

    virtual void SetToken( const std::string& str )
    {
      //printf( "Model::SetToken( %s )\n", str.c_str() );

      if( str.size() > 0 )
	{
	  world->AddModelName( this, str );
	  Ancestor::SetToken( str );
	}
      else
	PRINT_ERR( "Model::SetToken() called with zero length string. Ignored." );
    }


    const std::string& GetModelType() const {return type;}
    std::string GetSayString(){return std::string(say_string);}

    /** Returns a pointer to the model identified by name, or NULL if
	it doesn't exist in this model. */
    Model* GetChild( const std::string& name ) const;

    /** return the update interval in usec */
    usec_t GetInterval(){ return interval; }

    class Visibility
    {
    public:
      bool blob_return;
      int fiducial_key;
      int fiducial_return;
      bool gripper_return;
      bool obstacle_return;
      double ranger_return; // 0 - 1

      Visibility();

      Visibility& Load( Worldfile* wf, int wf_entity );
      void Save( Worldfile* wf, int wf_entity );
    } vis;

    usec_t GetUpdateInterval() const { return interval; }
    usec_t GetEnergyInterval() const { return interval_energy; }
    //    usec_t GetPoseInterval() const { return interval_pose; }

    /** Render the model's blocks as an occupancy grid into the
	preallocated array of width by height pixels */
    void Rasterize( uint8_t* data,
		    unsigned int width, unsigned int height,
		    meters_t cellwidth, meters_t cellheight );

  private:
    /** Private copy constructor declared but not defined, to make it
	impossible to copy models. */
    explicit Model(const Model& original);

    /** Private assignment operator declared but not defined, to make it
	impossible to copy models */
    Model& operator=(const Model& original);

  protected:

    /** Register an Option for pickup by the GUI. */
    void RegisterOption( Option* opt );

    void AppendTouchingModels( std::set<Model*>& touchers );

    /** Check to see if the current pose will yield a collision with
	obstacles.  Returns a pointer to the first entity we are in
	collision with, or NULL if no collision exists.  Recursively
	calls TestCollision() on all descendents. */
    Model* TestCollision();

    void CommitTestedPose();

    void Map( unsigned int layer );

    /** Call Map on all layers */
    inline void Map(){ Map(0); Map(1); }

    void UnMap( unsigned int layer );

    /** Call UnMap on all layers */
    inline void UnMap(){ UnMap(0); UnMap(1); }

    void MapWithChildren( unsigned int layer );
    void UnMapWithChildren( unsigned int layer );

    // Find the root model, and map/unmap the whole tree.
    void MapFromRoot( unsigned int layer );
    void UnMapFromRoot( unsigned int layer );

    /** raytraces a single ray from the point and heading identified by
	pose, in local coords */
    RaytraceResult Raytrace( const Pose &pose,
			     const meters_t range,
			     const ray_test_func_t func,
			     const void* arg,
			     const bool ztest )
    {
      return world->Raytrace( LocalToGlobal(pose),
			      range,
			      func,
			      this,
			      arg,
			      ztest );
    }

    /** raytraces multiple rays around the point and heading identified
	by pose, in local coords */
    void Raytrace( const Pose &pose,
     		   const meters_t range,
     		   const radians_t fov,
     		   const ray_test_func_t func,
     		   const void* arg,
     		   const bool ztest,
		       std::vector<RaytraceResult>& results )
    {
      return world->Raytrace( LocalToGlobal(pose),
			      range,
			      fov,
			      func,
			      this,
			      arg,
			      ztest,
			      results );
    }

    virtual void UpdateCharge();

    static int UpdateWrapper( Model* mod, void* arg ){ mod->Update(); return 0; }

    /** Calls CallCallback( CB_UPDATE ) */
    void CallUpdateCallbacks( void );

    meters_t ModelHeight() const;

    void DrawBlocksTree();
    virtual void DrawBlocks();
    void DrawBoundingBox();
    void DrawBoundingBoxTree();
    virtual void DrawStatus( Camera* cam );
    void DrawStatusTree( Camera* cam );

    void DrawOriginTree();
    void DrawOrigin();

    void PushLocalCoords();
    void PopCoords();

    /** Draw the image stored in texture_id above the model */
    void DrawImage( uint32_t texture_id, Camera* cam, float alpha, double width=1.0, double height=1.0 );

    virtual void DrawPicker();
    virtual void DataVisualize( Camera* cam );
    virtual void DrawSelected(void);

    void DrawTrailFootprint();
    void DrawTrailBlocks();
    void DrawTrailArrows();
    void DrawGrid();
    //	void DrawBlinkenlights();
    void DataVisualizeTree( Camera* cam );
    void DrawFlagList();
    void DrawPose( Pose pose );

  public:
    virtual void PushColor( Color col ){ world->PushColor( col ); }
    virtual void PushColor( double r, double g, double b, double a ){ world->PushColor( r,g,b,a ); }
    virtual void PopColor()	{ world->PopColor(); }

    PowerPack* FindPowerPack() const;

    //void RecordRenderPoint( GSList** head, GSList* link,
    //					unsigned int* c1, unsigned int* c2 );

    void PlaceInFreeSpace( meters_t xmin, meters_t xmax,
			   meters_t ymin, meters_t ymax );

    /** Return a human-readable string describing the model's pose */
    std::string PoseString()
    { return pose.String(); }

    /** Look up a model pointer by a unique model ID */
    static Model* LookupId( uint32_t id )
    { return modelsbyid[id]; }

    /** Constructor */
    Model( World* world,
	   Model* parent = NULL,
	   const std::string& type = "model",
	   const std::string& name = "" );

    /** Destructor */
    virtual ~Model();

    /** Alternate constructor that creates dummy models with only a pose */
	 Model()
	   : mapped(false), alwayson(false), blockgroup(*this),
		  boundary(false), data_fresh(false), disabled(true), friction(0), has_default_block(false), log_state(false), map_resolution(0), mass(0), parent(NULL), rebuild_displaylist(false), stack_children(true), stall(false), subs(0), thread_safe(false),trail_index(0), event_queue_num(0), used(false), watts(0), watts_give(0),watts_take(0),wf(NULL), wf_entity(0), world(NULL)
	 {}

    void Say( const std::string& str );

    /** Attach a user supplied visualization to a model. */
    void AddVisualizer( Visualizer* custom_visual, bool on_by_default );

    /** remove user supplied visualization to a model - supply the same ptr passed to AddCustomVisualizer */
    void RemoveVisualizer( Visualizer* custom_visual );

    void BecomeParentOf( Model* child );

    void Load( Worldfile* wf, int wf_entity )
    {
      /** Set the worldfile and worldfile entity ID - must be called
	  before Load() */
      SetWorldfile( wf, wf_entity );
      Load(); // call virtual load
    }

    /** Set the worldfile and worldfile entity ID */
    void SetWorldfile( Worldfile* wf, int wf_entity )
    { this->wf = wf; this->wf_entity = wf_entity; }

    /** configure a model by reading from the current world file */
    virtual void Load();

    /** save the state of the model to the current world file */
    virtual void Save();

    /** Call Init() for all attached controllers. */
    void InitControllers();

    void AddFlag(  Flag* flag );
    void RemoveFlag( Flag* flag );

    void PushFlag( Flag* flag );
    Flag* PopFlag();

    unsigned int GetFlagCount() const { return flag_list.size(); }

    /** Disable the model. Its pose will not change due to velocity
	until re-enabled using Enable(). This is used for example when
	dragging a model with the mouse pointer. The model is enabled by
	default. */
    void Disable(){ disabled = true; };

    /** Enable the model, so that non-zero velocities will change the
	model's pose. Models are enabled by default. */
    void Enable(){ disabled = false; };

    /** Load a control program for this model from an external
	library */
    void LoadControllerModule( const char* lib );

    /** Sets the redraw flag, so this model will be redrawn at the
	earliest opportunity */
    void NeedRedraw();

    /** Force the GUI (if any) to redraw this model */
    void Redraw();

    /** Add a block to this model by loading it from a worldfile
	entity */
    void LoadBlock( Worldfile* wf, int entity );

    /** Add a block to this model centered at [x,y] with extent [dx, dy,
	dz] */
    void	AddBlockRect( meters_t x, meters_t y,
			      meters_t dx, meters_t dy,
			      meters_t dz );

    /** remove all blocks from this model, freeing their memory */
    void ClearBlocks();

    /** Returns a pointer to this model's parent model, or NULL if this
	model has no parent */
    Model* Parent() const { return this->parent; }

    /** Returns a pointer to the world that contains this model */
    World* GetWorld() const { return this->world; }

    /** return the root model of the tree containing this model */
    Model* Root(){ return(  parent ? parent->Root() : this ); }

    bool IsAntecedent( const Model* testmod ) const;

    /** returns true if model [testmod] is a descendent of this model */
    bool IsDescendent( const Model* testmod ) const;

    /** returns true if model [testmod] is a descendent or antecedent of this model */
    bool IsRelated( const Model* testmod ) const;

    /** get the pose of a model in the global CS */
    Pose GetGlobalPose() const;

    /** subscribe to a model's data */
    void Subscribe();

    /** unsubscribe from a model's data */
    void Unsubscribe();

    /** set the pose of model in global coordinates */
    void SetGlobalPose(  const Pose& gpose );

    /** set a model's pose in its parent's coordinate system */
    void SetPose(  const Pose& pose );

    /** add values to a model's pose in its parent's coordinate system */
    void AddToPose(  const Pose& pose );

    /** add values to a model's pose in its parent's coordinate system */
    void AddToPose(  double dx, double dy, double dz, double da );

    /** set a model's geometry (size and center offsets) */
    void SetGeom(  const Geom& src );

    /** Set a model's fiducial return value. Values less than zero
	are not detected by the fiducial sensor. */
    void SetFiducialReturn(  int fid );

    /** Get a model's fiducial return value. */
    int GetFiducialReturn()  const { return vis.fiducial_return; }

    /** set a model's fiducial key: only fiducial finders with a
	matching key can detect this model as a fiducial. */
    void SetFiducialKey(  int key );

    Color GetColor() const { return color; }

    /** return a model's unique process-wide identifier */
    uint32_t GetId()  const { return id; }

    /** Get the total mass of a model and it's children recursively */
    kg_t GetTotalMass() const;

    /** Get the mass of this model's children recursively. */
    kg_t GetMassOfChildren() const;

    /** Change a model's parent - experimental*/
    int SetParent( Model* newparent);

    /** Get (a copy of) the model's geometry - it's size and local
	pose (offset from origin in local coords). */
    Geom GetGeom() const { return geom; }

    /** Get (a copy of) the pose of a model in its parent's coordinate
	system.  */
    Pose GetPose() const { return pose; }


    // guess what these do?
    void SetColor( Color col );
    void SetMass( kg_t mass );
    void SetStall( bool stall );
    void SetGravityReturn( bool val );
    void SetGripperReturn( bool val );
    void SetStickyReturn( bool val );
    void SetRangerReturn( double val );
    void SetObstacleReturn( bool val );
    void SetBlobReturn( bool val );
    void SetRangerReturn( bool val );
    void SetBoundary( bool val );
    void SetGuiNose( bool val );
    void SetGuiMove( bool val );
    void SetGuiGrid( bool val );
    void SetGuiOutline( bool val );
    void SetWatts( watts_t watts );
    void SetMapResolution( meters_t res );
    void SetFriction( double friction );

    bool DataIsFresh() const { return this->data_fresh; }

    /* attach callback functions to data members. The function gets
       called when the member is changed using SetX() accessor method */

    /** Add a callback. The specified function is called whenever the
	indicated model method is called, and passed the user
	data.  @param cb Pointer the function to be called.  @param
	user Pointer to arbitrary user data, passed to the callback
	when called.
    */
    void AddCallback( callback_type_t type,
		      model_callback_t cb,
		      void* user );

    int RemoveCallback( callback_type_t type,
			model_callback_t callback );

    int CallCallbacks(  callback_type_t type );


    virtual void Print( char* prefix ) const;
    virtual const char* PrintWithPose() const;

    /** Given a global pose, returns that pose in the model's local
	coordinate system. */
    Pose GlobalToLocal( const Pose& pose ) const;

    /** Return the global pose (i.e. pose in world coordinates) of a
	pose specified in the model's local coordinate system */
    Pose LocalToGlobal( const Pose& pose ) const
    {
      return( ( GetGlobalPose() + geom.pose ) + pose );
    }

    /** Return a vector of global pixels corresponding to a vector of local points. */
    std::vector<point_int_t>  LocalToPixels( const std::vector<point_t>& local ) const;

    /** Return the 2d point in world coordinates of a 2d point
	specified in the model's local coordinate system */
    point_t LocalToGlobal( const point_t& pt) const;

    /** returns the first descendent of this model that is unsubscribed
	and has the type indicated by the string */
    Model* GetUnsubscribedModelOfType( const std::string& type ) const;

    /** returns the first descendent of this model that is unused
	and has the type indicated by the string. This model is tagged as used. */
    Model* GetUnusedModelOfType( const std::string& type );

    /** Returns the value of the model's stall boolean, which is true
	iff the model has crashed into another model */
    bool Stalled() const { return this->stall; }

    /** Returns the current number of subscriptions. If alwayson, this
	is never less than 1.*/
    unsigned int GetSubscriptionCount() const { return subs; }

    /** Returns true if the model has one or more subscribers, else false. */
    bool HasSubscribers() const { return( subs > 0 ); }

    static std::map< std::string, creator_t> name_map;

  protected:
    virtual void Startup();
    virtual void Shutdown();
    virtual void Update();
  };


  // BLOBFINDER MODEL --------------------------------------------------------
  /// %ModelBlobfinder class
  class ModelBlobfinder : public Model
  {
  public:
    /** Sample data */
    class Blob
    {
    public:
      Color color;
      uint32_t left, top, right, bottom;
      meters_t range;
    };

    /** Visualize blobinder data in the GUI. */
    class Vis : public Visualizer
    {
    public:
      Vis( World* world );
      virtual ~Vis( void ){}
      virtual void Visualize( Model* mod, Camera* cam );
    } vis;

  private:
    /** Vector of blobs detected in the field of view. Use GetBlobs()
	or GetBlobsMutable() to acess the vector. */
    std::vector<Blob> blobs;

    /** The sensor detects model blocks of each of these colors. Use
	AddColor() and RemoveColor()
	to add and remove colors at run time.*/
    std::vector<Color> colors;

    // predicate for ray tracing
    static bool BlockMatcher( Block* testblock, Model* finder );

  public:
    radians_t fov; ///< Horizontal field of view in radians, in the range 0 to pi.
    radians_t pan; ///< Horizontal pan angle in radians, in the range -pi to +pi.
    meters_t range; ///< Maximum distance at which blobs are detected (a little artificial, but setting this small saves computation  time.
    unsigned int scan_height; ///< Height of the input image in pixels.
    unsigned int scan_width; ///< Width of the input image in pixels.

    // constructor
    ModelBlobfinder( World* world,
		     Model* parent,
		     const std::string& type );
    // destructor
    ~ModelBlobfinder();

    virtual void Startup();
    virtual void Shutdown();
    virtual void Update();
    virtual void Load();

    /** Returns a non-mutable const reference to the detected blob
	data. Use this if you don't need to modify the model's
	internal data, e.g. if you want to copy it into a new
	vector.*/
    const std::vector<Blob>& GetBlobs() const { return blobs; }

    /** Returns a mutable reference to the model's internal detected
	blob data. Use this with caution, if at all. */
    std::vector<Blob>& GetBlobsMutable() { return blobs; }

    /** Start finding blobs with this color.*/
    void AddColor( Color col );

    /** Stop tracking blobs with this color */
    void RemoveColor( Color col );

    /** Stop tracking all colors. Call this to clear the defaults, then
	add colors individually with AddColor(); */
    void RemoveAllColors();
  };





  // Light indicator model
  class ModelLightIndicator : public Model
  {
  public:
    ModelLightIndicator( World* world,
			 Model* parent,
			 const std::string& type );
    ~ModelLightIndicator();

    void SetState(bool isOn);

  protected:
    virtual void DrawBlocks();

  private:
    bool m_IsOn;
  };

  // \todo  GRIPPER MODEL --------------------------------------------------------


  class ModelGripper : public Model
  {
  public:

    enum paddle_state_t {
      PADDLE_OPEN = 0, // default state
      PADDLE_CLOSED,
      PADDLE_OPENING,
      PADDLE_CLOSING,
    };

    enum lift_state_t {
      LIFT_DOWN = 0, // default state
      LIFT_UP,
      LIFT_UPPING, // verbed these to match the paddle state
      LIFT_DOWNING,
    };

    enum cmd_t {
      CMD_NOOP = 0, // default state
      CMD_OPEN,
      CMD_CLOSE,
      CMD_UP,
      CMD_DOWN
    };


    /** gripper configuration
     */
    struct config_t
    {
      Size paddle_size; ///< paddle dimensions
      paddle_state_t paddles;
      lift_state_t lift;
      double paddle_position; ///< 0.0 = full open, 1.0 full closed
      double lift_position; ///< 0.0 = full down, 1.0 full up
      Model* gripped;
      bool paddles_stalled; // true iff some solid object stopped the paddles closing or opening
      double close_limit; ///< How far the gripper can close. If < 1.0, the gripper has its mouth full.
      bool autosnatch; ///< if true, cycle the gripper through open-close-up-down automatically
      double break_beam_inset[2]; ///< distance from the end of the paddle
      Model* beam[2]; ///< points to a model detected by the beams
      Model* contact[2]; ///< pointers to a model detected by the contacts
    };

  private:
    virtual void Update();
    virtual void DataVisualize( Camera* cam );

    void FixBlocks();
    void PositionPaddles();
    void UpdateBreakBeams();
    void UpdateContacts();

    config_t cfg;
    cmd_t cmd;

    Block* paddle_left;
    Block* paddle_right;

    static Option showData;

  public:
    static const Size size;

    // constructor
    ModelGripper( World* world,
		  Model* parent,
		  const std::string& type );
    // destructor
    virtual ~ModelGripper();

    virtual void Load();
    virtual void Save();

    /** Configure the gripper */
    void SetConfig( config_t & newcfg ){ this->cfg = newcfg; FixBlocks(); }

    /** Returns the state of the gripper .*/
    config_t GetConfig(){ return cfg; };

    /** Set the current activity of the gripper. */
    void SetCommand( cmd_t cmd ) { this->cmd = cmd; }
    /** Command the gripper paddles to close. Wrapper for SetCommand( CMD_CLOSE ). */
    void CommandClose() { SetCommand( CMD_CLOSE ); }
    /** Command the gripper paddles to open. Wrapper for SetCommand( CMD_OPEN ). */
    void CommandOpen() { SetCommand( CMD_OPEN ); }
    /** Command the gripper lift to go up. Wrapper for SetCommand( CMD_UP ). */
    void CommandUp() { SetCommand( CMD_UP ); }
    /** Command the gripper lift to go down. Wrapper for SetCommand( CMD_DOWN ). */
    void CommandDown() { SetCommand( CMD_DOWN ); }
  };


  // \todo BUMPER MODEL --------------------------------------------------------

  //   typedef struct
  //   {
  //     Pose pose;
  //     meters_t length;
  //   } bumper_config_t;

  //   typedef struct
  //   {
  //     Model* hit;
  //     point_t hit_point;
  //   } bumper_sample_t;


  // FIDUCIAL MODEL --------------------------------------------------------

  /// %ModelFiducial class
  class ModelFiducial : public Model
  {
  public:
    /** Detected fiducial data */
    class Fiducial
    {
    public:
      meters_t range; ///< range to the target
      radians_t bearing; ///< bearing to the target
      Pose geom; ///< size and relative angle of the target
		 //Pose pose_rel; /// relative pose of the target in local coordinates
      Pose pose; ///< Absolute accurate position of the target in world coordinates (it's cheating to use this in robot controllers!)
      Model* mod; ///< Pointer to the model (real fiducial detectors can't do this!)
      int id; ///< the fiducial identifier of the target (i.e. its fiducial_return value), or -1 if none can be detected.
    };

  private:
    // if neighbor is visible, add him to the fiducial scan
    void AddModelIfVisible( Model* him );

    virtual void Update();
    virtual void DataVisualize( Camera* cam );

    static Option showData;
    static Option showFov;

    std::vector<Fiducial> fiducials;

  public:
    ModelFiducial( World* world,
		   Model* parent,
		   const std::string& type );
    virtual ~ModelFiducial();

    virtual void Load();
    void Shutdown( void );

    meters_t max_range_anon;///< maximum detection range
    meters_t max_range_id; ///< maximum range at which the ID can be read
    meters_t min_range; ///< minimum detection range
    radians_t fov; ///< field of view
    radians_t heading; ///< center of field of view
    int key; ///< /// only detect fiducials with a key that matches this one (defaults 0)
    bool ignore_zloc;  ///< Are we ignoring the Z-loc of the fiducials we detect compared to the fiducial detector?

    /** Access the dectected fiducials. C++ style. */
    std::vector<Fiducial>& GetFiducials() { return fiducials; }

    /** Access the dectected fiducials, C style. */
    Fiducial* GetFiducials( unsigned int* count )
    {
      if( count ) *count = fiducials.size();
      return &fiducials[0];
    }
  };


  // RANGER MODEL --------------------------------------------------------

  /// %ModelRanger class
  class ModelRanger : public Model
  {
  public:
  public:
    ModelRanger( World* world, Model* parent, const std::string& type );
    virtual ~ModelRanger();

    virtual void Print( char* prefix ) const;

    class Vis : public Visualizer
    {
    public:
      static Option showArea;
      static Option showStrikes;
      static Option showFov;
      static Option showBeams;
      static Option showTransducers;

      Vis( World* world );
      virtual ~Vis( void ){}
      virtual void Visualize( Model* mod, Camera* cam );
    } vis;

    class Sensor
    {
    public:
      Pose pose;
      Size size;
      Bounds range;
      radians_t fov;
      unsigned int sample_count;
      Color color;

      std::vector<meters_t> ranges;
      std::vector<double> intensities;
      std::vector<double> bearings;

      Sensor() : pose( 0,0,0,0 ),
		 size( 0.02, 0.02, 0.02 ), // teeny transducer
		 range( 0.0, 5.0 ),
		 fov( 0.1 ),
		 sample_count(1),
		 color( Color(0,0,1,0.15)),
		 ranges(),
		 intensities(),
		 bearings()
      {}

      void Update( ModelRanger* rgr );
      void Visualize( Vis* vis, ModelRanger* rgr ) const;
      std::string String() const;
      void Load( Worldfile* wf, int entity );
    };

    /** returns a const reference to a vector of range and reflectance samples */
    const std::vector<Sensor>& GetSensors() const
    { return sensors; }

    /** returns a mutable reference to a vector of range and reflectance samples */
    std::vector<Sensor>& GetSensorsMutable()
    { return sensors; }

    void LoadSensor( Worldfile* wf, int entity );

  private:
    std::vector<Sensor> sensors;

  protected:

    virtual void Startup();
    virtual void Shutdown();
    virtual void Update();
  };

  // BLINKENLIGHT MODEL ----------------------------------------------------
  class ModelBlinkenlight : public Model
  {
  private:
    double dutycycle;
    bool enabled;
    msec_t period;
    bool on;

    static Option showBlinkenData;
  public:
    ModelBlinkenlight( World* world,
		       Model* parent,
		       const std::string& type );

    ~ModelBlinkenlight();

    virtual void Load();
    virtual void Update();
    virtual void DataVisualize( Camera* cam );
  };


  // CAMERA MODEL ----------------------------------------------------

  /// %ModelCamera class
  class ModelCamera : public Model
  {
  public:
    typedef struct
    {
      // GL_V3F
      GLfloat x, y, z;
    } ColoredVertex;

  private:
    Canvas* _canvas;

    GLfloat* _frame_data;  //opengl read buffer
    GLubyte* _frame_color_data;  //opengl read buffer

    bool _valid_vertexbuf_cache;
    ColoredVertex* _vertexbuf_cache; //cached unit vectors with appropriate rotations (these must be scalled by z-buffer length)

    int _width;         //width of buffer
    int _height;        //height of buffer
    static const int _depth = 4;

    int _camera_quads_size;
    GLfloat* _camera_quads;
    GLubyte* _camera_colors;

    static Option showCameraData;

    PerspectiveCamera _camera;
    double _yaw_offset; //position camera is mounted at
    double _pitch_offset;

    ///Take a screenshot from the camera's perspective. return: true for sucess, and data is available via FrameDepth() / FrameColor()
    bool GetFrame();

  public:
    ModelCamera( World* world,
		 Model* parent,
		 const std::string& type );

    ~ModelCamera();

    virtual void Load();

    ///Capture a new frame ( calls GetFrame )
    virtual void Update();

    ///Draw Camera Model - TODO
    //virtual void Draw( uint32_t flags, Canvas* canvas );

    ///Draw camera visualization
    virtual void DataVisualize( Camera* cam );

    ///width of captured image
    int getWidth( void ) const { return _width; }

    ///height of captured image
    int getHeight( void ) const { return _height; }

    ///get reference to camera used
    const PerspectiveCamera& getCamera( void ) const { return _camera; }

    ///get a reference to camera depth buffer
    const GLfloat* FrameDepth() const { return _frame_data; }

    ///get a reference to camera color image. 4 bytes (RGBA) per pixel
    const GLubyte* FrameColor() const { return _frame_color_data; }

    ///change the pitch
    void setPitch( double pitch ) { _pitch_offset = pitch; _valid_vertexbuf_cache = false; }

    ///change the yaw
    void setYaw( double yaw ) { _yaw_offset = yaw; _valid_vertexbuf_cache = false; }
  };

  // POSITION MODEL --------------------------------------------------------

  /// %ModelPosition class
  class ModelPosition : public Model
  {
    friend class Canvas;
    friend class World;

  public:
    /** Define a position  control method */
    typedef enum
      { CONTROL_ACCELERATION,
	CONTROL_VELOCITY,
	CONTROL_POSITION
      } ControlMode;

    /** Define a localization method */
    typedef enum
      { LOCALIZATION_GPS,
	LOCALIZATION_ODOM
      } LocalizationMode;

    /** Define a driving method */
    typedef enum
      { DRIVE_DIFFERENTIAL,
	DRIVE_OMNI,
	DRIVE_CAR
      } DriveMode;

  private:
    Velocity velocity;
    Pose goal;///< the current velocity or pose to reach, depending on the value of control_mode
    ControlMode control_mode;
    DriveMode drive_mode;
    LocalizationMode localization_mode; ///< global or local mode
    Velocity integration_error; ///< errors to apply in simple odometry model
    double wheelbase;

  public:
    /** Set the min and max acceleration in all 4 DOF */
    Bounds acceleration_bounds[4];

    /** Set the min and max velocity in all 4 DOF */
    Bounds velocity_bounds[4];

    // constructor
    ModelPosition( World* world,
		   Model* parent,
		   const std::string& type );
    // destructor
    ~ModelPosition();

    /** Get (a copy of) the model's velocity in its local reference
	frame. */
    Velocity GetVelocity() const { return velocity; }
    void SetVelocity( const Velocity& val );
    /** get the velocity of a model in the global CS */
    Velocity GetGlobalVelocity()  const;
    /* set the velocity of a model in the global coordinate system */
    void SetGlobalVelocity( const Velocity& gvel );

    /** Specify a point in space. Arrays of Waypoints can be attached to
	Models and visualized. */
    class Waypoint
    {
    public:
      Waypoint( meters_t x, meters_t y, meters_t z, radians_t a, Color color ) ;
      Waypoint( const Pose& pose, Color color ) ;
      Waypoint();
      void Draw() const;

      Pose pose;
      Color color;
    };

    std::vector<Waypoint> waypoints;

    class WaypointVis : public Visualizer
    {
    public:
      WaypointVis();
      virtual ~WaypointVis( void ){}
      virtual void Visualize( Model* mod, Camera* cam );
    } wpvis;

    class PoseVis : public Visualizer
    {
    public:
      PoseVis();
      virtual ~PoseVis( void ){}
      virtual void Visualize( Model* mod, Camera* cam );
    } posevis;

    /** Set the current pose estimate.*/
    void SetOdom( Pose odom );

    /** Sets the control_mode to CONTROL_VELOCITY and sets
	the goal velocity. */
    void SetSpeed( double x, double y, double a );
    void SetXSpeed( double x );
    void SetYSpeed( double y );
    void SetZSpeed( double z );
    void SetTurnSpeed( double a );
    void SetSpeed( Velocity vel );
    /** Set velocity along all axes to  to zero. */
    void Stop();

    /** Sets the control mode to CONTROL_POSITION and sets
	the goal pose */
    void GoTo( double x, double y, double a );
    void GoTo( Pose pose );

    /** Sets the control mode to CONTROL_ACCELERATION and sets the
	current accelerations to x, y (meters per second squared) and
	a (radians per second squared) */
    void SetAcceleration( double x, double y, double a );

    // localization state
    Pose est_pose; ///< position estimate in local coordinates
    Pose est_pose_error; ///< estimated error in position estimate
    Pose est_origin; ///< global origin of the local coordinate system

  protected:
    virtual void Move();
    virtual void Startup();
    virtual void Shutdown();
    virtual void Update();
    virtual void Load();
  };


  // ACTUATOR MODEL --------------------------------------------------------

  /// %ModelActuator class
  class ModelActuator : public Model
  {
  public:
    /** Define a actuator control method */
    typedef enum
      { CONTROL_VELOCITY,
	CONTROL_POSITION
      } ControlMode;

    /** Define an actuator type */
    typedef enum
      { TYPE_LINEAR,
	TYPE_ROTATIONAL
      } ActuatorType;

  private:
    double goal; //< the current velocity or pose to reach depending on the value of control_mode
    double pos;
    double max_speed;
    double min_position;
    double max_position;
    double start_position;
    double cosa;
    double sina;
    ControlMode control_mode;
    ActuatorType actuator_type;
    point3_t axis;

    Pose InitialPose;
  public:
    // constructor
    ModelActuator( World* world,
		   Model* parent,
		   const std::string& type );
    // destructor
    ~ModelActuator();

    virtual void Startup();
    virtual void Shutdown();
    virtual void Update();
    virtual void Load();

    /** Sets the control_mode to CONTROL_VELOCITY and sets
	the goal velocity. */
    void SetSpeed( double speed );

    double GetSpeed() const {return goal;}

    /** Sets the control mode to CONTROL_POSITION and sets
	the goal pose */
    void GoTo( double pose );

    double GetPosition() const {return pos;};
    double GetMaxPosition() const {return max_position;};
    double GetMinPosition() const {return min_position;};

    ActuatorType GetType() const { return actuator_type; }
    point3_t GetAxis() const { return axis; }
  };

  // WIFI MODEL --------------------------------------------------------
  // Forward declare the wifi model so the comm system can make use of it.
  class ModelWifi;
  /**
   * A base class for messages being passed between robots via Wifi.
   *
   */
  class WifiMessageBase
  {
    friend class ModelWifi;

    protected:
      std::string sender_mac;
      in_addr_t sender_ip_in;
      in_addr_t sender_netmask_in;
      uint32_t sender_id;

      in_addr_t recipient_ip_in;
      in_addr_t intended_recipient_ip_in;
      in_addr_t recipient_netmask_in;
      uint32_t  recipient_id;

      std::string message;

    public:
      WifiMessageBase(){};
      WifiMessageBase(const WifiMessageBase& toCopy);           ///< Copy constructor
      WifiMessageBase& operator=(const WifiMessageBase& toCopy);///< Equals operator overload
      virtual ~WifiMessageBase(){};

      /**
       * Clone a copy of the wifi message.  Used for making a deep copy of a wifi message and its subclasses
       * when handling a pointer to the wifi message base class.
       */
      virtual WifiMessageBase* Clone() { return new WifiMessageBase(*this); };

      inline void SetMessage(const std::string & value) { message = value; };
      inline const std::string & GetMessage() { return message; };

      inline void SetSenderMac(const std::string & value) { sender_mac = value; };
      inline const std::string & GetSenderMac() { return sender_mac; };

      inline void SetSenderIp(in_addr_t value) { sender_ip_in = value; };
      inline in_addr_t GetSenderIp() { return sender_ip_in; };

      inline void SetSenderNetmask(in_addr_t value) { sender_netmask_in = value; };
      inline in_addr_t GetSenderNetmask() { return sender_netmask_in; };

      inline void SetSenderId(uint32_t value) { sender_id = value; };
      inline uint32_t GetSenderId() { return sender_id; };

      void SetBroadcastIp();
      inline bool IsBroadcastIp() { return (recipient_ip_in & ~recipient_netmask_in) == ~recipient_netmask_in; };
      inline void SetRecipientIp(in_addr_t value) { recipient_ip_in = value; };
      inline in_addr_t GetRecipientIp() {return recipient_ip_in; };
      inline void SetIntendedRecipientIp(in_addr_t value) { intended_recipient_ip_in = value; };
      inline in_addr_t GetIntendedRecipientIp() {return intended_recipient_ip_in; };

      inline void SetRecipientNetmask(in_addr_t value) { recipient_netmask_in = value; };
      inline in_addr_t GetRecipientNetmask() { return recipient_netmask_in; };

      inline void SetRecipientId(uint32_t value) { recipient_id = value; };
      inline uint32_t GetRecipientId() {return recipient_id; };

  };//end class WifiMessageBase

 /**
  * Interface for inter-robot communication
  */
  typedef void (*stg_comm_rx_msg_fn_t)(WifiMessageBase * msg );

  /**
   * Communication class implements call-back based message passing for the Wifi model.
   */
  class Communication
  {
    private:
    stg_comm_rx_msg_fn_t ReceiveMsgFn;

    void *arg;

    ModelWifi * wifi_p; ///< Pointer to the Wifi Model that this comm class is associated with

    void SetMessageSender(WifiMessageBase * to_send); /// <<Set the sender props based on the fact we're sending it.
    void SendMessage(WifiMessageBase * to_send); ///< Send wifi message to message addressee, taking into account robots in range.

    public:
    Communication( ModelWifi * parent_p );
    ~Communication();

    void SendUnicastMessage(WifiMessageBase * to_send); ///< Send wifi message to specifc recipient if they're in range.
    void SendBroadcastMessage(WifiMessageBase * to_send); ///< Send wifi message to all robots in range.

    bool IsReceiveMsgFnSet() { return this->ReceiveMsgFn; }; ///< Determine if receive message is set for robot.
    void CallReceiveMsgFn(WifiMessageBase * msg) { this->ReceiveMsgFn( msg ); } ///< Call message receive Fn.
    void SetReceiveMsgFn( stg_comm_rx_msg_fn_t rxfn) { this->ReceiveMsgFn = rxfn; } ///< Set the receive message fn.

  };//end class Communication


  /**
   * Enumerated type defining the different possible wifi models we can use.
   */
  enum WifiModelTypeEnum { FRIIS, ITU_INDOOR, LOG_DISTANCE, RAYTRACE, SIMPLE };

  /**
   * This class encapsulates the various configuration parameters of a robot using the simulated WIFI.
   */
  class WifiConfig
  {
    protected:
      static const std::string MODEL_NAME_FRIIS;        ///< String constant used when parsing world file.
      static const std::string MODEL_NAME_ITU_INDOOR;   ///< String constant used when parsing world file.
      static const std::string MODEL_NAME_LOG_DISTANCE; ///< String constant used when parsing world file.
      static const std::string MODEL_NAME_RAYTRACE;     ///< String constant used when parsing world file.
      static const std::string MODEL_NAME_SIMPLE;       ///< String constant used when parsing world file.

      std::string essid;    ///< Wifi network ESSID
      std::string mac;      ///< MAC of the current WIFI interface
      in_addr_t ip_in;      ///< IP address of the wifi interface
      in_addr_t netmask_in; ///< Netmask of the inet address.
      double power;         ///< Wifi transmission power
      double sensitivity;   ///< Wifi transmission sensitivity
      double freq;          ///< Wifi frequency
      meters_t range;   ///< Range
      WifiModelTypeEnum model; ///< Model type
      double plc;           ///< distance power loss coefficient
      double ple;           ///< path loss distance exponent
      double sigma;         ///< standard derivation of gaussian random variable
      double range_db;      ///< setting the individual ranges to render in render_cfg
      double wall_factor;   ///< Wall factor for use in raytracing
      double power_dbm;     ///< Power loss in db/m

      /**
       * The link success rate detemines the percent of time that
       * links between two robots will be succesfully established.
       * It can be used to simulate conditions where WIFI operation
       * is erratic and prone to random failures.
       * 100 = completely reliable linkups
       * 0 = completely unreliable linkups
       */
      int link_success_rate;
    public:
      WifiConfig();

      // All the setters
      inline void SetEssid(const std::string & value) { essid = value; };
      inline void SetMac(const std::string & value) {mac = value; };
      inline void SetIp(const in_addr_t value) {ip_in = value; };
      void        SetIp(const std::string & value) { ip_in = inet_addr(value.c_str()); };
      inline void SetNetmask(const in_addr_t value) {netmask_in = value;};
      void        SetNetmask(const std::string & value) { netmask_in = inet_addr(value.c_str()); };
      inline void SetPower(const double value) {power = value; };
      inline void SetSensitivity(const double value) {sensitivity = value; };
      inline void SetFreq(const double value) {freq = value; };
      inline void SetRange(const meters_t value) { range = value; };
      inline void SetModel(WifiModelTypeEnum value) {model = value; };
      void        SetModel(const std::string & value);
      inline void SetPlc(const double value) {plc = value; };
      inline void SetPle(const double value) {ple = value; };
      inline void SetSigma(const double value) {sigma=value;};
      inline void SetRangeDb(const double value) {range_db=value;};
      inline void SetWallFactor(const double value) {wall_factor=value;};
      inline void SetPowerDbm(const double value) {power_dbm = value;};
      inline void SetLinkSuccessRate(const int value) { link_success_rate = value;};

      //All the getters
      inline std::string & GetEssid() { return essid; };
      inline std::string & GetMac() { return mac; };
      inline in_addr_t GetIp() { return ip_in; };
      std::string  GetIpString() { std::string addr_str( inet_ntoa(*(struct in_addr *)&ip_in)); return addr_str; };
      inline in_addr_t GetNetmask() { return netmask_in; };
      std::string  GetNetmaskString() { std::string addr_str( inet_ntoa(*(struct in_addr *)&netmask_in)); return addr_str; };
      inline double GetPower() { return power; };
      inline double GetSensitivity() { return sensitivity; };
      inline double GetFreq() { return freq; };
      inline meters_t GetRange() { return range; };
      inline WifiModelTypeEnum GetModel() { return model; };
      const std::string & GetModelString();
      inline double GetPlc() { return plc; };
      inline double GetPle() { return ple; };
      inline double GetSigma() { return sigma; };
      inline double GetRangeDb() { return range_db; };
      inline double GetWallFactor() { return wall_factor;};
      inline double GetPowerDbm() { return power_dbm;};
      inline int GetLinkSuccessRate() { return link_success_rate;};
  };//end WifiConfig

  /**
   * Information about a robot that is the wifi neighbor of the current robot.
   */
  class WifiNeighbor
  {
    protected:
      uint32_t id;            ///< id of neighbor
      Pose pose;              ///< global pose of corresponding neighbour.
      std::string essid;      ///< essid of the neighbor
      std::string mac;        ///< mac of the neighbor
      in_addr_t ip_in;        ///< ip address of the neighbor
      in_addr_t netmask_in;   ///< netmask of the neighbor
      double freq;            ///< Neighbor's frequency
      double db;              ///< Neighbor's signal-strength

      Communication * comm;   ///< Neighbor's comm interface
    public:
      WifiNeighbor() { comm = NULL; };
      ~WifiNeighbor() { comm = NULL; essid.clear(); mac.clear();};

      //All the setters
      inline void SetId(const uint32_t value) { id = value; };
      inline void SetPose(const Pose & value) { pose = value; };
      inline void SetEssid(const std::string & value) { essid = value; };
      inline void SetMac(const std::string & value) { mac = value; };
      inline void SetIp(const in_addr_t value) { ip_in = value; };
      inline void SetNetmask(const in_addr_t value) { netmask_in = value; };
      inline void SetFreq(const double value) { freq = value; };
      inline void SetDb(const double value) {db = value; };
      inline void SetCommunication(Communication * value) { comm = value; };

      //All the geters
      inline uint32_t GetId() { return id; };
      inline Pose & GetPose() { return pose; };
      inline std::string & GetEssid() { return essid; };
      inline std::string & GetMac() { return mac; };
      inline in_addr_t GetIp() { return ip_in; };
      inline in_addr_t GetNetmask() { return netmask_in; };
      inline double GetFreq() { return freq; };
      inline double GetDb() { return db; };
      inline Communication * GetCommunication() { return comm; };
  };

  /**
   * Typedef defining a vector of neighbors
   */
  typedef std::vector<WifiNeighbor> WifiNeighborVec;

  /** Wifi link information.
  */
  typedef struct
  {
    WifiNeighborVec neighbors;   ///< Information about each radio neighbor to the current wifi.
  } stg_wifi_data_t;

  /**
   * The Wifi Model class.
   */
  class ModelWifi : public Model
  {
    friend class Canvas;
    friend class Communication;

    private:
    WifiConfig      wifi_config;   /* Wifi configuration parameters for the current WIFI interace. */
    stg_wifi_data_t link_data;     /* Link information for each neighbor */

    void CompareModels( Model * value, Model * user );
    bool DoesLinkSucceed();
    std::string GetUniqueMac( World* world );
    static Option showWifi;

    unsigned int random_seed; ///< seed for the random number generator

#ifdef HAVE_BOOST_RANDOM
    // Note: Although boost is not strictly required for random number generation, the quality of the random
    // numbers available using the boost library is generally going to be higher.

    // We will use a random number generator unique to EACH
    BaseBoostGenerator random_generator; ///< Boost random number generator

    // Define a uniform random number distribution of integer values between
    // 1 and 6 inclusive.
    typedef boost::uniform_int<> uniform_distribution_type;
    typedef boost::variate_generator<BaseBoostGenerator&, uniform_distribution_type> BoostRandomIntGenerator;

    // Define a normal distribution of doubles for the log distance function.
    typedef boost::normal_distribution<> normal_distribution_type;
    typedef boost::variate_generator<BaseBoostGenerator&, normal_distribution_type> BoostNormalGenerator;

    //Define the communication and normal distribution random number generators.
    BoostRandomIntGenerator * comm_reliability_gen;
    BoostNormalGenerator * normal_gen;
#endif

    public:
    Communication comm;           ///< Inter-robot communication interface.

    // constructor
    ModelWifi( World* world, Model* parent, const std::string& type );
    // destructor
    ~ModelWifi();

    virtual void Startup();
    virtual void Shutdown();
    virtual void Update();
    virtual void Load();
    virtual void DataVisualize( Camera* cam );

    /** returns the quality of link to each neighbor */
    stg_wifi_data_t* GetLinkInfo( );

    /** Append link information to the light of neighbors */
    void AppendLinkInfo( ModelWifi* mod, double db );

    // Get the user-tweakable configuration of the wifi model
    WifiConfig * GetConfig( ) { return &wifi_config; };

    Communication * GetCommunication( ) { return &comm; };

  };//end class ModelWifi


}; // end namespace stg

#endif
