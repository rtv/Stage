#ifndef STG_H
#define STG_H
/*
 *  Stage : a multi-robot simulator.  
 * 
 *  Copyright (C) 2001-2004 Richard Vaughan, Andrew Howard and Brian
 *  Gerkey for the Player/Stage Project
 *  http://playerstage.sourceforge.net
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
 * Authors: Richard Vaughan vaughan@sfu.ca 
 *          Andrew Howard ahowards@usc.edu
 *          Brian Gerkey gerkey@stanford.edu
 * Date: 1 June 2003
 * CVS: $Id: stage.h,v 1.149 2005-07-30 00:04:41 rtv Exp $
 */


/*! \file stage.h 
  Stage library header file

  This header file contains the external interface for the Stage
  library
*/


/** @defgroup libstage libstage
    A C library for creating robot simulations
    @{
*/

#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h> // for portable int types eg. uint32_t
#include <sys/types.h>
#include <sys/time.h>
#include <assert.h>
#include <pthread.h>
#include <semaphore.h>

#include <glib.h> // we use GLib's data structures extensively
#include <rtk.h> // and graphics stuff pulled from Andrew Howard's RTK2 library

#ifdef __cplusplus
extern "C" {
#endif 

#include "config.h"
#include "replace.h"


  /** @addtogroup stg_model
      @{ */

  /** any integer value other than this is a valid fiducial ID 
   */
  // TODO - fix this up
#define FiducialNone 0
  

  // Basic self-describing measurement types. All packets with real
  // measurements are specified in these terms so changing types here
  // should work throughout the code If you change these, be sure to
  // change the byte-ordering macros below accordingly.
  typedef int stg_id_t;
  typedef double stg_meters_t;
  typedef double stg_radians_t;
  typedef unsigned long stg_msec_t;
  typedef double stg_kg_t; // Kilograms (mass)
  typedef double stg_joules_t; // Joules (energy)
  typedef double stg_watts_t; // Watts (Joules per second) (energy expenditure)
  typedef int stg_bool_t;
  typedef double stg_friction_t;
  typedef uint32_t stg_color_t;
  typedef int stg_obstacle_return_t;
  typedef int stg_blob_return_t;
  typedef int stg_fiducial_return_t;
  typedef int stg_ranger_return_t;
  
  typedef enum { STG_GRIP_NO = 0, STG_GRIP_YES } stg_gripper_return_t;
  
  /** specify a rectangular size 
   */
  typedef struct 
  {
    stg_meters_t x, y;
  } stg_size_t;
  
  /** specify a 3 axis position, in x, y and heading.
   */
  typedef struct
  {
    stg_meters_t x, y, a;
  } stg_pose_t;
  
  /** specify a 3 axis velocity in x, y and heading.
   */
  typedef stg_pose_t stg_velocity_t;  

  /** specify an object's geometry: position and rectangular size
   */
  typedef struct
  {
    stg_pose_t pose;
    stg_size_t size;
  } stg_geom_t;

  // ENERGY --------------------------------------------------------------
  
  /** energy data packet */
  typedef struct
  {
    /** estimate of current energy stored */
    stg_joules_t stored;

    /** maximum storage capacity */
    stg_joules_t capacity;

    /** total joules received */
    stg_joules_t input_joules;

    /** total joules supplied */
    stg_joules_t output_joules;

    /** estimate of current energy output */
    stg_watts_t input_watts;

    /** estimate of current energy input */
    stg_watts_t output_watts;

    /** TRUE iff the device is receiving energy from a charger */
    stg_bool_t charging;

    /** the range to the charger, if attached, in meters */
    stg_meters_t range;
  } stg_energy_data_t;

  /** energy config packet (use this to set or get energy configuration)*/
  typedef struct
  {
    /** maximum storage capacity */
    stg_joules_t capacity;

    /** when charging another device, supply this many joules/sec at most*/
    stg_watts_t give;

    /** when charging from another device, receive this many
	joules/sec at most*/
    stg_watts_t take;

    /** length of the charging probe */
    stg_meters_t probe_range;
  } stg_energy_config_t;

  // there is currently no energy command packet

  // BLINKENLIGHT -------------------------------------------------------

  //typedef struct
  //{
  //int enable;
  //stg_msec_t period;
  //} stg_blinkenlight_t;

  // GRIPPER ------------------------------------------------------------

  // Possible Gripper return values
  //typedef enum 
  //{
  //  GripperDisabled = 0,
  //  GripperEnabled
  //} stg_gripper_return_t;

  // GUIFEATURES -------------------------------------------------------
  
  // Movement masks for figures
#define STG_MOVE_TRANS (1 << 0)
#define STG_MOVE_ROT   (1 << 1)
#define STG_MOVE_SCALE (1 << 2)
  
  typedef int stg_movemask_t;
  
  typedef struct
  {
    uint8_t show_data;
    uint8_t show_cfg;
    uint8_t show_cmd;
    
    uint8_t nose;
    uint8_t grid;
    //uint8_t boundary;
    uint8_t outline;
    stg_movemask_t movemask;
  } stg_guifeatures_t;


  // LASER ------------------------------------------------------------

  /// laser return value
  typedef enum 
    {
      LaserTransparent, ///<not detected by laser model 
      LaserVisible, ///< detected by laser with a reflected intensity of 0 
      LaserBright  ////< detected by laser with a reflected intensity of 1 
    } stg_laser_return_t;

  /**@}*/
  

  /// returns the real (wall-clock) time in seconds
  stg_msec_t stg_timenow( void );
  
  /** initialize the stage library - optionally pass in the arguments
      to main, so Stage or rtk or gtk or xlib can read the options. 
  */
  int stg_init( int argc, char** argv );
  
  /** Get a string identifying the version of stage. The string is
      generated by autoconf 
  */
  const char* stg_get_version_string( void );
  
  /// if stage wants to quit, this will return non-zero
  int stg_quit_test( void );

  /// set stage's quit flag
  void stg_quit_request( void );

  /// report an error
  void stg_err( const char* err );


  // UTILITY STUFF ----------------------------------------------------

  /** @defgroup util Utilities
      @{
  */
  
  void stg_pose_sum( stg_pose_t* result, stg_pose_t* p1, stg_pose_t* p2 );

  // ROTATED RECTANGLES -------------------------------------------------

  /** @defgroup rotrect Rotated Rectangles
      @{ 
  */
  
  typedef struct
  {
    stg_pose_t pose;
    stg_size_t size;
  } stg_rotrect_t; // rotated rectangle
  
  /** normalizes the set [rects] of [num] rectangles, so that they fit
      exactly in a unit square.
  */
  void stg_normalize_rects( stg_rotrect_t* rects, int num );
  
  /** load the image file [filename] and convert it to an array of
      rectangles, filling in the number of rects, width and
      height. Caller must free the rectangle array.
   */
  int stg_load_image( const char* filename, 
		      stg_rotrect_t** rects,
		      int* rect_count,
		      int* widthp, int* heightp );
  
  /**@}*/

  /** print human-readable description of a geometry struct on stdout 
   */
  void stg_print_geom( stg_geom_t* geom );
  
  /** Look up the color in the X11 database.  (i.e. transform color name to
      color value).  If the color is not found in the database, a bright
      red color (0xF00) will be returned instead.
  */
  stg_color_t stg_lookup_color(const char *name);

  // POINTS ---------------------------------------------------------

  /** @defgroup stg_point Points
      Creating and manipulating points
      @{
  */

  typedef struct
  {
    stg_meters_t x, y;
  } stg_point_t;

  /// create an array of [count] points. Caller must free the space.
  stg_point_t* stg_points_create( size_t count );

  /// create a single point structure. Caller must free the space.
  stg_point_t* stg_point_create( void );

  /// frees a point array
  void stg_points_destroy( stg_point_t* pts );

  /// frees a single point
  void stg_point_destroy( stg_point_t* pt );

  /**@}*/

  // POLYGONS ---------------------------------------------------------

  /** @defgroup stg_polygon Polygons
      Creating and manipulating polygons
      @{
  */

  typedef struct
  {
    /// pointer to an array of points
    GArray* points;

    /// if TRUE, this polygon is drawn filled
    stg_bool_t filled; 

    /// render color of this polygon - TODO  - implement color rendering
    stg_color_t color;
  } stg_polygon_t; 

  
  /// return an array of [count] polygons. Caller must free() the space.
  stg_polygon_t* stg_polygons_create( int count );
  
  /// destroy an array of [count] polygons
  void stg_polygons_destroy( stg_polygon_t* p, size_t count );
  
  /// return a single polygon structure. Caller  must free() the space.
  stg_polygon_t* stg_polygon_create( void );
  
  /** creates a unit square polygon
   */
  stg_polygon_t* stg_unit_polygon_create( void );
  
  /** load [filename], an image format understood by gdk-pixbuf, and
      return a set of rectangles that approximate the image. Caller
      must free the array of rectangles. If width and height are
      non-null, they are filled in with the size of the image in pixels 
  */

  /// destroy a single polygon
  void stg_polygon_destroy( stg_polygon_t* p );
  
  /// Copies [count] points from [pts] into polygon [poly], allocating
  /// memory if mecessary. Any previous points in [poly] are
  /// overwritten.
  void stg_polygon_set_points( stg_polygon_t* poly, stg_point_t* pts, size_t count );			       
  /// Appends [count] points from [pts] into polygon [poly],
  /// allocating memory if mecessary.
  void stg_polygon_append_points( stg_polygon_t* poly, stg_point_t* pts, size_t count );			       

  stg_polygon_t* stg_rects_to_polygons( stg_rotrect_t* rects, size_t count );
  
  /// scale the array of [num] polygons so that all its points fit
  /// exactly in a rectagle of pwidth] by [height] units
  void stg_normalize_polygons( stg_polygon_t* polys, int num, 
			       double width, double height );
  
  /// print a human-readable description of a polygon on stdout
  void stg_polygon_print( stg_polygon_t* poly );
  
  /// print a human-readable description of an array of polygons on stdout
  void stg_polygons_print( stg_polygon_t* polys, unsigned int count );

  /**@}*/

  // end util documentation group
  /**@}*/


  // end property typedefs -------------------------------------------------


  
  // get property structs with default values filled in --------------------
  
  /** @defgroup defaults Defaults
      @{ */
  
  void stg_get_default_pose( stg_pose_t* pose );
  void stg_get_default_geom( stg_geom_t* geom );
  
  /**@}*/

  // end defaults --------------------------------------------
  

  // forward declare struct types
  struct _stg_world; 
  struct _stg_model;
  struct _stg_matrix;
  struct _gui_window;

  /** @addtogroup stg_model
      @{ */

  /** opaque data structure implementing a model
   */
  typedef struct _stg_model stg_model_t;

  /**@}*/

  //  WORLD --------------------------------------------------------

  /** @defgroup stg_world Worlds
     Implements a world - a collection of models and a matrix.   
     @{
  */
  
  /** opaque data structure implementing a world
   */
  typedef struct _stg_world stg_world_t;
  
  
  /** Create a new world, to be configured and populated by user code
   */
  stg_world_t* stg_world_create( stg_id_t id, 
				 const char* token, 
				 int sim_interval, 
				 int real_interval,
				 double ppm,
				 double width,
				 double height );

  /** Create a new world as described in the worldfile [worldfile_path] 
   */
  stg_world_t* stg_world_create_from_file( const char* worldfile_path );

  /** Destroy a world and everything it contains
   */
  void stg_world_destroy( stg_world_t* world );
  
  /** Stop the world clock
   */
  void stg_world_stop( stg_world_t* world );
  
  /** Start the world clock
   */
  void stg_world_start( stg_world_t* world );

  /** Run one simulation step. Returns 0 if all is well, or a positive error code 
   */
  int stg_world_update( stg_world_t* world, int sleepflag );

  /** configure the world by reading from the current world file */
  void stg_world_load( stg_world_t* mod );

  /** save the state of the world to the current world file */
  void stg_world_save( stg_world_t* mod );

  /** print human-readable information about the world on stdout 
   */
  void stg_world_print( stg_world_t* world );

  /** Set the duration in milliseconds of each simulation update step 
   */
  void stg_world_set_interval_real( stg_world_t* world, unsigned int val );
  
  /** Set the real time in intervals that Stage should attempt to take
      for each simulation update step. If Stage has too much
      computation to do, it might take longer than this. */
  void stg_world_set_interval_sim( stg_world_t* world, unsigned int val );

 /// get a model pointer from its ID
  stg_model_t* stg_world_get_model( stg_world_t* world, stg_id_t mid );
  
  /// get a model pointer from its name
  stg_model_t* stg_world_model_name_lookup( stg_world_t* world, const char* name );
  

  struct stg_property;
  
  typedef int (*stg_property_callback_t)(stg_model_t* mod, char* name, void* data, size_t len, void* userdata );
  
  typedef void (*stg_property_storage_func_t)( struct stg_property* prop,
					      void* data, size_t len );


  /// install a property callback on every model in the world that has this property
  void stg_world_add_property_callback( stg_world_t* world,
					char* propname,
					stg_property_callback_t callback,
					void* userdata );
  
  /// remove a property callback from every model in the world that has this property
  void stg_world_remove_property_callback( stg_world_t* world,
					   char* propname,
					   stg_property_callback_t callback );
  
  /// add an item to the View menu that will automatically install and
  /// remove a callback when the item is toggled
/*   void stg_world_add_property_toggles( stg_world_t* world,  */
/* 				       const char* propname,  */
/* 				       stg_property_callback_t callback_on, */
/* 				       void* arg_on, */
/* 				       stg_property_callback_t callback_off, */
/* 				       void* arg_off, */
/* 				       const char* label, */
/* 				       int enabled ); */
  
  void stg_model_add_property_toggles( stg_model_t* mod, 
				       const char* propname, 
				       stg_property_callback_t callback_on,
				       void* arg_on,
				       stg_property_callback_t callback_off,
				       void* arg_off,
				       const char* label,
				       int enabled );

  int stg_model_fig_clear_cb( stg_model_t* mod, void* data, size_t len, 
			      void* userp );
  /**@}*/



  //  MODEL --------------------------------------------------------
    
  // group the docs of all the model types
  /** @defgroup stg_models Models
      @{ */
  
  /** @defgroup stg_model Basic model
      Implements the basic object
      @{ */
  
#define STG_PROPNAME_MAX 128
  
  typedef struct
  {
    stg_property_callback_t callback;
    void* arg;
  } stg_cbarg_t;

  typedef struct stg_property {
    char name[STG_PROPNAME_MAX];
    void* data;
    size_t len;
    stg_property_storage_func_t storage_func;
    GList* callbacks; // functions called when this property is set
    //GList* callbacks_userdata;
    stg_model_t* mod; // the model to which this property belongs
    //void* user; // pointer passed into every callback function
  } stg_property_t;
  
  
  typedef int(*stg_model_initializer_t)(stg_model_t*);
  
  /// create a new model
  stg_model_t* stg_model_create(  stg_world_t* world,
				  stg_model_t* parent, 
				  stg_id_t id, 
				  char* token,
				  stg_model_initializer_t initializer );

  /** destroy a model, freeing its memory */
  void stg_model_destroy( stg_model_t* mod );

  /** get the pose of a model in the global CS */
  void stg_model_get_global_pose( stg_model_t* mod, stg_pose_t* pose );

  /** get the velocity of a model in the global CS */
  void stg_model_get_global_velocity( stg_model_t* mod, stg_velocity_t* gvel );

  /* set the velocity of a model in the global coordinate system */
  //void stg_model_set_global_velocity( stg_model_t* mod, stg_velocity_t* gvel );

  /** subscribe to a model's data */
  void stg_model_subscribe( stg_model_t* mod );

  /** unsubscribe from a model's data */
  void stg_model_unsubscribe( stg_model_t* mod );

  /** configure a model by reading from the current world file */
  void stg_model_load( stg_model_t* mod );

  /** save the state of the model to the current world file */
  void stg_model_save( stg_model_t* mod );

  /** get a human-readable string for the model's type */
  //const char* stg_model_type_string( stg_model_type_t type );

  // SET properties - use these to set props, don't set them directly

  /** set the pose of model in global coordinates */
  int stg_model_set_global_pose( stg_model_t* mod, stg_pose_t* gpose );
  
  /** set a model's velocity in it's parent's coordinate system */
  int stg_model_set_velocity( stg_model_t* mod, stg_velocity_t* vel );
  
  /** Get exclusive access to a model, for threaded
      applications. Release with stg_model_unlock(). */
  void stg_model_lock( stg_model_t* mod );
  
  /** Release exclusive access to a model, obtained with stg_model_lock() */
  void stg_model_unlock( stg_model_t* mod );

  /** Change a model's parent - experimental*/
  int stg_model_set_parent( stg_model_t* mod, stg_model_t* newparent);
  
  void stg_model_get_geom( stg_model_t* mod, stg_geom_t* dest );
  void stg_model_get_velocity( stg_model_t* mod, stg_velocity_t* dest );
  
  stg_property_t* stg_model_set_property( stg_model_t* mod, 
					  const char* prop, 
					  void* data, 
					  size_t len );
  
  stg_property_t* stg_model_set_property_ex( stg_model_t* mod, 
					     const char* prop, 
					     void* data, 
					     size_t len,
					     stg_property_storage_func_t func );
  
  /** gets the named property data. if len is non-NULL, it is set with
      the size of the data in bytes */
  void* stg_model_get_property( stg_model_t* mod, 
				const char* prop,
				size_t* len );
  
  /** gets a property of a known size. Fail assertion if the size isn't right.
   */
  void* stg_model_get_property_fixed( stg_model_t* mod, 
				      const char* name,
				      size_t size );

  void stg_model_property_refresh( stg_model_t* mod, const char* propname );


  /// gets a model's "polygons" property and fills poly_count with the
  /// number of polygons to be found
  stg_polygon_t* stg_model_get_polygons( stg_model_t* mod, size_t* poly_count );
  void stg_model_set_polygons( stg_model_t* mod,
			       stg_polygon_t* polys, 
			       size_t poly_count );

  // get a copy of the property data - caller must free it
  //int stg_model_copy_property_data( stg_model_t* mod, const char* prop,
  //			   void** data );
  
  int stg_model_add_property_callback( stg_model_t* mod, const char* prop, 
				       stg_property_callback_t, void* user );
  
  int stg_model_remove_property_callback( stg_model_t* mod, const char* prop, 
					  stg_property_callback_t );
  
  int stg_model_remove_property_callbacks( stg_model_t* mod, const char* prop );

  /** print human-readable information about the model on stdout
   */
  void stg_model_print( stg_model_t* mod );
  
  /** returns TRUE iff [testmod] exists above [mod] in a model tree 
   */
  int stg_model_is_antecedent( stg_model_t* mod, stg_model_t* testmod );
  
  /** returns TRUE iff [testmod] exists below [mod] in a model tree 
   */
  int stg_model_is_descendent( stg_model_t* mod, stg_model_t* testmod );
  
  /** returns TRUE iff [mod1] and [mod2] both exist in the same model
      tree 
  */
  int stg_model_is_related( stg_model_t* mod1, stg_model_t* mod2 );

  /** return the top-level model above mod */
  stg_model_t* stg_model_root( stg_model_t* mod );

  /** add a pointer to each model in the tree starting at root to the
      array. Returns the number of model pointers added */
  int stg_model_tree_to_ptr_array( stg_model_t* root, GPtrArray* array );

  /** initialize a model - called when a model goes from zero to one subscriptions */
  int stg_model_startup( stg_model_t* mod );

  /** finalize a model - called when a model goes from one to zero subscriptions */
  int stg_model_shutdown( stg_model_t* mod );


  int stg_model_update( stg_model_t* model );

  /** convert a global pose into the model's local coordinate system */
  void stg_model_global_to_local( stg_model_t* mod, stg_pose_t* pose );
  void stg_model_local_to_global( stg_model_t* mod, stg_pose_t* pose );

 
  /**@}*/


  // BLOBFINDER MODEL --------------------------------------------------------
  
  /** @defgroup stg_model_blobfinder Blobfinder
      Implements the blobfinder  model.
      @{ */
  
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
    stg_radians_t pan, tilt, zoom;
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

  /** Create a new blobfinder model 
   */
  stg_model_t* stg_blobfinder_create( stg_world_t* world,	
				      stg_model_t* parent, 
				      stg_id_t id,  
				      char* token );   
  /**@}*/

  // LASER MODEL --------------------------------------------------------
  
  /** @defgroup stg_model_laser Laser range scanner
      Implements the laser model: emulates a scanning laser rangefinder
      @{ */
  
  /** laser sample packet
   */
  typedef struct
  {
    uint32_t range; ///< range to laser hit in mm
    char reflectance; ///< intensity of the reflection 0-4
  } stg_laser_sample_t;
  
  /** laser configuration packet
   */
  typedef struct
  {
    //stg_geom_t geom;
    stg_radians_t fov; ///< field of view 
    stg_meters_t range_max; ///< the maximum range
    stg_meters_t range_min; ///< the miniimum range
    int samples; ///< the number of range measurements (and thus the size
    ///< of the array of stg_laser_sample_t's returned)
  } stg_laser_config_t;
  
  /** print human-readable version of the laser config struct
   */
  void stg_print_laser_config( stg_laser_config_t* slc );
  
  /** Create a new laser model 
   */
  stg_model_t* stg_laser_create( stg_world_t* world, 
				 stg_model_t* parent, 
				 stg_id_t id,
				 char* token );

  /// wrap the generic get_data call to be laser-specific. sizes are
  /// in terms of samples instead of bytes
  size_t stg_model_get_data_laser( stg_model_t* mod,
				   stg_laser_sample_t* data, 
				   size_t max_samples );
  /**@}*/

  // GRIPPER MODEL --------------------------------------------------------
  
  /** @defgroup stg_model_gripper Gripper 
      Implements the gripper model: emulates a pioneer 2D gripper
      @{ */
  
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
    
    stg_bool_t left_paddle_contact[3]; ///< non-zero iff left paddle touches something [1] inner, [2] front [3] outer contacts
    stg_bool_t right_paddle_contact[3]; ///< non-zero iff right paddle touches something[1] inner, [2] front [3] outer contacts
    
    stg_bool_t paddles_stalled; // true iff some solid object stopped
				// the paddles closing or opening

    int stack_count; ///< number of objects in stack

  } stg_gripper_data_t;


  /** print human-readable version of the gripper config struct
   */
  void stg_print_gripper_config( stg_gripper_config_t* slc );
  
  /** Create a new gripper model 
   */
  stg_model_t* stg_gripper_create( stg_world_t* world, 
				 stg_model_t* parent, 
				 stg_id_t id,
				 char* token );
  /**@}*/

  // FIDUCIAL MODEL --------------------------------------------------------
  
  /** @defgroup stg_model_fiducial Fidicial detector
      Implements the fiducial detector model
      @{ */

  /** fiducial config packet 
   */
  typedef struct
  {
    stg_meters_t max_range_anon;
    stg_meters_t max_range_id;
    stg_meters_t min_range;
    stg_radians_t fov; // field of view 
    stg_radians_t heading; // center of field of view

  } stg_fiducial_config_t;
  
  /** fiducial data packet 
   */
  typedef struct
  {
    stg_meters_t range; // range to the target
    stg_radians_t bearing; // bearing to the target 
    stg_pose_t geom; // size and relative angle of the target
    int id; // the identifier of the target, or -1 if none can be detected.
    
  } stg_fiducial_t;

  /** Create a new fiducial model 
   */
  stg_model_t* stg_fiducial_create( stg_world_t* world,  
				    stg_model_t* parent,  
				    stg_id_t id, 
				    char* token );  
  /**@}*/
  
  // RANGER MODEL --------------------------------------------------------
  
  /** @defgroup stg_model_ranger Range finder
      Implements the ranger model: emulates
      sonar, IR, and non-scanning laser range sensors 
      @{ */

  typedef struct
  {
    stg_meters_t min, max;
  } stg_bounds_t;
  
  typedef struct
  {
    stg_bounds_t range; // min and max range of sensor
    stg_radians_t angle; // viewing angle of sensor
  } stg_fov_t;
  
  typedef struct
  {
    stg_pose_t pose;
    stg_size_t size;
    stg_bounds_t bounds_range;
    stg_radians_t fov;
  } stg_ranger_config_t;
  
  typedef struct
  {
    stg_meters_t range;
    //double error; // TODO
  } stg_ranger_sample_t;
  
  /** Create a new ranger model 
   */
  stg_model_t* stg_ranger_create( stg_world_t* world,  
				  stg_model_t* parent, 
				  stg_id_t id, 
				  char* token );
  /**@}*/
  
  // POSITION MODEL --------------------------------------------------------
  
  /** @defgroup stg_model_position Position
      Implements the position model: a mobile robot base
      @{ */
  
