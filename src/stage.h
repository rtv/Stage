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
 * Desc: Types and functions for the Stage library
 * Author: Richard Vaughan vaughan@sfu.ca 
 * Date: 1 June 2003
 *
 * CVS: $Id: stage.h,v 1.115 2004-12-13 05:52:04 rtv Exp $
 */

/*! \file stage.h 
  \brief Stage library header file

  This is the header file that contains the external interface for the
  Stage library
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
#include <rtk.h> // andRTK's graphics stuff too

#ifdef __cplusplus
extern "C" {
#endif 

#include "config.h"
#include "replace.h"

#define STG_TOKEN_MAX 64

  // TODO - override from command line and world config request
#define STG_DEFAULT_WORLD_INTERVAL_MS 100 // 10Hz

  // Movement masks for figures  - these should match their RTK equivalents
#define STG_MOVE_TRANS (1 << 0)
#define STG_MOVE_ROT   (1 << 1)
#define STG_MOVE_SCALE (1 << 2)


#define STG_DEFAULT_WINDOW_WIDTH 700
#define STG_DEFAULT_WINDOW_HEIGHT 740
#define STG_DEFAULT_PPM 40
#define STG_DEFAULT_SHOWGRID 1
#define STG_DEFAULT_MOVIE_SPEED 1


  typedef enum
    {
      STG_MODEL_BASIC=0,
      STG_MODEL_POSITION,
      STG_MODEL_TEST,
      STG_MODEL_LASER,
      STG_MODEL_FIDUCIAL,
      STG_MODEL_RANGER,
      STG_MODEL_BLOB,
      STG_MODEL_COUNT // this must be the last entry - it counts the entries

    } stg_model_type_t;


  //  const char* stg_model_type_string( stg_model_type_t t );

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

#define HTON_M(m) htonl(m)   // byte ordering for METERS
#define NTOH_M(m) ntohl(m)
#define HTON_RAD(r) htonl(r) // byte ordering for RADIANS
#define NTOH_RAD(r) ntohl(r)
#define HTON_SEC(s) htonl(s) // byte ordering for SECONDS
#define NTOH_SEC(s) ntohl(s)
#define HTON_SEC(s) htonl(s) // byte ordering for SECONDS
#define NTOH_SEC(s) ntohl(s)


  typedef struct 
  {
    stg_meters_t x, y;
  } stg_size_t;

  typedef uint32_t stg_color_t;

  // Look up the color in the X11 database.  (i.e. transform color name to
  // color value).  If the color is not found in the database, a bright
  // red color will be returned instead.
  stg_color_t stg_lookup_color(const char *name);

  // used for specifying 3 axis positions
  typedef struct
  {
    stg_meters_t x, y, a;
  } stg_pose_t;

  typedef struct
  {
    stg_pose_t pose;
    stg_size_t size;
  } stg_geom_t;


  // returns a string that describes a property type from the enum above
  const char* stg_property_string( stg_id_t id );

  void stg_print_geom( stg_geom_t* geom );

  // POSITION -------------------------------------------------------------

#define STG_MM_POSITION_RESETODOM 77

  typedef enum
    { STG_POSITION_CONTROL_VELOCITY, STG_POSITION_CONTROL_POSITION }
  stg_position_control_mode_t;

  typedef enum
    { STG_POSITION_STEER_DIFFERENTIAL, STG_POSITION_STEER_INDEPENDENT }
  stg_position_steer_mode_t;

  typedef struct
  {
    stg_meters_t x,y,a;
    stg_position_control_mode_t mode; 
  } stg_position_cmd_t;

  typedef struct
  {
    stg_position_steer_mode_t steer_mode;
    //stg_bool_t motor_disable; // if non-zero, the motors are disabled
    //stg_pose_t odometry
  } stg_position_cfg_t;

  typedef stg_pose_t stg_velocity_t;

  typedef struct
  {
    stg_pose_t pose; // current position estimate
    stg_velocity_t velocity; // current velocity estimate
    stg_bool_t stall; // motors stalled flag
  } stg_position_data_t;

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

  typedef struct
  {
    int enable;
    stg_msec_t period;
  } stg_blinkenlight_t;

  // GRIPPER ------------------------------------------------------------

  // Possible Gripper return values
  typedef enum 
    {
      GripperDisabled = 0,
      GripperEnabled
    } stg_gripper_return_t;



  // RANGER ------------------------------------------------------------

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

  //typedef struct
  //{
  //int sensor_count;
  //stg_ranger_sensor_t sensors[ STG_RANGER_SENSOR_MAX ];
  //} stg_ranger_config_t;

  typedef struct
  {
    stg_meters_t range;
    //double error; // TODO
  } stg_ranger_sample_t;

  // RECTS --------------------------------------------------------------

  typedef struct
  {
    stg_meters_t x, y, w, h;
  } stg_rect_t;

  typedef struct
  {
    stg_pose_t pose;
    stg_size_t size;
  } stg_rotrect_t; // rotated rectangle

  // specify a line from (x1,y1) to (x2,y2), all in meters
  typedef struct
  {
    stg_meters_t x1, y1, x2, y2;
  } stg_line_t;

  typedef struct
  {
    int toplx, toply, toprx, topry, botlx, botly, botrx, botry;
  } stg_corners_t;

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
 
  // MOVEMASK ---------------------------------------------------------
   
  typedef int stg_movemask_t;

  typedef struct
  {
    uint8_t show_data;
    uint8_t show_cfg;
    uint8_t show_cmd;

    uint8_t nose;
    uint8_t grid;
    uint8_t boundary;
    stg_movemask_t movemask;
  } stg_guifeatures_t;


  // LASER  ------------------------------------------------------------

  /** @addtogroup stg_laser */
  /** @{ */ 

  /// laser return value
  typedef enum 
    {
      LaserTransparent, ///<not detected by laser model 
      LaserVisible, ///< detected by laser with a reflected intensity of 0 
      LaserBright  ////< detected by laser with a reflected intensity of 1 
    } stg_laser_return_t;


  /// the default value
