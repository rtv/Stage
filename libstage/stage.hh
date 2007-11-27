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
 * CVS: $Id: stage.hh,v 1.1.2.18 2007-11-27 05:36:01 rtv Exp $
 */

/*! \file stage.h 
  Stage library header file

  This header file contains the external interface for the Stage
  library
*/

#include <unistd.h>
#include <stdint.h> // for portable int types eg. uint32_t
#include <sys/types.h>
#include <sys/time.h>
#include <glib.h> // we use GLib's data structures extensively
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <glib.h>

#ifdef __APPLE__
#include <OpenGl/gl.h>
#include <OpenGl/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

// todo: consider moving these out
#include "worldfile.hh"
#include "colors.h"

#define STG_STRING_AUTHORS {			\
    "Richard Vaughan",				\
      "Brian Gerkey",				\
      "Andrew Howard",				\
      "Reed Hedges",				\
      "Pooya Karimian",				\
      "and contributors",                       \
      NULL }
  
#define STG_STRING_COPYRIGHT "Copyright the Authors 2000-2007"
#define STG_STRING_WEBSITE "http://playerstage.org"
#define STG_STRING_DESCRIPTION "Robot simulation library\nPart of the Player Project"
#define STG_STRING_LICENSE "Stage robot simulation library\n"\
    "Copyright (C) 2000-2007 Richard Vaughan\n"\
    "Part of the Player Project [http://playerstage.org]\n"\
    "\n"\
    "This program is free software; you can redistribute it and/or\n"\
    "modify it under the terms of the GNU General Public License\n"\
    "as published by the Free Software Foundation; either version 2\n"\
    "of the License, or (at your option) any later version.\n"\
    "\n"\
    "This program is distributed in the hope that it will be useful,\n"\
    "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"\
    "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"\
    "GNU General Public License for more details.\n"\
    "\n"\
    "You should have received a copy of the GNU General Public License\n"\
    "along with this program; if not, write to the Free Software\n"\
    "Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.\n"\
    "\n"\
    "The text of the license may also be available online at\n"\
    "http://www.gnu.org/licenses/old-licenses/gpl-2.0.html\n"

/** Returns a string indentifying the package version number, e.g. "3.0.0" */
const char* stg_version_string( void );
const char* stg_package_string( void );


//#ifdef __cplusplus
//extern "C" {
//#endif 

/** @defgroup libstage libstage API reference

    libstage (The Stage Library) provides a C code library for
    simulating a population of mobile robots and sensors. It is
    usually used as a plugin driver for <a
    href="http://playerstage.sf.net/player/player.html">Player</a>,
    but it can also be used directly to build custom simulations.

    libstage is modular and fairly simple to use. The following code is
    enough to get a complete robot simulation running:

@verbatim
#include "stage.h"

int main( int argc, char* argv[] )
{ 
  stg_world_t* world = stg_world_create_from_file( argv[1] );
  
  while( (stg_world_update( world,TRUE )==0) )
    {}
  
  stg_world_destroy( world );
  
  return 0;
}
@endverbatim

@par Contact and support

For help with libstage, please use the mailing list playerstage_users@lists.sourceforge.net. 
@{
*/

// forward declare
  class StgModel;

typedef enum {
  STG_IMAGE_FORMAT_PNG,
  STG_IMAGE_FORMAT_JPG
} stg_image_format_t;

/** any integer value other than this is a valid fiducial ID 
 */
#define FiducialNone 0

/** Limit the length of the character strings that identify models */
#define STG_TOKEN_MAX 64

/** @defgroup types Measurement Types
    Basic self-describing measurement types. All packets with real
    measurements are specified in these terms so changing types here
    should work throughout the code.
  @{
*/    

/** uniquely identify a model */
typedef int stg_id_t;

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
typedef int stg_bool_t;

//typedef double stg_friction_t;

/** 32-bit ARGB color packed 0xAARRGGBB */
typedef uint32_t stg_color_t;

stg_color_t stg_color_pack( double r, double g, double b, double a );
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
  
  /** specify a rectangular size 
   */
  typedef struct 
  {
    stg_meters_t x, y, z;
  } stg_size_t;
  
  /** \struct stg_pose_t
      Specify a 3 axis position, in x, y and heading.
   */
  typedef struct
  {
    stg_meters_t x, y, z;
    stg_radians_t a; // rotation about the z axis only
  } stg_pose_t;
  
  /** specify a 3 axis velocity in x, y and heading.
   */
  typedef stg_pose_t stg_velocity_t;  

  /** specify an object's basic geometry: position and rectangular
      size
   */
  typedef struct
  {
    stg_pose_t pose;
    stg_size_t size;
  } stg_geom_t;
  
  /** bound a range of values, from min to max */
  typedef struct
  {
    double min; //< smallest value in range
    double max; //< largest value in range
  } stg_bounds_t;
  
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

  /** A handy triple for PTZ angles */
  typedef struct 
  {
    stg_radians_t pan;
    stg_radians_t tilt;
    stg_radians_t zoom;
  } stg_ptz_t;

  /*@}*/ // end types


/** Returns the real (wall-clock) time in milliseconds since the
    epoch. */
//stg_msec_t stg_realtime( void );

/** Returns the real (wall-clock) time in milliseconds since the
    simulation started. */
//stg_msec_t stg_realtime_since_start( void );
  

  /// iff stage wants to quit, this will return true, else false
bool stg_quit_test( void );

  /** set stage's quit flag. Stage will quit cleanly very soon after
      this function is called. */
  void stg_quit_request( void );



/** take binary sign of a, either -1, or 1 if >= 0 */
#define SGN(a)		(((a)<0) ? -1 : 1)
/** matching function should return 0 iff the candidate block is
    acceptable */
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
  
  /** Get a string identifying the version of stage. The string is
      generated by autoconf 
  */
  //const char* stg_version_string( void );


  // POINTS ---------------------------------------------------------
  /** @ingroup libstage
      @defgroup stg_points Points
      Defines a point on the 2D plane.
      @{ 
  */

  /** define a point on the plane */
  typedef struct
  {
    stg_meters_t x, y;
  } stg_point_t;


  /** define an integer point on the plane */
  typedef struct
  {
    uint32_t x,y;
  } stg_point_int_t;
  
  /// Define a point in 3D space
  typedef struct
  {
    double x,y,z;
  } stg_point_3d_t;

  /** define an integer point in 3D space */
  // typedef struct
