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
 * CVS: $Id: stage.h,v 1.98 2004-10-02 01:37:27 rtv Exp $
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

 typedef enum
    {
      STG_PROP_TIME, // double - time since startup in seconds
      STG_PROP_MASS,
      STG_PROP_COLOR,
      STG_PROP_GEOM, 
      STG_PROP_PARENT, 
      STG_PROP_PLAYERID,
      STG_PROP_POSE,
      STG_PROP_ENERGYCONFIG,
      STG_PROP_ENERGYDATA,
      STG_PROP_PUCKRETURN,
      STG_PROP_LINES,
      STG_PROP_VELOCITY, 
      STG_PROP_LASERRETURN,
      STG_PROP_OBSTACLERETURN,
      STG_PROP_BLOBRETURN,
      STG_PROP_VISIONRETURN,
      STG_PROP_RANGERRETURN, 
      STG_PROP_FIDUCIALRETURN,
      STG_PROP_GUIFEATURES,
      STG_PROP_DATA, 
      STG_PROP_CONFIG, 
      STG_PROP_COMMAND,
      STG_PROP_COUNT // this must be the last entry (it's not a real
      // property - it just counts 'em).
    } stg_prop_type_t;

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

  //  --------------------------------------------------------------

  // standard energy consumption of some devices in W.
  //
  // The MOTIONKG value is a hack to approximate cost of motion, as
  // Stage does not yet have an acceleration model.
  //
#define STG_ENERGY_COST_LASER 20.0 // 20 Watts! (LMS200 - from SICK web site)
#define STG_ENERGY_COST_FIDUCIAL 10.0 // 10 Watts
#define STG_ENERGY_COST_RANGER 0.5 // 500mW (estimate)
#define STG_ENERGY_COST_MOTIONKG 10.0 // 10 Watts per KG when moving 
#define STG_ENERGY_COST_BLOB 4.0 // 4W (estimate)

  typedef struct
  {
    stg_joules_t joules; // current energy stored in Joules/1000
    stg_watts_t watts; // current power expenditure in mW (mJoules/sec)
    int charging; // 1 if we are receiving energy, -1 if we are
    // supplying energy, 0 if we are neither charging nor
    // supplying energy.
    stg_meters_t range; // the range that our charging probe hit a charger
  } stg_energy_data_t;

  typedef struct
  {
    stg_joules_t capacity; // maximum energy we can store (we start fully charged)
    stg_meters_t probe_range; // the length of our recharge probe
    //stg_pose_t probe_pose; // TODO - the origin of our probe

    stg_watts_t give_rate; // give this many Watts to a probe that hits me (possibly 0)
  
    stg_watts_t trickle_rate; // this much energy is consumed or
    // received by this device per second as a
    // baseline trickle. Positive values mean
    // that the device is just burning energy
    // stayting alive, which is appropriate
    // for most devices. Negative values mean
    // that the device is receiving energy
    // from the environment, simulating a
    // solar cell or some other ambient energy
    // collector

  } stg_energy_config_t;


  // BLINKENLIGHT ------------------------------------------------------------

  // a number of milliseconds, used for example as the blinkenlight interval