#define STG_DEFAULT_LASERRETURN LaserVisible

  ///laser data structure
  typedef struct
  {
    uint32_t range; ///< range to laser hit in mm
    char reflectance; ///< intensity of the reflection 0-4
  } stg_laser_sample_t;

  /// laser configuration structure
  typedef struct
  {
    //stg_geom_t geom;
    stg_radians_t fov; ///< field of view 
    stg_meters_t range_max; ///< the maximum range
    stg_meters_t range_min; ///< the miniimum range
    int samples; ///< the number of range measurements (and thus the size
    ///< of the array of stg_laser_sample_t's returned)
  } stg_laser_config_t;

  /** @} */

  // print human-readable version of the struct
  void stg_print_laser_config( stg_laser_config_t* slc );

  // BlobFinder ------------------------------------------------------------

#define STG_BLOB_CHANNELS_MAX 16

  typedef struct
  {
    int channel_count; // 0 to STG_BLOBFINDER_CHANNELS_MAX
    stg_color_t channels[STG_BLOB_CHANNELS_MAX];
    int scan_width;
    int scan_height;
    stg_meters_t range_max;
    stg_radians_t pan, tilt, zoom;
  } stg_blobfinder_config_t;

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


  // fiducial finder ------------------------------------------------------

  // any integer value other than this is a valid fiducial ID
  // TODO - fix this up