//   {
//     uint32_t x,y,z;
//   } stg_point_3d_int_t;

  
  /** Create an array of [count] points. Caller must free the returned
      pointer, preferably with stg_points_destroy(). */
  stg_point_t* stg_points_create( size_t count );

  /** frees a point array */
  void stg_points_destroy( stg_point_t* pts );

  /** create an array of 4 points containing the corners of a unit
      square.*/
  stg_point_t* stg_unit_square_points_create( void );
  /*@}*/


  //  MODEL --------------------------------------------------------
    
  // group the docs of all the model types
  /** @ingroup libstage
      @defgroup stg_model Models
      Implements the basic object
      @{ 
  */
  
  // Movement masks for figures
#define STG_MOVE_TRANS (1 << 0)
#define STG_MOVE_ROT   (1 << 1)
#define STG_MOVE_SCALE (1 << 2)
  
  typedef int stg_movemask_t;



#define STG_MP_PREFIX          "_mp_"

#define STG_MP_POSE                     "_mp_pose"
#define STG_MP_VELOCITY                 "_mp_velocity"
#define STG_MP_GEOM                     "_mp_geom"
#define STG_MP_COLOR                    "_mp_color"
#define STG_MP_WATTS                    "_mp_watts"
#define STG_MP_FIDUCIAL_RETURN          "_mp_fiducial_return"
#define STG_MP_LASER_RETURN             "_mp_laser_return"
#define STG_MP_OBSTACLE_RETURN          "_mp_obstacle_return"
#define STG_MP_RANGER_RETURN            "_mp_ranger_return"
#define STG_MP_GRIPPER_RETURN           "_mp_gripper_return"
#define STG_MP_MASS                     "_mp_mass"
  
  /* TODO - complete the set of named properties */

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

  /// laser return value
  typedef enum 
    {
      LaserTransparent, ///<not detected by laser model 
      LaserVisible, ///< detected by laser with a reflected intensity of 0 
      LaserBright  ////< detected by laser with a reflected intensity of 1 
    } stg_laser_return_t;


  
typedef struct {
  double x, y, z;
} stg_point3_t;

typedef stg_point3_t stg_vertex_t; // same thing

typedef struct {
  stg_point3_t min, max;
} stg_bounds3_t;

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
  } stg_d_type_t;
  
  /** the start of all stg_d structures looks like this */
  typedef struct stg_d_hdr
  {
    stg_d_type_t type;
  } stg_d_hdr_t;

  /** push is just the header, but we define it for syntax checking */
  typedef stg_d_hdr_t stg_d_push_t;
  /** pop is just the header, but we define it for syntax checking */
  typedef stg_d_hdr_t stg_d_pop_t;
  
  /** stg_d draw command */
  typedef struct stg_d_draw
  {
    /** required type field */
    stg_d_type_t type;
    /** number of vertices */
    size_t vert_count;
    /** array of vertex data follows at the end of this struct*/
    stg_vertex_t verts[]; 
  } stg_d_draw_t;
  
  /** stg_d translate command */
  typedef struct stg_d_translate
  {
    /** required type field */
    stg_d_type_t type; 
    /** distance to translate, in 3 axes */
    stg_point3_t vector;
  } stg_d_translate_t;
  
  /** stg_d rotate command */
  typedef struct stg_d_rotate
  {
    /** required type field */
    stg_d_type_t type; 
    /** vector about which we should rotate */
    stg_point3_t vector;
    /** angle to rotate in radians */
    double angle;
  } stg_d_rotate_t;

void stg_d_render( stg_d_draw_t* d );



// MACROS ------------------------------------------------------
// Some useful macros
  
/** @ingroup libstage
    @defgroup libstage_util Utilities
    Various useful macros and functions that don't belong anywhere else.
    @{
*/
  
  /** Look up the color in the X11 database.  (i.e. transform color
      name to color value).  If the color is not found in the
      database, a bright red color (0xF00) will be returned instead.
  */
  stg_color_t stg_lookup_color(const char *name);

  /** calculate the sum of [p1] and [p2], in [p1]'s coordinate system, and
      copy the result into result. */
  void stg_pose_sum( stg_pose_t* result, stg_pose_t* p1, stg_pose_t* p2 );

  // PRETTY PRINTING -------------------------------------------------
  
  /** Report an error, with a standard, friendly message header */
  void stg_print_err( const char* err );
  /** Print human-readable geometry on stdout */
  void stg_print_geom( stg_geom_t* geom );
  /** Print human-readable pose on stdout */
  void stg_print_pose( stg_pose_t* pose );
  /** Print human-readable velocity on stdout */
  void stg_print_velocity( stg_velocity_t* vel );
  /** Print human-readable version of the gripper config struct */
  //void stg_print_gripper_config( stg_gripper_config_t* slc );
  /** Print human-readable version of the laser config struct */
  //void stg_print_laser_config( stg_laser_config_t* slc );
  


/** @ingroup libstage_util
    @defgroup floatcomparison Floating point comparisons

 Macros for comparing floating point numbers. It's a troublesome
 limitation of C and C++ that floating point comparisons are not very
 accurate. These macros multiply their arguments by a large number
 before comparing them, to improve resolution.

  @{
*/

/** Precision of comparison. The number of zeros to the left of the
   decimal point determines the accuracy of the comparison in decimal
   places to the right of the point. E.g. precision of 100000.0 gives
   a comparison precision of within 0.000001 */
#define PRECISION 100000.0

/** TRUE iff A and B are equal to within PRECISION */
#define EQ(A,B) ((lrint(A*PRECISION))==(lrint(B*PRECISION)))

/** TRUE iff A is less than B, subject to PRECISION */
#define LT(A,B) ((lrint(A*PRECISION))<(lrint(B*PRECISION)))

/** TRUE iff A is greater than B, subject to PRECISION */
#define GT(A,B) ((lrint(A*PRECISION))>(lrint(B*PRECISION)))

