#ifndef STG_H
#define STG_H
/*
 *  Stage : a multi-robot simulator.  Part of the Player Project
 * 
 *  Copyright (C) 2001-2007 Richard Vaughan, Brian Gerkey, Andrew
 *  Howard
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

/* File: stage.h
 * Desc: External header file for the Stage library
 * Author: Richard Vaughan (vaughan@sfu.ca) 
 * Date: 1 June 2003
 * CVS: $Id: stage.hh,v 1.15 2008-04-01 23:57:41 rtv Exp $
 */

/*! \file stage.h 
  Stage library header file

  This header file contains the external interface for the Stage
  library
*/

/**
   \defgroup libstage libstage: the Stage Robot Simulation Library

   Here is where I describe libstage for the developer.

*/
/*@{*/

#include <unistd.h>
#include <stdint.h> // for portable int types eg. uint32_t
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>

// we use GLib's data structures extensively. Perhaps we'll move to
// C++ STL types to lose this dependency one day.
#include <glib.h> 

// FLTK Gui includes
#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Gl_Window.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Menu_Button.H>
#include <FL/Fl_Value_Slider.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>
#include <FL/fl_draw.H>
#include <FL/gl.h> // FLTK takes care of platform-specific GL stuff
#include <FL/glut.H>

#ifdef __APPLE__
#include <OpenGL/glu.h>
#else
#include <GL/glu.h>
#endif 


/** The Stage library uses its own namespace */
namespace Stg 
{
  /** Initialize the Stage library */
  void Init( int* argc, char** argv[] );

  /** returns true iff Stg::Init() has been called. */
  bool InitDone(); 

  /** Returns a hash table of model creator functions indexed by worldfile model string */
  GHashTable* Typetable();
  
  // foreward declare
  class Worldfile;

  /// Copyright string
  const char COPYRIGHT[] =				       
    "Copyright Richard Vaughan and contributors 2000-2008";

  /// Author string
  const char AUTHORS[] =					
    "Richard Vaughan, Brian Gerkey, Andrew Howard, Reed Hedges, Pooya Karimian and contributors.";
  
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
  
  /** The maximum length of a Stage model identifier string 
   */
  const uint32_t TOKEN_MAX = 64;

  /** Convenient constant 
   */
  const double thousand = 1e3;

  /** Convenient constant 
   */
  const double million = 1e6;

  /** Convenient constant 
   */
  const double billion = 1e9;

  /** convert an angle in radians to degrees
   */
  inline double rtod( double r ){ return( r*180.0/M_PI ); }
  
  /** convert an angle in degrees to radians 
   */
  inline double dtor( double d ){ return( d*M_PI/180.0 ); }
  
  /** Normalize an angle to within +/_ M_PI 
   */
  inline double normalize( double a ){ return( atan2(sin(a), cos(a)));}
  
  /** take binary sign of a, either -1, or 1 if >= 0 
   */
  inline int sgn( int a){ return( a<0 ? -1 : 1); }
  
  /** take binary sign of a, either -1, or 1 if >= 0 
   */
  inline double sgn( double a){ return( a<0 ? -1.0 : 1.0); }
  
  // forward declare
  class StgModel;
  
  /** Describe the image format used for saving screenshots 
   */
  typedef enum {
    STG_IMAGE_FORMAT_PNG,
    STG_IMAGE_FORMAT_JPG
  } stg_image_format_t;
  
  /** any integer value other than this is a valid fiducial ID  
   */
  enum { FiducialNone = 0 };

  /** Value that Uniquely identifies a model
   */
  typedef uint32_t stg_id_t;

  /** Metres: floating point unit of distance 
   */
  typedef double stg_meters_t;

  /** Radians: unit of angle 
   */
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

  //typedef double stg_friction_t;

  /** 32-bit ARGB color packed 0xAARRGGBB */
  typedef uint32_t stg_color_t;

  /** Specify a color from its components */
  stg_color_t stg_color_pack( double r, double g, double b, double a );

  /** Obtain the components of a color */
  void stg_color_unpack( stg_color_t col, 
			 double* r, double* g, double* b, double* a );

  /** obstacle value. 0 means the model does not behave, or is sensed,
      as an obstacle */
  //typedef int stg_obstacle_return_t;

  /** blobfinder return value. 0 means not detected by the
      blobfinder */
  //typedef int stg_blob_return_t;

  /** fiducial return value. 0 means not detected as a fiducial */
  //typedef int stg_fiducial_return_t;

  //typedef int stg_ranger_return_t;
  
  //typedef enum { STG_GRIP_NO = 0, STG_GRIP_YES } stg_gripper_return_t;
  
  /** specify a rectangular size */
  typedef struct 
  {
    stg_meters_t x, y, z;
  } stg_size_t;
  
  /** Specify a 3 axis position, in x, y and heading. */
  typedef struct
  {
    stg_meters_t x, y, z; //< location in 3 axes
    stg_radians_t a; //< rotation about the z axis. 
  } stg_pose_t;
  
  /** specify a 3 axis velocity in x, y and heading. */
  typedef stg_pose_t stg_velocity_t;  

  /** specify an object's basic geometry: position and rectangular
      size.  */
  typedef struct
  {
    stg_pose_t pose; //< position
    stg_size_t size; //< extent
  } stg_geom_t;
  
  /** bound a range of values, from min to max */
  typedef struct
  {
    double min; //< smallest value in range
    double max; //< largest value in range
  } stg_bounds_t;
  
  /** bound a volume along the x,y,z axes */
  typedef struct
  {
    stg_bounds_t x; //< volume extent along x axis
    stg_bounds_t y; //< volume extent along y axis
    stg_bounds_t z; //< volume extent along z axis 
  } stg_bounds3d_t;

  /** bound a range of range values, from min to max  
   */
  typedef struct
  {
    stg_meters_t min; //< smallest value in range
    stg_meters_t max; //< largest value in range
  } stg_range_bounds_t;
  
  /** define a three-dimensional bounding box */
  typedef struct
  {
    stg_bounds_t x, y, z;
  } stg_bbox3d_t;