#define FiducialNone 0

  typedef int stg_fiducial_return_t;

  typedef struct
  {
    stg_meters_t max_range_anon;
    stg_meters_t max_range_id;
    stg_meters_t min_range;
    stg_radians_t fov; // field of view 
    stg_radians_t heading; // center of field of view

  } stg_fiducial_config_t;


  typedef struct
  {
    stg_meters_t range; // range to the target
    stg_radians_t bearing; // bearing to the target 
    stg_pose_t geom; // size and relative angle of the target
    int id; // the identifier of the target, or -1 if none can be detected.

  } stg_fiducial_t;

  // end property typedefs -------------------------------------------------

  // get property structs with default values filled in --------------------
  
  void stg_get_default_pose( stg_pose_t* pose );
  void stg_get_default_geom( stg_geom_t* geom );
  
  // end defaults --------------------------------------------
  

  /// Structure that holds the GUI figure for each model
  typedef struct
  {
    rtk_fig_t* top;
    rtk_fig_t* geom;
    rtk_fig_t* grid;
    rtk_fig_t* data;
    rtk_fig_t* cmd;
    rtk_fig_t* cfg;
    rtk_fig_t* data_bg; // background (used e.g for laser scan fill)
    rtk_fig_t* cmd_bg; 
    rtk_fig_t* cfg_bg; 
  } gui_model_t;

  // forward declare
  struct _stg_world; 
  struct _stg_model;

  typedef struct 
  {
    rtk_canvas_t* canvas;
  
    struct _stg_world* world; // every window shows a single world

    // rtk doesn't support status bars, so we'll use gtk directly
    GtkStatusbar* statusbar;
    GtkLabel* timelabel;

    int wf_section; // worldfile section for load/save

    rtk_fig_t* bg; // background
    rtk_fig_t* matrix;
    rtk_fig_t* poses;

    gboolean show_matrix;  
    gboolean fill_polygons;
    gboolean show_geom;
    
    //gboolean movie_exporting;
    //int movie_speed;
    //int movie_count;
    //int movie_tag;

    int frame_series;
    int frame_index;
    int frame_callback_tag;
    int frame_interval;
    int frame_format;

    rtk_menu_t** menus;
    rtk_menuitem_t** mitems;
    int menu_count;
    int mitem_count;
  
    rtk_menuitem_t** mitems_mspeed;
    int mitems_mspeed_count;
  
    struct _stg_model* selection_active;
  
    uint8_t render_data_flag[STG_MODEL_COUNT];
    uint8_t render_cfg_flag[STG_MODEL_COUNT];
    uint8_t render_cmd_flag[STG_MODEL_COUNT];

  } gui_window_t;

  typedef struct 
  {
    double ppm; // pixels per meter (1/resolution)
    GHashTable* table;
  
    double medppm;
    GHashTable* medtable;

    double bigppm;
    GHashTable* bigtable;
    
    long array_width, array_height, array_origin_x, array_origin_y;
    // faster than hash tables but take up more space
    GPtrArray* array;
    GPtrArray* medarray;
    GPtrArray* bigarray;

    // todo - record a timestamp for matrix mods so devices can see if
    //the matrix has changed since they last peeked into it
    // stg_msec_t last_mod_time;
  } stg_matrix_t;
  
  typedef struct
  {
    glong x; // address a very large space of cells
    glong y;
  } stg_matrix_coord_t;


  //typedef void(*func_init_t)(struct _stg_model*);
  typedef int(*func_update_t)(struct _stg_model*);
  typedef int(*func_startup_t)(struct _stg_model*);
  typedef int(*func_shutdown_t)(struct _stg_model*);

  typedef int(*func_set_command_t)(struct _stg_model*,void*,size_t);
  typedef int(*func_set_data_t)(struct _stg_model*,void*,size_t);
  typedef int(*func_set_config_t)(struct _stg_model*,void*,size_t);

  typedef void*(*func_get_command_t)(struct _stg_model*,size_t*);
  typedef void*(*func_get_data_t)(struct _stg_model*,size_t*);
  typedef void*(*func_get_config_t)(struct _stg_model*,size_t*);

  typedef void(*func_data_notify_t)( void* );

  typedef void(*func_render_t)(struct _stg_model*,void*,size_t);


  /// defines a simulated world
  typedef struct _stg_world
  {
    stg_id_t id; // Stage's identifier for this world
   
    GHashTable* models; // the models that make up the world, indexed by id
    GHashTable* models_by_name; // the models that make up the world, indexed by name
  
    // the number of models of each type is counted so we can
    // automatically generate names for them
    int child_type_count[ STG_MODEL_COUNT ];
   
    stg_matrix_t* matrix;

    char* token;


    stg_msec_t sim_time; // the current time in this world
    stg_msec_t sim_interval; // this much simulated time elapses each step.
   
    stg_msec_t wall_interval; // real-time interval between updates -
			      // set this to zero for 'as fast as possible'

    stg_msec_t wall_last_update; // the wall-clock time of the last update

    // the wallclock-time interval elapsed between the last two
    // updates - compare this with sim_interval to see the ratio of
    // sim to real time.
    stg_msec_t real_interval_measured;

    double ppm; // the resolution of the world model in pixels per meter

    int paused; // the world only updates when this is zero
   
    gboolean destroy;

    gui_window_t* win; // the gui window associated with this world
   
    ///  a hooks for the user to store things in the world
    void* user;
    size_t user_len;

  } stg_world_t;

  /** @addtogroup stg_model
      structure that describes a model
  */
  typedef struct _stg_model
  {
    stg_id_t id; // used as hash table key
    stg_world_t* world; // pointer to the world in which this model exists
    char* token; // automatically-generated unique ID string
    int type; // what kind of a model am I?

    struct _stg_model *parent; // the model that owns this one, possibly NULL

    GPtrArray* children; // the models owned by this model

    // the number of children of each type is counted so we can
    // automatically generate names for them
    int child_type_count[ STG_MODEL_COUNT ];

    int subs;     // the number of subscriptions to this model
    gui_model_t gui; // all the gui stuff    
    stg_msec_t interval; // time between updates in ms
    stg_msec_t interval_elapsed; // time since last update in ms
  
    // todo - thread-safe version
    // allow exclusive access to this model
    
    // the generic buffers used by specialized model types
    void *data, *cmd, *cfg;
    size_t data_len, cmd_len, cfg_len;
    pthread_mutex_t data_mutex, cmd_mutex, cfg_mutex;
    
    // an array of polygons that make up the model's body. Possibly
    // zero elements.
    GArray* polygons;
    
    // basic model properties
    stg_bool_t blob_return; 
    stg_laser_return_t laser_return; // value returned to a laser sensor
    stg_bool_t obstacle_return; // if TRUE, we are included in obstacle detection
    stg_fiducial_return_t fiducial_return; // value returned to a fiducial finder
    stg_pose_t pose; // current pose in parent's CS
    stg_pose_t odom; // accumulate odometric pose estimate
    stg_velocity_t velocity; // current velocity
    stg_bool_t stall; // true IFF we hit an obstacle
    stg_geom_t geom; // pose and size in local CS
    stg_color_t color; // RGB color 
    stg_kg_t mass; // mass in kg
    stg_guifeatures_t guifeatures; // controls how we are rendered in the GUI

    // type-dependent functions for this model, implementing simple
    // polymorphism
    func_startup_t f_startup;
    func_shutdown_t f_shutdown;
    func_update_t f_update;
    func_render_t f_render_data;
    func_render_t f_render_cmd;
    func_render_t f_render_cfg;

    // pulled these out as I never use them
    //func_set_data_t f_set_data;
    //func_get_data_t f_get_data;
    //func_set_command_t f_set_command;
    //func_get_command_t f_get_command;
    //func_set_config_t f_set_config;
    //func_get_config_t f_get_config;
    
    /// if set, this callback is run when we do model_put_data() -
    /// it's used by the player plugin to notify Player that data is
    /// ready.
    func_data_notify_t data_notify;
    void* data_notify_arg;

    // todo - add this as a property?
    // stg_joules_t energy_consumed;
    // stg_energy_config_t energy_config;   // these are a little strange
    // stg_energy_data_t energy_data;
    // double friction; // units? our model doesn't have a direct physical value

  } stg_model_t;  
  
  