/** TRUE iff A is greater than or equal B, subject to PRECISION */
#define GTE(A,B) ((lrint(A*PRECISION))>=(lrint(B*PRECISION)))

/** TRUE iff A is less than or equal to B, subject to PRECISION */
#define LTE(A,B) ((lrint(A*PRECISION))<=(lrint(B*PRECISION)))

#define STG_SHOW_BLOCKS       1
#define STG_SHOW_DATA         2
#define STG_SHOW_GEOM         4
#define STG_SHOW_GRID         8
#define STG_SHOW_OCCUPANCY    16
#define STG_SHOW_TRAILS       32
#define STG_SHOW_FOLLOW       64
#define STG_SHOW_CLOCK        128
#define STG_SHOW_QUADTREE     256

// STAGE INTERNAL

// forward declare
class StgWorld;
class StgModel;

/** @ingroup libstage
    @defgroup libstage_internal Internals

    These are internal docs. Don't use these functions in user
    code. Let us know if there is anything useful here that should be
    exposed in the external interface.

    @{ 
*/



#define STG_DEFAULT_WINDOW_WIDTH 800
#define STG_DEFAULT_WINDOW_HEIGHT 840

GHashTable* stg_create_typetable( void );

typedef struct 
{
  const char* token;
  StgModel* (*creator_fn)( StgWorld*, StgModel*, stg_id_t, char* );
} stg_typetable_entry_t;


void gl_pose_shift( stg_pose_t* pose );
void gl_coord_shift( double x, double y, double z, double a  );

/** Render a string at [x,y,z] in the current color */
 void gl_draw_string( float x, float y, float z, char *string);


void stg_block_list_scale( GList* blocks, 
			   stg_size_t* size );
void stg_block_list_destroy( GList* list );
void gui_world_set_title( StgWorld* world, char* txt );

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


// ROTATED RECTANGLES -------------------------------------------------

/** @ingroup libstage_internal
    @defgroup rotrect Rotated Rectangles
    @{ 
*/

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

/**@}*/

// /** matching function should return 0 iff the candidate block is
//     acceptable */
class StgBlock;

typedef bool (*stg_block_match_func_t)(StgBlock* candidate, const void* arg );


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

// ANCESTOR CLASS

  /** Define a callback function type that can be attached to a
      record within a model and called whenever the record is set.*/
  typedef int (*stg_model_callback_t)( StgModel* mod, void* user );
  
  
/// Base class for StgModel and StgWorld
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
  void SetNumChildrenOfType( const char* typestr, unsigned int count );
  
  virtual void AddChild( StgModel* mod );
  virtual void RemoveChild( StgModel* mod );
  virtual void GetGlobalPose( stg_pose_t* gpose );  
  
  const char* Token( void )
  { return this->token; }
  
  // PURE VIRTUAL - descendents must implement
  virtual void PushColor( stg_color_t col ) = 0;
  virtual void PushColor( double r, double g, double b, double a ) = 0;
  virtual void PopColor() = 0; // does nothing
};


typedef struct
{
  uint32_t counter;
  GSList** lists;
} stg_bigblock_t;

class StgBlockGrid 
{
private:
  stg_bigblock_t* map;
  
  GTrashStack* trashstack;  
  uint32_t width, height, bwidth, bheight;

public:
  uint32_t numbits;
  StgBlockGrid( uint32_t width, uint32_t height );
  ~StgBlockGrid();
  void AddBlock( uint32_t x, uint32_t y, StgBlock* block );
  void RemoveBlock( uint32_t x, uint32_t y, StgBlock* block );
  GSList* GetList( uint32_t x, uint32_t y );
  void RemoveBlock( StgBlock* block );
  void Draw( bool drawall );
  
  /** Returns the number of blocks occupying the big block, specified
      in big block coordinates, NOT in small blocks.*/
  uint32_t BigBlockOccupancy( uint32_t bbx, uint32_t bby );
};
  



const uint32_t INTERVAL_LOG_LEN = 32;

/// WORLD CLASS
class StgWorld : public StgAncestor
{
protected:
  stg_usec_t real_time_next_update;
  stg_usec_t real_time_start;
  static bool init_done;


  bool quit; // quit this world ASAP
  static bool quit_all; // quit all worlds ASAP


private:
  
  static unsigned int next_id; //< initialized to zero, used to
			       //allocate unique sequential world ids
  static GHashTable* typetable;  
  
  inline void MetersToPixels( stg_meters_t mx, stg_meters_t my, 
			      int32_t *px, int32_t *py );
  
public:

  StgWorld( void );
  
  StgWorld( const char* token, 
	    stg_msec_t interval_sim, 
	    stg_msec_t interval_real,
	    double ppm, 
	    double width,
	    double height );
  
  static void Init( int* argc, char** argv[] );
  
  void Initialize( const char* token, 
		   stg_msec_t interval_sim, 
		   stg_msec_t interval_real,
		   double ppm, 
		   double width,
		   double height );
  
  virtual ~StgWorld( void );
  
  bool graphics;

  virtual void PushColor( stg_color_t col ) { /* do nothing */  };
  virtual void PushColor( double r, double g, double b, double a ) { /* do nothing */  };
  virtual void PopColor(){ /* do nothing */  };

  stg_usec_t real_time_now;
  stg_usec_t RealTimeNow(void);
  stg_usec_t RealTimeSinceStart(void);
  void PauseUntilNextUpdateTime(void);

  virtual void Load( const char* worldfile_path );
  //virtual void Load( void );
  virtual void Reload( void );
  virtual void Save( void );
  virtual bool Update(void);
  virtual bool RealTimeUpdate(void);
  
  void Quit(){ this->quit = true; }

  void Start();
  void Stop();

  bool destroy;

  stg_id_t id;
  stg_meters_t width, height;

  GHashTable* models_by_id; ///< the models that make up the world, indexed by id
  GHashTable* models_by_name; ///< the models that make up the world, indexed by name
  GList* velocity_list; ///< a list of models that have non-zero velocity, for efficient updating
  CWorldFile* wf; ///< If set, points to the worldfile used to create this world

