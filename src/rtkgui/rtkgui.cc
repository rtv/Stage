/*
 *  Stage : a multi-robot simulator.
 *  Copyright (C) 2001, 2002 Richard Vaughan, Andrew Howard and Brian Gerkey.
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
/*
 * Desc: The RTK gui implementation
 * Author: Richard Vaughan, Andrew Howard
 * Date: 7 Dec 2000
 * CVS info: $Id: rtkgui.cc,v 1.1.2.6 2003-02-08 01:20:37 rtv Exp $
 */

//
// all this GUI stuff should be unravelled from the CWorld class eventually - rtv
//

//#undef DEBUG
//#undef VERBOSE
#define DEBUG 
//#define VERBOSE

#include <errno.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <signal.h>
#include <math.h>
#include <unistd.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pwd.h>
#include <stdio.h>
#include <netdb.h>

#include <fstream>
#include <iostream>

#include "rtk.h"
#include "library.hh"
#include "entity.hh"
#include "matrix.hh"
#include "rtkgui.hh"

extern int quit;

// Basic GUI elements
rtk_canvas_t *canvas = NULL; // the canvas
rtk_app_t *app = NULL; // the RTK/GTK application

rtk_fig_t* matrix_fig = NULL;

// Timing info for the gui.
// [rtk_update_rate] is the update rate (Hz).
// [rtk_update_time] is the last time the gui was update (simulation time).
double rtk_update_rate;
double rtk_update_time;

// The file menu
rtk_menu_t *file_menu= NULL;;
rtk_menuitem_t *save_menuitem= NULL;;
rtk_menuitem_t *exit_menuitem= NULL;;

// The stills menu
rtk_menu_t *stills_menu;
rtk_menuitem_t *stills_jpeg_menuitem= NULL;;
rtk_menuitem_t *stills_ppm_menuitem= NULL;;

// Export stills info
int stills_series;
int stills_count;

// The movie menu  
rtk_menu_t *movie_menu= NULL;;
struct CMovieOption
{
  rtk_menuitem_t *menuitem;
  int speed;
};
int movie_option_count;
CMovieOption movie_options[10];

// Export movie info
int movie_count;

// The view menu
rtk_menu_t *view_menu= NULL;;
rtk_menuitem_t *grid_item= NULL;;
rtk_menuitem_t *walls_item= NULL;;
rtk_menuitem_t *matrix_item= NULL;;
rtk_menuitem_t *objects_item= NULL;;
rtk_menuitem_t *data_item= NULL;;

// The action menu
rtk_menu_t* action_menu= NULL;;
rtk_menuitem_t* subscribedonly_item= NULL;;
rtk_menuitem_t* autosubscribe_item= NULL;;
int autosubscribe;

typedef struct 
{
  rtk_menu_t *menu;
  rtk_menuitem_t *items[ 1000 ]; // TODO - get rid of this fixed size buffer
} stage_menu_t;

// The view/device menu
stage_menu_t device_menu;

// the view/data menu
stage_menu_t data_menu;


// Number of exported images
int export_count;

// these are internal and can only be called in rtkgui.cc
static void RtkMenuHandling();
static void RtkUpdateMovieMenu();

// TODO - unwrap RTK code from around the models and put it in this subdir or file

// THESE FUNCTIONS ARE THE STANDARD INTERFACE FOR STAGE GUIS /////////////

// allow the GUI to do any startup it needs to, including cmdline parsing
int RtkGuiInit( int argc, char** argv )
{ 
  PRINT_DEBUG( "init_func" );
  return( rtk_init(&argc, &argv) );
}