typedef enum 
  { PointToPoint=0, PointToBearingRange } 
itl_mode_t;


  /** @addtogroup stg_world */
  /** @{ */

  stg_world_t* stg_world_create( stg_id_t id, 
				 char* token, 
				 int sim_interval, 
				 int real_interval,
				 double ppm_high,
				 double ppm_med,
				 double ppm_low );
   
  stg_world_t* stg_world_create_from_file( char* worldfile_path );
  void stg_world_destroy( stg_world_t* world );
  int stg_world_update( stg_world_t* world, int sleepflag );
  void stg_world_print( stg_world_t* world );


  /// get a model pointer from its ID
  stg_model_t* stg_world_get_model( stg_world_t* world, stg_id_t mid );

  /// get a model pointer from its name
  stg_model_t* stg_world_model_name_lookup( stg_world_t* world, const char* name );

  /**@}*/

  /** @addtogroup stg_model */
  /** @{ */

  /// create a new model
  stg_model_t* stg_model_create(  stg_world_t* world,
				  stg_model_t* parent, 
				  stg_id_t id, 
				  stg_model_type_t type,
				  char* token );

  /** destroy a model, freeing its memory */
  void stg_model_destroy( stg_model_t* mod );

  /** get the pose of a model in the global CS */
  void stg_model_get_global_pose( stg_model_t* mod, stg_pose_t* pose );

  /** get the velocity of a model in the global CS */
  void stg_model_get_global_velocity( stg_model_t* mod, stg_velocity_t* gvel );

  /** set the velocity of a model in the global coordinate system */
  void stg_model_set_global_velocity( stg_model_t* mod, stg_velocity_t* gvel );

  /** subscribe to a model's data */
  void stg_model_subscribe( stg_model_t* mod );

  /** unsubscribe from a model's data */
  void stg_model_unsubscribe( stg_model_t* mod );

  /** initialize a model - called when a model goes from zero to one subscriptions */
  int stg_model_startup( stg_model_t* mod );

  /** finalize a model - called when a model goes from one to zero subscriptions */
  int stg_model_shutdown( stg_model_t* mod );

  // SET properties - use these to set props, don't set them directly

  /** set the pose of a model in its parent's coordinate system */
  int stg_model_set_pose( stg_model_t* mod, stg_pose_t* pose );
  
  /** set the pose of model in global coordinates */
  int stg_model_set_global_pose( stg_model_t* mod, stg_pose_t* gpose );
  
  /** set a model's odometry */
  int stg_model_set_odom( stg_model_t* mod, stg_pose_t* pose );

  /** set a model's velocity in it's parent's coordinate system */
  int stg_model_set_velocity( stg_model_t* mod, stg_velocity_t* vel );
  
  /** set a model's size */
  int stg_model_set_size( stg_model_t* mod, stg_size_t* sz );

  /** set a model's  */
  int stg_model_set_color( stg_model_t* mod, stg_color_t* col );

  /** set a model's geometry */
  int stg_model_set_geom( stg_model_t* mod, stg_geom_t* geom );

  /** set a model's mass */
  int stg_model_set_mass( stg_model_t* mod, stg_kg_t* mass );

  /** set a model's GUI features */
  int stg_model_set_guifeatures( stg_model_t* mod, stg_guifeatures_t* gf );

  // TODO
  /* set a model's energy configuration */
  // int stg_model_set_energy_config( stg_model_t* mod, stg_energy_config_t* gf );

  /* set a model's energy data*/
  // int stg_model_set_energy_data( stg_model_t* mod, stg_energy_data_t* gf );

  /** set a model's polygon array*/
  int stg_model_set_polygons( stg_model_t* mod, 
			      stg_polygon_t* polys, size_t poly_count );
  
  /** get a model's polygon array */
  stg_polygon_t* stg_model_get_polygons( stg_model_t* mod, size_t* count);
  
  /** set a model's obstacle return value */
  int stg_model_set_obstaclereturn( stg_model_t* mod, stg_bool_t* ret );

  /** set a model's laser return value */
  int stg_model_set_laserreturn( stg_model_t* mod, stg_laser_return_t* val );

  /** set a model's fiducial return value */
  int stg_model_set_fiducialreturn( stg_model_t* mod, stg_fiducial_return_t* val );

  /** set a model's friction*/
  int stg_model_set_friction( stg_model_t* mod, stg_friction_t* fricp );

  // GET properties - use these to get props - don't get them directly
  stg_velocity_t*        stg_model_get_velocity( stg_model_t* mod );
  stg_geom_t*            stg_model_get_geom( stg_model_t* mod );
  stg_color_t            stg_model_get_color( stg_model_t* mod );
  stg_pose_t*            stg_model_get_pose( stg_model_t* mod );
  stg_pose_t*            stg_model_get_odom( stg_model_t* mod );
  stg_kg_t*              stg_model_get_mass( stg_model_t* mod );
  stg_guifeatures_t*     stg_model_get_guifeatures( stg_model_t* mod );
  stg_bool_t             stg_model_get_obstaclereturn( stg_model_t* mod );
  stg_laser_return_t     stg_model_get_laserreturn( stg_model_t* mod );
  stg_fiducial_return_t*  stg_model_get_fiducialreturn( stg_model_t* mod );
  stg_friction_t*        stg_model_get_friction( stg_model_t* mod );

  //stg_energy_data_t*     stg_model_get_energy_data( stg_model_t* mod );
  //stg_energy_config_t*   stg_model_get_energy_config( stg_model_t* mod );

  // wrappers for polymorphic functions
  int stg_model_set_command( stg_model_t* mod, void* cmd, size_t len );
  int stg_model_set_data( stg_model_t* mod, void* data, size_t len );
  int stg_model_set_config( stg_model_t* mod, void* cmd, size_t len );
  int stg_model_get_command( stg_model_t* mod, void* dest, size_t len );
  int stg_model_get_data( stg_model_t* mod, void* dest, size_t len );
  int stg_model_get_config( stg_model_t* mod, void* dest, size_t len );
  
  // generic versions that can be overridden
  //void* _model_get_data( stg_model_t* mod, size_t* len );
  //void* _model_get_cmd( stg_model_t* mod, size_t* len );
  //void* _model_get_cfg( stg_model_t* mod, size_t* len );
  //int _model_set_data( stg_model_t* mod, void* data, size_t len );
  //int _model_set_cmd( stg_model_t* mod, void* cmd, size_t len );
  //int _model_set_cfg( stg_model_t* mod, void* cfg, size_t len );

  int _model_update( stg_model_t* mod );
  //int _model_init( stg_model_t* mod );
  int _model_startup( stg_model_t* mod );
  int _model_shutdown( stg_model_t* mod );
 
   void model_update_cb( gpointer key, gpointer value, gpointer user );
  void model_print_cb( gpointer key, gpointer value, gpointer user );


  /** convert a global pose into the model's local coordinate system */
  void stg_model_global_to_local( stg_model_t* mod, stg_pose_t* pose );
  
  int stg_model_update( stg_model_t* model );
  void stg_model_update_velocity( stg_model_t* model );
  int stg_model_update_pose( stg_model_t* model );
  void stg_model_print( stg_model_t* mod );
  void stg_model_local_to_global( stg_model_t* mod, stg_pose_t* pose );
  void stg_model_energy_consume( stg_model_t* mod, stg_watts_t rate );
  void stg_model_map( stg_model_t* mod, gboolean render );
  void stg_model_map_with_children( stg_model_t* mod, gboolean render );
  
  rtk_fig_t* stg_model_prop_fig_create( stg_model_t* mod, 
				    rtk_fig_t* array[],
				    stg_id_t propid, 
				    rtk_fig_t* parent,
				    int layer );

  /** returns TRUE iff [testmod] exists above [mod] in a model tree */
  int stg_model_is_antecedent( stg_model_t* mod, stg_model_t* testmod );

  /** returns TRUE iff [testmod] exists below [mod] in a model tree */
  int stg_model_is_descendent( stg_model_t* mod, stg_model_t* testmod );

  /** returns TRUE iff [mod1] and [mod2] both exist in the same model
      tree */
  int stg_model_is_related( stg_model_t* mod1, stg_model_t* mod2 );


  void stg_model_render_geom( stg_model_t* mod );
  void stg_model_render_pose( stg_model_t* mod );
  void stg_model_render_polygons( stg_model_t* mod );

  /**@}*/


  // SOME DEFAULT VALUES FOR PROPERTIES -----------------------------------

  // world
