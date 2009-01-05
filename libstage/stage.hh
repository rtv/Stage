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

/** \defgroup libstage libstage: The Stage Robot Simulation Library
 *  Description of libstage for developers
 */

#include <unistd.h>
#include <stdint.h> // for portable int types eg. uint32_t
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <iostream>
#include <vector>

// we use GLib's data structures extensively. Perhaps we'll move to
// C++ STL types to lose this dependency one day.
#include <glib.h> 

// FLTK Gui includes
#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Gl_Window.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Window.H>
#include <FL/fl_draw.H>
#include <FL/gl.h> // FLTK takes care of platform-specific GL stuff
// except GLU for some reason
#ifdef __APPLE__
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#else
#include <GL/glu.h>
#include <GL/glut.h>
#endif 

#include "option.hh"
#include "file_manager.hh"

/** The Stage library uses its own namespace */
namespace Stg 
{
  // forward declare
  class Canvas;
  class Cell;
  class Worldfile;
  class World;
  class WorldGui;
  class Model;
  class OptionsDlg;
  class Camera;
  
  /** Initialize the Stage library */
  void Init( int* argc, char** argv[] );

  /** returns true iff Stg::Init() has been called. */
  bool InitDone();
  
  /** Create unique identifying numbers for each type of model, and a
      count of the number of types. */
  typedef enum {
    MODEL_TYPE_PLAIN = 0,
    MODEL_TYPE_LASER,
    MODEL_TYPE_FIDUCIAL,
    MODEL_TYPE_RANGER,
    MODEL_TYPE_POSITION,
    MODEL_TYPE_BLOBFINDER,
    MODEL_TYPE_BLINKENLIGHT,
    MODEL_TYPE_CAMERA,
    MODEL_TYPE_COUNT // must be the last entry, to count the number of
    // types
  } stg_model_type_t;


  /// Copyright string
  const char COPYRIGHT[] =				       
    "Copyright Richard Vaughan and contributors 2000-2008";

  /// Author string
  const char AUTHORS[] =					
    "Richard Vaughan, Brian Gerkey, Andrew Howard, Reed Hedges, Pooya Karimian, Toby Collett, Jeremy Asher, Alex Couture-Beil and contributors.";

  /// Project website string
  const char WEBSITE[] = "http://playerstage.org";

  /// Project description string
  const char DESCRIPTION[] =				       
    "Robot simulation library\nPart of the Player Project";

  /// Project distribution license string
  const char LICENSE[] = 
    "Stage robot simulation library\n"					\
    "Copyright (C) 2000-2008 Richard Vaughan and contributors\n"	\
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
  
  /** The maximum length of a Stage model identifier string */
  const uint32_t TOKEN_MAX = 64;

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
  double normalize( double a );

  /** take binary sign of a, either -1, or 1 if >= 0 */
  inline int sgn( int a){ return( a<0 ? -1 : 1); }

  /** take binary sign of a, either -1, or 1 if >= 0. */
  inline double sgn( double a){ return( a<0 ? -1.0 : 1.0); }


  /** Describe the image format used for saving screenshots. */
  typedef enum {
    STG_IMAGE_FORMAT_PNG,
    STG_IMAGE_FORMAT_JPG
  } stg_image_format_t;

  /** any integer value other than this is a valid fiducial ID */
  enum { FiducialNone = 0 };

  /** Value that uniquely identifies a model */
  typedef uint32_t stg_id_t;

  /** Metres: floating point unit of distance */
  typedef double stg_meters_t;

  /** Radians: unit of angle */
  typedef double stg_radians_t;

  /** time structure */
  typedef struct timeval stg_time_t;

  /** Milliseconds: unit of (short) time */
  typedef unsigned long stg_msec_t;

  /** Microseconds: unit of (very short) time */
  typedef uint64_t stg_usec_t;

  /** Kilograms: unit of mass */
  typedef double stg_kg_t; // Kilograms (mass)

  /** Joules: unit of energy */
  typedef double stg_joules_t;

  /** Watts: unit of power (energy/time) */
  typedef double stg_watts_t;

  /** boolean */
  typedef uint32_t stg_bool_t;

  /** 32-bit ARGB color packed 0xAARRGGBB */
  typedef uint32_t stg_color_t;

  /** Specify a color from its components */
  stg_color_t stg_color_pack( double r, double g, double b, double a );

  /** Obtain the components of a color */
  void stg_color_unpack( stg_color_t col, 
			 double* r, double* g, double* b, double* a );

  /** specify a rectangular size */
  class Size
  {
  public:
    stg_meters_t x, y, z;

    Size( stg_meters_t x, 
	  stg_meters_t y, 
	  stg_meters_t z )
      : x(x), y(y), z(z)
    {/*empty*/}

    /** default constructor uses default non-zero values */
    Size() : x( 0.1 ), y( 0.1 ), z( 0.1 )
    {/*empty*/}	
  };
  
  /** Specify a 3 axis position, in x, y and heading. */
  class Pose
  {
  public:
    stg_meters_t x, y, z; //< location in 3 axes
    stg_radians_t a; //< rotation about the z axis. 
    