#define STG_LIGHT_ON UINT_MAX
#define STG_LIGHT_OFF 0

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

  // MOVEMASK ---------------------------------------------------------
   
  typedef int stg_movemask_t;

  typedef struct
  {
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

  // TODO - line-of-sight messaging
  /* line-of-sight messaging packet */
  /* 
     #define STG_LOS_MSG_MAX_LEN 32
   
     typedef struct
     {
     int id;
     char bytes[STG_LOS_MSG_MAX_LEN];
     size_t len;
     int power; // transmit power or received power
     //  int consume;
     } stg_los_msg_t;

     // print the values in a message packet
     void stg_los_msg_print( stg_los_msg_t* msg );
  */

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

  /// Structure that holds the GUI figure for each model
  typedef struct
  {
    rtk_fig_t* top;
    rtk_fig_t* geom;
    rtk_fig_t* grid;
    rtk_fig_t* data;
    rtk_fig_t* cmd;
    rtk_fig_t* cfg;
    rtk_fig_t* bg; // background (used e.g for laser scan fill)
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

    gboolean movie_exporting;
    int movie_speed;
    int movie_count;
    int movie_tag;

    rtk_menu_t** menus;
    rtk_menuitem_t** mitems;
    int menu_count;
    int mitem_count;
  
    rtk_menuitem_t** mitems_mspeed;
    int mitems_mspeed_count;
  
    struct _stg_model* selection_active;
  
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


  typedef void(*func_init_t)(struct _stg_model*);
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

  // used to create special-purpose models
  typedef struct
  {
    func_init_t init;
    func_startup_t startup;
    func_shutdown_t shutdown;
    func_update_t update;
    func_set_data_t set_data;
    func_get_data_t get_data;
    func_set_command_t set_command;
    func_get_command_t get_command;
    func_set_config_t set_config;
    func_get_config_t get_config;
  } stg_lib_entry_t;

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

    //stg_lib_entry_t** library;

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
    stg_lib_entry_t* lib; // these are the pointers to my specialized functions

    struct _stg_model *parent; // the model that owns this one, possibly NULL

    GPtrArray* children; // the models owned by this model

    // the number of children of each type is counted so we can
    // automatically generate names for them
    int child_type_count[ STG_MODEL_COUNT ];

    // the number of subscriptions to each property
    //int subs[STG_PROP_COUNT]; 
    int subs;
  
    // the time that each property was last calculated
    //stg_msec_t update_times[STG_PROP_COUNT];

    /// if set, this callback is run when we do model_put_data() -
    /// it's used by the player plugin to notify Player that data is
    /// ready.
    func_data_notify_t data_notify;
    void* data_notify_arg;

    gui_model_t gui; // all the gui stuff

    // todo - add this as a property?
    stg_joules_t energy_consumed;

    stg_msec_t interval; // time between updates in ms
    stg_msec_t interval_elapsed; // time since last update in ms

    stg_pose_t odom;

    // the generic buffers used by specialized model types
    void *data, *cmd, *cfg;
    size_t data_len, cmd_len, cfg_len;

    stg_line_t* lines; // this buffer is lines_count * sizeof(stg_line_t) big
    int lines_count; // the number of lines
    double* polypoints; // if the lines can be converted into a polygon,
    // the polygon points are stored here, there are
    // lines_count polypoints

    // basic model properties
    stg_laser_return_t laser_return;
    stg_bool_t obstacle_return; 
    stg_bool_t blob_return; // visible in blobfinder?
    stg_fiducial_return_t fiducial_return;
    stg_pose_t pose;
    stg_velocity_t velocity;
    stg_bool_t stall;
    stg_geom_t geom;
    stg_color_t color;
    stg_kg_t mass;
    stg_guifeatures_t guifeatures;
    stg_energy_config_t energy_config;   // these are a little strange
    stg_energy_data_t energy_data;
  } stg_model_t;  

typedef enum 
  { PointToPoint=0, PointToBearingRange } 
itl_mode_t;

typedef struct
{
  double x, y, a;
  double cosa, sina;
  double range;
  double max_range;
  double big_incr;
  double small_incr;
  double med_incr;

  GPtrArray* models;
  int index;
  stg_matrix_t* matrix;

} itl_t;

  
  /// initialize the stage library - optionally pass in the arguments
  /// to main, so Stage or rtk or gtk or xlib can read the options.
  int stg_init( int argc, char** argv );

  /// get a string identifying the version of stage. The string is
  /// generated by autoconf.
  const char* stg_get_version_string( void );
  
typedef int(*stg_itl_test_func_t)(stg_model_t* finder, stg_model_t* found );

itl_t* itl_create( double x, double y, double a, double b, 
		   stg_matrix_t* matrix, itl_mode_t pmode );

void itl_destroy( itl_t* itl );
void itl_raytrace( itl_t* itl );
stg_model_t* itl_next( itl_t* itl );

stg_model_t* itl_first_matching( itl_t* itl, 
				stg_itl_test_func_t func, 
				stg_model_t* finder );
  /** @addtogroup stg_gui */
  /** @{ */
  void gui_startup( int* argc, char** argv[] ); 
  void gui_poll( void );
  void gui_shutdown( void );
  gui_window_t* gui_world_create( stg_world_t* world );
  void gui_world_destroy( stg_world_t* world );
  void stg_world_save( stg_world_t* world );
  int gui_world_update( stg_world_t* world );

  void gui_model_create( stg_model_t* model );
  void gui_model_destroy( stg_model_t* model );
  void gui_model_render( stg_model_t* model );
  void gui_model_nose( stg_model_t* model );
  void gui_model_pose( stg_model_t* mod );
  void gui_model_geom( stg_model_t* model );
  void gui_model_lines( stg_model_t* model );
  void gui_model_rangers( stg_model_t* mod );
  void gui_model_rangers_data( stg_model_t* mod );
  void gui_model_features( stg_model_t* mod );
  void gui_model_laser_data( stg_model_t* mod );
  void gui_model_laser( stg_model_t* mod );
  gui_model_t* gui_model_figs( stg_model_t* model );
  void gui_window_menus_create( gui_window_t* win );
  void gui_window_menus_destroy( gui_window_t* win );
  void gui_model_mouse(rtk_fig_t *fig, int event, int mode);
  void gui_model_display_pose( stg_model_t* mod, char* verb );
  /**@}*/

  /** @addtogroup stg_matrix */
  /** @{ */

  stg_matrix_t* stg_matrix_create( double ppm, double medppm, double bigppm );

  // frees all memory allocated by the matrix; first the cells, then the
  // cell array.
  void stg_matrix_destroy( stg_matrix_t* matrix );

  // removes all pointers from every cell in the matrix
  void stg_matrix_clear( stg_matrix_t* matrix );

  GPtrArray* stg_matrix_cell_get( stg_matrix_t* matrix, double x, double y);
  GPtrArray* stg_matrix_bigcell_get( stg_matrix_t* matrix, double x, double y);
  GPtrArray* stg_matrix_medcell_get( stg_matrix_t* matrix, double x, double y);

  // append the [object] to the pointer array at the cell
  void stg_matrix_cell_append(  stg_matrix_t* matrix, 
				double x, double y, void* object );

  // if [object] appears in the cell's array, remove it
  void stg_matrix_cell_remove(  stg_matrix_t* matrix,
				double x, double y, void* object );

  // these append to the [object] pointer to the cells on the edge of a
  // shape. shapes are described in meters about a center point. They
  // use the matrix.ppm value to scale from meters to matrix pixels.
  void stg_matrix_rectangle( stg_matrix_t* matrix,
			     double px, double py, double pth,
			     double dx, double dy, 
			     void* object, int add );

  void stg_matrix_line( stg_matrix_t* matrix, 
			double x1, double y1, 
			double x2, double y2,
			void* object, int add );

  void stg_matrix_lines( stg_matrix_t* matrix, 
			 stg_line_t* lines, int num_lines,
			 void* object, int add );
  /**@}*/

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

  /// create a new model  
  stg_model_t*  stg_world_model_create( stg_world_t* world, 
					stg_id_t id, 
					stg_id_t parent_id, 
					stg_model_type_t type, 
					stg_lib_entry_t* lib,
					char* token );

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
				  stg_lib_entry_t* lib, 
				  char* token );

  /// destroy a model, freeing its memory
  void stg_model_destroy( stg_model_t* mod );
  void model_destroy_cb( gpointer mod );
  void stg_model_global_pose( stg_model_t* mod, stg_pose_t* pose );
  void stg_model_subscribe( stg_model_t* mod );
  void stg_model_unsubscribe( stg_model_t* mod );

  // this calls one of the set property functions, according to the propid value
  int stg_model_set_prop( stg_model_t* mod, stg_id_t propid, void* data, size_t len );
  int stg_model_get_prop( stg_model_t* mod, stg_id_t pid, void** data, size_t* len );

  // SET properties - use these to set props, don't set them directly
  int stg_model_set_pose( stg_model_t* mod, stg_pose_t* pose );
  int stg_model_set_odom( stg_model_t* mod, stg_pose_t* pose );
  int stg_model_set_velocity( stg_model_t* mod, stg_velocity_t* vel );
  int stg_model_set_size( stg_model_t* mod, stg_size_t* sz );
  int stg_model_set_color( stg_model_t* mod, stg_color_t* col );
  int stg_model_set_geom( stg_model_t* mod, stg_geom_t* geom );
  int stg_model_set_mass( stg_model_t* mod, stg_kg_t* mass );
  int stg_model_set_guifeatures( stg_model_t* mod, stg_guifeatures_t* gf );
  int stg_model_set_energy_config( stg_model_t* mod, stg_energy_config_t* gf );
  int stg_model_set_energy_data( stg_model_t* mod, stg_energy_data_t* gf );
  int stg_model_set_lines( stg_model_t* mod, stg_line_t* lines, size_t lines_count );
  int stg_model_set_obstaclereturn( stg_model_t* mod, stg_bool_t* ret );
  int stg_model_set_laserreturn( stg_model_t* mod, stg_laser_return_t* val );
  int stg_model_set_fiducialreturn( stg_model_t* mod, stg_fiducial_return_t* val );

  // GET properties - use these to get props - don't get them directly
  stg_velocity_t*      stg_model_get_velocity( stg_model_t* mod );
  stg_geom_t*          stg_model_get_geom( stg_model_t* mod );
  stg_color_t          stg_model_get_color( stg_model_t* mod );
  stg_pose_t*          stg_model_get_pose( stg_model_t* mod );
  stg_pose_t*          stg_model_get_odom( stg_model_t* mod );
  stg_kg_t*            stg_model_get_mass( stg_model_t* mod );
  stg_line_t*          stg_model_get_lines( stg_model_t* mod, size_t* count );
  stg_guifeatures_t*   stg_model_get_guifeaturess( stg_model_t* mod );
  stg_energy_data_t*   stg_model_get_energy_data( stg_model_t* mod );
  stg_energy_config_t* stg_model_get_energy_config( stg_model_t* mod );
  stg_bool_t           stg_model_get_obstaclereturn( stg_model_t* mod );
  stg_laser_return_t   stg_model_get_laserreturn( stg_model_t* mod );
  stg_fiducial_return_t*  stg_model_get_fiducialreturn( stg_model_t* mod );

  // special
  int stg_model_set_command( stg_model_t* mod, void* cmd, size_t len );
  int stg_model_set_data( stg_model_t* mod, void* data, size_t len );
  int stg_model_set_config( stg_model_t* mod, void* cmd, size_t len );

  int _set_data( stg_model_t* mod, void* data, size_t len );
  int _set_cmd( stg_model_t* mod, void* cmd, size_t len );
  int _set_cfg( stg_model_t* mod, void* cfg, size_t len );

  void* stg_model_get_command( stg_model_t* mod, size_t* len );
  void* stg_model_get_data( stg_model_t* mod, size_t* len );
  void* stg_model_get_config( stg_model_t* mod, size_t* len );

  int stg_model_update( stg_model_t* model );
  void model_update_cb( gpointer key, gpointer value, gpointer user );
  void stg_model_update_velocity( stg_model_t* model );
  int stg_model_update_pose( stg_model_t* model );
  void stg_model_print( stg_model_t* mod );
  void model_print_cb( gpointer key, gpointer value, gpointer user );
  void stg_model_local_to_global( stg_model_t* mod, stg_pose_t* pose );
  void stg_model_energy_consume( stg_model_t* mod, stg_watts_t rate );
  void stg_model_lines_render( stg_model_t* mod );
  void stg_model_map( stg_model_t* mod, gboolean render );
  void stg_model_map_with_children( stg_model_t* mod, gboolean render );

  rtk_fig_t* stg_model_prop_fig_create( stg_model_t* mod, 
				    rtk_fig_t* array[],
				    stg_id_t propid, 
				    rtk_fig_t* parent,
				    int layer );

  int stg_model_is_antecedent( stg_model_t* mod, stg_model_t* testmod );
  int stg_model_is_descendent( stg_model_t* mod, stg_model_t* testmod );

  /// returns 1 if mod1 and mod2 are in the same tree
  int stg_model_is_related( stg_model_t* mod1, stg_model_t* mod2 );

  void stg_model_render_lines( stg_model_t* mod );
  void stg_model_render_geom( stg_model_t* mod );
  void stg_model_render_pose( stg_model_t* mod );

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
#define STG_DEFAULT_GUI_NOSE TRUE
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

  // returns an array of 4 * num_rects stg_line_t's
  stg_line_t* stg_rects_to_lines( stg_rotrect_t* rects, int num_rects );
  void stg_normalize_lines( stg_line_t* lines, int num );
  void stg_scale_lines( stg_line_t* lines, int num, double xscale, double yscale );
  void stg_translate_lines( stg_line_t* lines, int num, double xtrans, double ytrans );

  // returns the real (wall-clock) time in seconds
  stg_msec_t stg_timenow( void );

  // if stage wants to quit, this will return non-zero
  int stg_quit_test( void );

  // set stage's quit flag
  void stg_quit_request( void );


  void stg_err( char* err );


  // TOKEN -----------------------------------------------------------------------
  // token stuff for parsing worldfiles