int RtkGuiCreateApp( void )
{
  app = rtk_app_create();
  canvas = rtk_canvas_create(app);
      
  // Add some menu items
  file_menu = rtk_menu_create(canvas, "File");
  save_menuitem = rtk_menuitem_create(file_menu, "Save", 0);
  stills_menu = rtk_menu_create_sub(file_menu, "Capture stills");
  movie_menu = rtk_menu_create_sub(file_menu, "Capture movie");
  exit_menuitem = rtk_menuitem_create(file_menu, "Exit", 0);
  
  stills_jpeg_menuitem = rtk_menuitem_create(stills_menu, "JPEG format", 1);
  stills_ppm_menuitem = rtk_menuitem_create(stills_menu, "PPM format", 1);
  stills_series = 0;
  stills_count = 0;
  
  movie_options[0].menuitem = rtk_menuitem_create(movie_menu, "Speed x1", 1);
  movie_options[0].speed = 1;
  movie_options[1].menuitem = rtk_menuitem_create(movie_menu, "Speed x2", 1);
  movie_options[1].speed = 2;
  movie_options[2].menuitem = rtk_menuitem_create(movie_menu, "Speed x5", 1);
  movie_options[2].speed = 5;
  movie_options[3].menuitem = rtk_menuitem_create(movie_menu, "Speed x10", 1);
  movie_options[3].speed = 10;
  movie_option_count = 4;
  
  movie_count = 0;
  
  // Create the view menu
  view_menu = rtk_menu_create(canvas, "View");
  // create the view menu items and set their initial checked state
  grid_item = rtk_menuitem_create(view_menu, "Grid", 1);
  matrix_item = rtk_menuitem_create(view_menu, "Matrix", 1);
  objects_item = rtk_menuitem_create(view_menu, "Objects", 1);
  data_item = rtk_menuitem_create(view_menu, "Data", 1);
  
  rtk_menuitem_check(matrix_item, 0);
  rtk_menuitem_check(objects_item, 1);
  rtk_menuitem_check(data_item, 1);
  
  // create the action menu
  action_menu = rtk_menu_create(canvas, "Action");
  subscribedonly_item = rtk_menuitem_create(action_menu, 
					    "Subscribe to all", 1);
  
   
  /* BROKEN
  //zero the view menus
  memset( &device_menu,0,sizeof(stage_menu_t));
  memset( &data_menu,0,sizeof(stage_menu_t));
  
  // Create the view/device sub menu
  assert( data_menu.menu = 
  rtk_menu_create_sub(view_menu, "Data"));
  
  // Create the view/data sub menu
  assert( device_menu.menu = 
  rtk_menu_create_sub(view_menu, "Object"));
  
  // each device adds itself to the correct view menus in its rtkstartup()
  */
}

// Initialise the GUI

int RtkGuiLoad( stage_gui_config_t* cfg )
{
  PRINT_DEBUG( "load_func" );

  int width = cfg->width; // window size in pixels
  int height = cfg->height; 

  // (TODO - we could get the sceen size from X if we tried?)
  if (width > 1024)
    width = 1024;
  if (height > 768)
    height = 768;
  
  // get this stuff out of the config eventually
  double scale = 1/cfg->ppm;
  double dx, dy;
  double ox, oy;

  // Size in meters
  dx = width * scale;
  dy = height * scale;
  
  // Origin of the canvas
  ox = cfg->originx + dx/2.0;
  oy = cfg->originy + dy/2.0;

  rtk_update_time = 0;
  rtk_update_rate = 10;
  
  if( app == NULL ) // we need to create the basic data for the app
    {
      RtkGuiCreateApp(); // this builds the app, canvas, menus, etc
      // Start the gui; dont run in a separate thread and dont let it do
      // its own updates.
      rtk_app_main_init(app);
    }  
  
  // configure the GUI
  rtk_canvas_size( canvas, width, height );
  rtk_canvas_scale( canvas, scale, scale);
  rtk_canvas_origin( canvas, ox, oy);
  
  // check the menu items appropriately
  rtk_menuitem_check(grid_item, cfg->showgrid);
  rtk_menuitem_check(subscribedonly_item, cfg->showsubscribedonly);
 
  rtk_canvas_render( canvas );

  return 0; // success
}

// called frequently so the GUI can handle events
// the rtk gui is contained inside Stage's main thread, so it does a lot
// of work here.
// (if the GUI has its own thread you may not need to do much in here)
int RtkGuiUpdate( void )
{  
  //PRINT_DEBUG( "updating gui" );
  
  // if we have no GUI data, do nothing
  if( !app || !canvas )
    return 0; 

  // Process events
  rtk_app_main_loop( app);
  
  // Refresh the gui at a fixed rate (in simulator time).
  //if (this->m_sim_time < this->rtk_update_time + 1 / this->rtk_update_rate)
  //return;
  //this->rtk_update_time = 
  //this->m_sim_time - fmod(this->m_sim_time, 1 / this->rtk_update_rate);
  
  // Process menus
  RtkMenuHandling();      
  
  // Update the object tree
  //root->RtkUpdate();
  
  // Render the canvas
  rtk_canvas_render( canvas);
  
  return 0; // success
  
  //RtkUpdate();
}


// have to wrap the polymorphic call as this is a callback
int RtkGuiEntityStartup( CEntity* ent )
{ 
  PRINT_DEBUG1( "gui startup for ent %d", ent->stage_id );
  
  // explot the polymorphism here
  assert(ent);
  ent->RtkStartup();
}

// have to wrap the polymorphic call as this is a callback
int RtkGuiEntityShutdown( CEntity* ent )
{
  PRINT_DEBUG1( "gui shutdown for ent %d", ent->stage_id );

  // explot the polymorphism here
  assert(ent);
  ent->RtkShutdown();
}