#define STG_DEFAULT_RESOLUTION    0.02  // 2cm pixels
#define STG_DEFAULT_INTERVAL_REAL 100   // msec between updates
#define STG_DEFAULT_INTERVAL_SIM  100 

  // basic model
#define STG_DEFAULT_MASS 10.0  // kg
#define STG_DEFAULT_POSEX 0.0  // start at the origin by default
#define STG_DEFAULT_POSEY 0.0
#define STG_DEFAULT_POSEA 0.0
#define STG_DEFAULT_GEOM_POSEX 0.0 // no origin offset by default
#define STG_DEFAULT_GEOM_POSEY 0.0
#define STG_DEFAULT_GEOM_POSEA 0.0
#define STG_DEFAULT_GEOM_SIZEX 1.0 // 1m square by default
#define STG_DEFAULT_GEOM_SIZEY 1.0
#define STG_DEFAULT_OBSTACLERETURN TRUE
#define STG_DEFAULT_LASERRETURN LaserVisible
#define STG_DEFAULT_RANGERRETURN TRUE
#define STG_DEFAULT_BLOBRETURN TRUE
#define STG_DEFAULT_COLOR (0xFF0000) // red

  // GUI
#define STG_DEFAULT_GUI_MOVEMASK (STG_MOVE_TRANS | STG_MOVE_ROT)
#define STG_DEFAULT_GUI_NOSE FALSE
#define STG_DEFAULT_GUI_GRID FALSE
#define STG_DEFAULT_GUI_BOUNDARY FALSE

  // energy
