#ifndef _RTKGUI_HH
#define _RTKGUI_HH

#if HAVE_CONFIG_H
#  include <config.h>
#endif

//#ifdef INCLUDE_RTK

#include <rtk.h>

class CEntity;
struct stg_world;

typedef struct 
{
  rtk_menu_t *menu;
  rtk_menuitem_t *items[ 1000 ]; // TODO - get rid of this fixed size buffer
} stg_gui_menu_t;

typedef struct 
{
  rtk_menuitem_t *menuitem;
  int speed;
} stg_gui_movie_option_t;

typedef rtk_app_t stg_gui_app_t;

typedef enum
  {
    STG_GUI_DATA_NEIGHBORS,
    STG_GUI_DATA_RANGER,
    STG_GUI_DATA_LASER,
    STG_GUI_DATA_BLOBFINDER,
    STG_GUI_DATA_COUNT // this must be the last entry
  } stg_gui_data_types;

typedef enum
  {
    STG_GUI_OBJECT_RECT,
    STG_GUI_OBJECT_LIGHT,
    STG_GUI_OBJECT_SENSOR,
    STG_GUI_OBJECT_NOSE,
    STG_GUI_OBJECT_USER,
    STG_GUI_OBJECT_COUNT // this must be the last entry
  } stg_gui_object_types;


enum {
  STG_MITEM_FILE_SAVE,
  STG_MITEM_FILE_LOAD,
  STG_MITEM_FILE_QUIT,
  STG_MITEM_FILE_STILLS__JPG,
  STG_MITEM_FILE_STILLS_PPM,
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
  STG_MENU_FILE_STILLS,
  STG_MENU_FILE_MOVIES,
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
  
  // The movie menu  
  rtk_menu_t *movie_menu;
  int movie_option_count;
  stg_gui_movie_option_t movie_options[10];
  
  // Export movie info
  int movie_count;
  
  // Number of exported images
  int export_count;

  // a list of figures that should be cleared soon
  GList* countdowns;

  // rtk doesn't support status bars, so we'll use gtk directly
  GtkStatusbar* statusbar;

  guint tag_refresh;
  guint tag_countdown;
  
  struct stg_world* world;

} stg_gui_window_t;

typedef struct
{
  rtk_fig_t* fig;
  int timeleft; //milliseconds until the figure must be cleared
} stg_gui_countdown_t;


typedef struct
{
  CEntity* ent;
  stg_gui_window_t* win;

  //bool grid_enable;
  //double grid_major, grid_minor;
  int movemask;
  
  // a figure for each of our object types
  rtk_fig_t* fig[STG_GUI_DATA_COUNT];
  // a  countdown figure for each of our sensor types
  stg_gui_countdown_t datafigs[STG_GUI_DATA_COUNT];
} stg_gui_model_t;

//INIT
int stg_gui_init( int* argc, char*** argv );

// WINDOWS
stg_gui_window_t* stg_gui_window_create( struct stg_world* world, int width, int height);
void stg_gui_window_destroy( stg_gui_window_t* win );
int stg_gui_window_update( struct stg_world* world, stg_prop_id_t prop );

gboolean stg_gui_window_callback( gpointer win );

// a timeout callback that clears all figures in the countdowns list
gboolean stg_gui_window_clear_countdowns( gpointer data );


// MODELS
stg_gui_model_t* stg_gui_model_create(CEntity* ent);
void stg_gui_model_destroy( stg_gui_model_t* model );
int stg_gui_model_update( CEntity* ent, stg_prop_id_t prop );
void stg_gui_rangers_render( CEntity* ent );
// render the entity's laser data
void stg_gui_laser_render( CEntity* ent );
void stg_gui_neighbor_render( CEntity* ent, GArray* neighbors );
void stg_gui_model_rangers( CEntity* ent );

// MISC
rtk_fig_t* stg_gui_grid_create( rtk_canvas_t* canvas, rtk_fig_t* parent, 
				double origin_x, double origin_y, double origin_a, 
				double width, double height, double major, double minor );


void RtkOnMouse(rtk_fig_t *fig, int event, int mode);

int stg_gui_los_msg_send( CEntity* ent, stg_los_msg_t* msg );
int stg_gui_los_msg_recv( CEntity* receiver, CEntity* sender, 
			  stg_los_msg_t* msg );
#endif