  stg_usec_t sim_time; ///< the current sim time in this world in ms
  stg_usec_t wall_last_update; ///< the real time of the last update in ms

  long unsigned int updates; ///< the number of simulated time steps executed so far
  
  bool dirty; ///< iff true, a gui redraw would be required

  stg_usec_t interval_real;   ///< real-time interval between updates - set this to zero for 'as fast as possible
  stg_usec_t interval_sleep_max;
  stg_usec_t interval_sim; ///< temporal resolution: milliseconds that elapse between simulated time steps 

  stg_usec_t interval_log[INTERVAL_LOG_LEN];
  
  int total_subs; ///< the total number of subscriptions to all models
  double ppm; ///< the resolution of the world model in pixels per meter  

  bool paused; ///< the world only updates when this is false

  GList* update_list; //< the descendants that need Update() called
  
  StgModel* GetModel( const stg_id_t id );
  StgModel* GetModel( const char* name );
  
  virtual void AddModel( StgModel* mod );
  virtual void RemoveModel( StgModel* mod );
  void AddModelName( StgModel* mod );  

  void StartUpdatingModel( StgModel* mod );
  void StopUpdatingModel( StgModel* mod );
  
  // the matrix used to be a separate thing: it has been folded into the world
  
  // A quad tree of cells. Each leaf node contains a list of
  // pointers to objects located at that cell
  //StgCell* root;

  StgBlockGrid* bgrid;

  //uint16_t* sparsetable;

  void MapBlock( StgBlock* block );
  
  stg_meters_t Raytrace( StgModel* finder,
			 stg_pose_t* pose, 			 
			 stg_meters_t max_range,
			 stg_block_match_func_t func,
			 const void* arg,
			 StgModel** hit_model );

  void ClockString( char* str, size_t maxlen );

 private:
  // todo - hide some stuff here?

};

// MODEL CLASS
class StgModel : public StgAncestor
{
public:
  
  // constructor
  StgModel( StgWorld* world, StgModel* parent, int id, char* typestr );
  
  // destructor
  virtual ~StgModel( void );
  
  stg_id_t id; // globally unique ID used as hash table key and
	       // worldfile section identifier

  StgWorld* world; // pointer to the world in which this model exists

  const char* typestr;
  stg_pose_t pose;
  stg_velocity_t velocity;    
  stg_watts_t watts; //< power consumed by this model
  stg_color_t color;
  stg_kg_t mass;
  stg_geom_t geom;
  int laser_return;
  int obstacle_return;
  int blob_return;
  int gripper_return;
  int ranger_return;
  int fiducial_return;
  int fiducial_key;
  int boundary;
  stg_meters_t map_resolution;
  stg_bool_t stall;
  
  char* say_string; // if non-null, this string is displayed in the GUI

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
  
  // display lists
  //int dl_body, dl_data, dl_cmd, dl_cfg, dl_grid, dl_debug, dl_raytrace;
  //GList* raytrace_dl_list;

  bool body_dirty; //< iff true, regenerate block display list before redraw
  bool data_dirty; //< iff true, regenerate data display list before redraw

  stg_pose_t global_pose;

  bool gpose_dirty; //< set this to indicate that global pose may have
		    //changed

  bool data_fresh; ///< this can be set to indicate that the model has
		   ///new data that may be of interest to users. This
		   ///allows polling the model instead of adding a
		   ///data callback.  
public:
  // end experimental    

  //virtual int Initialize( void ); //< post-construction initialiser
  virtual void Startup( void );
  virtual void Shutdown( void );
  virtual void Update( void );
  virtual void UpdatePose( void );

  void Say( char* str );
    void UpdateIfDue( void );

  /** configure a model by reading from the current world file */
  virtual void Load( void );
  /** save the state of the model to the current world file */
  virtual void Save( void );
  virtual void Draw( uint32_t flags );
  virtual void DrawPicker( void );
  virtual void DataVisualize( void );
  void DrawGrid();
  virtual void DrawSelected(void);

  virtual void PushColor( stg_color_t col )
  { world->PushColor( col ); }
  
  virtual void PushColor( double r, double g, double b, double a )
  { world->PushColor( r,g,b,a ); }
    
  virtual void PopColor()
  { world->PopColor(); }
      
  // call this to ensure the GUI window is redrawn
  void NeedRedraw( void );

  void AddBlock( stg_point_t* pts, 
		 size_t pt_count, 
		 stg_meters_t zmin, 
		 stg_meters_t zmax,
		 stg_color_t color,
		 bool inherit_color );

  void AddBlockRect( double x, double y, 
		     double width, double height );

  /** remove all blocks from this model, freeing their memory */
  void ClearBlocks( void );

  const char* TypeStr( void )
  { return this->typestr; }

  StgModel* Parent( void )
  { return this->parent; }

  bool Stall( void )
  { return this->stall; }

  int GuiMask( void )
  { return this->gui_mask; };
  
  /// synonym for Root(); TODO: get rid of this
  StgWorld* World( void )
  { return this->world; }

  /// return the root model of the tree containing this model
  StgModel* Root( void )
  { return(  this->parent ? this->parent->Root() : this ); }
  
  bool ObstacleReturn( void )
  { return this->obstacle_return; }
  
  int LaserReturn( void )
  { return this->laser_return; }

  int RangerReturn( void )
  { return this->ranger_return; }
  

  StgModel* TestCollision( stg_pose_t* pose, 
			   double* hitx, double* hity );

  void Map();
  void UnMap();

  void MapWithChildren();
  void UnMapWithChildren();

  int TreeToPtrArray( GPtrArray* array );

  bool IsAntecedent( StgModel* testmod );

  // returns true if model [testmod] is a descendent of model [mod]
  bool IsDescendent( StgModel* testmod );

  bool IsRelated( StgModel* mod2 );

    /** get the pose of a model in the global CS */
  void GetGlobalPose(  stg_pose_t* pose );

  /** get the velocity of a model in the global CS */
  void GetGlobalVelocity(  stg_velocity_t* gvel );

  /* set the velocity of a model in the global coordinate system */
  void SetGlobalVelocity(  stg_velocity_t* gvel );

  /** subscribe to a model's data */
  void Subscribe( void );