#define STG_DEFAULT_ENERGY_CAPACITY 1000.0
#define STG_DEFAULT_ENERGY_CHARGEENABLE 1
#define STG_DEFAULT_ENERGY_PROBERANGE 0.0
#define STG_DEFAULT_ENERGY_GIVERATE 0.0
#define STG_DEFAULT_ENERGY_TRICKLERATE 0.1

  // laser
#define STG_DEFAULT_LASER_POSEX 0.0
#define STG_DEFAULT_LASER_POSEY 0.0
#define STG_DEFAULT_LASER_POSEA 0.0
#define STG_DEFAULT_LASER_SIZEX 0.15
#define STG_DEFAULT_LASER_SIZEY 0.15
#define STG_DEFAULT_LASER_MINRANGE 0.0
#define STG_DEFAULT_LASER_MAXRANGE 8.0
#define STG_DEFAULT_LASER_FOV M_PI
#define STG_DEFAULT_LASER_SAMPLES 180

  // blobfinder
#define STG_DEFAULT_BLOB_CHANNELCOUNT 6
#define STG_DEFAULT_BLOB_SCANWIDTH 80
#define STG_DEFAULT_BLOB_SCANHEIGHT 60 
#define STG_DEFAULT_BLOB_RANGEMAX 8.0 
#define STG_DEFAULT_BLOB_PAN 0.0
#define STG_DEFAULT_BLOB_TILT 0.0
#define STG_DEFAULT_BLOB_ZOOM DTOR(60)

  // fiducialfinder
