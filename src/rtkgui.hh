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

typedef struct
{
  // Timing info for the gui.
  // [update_interval] is the time to wait between GUI updates (simulated seconds).
  // [root_check_interval] is the time to wait between fitting the root entity nicely into the canvas
  double rtkgui_update_interval;
  double rtkgui_fit_interval; 
  
  // Basic GUI elements
  rtk_canvas_t *canvas ; // the canvas
  rtk_fig_t* fig_matrix;
  rtk_fig_t* fig_grid;

  // The file menu
  rtk_menu_t *file_menu;
  rtk_menuitem_t *save_menuitem;
  rtk_menuitem_t *exit_menuitem;
  
  // The stills menu
  rtk_menu_t *stills_menu;
  rtk_menuitem_t *stills_jpeg_menuitem;
  rtk_menuitem_t *stills_ppm_menuitem;
  
  // Export stills info
  int stills_series;
  int stills_count;
  
  // The movie menu  
  rtk_menu_t *movie_menu;
  int movie_option_count;
  stg_gui_movie_option_t movie_options[10];
  
  // Export movie info
  int movie_count;
  
  // The view menu
  rtk_menu_t *view_menu;
  rtk_menuitem_t *grid_item;
  rtk_menuitem_t *walls_item;
  rtk_menuitem_t *matrix_item; 
  rtk_menuitem_t *objects_item;
  rtk_menuitem_t *data_item;
  rtk_menuitem_t *lights_item;

  // The action menu
  rtk_menu_t* action_menu;
  rtk_menuitem_t* subscribedonly_item;
  rtk_menuitem_t* autosubscribe_item;
  int autosubscribe;
  
  // The view/device menu
  stg_gui_menu_t device_menu;
  
  // the view/data menu
  stg_gui_menu_t data_menu;
  
  // Number of exported images
  int export_count;

  // rtk doesn't support status bars, so we'll use gtk directly
  GtkStatusbar* statusbar;

  guint source_tag;
  
  struct stg_world* world;

} stg_gui_window_t;


/*typedef enum
  {
  STG_FIG_BODY = 0,
  STG_FIG_NUM_TYPES; //must be last in this enum
  }*/

typedef struct
{
  CEntity* ent;
  stg_gui_window_t* win;

  bool grid_enable;
  double grid_major, grid_minor;
  
  rtk_fig_t *fig, *fig_rects, *fig_grid, *fig_user, *fig_trans, *fig_light;   

  int movemask;
  
  //int type; // the model type
} stg_gui_model_t;

//INIT
int stg_gui_init( int* argc, char*** argv );

// WINDOWS
stg_gui_window_t* stg_gui_window_create( struct stg_world* world, int width, int height);
void stg_gui_window_destroy( stg_gui_window_t* win );
int stg_gui_window_update( struct stg_world* world, stg_prop_id_t prop );

gboolean stg_gui_window_callback( gpointer win );

// MODELS
stg_gui_model_t* stg_gui_model_create(CEntity* ent);
void stg_gui_model_destroy( stg_gui_model_t* model );
int stg_gui_model_update( CEntity* ent, stg_prop_id_t prop );
void stg_gui_transducers_render( CEntity* ent );
// render the entity's laser data
void stg_gui_laser_render( CEntity* ent );
void stg_gui_neighbor_render( CEntity* ent, GArray* neighbors );

// MISC
rtk_fig_t* stg_gui_grid_create( rtk_canvas_t* canvas, rtk_fig_t* parent, 
				double origin_x, double origin_y, double origin_a, 
				double width, double height, double major, double minor );


void RtkOnMouse(rtk_fig_t *fig, int event, int mode);

int stg_gui_los_msg_send( CEntity* ent, stg_los_msg_t* msg );
int stg_gui_los_msg_recv( CEntity* receiver, CEntity* sender, 
			  stg_los_msg_t* msg );
#endif