    Pose( stg_meters_t x, 
	  stg_meters_t y, 
	  stg_meters_t z,
	  stg_radians_t a ) 
      : x(x), y(y), z(z), a(a)
    { /*empty*/ }
    
    Pose() : x(0.0), y(0.0), z(0.0), a(0.0)
    { /*empty*/ }		 
    
    virtual ~Pose(){};
    
    /** return a random pose within the bounding rectangle, with z=0 and
	angle random */
    static Pose Random( stg_meters_t xmin, stg_meters_t xmax, 
			stg_meters_t ymin, stg_meters_t ymax )
    {		 
      return Pose( xmin + drand48() * (xmax-xmin),
		   ymin + drand48() * (ymax-ymin),
		   0, 
		   normalize( drand48() * (2.0 * M_PI) ));
    }
    
    virtual void Print( const char* prefix )
    {
      printf( "%s pose [x:%.3f y:%.3f z:%.3f a:%.3f]\n",
	      prefix, x,y,z,a );
    }
  };
  
  
  /** specify a 3 axis velocity in x, y and heading. */
  class Velocity : public Pose
  {
  public:
    Velocity( stg_meters_t x, 
	      stg_meters_t y, 
	      stg_meters_t z,
	      stg_radians_t a ) 
    { /*empty*/ }
    
    Velocity()
    { /*empty*/ }		 
    
    virtual void Print( const char* prefix )
    {
      printf( "%s velocity [x:%.3f y:%.3f z:%3.f a:%.3f]\n",
	      prefix, x,y,z,a );
    }
  };
  
  /** specify an object's basic geometry: position and rectangular
      size.  */
  class Geom
  {
  public:
    Pose pose; //< position
    Size size; //< extent
    
    void Print( const char* prefix )
    {
      printf( "%s geom pose: (%.2f,%.2f,%.2f) size: [%.2f,%.2f]\n",
	      prefix,
	      pose.x,
	      pose.y,
	      pose.a,
	      size.x,
	      size.y );
    }
  };
  
  class Waypoint
  {
  public:
    Waypoint( stg_meters_t x, stg_meters_t y, stg_meters_t z, stg_radians_t a, stg_color_t color ) ;
    Waypoint();
    void Draw();
    
    Pose pose;
    stg_color_t color;
  };
  
  /** bound a range of values, from min to max. min and max are initialized to zero. */
  class Bounds
  {
  public:
    double max; //< largest value in range
    double min; //< smallest value in range
    
    Bounds() : max(0), min(0)  
    { /* empty*/  };
  };
  
  /** bound a volume along the x,y,z axes. All bounds initialized to zero. */
  typedef struct
  {
    Bounds x; //< volume extent along x axis
    Bounds y; //< volume extent along y axis
    Bounds z; //< volume extent along z axis 
  } stg_bounds3d_t;
  
  
  /** define a three-dimensional bounding box, initialized to zero */
  typedef struct
  {
    Bounds x, y, z;
  } stg_bbox3d_t;
  
  /** define a field-of-view: an angle and range bounds */
  typedef struct
  {
    Bounds range; //< min and max range of sensor
    stg_radians_t angle; //< width of viewing angle of sensor
  } stg_fov_t;
  
  /** define a point on a 2d plane */
  typedef struct
  {
    stg_meters_t x, y;
  } stg_point_t;
  
  /** define a point in 3d space */
  typedef struct
  {
    float x, y, z;
  } stg_vertex_t;
  
  /** define vertex and its color */
  typedef struct
  {
    float x, y, z, r, g, b, a;
  } stg_colorvertex_t;
  
  /** define a point in 3d space */
  typedef struct
  {
    stg_meters_t x, y, z;
  } stg_point3_t;

  /** define an integer point on the 2d plane */
  typedef struct
  {
    int32_t x,y;
  } stg_point_int_t;

  /** Create an array of [count] points. Caller must free the returned
      pointer, preferably with stg_points_destroy().  */
  stg_point_t* stg_points_create( size_t count );

  /** frees a point array */ 
  void stg_points_destroy( stg_point_t* pts );

  /** create an array of 4 points containing the corners of a unit
      square.  */
  stg_point_t* stg_unit_square_points_create();


  typedef uint32_t stg_movemask_t;

  const uint32_t STG_MOVE_TRANS = (1 << 0); //< bitmask for stg_movemask_t
  const uint32_t STG_MOVE_ROT   = (1 << 1); //< bitmask for stg_movemask_t
  const uint32_t STG_MOVE_SCALE = (1 << 2); //< bitmask for stg_movemask_t

