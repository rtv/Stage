#ifndef _RTKGUI_HH
#define _RTKGUI_HH

#if HAVE_CONFIG_H
#  include <config.h>
#endif

//#ifdef INCLUDE_RTK

#include <rtk.h>

class CEntity;
struct ss_world;

typedef struct 
{
  rtk_menu_t *menu;
  rtk_menuitem_t *items[ 1000 ]; // TODO - get rid of this fixed size buffer
} gui_menu_t;

typedef struct 
{
  rtk_menuitem_t *menuitem;
  int speed;
} gui_movie_option_t;

typedef rtk_app_t gui_app_t;

typedef enum
  {
    GUI_DATA_NEIGHBORS,
    GUI_DATA_RANGER,
    GUI_DATA_LASER,
    GUI_DATA_BLOBFINDER,
    GUI_DATA_COUNT // this must be the last entry
  } gui_data_types;

typedef enum
  {
    GUI_OBJECT_RECT,
    GUI_OBJECT_LIGHT,
    GUI_OBJECT_SENSOR,
    GUI_OBJECT_NOSE,
    GUI_OBJECT_USER,
    GUI_OBJECT_COUNT // this must be the last entry
  } gui_object_types;


enum {
  STG_MITEM_FILE_SAVE,
  STG_MITEM_FILE_LOAD,
  STG_MITEM_FILE_QUIT,
  STG_MITEM_FILE_IMAGE_JPG,
  STG_MITEM_FILE_IMAGE_PPM,
  STG_MITEM_FILE_MOVIE_1,
  STG_MITEM_FILE_MOVIE_2,
  STG_MITEM_FILE_MOVIE_5,
  STG_MITEM_FILE_MOVIE_10,
  STG_MITEM_VIEW_MATRIX,
  STG_MITEM_VIEW_GRID,
  STG_MITEM_VIEW_OBJECT_BODY,
  STG_MITEM_VIEW_OBJECT_LIGHT,
  STG_MITEM_VIEW_OBJECT_SENSOR,
  STG_MITEM_VIEW_OBJECT_USER,
  STG_MITEM_VIEW_DATA_NEIGHBORS,
  STG_MITEM_VIEW_DATA_LASER,
  STG_MITEM_VIEW_DATA_RANGER,
  STG_MITEM_VIEW_DATA_BLOBFINDER,
  STG_MITEM_VIEW_REFRESH_25,
  STG_MITEM_VIEW_REFRESH_50,
  STG_MITEM_VIEW_REFRESH_100,
  STG_MITEM_VIEW_REFRESH_200,
  STG_MITEM_VIEW_REFRESH_500,
  STG_MITEM_VIEW_REFRESH_1000,
  STG_MITEM_COUNT
};

enum {
  STG_MENU_FILE,
  STG_MENU_FILE_IMAGE,
  STG_MENU_FILE_MOVIE,
  STG_MENU_VIEW,
  STG_MENU_VIEW_REFRESH,
  STG_MENU_VIEW_OBJECT,
  STG_MENU_VIEW_DATA,
  STG_MENU_COUNT
};

typedef struct
{
  // Timing info for the gui.
  // [update_interval] is the time to wait between GUI updates (wall clock time).
  // [root_check_interval] is the time to wait between fitting the root entity nicely into the canvas
  double rtkgui_update_interval;
  double rtkgui_fit_interval; 
  
  // Basic GUI elements
  rtk_canvas_t *canvas ; // the canvas
  rtk_fig_t* fig_matrix;
  rtk_fig_t* fig_grid;
  
  rtk_menu_t *menus[STG_MENU_COUNT];
  rtk_menuitem_t* mitems[STG_MITEM_COUNT];
  
  // Export stills info
  int stills_series;
  int stills_count;
  int stills_exporting;

  // Export movie info
  int movie_count;
  int movies_exporting;
  //guint movie_tag;
  
  // a list of figures that should be cleared soon
  GList* countdowns;

  // rtk doesn't support status bars, so we'll use gtk directly
  GtkStatusbar* statusbar;

  guint tag_refresh;
  guint tag_countdown;
  
  struct ss_world* world;

} gui_window_t;

typedef struct
{
  rtk_fig_t* fig;
  int timeleft; //milliseconds until the figure must be cleared
} gui_countdown_t;


typedef struct
{
  CEntity* ent;
  gui_window_t* win;

  //bool grid_enable;
  //double grid_major, grid_minor;
  int movemask;
  
  // a figure for each of our object types
  rtk_fig_t* fig[GUI_OBJECT_COUNT];
  // a  countdown figure for each of our sensor types
  gui_countdown_t datafigs[GUI_DATA_COUNT];
} gui_model_t;

//INIT
int gui_init( int* argc, char*** argv );

// WINDOWS
gui_window_t* gui_window_create( struct ss_world* world, int width, int height);
void gui_window_destroy( gui_window_t* win );
int gui_window_update( struct ss_world* world, stg_prop_id_t prop );

gboolean gui_window_callback( gpointer win );

// a timeout callback that clears all figures in the countdowns list
gboolean gui_window_clear_countdowns( gpointer data );


// MODELS
gui_model_t* gui_model_create(CEntity* ent);
void gui_model_destroy( gui_model_t* model );
int gui_model_update( CEntity* ent, stg_prop_id_t prop );
void gui_rangers_render( CEntity* ent );
// render the entity's laser data
void gui_laser_render( CEntity* ent );
void gui_neighbor_render( CEntity* ent, GArray* neighbors );
void gui_model_rangers( CEntity* ent );

// MISC
rtk_fig_t* gui_grid_create( rtk_canvas_t* canvas, rtk_fig_t* parent, 
				double origin_x, double origin_y, double origin_a, 
				double width, double height, double major, double minor );


void RtkOnMouse(rtk_fig_t *fig, int event, int mode);

int gui_los_msg_send( CEntity* ent, stg_los_msg_t* msg );
int gui_los_msg_recv( CEntity* receiver, CEntity* sender, 
			  stg_los_msg_t* msg );
#endif
