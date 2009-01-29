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

/** @brief The Stage library uses its own namespace */
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
  class FileManager;
  class Option;

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
    "Copyright Richard Vaughan and contributors 2000-2009";

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

	 void Load( Worldfile* wf, int section, const char* keyword );
	 void Save( Worldfile* wf, int section, const char* keyword );
  };
  
  /** Specify a 3 axis position, in x, y and heading. */
  class Pose
  {
  public:
    stg_meters_t x, y, z;///< location in 3 axes
    stg_radians_t a;///< rotation about the z axis. 
    
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
    
	 /** Print pose in human-readable format on stdout
		  @param prefix Character string to prepend to pose output 
	 */
    virtual void Print( const char* prefix )
    {
      printf( "%s pose [x:%.3f y:%.3f z:%.3f a:%.3f]\n",
				  prefix, x,y,z,a );
    }
	 
	 void Load( Worldfile* wf, int section, const char* keyword );
	 void Save( Worldfile* wf, int section, const char* keyword );
  };
  
  
  /** Specify a 3 axis velocity in x, y and heading. 	*/
  class Velocity : public Pose
  {
  public:
	 /** @param x x position in meters 
		  @param y y position in meters 
		  @param z z position in meters 
		  @param a heading in radians 
	 */
    Velocity( stg_meters_t x, 
				  stg_meters_t y, 
				  stg_meters_t z,
				  stg_radians_t a ) 
    { /*empty*/ }
    
    Velocity()
    { /*empty*/ }		 
    
	 /** Print velocity in human-readable format on stdout
		  @param prefix Character string to prepend to pose output 
	 */
    virtual void Print( const char* prefix )
    {
      printf( "%s velocity [x:%.3f y:%.3f z:%3.f a:%.3f]\n",
				  prefix, x,y,z,a );
    }
  };
  
  /** Specify an object's basic geometry: position and rectangular
      size.  */
  class Geom
  {
  public:
    Pose pose;///< position
    Size size;///< extent
    
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
  
  /** Specify a point in space. Arrays of Waypoints can be attached to
		Models and visualized. */
  class Waypoint
  {
  public:
    Waypoint( stg_meters_t x, stg_meters_t y, stg_meters_t z, stg_radians_t a, stg_color_t color ) ;
    Waypoint();
    void Draw();
    
    Pose pose;
    stg_color_t color;
  };
  
  /** Bound a range of values, from min to max. min and max are initialized to zero. */
  class Bounds
  {
  public:
	 /// largest value in range, initially zero
    double max; 
	 /// smallest value in range, initially zero
    double min;
    
    Bounds() : max(0), min(0)  
    { /* empty*/  };
  };
  
  /** Bound a volume along the x,y,z axes. All bounds initialized to zero. */
  typedef struct
  {
	 /// volume extent along x axis, intially zero
    Bounds x; 
	 /// volume extent along y axis, initially zero
    Bounds y; 
	 /// volume extent along z axis, initially zero
    Bounds z; 
  } stg_bounds3d_t;
  
  
  /** Define a three-dimensional bounding box, initialized to zero */
  typedef struct
  {
    Bounds x, y, z;
  } stg_bbox3d_t;
  
  /** define a field-of-view: an angle and range bounds */
  typedef struct
  {
    Bounds range; ///< min and max range of sensor
    stg_radians_t angle; ///< width of viewing angle of sensor
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

  const uint32_t STG_MOVE_TRANS = (1 << 0); ///< bitmask for stg_movemask_t
  const uint32_t STG_MOVE_ROT   = (1 << 1); ///< bitmask for stg_movemask_t
  const uint32_t STG_MOVE_SCALE = (1 << 2); ///< bitmask for stg_movemask_t

  const char MP_PREFIX[] =             "_mp_";
  const char MP_POSE[] =               "_mp_pose";
  const char MP_VELOCITY[] =           "_mp_velocity";
  const char MP_GEOM[] =               "_mp_geom";
  const char MP_COLOR[] =              "_mp_color";
  const char MP_WATTS[] =              "_mp_watts";
  const char MP_FIDUCIAL_RETURN[] =    "_mp_fiducial_return";
  const char MP_LASER_RETURN[] =       "_mp_laser_return";
  const char MP_OBSTACLE_RETURN[] =    "_mp_obstacle_return";
  const char MP_RANGER_RETURN[] =      "_mp_ranger_return";
  const char MP_GRIPPER_RETURN[] =     "_mp_gripper_return";
  const char MP_MASS[] =               "_mp_mass";


  /// laser return value
  typedef enum 
    {
      LaserTransparent=0, ///<not detected by laser model 
      LaserVisible, ///< detected by laser with a reflected intensity of 0 
      LaserBright  ///< detected by laser with a reflected intensity of 1 
    } stg_laser_return_t;

  
  /** Convenient OpenGL drawing routines, used by visualization
		code. */
  namespace Gl
  {
	 void pose_shift( const Pose &pose );
	 void pose_inverse_shift( const Pose &pose );
	 void coord_shift( double x, double y, double z, double a  );
	 void draw_grid( stg_bounds3d_t vol );
	 /** Render a string at [x,y,z] in the current color */
	 void draw_string( float x, float y, float z, const char *string);
	 void draw_speech_bubble( float x, float y, float z, const char* str );
	 void draw_octagon( float w, float h, float m );
	 void draw_vector( double x, double y, double z );
	 void draw_origin( double len );
  }


  /** @brief Interface to the internal drawing system. */
  namespace Draw
  {
    typedef struct {
      double x, y, z;
    } vertex_t;

    typedef struct {
      vertex_t min, max;
    } bounds3_t;

    typedef enum {
      DRAW_POINTS,
      DRAW_LINES,
      DRAW_LINE_STRIP,
      DRAW_LINE_LOOP,
      DRAW_TRIANGLES,
      DRAW_TRIANGLE_STRIP,
      DRAW_TRIANGLE_FAN,
      DRAW_QUADS,
      DRAW_QUAD_STRIP,
      DRAW_POLYGON,
      PUSH,
      POP,
      ROTATE,
      TRANSLATE,
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


  class Flag
  {
  public:
    stg_color_t color;
    double size;

    Flag( stg_color_t color, double size );
    Flag* Nibble( double portion );
  };


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
  
  class Puck
  {
  private:
	 void BuildDisplayList();

  public:
	 stg_color_t color;
	 int displaylist;
	 stg_meters_t height;
	 Pose pose;
	 stg_meters_t radius;
	 
	 Puck();
	 void Load( Worldfile* wf, int section );
	 void Save( Worldfile* wf, int section );
	 
	 void Draw();  
  };
  
  // ANCESTOR CLASS
  /** Base class for Model and World */
  class Ancestor
  {
    friend class Canvas; // allow Canvas access to our private members
	 
  protected:
    GList* children;
    bool debug;
	 GList* puck_list;
    char* token;
	 
	 void Load( Worldfile* wf, int section );
	 void Save( Worldfile* wf, int section );
	 
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
    { token = strdup( str ); } // little memory leak	 
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
  class PowerPack;

  /// %Charger class
  class Charger
  {
	 World* world;
	 stg_watts_t watts;
	 stg_bounds3d_t volume;
	 
  public:
	 Charger( World* world );
	 void ChargeIfContained( PowerPack* pp, Pose pose );
	 bool Contains( Pose pose );
	 void Charge( PowerPack* pp );
	 void Visualize();
	 void Load( Worldfile* wf, int entity );
  };

  /// %World class
  class World : public Ancestor
  {
    friend class Model; // allow access to private members
    friend class Block;
    //friend class StgTime;
    friend class Canvas;
	 friend class Charger;

  private:
  
    static GList* world_list; ///< all the worlds that exist
    static bool quit_all; ///< quit all worlds ASAP  
    static void UpdateCb( World* world);
    static unsigned int next_id; ///<initially zero, used to allocate unique sequential world ids
	 
	 GList* chargers;
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
    stg_usec_t interval_sim; ///< temporal resolution: microseconds that elapse between simulated time steps 
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
    void LoadPuck( Worldfile* wf, int entity, GHashTable* entitytable );
    void LoadCharger( Worldfile* wf, int entity );

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
	 
	 void TryCharge( PowerPack* pp, Pose pose );

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
    Model* mod; ///< model to which this block belongs
  
    size_t pt_count; ///< the number of points
    stg_point_t* pts; ///< points defining a polygon
	 
    Size size;
	 
    Bounds local_z; ///<  z extent in local coords

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
      //formula found at http://www.cs.unc.edu/~hoff/techrep/openglz.html
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
    friend class Model;
    friend class Option;

  private:

    Canvas* canvas;
    std::vector<Option*> drawOptions;
    FileManager* fileMan; ///< Used to load and save worldfiles
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

    virtual void RemoveChild( Model* mod );	 
  };


  class Camera;

  /** energy data packet */
  class PowerPack
  {
  public:
	 PowerPack( Model* mod );

	 /** The model that owns this object */
	 Model* mod;
    
	 /** Energy stored */
	 stg_joules_t stored;

	 /** Energy capacity */
	 stg_joules_t capacity;

	 /** TRUE iff the device is receiving energy from a charger */
	 bool charging;

	 /** OpenGL visualization of the powerpack state */
	 void Visualize( Camera* cam );

	 /** Print human-readable status on stdout, prefixed with the
		  argument string */
	 void Print( char* prefix );
  };

  class Visibility
  {
  public:
	 bool blob_return;
	 int fiducial_key;
	 int fiducial_return;
	 bool gripper_return;
	 stg_laser_return_t laser_return;
	 bool obstacle_return;
	 bool ranger_return;
  
	 Visibility();
	 void Load( Worldfile* wf, int wf_entity );
  };

  /** Abstract class for adding visualizations to models. DataVisualize must be overloaded, and is then called in the models local coord system */
  class CustomVisualizer {
  public:
	 //TODO allow user to specify name - which will show up in display filter
	 virtual ~CustomVisualizer( void ) { }
	 virtual void DataVisualize( Camera* cam ) = 0;
	 virtual const std::string& name() = 0; //must return a name for visualization (careful not to return stack-memory)
  };



  /* Hooks for attaching special callback functions (not used as
	  variables - we just need unique addresses for them.) */  
  class CallbackHooks
  {
  public:
	 char load;
	 char save;
	 char shutdown;
	 char startup;
	 char update;
  };

  /** Records model state and functionality in the GUI, if used */
  class GuiState
  {
  public:
	 bool grid;
	 unsigned int mask;
	 bool nose;
	 bool outline;
  
	 GuiState();
	 void Load( Worldfile* wf, int wf_entity );
  };

  // class Option {
  // private:
  //   friend bool compare( const Option* lhs, const Option* rhs );
  
  //   std::string optName;
  //   bool value;
  //   /** worldfile entry string for loading and saving this value */
  //   std::string wf_token; 
  //   std::string shortcut;
  //   Fl_Menu_* menu;
  //   int menuIndex;
  //   Fl_Callback* menuCb;
  //   Fl_Widget* menuCbWidget;
  
  // public:
  //   Option( std::string n, std::string tok, std::string key, bool v, WorldGui* worldgui );	  
  
  //   const std::string name() const { return optName; }
  //   inline bool val() const { return value; }
  //   inline operator bool() { return val(); }
  //   inline bool operator<( const Option& rhs ) const
  //   { return optName<rhs.optName; } 
  //   void set( bool val );
  //   void invert() { set( !value ); }
  
  //   // Comparator to dereference Option pointers and compare their strings
  //   struct optComp {
  // 	 inline bool operator()( const Option* lhs, const Option* rhs ) const
  // 	 { return lhs->operator<(*rhs); } 
  //   };
  
  
  //   void createMenuItem( Fl_Menu_Bar* menu, std::string path );
  // 		void menuCallback( Fl_Callback* cb, Fl_Widget* w );
  //   static void toggleCb( Fl_Widget* w, void* p );
  //   void Load( Worldfile* wf, int section );
  //   void Save( Worldfile* wf, int section );	  
  // };


  /// %Model class
  class Model : public Ancestor
  {
	 friend class Ancestor;
	 friend class World;
	 friend class WorldGui;
	 friend class Canvas;
	 friend class Block;
	 friend class Region;
	 friend class BlockGroup;
  
  private:
	 /** the number of models instatiated - used to assign unique IDs */
	 static uint32_t count;
	 static GHashTable*  modelsbyid;
	 std::vector<Option*> drawOptions;
	 const std::vector<Option*>& getOptions() const { return drawOptions; }
  
  protected:
	 GMutex* access_mutex;
	 GPtrArray* blinkenlights;  
	 BlockGroup blockgroup;
	 /**  OpenGL display list identifier for the blockgroup */
	 int blocks_dl;

	 /** Iff true, 4 thin blocks are automatically added to the model,
		  forming a solid boundary around the bounding box of the
		  model. */
	 int boundary;

	 /** callback functions can be attached to any field in this
		  structure. When the internal function model_change(<mod>,<field>)
		  is called, the callback is triggered */
	 GHashTable* callbacks;

	 /** Default color of the model's blocks.*/
	 stg_color_t color;

	 /** This can be set to indicate that the model has new data that
		  may be of interest to users. This allows polling the model
		  instead of adding a data callback. */
	 bool data_fresh;
	 stg_bool_t disabled; ///< if non-zero, the model is disabled  
	 GList* custom_visual_list;
	 GList* flag_list;
	 Geom geom;
	 Pose global_pose;
	 bool gpose_dirty; ///< set this to indicate that global pose may have changed  
	 /** Controls our appearance and functionality in the GUI, if used */
	 GuiState gui;
  
	 bool has_default_block;
  
	 /* hooks for attaching special callback functions (not used as
		 variables - we just need unique addresses for them.) */  
	 CallbackHooks hooks;
  
	 /** unique process-wide identifier for this model */
	 uint32_t id;	
	 ctrlinit_t* initfunc;
	 stg_usec_t interval; ///< time between updates in us
	 stg_usec_t last_update; ///< time of last update in us  
	 bool map_caches_are_invalid;
	 stg_meters_t map_resolution;
	 stg_kg_t mass;
	 bool on_update_list;
	 bool on_velocity_list;

	 /** Pointer to the parent of this model, possibly NULL. */
	 Model* parent; 

	 /** The pose of the model in it's parents coordinate frame, or the
		  global coordinate frame is the parent is NULL. */
	 Pose pose;


	 /** GData datalist can contain arbitrary named data items. Can be used
		  by derived model types to store properties, and for user code
		  to associate arbitrary items with a model. */
	 GData* props;
	 bool rebuild_displaylist; ///< iff true, regenerate block display list before redraw
	 char* say_string;   ///< if non-null, this string is displayed in the GUI 

	 stg_bool_t stall;
	 /** Thread safety flag. Iff true, Update() may be called in
		  parallel with other models. Defaults to false for safety */
	 int subs;    ///< the number of subscriptions to this model
	 bool thread_safe;
	 GArray* trail;
	 stg_model_type_t type;  
	 bool used;   ///< TRUE iff this model has been returned by GetUnusedModelOfType()  
	 Velocity velocity;
	 stg_watts_t watts;///< power consumed by this model
	 Worldfile* wf;
	 int wf_entity;
	 World* world; // pointer to the world in which this model exists
	 WorldGui* world_gui; //pointer to the GUI world - NULL if running in non-gui mode

  public:

	 Visibility vis;

	 /** Optional attached PowerPack, defaults to NULL */
	 PowerPack* power_pack;

	 void Lock()
	 { 
		if( access_mutex == NULL )
		  access_mutex = g_mutex_new();
		
		assert( access_mutex );
		g_mutex_lock( access_mutex );
	 }
	 
	 void Unlock()
	 { 
		assert( access_mutex );
		g_mutex_unlock( access_mutex );
	 }
	 

  private: 
	 /** Private copy constructor declared but not defined, to make it
		  impossible to copy models. */
	 explicit Model(const Model& original);

	 /** Private assignment operator declared but not defined, to make it
		  impossible to copy models */
	 Model& operator=(const Model& original);

  protected:

	 /// Register an Option for pickup by the GUI
	 void registerOption( Option* opt )
	 { drawOptions.push_back( opt ); }

	 /** Check to see if the current pose will yield a collision with
		  obstacles.  Returns a pointer to the first entity we are in
		  collision with, or NULL if no collision exists. */
	 Model* TestCollision();
  
	 void CommitTestedPose();

	 void Map();
	 void UnMap();

	 void MapWithChildren();
	 void UnMapWithChildren();
  
	 int TreeToPtrArray( GPtrArray* array );
  
	 /** raytraces a single ray from the point and heading identified by
		  pose, in local coords */
	 stg_raytrace_result_t Raytrace( const Pose &pose,
												const stg_meters_t range, 
												const stg_ray_test_func_t func,
												const void* arg,
												const bool ztest = true );
  
	 /** raytraces multiple rays around the point and heading identified
		  by pose, in local coords */
	 void Raytrace( const Pose &pose,
						 const stg_meters_t range, 
						 const stg_radians_t fov, 
						 const stg_ray_test_func_t func,
						 const void* arg,
						 stg_raytrace_result_t* samples,
						 const uint32_t sample_count,
						 const bool ztest = true  );
  
	 stg_raytrace_result_t Raytrace( const stg_radians_t bearing, 			 
												const stg_meters_t range,
												const stg_ray_test_func_t func,
												const void* arg,
												const bool ztest = true );
  
	 void Raytrace( const stg_radians_t bearing, 			 
						 const stg_meters_t range,
						 const stg_radians_t fov,
						 const stg_ray_test_func_t func,
						 const void* arg,
						 stg_raytrace_result_t* samples,
						 const uint32_t sample_count,
						 const bool ztest = true );
  

	 /** Causes this model and its children to recompute their global
		  position instead of using a cached pose in
		  Model::GetGlobalPose()..*/
	 void GPoseDirtyTree();

	 virtual void Startup();
	 virtual void Shutdown();
	 virtual void Update();
	 virtual void UpdatePose();

	 void StartUpdating();
	 void StopUpdating();

	 Model* ConditionalMove( Pose newpose );

	 stg_meters_t ModelHeight();

	 bool UpdateDue( void );
	 void UpdateIfDue();
  
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
	 void DrawImage( uint32_t texture_id, Camera* cam, float alpha );
  
  
	 /** static wrapper for DrawBlocks() */
	 static void DrawBlocks( gpointer dummykey, 
									 Model* mod, 
									 void* arg );
  
	 virtual void DrawPicker();
	 virtual void DataVisualize( Camera* cam );
  
	 virtual void DrawSelected(void);
  
	 void DrawTrailFootprint();
	 void DrawTrailBlocks();
	 void DrawTrailArrows();
	 void DrawGrid();
  

	 void DrawBlinkenlights();

	 void DataVisualizeTree( Camera* cam );
	
	 virtual void PushColor( stg_color_t col )
	 { world->PushColor( col ); }
	
	 virtual void PushColor( double r, double g, double b, double a )
	 { world->PushColor( r,g,b,a ); }
	
	 virtual void PopColor(){ world->PopColor(); }
	
	 void DrawFlagList();

	 void DrawPose( Pose pose );

  public:
	 void RecordRenderPoint( GSList** head, GSList* link, 
									 unsigned int* c1, unsigned int* c2 );

	 void PlaceInFreeSpace( stg_meters_t xmin, stg_meters_t xmax, 
									stg_meters_t ymin, stg_meters_t ymax );
	
	 /** Look up a model pointer by a unique model ID */
	 static Model* LookupId( uint32_t id )
	 { return (Model*)g_hash_table_lookup( modelsbyid, (void*)id ); }
	
	 /** Constructor */
	 Model( World* world, 
			  Model* parent, 
			  stg_model_type_t type = MODEL_TYPE_PLAIN );

	 /** Destructor */
	 virtual ~Model();
	
	 void Say( const char* str );
	 /** Attach a user supplied visualization to a model */
	 void AddCustomVisualizer( CustomVisualizer* custom_visual );
	 /** remove user supplied visualization to a model - supply the same ptr passed to AddCustomVisualizer */
	 void RemoveCustomVisualizer( CustomVisualizer* custom_visual );

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
	
	 /** Should be called after all models are loaded, to do any last-minute setup */
	 void Init();	
	 void InitRecursive();

	 void AddFlag(  Flag* flag );
	 void RemoveFlag( Flag* flag );
	
	 void PushFlag( Flag* flag );
	 Flag* PopFlag();
	
	 int GetFlagCount(){ return g_list_length( flag_list ); }
  
	 /** Add a pointer to a blinkenlight to the model. */
	 void AddBlinkenlight( stg_blinkenlight_t* b )
	 { g_ptr_array_add( this->blinkenlights, b ); }
  
	 /** Clear all blinkenlights from the model. Does not destroy the
		  blinkenlight objects. */
	 void ClearBlinkenlights()
	 {  g_ptr_array_set_size( this->blinkenlights, 0 ); }
  
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
	 void LoadControllerModule( char* lib );
	
	 /** Sets the redraw flag, so this model will be redrawn at the
		  earliest opportunity */
	 void NeedRedraw();
	
	 /** Add a block to this model by loading it from a worldfile
		  entity */
	 void LoadBlock( Worldfile* wf, int entity );

	 /** Add a block to this model centered at [x,y] with extent [dx, dy,
		  dz] */
	 void AddBlockRect( stg_meters_t x, stg_meters_t y, 
							  stg_meters_t dx, stg_meters_t dy, 
							  stg_meters_t dz );
	
	 /** remove all blocks from this model, freeing their memory */
	 void ClearBlocks();
  
	 /** Returns a pointer to this model's parent model, or NULL if this
		  model has no parent */
	 Model* Parent(){ return this->parent; }

	 Model* GetModel( const char* name );
	 //int GuiMask(){ return this->gui_mask; };

	 /** Returns a pointer to the world that contains this model */
	 World* GetWorld(){ return this->world; }
  
	 /** return the root model of the tree containing this model */
	 Model* Root(){ return(  parent ? parent->Root() : this ); }
  
	 bool IsAntecedent( Model* testmod );
	
	 /** returns true if model [testmod] is a descendent of this model */
	 bool IsDescendent( Model* testmod );
	
	 /** returns true if model [testmod] is a descendent or antecedent of this model */
	 bool IsRelated( Model* testmod );

	 /** get the pose of a model in the global CS */
	 Pose GetGlobalPose();
	
	 /** get the velocity of a model in the global CS */
	 Velocity GetGlobalVelocity();
	
	 /* set the velocity of a model in the global coordinate system */
	 void SetGlobalVelocity(  Velocity gvel );
	
	 /** subscribe to a model's data */
	 void Subscribe();
	
	 /** unsubscribe from a model's data */
	 void Unsubscribe();
	
	 /** set the pose of model in global coordinates */
	 void SetGlobalPose(  Pose gpose );
	
	 /** set a model's velocity in its parent's coordinate system */
	 void SetVelocity(  Velocity vel );
	
	 /** set a model's pose in its parent's coordinate system */
	 void SetPose(  Pose pose );
	
	 /** add values to a model's pose in its parent's coordinate system */
	 void AddToPose(  Pose pose );
	
	 /** add values to a model's pose in its parent's coordinate system */
	 void AddToPose(  double dx, double dy, double dz, double da );
	
	 /** set a model's geometry (size and center offsets) */
	 void SetGeom(  Geom src );
  
	 /** Set a model's fiducial return value. Values less than zero
		  are not detected by the fiducial sensor. */
	 void SetFiducialReturn(  int fid );
  
	 /** Get a model's fiducial return value. */
	 int GetFiducialReturn()
	 { return vis.fiducial_return; }
  
	 /** set a model's fiducial key: only fiducial finders with a
		  matching key can detect this model as a fiducial. */
	 void SetFiducialKey(  int key );
	
	 stg_color_t GetColor(){ return color; }
	
	 //  stg_laser_return_t GetLaserReturn(){ return laser_return; }
	
	 /** Change a model's parent - experimental*/
	 int SetParent( Model* newparent);
	
	 /** Get a model's geometry - it's size and local pose (offset from
		  origin in local coords) */
	 Geom GetGeom(){ return geom; }
	
	 /** Get the pose of a model in its parent's coordinate system  */
	 Pose GetPose(){ return pose; }
	
	 /** Get a model's velocity (in its local reference frame) */
	 Velocity GetVelocity(){ return velocity; }
	
	 // guess what these do?
	 void SetColor( stg_color_t col );
	 void SetMass( stg_kg_t mass );
	 void SetStall( stg_bool_t stall );
	 void SetGripperReturn( int val );
	 void SetLaserReturn( stg_laser_return_t val );
	 void SetObstacleReturn( int val );
	 void SetBlobReturn( int val );
	 void SetRangerReturn( int val );
	 void SetBoundary( int val );
	 void SetGuiNose( int val );
	 void SetGuiMask( int val );
	 void SetGuiGrid( int val );
	 void SetGuiOutline( int val );
	 void SetWatts( stg_watts_t watts );
	 void SetMapResolution( stg_meters_t res );
	
	 bool DataIsFresh(){ return this->data_fresh; }
	
	 /* attach callback functions to data members. The function gets
		 called when the member is changed using SetX() accessor method */
	
	 void AddCallback( void* address, 
							 stg_model_callback_t cb, 
							 void* user );
	
	 int RemoveCallback( void* member,
								stg_model_callback_t callback );
	
	 void CallCallbacks(  void* address );
	
	 /* wrappers for the generic callback add & remove functions that hide
		 some implementation detail */
	
	 void AddStartupCallback( stg_model_callback_t cb, void* user )
	 { AddCallback( &hooks.startup, cb, user ); };
	
	 void RemoveStartupCallback( stg_model_callback_t cb )
	 { RemoveCallback( &hooks.startup, cb ); };
	
	 void AddShutdownCallback( stg_model_callback_t cb, void* user )
	 { AddCallback( &hooks.shutdown, cb, user ); };
	
	 void RemoveShutdownCallback( stg_model_callback_t cb )
	 { RemoveCallback( &hooks.shutdown, cb ); }
	
	 void AddLoadCallback( stg_model_callback_t cb, void* user )
	 { AddCallback( &hooks.load, cb, user ); }
	
	 void RemoveLoadCallback( stg_model_callback_t cb )
	 { RemoveCallback( &hooks.load, cb ); }
	
	 void AddSaveCallback( stg_model_callback_t cb, void* user )
	 { AddCallback( &hooks.save, cb, user ); }
	
	 void RemoveSaveCallback( stg_model_callback_t cb )
	 { RemoveCallback( &hooks.save, cb ); }
  
	 void AddUpdateCallback( stg_model_callback_t cb, void* user )
	 { 
		AddCallback( &hooks.update, cb, user ); 
		Subscribe(); // if attaching a callback here, assume we want updates to happen
	 }
	
	 void RemoveUpdateCallback( stg_model_callback_t cb )
	 { 
		RemoveCallback( &hooks.update, cb ); 
		Unsubscribe();
	 }
	
	 /** named-property interface 
	  */
	 void* GetProperty( char* key );
	
	 /** @brief Set a named property of a Stage model.
	 
		  Set a property of a Stage model. 
	 
		  This function can set both predefined and user-defined
		  properties of a model. Predefined properties are intrinsic to
		  every model, such as pose and color. Every supported predefined
		  properties has its identifying string defined as a preprocessor
		  macro in stage.h. Users should use the macro instead of a
		  hard-coded string, so that the compiler can help you to avoid
		  mis-naming properties.
	 
		  User-defined properties allow the user to attach arbitrary data
		  pointers to a model. User-defined property data is not copied,
		  so the original pointer must remain valid. User-defined property
		  names are simple strings. Names beginning with an underscore
		  ('_') are reserved for internal libstage use: users should not
		  use names beginning with underscore (at risk of causing very
		  weird behaviour).
	 
		  Any callbacks registered for the named property will be called.      
	 
		  Returns 0 on success, or a positive error code on failure.
	 
		  *CAUTION* The caller is responsible for making sure the pointer
		  points to data of the correct type for the property, so use
		  carefully. Check the docs or the equivalent
		  stg_model_set_<property>() function definition to see the type
		  of data required for each property.
	 */ 
	 int SetProperty( char* key, void* data );
	 void UnsetProperty( char* key );
		
	 virtual void Print( char* prefix );
	 virtual const char* PrintWithPose();
	
	 /** Convert a pose in the world coordinate system into a model's
		  local coordinate system. Overwrites [pose] with the new
		  coordinate. */
	 Pose GlobalToLocal( const Pose pose );
	
	 /** Return the global pose (i.e. pose in world coordinates) of a
		  pose specified in the model's local coordinate system */
	 Pose LocalToGlobal( const Pose pose );
	
	 /** Return the 3d point in world coordinates of a 3d point
		  specified in the model's local coordinate system */
	 stg_point3_t LocalToGlobal( const stg_point3_t local );
	
	 /** returns the first descendent of this model that is unsubscribed
		  and has the type indicated by the string */
	 Model* GetUnsubscribedModelOfType( const stg_model_type_t type );
	
	 /** returns the first descendent of this model that is unused
		  and has the type indicated by the string. This model is tagged as used. */
	 Model* GetUnusedModelOfType( const stg_model_type_t type );
  
	 /** Returns the value of the model's stall boolean, which is true
		  iff the model has crashed into another model */
	 bool Stalled(){ return this->stall; }
  };


  // BLOBFINDER MODEL --------------------------------------------------------

  /** blobfinder data packet 
	*/
  typedef struct
  {
	 stg_color_t color;
	 uint32_t left, top, right, bottom;
	 stg_meters_t range;
  } stg_blobfinder_blob_t;

  /// %ModelBlobfinder class
  class ModelBlobfinder : public Model
  {
  private:
	 GArray* colors;
	 GArray* blobs;

	 // predicate for ray tracing
	 static bool BlockMatcher( Block* testblock, Model* finder );

	 static Option showBlobData;
	
	 virtual void DataVisualize( Camera* cam );

  public:
	 unsigned int scan_width;
	 unsigned int scan_height;
	 stg_meters_t range;
	 stg_radians_t fov;
	 stg_radians_t pan;

	 static  const char* typestr;

	 // constructor
	 ModelBlobfinder( World* world,
							Model* parent );
	 // destructor
	 ~ModelBlobfinder();

	 virtual void Startup();
	 virtual void Shutdown();
	 virtual void Update();
	 virtual void Load();

	 stg_blobfinder_blob_t* GetBlobs( unsigned int* count )
	 { 
		if( count ) *count = blobs->len;
		return (stg_blobfinder_blob_t*)blobs->data;
	 }

	 /** Start finding blobs with this color.*/
	 void AddColor( stg_color_t col );

	 /** Stop tracking blobs with this color */
	 void RemoveColor( stg_color_t col );

	 /** Stop tracking all colors. Call this to clear the defaults, then
		  add colors individually with AddColor(); */
	 void RemoveAllColors();
  };




  // LASER MODEL --------------------------------------------------------

  /** laser sample packet
	*/
  typedef struct
  {
	 stg_meters_t range; ///< range to laser hit in meters
	 double reflectance; ///< intensity of the reflection 0.0 to 1.0
  } stg_laser_sample_t;

  typedef struct
  {
	 uint32_t sample_count;
	 uint32_t resolution;
	 Bounds range_bounds;
	 stg_radians_t fov;
	 stg_usec_t interval;
  } stg_laser_cfg_t;

  /// %ModelLaser class
  class ModelLaser : public Model
  {
  private:
	 /** OpenGL displaylist for laser data */
	 int data_dl; 
	 bool data_dirty;

	 stg_laser_sample_t* samples;
	 uint32_t sample_count;
	 stg_meters_t range_min, range_max;
	 stg_radians_t fov;
	 uint32_t resolution;
  
	 static Option showLaserData;
	 static Option showLaserStrikes;
  
  public:
	 static const char* typestr;
	 // constructor
	 ModelLaser( World* world,
					 Model* parent ); 
  
	 // destructor
	 ~ModelLaser();
  
	 virtual void Startup();
	 virtual void Shutdown();
	 virtual void Update();
	 virtual void Load();
	 virtual void Print( char* prefix );
	 virtual void DataVisualize( Camera* cam );
  
	 uint32_t GetSampleCount(){ return sample_count; }
  
	 stg_laser_sample_t* GetSamples( uint32_t* count=NULL);
  
	 void SetSamples( stg_laser_sample_t* samples, uint32_t count);
  
	 // Get the user-tweakable configuration of the laser
	 stg_laser_cfg_t GetConfig( );
  
	 // Set the user-tweakable configuration of the laser
	 void SetConfig( stg_laser_cfg_t cfg );  
  };

  // \todo  GRIPPER MODEL --------------------------------------------------------

  //   typedef enum {
  //     STG_GRIPPER_PADDLE_OPEN = 0, // default state
  //     STG_GRIPPER_PADDLE_CLOSED, 
  //     STG_GRIPPER_PADDLE_OPENING,
  //     STG_GRIPPER_PADDLE_CLOSING,
  //   } stg_gripper_paddle_state_t;

  //   typedef enum {
  //     STG_GRIPPER_LIFT_DOWN = 0, // default state
  //     STG_GRIPPER_LIFT_UP, 
  //     STG_GRIPPER_LIFT_UPPING, // verbed these to match the paddle state
  //     STG_GRIPPER_LIFT_DOWNING, 
  //   } stg_gripper_lift_state_t;

  //   typedef enum {
  //     STG_GRIPPER_CMD_NOP = 0, // default state
  //     STG_GRIPPER_CMD_OPEN, 
  //     STG_GRIPPER_CMD_CLOSE,
  //     STG_GRIPPER_CMD_UP, 
  //     STG_GRIPPER_CMD_DOWN    
  //   } stg_gripper_cmd_type_t;

  //   /** gripper configuration packet
  //    */
  //   typedef struct
  //   {
  //     Size paddle_size; ///< paddle dimensions 

  //     stg_gripper_paddle_state_t paddles;
  //     stg_gripper_lift_state_t lift;

  //     double paddle_position; ///< 0.0 = full open, 1.0 full closed
  //     double lift_position; ///< 0.0 = full down, 1.0 full up

  //     stg_meters_t inner_break_beam_inset; ///< distance from the end of the paddle
  //     stg_meters_t outer_break_beam_inset; ///< distance from the end of the paddle  
  //     stg_bool_t paddles_stalled; // true iff some solid object stopped
  // 				// the paddles closing or opening

  //     GSList *grip_stack;  ///< stack of items gripped
  //     int grip_stack_size; ///< maximum number of objects in stack, or -1 for unlimited

  //     double close_limit; ///< How far the gripper can close. If < 1.0, the gripper has its mouth full.

  //   } stg_gripper_config_t;

  //   /** gripper command packet
  //    */
  //   typedef struct
  //   {
  //     stg_gripper_cmd_type_t cmd;
  //     int arg;
  //   } stg_gripper_cmd_t;


  //   /** gripper data packet
  //    */
  //   typedef struct
  //   {
  //     stg_gripper_paddle_state_t paddles;
  //     stg_gripper_lift_state_t lift;

  //     double paddle_position; ///< 0.0 = full open, 1.0 full closed
  //     double lift_position; ///< 0.0 = full down, 1.0 full up

  //     stg_bool_t inner_break_beam; ///< non-zero iff beam is broken
  //     stg_bool_t outer_break_beam; ///< non-zero iff beam is broken

  //     stg_bool_t paddle_contacts[2]; ///< non-zero iff paddles touch something

  //     stg_bool_t paddles_stalled; // true iff some solid object stopped
  // 				// the paddles closing or opening

  //     int stack_count; ///< number of objects in stack


  //   } stg_gripper_data_t;


  // \todo BUMPER MODEL --------------------------------------------------------

  //   typedef struct
  //   {
  //     Pose pose;
  //     stg_meters_t length;
  //   } stg_bumper_config_t;

  //   typedef struct
  //   {
  //     Model* hit;
  //     stg_point_t hit_point;
  //   } stg_bumper_sample_t;


  // FIDUCIAL MODEL --------------------------------------------------------

  /** fiducial config packet 
	*/
  typedef struct
  {
	 stg_meters_t max_range_anon;///< maximum detection range
	 stg_meters_t max_range_id; ///< maximum range at which the ID can be read
	 stg_meters_t min_range; ///< minimum detection range
	 stg_radians_t fov; ///< field of view 
	 stg_radians_t heading; ///< center of field of view

	 /// only detects fiducials with a key string that matches this one
	 /// (defaults to NULL)
	 int key;
  } stg_fiducial_config_t;

  /** fiducial data packet 
	*/
  typedef struct
  {
	 stg_meters_t range; ///< range to the target
	 stg_radians_t bearing; ///< bearing to the target 
	 Pose geom; ///< size and relative angle of the target
	 Pose pose; ///< Absolute accurate position of the target in world coordinates (it's cheating to use this in robot controllers!)
	 Model* mod; ///< Pointer to the model (real fiducial detectors can't do this!)
	 int id; ///< the fiducial identifier of the target (i.e. its fiducial_return value), or -1 if none can be detected.  
  } stg_fiducial_t;

  /// %ModelFiducial class
  class ModelFiducial : public Model
  {
  private:
	 // if neighbor is visible, add him to the fiducial scan
	 void AddModelIfVisible( Model* him );

	 // static wrapper function can be used as a function pointer
	 static int AddModelIfVisibleStatic( Model* him, ModelFiducial* me )
	 { if( him != me ) me->AddModelIfVisible( him ); return 0; }

	 virtual void Update();
	 virtual void DataVisualize( Camera* cam );

	 GArray* data;
	
	 static Option showFiducialData;
	
  public:
	 static const char* typestr;
	 // constructor
	 ModelFiducial( World* world,
						 Model* parent );
	 // destructor
	 virtual ~ModelFiducial();
  
	 virtual void Load();
	 void Shutdown( void );

	 stg_meters_t max_range_anon;///< maximum detection range
	 stg_meters_t max_range_id; ///< maximum range at which the ID can be read
	 stg_meters_t min_range; ///< minimum detection range
	 stg_radians_t fov; ///< field of view 
	 stg_radians_t heading; ///< center of field of view
	 int key; ///< /// only detect fiducials with a key that matches this one (defaults 0)

	 stg_fiducial_t* fiducials; ///< array of detected fiducials
	 uint32_t fiducial_count; ///< the number of fiducials detected
  };


  // RANGER MODEL --------------------------------------------------------

  typedef struct
  {
	 Pose pose;
	 Size size;
	 Bounds bounds_range;
	 stg_radians_t fov;
	 int ray_count;
  } stg_ranger_sensor_t;

  /// %ModelRanger class
  class ModelRanger : public Model
  {
  protected:

	 virtual void Startup();
	 virtual void Shutdown();
	 virtual void Update();
	 virtual void DataVisualize( Camera* cam );

  public:
	 static const char* typestr;
	 // constructor
	 ModelRanger( World* world,
					  Model* parent );
	 // destructor
	 virtual ~ModelRanger();

	 virtual void Load();
	 virtual void Print( char* prefix );

	 uint32_t sensor_count;
	 stg_ranger_sensor_t* sensors;
	 stg_meters_t* samples;
	
  private:
	 static Option showRangerData;
	 static Option showRangerTransducers;
	
  };

  // BLINKENLIGHT MODEL ----------------------------------------------------
  class ModelBlinkenlight : public Model
  {
  private:
	 double dutycycle;
	 bool enabled;
	 stg_msec_t period;
	 bool on;

	 static Option showBlinkenData;
  public:

	 static const char* typestr;
	 ModelBlinkenlight( World* world,
							  Model* parent );

	 ~ModelBlinkenlight();

	 virtual void Load();
	 virtual void Update();
	 virtual void DataVisualize( Camera* cam );
  };

  // CAMERA MODEL ----------------------------------------------------
  typedef struct {
	 // GL_V3F
	 GLfloat x, y, z;
  } ColoredVertex;
	
  /// %ModelCamera class
  class ModelCamera : public Model
  {
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
	 float _yaw_offset; //position camera is mounted at
	 float _pitch_offset;
		
	 ///Take a screenshot from the camera's perspective. return: true for sucess, and data is available via FrameDepth() / FrameColor()
	 bool GetFrame();
	
  public:
	 ModelCamera( World* world,
					  Model* parent ); 

	 ~ModelCamera();

	 virtual void Load();
	
	 ///Capture a new frame ( calls GetFrame )
	 virtual void Update();
	
	 ///Draw Camera Model - TODO
	 //virtual void Draw( uint32_t flags, Canvas* canvas );
	
	 ///Draw camera visualization
	 virtual void DataVisualize( Camera* cam );
	
	 ///width of captured image
	 inline int getWidth( void ) const { return _width; }
	
	 ///height of captured image
	 inline int getHeight( void ) const { return _height; }
	
	 ///get reference to camera used
	 inline const PerspectiveCamera& getCamera( void ) const { return _camera; }
	
	 ///get a reference to camera depth buffer
	 inline const GLfloat* FrameDepth() const { return _frame_data; }
	
	 ///get a reference to camera color image. 3 bytes (RGB) per pixel
	 inline const GLubyte* FrameColor() const { return _frame_color_data; }
	
	 ///change the pitch
	 inline void setPitch( float pitch ) { _pitch_offset = pitch; _valid_vertexbuf_cache = false; }
	
	 ///change the yaw
	 inline void setYaw( float yaw ) { _yaw_offset = yaw; _valid_vertexbuf_cache = false; }
  };

  // POSITION MODEL --------------------------------------------------------

  /** Define a position  control method */
  typedef enum
	 { STG_POSITION_CONTROL_VELOCITY, 
		STG_POSITION_CONTROL_POSITION 
	 } stg_position_control_mode_t;

  /** Define a localization method */
  typedef enum
	 { STG_POSITION_LOCALIZATION_GPS, 
		STG_POSITION_LOCALIZATION_ODOM 
	 } stg_position_localization_mode_t;

  /** Define a driving method */
  typedef enum
	 { STG_POSITION_DRIVE_DIFFERENTIAL, 
		STG_POSITION_DRIVE_OMNI, 
		STG_POSITION_DRIVE_CAR 
	 } stg_position_drive_mode_t;


  /// %ModelPosition class
  class ModelPosition : public Model
  {
	 friend class Canvas;

  private:
	 Pose goal;///< the current velocity or pose to reach, depending on the value of control_mode
	 stg_position_control_mode_t control_mode;
	 stg_position_drive_mode_t drive_mode;
	 stg_position_localization_mode_t localization_mode; ///< global or local mode
	 Velocity integration_error; ///< errors to apply in simple odometry model

	 Waypoint* waypoints;
	 uint32_t waypoint_count;	 
	 void DrawWaypoints();

  private:
	 static Option showCoords;
	 static Option showWaypoints;

  public:
	 static const char* typestr;
	 // constructor
	 ModelPosition( World* world,
						 Model* parent );
	 // destructor
	 ~ModelPosition();

	 virtual void Startup();
	 virtual void Shutdown();
	 virtual void Update();
	 virtual void Load();
	 
	 virtual void DataVisualize( Camera* cam );

	 /** Set the waypoint array pointer. Returns the old pointer, in case you need to free/delete[] it */
	 Waypoint* SetWaypoints( Waypoint* wps, uint32_t count );
	
	 /** Set the current pose estimate.*/
	 void SetOdom( Pose odom );

	 /** Sets the control_mode to STG_POSITION_CONTROL_VELOCITY and sets
		  the goal velocity. */
	 void SetSpeed( double x, double y, double a );
	 void SetXSpeed( double x );
	 void SetYSpeed( double y );
	 void SetZSpeed( double z );
	 void SetTurnSpeed( double a );
	 void SetSpeed( Velocity vel );

	 /** Sets the control mode to STG_POSITION_CONTROL_POSITION and sets
		  the goal pose */
	 void GoTo( double x, double y, double a );
	 void GoTo( Pose pose );

	 // localization state
	 Pose est_pose; ///< position estimate in local coordinates
	 Pose est_pose_error; ///< estimated error in position estimate
	 Pose est_origin; ///< global origin of the local coordinate system
  };


  class ModelScooper : public Model
  {
	 GList* items;
	 
  public:
	 ModelScooper( World* world, Model* parent );	 	  
  };


}; // end namespace stg

#endif