  const char STG_MP_PREFIX[] =             "_mp_";
  const char STG_MP_POSE[] =               "_mp_pose";
  const char STG_MP_VELOCITY[] =           "_mp_velocity";
  const char STG_MP_GEOM[] =               "_mp_geom";
  const char STG_MP_COLOR[] =              "_mp_color";
  const char STG_MP_WATTS[] =              "_mp_watts";
  const char STG_MP_FIDUCIAL_RETURN[] =    "_mp_fiducial_return";
  const char STG_MP_LASER_RETURN[] =       "_mp_laser_return";
  const char STG_MP_OBSTACLE_RETURN[] =    "_mp_obstacle_return";
  const char STG_MP_RANGER_RETURN[] =      "_mp_ranger_return";
  const char STG_MP_GRIPPER_RETURN[] =     "_mp_gripper_return";
  const char STG_MP_MASS[] =               "_mp_mass";


  /// laser return value
  typedef enum 
    {
      LaserTransparent=0, ///<not detected by laser model 
      LaserVisible, ///< detected by laser with a reflected intensity of 0 
      LaserBright  ////< detected by laser with a reflected intensity of 1 
    } stg_laser_return_t;


  namespace Draw
  {
    typedef struct {
      double x, y, z;
    } vertex_t;

    typedef struct {
      vertex_t min, max;
    } bounds3_t;

    typedef enum {
      STG_D_DRAW_POINTS,
      STG_D_DRAW_LINES,
      STG_D_DRAW_LINE_STRIP,
      STG_D_DRAW_LINE_LOOP,
      STG_D_DRAW_TRIANGLES,
      STG_D_DRAW_TRIANGLE_STRIP,
      STG_D_DRAW_TRIANGLE_FAN,
      STG_D_DRAW_QUADS,
      STG_D_DRAW_QUAD_STRIP,
      STG_D_DRAW_POLYGON,
      STG_D_PUSH,
      STG_D_POP,
      STG_D_ROTATE,
      STG_D_TRANSLATE,
    } type_t;

    /** the start of all stg_d structures looks like this */
    typedef struct
    {
      type_t type;
    } hdr_t;

    /** push is just the header, but we define it for syntax checking */
    typedef hdr_t push_t;
    /** pop is just the header, but we define it for syntax checking */
    typedef hdr_t pop_t;

    /** stg_d draw command */
    typedef struct
    {
      /** required type field */
      type_t type;
      /** number of vertices */
      size_t vert_count;
      /** array of vertex data follows at the end of this struct*/
      vertex_t verts[];
    } draw_t;

    /** stg_d translate command */
    typedef struct
    {
      /** required type field */
      type_t type;
      /** distance to translate, in 3 axes */
      vertex_t vector;
    } translate_t;

    /** stg_d rotate command */
    typedef struct
    {
      /** required type field */
      type_t type;
      /** vector about which we should rotate */
      vertex_t vector;
      /** angle to rotate */
      stg_radians_t angle;
    } rotate_t;

    /** Create a draw_t object of specified type from a vertex array */
    draw_t* create( type_t type,  
		    vertex_t* verts,
		    size_t vert_count );

    /** Delete the draw_t object, deallocting its memory */
    void destroy( draw_t* d );
  } // end namespace draw


  // MACROS ------------------------------------------------------
  // Some useful macros


  /** Look up the color in the X11 database.  (i.e. transform color
      name to color value).  If the color is not found in the
      database, a bright red color (0xF00) will be returned instead.
  */
  stg_color_t stg_lookup_color(const char *name);

  /** returns the sum of [p1] + [p2], in [p1]'s coordinate system */
  Pose pose_sum( const Pose& p1, const Pose& p2 );
  
  /** returns a new pose, with each axis scaled */
  Pose pose_scale( const Pose& p1, const double x, const double y, const double z );


  // PRETTY PRINTING -------------------------------------------------

  /** Report an error, with a standard, friendly message header */
  void stg_print_err( const char* err );
  /** Print human-readable geometry on stdout */
  void stg_print_geom( Geom* geom );
  /** Print human-readable pose on stdout */
  void stg_print_pose( Pose* pose );
  /** Print human-readable velocity on stdout */
  void stg_print_velocity( Velocity* vel );

  /** A model creator function. Each model type must define a function of this type. */
  typedef Model* (*stg_creator_t)( World*, Model* );
  
  typedef struct 
  {
    const char* token;
    stg_creator_t creator;
  } stg_typetable_entry_t;
  
  /** a global (to the namespace) table mapping names to model types */
  extern stg_typetable_entry_t typetable[MODEL_TYPE_COUNT];

  void RegisterModel( stg_model_type_t type, 
		      const char* name, 
		      stg_creator_t creator );

  void RegisterModels();

  void gl_draw_grid( stg_bounds3d_t vol );
  void gl_pose_shift( const Pose &pose );
  void gl_pose_inverse_shift( const Pose &pose );
  void gl_coord_shift( double x, double y, double z, double a  );

  class Flag
  {
  public:
    stg_color_t color;
    double size;

    Flag( stg_color_t color, double size );
    Flag* Nibble( double portion );
  };

  /** Render a string at [x,y,z] in the current color */
  void gl_draw_string( float x, float y, float z, const char *string);
  void gl_speech_bubble( float x, float y, float z, const char* str );
  void gl_draw_octagon( float w, float h, float m );
  void gl_draw_vector( double x, double y, double z );
  void gl_draw_origin( double len );

