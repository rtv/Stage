#ifndef _STAGE_INTERNAL_H
#define _STAGE_INTERNAL_H

// internal function declarations that are not part of the external
// interface to Stage


#include <assert.h>
#include <pthread.h>
#include <semaphore.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <glib.h>
#include <glib-object.h>

// TODO - remove these
#include <gtk/gtk.h>
//#include <rtk.h> 
//#include <sys/socket.h>

#include "stage.h"
#include "config.h" // results of autoconf's system configuration tests
#include "replace.h" // Stage's implementations of missing system calls
#include "worldfile.hh"

/** @ingroup libstage
    @defgroup libstage_internal Internals

    These are internal docs. Don't use these functions in user
    code. Let us know if there is anything useful here that should be
    exposed in the external interface.

    @{ 
*/


#define STG_DEFAULT_WINDOW_WIDTH 400
#define STG_DEFAULT_WINDOW_HEIGHT 440

//#ifdef __cplusplus
//extern "C" {
//#endif 
  
void stg_color_to_glcolor4dv( stg_color_t scol, double gcol[4] );
void push_color( double col[4] );
void push_color_rgb( double r, double g, double b );
void push_color_rgba( double r, double g, double b, double a );
void pop_color( void );
void gl_pose_shift( stg_pose_t* pose );
void gl_coord_shift( double x, double y, double z, double a  );
void stg_block_list_scale( GList* blocks, 
			   stg_size_t* size );
void stg_block_list_destroy( GList* list );
void gui_world_set_title( stg_world_t* world, char* txt );
StgModel* stg_world_model_name_lookup( stg_world_t* world, const char* name );


typedef int(*stg_model_callback_t)(StgModel* mod, void* user );
  
  typedef struct stg_block
  {
    StgModel* mod; //< model to which this block belongs
    stg_point_t* pts; //< points defining a polygon
    size_t pt_count; //< the number of points
    stg_meters_t zmin; 
    stg_meters_t zmax; 
    stg_color_t color;
    bool inherit_color;
    //    int display_list; //< id of the OpenGL displaylist to draw this block
  } stg_block_t;
  
  
  /** Create a new block. A model's body is a list of these
      blocks. The point data is copied, so pts can safely be freed
      after calling this.*/
  stg_block_t* stg_block_create( StgModel* mod,
				 stg_point_t* pts, 
				 size_t pt_count,
				 stg_meters_t height,
				 stg_meters_t z_offset,
				 stg_color_t color,
				 bool inherit_color );
    
  /** destroy a block, freeing all memory */
  void stg_block_destroy( stg_block_t* block );
  
  void stg_block_update( stg_block_t* block );
  void stg_block_render( stg_block_t* block );

  void gui_startup( int* argc, char** argv[] ); 
  void gui_shutdown( void );
  void gui_configure( stg_world_t* world, 
		      int width, 
		      int height, 
		      double xscale, 
		      double yscale, 
		      double xcenter, 
		      double ycenter );

  void gui_load( stg_world_t* world, int section );
  void gui_save( stg_world_t* world );
  
  void* gui_world_create( stg_world_t* world );
  void gui_world_destroy( stg_world_t* world );
  int gui_world_update( stg_world_t* world );
  void stg_world_add_model( stg_world_t* world, StgModel* mod  );
  void gui_world_geom( stg_world_t* world );

  void gui_add_view_item( const gchar *name,
			  const gchar *label,
			  const gchar *tooltip,
			  GCallback callback,
			  gboolean  is_active,
			  void* userdata );