void UnrenderMatrix()
{
  if( matrix_fig )
    {
      rtk_fig_destroy( matrix_fig );
      matrix_fig = NULL;
    }
}

void RenderMatrix( )
{
  assert( CEntity::matrix );

  // Create a figure representing this object
  if( matrix_fig )
    UnrenderMatrix();
  
  matrix_fig = rtk_fig_create( canvas, NULL, 60);
  
  // Set the default color 
  rtk_fig_color_rgb32( matrix_fig, ::LookupColor(MATRIX_COLOR) );
  
  double pixel_size = 1.0 / CEntity::matrix->ppm;
  
  // render every pixel as an unfilled rectangle
  for( int y=0; y<CEntity::matrix->get_height(); y++ )
    for( int x=0; x<CEntity::matrix->get_width(); x++ )
      if( *(CEntity::matrix->get_cell( x, y )) )
	rtk_fig_rectangle( matrix_fig, x*pixel_size, y*pixel_size, 0, pixel_size, pixel_size, 0 );
}



int RtkGuiEntityUpdate( CEntity* ent )
{
  assert(ent);
  ent->RtkUpdate();
}

// this functionality was previously in CEntity::SetProperty - this is
// a lot cleaner, though I didn't want to shoehorn it into the world
// class so it's sitting out here as a regular function. - rtv
int RtkGuiEntityPropertyChange( CEntity* ent, stage_prop_id_t prop )
{
  PRINT_DEBUG2( "setting prop %d on ent %d", prop, ent->stage_id );

  assert(ent);
  assert( prop < STG_PROPERTY_COUNT );
  
  // if it has no fig, do nothing
  if( !ent->fig )
    return 0; 
  
  double px, py, pa;
  
  switch( prop )
    {
      // these require just moving the figure
    case STG_PROP_ENTITY_POSE:
      ent->GetPose( px, py, pa );
      PRINT_DEBUG3( "moving figure to %.2f %.2f %.2f", px,py,pa );
      rtk_fig_origin(ent->fig, px, py, pa );
      break;

      // these all need us to totally redraw the object
    case STG_PROP_ENTITY_PARENT:
    case STG_PROP_ENTITY_SIZE:
    case STG_PROP_ENTITY_ORIGIN:
    case STG_PROP_ENTITY_NAME:
    case STG_PROP_ENTITY_COLOR:
    case STG_PROP_ENTITY_LASERRETURN:
    case STG_PROP_ENTITY_SONARRETURN:
    case STG_PROP_ENTITY_IDARRETURN:
    case STG_PROP_ENTITY_OBSTACLERETURN:
    case STG_PROP_ENTITY_VISIONRETURN:
    case STG_PROP_ENTITY_PUCKRETURN:
    case STG_PROP_ENTITY_PLAYERID:
    case STG_PROP_ENTITY_RECTS:
    case STG_PROP_ENTITY_CIRCLES:
      RtkGuiEntityShutdown( ent ); 
      RtkGuiEntityStartup( ent );
      break;

      // these are ignored for now
    case STG_PROP_ENTITY_COMMAND:
    case STG_PROP_ENTITY_DATA:
    case STG_PROP_ENTITY_CONFIG:
      PRINT_DEBUG1( "gui redraw command/data for %d", prop ); 
      ent->RtkUpdate();
      break;
      
    default:
      PRINT_WARN1( "property change %d unhandled by rtkgui", prop ); 
      break;
    } 
  
  return 0;
}
 


// END OF INTERFACE FUNCTIONS //////////////////////////////////////////

void AddToMenu( stage_menu_t* menu, CEntity* ent, int check )
{
  /* BROKEN
  assert( menu );
  assert( ent );

  // if there's no menu item for this type yet
  if( menu->items[ ent->lib_entry->type_num ] == NULL )
    // create a new menu item
    assert( menu->items[ ent->lib_entry->type_num ] =  
	    rtk_menuitem_create( menu->menu, ent->lib_entry->token, 1 ));
  
  rtk_menuitem_check( menu->items[ ent->lib_entry->type_num ], check );
  */
}


void AddToDataMenu(  CEntity* ent, int check )
{
  /* BROKEN
  assert( ent );
  AddToMenu( &this->data_menu, ent, check );
  */
}

void AddToDeviceMenu(  CEntity* ent, int check )
{
  /* BROKEN
  assert( ent );
  AddToMenu( &this->device_menu, ent, check );
  */
}

// devices check this to see if they should display their data
bool ShowDeviceData( int devtype )
{
  //return rtk_menuitem_ischecked(this->data_item);
  
  /* BROKEN
  rtk_menuitem_t* menu_item = data_menu.items[ devtype ];
  
  if( menu_item )
    return( rtk_menuitem_ischecked( menu_item ) );  
  else // if there's no option in the menu, display this data
    return true;
  */
}