  typedef int(*stg_model_callback_t)(Model* mod, void* user );
  typedef int(*stg_cell_callback_t)(Cell* cell, void* user );
  
  // return val, or minval if val < minval, or maxval if val > maxval
  double constrain( double val, double minval, double maxval );
    
  typedef struct 
  {
    int enabled;
    Pose pose;
    stg_meters_t size; ///< rendered as a sphere with this diameter
    stg_color_t color;
    stg_msec_t period; ///< duration of a complete cycle
    double duty_cycle; ///< mark/space ratio
  } stg_blinkenlight_t;

  typedef struct {
    Pose pose;
    stg_color_t color;
    stg_usec_t time;
  } stg_trail_item_t;
  
  /** container for a callback function and a single argument, so
      they can be stored together in a list with a single pointer. */
  typedef struct
  {
    stg_model_callback_t callback;
    void* arg;
  } stg_cb_t;

  stg_cb_t* cb_create( stg_model_callback_t callback, void* arg );
  void cb_destroy( stg_cb_t* cb );

  /** defines a rectangle of [size] located at [pose] */
  typedef struct
  {
    Pose pose;
    Size size;
  } stg_rotrect_t; // rotated rectangle

  /** normalizes the set [rects] of [num] rectangles, so that they fit
      exactly in a unit square.
  */
  void stg_rotrects_normalize( stg_rotrect_t* rects, int num );

  /** load the image file [filename] and convert it to an array of
      rectangles, filling in the number of rects, width and
      height. Memory is allocated for the rectangle array [rects], so
      the caller must free [rects].
  */
  int stg_rotrects_from_image_file( const char* filename, 
				    stg_rotrect_t** rects,
				    unsigned int* rect_count,
				    unsigned int* widthp, 
				    unsigned int* heightp );

  
  /** matching function should return true iff the candidate block is
      stops the ray, false if the block transmits the ray */
  typedef bool (*stg_ray_test_func_t)(Model* candidate, 
				      Model* finder, 
				      const void* arg );
  
  // TODO - some of this needs to be implemented, the rest junked.

  /*   //  -------------------------------------------------------------- */

  /*   // standard energy consumption of some devices in W. */
  /*   // */
  /*   // The MOTIONKG value is a hack to approximate cost of motion, as */
  /*   // Stage does not yet have an acceleration model. */
  /*   // */
  /* #define STG_ENERGY_COST_LASER 20.0 // 20 Watts! (LMS200 - from SICK web site) */
  /* #define STG_ENERGY_COST_FIDUCIAL 10.0 // 10 Watts */
  /* #define STG_ENERGY_COST_RANGER 0.5 // 500mW (estimate) */
  /* #define STG_ENERGY_COST_MOTIONKG 10.0 // 10 Watts per KG when moving  */
  /* #define STG_ENERGY_COST_BLOB 4.0 // 4W (estimate) */

  /*   typedef struct */
  /*   { */
  /*     stg_joules_t joules; // current energy stored in Joules/1000 */
  /*     stg_watts_t watts; // current power expenditure in mW (mJoules/sec) */
  /*     int charging; // 1 if we are receiving energy, -1 if we are */
  /*     // supplying energy, 0 if we are neither charging nor */
  /*     // supplying energy. */
  /*     stg_meters_t range; // the range that our charging probe hit a charger */
  /*   } stg_energy_data_t; */

  /*   typedef struct */
  /*   { */
  /*     stg_joules_t capacity; // maximum energy we can store (we start fully charged) */
  /*     stg_meters_t probe_range; // the length of our recharge probe */
  /*     //Pose probe_pose; // TODO - the origin of our probe */

  /*     stg_watts_t give_rate; // give this many Watts to a probe that hits me (possibly 0) */

  /*     stg_watts_t trickle_rate; // this much energy is consumed or */
  /*     // received by this device per second as a */
  /*     // baseline trickle. Positive values mean */
  /*     // that the device is just burning energy */
  /*     // stayting alive, which is appropriate */
  /*     // for most devices. Negative values mean */
  /*     // that the device is receiving energy */
  /*     // from the environment, simulating a */
  /*     // solar cell or some other ambient energy */
  /*     // collector */

  /*   } stg_energy_config_t; */


  /*   // BLINKENLIGHT ------------------------------------------------------------ */

  /*   // a number of milliseconds, used for example as the blinkenlight interval */
  /* #define STG_LIGHT_ON UINT_MAX */
  /* #define STG_LIGHT_OFF 0 */

  //typedef int stg_interval_ms_t;

  // list iterator macros
#define LISTFUNCTION( LIST, TYPE, FUNC ) for( GList* it=LIST; it; it=it->next ) FUNC((TYPE)it->data);
#define LISTMETHOD( LIST, TYPE, METHOD ) for( GList* it=LIST; it; it=it->next ) ((TYPE)it->data)->METHOD();
#define LISTFUNCTIONARG( LIST, TYPE, FUNC, ARG ) for( GList* it=LIST; it; it=it->next ) FUNC((TYPE)it->data, ARG);
#define LISTMETHODARG( LIST, TYPE, METHOD, ARG ) for( GList* it=LIST; it; it=it->next ) ((TYPE)it->data)->METHOD(ARG);

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