//  void gui_add_tree_item( stg_model_t* mod );
  
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
    GtkAction* action; // action associated with this toggle, may be NULL
    char* path;
  } stg_property_toggle_args_t;
    
  
  
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


  // the struct is defined in gui.h so that the sim engine and GUI
  // code are separated.
  typedef struct _gui_window gui_window_t;
             
  // defines a simulated world
  struct _stg_world
  {
    stg_id_t id; ///< Stage's unique identifier for this world
    
    GHashTable* models; ///< the models that make up the world, indexed by id
    GHashTable* models_by_name; ///< the models that make up the world, indexed by name

    CWorldFile* wf; ///< If set, points to the worldfile used to create this world

    /** a list of top-level models, i.e. models who have the world as
    their parent and whose position is therefore specified in world
    coordinates */
    GList* children;

    /** a list of models that should be updated with a call to
	stg_model_udpate(); */
    GList* update_list;

    /** a list of models that have non-zero velocity, so need their pose updated */
    GList* velocity_list;

    stg_meters_t width; ///< x size of the world 
    stg_meters_t height; ///< y size of the world

    /** the number of models of each type is counted so we can
	automatically generate names for them
    */
    //int child_type_count[256];
    GHashTable* child_type_count;

    struct _stg_matrix* matrix; ///< occupancy quadtree for model raytracing

    char* token; ///< the name of this world

    stg_msec_t sim_time_ms; ///< the current time in this world
    stg_msec_t sim_interval_ms; ///< this much simulated time elapses each step.
    long unsigned int updates; ///< the number of simulaticuted
    
    /** real-time interval between updates - set this to zero for 'as fast as possible' 
     */
    stg_msec_t wall_interval;
    stg_msec_t wall_last_update; ///< the wall-clock time of the last world update

    stg_msec_t gui_interval; ///< real-time interval between GUI canvas updates
    stg_msec_t gui_last_update; ///< the wall-clock time of the last gui canvas update

    /** the wallclock-time interval elapsed between the last two
	updates - compare this with sim_interval to see the ratio of
	sim to real time 
    */
    stg_msec_t real_interval_measured;

    double ppm; ///< the resolution of the world model in pixels per meter

    gboolean paused; ///< the world only updates when this is zero
   
    gboolean destroy; ///< this world should be destroyed ASAP

    gui_window_t* win; ///< the gui window associated with this world

    int subs; ///< the total number of subscriptions to all models

    int section_count;
  };

void stg_world_start_updating_model( stg_world_t* world, StgModel* mod );
void stg_world_stop_updating_model( stg_world_t* world, StgModel* mod );

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

  
  // MATRIX  -----------------------------------------------------------------------
  
  /** @ingroup libstage_internal
      @defgroup stg_matrix Matrix occupancy quadtree
      Occupancy quadtree underlying Stage's sensing and collision models. 
      @{ 
  */
  
  /** A node in the occupancy quadtree */
  typedef struct stg_cell
  {
    void* data;
    double x, y;
    double size;
    
    // bounding box
    double xmin,ymin,xmax,ymax;
    
    //stg_rtk_fig_t* fig; // for debugging

    struct stg_cell* children[4];
    struct stg_cell* parent;
  } stg_cell_t;
  
  /** in the cell-tree which contains cell, return the smallest cell that contains the point x,y. cell need not be the root of the tree
   */
  stg_cell_t* stg_cell_locate( stg_cell_t* cell, double x, double y );
  
  void stg_cell_unrender( stg_cell_t* cell );
  void stg_cell_render( stg_cell_t* cell );
  void stg_cell_render_tree( stg_cell_t* cell );
  void stg_cell_unrender_tree( stg_cell_t* cell );

  /** Occupancy quadtree structure */
  typedef struct _stg_matrix
  {
    double ppm; // pixels per meter (1/resolution)
    double width, height;
    
    // A quad tree of cells. Each leaf node contains a list of
    // pointers to objects located at that cell
    stg_cell_t* root;
    
    // hash table stores all the pointers to objects rendered in the
    // quad tree, each associated with a list of cells in which it is
    // rendered. This allows us to remove objects from the tree
    // without doing the geometry again
    GHashTable* ptable;

    /* TODO */
    // lists of cells that have changed recently. This is used by the
    // GUI to render cells very quickly, and could also be used by devices
    //GSList* cells_changed;
    
    // debug figure. if this is non-NULL, debug info is drawn here
    //stg_rtk_fig_t* fig;

    // todo - record a timestamp for matrix mods so devices can see if
    //the matrix has changed since they last peeked into it
    // stg_msec_t last_mod_time;
  } stg_matrix_t;
  

  /** Create a new matrix structure
   */
  stg_matrix_t* stg_matrix_create( double ppm, double width, double height );
  
  /** Frees all memory allocated by the matrix; first the cells, then the   
      cell array.
  */
  void stg_matrix_destroy( stg_matrix_t* matrix );
  
  /** removes all pointers from every cell in the matrix
   */
  void stg_matrix_clear( stg_matrix_t* matrix );
  
  /** Get the pointer from the cell at x,y (in meters).
   */
  void* stg_matrix_cell_get( stg_matrix_t* matrix, int r, double x, double y);

  /** append the pointer [object] to the list of pointers in the [matrix] cell at location [x,y]
   */
  void stg_matrix_cell_append(  stg_matrix_t* matrix, 
				double x, double y, void* object );
  
  /** remove [object] from all cells it occupies in the matrix
   */
  void stg_matrix_remove_obect( stg_matrix_t* matrix, void* object );
  
  /** if [object] appears in the cell's list, remove it
   */
  void stg_matrix_cell_remove(  stg_matrix_t* matrix,
				double x, double y, void* object );
  
  /** Append to the [object] pointer to the cells on the edge of a
      rectangle, described in meters about a center point.
  */
  void stg_matrix_rectangle( stg_matrix_t* matrix,
			     double px, double py, double pth,
			     double dx, double dy, 
			     void* object );
  
  /** Render [object] as a line in the matrix.
  */
  void stg_matrix_line( stg_matrix_t* matrix, 
			double x1, double y1, 
			double x2, double y2,
			void* object );
  /** Render block into the matrix as a series of lines. The block itself
      is the rendered object */
  void stg_matrix_block( stg_matrix_t* matrix,
			 stg_pose_t* origin,
			 stg_block_t* block );

  /** remove all references to this block from the matrix */
  void stg_matrix_remove_block( stg_matrix_t* matrix, stg_block_t* block );


  /** specify a line from (x1,y1) to (x2,y2), all in meters
   */
  typedef struct
  {
    stg_meters_t x1, y1, x2, y2;
  } stg_line_t;
  

  /** Call stg_matrix_line for each of [num_lines] lines 
   */
  void stg_matrix_lines( stg_matrix_t* matrix, 
			 stg_line_t* lines, int num_lines,
			 void* object );
    

  /** remove all reference to an object from the matrix
   */
  void stg_matrix_remove_object( stg_matrix_t* matrix, void* object );


  // RAYTRACE ITERATORS -------------------------------------------------------------
    
  typedef struct
  {
    double x,y,z; // hit location
    double range; // range to hit location
    StgModel* mod; // hit model
    //stg_block_t* block; // hit block
  } stg_hit_t;

 typedef struct
  {
    double x, y, z, a;
    double cosa, sina, tana;
    double range;
    double max_range;
    double* incr;

    GSList* models;
    int index;
    stg_matrix_t* matrix;    
  } itl_t;

  typedef enum { PointToPoint=0, PointToBearingRange } itl_mode_t;

  typedef int(*stg_itl_test_func_t)(StgModel* finder, StgModel* found );