bool ShowDeviceBody( int devtype )
{
  //return rtk_menuitem_ischecked(this->objects_item);

  /* BROKEN
  rtk_menuitem_t* menu_item = device_menu.items[ devtype ];
  
  if( menu_item )
    return( rtk_menuitem_ischecked( menu_item ) );  
  else // if there's no option in the menu, display this data
    return true;
  */
}



// Update the GUI
void RtkMenuHandling()
{
  char filename[128];
    
  // See if we need to quit the program
  if (rtk_menuitem_isactivated(exit_menuitem))
    ::quit = 1;
  if (rtk_canvas_isclosed(canvas))
    ::quit = 1;

  // Save the world file
  //if (rtk_menuitem_isactivated(save_menuitem))
  //Save();
  
  // Start/stop export (jpeg)
  if (rtk_menuitem_isactivated(stills_jpeg_menuitem))
  {
    if (rtk_menuitem_ischecked(stills_jpeg_menuitem))
    {
      stills_series++;
      rtk_menuitem_enable(stills_ppm_menuitem, 0);
    }
    else
      rtk_menuitem_enable(stills_ppm_menuitem, 1);      
  }
  if (rtk_menuitem_ischecked(stills_jpeg_menuitem))
  {
    snprintf(filename, sizeof(filename), "stage-%03d-%04d.jpg",
             stills_series, stills_count++);
    printf("saving [%s]\n", filename);
    rtk_canvas_export_image(canvas, filename, RTK_IMAGE_FORMAT_JPEG);
  }

  // Start/stop export (ppm)
  if (rtk_menuitem_isactivated(stills_ppm_menuitem))
  {
    if (rtk_menuitem_ischecked(stills_ppm_menuitem))
    {
      stills_series++;
      rtk_menuitem_enable(stills_jpeg_menuitem, 0);
    }
    else
      rtk_menuitem_enable(stills_jpeg_menuitem, 1);      
  }
  if (rtk_menuitem_ischecked(stills_ppm_menuitem))
  {
    snprintf(filename, sizeof(filename), "stage-%03d-%04d.ppm",
             stills_series, stills_count++);
    printf("saving [%s]\n", filename);
    rtk_canvas_export_image(canvas, filename, RTK_IMAGE_FORMAT_PPM);
  }

  // Update movie menu
  RtkUpdateMovieMenu();
  
  // Show or hide the grid
  //if (rtk_menuitem_ischecked(grid_item))
  //rtk_fig_show( CEntity::rootfig_grid, 1);
  //else
  //rtk_fig_show(fig_grid, 0);

  // clear any matrix rendering, then redraw if the emnu item is checked
  //matrix->unrender();
  if (rtk_menuitem_ischecked(matrix_item))
    RenderMatrix();
  else
    if( matrix_fig ) UnrenderMatrix();
  
  // enable/disable subscriptions to show sensor data
  static bool lasttime = rtk_menuitem_ischecked(subscribedonly_item);
  bool thistime = rtk_menuitem_ischecked(subscribedonly_item);

  // for now i check if the menu item changed
  if( thistime != lasttime )
  {
    if( thistime )  // change the subscription counts of any player-capable ent
      CEntity::root->FamilySubscribe();
    else
      CEntity::root->FamilyUnsubscribe();
    // remember this state
    lasttime = thistime;
  }
      
  return;
}


// Handle the movie sub-menu.
// This is still yucky.
void RtkUpdateMovieMenu()
{
  int i, j;
  char filename[128];
  CMovieOption *option;
  
  for (i = 0; i < movie_option_count; i++)
  {
    option = movie_options + i;
    
    if (rtk_menuitem_isactivated(option->menuitem))
    {
      if (rtk_menuitem_ischecked(option->menuitem))
      {
        // Start the movie capture
        snprintf(filename, sizeof(filename), "stage-%03d-sp%02d.mpg",
                 movie_count++, option->speed);
        rtk_canvas_movie_start(canvas, filename,
                               rtk_update_rate, option->speed);

        // Disable all other capture options
        for (j = 0; j < movie_option_count; j++)
          rtk_menuitem_enable(movie_options[j].menuitem, i == j);
      }
      else
      {
        // Stop movie capture
        rtk_canvas_movie_stop(canvas);

        // Enable all capture options
        for (j = 0; j < movie_option_count; j++)
          rtk_menuitem_enable(movie_options[j].menuitem, 1);
      }
    }

    // Export the frame
    if (rtk_menuitem_ischecked(option->menuitem))
      rtk_canvas_movie_frame(canvas);
  }
  return;
}