  /** Define a callback function type that can be attached to a
      record within a model and called whenever the record is set.*/
  typedef int (*stg_model_callback_t)( Model* mod, void* user );


  // ANCESTOR CLASS
  /** Base class for Model and World */
  class Ancestor
  {
    friend class Canvas; // allow Canvas access to our private members
	 
  protected:
    GList* children;
    char* token;
    bool debug;
	 
  public:
	 
    /** recursively call func( model, arg ) for each descendant */
    void ForEachDescendant( stg_model_callback_t func, void* arg );
	 
    /** array contains the number of each type of child model */
    unsigned int child_type_counts[MODEL_TYPE_COUNT];
	 
    Ancestor();
    virtual ~Ancestor();
	 
    virtual void AddChild( Model* mod );
    virtual void RemoveChild( Model* mod );
    virtual Pose GetGlobalPose();
	 
    const char* Token()
    { return token; }
	 
    void SetToken( const char* str )
    { token = strdup( str ); } // teeny memory leak
  };

  /** raytrace sample
   */
  typedef struct
  {
    Pose pose; ///< location and direction of the ray origin
    stg_meters_t range; ///< range to beam hit in meters
    Model* mod; ///< the model struck by this beam
    stg_color_t color; ///< the color struck by this beam
  } stg_raytrace_result_t;


  const uint32_t INTERVAL_LOG_LEN = 32;

  // defined in stage_internal.hh
  class Region;
  class SuperRegion;
  class BlockGroup;

  /// %World class
  class World : public Ancestor
  {
    friend class Model; // allow access to private members
    friend class Block;
    //friend class StgTime;
    friend class Canvas;
  
  private:
  
    static GList* world_list; ///< all the worlds that exist
    static bool quit_all; ///< quit all worlds ASAP  
    static void UpdateCb( World* world);
    static unsigned int next_id; ///<initially zero, used to allocate unique sequential world ids
	 
    bool destroy;
    bool dirty; ///< iff true, a gui redraw would be required
    GHashTable* models_by_name; ///< the models that make up the world, indexed by name
    double ppm; ///< the resolution of the world model in pixels per meter   
    bool quit; ///< quit this world ASAP  

    /** World::quit is set true when this simulation time is reached */
    stg_usec_t quit_time;
    stg_usec_t real_time_now; ///< The current real time in microseconds
    stg_usec_t real_time_start; ///< the real time at which this world was created
    GMutex* thread_mutex;
    GThreadPool *threadpool;
    int total_subs; ///< the total number of subscriptions to all models
    unsigned int update_jobs_pending;
    GList* velocity_list; ///< Models with non-zero velocity and should have their poses updated
    unsigned int worker_threads;
    GCond* worker_threads_done;

  protected:	 
  
    stg_bounds3d_t extent; ///< Describes the 3D volume of the world
    bool graphics;///< true iff we have a GUI
    stg_usec_t interval_sim; ///< temporal resolution: milliseconds that elapse between simulated time steps 
    GList* ray_list;///< List of rays traced for debug visualization
    stg_usec_t sim_time; ///< the current sim time in this world in ms
    GHashTable* superregions;
    SuperRegion* sr_cached; ///< The last superregion looked up by this world
    GList* update_list; ///< Models that have a subscriber or controller, and need to be updated
    long unsigned int updates; ///< the number of simulated time steps executed so far
    Worldfile* wf; ///< If set, points to the worldfile used to create this world

  public:
    static const int DEFAULT_PPM = 50;  // default resolution in pixels per meter
    static const stg_msec_t DEFAULT_INTERVAL_SIM = 100;  ///< duration of sim timestep

    /** hint that the world needs to be redrawn if a GUI is attached */
    void NeedRedraw(){ dirty = true; };

    void LoadModel( Worldfile* wf, int entity, GHashTable* entitytable );
    void LoadBlock( Worldfile* wf, int entity, GHashTable* entitytable );
    void LoadBlockGroup( Worldfile* wf, int entity, GHashTable* entitytable );

    SuperRegion* AddSuperRegion( const stg_point_int_t& coord );
    SuperRegion* GetSuperRegion( const stg_point_int_t& coord );
    SuperRegion* GetSuperRegionCached( const stg_point_int_t& coord);
    void ExpireSuperRegion( SuperRegion* sr );

    inline Cell* GetCell( const int32_t x, const int32_t y );
	 
    void ForEachCellInPolygon( const stg_point_t pts[], 
			       const uint32_t pt_count,
			       stg_cell_callback_t cb,
			       void* cb_arg );
  
    void ForEachCellInLine( const stg_point_t pt1,
			    const stg_point_t pt2, 
			    stg_cell_callback_t cb,
			    void* cb_arg );
  
    void ForEachCellInLine( stg_meters_t x1, stg_meters_t y1,
			    stg_meters_t x2, stg_meters_t y2,
			    stg_cell_callback_t cb,
			    void* cb_arg );
		