itl_t* itl_create( double x, double y, double z, double a, double b, 
		   stg_matrix_t* matrix, itl_mode_t pmode );

  void itl_destroy( itl_t* itl );
void itl_raytrace( itl_t* itl );

StgModel* stg_first_model_on_ray( double x, double y, double z, 
				  double a, double b, 
				  stg_matrix_t* matrix, 
				  itl_mode_t pmode,
				  stg_itl_test_func_t func, 
				  StgModel* finder,
				  stg_meters_t* hitrange,
				  stg_meters_t* hitx,
				  stg_meters_t* hity );

StgModel* itl_first_matching( itl_t* itl, 
			      stg_itl_test_func_t func, 
			      StgModel* finder );

  /** @} */

  /** @ingroup libstage_internal
      @defgroup worldfile worldfile C wrappers
      @{
  */
  
  // C wrappers for C++ worldfile functions
/*   void wf_warn_unused( void ); */
/*   int wf_property_exists( int section, char* token ); */
/*   int wf_read_int( int section, char* token, int def ); */
/*   double wf_read_length( int section, char* token, double def ); */
/*   double wf_read_angle( int section, char* token, double def ); */
/*   double wf_read_float( int section, char* token, double def ); */
/*   const char* wf_read_tuple_string( int section, char* token, int index, char* def ); */
/*   double wf_read_tuple_float( int section, char* token, int index, double def ); */
/*   double wf_read_tuple_length( int section, char* token, int index, double def ); */
/*   double wf_read_tuple_angle( int section, char* token, int index, double def ); */
/*   const char* wf_read_string( int section, char* token, char* def ); */

/*   void wf_write_int( int section, char* token, int value ); */
/*   void wf_write_length( int section, char* token, double value ); */
/*   void wf_write_angle( int section, char* token, double value ); */
/*   void wf_write_float( int section, char* token, double value ); */
/*   void wf_write_tuple_string( int section, char* token, int index, char* value ); */
/*   void wf_write_tuple_float( int section, char* token, int index, double value ); */
/*   void wf_write_tuple_length( int section, char* token, int index, double value ); */
/*   void wf_write_tuple_angle( int section, char* token, int index, double value ); */
/*   void wf_write_string( int section, char* token, char* value ); */

/*   void wf_save( void ); */
/*   void wf_load( char* path ); */
/*   int wf_section_count( void ); */
/*   const char* wf_get_section_type( int section ); */
/*   int wf_get_parent_section( int section ); */
/*   const char* wf_get_filename( void); */

  /** @} */


/** @} */  
// end of libstage_internal documentation  


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



#endif // _STAGE_INTERNAL_H