  /** define a field-of-view: an angle and range bounds */
  typedef struct
  {
    stg_bounds_t range; //< min and max range of sensor
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
  
  /** define an integer point on the plane 
   */
  typedef struct
  {
    int32_t x,y;
  } stg_point_int_t;
  
  /** Create an array of [count] points. Caller must free the returned
      pointer, preferably with stg_points_destroy(). 
  */
  stg_point_t* stg_points_create( size_t count );

  /** frees a point array */ 
  void stg_points_destroy( stg_point_t* pts );
  
  /** create an array of 4 points containing the corners of a unit
      square.  */
  stg_point_t* stg_unit_square_points_create();


  /** matching function should return 0 iff the candidate block is
      acceptable  */
  typedef int(*stg_line3d_func_t)(int32_t x, int32_t y, int32_t z,
				  void* arg );


  /** Visit every voxel along a vector from (x,y,z) to (x+dx, y+dy, z+dz
      ). Call the function for every voxel, passing in the current voxel
      coordinates followed by the two arguments. Adapted from Graphics
      Gems IV, algorithm by Cohen & Kaufman 1991 */
  int stg_line_3d( int32_t x, int32_t y, int32_t z,
		   int32_t dx, int32_t dy, int32_t dz,
		   stg_line3d_func_t visit_voxel,
		   void* arg );

  /** Visit every voxel along a polygon defined by the array of
      points. Calls stg_line_3d(). */
  int stg_polygon_3d( stg_point_int_t* pts, 
		      unsigned int pt_count, 
		      stg_line3d_func_t visit_voxel, 
		      void* arg );
  

  const uint32_t STG_MOVE_TRANS = (1 << 0);
  const uint32_t STG_MOVE_ROT   = (1 << 1);
  const uint32_t STG_MOVE_SCALE = (1 << 2);
  
  typedef uint32_t stg_movemask_t;

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
      LaserTransparent, ///<not detected by laser model 
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
  typedef  struct
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

  /** calculate the sum of [p1] and [p2], in [p1]'s coordinate system, and
      copy the result into result. */
  void stg_pose_sum( stg_pose_t* result, stg_pose_t* p1, stg_pose_t* p2 );
  
  /** returns the sum of [p1] + [p2], in [p1]'s coordinate system */
  stg_pose_t pose_sum( stg_pose_t p1, stg_pose_t p2 );

  // PRETTY PRINTING -------------------------------------------------
  
  /** Report an error, with a standard, friendly message header */
  void stg_print_err( const char* err );
  /** Print human-readable geometry on stdout */
  void stg_print_geom( stg_geom_t* geom );
  /** Print human-readable pose on stdout */
  void stg_print_pose( stg_pose_t* pose );
  /** Print human-readable velocity on stdout */
  void stg_print_velocity( stg_velocity_t* vel );
  
  const uint32_t STG_SHOW_BLOCKS =     (1<<0);
  const uint32_t STG_SHOW_DATA =       (1<<1); 
  const uint32_t STG_SHOW_GEOM =       (1<<2);
  const uint32_t STG_SHOW_GRID =       (1<<3);
  const uint32_t STG_SHOW_OCCUPANCY =  (1<<4);
  const uint32_t STG_SHOW_TRAILS =     (1<<5); 
  const uint32_t STG_SHOW_FOLLOW =     (1<<6); 
  const uint32_t STG_SHOW_CLOCK =      (1<<7);
  const uint32_t STG_SHOW_QUADTREE =   (1<<8);
  const uint32_t STG_SHOW_ARROWS =     (1<<9);
  const uint32_t STG_SHOW_FOOTPRINT =  (1<<10);
  const uint32_t STG_SHOW_BLOCKS_2D =  (1<<10);

  // forward declare
  class StgWorld;
  class StgModel;

  GHashTable* stg_create_typetable();

  /** A model creator function. Each model type must define a function of this type. */
  typedef StgModel* (*stg_creator_t)( StgWorld*, StgModel*, stg_id_t, char* );
  
  typedef struct 
  {
    const char* token;
    stg_creator_t creator_fn;
  } stg_typetable_entry_t;


  void gl_draw_grid( stg_bounds3d_t vol );
  void gl_pose_shift( stg_pose_t* pose );
  void gl_coord_shift( double x, double y, double z, double a  );

  class StgFlag
  {
  public:
    stg_color_t color;
    double size;

    StgFlag( stg_color_t color, double size );
    StgFlag* Nibble( double portion );
  };
  
  /** Render a string at [x,y,z] in the current color */
  void gl_draw_string( float x, float y, float z, char *string);


  void stg_block_list_scale( GList* blocks, 
			     stg_size_t* size );
  void stg_block_list_destroy( GList* list );


  typedef int(*stg_model_callback_t)(StgModel* mod, void* user );
  
  // return val, or minval if val < minval, or maxval if val > maxval
  double constrain( double val, double minval, double maxval );

  
  /** container for a callback function and a single argument, so
      they can be stored together in a list with a single pointer. */
  typedef struct
  {
    stg_model_callback_t callback;
    void* arg;
  } stg_cb_t;

  stg_cb_t* cb_create( stg_model_callback_t callback, void* arg );
  void cb_destroy( stg_cb_t* cb );

  typedef struct
  {
    StgModel* mod;
    void* member;
    char* name;
    stg_model_callback_t callback_on;
    stg_model_callback_t callback_off;
    void* arg_on; // argument to callback_on
    void* arg_off; // argument to callback_off
    //int default_state; // disabled = 0
    //GtkAction* action; // action associated with this toggle, may be NULL
    char* path;
  } stg_property_toggle_args_t;



  /** defines a rectangle of [size] located at [pose] */
  typedef struct
  {
    stg_pose_t pose;
    stg_size_t size;
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

  // /** matching function should return 0 iff the candidate block is
  //     acceptable */
  class StgBlock;

  typedef bool (*stg_block_match_func_t)(StgBlock* candidate, StgModel* finder, const void* arg );

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
  /*     //stg_pose_t probe_pose; // TODO - the origin of our probe */

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


  /*   // TOKEN ----------------------------------------------------------------------- */
  /*   // token stuff for parsing worldfiles - this may one day replace
       the worldfile c++ code */

  /* #define CFG_OPEN '(' */
  /* #define CFG_CLOSE ')' */
  /* #define STR_OPEN '\"' */
  /* #define STR_CLOSE '\"' */
  /* #define TPL_OPEN '[' */
  /* #define TPL_CLOSE ']' */

  /*   typedef enum { */
  /*     STG_T_NUM = 0, */
  /*     STG_T_BOOLEAN, */
  /*     STG_T_MODELPROP, */
  /*     STG_T_WORLDPROP, */
  /*     STG_T_NAME, */
  /*     STG_T_STRING, */
  /*     STG_T_KEYWORD, */
  /*     STG_T_CFG_OPEN, */
  /*     STG_T_CFG_CLOSE, */
  /*     STG_T_TPL_OPEN, */
  /*     STG_T_TPL_CLOSE, */
  /*   } stg_token_type_t; */




  /* typedef struct stg_token  */
  /* { */
  /*   char* token; ///< the text of the token */
  /*   stg_token_type_t type; ///< the type of the token */
  /*   unsigned int line; ///< the line on which the token appears */
  
  /*   struct stg_token* next; ///< linked list support */
  /*   struct stg_token* child; ///< tree support */
  
  /* } stg_token_t; */

  /*   stg_token_t* stg_token_next( stg_token_t* tokens ); */
  /*   /// print [token] formatted for a human reader, with a string [prefix] */
  /*   void stg_token_print( char* prefix,  stg_token_t* token ); */

  /*   /// print a token array suitable for human reader */
  /*   void stg_tokens_print( stg_token_t* tokens ); */
  /*   void stg_tokens_free( stg_token_t* tokens ); */
  
  /*   /// create a new token structure from the arguments */
  /*   stg_token_t* stg_token_create( const char* token, stg_token_type_t type, int line ); */

  /*   /// add a token to a list */
  /*   stg_token_t* stg_token_append( stg_token_t* head, */
  /* 				 char* token, stg_token_type_t type, int line ); */

  /*   const char* stg_token_type_string( stg_token_type_t type ); */

  /*   const char* stg_model_type_string( stg_model_type_t type ); */
  
  /*   //  functions for parsing worldfiles */
  /*   stg_token_t* stg_tokenize( FILE* wf ); */
  /*   //StgWorld* sc_load_worldblock( stg_client_t* cli, stg_token_t** tokensptr ); */
  /*   //stg_model_t* sc_load_modelblock( StgWorld* world, stg_model_t* parent, */
  /*   //			   stg_token_t** tokensptr ); */




  //#ifdef __cplusplus
  //}
  //#endif 


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

  class StgBlock;
  class StgModel;


  stg_msec_t stg_realtime();
  stg_msec_t stg_realtime_since_start( void );



  // ANCESTOR CLASS

  /** Define a callback function type that can be attached to a
      record within a model and called whenever the record is set.*/
  typedef int (*stg_model_callback_t)( StgModel* mod, void* user );
  
  
  /** Base class for StgModel and StgWorld */
  class StgAncestor
  {
    friend class StgCanvas; // allow StgCanvas access to our private members
  
  protected:
    GList* children;
    GHashTable* child_types;
    char* token;
    bool debug;

  public:
    StgAncestor();
    virtual ~StgAncestor();
  
    unsigned int GetNumChildrenOfType( const char* typestr );
    void IncrementNumChildrenOfType( const char* typestr );
  
    virtual void AddChild( StgModel* mod );
    virtual void RemoveChild( StgModel* mod );
    virtual stg_pose_t GetGlobalPose();  
  
    const char* Token()
    { return this->token; }
  
    // PURE VIRTUAL - descendents must implement
    virtual void PushColor( stg_color_t col ) = 0;
    virtual void PushColor( double r, double g, double b, double a ) = 0;
    virtual void PopColor() = 0; // does nothing
  };

  
  typedef struct 
  {
    int enabled;
    stg_pose_t pose;
    stg_meters_t size; ///< rendered as a sphere with this diameter
    stg_color_t color;
    stg_msec_t period; ///< duration of a complete cycle
    double duty_cycle; ///< mark/space ratio
  } stg_blinkenlight_t;

  typedef struct
  {
    uint32_t counter;
    GSList** lists;
  } stg_bigblock_t;

  typedef struct
  {
    StgWorld* world;
    StgBlock* block;
  } stg_render_info_t;


  class StgBlockGrid 
  {
  private:
    //stg_bigblock_t* map;
    GSList** cells;

    //GTrashStack* trashstack;  
    uint32_t width, height;// bwidth, bheight;

  public:
    //uint32_t numbits;
    StgBlockGrid( uint32_t width, uint32_t height );
    ~StgBlockGrid();
    void AddBlock( uint32_t x, uint32_t y, StgBlock* block );
    void RemoveBlock( uint32_t x, uint32_t y, StgBlock* block );
    GSList* GetList( uint32_t x, uint32_t y );
    void GlobalRemoveBlock( StgBlock* block );
    void Draw( bool drawall );
  
    /** Returns the number of blocks occupying the big block, specified
	in big block coordinates, NOT in small blocks.*/
    //uint32_t BigBlockOccupancy( uint32_t bbx, uint32_t bby );
  };
  
  /** raytrace sample
   */
  typedef struct
  {
    stg_pose_t pose; ///< location and direction of the ray origin
    stg_meters_t range; ///< range to beam hit in meters
    StgBlock* block; ///< the block struck by this beam
  } stg_raytrace_sample_t;


  const uint32_t INTERVAL_LOG_LEN = 32;
  
  class Region;
  class SuperRegion;

  /** WORLD CLASS */
  class StgWorld : public StgAncestor
  {
    friend class StgModel; // allow access to private members
    friend class StgBlock;
    friend class StgTime;

  private:

    static bool quit_all; // quit all worlds ASAP  
    static unsigned int next_id; //< initialized to zero, used to
    //allocate unique sequential world ids
    static int AddBlockPixel( int x, int y, int z,
			      stg_render_info_t* rinfo ) ; //< used as a callback by StgModel

    stg_usec_t real_time_next_update;
    stg_usec_t real_time_start;
  
    bool quit; // quit this world ASAP

  
    inline void MetersToPixels( stg_meters_t mx, stg_meters_t my, 
				int32_t *px, int32_t *py );
    
    void Initialize( const char* token, 
		     stg_msec_t interval_sim, 
		     stg_msec_t interval_real,
		     double ppm );
    //	     double width,
    //	     double height );
  

    virtual void PushColor( stg_color_t col ) { /* do nothing */  };
    virtual void PushColor( double r, double g, double b, double a ) { /* do nothing */  };
    virtual void PopColor(){ /* do nothing */  };

    stg_usec_t real_time_now;
 
    /** StgWorld::quit is set true when this simulation time is reached */
    stg_usec_t quit_time; 
 
    bool destroy;

    stg_id_t id;

    GHashTable* models_by_id; ///< the models that make up the world, indexed by id
    GHashTable* models_by_name; ///< the models that make up the world, indexed by name
    GList* velocity_list; ///< a list of models that have non-zero velocity, for efficient updating

    stg_usec_t sim_time; ///< the current sim time in this world in ms
    stg_usec_t wall_last_update; ///< the real time of the last update in ms

    long unsigned int updates; ///< the number of simulated time steps executed so far
  
    bool dirty; ///< iff true, a gui redraw would be required

    stg_usec_t interval_sleep_max;
    stg_usec_t interval_sim; ///< temporal resolution: milliseconds that elapse between simulated time steps 

    stg_usec_t interval_log[INTERVAL_LOG_LEN];
  
    int total_subs; ///< the total number of subscriptions to all models
    double ppm; ///< the resolution of the world model in pixels per meter  

    bool paused; ///< the world only updates when this is false

    GList* update_list; //< the descendants that need Update() called
    void AddModelName( StgModel* mod );  

    void StartUpdatingModel( StgModel* mod );
    void StopUpdatingModel( StgModel* mod );
  
    //void MapBlock( StgBlock* block );

    SuperRegion* CreateSuperRegion( int32_t x, int32_t y );
    void DestroySuperRegion( SuperRegion* sr );

    void Raytrace( stg_pose_t pose, 			 
		   stg_meters_t range,
		   stg_block_match_func_t func,
		   StgModel* finder,
		   const void* arg,
		   stg_raytrace_sample_t* sample );
  
    void Raytrace( stg_pose_t pose, 			 
		   stg_meters_t range,
		   stg_radians_t fov,
		   stg_block_match_func_t func,
		   StgModel* finder,
		   const void* arg,
		   stg_raytrace_sample_t* samples,
		   uint32_t sample_count );
  
    void RemoveBlock( int x, int y, StgBlock* block );
    //{ }//bgrid->RemoveBlock( x, y, block ); };
  
  protected:
    stg_usec_t interval_real;   ///< real-time interval between updates - set this to zero for 'as fast as possible

    GHashTable* superregions;

    GList* ray_list;
    // store rays traced for debugging purposes
    void RecordRay( double x1, double y1, double x2, double y2 );
    Worldfile* wf; ///< If set, points to the worldfile used to create this world
    bool graphics;
    stg_bounds3d_t extent;
    
    /** Enlarge the bounding volume to include this point */
    void Extend( stg_point3_t pt );
    
    //GHashTable* blocks;
    GArray lines;

  public:

    StgWorld();
  
    StgWorld( const char* token, 
	      stg_msec_t interval_sim, 
	      stg_msec_t interval_real,
	      double ppm ); 
  
    virtual ~StgWorld();
    
    stg_usec_t SimTimeNow(void){ return sim_time;} ;
    stg_usec_t RealTimeNow(void);
    stg_usec_t RealTimeSinceStart(void);
    void PauseUntilNextUpdateTime(void);
    void IdleUntilNextUpdateTime( int (*idler)(void) );

    void AddBlock( StgBlock* block );
    void RemoveBlock( StgBlock* block );
  
    stg_usec_t GetSimInterval(){ return interval_sim; }; 
    

    Worldfile* GetWorldFile(){ return wf; };
  
    virtual void Load( const char* worldfile_path );
    virtual void Reload();
    virtual void Save();
    virtual bool Update(void);
    virtual bool RealTimeUpdate(void);
    virtual bool RealTimeUpdateWithIdler( int (*idler)(void) );

    virtual void AddModel( StgModel* mod );
    virtual void RemoveModel( StgModel* mod );
  
    void Start(){ paused = false; };
    void Stop(){ paused = true; };
    void TogglePause(){ paused = !paused; };
    bool TestQuit(){ return( quit || quit_all );  }
    void Quit(){ quit = true; }
    void QuitAll(){ quit_all = true; }
    void CancelQuit(){ quit = false; }
    void CancelQuitAll(){ quit_all = false; }
  
    double Resolution(){ return ppm; };
  
    StgModel* GetModel( const stg_id_t id );
    StgModel* GetModel( const char* name );
    

    GList* GetRayList(){ return ray_list; };
    void ClearRays();
    
    void ClockString( char* str, size_t maxlen );

    stg_bounds3d_t GetExtent(){ return extent; };  
        
    void ForEachModel( GHFunc func, void* arg )
    { g_hash_table_foreach( models_by_id, func, arg ); };

    long unsigned int GetUpdateCount()
    { return updates; }
  };


  typedef struct {
    stg_pose_t pose;
    stg_color_t color;
    stg_usec_t time;
  } stg_trail_item_t;
  
  typedef int ctrlinit_t( StgModel* mod );
  //typedef void ctrlupdate_t( StgModel* mod );

  // MODEL CLASS
  class StgModel : public StgAncestor
  {
    friend class StgAncestor;
    friend class StgWorld;
    friend class StgWorldGui;
    friend class StgCanvas;

  protected:

    const char* typestr;
    stg_pose_t pose;
    stg_velocity_t velocity;    
    stg_watts_t watts; //< power consumed by this model
    stg_color_t color;
    stg_kg_t mass;
    stg_geom_t geom;
    stg_laser_return_t laser_return;
    int obstacle_return;
    int blob_return;
    int gripper_return;
    int ranger_return;
    int fiducial_return;
    int fiducial_key;
    int boundary;
    stg_meters_t map_resolution;
    stg_bool_t stall;
  
    /** if non-null, this string is displayed in the GUI */
    char* say_string; 

    bool on_velocity_list;

    int gui_nose;
    int gui_grid;
    int gui_outline;
    int gui_mask;
  
    StgModel* parent; //< the model that owns this one, possibly NULL
  
    /** GData datalist can contain arbitrary named data items. Can be used
	by derived model types to store properties, and for user code
	to associate arbitrary items with a model. */
    GData* props;
  
    /** callback functions can be attached to any field in this
	structure. When the internal function model_change(<mod>,<field>)
	is called, the callback is triggered */
    GHashTable* callbacks;
  
    int subs;     //< the number of subscriptions to this model
  
    stg_usec_t interval; //< time between updates in ms
    stg_usec_t last_update; //< time of last update in ms
  
    stg_bool_t disabled; //< if non-zero, the model is disabled
   
    GList* d_list;    
    GList* blocks; //< list of stg_block_t structs that comprise a body
  
    GArray* trail;
  
    //  stg_trail_item_t* history; 

    bool body_dirty; //< iff true, regenerate block display list before redraw
    bool data_dirty; //< iff true, regenerate data display list before redraw

    stg_pose_t global_pose;

    bool gpose_dirty; //< set this to indicate that global pose may have
    //changed

    bool data_fresh; ///< this can be set to indicate that the model has
    ///new data that may be of interest to users. This
    ///allows polling the model instead of adding a
    ///data callback.  

    stg_id_t id; // globally unique ID used as hash table key and
    // worldfile section identifier

    StgWorld* world; // pointer to the world in which this model exists

    StgModel* TestCollision( stg_pose_t* pose, 
			     double* hitx, double* hity );


    void Map();
    void UnMap();

    void MapWithChildren();
    void UnMapWithChildren();

    int TreeToPtrArray( GPtrArray* array );
  

    // raytraces from the point and heading identified by pose, in local coords
    void Raytrace( stg_pose_t pose,
		   stg_meters_t range, 
		   stg_block_match_func_t func,
		   const void* arg,
		   stg_raytrace_sample_t* sample );

    // this version raytraces from our local origin in the direction [angle]
    void Raytrace( stg_radians_t angle, 
		   stg_meters_t range, 
		   stg_block_match_func_t func,
		   const void* arg,
		   stg_raytrace_sample_t* sample );

    // raytraces from the point and heading identified by pose, in local coords
    void Raytrace( stg_pose_t pose,
		   stg_meters_t range, 
		   stg_radians_t fov, 
		   stg_block_match_func_t func,
		   const void* arg,
		   stg_raytrace_sample_t* samples,
		   uint32_t sample_count );

    // this version raytraces from our local origin in the direction [angle]
    void Raytrace( stg_radians_t angle, 
		   stg_meters_t range, 
		   stg_radians_t fov, 
		   stg_block_match_func_t func,
		   const void* arg,
		   stg_raytrace_sample_t* samples,
		   uint32_t sample_count );

  
    /** Causes this model and its children to recompute their global
	position instead of using a cached pose in
	StgModel::GetGlobalPose()..*/
    void GPoseDirtyTree();

    virtual void Startup();
    virtual void Shutdown();
    virtual void Update();
    virtual void UpdatePose();
    virtual void Draw( uint32_t flags );

    virtual void DrawBlocks();

    // static wrapper for DrawBlocks()
    static void DrawBlocks( gpointer dummykey, 
			    StgModel* mod, 
			    void* arg );
      
    virtual void DrawPicker();
    virtual void DataVisualize();

    virtual void DrawSelected(void);

    void DrawTrailFootprint();
    void DrawTrailBlocks();
    void DrawTrailArrows();
    void DrawGrid();
    void UpdateIfDue();

    /* hooks for attaching special callback functions (not used as
       variables) */
    char startup, shutdown, load, save, update;
    
    ctrlinit_t* initfunc;
    //ctrlupdate_t* updatefunc;

    GList* flag_list;
    
    GPtrArray* blinkenlights;
    void DrawBlinkenlights();

  public:
  
    // constructor
    StgModel( StgWorld* world, StgModel* parent, stg_id_t id, char* typestr );
  
    // destructor
    virtual ~StgModel();

    void Say( char* str );
  
    stg_id_t Id(){ return id; };

    /** configure a model by reading from the current world file */
    virtual void Load();
    /** save the state of the model to the current world file */
    virtual void Save();

    /** Should be called after all models are loaded, to do any last-minute setup */
    void Init();
    
    
    void AddFlag(  StgFlag* flag );
    void RemoveFlag( StgFlag* flag );
    
    void PushFlag( StgFlag* flag );
    StgFlag* PopFlag();

    int GetFlagCount(){ return g_list_length( flag_list ); }

    void DrawFlagList();
    
    void AddBlinkenlight( stg_blinkenlight_t* b )
    {
      g_ptr_array_add( this->blinkenlights, b );
    }
    
    void ClearBlinkenlights()
    {
      g_ptr_array_set_size( this->blinkenlights, 0 );
    }
    
    virtual void PushColor( stg_color_t col )
    { world->PushColor( col ); }
  
    virtual void PushColor( double r, double g, double b, double a )
    { world->PushColor( r,g,b,a ); }
    
    virtual void PopColor(){ world->PopColor(); }

    void Enable(){ disabled = false; };
    void Disable(){ disabled = true; };
    
    // Load a control program for this model from an external library
    void LoadControllerModule( char* lib );

    // call this to ensure the GUI window is redrawn
    void NeedRedraw();

    void AddBlock( stg_point_t* pts, 
		   size_t pt_count, 
		   stg_meters_t zmin, 
		   stg_meters_t zmax,
		   stg_color_t color,
		   bool inherit_color );

    void AddBlockRect( double x, double y, 
		       double width, double height );

    /** remove all blocks from this model, freeing their memory */
    void ClearBlocks();

    const char* TypeStr(){ return this->typestr; }
    StgModel* Parent(){ return this->parent; }
    StgModel* GetModel( const char* name );
    bool Stall(){ return this->stall; }
    int GuiMask(){ return this->gui_mask; };  
    StgWorld* GetWorld(){ return this->world; }

    /// return the root model of the tree containing this model
    StgModel* Root()
    { return(  parent ? parent->Root() : this ); }
  
    bool ObstacleReturn(){ return obstacle_return; }  
    stg_laser_return_t GetLaserReturn(){ return laser_return; }
    int GetRangerReturn(){ return ranger_return; }  
    int FiducialReturn(){ return fiducial_return; }
    int FiducialKey(){ return fiducial_key; }

    bool IsAntecedent( StgModel* testmod );

    // returns true if model [testmod] is a descendent of model [mod]
    bool IsDescendent( StgModel* testmod );

    bool IsRelated( StgModel* mod2 );

    /** get the pose of a model in the global CS */
    stg_pose_t GetGlobalPose();

    /** get the velocity of a model in the global CS */
    stg_velocity_t GetGlobalVelocity();

    /* set the velocity of a model in the global coordinate system */
    void SetGlobalVelocity(  stg_velocity_t gvel );

    /** subscribe to a model's data */
    void Subscribe();

    /** unsubscribe from a model's data */
    void Unsubscribe();

    /** set the pose of model in global coordinates */
    void SetGlobalPose(  stg_pose_t gpose );
  
    /** set a model's velocity in its parent's coordinate system */
    void SetVelocity(  stg_velocity_t vel );
 
    /** set a model's pose in its parent's coordinate system */
    void SetPose(  stg_pose_t pose );

    /** add values to a model's pose in its parent's coordinate system */
    void AddToPose(  stg_pose_t pose );

    /** add values to a model's pose in its parent's coordinate system */
    void AddToPose(  double dx, double dy, double dz, double da );

    /** set a model's geometry (size and center offsets) */
    void SetGeom(  stg_geom_t src );

    /** set a model's geometry (size and center offsets) */
    void SetFiducialReturn(  int fid );

    /** set a model's fiducial key: only fiducial finders with a
	matching key can detect this model as a fiducial. */
    void SetFiducialKey(  int key );
  
    stg_color_t GetColor(){ return color; }
  
    //  stg_laser_return_t GetLaserReturn(){ return laser_return; }
  
    /** Change a model's parent - experimental*/
    int SetParent( StgModel* newparent);
  
    /** Get a model's geometry - it's size and local pose (offset from
	origin in local coords) */
    stg_geom_t GetGeom(){ return geom; }

    /** Get the pose of a model in its parent's coordinate system  */
    stg_pose_t GetPose(){ return pose; }

    /** Get a model's velocity (in its local reference frame) */
    stg_velocity_t GetVelocity(){ return velocity; }

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
    { AddCallback( &startup, cb, user ); };
  
    void RemoveStartupCallback( stg_model_callback_t cb )
    { RemoveCallback( &startup, cb ); };
  
    void AddShutdownCallback( stg_model_callback_t cb, void* user )
    { AddCallback( &shutdown, cb, user ); };
  
    void RemoveShutdownCallback( stg_model_callback_t cb )
    { RemoveCallback( &shutdown, cb ); }
  
    void AddLoadCallback( stg_model_callback_t cb, void* user )
    { AddCallback( &load, cb, user ); }
  
    void RemoveLoadCallback( stg_model_callback_t cb )
    { RemoveCallback( &load, cb ); }
  
    void AddSaveCallback( stg_model_callback_t cb, void* user )
    { AddCallback( &save, cb, user ); }
  
    void RemoveSaveCallback( stg_model_callback_t cb )
    { RemoveCallback( &save, cb ); }
  
    void AddUpdateCallback( stg_model_callback_t cb, void* user )
    { AddCallback( &update, cb, user ); }
  
    void RemoveUpdateCallback( stg_model_callback_t cb )
    { RemoveCallback( &update, cb ); }
  
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
  

    void AddPropertyToggles( void* member, 
			     stg_model_callback_t callback_on,
			     void* arg_on,
			     stg_model_callback_t callback_off,
			     void* arg_off,
			     char* name,
			     char* label,
			     gboolean enabled );  

    virtual void Print( char* prefix );
    virtual const char* PrintWithPose();

    /** Convert a pose in the world coordinate system into a model's
	local coordinate system. Overwrites [pose] with the new
	coordinate. */
    void GlobalToLocal( stg_pose_t* pose );
  
    /** Convert a pose in the model's local coordinate system into the
	world coordinate system. Overwrites [pose] with the new
	coordinate. */
    //void LocalToGlobal( stg_pose_t* pose );
    
    /** Return the global pose (i.e. pose in world coordinates) of a
	pose specified in the model's local coordinate system */
    stg_pose_t LocalToGlobal( stg_pose_t pose );

    /** Return the 3d point in world coordinates of a 3d point
	specified in the model's local coordinate system */
    stg_point3_t LocalToGlobal( stg_point3_t local );
  
    /** returns the first descendent of this model that is unsubscribed
	and has the type indicated by the string */
    StgModel* GetUnsubscribedModelOfType( char* modelstr );
  
    // static wrapper for the constructor - all models must implement
    // this method and add an entry in typetable.cc
    static StgModel* Create( StgWorld* world,
			     StgModel* parent, 
			     stg_id_t id, 
			     char* typestr )
    { 
      return new StgModel( world, parent, id, typestr ); 
    }    

    // iff true, model may output some debugging visualizations and other info
    bool debug;

  };
  
  // BLOCKS
  class StgBlock
  {
  public:
  
    /** Block Constructor. A model's body is a list of these
	blocks. The point data is copied, so pts can safely be freed
	after constructing the block.*/
    StgBlock( StgModel* mod,
	      stg_point_t* pts, 
	      size_t pt_count,
	      stg_meters_t height,
	      stg_meters_t z_offset,
	      stg_color_t color,
	      bool inherit_color );
  
    ~StgBlock();
  
    void Map();
    void UnMap(); // draw the block into the world
  
    void DrawGlobal(); // draw the block in OpenGL using pts_global coords
    void Draw(); // draw the block in OpenGL
    void Draw2D(); // draw the block in OpenGL
    void DrawSolid(); // draw the block in OpenGL as a solid single color
    void DrawFootPrint(); // draw the projection of the block onto the z=0 plane
    
    static void ScaleList( GList* blocks, stg_size_t* size );
    void RecordRenderPoint( GSList** head, GSList* link, unsigned int* c1, unsigned int* c2 );
  
    StgModel* Model(){ return mod; };
  
    stg_point_t* Points( unsigned int *count )
    { if( count ) *count = pt_count; return pts; };	       
  
    bool IntersectGlobalZ( stg_meters_t z )
    { return( z >= global_zmin &&  z < global_zmax ); }
  
    stg_color_t Color()
    { return( inherit_color ? mod->GetColor() : color ); }

  private:
    stg_point_t* pts; //< points defining a polygon
    size_t pt_count; //< the number of points
    stg_meters_t zmin; 
    stg_meters_t zmax; 

    stg_meters_t global_zmin; 
    stg_meters_t global_zmax; 

    StgModel* mod; //< model to which this block belongs

    stg_point_int_t* pts_global_pixels; //< points defining a polygon in global coords
    stg_vertex_t* global_vertices; //< points defining a polygon in global coords
    //GLubyte* colors;
    GLubyte* edge_indices;
    
    stg_color_t color;
    bool inherit_color;  

    GArray* rendered_points;

    inline void DrawTop();
    inline void DrawSides();
  
    inline void PushColor( stg_color_t col )
    { mod->PushColor( col ); }
  
    inline void PushColor( double r, double g, double b, double a )
    { mod->PushColor( r,g,b,a ); }

    inline void PopColor()
    { mod->PopColor(); }
  };

  // COLOR STACK CLASS
  class GlColorStack
  {
  public:
    GlColorStack();
    ~GlColorStack();
  
    void Push( GLdouble col[4] );
    void Push( double r, double g, double b, double a );
    void Push( double r, double g, double b );
    void Push( stg_color_t col );
  
    void Pop();
  
    unsigned int Length()
    { return g_queue_get_length( colorstack ); }

  private:
    GQueue* colorstack;
  };


/** Implements a FLTK / OpenGL graphical user interfae.      
*/
  class StgCanvas : public Fl_Gl_Window
  {
    friend class StgWorldGui; // allow access to private members

  private:
    GlColorStack colorstack;
    double scale;
    double panx, pany, stheta, sphi;
    int startx, starty;
    bool dragging;
    bool rotating;
    GList* selected_models; ///< a list of models that are currently
    ///selected by the user
    StgModel* last_selection; ///< the most recently selected model
    ///(even if it is now unselected).
    uint32_t showflags;
    stg_msec_t interval; // window refresh interval in ms

    GList* ray_list;
    void RecordRay( double x1, double y1, double x2, double y2 );
    void DrawRays();
    void ClearRays();
    void DrawGlobalGrid();

  public:
    StgCanvas( StgWorld* world, int x, int y, int W,int H);
    ~StgCanvas();
  
    bool graphics;
    StgWorld* world;

    void FixViewport(int W,int H);
    virtual void draw();
    virtual int handle( int event );
    void resize(int X,int Y,int W,int H);

    void CanvasToWorld( int px, int py, 
			double *wx, double *wy, double* wz );

    StgModel* Select( int x, int y );
  
    inline void PushColor( stg_color_t col )
    { colorstack.Push( col ); } 
  
    void PushColor( double r, double g, double b, double a )
    { colorstack.Push( r,g,b,a ); }
  
    void PopColor(){ colorstack.Pop(); } 
  
    void InvertView( uint32_t invertflags );

    uint32_t GetShowFlags(){ return showflags; }

    void SetShowFlags( uint32_t flags ){ showflags = flags; }

    static void TimerCallback( StgCanvas* canvas );
  };

  /** Extends StgWorld to present an OpenGL-based GUI to the user */
  class StgWorldGui : public StgWorld, public Fl_Window 
  {
    friend class StgCanvas;
  
  private:
    int wf_section;
    StgCanvas* canvas;
    Fl_Menu_Bar* mbar;
    //StgBlockGrid* GetBlockGrid(){ return bgridx; };

  public:
    StgWorldGui(int W,int H,const char*L=0);
    ~StgWorldGui();

    // overload inherited methods
    virtual bool RealTimeUpdate();
    virtual bool Update();

    virtual void Load( const char* filename );
    virtual void Save();
  
    // static callback functions
    static void SaveCallback( Fl_Widget* wid, StgWorldGui* world );
  
    virtual void PushColor( stg_color_t col )
    { canvas->PushColor( col ); } 
  
    virtual void PushColor( double r, double g, double b, double a )
    { canvas->PushColor( r,g,b,a ); }
  
    virtual void PopColor()
    { canvas->PopColor(); } 

    void DrawTree( bool leaves );
    void DrawFloor();
  };



  // end doc group libstage_utilities


  // BLOBFINDER MODEL --------------------------------------------------------
  
  /** blobfinder data packet 
   */
  typedef struct
  {
    stg_color_t color;
    uint32_t left, top, right, bottom;
    stg_meters_t range;
  } stg_blobfinder_blob_t;


  class StgModelBlobfinder : public StgModel
  {
  private:
  
    GArray* colors;
    GArray* blobs;
  
    // predicate for ray tracing
    static bool BlockMatcher( StgBlock* testblock, StgModel* finder );
  
  public:
  
    unsigned int scan_width;
    unsigned int scan_height;
    stg_meters_t range;
    stg_radians_t fov;
    stg_radians_t pan;

    // constructor
    StgModelBlobfinder( StgWorld* world,
			StgModel* parent, 
			stg_id_t id, 
			char* typestr);
  
    // destructor
    ~StgModelBlobfinder();
    
    virtual void Startup();
    virtual void Shutdown();
    virtual void Update();
    virtual void Load();
    virtual void DataVisualize();

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

    // static wrapper for the constructor - all models must implement
    // this method and add an entry in typetable.cc
    static StgModel* Create( StgWorld* world,
			     StgModel* parent, 
			     stg_id_t id, 
			     char* typestr )
    { 
      return (StgModel*)new StgModelBlobfinder( world, parent, id, typestr ); 
    }    
  };
  
  // ENERGY model --------------------------------------------------------------
  
  /** energy data packet */
  typedef struct
  {
    /** estimate of current energy stored */
    stg_joules_t stored;

    /** TRUE iff the device is receiving energy from a charger */
    stg_bool_t charging;

    /** diatance to charging device */
    stg_meters_t range;

    /** an array of pointers to connected models */
    GPtrArray* connections;
  } stg_energy_data_t;

  /** energy config packet (use this to set or get energy configuration)*/
  typedef struct
  {
    /** maximum storage capacity */
    stg_joules_t capacity;

    /** When charging another device, supply this many Joules/sec at most*/
    stg_watts_t give_rate;

    /** When charging from another device, receive this many Joules/sec at most*/
    stg_watts_t take_rate;

    /** length of the charging probe */
    stg_meters_t probe_range;
    
    /**  iff TRUE, this device will supply power to connected devices */
    stg_bool_t give;

  } stg_energy_config_t;

  // there is currently no energy command packet

  
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
    stg_range_bounds_t range_bounds;
    stg_radians_t fov;
  } stg_laser_cfg_t;

  class StgModelLaser : public StgModel
  {
  private:
    int dl_debug_laser;
  
    stg_laser_sample_t* samples;
    uint32_t sample_count;
    stg_meters_t range_min, range_max;
    stg_radians_t fov;
    uint32_t resolution;
  
  public:
    // constructor
    StgModelLaser( StgWorld* world,
		   StgModel* parent, 
		   stg_id_t id, 
		   char* typestr );
  
    // destructor
    ~StgModelLaser();
  
    virtual void Startup();
    virtual void Shutdown();
    virtual void Update();
    virtual void Load();  
    virtual void Print( char* prefix );
    virtual void DataVisualize();
  
    uint32_t GetSampleCount(){ return sample_count; }
    
    stg_laser_sample_t* GetSamples( uint32_t* count=NULL);
    
    void SetSamples( stg_laser_sample_t* samples, uint32_t count);
    
    // Get the user-tweakable configuration of the laser
    stg_laser_cfg_t GetConfig( );    

    // Set the user-tweakable configuration of the laser
    void SetConfig( stg_laser_cfg_t cfg );
  
  
    // static wrapper for the constructor - all models must implement
    // this method and add an entry in typetable.cc
    static StgModel* Create( StgWorld* world,
			     StgModel* parent, 
			     stg_id_t id, 
			     char* typestr )
    { 
      return (StgModel*)new StgModelLaser( world, parent, id, typestr ); 
    }    
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
//     stg_size_t paddle_size; ///< paddle dimensions 

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
//     stg_pose_t pose;
//     stg_meters_t length;
//   } stg_bumper_config_t;
  
//   typedef struct
//   {
//     StgModel* hit;
//     stg_point_t hit_point;
//   } stg_bumper_sample_t;


  // FIDUCIAL MODEL --------------------------------------------------------
  
  /** fiducial config packet 
   */
  typedef struct
  {
    stg_meters_t max_range_anon; //< maximum detection range
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
    stg_pose_t geom; ///< size and relative angle of the target
    stg_pose_t pose; ///< Absolute accurate position of the target in world coordinates (it's cheating to use this in robot controllers!)
    int id; ///< the identifier of the target, or -1 if none can be detected.
    
  } stg_fiducial_t;

  class StgModelFiducial : public StgModel
  {
  private:
    // if neighbor is visible, add him to the fiducial scan
    void AddModelIfVisible( StgModel* him );
  
    // static wrapper function can be used as a function pointer
    static void AddModelIfVisibleStatic( gpointer key, 
					 StgModel* him, 
					 StgModelFiducial* me )
    { if( him != me ) me->AddModelIfVisible( him ); };

    virtual void Update();
    virtual void DataVisualize();
  
    GArray* data;

  public:
    // constructor
    StgModelFiducial( StgWorld* world,
		      StgModel* parent, 
		      stg_id_t id, 
		      char* typestr );
  
    // destructor
    virtual ~StgModelFiducial();
  
    virtual void Load();
  
    stg_meters_t max_range_anon; //< maximum detection range
    stg_meters_t max_range_id; ///< maximum range at which the ID can be read
    stg_meters_t min_range; ///< minimum detection range
    stg_radians_t fov; ///< field of view 
    stg_radians_t heading; ///< center of field of view
    int key; ///< /// only detect fiducials with a key that matches this one (defaults 0)
  
    stg_fiducial_t* fiducials; ///< array of detected fiducials
    uint32_t fiducial_count; ///< the number of fiducials detected
  
    // static wrapper for the constructor - all models must implement
    // this method and add an entry in typetable.cc
    static StgModel* Create( StgWorld* world,
			     StgModel* parent, 
			     stg_id_t id, 
			     char* typestr )
    { 
      return (StgModel*)new StgModelFiducial( world, parent, id, typestr ); 
    }    
  };


  // RANGER MODEL --------------------------------------------------------

  typedef struct
  {
    stg_pose_t pose;
    stg_size_t size;
    stg_bounds_t bounds_range;
    stg_radians_t fov;
    int ray_count;
  } stg_ranger_sensor_t;

  class StgModelRanger : public StgModel
  {
  protected:
  
    virtual void Startup();
    virtual void Shutdown();
    virtual void Update();
    virtual void DataVisualize();
  
  public:
    // constructor
    StgModelRanger( StgWorld* world,
		    StgModel* parent, 
		    stg_id_t id, 
		    char* typestr );
  
    // destructor
    virtual ~StgModelRanger();
  
    virtual void Load();
    virtual void Print( char* prefix );
  
    uint32_t sensor_count;
    stg_ranger_sensor_t* sensors;
    stg_meters_t* samples;
  
    // static wrapper for the constructor - all models must implement
    // this method and add an entry in typetable.cc
    static StgModel* Create( StgWorld* world,
			     StgModel* parent, 
			     stg_id_t id, 
			     char* typestr )
    { 
      return (StgModel*)new StgModelRanger( world, parent, id, typestr ); 
    }    
  };

  // BLINKENLIGHT MODEL ----------------------------------------------------
  class StgModelBlinkenlight : public StgModel
  {
  private:
    double dutycycle;
    bool enabled;
    double period;
    bool on;

  public:

    StgModelBlinkenlight( StgWorld* world,
			  StgModel* parent, 
			  stg_id_t id, 
			  char* typestr );

    ~StgModelBlinkenlight();

    virtual void Load();
    virtual void Update();
    virtual void Draw( uint32_t flags );

    // static wrapper for the constructor - all models must implement
    // this method and add an entry in typetable.cc
    static StgModel* Create( StgWorld* world,
			     StgModel* parent, 
			     stg_id_t id, 
			     char* typestr )
    { 
      return (StgModel*)new StgModelBlinkenlight( world, parent, id, typestr ); 
    }    
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
  
  
  class StgModelPosition : public StgModel
  {
  private:
    stg_pose_t goal; //< the current velocity or pose to reach, depending on the value of control_mode
    stg_position_control_mode_t control_mode; 
    stg_position_drive_mode_t drive_mode;
    stg_position_localization_mode_t localization_mode; ///< global or local mode
    stg_velocity_t integration_error; ///< errors to apply in simple odometry model
    stg_velocity_t integration_bias; ///< errors to apply in simple odometry model
    
  public:
    // constructor
    StgModelPosition( StgWorld* world,
		      StgModel* parent, 
		      stg_id_t id, 
		      char* typestr);
  
    // destructor
    ~StgModelPosition();
    
    virtual void Startup();
    virtual void Shutdown();
    virtual void Update();
    virtual void Load();

    /** Set the current pose estimate.*/
    void SetOdom( stg_pose_t odom );
  
    /** Sets the control_mode to STG_POSITION_CONTROL_VELOCITY and sets
	the goal velocity. */
    void SetSpeed( double x, double y, double a ); 
    void SetXSpeed( double x ); 
    void SetYSpeed( double y ); 
    void SetZSpeed( double z ); 
    void SetTurnSpeed( double a ); 
    void SetSpeed( stg_velocity_t vel );

    /** Sets the control mode to STG_POSITION_CONTROL_POSITION and sets
	the goal pose */
    void GoTo( double x, double y, double a );
    void GoTo( stg_pose_t pose );
  
    // localization state
    stg_pose_t est_pose; ///< position estimate in local coordinates
    stg_pose_t est_pose_error; ///< estimated error in position estimate
    stg_pose_t est_origin; ///< global origin of the local coordinate system

    // static wrapper for the constructor - all models must implement
    // this method and add an entry in typetable.cc
    static StgModel* Create( StgWorld* world,
			     StgModel* parent, 
			     stg_id_t id, 
			     char* typestr )
    { 
      return (StgModel*)new StgModelPosition( world, parent, id, typestr ); 
    }    

  };


}; // end namespace stg

/*@}*/

#endif