    /** convert a distance in meters to a distance in world occupancy
	grid pixels */
    int32_t MetersToPixels( stg_meters_t x )
    { return (int32_t)floor(x * ppm); };
	 
    // dummy implementations to be overloaded by GUI subclasses
    virtual void PushColor( stg_color_t col ) { /* do nothing */  };
    virtual void PushColor( double r, double g, double b, double a ) { /* do nothing */  };
    virtual void PopColor(){ /* do nothing */  };
	 
    SuperRegion* CreateSuperRegion( stg_point_int_t origin );
    void DestroySuperRegion( SuperRegion* sr );
	 
    stg_raytrace_result_t Raytrace( const Pose& pose, 			 
				    const stg_meters_t range,
				    const stg_ray_test_func_t func,
				    const Model* finder,
				    const void* arg,
				    const bool ztest );
  
    void Raytrace( const Pose &pose, 			 
		   const stg_meters_t range,
		   const stg_radians_t fov,
		   const stg_ray_test_func_t func,
		   const Model* finder,
		   const void* arg,
		   stg_raytrace_result_t* samples,
		   const uint32_t sample_count,
		   const bool ztest );
  

    /** Enlarge the bounding volume to include this point */
    void Extend( stg_point3_t pt );
  
    virtual void AddModel( Model* mod );
    virtual void RemoveModel( Model* mod );
  
    GList* GetRayList(){ return ray_list; };
    void ClearRays();
  
    /** store rays traced for debugging purposes */
    void RecordRay( double x1, double y1, double x2, double y2 );

    /** Returns true iff the current time is greater than the time we
	should quit */
    bool PastQuitTime();
    
    void StartUpdatingModel( Model* mod )
    { update_list = g_list_append( update_list, mod ); }
    
    void StopUpdatingModel( Model* mod )
    { update_list = g_list_remove( update_list, mod ); }
    
    void StartUpdatingModelPose( Model* mod )
    { velocity_list = g_list_append( velocity_list, mod ); }
    
    void StopUpdatingModelPose( Model* mod )
    { velocity_list = g_list_remove( velocity_list, mod ); }
    
    static void update_thread_entry( Model* mod, World* world );
	 
  public:
    /** returns true when time to quit, false otherwise */
    static bool UpdateAll(); 
	 
    World( const char* token = "MyWorld", 
	   stg_msec_t interval_sim = DEFAULT_INTERVAL_SIM,
	   double ppm = DEFAULT_PPM );
	 
    virtual ~World();
  
    stg_usec_t SimTimeNow(void){ return sim_time; }
    stg_usec_t RealTimeNow(void);
    stg_usec_t RealTimeSinceStart(void);
	 
    stg_usec_t GetSimInterval(){ return interval_sim; };
	 
    Worldfile* GetWorldFile(){ return wf; };
	 
    inline virtual bool IsGUI() { return false; }
	 
    virtual void Load( const char* worldfile_path );
    virtual void UnLoad();
    virtual void Reload();
    virtual bool Save( const char* filename );
    virtual bool Update(void);
	 
    bool TestQuit(){ return( quit || quit_all );  }
    void Quit(){ quit = true; }
    void QuitAll(){ quit_all = true; }
    void CancelQuit(){ quit = false; }
    void CancelQuitAll(){ quit_all = false; }
	 
    /** Get the resolution in pixels-per-metre of the underlying
	discrete raytracing model */ 
    double Resolution(){ return ppm; };
   
    /** Returns a pointer to the model identified by name, or NULL if
	nonexistent */
    Model* GetModel( const char* name );
  
    /** Return the 3D bounding box of the world, in meters */
    stg_bounds3d_t GetExtent(){ return extent; };
  
    /** Return the number of times the world has been updated. */
    long unsigned int GetUpdateCount() { return updates; }
  };

  class Block
  {
    friend class BlockGroup;
    friend class Model;
    friend class SuperRegion;
    friend class World;
    friend class Canvas;
  public:
  
    /** Block Constructor. A model's body is a list of these
	blocks. The point data is copied, so pts can safely be freed
	after constructing the block.*/
    Block( Model* mod,  
	   stg_point_t* pts, 
	   size_t pt_count,
	   stg_meters_t zmin,
	   stg_meters_t zmax,
	   stg_color_t color,
	   bool inherit_color );
  
    /** A from-file  constructor */
    Block(  Model* mod,  Worldfile* wf, int entity);
	 
    ~Block();
	 
    /** render the block into the world's raytrace data structure */
    void Map(); 
	 
    /** remove the block from the world's raytracing data structure */
    void UnMap();
	 
    void Draw( Model* mod );  
    void DrawSolid(); // draw the block in OpenGL as a solid single color
    void DrawFootPrint(); // draw the projection of the block onto the z=0 plane
	 
    void RecordRendering( Cell* cell )
    { g_ptr_array_add( rendered_cells, (gpointer)cell ); };
  