#define STG_DEFAULT_FIDUCIAL_RANGEMIN 0
#define STG_DEFAULT_FIDUCIAL_RANGEMAXID 5
#define STG_DEFAULT_FIDUCIAL_RANGEMAXANON 8
#define STG_DEFAULT_FIDUCIAL_FOV DTOR(180)
 
  //  FUNCTION DEFINITIONS


  // utilities ---------------------------------------------------------------

  // normalizes the set [rects] of [num] rectangles, so that they fit
  // exactly in a unit square.
  void stg_normalize_rects( stg_rotrect_t* rects, int num );

  // returns the real (wall-clock) time in seconds
  stg_msec_t stg_timenow( void );

  // if stage wants to quit, this will return non-zero
  int stg_quit_test( void );

  // set stage's quit flag
  void stg_quit_request( void );


  void stg_err( char* err );

  //
  // Some useful macros

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

  //#define SUCCESS TRUE
  //#define FAIL FALSE

#define MILLION 1000000L
#define BILLION 1000000000.0

#ifndef M_PI
#define M_PI        3.14159265358979323846
#endif

#ifndef TWOPI
#define TWOPI 2.0 * M_PI
#endif

  // Convert radians to degrees
#ifndef RTOD
#define RTOD(r) ((r) * 180.0 / M_PI)
#endif

  // Convert degrees to radians
#ifndef DTOR
#define DTOR(d) ((d) * M_PI / 180.0)
#endif
 
  // Normalize angle to domain -pi, pi
#ifndef NORMALIZE
#define NORMALIZE(z) atan2(sin(z), cos(z))
#endif

  //#define MIN(X,Y) ( x<y ? x : y )
  //#define MAX(X,Y) ( x>y ? x : y )

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
#define PRINT_MSG(m) printf( "stage: "m" (%s %s)\n", __FILE__, __FUNCTION__)
#define PRINT_MSG1(m,a) printf( "stage: "m" (%s %s)\n", a, __FILE__, __FUNCTION__)    
#define PRINT_MSG2(m,a,b) printf( "stage: "m" (%s %s)\n", a, b, __FILE__, __FUNCTION__) 
#define PRINT_MSG3(m,a,b,c) printf( "stage: "m" (%s %s)\n", a, b, c, __FILE__, __FUNCTION__)
#define PRINT_MSG4(m,a,b,c,d) printf( "stage: "m" (%s %s)\n", a, b, c, d, __FILE__, __FUNCTION__)
#define PRINT_MSG5(m,a,b,c,d,e) printf( "stage: "m" (%s %s)\n", a, b, c, d, e,__FILE__, __FUNCTION__)
#else
#define PRINT_MSG(m) printf( "stage: "m"\n" )
#define PRINT_MSG1(m,a) printf( "stage: "m"\n", a)
#define PRINT_MSG2(m,a,b) printf( "stage: "m"\n,", a, b )
#define PRINT_MSG3(m,a,b,c) printf( "stage: "m"\n", a, b, c )
#define PRINT_MSG4(m,a,b,c,d) printf( "stage: "m"\n", a, b, c, d )
#define PRINT_MSG5(m,a,b,c,d,e) printf( "stage: "m"\n", a, b, c, d, e )
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

#ifdef __cplusplus
}
#endif 



#endif
