#ifndef GUI_H
#define GUI_H
// GUI constants

#define STG_LAYER_BACKGROUND 10
#define STG_LAYER_BODY 30
#define STG_LAYER_GRID 20
#define STG_LAYER_USER 99
#define STG_LAYER_GEOM 80
#define STG_LAYER_MATRIX 40
#define STG_LAYER_MATRIX_TREE 39
#define STG_LAYER_DEBUG 98

#define STG_LAYER_BLOBGEOM 51
#define STG_LAYER_BLOBCONFIG 52
#define STG_LAYER_BLOBDATA 53

#define STG_LAYER_PTZGEOM 54
#define STG_LAYER_PTZONFIG 56
#define STG_LAYER_PTZDATA 57

#define STG_LAYER_NEIGHBORGEOM 61
#define STG_LAYER_NEIGHBORCONFIG 62
#define STG_LAYER_NEIGHBORDATA 63

#define STG_LAYER_ENERGYGEOM 71
#define STG_LAYER_ENERGYCONFIG 72
#define STG_LAYER_ENERGYDATA 73

#define STG_GRID_MAJOR_COLOR "gray85"
#define STG_GRID_MINOR_COLOR "gray95"
#define STG_GRID_AXIS_COLOR "gray40"
#define STG_BACKGROUND_COLOR "ivory"
#define STG_BOUNDINGBOX_COLOR "magenta"
#define STG_MATRIX_COLOR "dark green"

  // model color defaults
#define STG_FIDUCIAL_COLOR "lime green"
#define STG_FIDUCIAL_CFG_COLOR "green"

#define STG_ENERGY_COLOR "purple"
#define STG_ENERGY_CFG_COLOR "magenta"
   
#define STG_DEBUG_COLOR "red"
#define STG_BLOB_CFG_COLOR "gray75"

#define STG_LAYER_RANGERGEOM 41
#define STG_LAYER_RANGERCONFIG 42
#define STG_LAYER_RANGERDATA 43

#define STG_LAYER_BUMPERGEOM 60
#define STG_LAYER_BUMPERCONFIG 61
#define STG_LAYER_BUMPERDATA 62

#define STG_RANGER_COLOR "gray75" 
#define STG_RANGER_GEOM_COLOR "orange"
#define STG_RANGER_CONFIG_COLOR "gray90"

#define STG_LAYER_LASERGEOM 32
#define STG_LAYER_LASERDATA 31
#define STG_LAYER_LASERCONFIG 32

#define STG_LAYER_POSITIONGEOM 45
#define STG_LAYER_POSITIONDATA 47
#define STG_LAYER_POSITIONCONFIG 46

#define STG_LASER_COLOR "steel blue"
#define STG_LASER_FILL_COLOR "powder blue"
#define STG_LASER_GEOM_COLOR "blue"
#define STG_LASER_CFG_COLOR "light steel blue"
#define STG_LASER_BRIGHT_COLOR "blue"

#define STG_LAYER_GRIPPERGEOM 85
#define STG_LAYER_GRIPPERDATA 87
#define STG_LAYER_GRIPPERCONFIG 86

#define STG_GRIPPER_COLOR "steel blue"
#define STG_GRIPPER_FILL_COLOR "light blue"
#define STG_GRIPPER_GEOM_COLOR "blue"
#define STG_GRIPPER_CFG_COLOR "light steel blue"
#define STG_GRIPPER_BRIGHT_COLOR "blue"

  // GUI colors   
#define STG_GRID_MAJOR_COLOR "gray85"
#define STG_GRID_MINOR_COLOR "gray95"
#define STG_MATRIX_COLOR "dark green"
#define STG_BACKGROUND_COLOR "ivory"
  
  // model color defaults
#define STG_GENERIC_COLOR "black"
#define STG_POSITION_COLOR "red"
#define STG_WALL_COLOR "dark blue"
#define STG_SONAR_COLOR "gray70"

#define STG_LAYER_SPEECHDATA 88
#define STG_SPEECH_COLOR "navy"

//#include "config.h"


//#include <gtk/gtk.h>
/* #include <gtk/gtkgl.h> */
/* #include <gdk/gdkglglext.h> */
/* #include <GL/gl.h> */
/* #include <GL/glu.h> */

#include "stage_internal.h"

// default GUI window refresh interval in milliseconds
#define STG_DEFAULT_REDRAW_INTERVAL 100

/// Define a point in 3D space
typedef struct
{
  double x,y,z;
} stg_point_3d_t;

  /** Defines the GUI window */
struct _gui_window
{
  // the interval between window redraws, in milliseconds
  //int redraw_interval;
  //guint timeout_id; // identifier of the glib timeout signal source
  
  // Gtk stuff
  GtkWidget* frame;    
  GtkWidget *layout;
  GtkWidget *menu_bar;
  //GtkWidget* scrolled_win;
  
  // The status bar widget
  GtkStatusbar *status_bar;
  GtkProgressBar *perf_bar;
  GtkProgressBar *rt_bar;
  GtkLabel *clock_label;
  GtkActionGroup *action_group;

  // the main drawing widget
  GtkWidget* canvas;
  int dirty;
  
  int draw_list;
  int debug_list;

  stg_radians_t stheta; ///< view rotation about x axis
  stg_radians_t sphi; ///< view rotation about x y axis
  double scale; ///< view scale
  double panx; ///< pan along x axis in meters
  double pany; ///< pan along y axis in meters
  
  stg_point_t click_point; ///< The place where the most recent
  ///< mouse click happened, in world coords
    
  // fall back to vintage RTK style
  /*     stg_rtk_canvas_t* canvas; */
  /*     stg_rtk_fig_t* bg; // background */
  /*     stg_rtk_fig_t* matrix; */
  /*     stg_rtk_fig_t* matrix_tree; */
  /*     stg_rtk_fig_t* poses; */
  
  stg_world_t* world; // every window shows a single world
  
  GtkStatusbar* statusbar;
  GtkLabel* timelabel;
  
  int wf_section; // worldfile section for load/save
  
  gboolean follow_selection;
  gboolean show_matrix;  
  gboolean fill_polygons;
  gboolean show_geom;
  gboolean show_polygons;
  gboolean show_grid;
  gboolean show_data;
  gboolean show_cfg;
  gboolean show_cmd;
  gboolean show_alpha;
  gboolean show_thumbnail;
  gboolean show_bboxes;
   
  gboolean dragging;
  
  int frame_series;
  int frame_index;
  int frame_callback_tag;
  int frame_interval;
  int frame_format;
  
  stg_model_t* selection_active;
  stg_model_t* selection_last;
  stg_pose_t selection_pose_start;
  stg_point_3d_t selection_pointer_start;
  
  GList* toggle_list;  
};

void  signal_destroy( GtkObject *object, gpointer user_data );
gboolean  signal_delete( GtkWidget *widget, GdkEvent *event, gpointer user_data );

void gui_enable_alpha( stg_world_t* world, int enable );
GtkWidget* gui_create_canvas( stg_world_t* world );
int gui_model_create( stg_model_t* mod );

#endif // GUI_H