    stg_point_t* Points( unsigned int *count )
    { if( count ) *count = pt_count; return pts; };	       
  
    //bool IntersectGlobalZ( stg_meters_t z )
    //{ return( z >= global_zmin &&  z <= global_zmax ); }
  
    void AddToCellArray( GPtrArray* ptrarray );
    void RemoveFromCellArray( GPtrArray* ptrarray );

    void GenerateCandidateCells();
  
    //   /** Prepare to render the block in a new position in global coordinates */
    //   void SetPoseTentative( const Pose pose );
    
    Model* TestCollision();
  
    void SwitchToTestedCells();
  
    void Load( Worldfile* wf, int entity );
  
    Model* GetModel(){ return mod; };
  
    stg_color_t GetColor();
  
  private:
    Model* mod; //< model to which this block belongs
  
    size_t pt_count; //< the number of points
    stg_point_t* pts; //< points defining a polygon
	 
    Size size;

	 
    Bounds local_z; //<  z extent in local coords

    stg_color_t color;
    bool inherit_color;
	 
    void DrawTop();
    void DrawSides();
	 
    /** z extent in global coordinates */
    Bounds global_z;
	 
    bool mapped;
	 
    /** an array of pointers to cells into which this block has been
	rendered (speeds up UnMapping) */  
    GPtrArray* rendered_cells;
  
    /** When moving a model, we test for collisions by generating, for
	each block, a list of the cells in which it would be rendered if the
	move were to be successful. If no collision occurs, the move is
	allowed - the rendered cells are cleared, the potential cells are
	written, and the pointers to the rendered and potential cells are
	switched for next time (avoiding a memory copy).*/
    GPtrArray* candidate_cells;
  };


  class BlockGroup
  {
    friend class Model;

  private:
    int displaylist;
    void BuildDisplayList( Model* mod );

    GList* blocks;
    uint32_t count;
    Size size;
    stg_point3_t offset;
    stg_meters_t minx, maxx, miny, maxy;

  public:
    BlockGroup();
    ~BlockGroup();
	 
    GList* GetBlocks(){ return blocks; };
    uint32_t GetCount(){ return count; };
    Size GetSize(){ return size; };
    stg_point3_t GetOffset(){ return offset; };

    /** establish the min and max of all the blocks, so we can scale this
	group later */
    void CalcSize();
    void AppendBlock( Block* block );
    void CallDisplayList( Model* mod );
    void Clear() ; /** deletes all blocks from the group */
	 
    /** Returns a pointer to the first model detected to be colliding
	with a block in this group, or NULL, if none are detected. */
    Model* TestCollision();
    void SwitchToTestedCells();
	 
    void Map();
    void UnMap();
	 
    void DrawSolid( const Geom &geom); // draw the block in OpenGL as a solid single color
    void DrawFootPrint( const Geom &geom); // draw the
    // projection of the
    // block onto the z=0
    // plane

    void LoadBitmap( Model* mod, const char* bitmapfile, Worldfile *wf );
    void LoadBlock( Model* mod, Worldfile* wf, int entity );
  };


  typedef int ctrlinit_t( Model* mod );
  //typedef void ctrlupdate_t( Model* mod );


  // BLOCKS

  class Camera 
  {
  protected:
    float _pitch; //left-right (about y)
    float _yaw; //up-down (about x)
    float _x, _y, _z;
	
  public:
    Camera() : _pitch( 0 ), _yaw( 0 ), _x( 0 ), _y( 0 ), _z( 0 ) { }
    virtual ~Camera() { }

    virtual void Draw( void ) const = 0;
    virtual void SetProjection( void ) const = 0;

    inline float yaw( void ) const { return _yaw; }
    inline float pitch( void ) const { return _pitch; }
	
    inline float x( void ) const { return _x; }
    inline float y( void ) const { return _y; }
    inline float z( void ) const { return _z; }
	
    virtual void reset() = 0;
    virtual void Load( Worldfile* wf, int sec ) = 0;

    //TODO data should be passed in somehow else. (at least min/max stuff)
    //virtual void SetProjection( float pixels_width, float pixels_height, float y_min, float y_max ) const = 0;
  };

  class PerspectiveCamera : public Camera
  {
  private:
    float _z_near;
    float _z_far;
    float _vert_fov;
    float _horiz_fov;
    float _aspect;

  public:
    PerspectiveCamera( void );

    virtual void Draw( void ) const;
    virtual void SetProjection( void ) const;
    //void SetProjection( float aspect ) const;
    void update( void );

    void strafe( float amount );
    void forward( float amount );
	