#define STG_MM_POSITION_RESETODOM 77
  
  typedef enum
    { STG_POSITION_CONTROL_VELOCITY, STG_POSITION_CONTROL_POSITION }
    stg_position_control_mode_t;
  
  /** "position_drive" property */
  typedef enum
    { STG_POSITION_DRIVE_DIFFERENTIAL, STG_POSITION_DRIVE_OMNI }
  stg_position_drive_mode_t;
  
  /** "position_cmd" property */
  typedef struct
  {
    stg_meters_t x,y,a;
    stg_position_control_mode_t mode;
  } stg_position_cmd_t;

  /** "position_odom" property */
  typedef struct
  {
    stg_pose_t pose;
    stg_pose_t error;
  } stg_position_pose_estimate_t;
  
  /** "position_stall" property */
  typedef int stg_position_stall_t;

  /** "position_data" property */
/*   typedef struct */
/*   { */
/*     stg_pose_t pose; // current position estimate */
/*     stg_pose_t pose_error; // error estimate of the pose estimate */
/*     stg_velocity_t velocity; // current velocity estimate */
/*     stg_bool_t stall; // motors stalled flag */
/*   } stg_position_data_t; */

  /// create a new position model
  stg_model_t* stg_position_create( stg_world_t* world,  stg_model_t* parent,  stg_id_t id, char* token );
  
  /// set the current odometry estimate 
  void stg_model_position_set_odom( stg_model_t* mod, stg_pose_t* odom ); 

  /**@}*/
  
  // end the group of all models
  /**@}*/
  


// MACROS ------------------------------------------------------
// Some useful macros


#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define MILLION 1e6
#define BILLION 1e9

#ifndef M_PI
#define M_PI 3.14159265358979323846
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


#ifdef __cplusplus
}
#endif 

// end documentation group libstage
/**@}*/

#endif
