
// internal function declarations that are not part of the external
// interface to Stage

/** @addtogroup libstage
    @{
*/

#include "stage.h"

#ifdef __cplusplus
extern "C" {
#endif 
  
  /** @defgroup stg_gui GUI Window
      Code for the Stage user interface window.
      @{
  */
  
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

  void gui_startup( int* argc, char** argv[] ); 
  void gui_poll( void );
  void gui_shutdown( void );
  gui_window_t* gui_world_create( stg_world_t* world );
  void gui_world_destroy( stg_world_t* world );
  void stg_world_save( stg_world_t* world );
  int gui_world_update( stg_world_t* world );
  void stg_world_add_model( stg_world_t* world, stg_model_t* mod  );

  void gui_load( gui_window_t* win, int section );
  void gui_save( gui_window_t* win );

  /// render the geometry of all models in the world
  void gui_world_geom( stg_world_t* world );

  /// render the data of all models in the world
  void gui_world_render_data( stg_world_t* world );

  /// render the configuration of all models in the world
  void gui_world_render_cfg( stg_world_t* world );

  /// render the commands of all models in the world
  void gui_world_render_cmd( stg_world_t* world );
  
  /// get the structure containing all this model's figures
  gui_model_t* gui_model_figs( stg_model_t* model );

  void gui_model_create( stg_model_t* model );
  void gui_model_destroy( stg_model_t* model );
  void gui_model_display_pose( stg_model_t* mod, char* verb );
  void gui_model_features( stg_model_t* mod );
  void gui_model_geom( stg_model_t* model );
  void gui_model_mouse(rtk_fig_t *fig, int event, int mode);
  void gui_model_move( stg_model_t* mod );
  void gui_model_nose( stg_model_t* model );
  void gui_model_polygons( stg_model_t* model );
  void gui_model_render_command( stg_model_t* mod );
  void gui_model_render_config( stg_model_t* mod );
  void gui_model_render_data( stg_model_t* mod );
  void gui_window_menus_create( gui_window_t* win );
  void gui_window_menus_destroy( gui_window_t* win );

  /**@}*/


  /** @addtogroup stg_model
      @{ */

  /// Callback functions

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

  typedef void(*func_render_t)(struct _stg_model*);

  typedef void(*func_load_t)(struct _stg_model*);
  typedef void(*func_save_t)(struct _stg_model*);

  struct _stg_model
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
  
    //int section; // worldfile section number

    // todo - thread-safe version
    // allow exclusive access to this model
    
    // the generic buffers used by specialized model types.
    // For speed, these are implemented directly, rather than by datalist.
    void *data, *cmd, *cfg;
    size_t data_len, cmd_len, cfg_len;
    pthread_mutex_t data_mutex, cmd_mutex, cfg_mutex;
    
    // an array of polygons that make up the model's body. Possibly
    // zero elements.
    GArray* polygons;
    
    // basic model properties
    stg_blob_return_t blob_return; 
    stg_laser_return_t laser_return; // value returned to a laser sensor
    stg_obstacle_return_t obstacle_return; // if non-zero, we are included in obstacle detection
    stg_fiducial_return_t fiducial_return; // value returned to a fiducial finder
    stg_pose_t pose; // current pose in parent's CS
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
    func_load_t f_load;
    func_save_t f_save;

    // a datalist can contain arbitrary named data items. Used by
    // derived model types to store properties, and for user code to
    // associate arbitrary items with a model.
    GData* props;

    /// if set, this callback is run when we do model_put_data() -
    /// it's used by the player plugin to notify Player that data is
    /// ready.
    func_data_notify_t data_notify;
    void* data_notify_arg;
    
    // the device is consuming this much energy
    stg_watts_t watts;
    
    char extend[0]; // this must be the last field!

  };

  // internal functions
  
  int _model_update( stg_model_t* mod );
  //int _model_init( stg_model_t* mod );
  int _model_startup( stg_model_t* mod );
  int _model_shutdown( stg_model_t* mod );

  void stg_model_update_velocity( stg_model_t* model );
  int stg_model_update_pose( stg_model_t* model );
  void stg_model_energy_consume( stg_model_t* mod, stg_watts_t rate );
  void stg_model_map( stg_model_t* mod, gboolean render );
  void stg_model_map_with_children( stg_model_t* mod, gboolean render );
  
  rtk_fig_t* stg_model_prop_fig_create( stg_model_t* mod, 
				    rtk_fig_t* array[],
				    stg_id_t propid, 
				    rtk_fig_t* parent,
				    int layer );

  void stg_model_render_geom( stg_model_t* mod );
  void stg_model_render_pose( stg_model_t* mod );
  void stg_model_render_polygons( stg_model_t* mod );

  /**@}*/  

  /** @addtogroup stg_world
      @{ */

  /// defines a simulated world
  struct _stg_world
  {
    stg_id_t id; // Stage's identifier for this world
    
    GHashTable* models; // the models that make up the world, indexed by id
    GHashTable* models_by_name; // the models that make up the world, indexed by name
    
    // the number of models of each type is counted so we can
    // automatically generate names for them
    int child_type_count[ STG_MODEL_COUNT ];
    
    struct _stg_matrix* matrix;

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
  };

  /**@}*/

  
 
 
  // MATRIX  -----------------------------------------------------------------------
  
  /** @defgroup stg_matrix Matrix
      Underlying bitmap world representation
      @{ 
  */
  
  typedef struct _stg_matrix
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


  /** Create a new matrix structure
   */
  stg_matrix_t* stg_matrix_create( double ppm, double medppm, double bigppm );
  
  /** Frees all memory allocated by the matrix; first the cells, then the   
      cell array.
  */
  void stg_matrix_destroy( stg_matrix_t* matrix );
  
  /** removes all pointers from every cell in the matrix
   */
  void stg_matrix_clear( stg_matrix_t* matrix );
  
  /** Get the pointer array from the small cell at x,y (in meters).
   */
  GPtrArray* stg_matrix_cell_get( stg_matrix_t* matrix, double x, double y);

  /** Get the pointer array from the big cell at x,y (in meters).
   */
  GPtrArray* stg_matrix_bigcell_get( stg_matrix_t* matrix, double x, double y);
  
  /** Get the pointer array from the medium cell at x,y (in meters).
   */
  GPtrArray* stg_matrix_medcell_get( stg_matrix_t* matrix, double x, double y);
  
  /** append the [object] to the pointer array at the cell
   */
  void stg_matrix_cell_append(  stg_matrix_t* matrix, 
				double x, double y, void* object );

  /** if [object] appears in the cell's array, remove it
   */
  void stg_matrix_cell_remove(  stg_matrix_t* matrix,
				double x, double y, void* object );
  
  /** Append to the [object] pointer to the cells on the edge of a
      rectangle, described in meters about a center point. Uses
      the matrix.ppm value to scale from meters to matrix pixels.
  */
  void stg_matrix_rectangle( stg_matrix_t* matrix,
			     double px, double py, double pth,
			     double dx, double dy, 
			     void* object, int add );
  
  /** Render/unrender [object] as a line in the matrix. Render if
      [add] is true, unrender is [add] is false.
  */
  void stg_matrix_line( stg_matrix_t* matrix, 
			double x1, double y1, 
			double x2, double y2,
			void* object, int add );

  /** specify a line from (x1,y1) to (x2,y2), all in meters
   */
  typedef struct
  {
    stg_meters_t x1, y1, x2, y2;
  } stg_line_t;
  
  //typedef struct
  // {
  //int toplx, toply, toprx, topry, botlx, botly, botrx, botry;
  //} stg_corners_t;


  /** Call stg_matrix_line for each of [num_lines] lines 
   */
  void stg_matrix_lines( stg_matrix_t* matrix, 
			 stg_line_t* lines, int num_lines,
			 void* object, int add );
  
  /** render a polygon into the matrix
   */
  void stg_matrix_polygon( stg_matrix_t* matrix,
			   double x, double y, double a,
			   stg_polygon_t* poly,
			   void* object, int add );
  
  /** render an array of polygons into the matrix
   */
  void stg_matrix_polygons( stg_matrix_t* matrix,
			    double x, double y, double a,
			    stg_polygon_t* polys, int num_polys,
			    void* object, int add );

  /**@}*/

  // RAYTRACE ITERATORS -------------------------------------------------------------
  
  /** @defgroup stg_itl Raytracing
      Iterators for raytracing in a matrix
      @{ */
  
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
  
  typedef enum { PointToPoint=0, PointToBearingRange } itl_mode_t;
  
  typedef int(*stg_itl_test_func_t)(stg_model_t* finder, stg_model_t* found );
  
  itl_t* itl_create( double x, double y, double a, double b, 
		     stg_matrix_t* matrix, itl_mode_t pmode );
  
  void itl_destroy( itl_t* itl );
  void itl_raytrace( itl_t* itl );

  // deprecated - remove
  //stg_model_t* itl_next( itl_t* itl );
  
  stg_model_t* itl_first_matching( itl_t* itl, 
				   stg_itl_test_func_t func, 
				   stg_model_t* finder );
  /** @} */  
  
  /** @defgroup worldfile worldfile C wrappers
      @{
  */
  
  // C wrappers for C++ worldfile functions
  int wf_read_int( int section, char* token, int def );
  double wf_read_length( int section, char* token, double def );
  double wf_read_angle( int section, char* token, double def );
  double wf_read_float( int section, char* token, double def );
  const char* wf_read_tuple_string( int section, char* token, int index, char* def );
  double wf_read_tuple_float( int section, char* token, int index, double def );
  double wf_read_tuple_length( int section, char* token, int index, double def );
  double wf_read_tuple_angle( int section, char* token, int index, double def );
  const char* wf_read_string( int section, char* token, char* def );

  void wf_write_int( int section, char* token, int value );
  void wf_write_length( int section, char* token, double value );
  void wf_write_angle( int section, char* token, double value );
  void wf_write_float( int section, char* token, double value );
  void wf_write_tuple_string( int section, char* token, int index, char* value );
  void wf_write_tuple_float( int section, char* token, int index, double value );
  void wf_write_tuple_length( int section, char* token, int index, double value );
  void wf_write_tuple_angle( int section, char* token, int index, double value );
  void wf_write_string( int section, char* token, char* value );

  void wf_save( void );
  void wf_load( char* path );
  int wf_section_count( void );
  const char* wf_get_section_type( int section );
  int wf_get_parent_section( int section );

  /** @} */

  // CALLBACK WRAPPERS ------------------------------------------------------------

  /** @defgroup stg_callbacks Callback wrappers
      Wrappers used as callback functions for Glib container iterators
      @{ 
  */
  
  // callback wrappers for other functions
  void model_update_cb( gpointer key, gpointer value, gpointer user );
  void model_print_cb( gpointer key, gpointer value, gpointer user );
  void model_destroy_cb( gpointer mod );

  /** @} */
  
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


// end documentation group stage
/**@}*/




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
/*   //stg_world_t* sc_load_worldblock( stg_client_t* cli, stg_token_t** tokensptr ); */
/*   //stg_model_t* sc_load_modelblock( stg_world_t* world, stg_model_t* parent, */
/*   //			   stg_token_t** tokensptr ); */




#ifdef __cplusplus
}
#endif 