  /** unsubscribe from a model's data */
  void Unsubscribe( void );

  /** set the pose of model in global coordinates */
  void SetGlobalPose(  stg_pose_t* gpose );
  
  /** When called, this causes this model and its children to
      recompute their global position instead of using a cached pose
      in StgModel::GetGlobalPose()..*/
  void GPoseDirtyTree( void );

  /** set a model's velocity in its parent's coordinate system */
  void SetVelocity(  stg_velocity_t* vel );
 
  /** set a model's pose in its parent's coordinate system */
  void SetPose(  stg_pose_t* pose );

  /** add values to a model's pose in its parent's coordinate system */
  void AddToPose(  stg_pose_t* pose );

  /** add values to a model's pose in its parent's coordinate system */
  void AddToPose(  double dy, double dy, double dz, double da );

  /** set a model's geometry (size and center offsets) */
  void SetGeom(  stg_geom_t* src );

  /** set a model's geometry (size and center offsets) */
  void SetFiducialReturn(  int fid );

  /** set a model's fiducial key: only fiducial finders with a
      matching key can detect this model as a fiducial. */
  void SetFiducialKey(  int key );
  
  void GetColor( stg_color_t* col )
  { if(color) memcpy( col, &this->color, sizeof(stg_color_t)); }
  
  void GetLaserReturn( int* val )
  { if(val) memcpy( val, &this->laser_return, sizeof(int)); }
  
  /** Change a model's parent - experimental*/
  int SetParent( StgModel* newparent);
  
  /** Get a model's geometry - it's size and local pose (offset from
      origin in local coords) */
  void GetGeom(  stg_geom_t* dest );

  /** Get the pose of a model in its parent's coordinate system  */
  void GetPose(  stg_pose_t* dest );

  /** Get a model's velocity (in its local reference frame) */
  void GetVelocity(  stg_velocity_t* dest );

  // guess what these do?
  void SetColor( stg_color_t col );
  void SetMass( stg_kg_t mass );
  void SetStall( stg_bool_t stall );
  void SetGripperReturn( int val );
  void SetLaserReturn( int val );
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
  
  /* hooks for attaching special callback functions (not used as
     variables) */
  char startup, shutdown, load, save, update;
  
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
  
  // string-based property interface
  void* GetProperty( char* key );
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
  void LocalToGlobal( stg_pose_t* pose );
  
  /** returns the first descendent of this model that is unsubscribed
      and has the type indicated by the string */
  StgModel* GetUnsubcribedModelOfType( char* modelstr );
  
  // this version raytraces from our local origin
  stg_meters_t Raytrace( stg_radians_t angle, 
			 stg_meters_t range, 
			 stg_block_match_func_t func,
			 const void* arg,
			 StgModel** hitmod );
  
  // raytraces from the point and heading identified by pose, in local coords
  stg_meters_t Raytrace( stg_pose_t* pose,
			 stg_meters_t range, 
			 stg_block_match_func_t func,
			 const void* arg,
			 StgModel** hitmod );
  
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

  void Draw(); // draw the block in OpenGL
  void DrawSolid(); // draw the block in OpenGL as a solid single color

  stg_point_t* pts; //< points defining a polygon
  size_t pt_count; //< the number of points
  stg_meters_t zmin; 
  stg_meters_t zmax; 

  StgModel* mod; //< model to which this block belongs
  stg_point_int_t* pts_global; //< points defining a polygon in global coords