#define CFG_OPEN '('
#define CFG_CLOSE ')'
#define STR_OPEN '\"'
#define STR_CLOSE '\"'
#define TPL_OPEN '['
#define TPL_CLOSE ']'

  typedef enum {
    STG_T_NUM = 0,
    STG_T_BOOLEAN,
    STG_T_MODELPROP,
    STG_T_WORLDPROP,
    STG_T_NAME,
    STG_T_STRING,
    STG_T_KEYWORD,
    STG_T_CFG_OPEN,
    STG_T_CFG_CLOSE,
    STG_T_TPL_OPEN,
    STG_T_TPL_CLOSE,
  } stg_token_type_t;


  typedef struct _stg_token {
    char* token; // the text of the token 
    stg_token_type_t type;
    unsigned int line; // the line on which the token appears 
  
    struct _stg_token* next; // linked list support
    struct _stg_token* child; // tree support
  
  } stg_token_t;

  stg_token_t* stg_token_next( stg_token_t* tokens );
  // print <token> formatted for a human reader, with a string <prefix>
  void stg_token_print( char* prefix,  stg_token_t* token );
  // print a token array suitable for human reader
  void stg_tokens_print( stg_token_t* tokens );
  void stg_tokens_free( stg_token_t* tokens );

  // create a new token structure from the arguments
  stg_token_t* stg_token_create( const char* token, stg_token_type_t type, int line );

  // add a token to a list
  stg_token_t* stg_token_append( stg_token_t* head, 
				 char* token, stg_token_type_t type, int line );

  const char* stg_token_type_string( stg_token_type_t type );

  const char* stg_model_type_string( stg_model_type_t type );


  // Bitmap loading -------------------------------------------------------

  // load <filename>, an image format understood by gdk-pixbuf, and
  // return a set of rectangles that approximate the image. Caller
  // must free the array of rectangles. If width and height are
  // non-null, they are filled in with the size of the image in pixels
  int stg_load_image( const char* filename, 
		      stg_rotrect_t** rects,
		      int* rect_count,
		      int* widthp, int* heightp );

  // functions for parsing worldfiles 
  //stg_token_t* stg_tokenize( FILE* wf );
  //stg_world_t* sc_load_worldblock( stg_client_t* cli, stg_token_t** tokensptr );
  //stg_model_t* sc_load_modelblock( stg_world_t* world, stg_model_t* parent, 
  //				stg_token_t** tokensptr );

  // ---------------------------------------------------------------------------


  // utility
  void stg_pose_sum( stg_pose_t* result, stg_pose_t* p1, stg_pose_t* p2 );

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
