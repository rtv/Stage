
#ifndef STG_GUI_H
#define STG_GUI_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "stage.h"
#include "rtk.h"
#include "world.h"
#include "model.h"


// GUI colors   
#define STG_GRID_MAJOR_COLOR "gray85"
#define STG_GRID_MINOR_COLOR "gray95"
#define STG_GRID_AXIS_COLOR "gray40"
#define STG_BACKGROUND_COLOR "ivory"
#define STG_BOUNDINGBOX_COLOR "magenta"
#define STG_MATRIX_COLOR "dark green"

// model color defaults
#define STG_GENERIC_COLOR "black"
#define STG_POSITION_COLOR "red"

#define STG_LASER_COLOR "light blue"
#define STG_LASER_GEOM_COLOR "blue"
#define STG_LASER_CFG_COLOR "light steel blue"

#define STG_FIDUCIAL_COLOR "lime green"
#define STG_FIDUCIAL_CFG_COLOR "green"

#define STG_ENERGY_COLOR "purple"
#define STG_ENERGY_CFG_COLOR "magenta"
   
#define STG_RANGER_COLOR "gray75" 
#define STG_RANGER_GEOM_COLOR "orange"
#define STG_RANGER_CONFIG_COLOR "gray90"

#define STG_DEBUG_COLOR "green"
#define STG_BLOB_CFG_COLOR "gray75"

#define STG_LAYER_BACKGROUND 10
#define STG_LAYER_BODY 30
#define STG_LAYER_GRID 20
#define STG_LAYER_USER 99
#define STG_LAYER_GEOM 80
#define STG_LAYER_MATRIX 40
#define STG_LAYER_DEBUG 98

#define STG_LAYER_LASERGEOM 31
#define STG_LAYER_LASERDATA 25
#define STG_LAYER_LASERCONFIG 32

#define STG_LAYER_RANGERGEOM 41
#define STG_LAYER_RANGERCONFIG 42
#define STG_LAYER_RANGERDATA 43

#define STG_LAYER_BLOBGEOM 51
#define STG_LAYER_BLOBCONFIG 52
#define STG_LAYER_BLOBDATA 53

#define STG_LAYER_NEIGHBORGEOM 61
#define STG_LAYER_NEIGHBORCONFIG 62
#define STG_LAYER_NEIGHBORDATA 63

#define STG_LAYER_ENERGYGEOM 71
#define STG_LAYER_ENERGYCONFIG 72
#define STG_LAYER_ENERGYDATA 73

#define STG_DEFAULT_WINDOW_WIDTH 700
#define STG_DEFAULT_WINDOW_HEIGHT 740
#define STG_DEFAULT_PPM 40
#define STG_DEFAULT_SHOWGRID 1
#define STG_DEFAULT_MOVIE_SPEED 1


typedef struct _gui_window
{
  rtk_canvas_t* canvas;
  
  world_t* world; // every window shows a single world

  // rtk doesn't support status bars, so we'll use gtk directly
  GtkStatusbar* statusbar;
  GtkLabel* timelabel;

  //GHashTable* guimods;

  rtk_fig_t* bg; // background
  rtk_fig_t* matrix;
  rtk_fig_t* poses;

  gboolean show_matrix;
  gboolean show_grid;
  gboolean show_rects;
  gboolean show_nose;
  gboolean show_geom;
  
  gboolean movie_exporting;
  int movie_speed;
  int movie_count;
  
  rtk_menu_t** menus;
  rtk_menuitem_t** mitems;
  int menu_count;
  int mitem_count;
  
  rtk_menuitem_t** mitems_mspeed;
  int mitems_mspeed_count;
  
} gui_window_t;


void gui_startup( int* argc, char** argv[] ); 
void gui_poll( void );
void gui_shutdown( void );

gui_window_t* gui_world_create( world_t* world );
void gui_world_destroy( world_t* world );
void gui_world_update( world_t* world );

void gui_model_create( model_t* model );
void gui_model_destroy( model_t* model );

void gui_model_render( model_t* model );

void gui_model_update( model_t* mod, stg_prop_type_t prop );

void gui_model_nose( model_t* model );
void gui_model_pose( model_t* mod );
void gui_model_geom( model_t* model );
void gui_model_lines( model_t* model );
void gui_model_nose( model_t* model );
void gui_model_rangers( model_t* mod );
void gui_model_rangers_data( model_t* mod );


void gui_model_laser_data( model_t* mod );
void gui_model_laser( model_t* mod );

//void gui_model_velocity( model_t* mod, stg_velocity_t* vel );

gui_model_t* gui_model_figs( model_t* model );

void gui_window_menus_create( gui_window_t* win );
void gui_window_menus_destroy( gui_window_t* win );

#ifdef __cplusplus
  }
#endif 


#endif