  void RecordRenderPoint( uint32_t x, uint32_t y );
  
private:
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

// // WORLDGTK class - a world plus a GUI
// class StgWorldGtk : public StgWorld
// {
// public:
//   StgWorldGtk( void );
//   virtual ~StgWorldGtk( void );
    
//   void Configure( int width, 
// 		  int height, 
// 		  double xscale, 
// 		  double yscale, 
// 		  double xcenter, 
// 		  double ycenter );

//   static void Init( int* argc, char** argv[] );

//   virtual void Load( const char* filename );
//   virtual void Save( void );
//   virtual void Draw( void );
//   virtual bool Update( void );
//   virtual bool RealTimeUpdate( void );
  
//   virtual void PushColor( stg_color_t col )
//   { colorstack.Push( col ); } 
  
//   virtual void PushColor( double r, double g, double b, double a )
//   { colorstack.Push( r,g,b,a ); }

//   virtual void PopColor()
//   { colorstack.Pop(); } 

//   void AddViewItem( const gchar *name,
// 		    const gchar *label,
// 		    const gchar *tooltip,
// 		    GCallback callback,
// 		    gboolean  is_active,
// 		    void* userdata );

// protected:
//   GlColorStack colorstack;
  
//   virtual void AddModel( StgModel*  mod );

// private:
//   GList* selected_models; ///<   a list of models that are currently selected by the user 

//   // Gtk stuff
//   GtkWidget* frame;    
//   GtkWidget *layout;
//   GtkWidget *menu_bar;
//   //GtkWidget* scrolled_win;
  
//   // The status bar widget
//   GtkStatusbar *status_bar;
//   GtkProgressBar *perf_bar;
//   GtkProgressBar *rt_bar;
//   GtkLabel *clock_label;
//   GtkActionGroup *action_group;

//   // the main drawing widget
//   GtkWidget* canvas;
//   int dirty;
  
//   int draw_list;
//   int debug_list;

//   char clock[512];

//   stg_radians_t stheta; ///< view rotation about x axis
//   stg_radians_t sphi; ///< view rotation about x y axis
//   double scale; ///< view scale
//   double panx; ///< pan along x axis in meters
//   double pany; ///< pan along y axis in meters
  
//   stg_point_t click_point; ///< The place where the most recent
//   ///< mouse click happened, in world coords
    
//   GtkStatusbar* statusbar;
//   GtkLabel* timelabel;
    
//   gboolean dragging;
  
//   int frame_series;
//   int frame_index;
//   int frame_callback_tag;
//   int frame_interval;
//   stg_image_format_t frame_format;
  
//   stg_pose_t selection_pose_start;
//   stg_point_3d_t selection_pointer_start;
  
//   GList* toggle_list;  

//   int wf_section;
  
//   //GdkGLConfig *glconfig;

//   // mouse click position, for dragging
//   int beginX, beginY;
  
// private:
//   stg_msec_t redraw_interval;
//   guint timer_handle;

//   bool follow_selection;
//   bool show_quadtree;  
//   bool show_occupancy;  
//   bool show_fill;
//   bool show_geom;
//   bool show_polygons;
//   bool show_grid;
//   bool show_data;
//   bool show_cfg;
//   bool show_cmd;
//   bool show_alpha;
//   bool show_thumbnail;
//   bool show_bboxes;
//   bool show_trails;

// public:
//   void SetFollowSelection( bool b ){ this->follow_selection = b; dirty=true; }
//   void SetShowFill( bool b ){ this->show_fill = b; dirty=true; }    
//   void SetShowGeom( bool b ){ this->show_geom = b; dirty=true; }    
//   void SetShowPolygons( bool b ){ this->show_polygons = b;  dirty=true; }
//   void SetShowAlpha( bool b ){ this->show_alpha = b; dirty=true; }
//   void SetShowThumbnail( bool b ){ this->show_thumbnail = b;  dirty=true;}
//   void SetShowBBoxes( bool b ){ this->show_bboxes = b; dirty=true; }
//   void SetShowTrails( bool b ){ this->show_trails = b; dirty=true; }
//   void SetShowGrid( bool b ){ this->show_grid = b; dirty=true; }
//   void SetShowQuadtree( bool b ){ this->show_quadtree = b; dirty=true; }
//   void SetShowOccupancy( bool b ){ this->show_occupancy = b; dirty=true; }
  
  
//   // signal handlers
//   //gboolean EvDelete( GtkWidget *widget );
//   //void EvDestroy( GtkWidget *widget, GdkEvent *event );
//   void Realize( GtkWidget *widget );
//   //void EvConfigure( GtkWidget *widget );
  
//   void CanvasMotionNotify( GdkEventMotion *event );
//   void CanvasButtonPress( GdkEventButton *event );
//   void CanvasButtonRelease( GdkEventButton *event );
  
//   gboolean DialogQuit( GtkWindow* parent );
//   void SetTitle( char* txt );
//   void RenderClock();
//   void SetWindowSize( unsigned int w, unsigned int h );
  
//   // exporting screen shots as image files
//   void ExportSequenceStart();
//   void ExportSequenceStop();
//   void ExportSequence();
//   void ExportWindow();
//   void SetExportFormat( stg_image_format_t format);
//   void SetExportInterval( int interval );
  
//   void Derotate();
//   void Zoom( double multiplier ); ///< multiply the scale factor by the argument
//   void SetScale( double scale ); ///< set the scale factor directly
//   void AboutBox();
//   GtkWidget* GetCanvas(){ return canvas; }
  
//   void PushModelStatus( StgModel* mod, char* verb );
//   void PopModelStatus();
  
//   // convert canvas window pixel coordinate to 3d world coordinate
//   void CanvasToWorld( int px, int py, 
// 		      double *wx, double *wy, double* wz );

//   StgModel* NearestRootModel( double wx, double wy );

//   };

/** @} */

/* FLTK code - experimenting with replacing GTK with a more portable
   and less bulky framework */
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Value_Slider.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Menu_Button.H>
#include <FL/Fl_Gl_Window.H>
#include <FL/gl.h>
#include <Fl/Fl_Box.H>

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

public:
  StgCanvas( StgWorld* world, int x, int y, int W,int H);
  ~StgCanvas();
  
  StgWorld* world;
  bool graphics;

  void FixViewport(int W,int H);
  virtual void draw();
  virtual int handle( int event );
  void resize(int X,int Y,int W,int H);

  void CanvasToWorld( int px, int py, 
		      double *wx, double *wy, double* wz );

  StgModel* Select( int x, int y );
  
  inline void PushColor( stg_color_t col )
  { colorstack.Push( col ); } 
  
  inline void PushColor( double r, double g, double b, double a )
  { colorstack.Push( r,g,b,a ); }
  
  inline void PopColor()
  { colorstack.Pop(); } 
  
  void InvertView( uint32_t invertflags );

  uint32_t GetShowFlags()
  { return showflags; };
  
  void SetShowFlags( uint32_t flags )
  { showflags = flags; };

  // set the bit indicated by flag to the value indicated by val
  //void SetShowFlag( uint32_t flag, bool val )
  // {
  //val ? (showflags |= flag) : (showflags ^= flag );
  //} 

  static void TimerCallback( StgCanvas* canvas );
};

class StgWorldGui : public StgWorld, public Fl_Window 
{
private:
  int wf_section;

public:
  StgWorldGui(int W,int H,const char*L=0);
  ~StgWorldGui();

  StgCanvas* canvas;
  Fl_Menu_Bar* mbar;
  
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
};

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define THOUSAND (1e3)
#define MILLION (1e6)
#define BILLION (1e9)

#ifndef M_PI
#define M_PI (3.14159265358979323846)
#endif

#ifndef TWOPI
#define TWOPI (2.0*M_PI)
#endif
  
#ifndef RTOD
  /// Convert radians to degrees
#define RTOD(r) ((r) * 180.0 / M_PI)
#endif
  
#ifndef DTOR
  /// Convert degrees to radians
#define DTOR(d) ((d) * M_PI / 180.0)
#endif
  
#ifndef NORMALIZE
  /// Normalize angle to domain -pi, pi
#define NORMALIZE(z) atan2(sin(z), cos(z))
#endif

// end doc group libstage_utilities
/** @} */ 




  // BLOBFINDER MODEL --------------------------------------------------------
  
#define STG_BLOB_CHANNELS_MAX 16
  
  /** blobfinder config packet
   */
  typedef struct
  {
    int channel_count; // 0 to STG_BLOBFINDER_CHANNELS_MAX
    stg_color_t channels[STG_BLOB_CHANNELS_MAX];
    int scan_width;
    int scan_height;
    stg_meters_t range_max;
  } stg_blobfinder_config_t;
  
  /** blobfinder data packet 
   */
  typedef struct
  {
    int channel;
    stg_color_t color;
    int xpos, ypos;   // all values are in pixels
    //int width, height;
    int left, top, right, bottom;
    int area;
    stg_meters_t range;
  } stg_blobfinder_blob_t;


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

