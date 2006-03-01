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
 * CVS: $Id: stage.h,v 1.181 2006-03-01 19:24:43 rtv Exp $
 */


/*! \file stage.h 
  Stage library header file

  This header file contains the external interface for the Stage
  library
*/

#include "config.h" // results of autoconf's system configuration tests
#include "replace.h" // Stage's implementations of missing system calls

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
  stg_init( argc, argv );

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


/** any integer value other than this is a valid fiducial ID 
 */
#define FiducialNone 0

#define STG_TOKEN_MAX 64

/** @defgroup types Measurement Types
    Basic self-describing measurement types. All packets with real
    measurements are specified in these terms so changing types here
    should work throughout the code.
  @{
*/    

  /** uniquely identify a model */
  typedef int stg_id_t;
  
  /** Metres: unit of distance */
  typedef double stg_meters_t;
  
  /** Radians: unit of angle */
  typedef double stg_radians_t;
  
  /** Milliseconds: unit of (short) time */
  typedef unsigned long stg_msec_t;

  /** Kilograms: unit of mass */
  typedef double stg_kg_t; // Kilograms (mass)

  /** Joules: unit of energy */
  typedef double stg_joules_t;

  /** Watts: unit of power (energy/time) */
  typedef double stg_watts_t; 

  /** boolean */
  typedef int stg_bool_t;

  //typedef double stg_friction_t;
  
  /** 24-bit RGB color packed 0x00RRGGBB */
  typedef uint32_t stg_color_t;

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
    stg_meters_t x, y;
  } stg_size_t;
  
  /** \struct stg_pose_t
      Specify a 3 axis position, in x, y and heading.
   */
  typedef struct
  {
    stg_meters_t x, y, a;
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


  // GUIFEATURES -------------------------------------------------------
  
  
 // forward declare struct types from player_internal.h
  struct _stg_model;
  struct _stg_matrix;
  struct _gui_window;
  struct _stg_world;

  typedef struct _stg_model stg_model_t; //  defined in stage_internal.
  typedef struct _stg_world stg_world_t; 



  /** Returns the real (wall-clock) time in milliseconds since the
      simulation started. */
  stg_msec_t stg_timenow( void );
  
  /** Initialize the stage library. Optionally pass in the arguments
      from main(), so Stage can read cmdline options. Stage then
      passes the arguments to GTK+ and Xlib so they can read their own
      options.
  */
  int stg_init( int argc, char** argv );
  
  
  /// if stage wants to quit, this will return non-zero
  int stg_quit_test( void );

  /** set stage's quit flag. Stage will quit cleanly very soon after
      this function is called. */
  void stg_quit_request( void );

  
  /** Get a string identifying the version of stage. The string is
      generated by autoconf 
  */
  const char* stg_version_string( void );


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

  /** Create an array of [count] points. Caller must free the returned
      pointer, preferably with stg_points_destroy(). */
  stg_point_t* stg_points_create( size_t count );

  /** frees a point array */
  void stg_points_destroy( stg_point_t* pts );

  /*@}*/

  // POLYLINES ---------------------------------------------------------

  /** @ingroup libstage
      @defgroup stg_polyline Polylines
      Defines a line defined by multiple points.
      @{ 
  */

  /** define a polyline: a set of connected vertices */
  typedef struct
  {
    stg_point_t* points; ///< Pointer to an array of points
    size_t points_count; ///< Number of points in array   
  } stg_polyline_t; 

  /*@}*/

  // POLYGONS ---------------------------------------------------------
  
  /** @ingroup libstage
      @defgroup stg_polygon Polygons
      Creating and manipulating polygons
      @{
  */
  
  /** define a polygon: a set of connected vertices drawn with a
      color. Can be drawn filled or unfilled. */
  typedef struct
  {
    /// pointer to an array of points
    GArray* points;
    
    /// if TRUE, this polygon is NOT drawn filled
    stg_bool_t unfilled; 
    
    /// render color of this polygon - TODO  - implement color rendering
    stg_color_t color;

    /// bounding box: width and height of the polygon
    stg_size_t bbox;

    void* _data; // temporary internal use only
  } stg_polygon_t; 

  
  /// return an array of [count] polygons. Caller must free() the space.
  stg_polygon_t* stg_polygons_create( int count );
  
  /// destroy an array of [count] polygons
  void stg_polygons_destroy( stg_polygon_t* p, size_t count );
  
  /// creates a unit square polygon
  stg_polygon_t* stg_unit_polygon_create( void );
    
  /// Copies [count] points from [pts] into polygon [poly], allocating
  /// memory if mecessary. Any previous points in [poly] are
  /// overwritten.
  void stg_polygon_set_points( stg_polygon_t* poly, stg_point_t* pts, size_t count );			       
  /// Appends [count] points from [pts] into polygon [poly],
  /// allocating memory if mecessary.
  void stg_polygon_append_points( stg_polygon_t* poly, stg_point_t* pts, size_t count );			       
  
  /// scale the array of [num] polygons so that all its points fit
  /// exactly in a rectagle of pwidth] by [height] units
  void stg_polygons_normalize( stg_polygon_t* polys, int num, 
			       double width, double height );
  
  /// print a human-readable description of a polygon on stdout
  void stg_polygon_print( stg_polygon_t* poly );
  
  /// print a human-readable description of an array of polygons on stdout
  void stg_polygons_print( stg_polygon_t* polys, unsigned int count );
  
  /** Interpret a bitmap file as a set of polygons. Returns an array
      of polygons. On exit [poly_count] is the number of polygons
      found.
   */
  stg_polygon_t* stg_polygons_from_image_file(  const char* filename, 
						size_t* poly_count );
  
  /**@}*/

  // end property typedefs -------------------------------------------------


 //  WORLD --------------------------------------------------------

  /** @ingroup libstage
      @defgroup stg_world Worlds
     Implements a world - a collection of models and a matrix.   
     @{
  */
  
  /** \struct stg_world_t
      Opaque data structure implementing a world.      
      Use the documented stg_world_<something> functions to manipulate
      worlds. Do not modify the structure directly unless you know
      what you are doing.
   */

  /** Create a new world, to be configured and populated
      manually. Usually this function is not used directly; use the
      function stg_world_create_from_file() to create a world based on
      a worldfile instead.
   */
  stg_world_t* stg_world_create( stg_id_t id, 
				 const char* token, 
				 int sim_interval, 
				 int real_interval,
				 double ppm,
				 double width,
				 double height );

  /** Create a new world as described in the worldfile
      [worldfile_path]
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

  /** Run one simulation step in world [world]. If [sleepflag] is
      non-zero, and the simulation update takes less than one
      real-time step, the simulation will nanosleep() for a while to
      reduce CPU load. Returns 0 if all is well, or a positive error
      code.
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

  /** look up a pointer to a model in [world] from the model's unique
      ID [mid]. */ 
  stg_model_t* stg_world_get_model( stg_world_t* world, stg_id_t mid );
  
  /** look up a pointer to a model from from the model's name. */
  stg_model_t* stg_world_model_name_lookup( stg_world_t* world, const char* name );
  
  /**@}*/

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

  /** \struct stg_model_t 
      Opaque data structure implementing a model.
      Use the documented stg_model_<something> functions to manipulate
      models. Do not modify the structure directly unless you know
      what you are doing.
   */

  /** Define a callback function type that can be attached to a
      record within a model and called whenever the record is set.
  */
  typedef int (*stg_model_callback_t)(stg_model_t* mod, void* user );

  void stg_model_add_callback( stg_model_t* mod, 
			       void* member, 
			       stg_model_callback_t cb, 
			       void* user );

  int stg_model_remove_callback( stg_model_t* mod,
				 void* member,
				 stg_model_callback_t callback );

  
  /** function type for an initialization function that configures a
      specialized model. Each special model type (laser, position,
      etc) has a single initializer function that is called when the
      model type is specified in the worldfile. The mapping is done in
      a table in typetable.cc.
  */
  typedef int(*stg_model_initializer_t)(stg_model_t*);
  

  
  /// laser return value
  typedef enum 
    {
      LaserTransparent, ///<not detected by laser model 
      LaserVisible, ///< detected by laser with a reflected intensity of 0 
      LaserBright  ////< detected by laser with a reflected intensity of 1 
    } stg_laser_return_t;

  /// create a new model
  stg_model_t* stg_model_create(  stg_world_t* world,
				  stg_model_t* parent, 
				  stg_id_t id, 
				  char* typestr );
  
  /** destroy a model, freeing its memory */
  void stg_model_destroy( stg_model_t* mod );

  /** get the pose of a model in the global CS */
  void stg_model_get_global_pose( stg_model_t* mod, stg_pose_t* pose );

  /** get the velocity of a model in the global CS */
  void stg_model_get_global_velocity( stg_model_t* mod, stg_velocity_t* gvel );

  /* set the velocity of a model in the global coordinate system */
  void stg_model_set_global_velocity( stg_model_t* mod, stg_velocity_t* gvel );

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
  void stg_model_set_global_pose( stg_model_t* mod, stg_pose_t* gpose );
  
  /** set a model's velocity in its parent's coordinate system */
  void stg_model_set_velocity( stg_model_t* mod, stg_velocity_t* vel );
 
  /** set a model's pose in its parent's coordinate system */
  void stg_model_set_pose( stg_model_t* mod, stg_pose_t* pose );

  /** set a model's geometry (size and center offsets) */
  void stg_model_set_geom( stg_model_t* mod, stg_geom_t* src );

  /** set a model's geometry (size and center offsets) */
  void stg_model_set_fiducial_return( stg_model_t* mod, int fid );

  /** set a model's fiducial key: only fiducial finders with a
      matching key can detect this model as a fiducial. */
  void stg_model_set_fiducial_key( stg_model_t* mod, int key );

  /** Change a model's parent - experimental*/
  int stg_model_set_parent( stg_model_t* mod, stg_model_t* newparent);
  
  /** Get a model's geometry - it's size and local pose (offset from
      origin in local coords) */
  void stg_model_get_geom( stg_model_t* mod, stg_geom_t* dest );

  /** Get the pose of a model in its parent's coordinate system  */
  void stg_model_get_pose( stg_model_t* mod, stg_pose_t* dest );

  /** Get a model's velocity (in its local reference frame) */
  void stg_model_get_velocity( stg_model_t* mod, stg_velocity_t* dest );
  
  /** gets a model's "polygons" property and fills poly_count with the
      number of polygons to be found */
  stg_polygon_t* stg_model_get_polygons( stg_model_t* mod, size_t* poly_count );
  void stg_model_set_polygons( stg_model_t* mod,
			       stg_polygon_t* polys, 
			       size_t poly_count );
  
  /** set an array oflines to be drawn for the model */
  void stg_model_set_lines( stg_model_t* mod,
			    stg_polyline_t* lines, 
			    size_t lines_count );
  
  // guess what these do?
  void stg_model_get_velocity( stg_model_t* mod, stg_velocity_t* dest );
  void stg_model_set_velocity( stg_model_t* mod, stg_velocity_t* vel );
  void stg_model_set_pose( stg_model_t* mod, stg_pose_t* pose );
  void stg_model_get_geom( stg_model_t* mod, stg_geom_t* dest );
  void stg_model_set_geom( stg_model_t* mod, stg_geom_t* geom );
  void stg_model_set_color( stg_model_t* mod, stg_color_t col );
  void stg_model_set_mass( stg_model_t* mod, stg_kg_t mass );
  void stg_model_set_stall( stg_model_t* mod, stg_bool_t stall );
  void stg_model_set_gripper_return( stg_model_t* mod, int val );
  void stg_model_set_laser_return( stg_model_t* mod, int val );
  void stg_model_set_obstacle_return( stg_model_t* mod, int val );
  void stg_model_set_blob_return( stg_model_t* mod, int val );
  void stg_model_set_ranger_return( stg_model_t* mod, int val );
  void stg_model_set_boundary( stg_model_t* mod, int val );
  void stg_model_set_gui_nose( stg_model_t* mod, int val );
  void stg_model_set_gui_mask( stg_model_t* mod, int val );
  void stg_model_set_gui_grid( stg_model_t* mod, int val );
  void stg_model_set_gui_outline( stg_model_t* mod, int val );
  void stg_model_set_watts( stg_model_t* mod, stg_watts_t watts );
  void stg_model_set_map_resolution( stg_model_t* mod, stg_meters_t res );


  /** print human-readable information about the model on stdout. If
      prefix is non-null, it is printed first.
   */
  void stg_model_print( stg_model_t* mod, char* prefix );
  
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
  
  /** Convert a tree of models into a GPtrArray containing the same models.*/
  GPtrArray* stg_model_array_from_tree( stg_model_t* root );
  
  /** initialize a model - called when a model goes from zero to one subscriptions */
  int stg_model_startup( stg_model_t* mod );

  /** finalize a model - called when a model goes from one to zero subscriptions */
  int stg_model_shutdown( stg_model_t* mod );

  /** Update a model by one simulation timestep. This is called by
      stg_world_update(), so users don't usually need to call this. */
  int stg_model_update( stg_model_t* model );
  
  /** Convert a pose in the world coordinate system into a model's
      local coordinate system. Overwrites [pose] with the new
      coordinate. */
  void stg_model_global_to_local( stg_model_t* mod, stg_pose_t* pose );
  
  /** Convert a pose in the model's local coordinate system into the
      world coordinate system. Overwrites [pose] with the new
      coordinate. */
  void stg_model_local_to_global( stg_model_t* mod, stg_pose_t* pose );

  int stg_model_fig_clear_cb( stg_model_t* mod, void* data, size_t len, 
			      void* userp );

  void stg_model_set_data( stg_model_t* mod, void* data, size_t len );
  void stg_model_set_cmd( stg_model_t* mod, void* cmd, size_t len );
  void stg_model_set_cfg( stg_model_t* mod, void* cfg, size_t len );
  void* stg_model_get_cfg( stg_model_t* mod, size_t* lenp );
  void* stg_model_get_data( stg_model_t* mod, size_t* lenp );
  void* stg_model_get_cmd( stg_model_t* mod, size_t* lenp );
  

  /** add an item to the View menu that will automatically install and
      remove a callback when the item is toggled. The specialized
      model types use this call to set up their data visualization. */
  void stg_model_add_property_toggles( stg_model_t* mod,
				       void* member,
				       stg_model_callback_t callback_on,
				       void* arg_on,
				       stg_model_callback_t callback_off,
				       void* arg_off,
				       const char* name,
				       const char* label,
				       gboolean enabled );


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
    uint32_t range; ///< range to laser hit in mm
    char reflectance; ///< intensity of the reflection 0-4
  } stg_laser_sample_t;
  
  /** laser configuration packet
   */
  typedef struct
  {
    stg_radians_t fov; ///< field of view 
    stg_meters_t range_max; ///< the maximum range
    stg_meters_t range_min; ///< the miniimum range

    /** the number of range measurements (and thus the size
    of the array of stg_laser_sample_t's returned) */ 
    int samples; 
  } stg_laser_config_t;
  
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
    int id; ///< the identifier of the target, or -1 if none can be detected.
    
  } stg_fiducial_t;

  // RANGER MODEL --------------------------------------------------------

  typedef struct
  {
    stg_pose_t pose;
    stg_size_t size;
    stg_bounds_t bounds_range;
    stg_radians_t fov;
    int ray_count;
  } stg_ranger_config_t;
  
  typedef struct
  {
    stg_meters_t range;
    //double error; // TODO
  } stg_ranger_sample_t;
  
  
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
  void stg_model_position_set_odom( stg_model_t* mod, stg_pose_t* odom ); 

  // end the group of all models
  /**@}*/
  


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
  void stg_print_gripper_config( stg_gripper_config_t* slc );
  /** Print human-readable version of the laser config struct */
  void stg_print_laser_config( stg_laser_config_t* slc );
  


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



/** @} */

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

// end doc group libstage_utilities
/** @} */ 

#ifdef __cplusplus
}
#endif 

// end documentation group libstage
/**@}*/

#endif