    inline void setPose( float x, float y, float z ) { _x = x; _y = y; _z = z; }
    inline void addPose( float x, float y, float z ) { _x += x; _y += y; _z += z; if( _z < 0.1 ) _z = 0.1; }
    void move( float x, float y, float z );
    inline void setFov( float horiz_fov, float vert_fov ) { _horiz_fov = horiz_fov; _vert_fov = vert_fov; }
    ///update vertical fov based on window aspect and current horizontal fov
    inline void setAspect( float aspect ) { 
      //std::cout << "aspect: " << aspect << " vert: " << _vert_fov << " => " << aspect * _vert_fov << std::endl;
      //_vert_fov = aspect / _horiz_fov;
      _aspect = aspect;
    }
    inline void setYaw( float yaw ) { _yaw = yaw; }
    inline float horizFov( void ) const { return _horiz_fov; }
    inline float vertFov( void ) const { return _vert_fov; }
    inline void addYaw( float yaw ) { _yaw += yaw; }
    inline void setPitch( float pitch ) { _pitch = pitch; }
    inline void addPitch( float pitch ) { _pitch += pitch; if( _pitch < 0 ) _pitch = 0; else if( _pitch > 180 ) _pitch = 180; }
	
    inline float realDistance( float z_buf_val ) const {
      //formulat found at http://www.cs.unc.edu/~hoff/techrep/openglz.html
      //Z = Zn*Zf / (Zf - z*(Zf-Zn))
      return _z_near * _z_far / ( _z_far - z_buf_val * ( _z_far - _z_near ) );
    }
    inline void scroll( float dy ) { _z += dy; }
    inline float nearClip( void ) const { return _z_near; }
    inline float farClip( void ) const { return _z_far; }
    inline void setClip( float near, float far ) { _z_far = far; _z_near = near; }
	
    inline void reset() { setPitch( 70 ); setYaw( 0 ); }
	
    void Load( Worldfile* wf, int sec );
    void Save( Worldfile* wf, int sec );
  };

  class OrthoCamera : public Camera
  {
  private:
    float _scale;
    float _pixels_width;
    float _pixels_height;
    float _y_min;
    float _y_max;
  
  public:
    OrthoCamera( void ) : _scale( 15 ) { }
    virtual void Draw() const;
    virtual void SetProjection( float pixels_width, float pixels_height, float y_min, float y_max );
    virtual void SetProjection( void ) const;
  
    void move( float x, float y );
    inline void setYaw( float yaw ) { _yaw = yaw;	}
    inline void setPitch( float pitch ) { _pitch = pitch; }
    inline void addYaw( float yaw ) { _yaw += yaw;	}
    inline void addPitch( float pitch ) {
      _pitch += pitch;
      if( _pitch > 90 )
	_pitch = 90;
      else if( _pitch < 0 )
	_pitch = 0;
    }
  
    inline void setScale( float scale ) { _scale = scale; }
    inline void setPose( float x, float y) { _x = x; _y = y; }
  
    void scale( float scale, float shift_x = 0, float h = 0, float shift_y = 0, float w = 0 );	
    inline void reset( void ) { _pitch = _yaw = 0; }
  
    inline float scale() const { return _scale; }
  
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

  private:

    Canvas* canvas;
    std::vector<Option*> drawOptions;
    FileManager fileMan; ///< Used to load and save worldfiles
    stg_usec_t interval_log[INTERVAL_LOG_LEN];
    stg_usec_t interval_real;   ///< real-time interval between updates - set this to zero for 'as fast as possible
    Fl_Menu_Bar* mbar;
    OptionsDlg* oDlg;
    bool pause_time;
    bool paused; ///< the world only updates when this is false
    stg_usec_t real_time_of_last_update;

    void UpdateOptions();
	
    // static callback functions
    static void windowCb( Fl_Widget* w, void* p );	
    static void fileLoadCb( Fl_Widget* w, void* p );
    static void fileSaveCb( Fl_Widget* w, void* p );
    static void fileSaveAsCb( Fl_Widget* w, void* p );
    static void fileExitCb( Fl_Widget* w, void* p );
    static void viewOptionsCb( Fl_Widget* w, void* p );
    static void optionsDlgCb( Fl_Widget* w, void* p );
    static void helpAboutCb( Fl_Widget* w, void* p );
	
    // GUI functions
    bool saveAsDialog();
    bool closeWindowQuery();
		
    virtual void AddModel( Model* mod );

  protected:
    virtual void PushColor( stg_color_t col );
    virtual void PushColor( double r, double g, double b, double a );
    virtual void PopColor();

    void DrawTree( bool leaves );
    void DrawFloor();
	
    Canvas* GetCanvas( void ) { return canvas; }

  public:
	
    WorldGui(int W,int H,const char*L=0);
    ~WorldGui();
	
    virtual bool Update();
	
    virtual void Load( const char* filename );
    virtual void UnLoad();
    virtual bool Save( const char* filename );
	
    inline virtual bool IsGUI() { return true; }

    void DrawBoundingBoxTree();
	
    void Start(){ paused = false; };
    void Stop(){ paused = true; };
    void TogglePause(){ paused = !paused; };
	 
    /** show the window - need to call this if you don't Load(). */
    void Show(); 

    /** Get human readable string that describes the current simulation
	time. */
    std::string ClockString( void );
	
    /** Set the minimum real time interval between world updates, in
	microeconds. */
    void SetRealTimeInterval( stg_usec_t usec )
    { interval_real = usec; }
  };


#include "model.hh"

}; // end namespace stg

#endif