// BLINKENLIGHT -------------------------------------------------------

  //typedef struct
  //{
  //int enable;
  //stg_msec_t period;
  //} stg_blinkenlight_t;

// PTZ MODEL --------------------------------------------------------
  
  /** ptz command: specify desired PTZ angles. Tilt has no effect.
   */
  typedef stg_ptz_t stg_ptz_cmd_t;

  /** ptz data: specifies actual PTZ angles. 
   */
  typedef stg_ptz_t stg_ptz_data_t;

  /** ptz config structure
   */
  typedef struct
  {
    stg_ptz_t min; ///< Minimum PTZ angles.
    stg_ptz_t max; ///< Maximum PTZ angles.
    stg_ptz_t goal; ///< The current desired angles. The device servos towards these values.
    stg_ptz_t speed; ///< The PTZ servo speeds.
  } stg_ptz_config_t;
  
  
  // LASER MODEL --------------------------------------------------------
  
  /** laser sample packet
   */
  typedef struct
  {
    stg_meters_t range; ///< range to laser hit in meters
    double reflectance; ///< intensity of the reflection 0.0 to 1.0
    stg_point_t hitpoint; ///< the location of the laser hit in local coordinates
  } stg_laser_sample_t;
  
  class StgModelLaser : public StgModel
  {
  private:
    int dl_debug_laser;
    
  public:
    // constructor
    StgModelLaser( StgWorld* world,
		   StgModel* parent, 
		   stg_id_t id, 
		   char* typestr );
    
    // destructor
    ~StgModelLaser( void );
    
    stg_laser_sample_t* samples;
    uint32_t sample_count;
    stg_meters_t range_min, range_max;
    stg_radians_t fov;
    uint32_t resolution;
    
    virtual void Startup( void );
    virtual void Shutdown( void );
    virtual void Update( void );
    virtual void Load( void );  
    virtual void Print( char* prefix );
    virtual void DataVisualize( void );
    //    virtual void InitGraphics(void);

    void SetSampleCount( unsigned int count );

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


  // GRIPPER MODEL --------------------------------------------------------
  
  typedef enum {
    STG_GRIPPER_PADDLE_OPEN = 0, // default state
    STG_GRIPPER_PADDLE_CLOSED, 
    STG_GRIPPER_PADDLE_OPENING,
    STG_GRIPPER_PADDLE_CLOSING,
  } stg_gripper_paddle_state_t;

  typedef enum {
    STG_GRIPPER_LIFT_DOWN = 0, // default state
    STG_GRIPPER_LIFT_UP, 
    STG_GRIPPER_LIFT_UPPING, // verbed these to match the paddle state
    STG_GRIPPER_LIFT_DOWNING, 
  } stg_gripper_lift_state_t;
  
  typedef enum {
    STG_GRIPPER_CMD_NOP = 0, // default state
    STG_GRIPPER_CMD_OPEN, 
    STG_GRIPPER_CMD_CLOSE,
    STG_GRIPPER_CMD_UP, 
    STG_GRIPPER_CMD_DOWN    
  } stg_gripper_cmd_type_t;
  
  /** gripper configuration packet
   */
  typedef struct
  {
    stg_size_t paddle_size; ///< paddle dimensions 

    stg_gripper_paddle_state_t paddles; 
    stg_gripper_lift_state_t lift;

    double paddle_position; ///< 0.0 = full open, 1.0 full closed
    double lift_position; ///< 0.0 = full down, 1.0 full up

    stg_meters_t inner_break_beam_inset; ///< distance from the end of the paddle
    stg_meters_t outer_break_beam_inset; ///< distance from the end of the paddle  
    stg_bool_t paddles_stalled; // true iff some solid object stopped
				// the paddles closing or opening
    
    GSList *grip_stack;  ///< stack of items gripped
    int grip_stack_size; ///< maximum number of objects in stack, or -1 for unlimited

    double close_limit; ///< How far the gripper can close. If < 1.0, the gripper has its mouth full.

  } stg_gripper_config_t;

  /** gripper command packet
   */
  typedef struct
  {
    stg_gripper_cmd_type_t cmd;
    int arg;
  } stg_gripper_cmd_t;


  /** gripper data packet
   */
  typedef struct
  {
    stg_gripper_paddle_state_t paddles; 
    stg_gripper_lift_state_t lift;
    
    double paddle_position; ///< 0.0 = full open, 1.0 full closed
    double lift_position; ///< 0.0 = full down, 1.0 full up

    stg_bool_t inner_break_beam; ///< non-zero iff beam is broken
    stg_bool_t outer_break_beam; ///< non-zero iff beam is broken
    
    stg_bool_t paddle_contacts[2]; ///< non-zero iff paddles touch something

    stg_bool_t paddles_stalled; // true iff some solid object stopped
				// the paddles closing or opening

    int stack_count; ///< number of objects in stack


  } stg_gripper_data_t;


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
    char* key;
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
public:
  // constructor
  StgModelFiducial( StgWorld* world,
		    StgModel* parent, 
		    stg_id_t id, 
		    char* typestr );
  
  // destructor
  virtual ~StgModelFiducial( void );
  
  //virtual void Startup( void );
  //virtual void Shutdown( void );
  virtual void Update( void );
  virtual void Load( void );
 
  stg_meters_t max_range_anon; //< maximum detection range
  stg_meters_t max_range_id; ///< maximum range at which the ID can be read
  stg_meters_t min_range; ///< minimum detection range
  stg_radians_t fov; ///< field of view 
  stg_radians_t heading; ///< center of field of view
  
  /// only detect fiducials with a key string that matches this one
  /// (defaults to NULL)
  char* key;
  
  // an array of stg_fiducial_t structs generated by the sensor
  GArray* data;
  
  // if neighbor is visible, add him to the fiducial scan
  void AddModelIfVisible( StgModelFiducial* him );
  
//   // static wrapper function can be used as a function pointer
//   static void AddModelIfVisibleStatic( gpointer key, 
// 				       StgModelFiducial* him, 
// 				       StgModelFiducial* me )
//   { if( him != me ) me->AddNeighborIfVisible( him ); };

};


  // RANGER MODEL --------------------------------------------------------

  typedef struct
  {
    stg_pose_t pose;
    stg_size_t size;
    stg_bounds_t bounds_range;
    stg_radians_t fov;
    int ray_count;
    stg_meters_t range;
    //double error; // TODO
  } stg_ranger_sensor_t;
  
