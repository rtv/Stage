
#ifndef STG_GUI_H
#define STG_GUI_H

#include "stage.h"
#include "rtk.h"
#include "world.h"
#include "model.h"

// GUI colors   
#define STG_GRID_MAJOR_COLOR "gray85"
#define STG_GRID_MINOR_COLOR "gray95"
#define STG_GRID_AXIS_COLOR "gray40"
#define STG_MATRIX_COLOR "dark green"
#define STG_BACKGROUND_COLOR "ivory"
#define STG_BOUNDINGBOX_COLOR "magenta"

// model color defaults
#define STG_GENERIC_COLOR "black"
#define STG_POSITION_COLOR "red"
#define STG_LASER_COLOR "blue"
#define STG_LASER_DATA_COLOR "powder blue"
#define STG_WALL_COLOR "dark blue"
#define STG_SONAR_COLOR "gray70"
#define STG_FIDUCIAL_COLOR "goldenrod2"
#define STG_RANGER_COLOR "gray90" 

#define STG_LAYER_BACKGROUND 20
#define STG_LAYER_BODY 30
#define STG_LAYER_DATA 60
#define STG_LAYER_LIGHT 70
#define STG_LAYER_SENSOR 55
#define STG_LAYER_GRID 20
#define STG_LAYER_USER 99
#define STG_LAYER_GEOM 80
#define STG_LAYER_MATRIX 40

#define STG_DEFAULT_WINDOW_WIDTH 700
#define STG_DEFAULT_WINDOW_HEIGHT 740
#define STG_DEFAULT_PPM 40
#define STG_DEFAULT_SHOWGRID 1
#define STG_DEFAULT_MOVIE_SPEED 1

typedef struct
{
  rtk_fig_t* top;

  rtk_fig_t* rangers;
  rtk_fig_t* ranger_data;
  rtk_fig_t* geom;
  rtk_fig_t* laser;
  rtk_fig_t* laser_data;
} gui_model_t;

typedef struct
{
  rtk_canvas_t* canvas;
  
  // rtk doesn't support status bars, so we'll use gtk directly
  GtkStatusbar* statusbar;
  
  GHashTable* guimods;

  rtk_fig_t* bg; // background
  rtk_fig_t* grid; 
  rtk_fig_t* matrix;
  rtk_fig_t* poses;

  gboolean show_matrix;
  gboolean show_grid;
  gboolean show_rangerdata;
  gboolean show_rangers;
  gboolean show_rects;
  gboolean show_nose;
  gboolean show_geom;
  gboolean show_laser;
  gboolean show_laserdata;

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

void gui_world_create( stg_world_t* world );
void gui_world_destroy( stg_world_t* world );
void gui_world_update( stg_world_t* world );

void gui_model_create( stg_model_t* model );
void gui_model_destroy( stg_model_t* model );

void gui_model_render( stg_model_t* model );

void gui_model_update( stg_model_t* mod, stg_prop_type_t prop );

void gui_model_nose( stg_model_t* model );
void gui_model_pose( stg_model_t* mod );
void gui_model_rects( stg_model_t* model );
void gui_model_nose( stg_model_t* model );
void gui_model_rangers( stg_model_t* mod );
void gui_model_rangers_data( stg_model_t* mod );

//void gui_model_size( stg_model_t* mod, stg_size_t* size );
//void gui_model_velocity( stg_model_t* mod, stg_velocity_t* vel );

gui_model_t* gui_model_figs( stg_model_t* model );

void gui_window_menus_create( gui_window_t* win );
void gui_window_menus_destroy( gui_window_t* win );

#endif