class StgModelRanger : public StgModel
{
public:
  // constructor
  StgModelRanger( StgWorld* world,
		  StgModel* parent, 
		  stg_id_t id, 
		  char* typestr );
  
  // destructor
  virtual ~StgModelRanger( void );
  
  virtual void Startup( void );
  virtual void Shutdown( void );
  virtual void Update( void );
  virtual void Load( void );
  virtual void Print( char* prefix );
  virtual void DataVisualize( void );

  size_t sensor_count;
  stg_ranger_sensor_t* sensors;

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

  // BUMPER MODEL --------------------------------------------------------

  typedef struct
  {
    stg_pose_t pose;
    stg_meters_t length;
  } stg_bumper_config_t;
  
  typedef struct
  {
    StgModel* hit;
    stg_point_t hit_point;
  } stg_bumper_sample_t;
  
  // POSITION MODEL --------------------------------------------------------
  
  typedef enum
    { STG_POSITION_CONTROL_VELOCITY, STG_POSITION_CONTROL_POSITION }
  stg_position_control_mode_t;
  
#define STG_POSITION_CONTROL_DEFAULT STG_POSITION_CONTROL_VELOCITY
  
  typedef enum
    { STG_POSITION_LOCALIZATION_GPS, STG_POSITION_LOCALIZATION_ODOM }
  stg_position_localization_mode_t;
  
#define STG_POSITION_LOCALIZATION_DEFAULT STG_POSITION_LOCALIZATION_GPS
  
  /** "position_drive" property */
  typedef enum
    { STG_POSITION_DRIVE_DIFFERENTIAL, STG_POSITION_DRIVE_OMNI, STG_POSITION_DRIVE_CAR }
  stg_position_drive_mode_t;
  
#define STG_POSITION_DRIVE_DEFAULT STG_POSITION_DRIVE_DIFFERENTIAL
  
  /** "position_cmd" property */
  typedef struct
  {
    stg_meters_t x,y,a;
    stg_position_control_mode_t mode;
  } stg_position_cmd_t;
  
  /** "position_data" property */
  typedef struct
  {
    stg_pose_t pose; ///< position estimate in local coordinates
    stg_pose_t pose_error; ///< estimated error in position estimate
    stg_pose_t origin; ///< global origin of the local coordinate system
    stg_velocity_t velocity; ///< current translation and rotaation speeds
    stg_velocity_t integration_error; ///< errors in simple odometry model
    //stg_bool_t stall; ///< TRUE iff the robot can't move due to a collision
    stg_position_localization_mode_t localization; ///< global or local mode
  } stg_position_data_t;
  
  /** position_cfg" property */
  typedef struct
  {
    stg_position_drive_mode_t drive_mode;
    stg_position_localization_mode_t localization_mode;
  } stg_position_cfg_t;

  /// set the current odometry estimate 
  //void stg_model_position_set_odom( stg_model_t* mod, stg_pose_t* odom ); 

class StgModelPosition : public StgModel
{
public:
  // constructor
  StgModelPosition( StgWorld* world,
		    StgModel* parent, 
		    stg_id_t id, 
		    char* typestr);
  
  // destructor
  ~StgModelPosition( void );
  
  // control state
  stg_pose_t goal; //< the current velocity or pose to reach,
		   //depending on the value of control_mode
  stg_position_control_mode_t control_mode;
  stg_position_drive_mode_t drive_mode;

  // localization state
  stg_pose_t est_pose; ///< position estimate in local coordinates
  stg_pose_t est_pose_error; ///< estimated error in position estimate
  stg_pose_t est_origin; ///< global origin of the local coordinate system

  stg_position_localization_mode_t localization_mode; ///< global or local mode
  stg_velocity_t integration_error; ///< errors to apply in simple odometry model
  
  virtual void Startup( void );
  virtual void Shutdown( void );
  virtual void Update( void );
  virtual void Load( void );

  /** Set the current pose estimate.*/
  void SetOdom( stg_pose_t* odom );
  
  /** Set the goal for the position device. If member control_mode ==
      STG_POSITION_CONTROL_VELOCITY, these are x,y, and rotation
      velocities. If control_mode == STG_POSITION_CONTROL_POSITION,
      [x,y,a] defines a 2D position and heading goal to achieve. */
  void Do( double x, double y, double a ) 
  { goal.x = x; goal.y = y; goal.a = a; }  

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


  // WIFI MODEL --------------------------------------------------------

  /** wifi config packet 
   */
  typedef struct
  {
    // Configuration for the wifi model goes here.  E.g., power, range of
    // propagation.
  } stg_wifi_config_t;

  /** wifi data packet 
   */
  typedef struct
  {
    // Simulated wifi data goes here.  E.g., for each neighbor within
    // range, record the corresponding signal strength.
  } stg_wifi_data_t;

  // SPEECH MODEL --------------------------------------------------------

#define STG_SPEECH_MAX_STRING_LEN 256

  typedef enum {
    STG_SPEECH_CMD_NOP = 0, // default state
    STG_SPEECH_CMD_SAY
  } stg_speech_cmd_type_t;

  /** speech configuration packet
   */
  typedef struct
  {
    char string[STG_SPEECH_MAX_STRING_LEN];
  } stg_speech_config_t;

  /** speech data packet
   */
  typedef struct
  {
    char string[STG_SPEECH_MAX_STRING_LEN];
  } stg_speech_data_t;

  /** speech command packet
   */
  typedef struct
  {
    stg_speech_cmd_type_t cmd;
    char string[STG_SPEECH_MAX_STRING_LEN];
  } stg_speech_cmd_t;

  // end the group of all models
  /**@}*/

// end documentation group libstage
/**@}*/

#endif
