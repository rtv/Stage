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
 * CVS info: $Id: rtkgui.cc,v 1.15.2.1 2003-02-01 02:14:30 rtv Exp $
 */

//
// all this GUI stuff should be unravelled from the CWorld class eventually - rtv
//

#if HAVE_CONFIG_H
  #include <config.h>
#endif
#if HAVE_STRINGS_H
  #include <strings.h>
#endif

// if stage was configured to use the RTK2 GUI
#ifdef INCLUDE_RTK2

//#undef DEBUG
//#undef VERBOSE
//#define DEBUG 
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

#include "world.hh"
#include "playerdevice.hh"
#include "library.hh"

// handle changes to the entity that might make it look different
void RtkEntityPropertyChange( CEntity* ent, EntityProperty prop );


// TODO - unwrap RTK code from around the place and have it work
// through the GUI hooks

// WORLD HOOKS /////////////////////////////
void GuiInit( int argc, char** argv )
{ 
  rtk_init(&argc, &argv);
}


void GuiUpdate( void )
{  
  RtkUpdate();
}

// ENTITY HOOKS ///////////////////////////
void GuiEntityStartup( CEntity* ent )
{ 
  /* do nothing */  
}

void GuiEntityShutdown( CEntity* ent )
{ 
  /* do nothing */ 
}

void GuiEntityUpdate( CEntity* ent )
{
  /* do nothing */ 
}

void GuiEntityPropertyChange( CEntity* ent, EntityProperty prop )
{ 
  RtkEntityPropertyChange( ent, prop );
}
 
////////////////////////////////////////////

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
  return rtk_menuitem_ischecked(this->data_item);
  
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
  return rtk_menuitem_ischecked(this->objects_item);

  /* BROKEN
  rtk_menuitem_t* menu_item = device_menu.items[ devtype ];
  
  if( menu_item )
    return( rtk_menuitem_ischecked( menu_item ) );  
  else // if there's no option in the menu, display this data
    return true;
  */
}


// Initialise the GUI
// TODO: fix this for client/server operation.
bool RtkLoad(CWorldFile *worldfile)
{
  int sx, sy;
  double scale = 0.01;
  double dx, dy;
  double ox, oy;
  double gridx, gridy;
  double minor, major;
  bool showgrid;
  bool subscribedonly;

  // Size of world in pixels
  sx = (int) this->matrix->width;
  sy = (int) this->matrix->height;
  
  // Grid size in meters
  gridx = sx / this->ppm;
  gridy = sy / this->ppm;
  
  // Place a hard limit, just to stop it going off the screen
  // (TODO - we could get the sceen size from X if we tried?)
  if (sx > 1024)
    sx = 1024;
  if (sy > 768)
    sy = 768;
  
  // Grid spacing
  minor = 0.2;
  major = 1.0;
  showgrid = true;

  // Size in meters
  dx = sx * scale;
  dy = sy * scale;
  
  // Origin of the canvas
  ox = dx / 2;
  oy = dy / 2;
  
  // only show data for subscribed devices
  subscribedonly = true;
  
  // if there's a worldfile we adjust the window params to the specified values
  if (worldfile != NULL)
  {
    int section = worldfile->LookupEntity("gui");

    // Size of canvas in pixels
    sx = (int) worldfile->ReadTupleFloat(section, "size", 0, sx);
    sy = (int) worldfile->ReadTupleFloat(section, "size", 1, sy);
    
    // Scale of the pixels
    scale = worldfile->ReadLength(section, "scale", 1 / this->ppm);
  
    // Size in meters
    dx = sx * scale;
    dy = sy * scale;

    // Origin of the canvas
    ox = worldfile->ReadTupleLength(section, "origin", 0, dx / 2);
    oy = worldfile->ReadTupleLength(section, "origin", 1, dy / 2);

    // Grid spacing
    minor = worldfile->ReadTupleLength(section, "grid", 0, 0.2);
    major = worldfile->ReadTupleLength(section, "grid", 1, 1.0);
    showgrid = worldfile->ReadInt(section, "showgrid", true);
    
    // toggle display of subscribed or all device data
    subscribedonly = worldfile->ReadInt(section, "showsubscribed", false);
  }
  else
    PRINT_DEBUG( "NO WORLDFILE - USING DEFAULT GUI PARAMS" );
  
  //printf( "sx %d sy %d dx %.2f dy %.2f ox %.2f oy %.2f scale %.2f\n",
  //  sx, sy, dx, dy, ox, oy, scale );

  gridx = ceil(gridx / major) * major;
  gridy = ceil(gridy / major) * major;
  
  this->app = rtk_app_create();
  this->rtk_update_time = 0;
  this->rtk_update_rate = 10;
  
  this->canvas = rtk_canvas_create(this->app);
  rtk_canvas_size(this->canvas, sx, sy);
  rtk_canvas_scale(this->canvas, scale, scale);
  rtk_canvas_origin(this->canvas, ox, oy);

  // Add some menu items
  this->file_menu = rtk_menu_create(this->canvas, "File");
  this->save_menuitem = rtk_menuitem_create(this->file_menu, "Save", 0);
  this->stills_menu = rtk_menu_create_sub(this->file_menu, "Capture stills");
  this->movie_menu = rtk_menu_create_sub(this->file_menu, "Capture movie");
  this->exit_menuitem = rtk_menuitem_create(this->file_menu, "Exit", 0);

  this->stills_jpeg_menuitem = rtk_menuitem_create(this->stills_menu, "JPEG format", 1);
  this->stills_ppm_menuitem = rtk_menuitem_create(this->stills_menu, "PPM format", 1);
  this->stills_series = 0;
  this->stills_count = 0;

  this->movie_options[0].menuitem = rtk_menuitem_create(this->movie_menu, "Speed x1", 1);
  this->movie_options[0].speed = 1;
  this->movie_options[1].menuitem = rtk_menuitem_create(this->movie_menu, "Speed x2", 1);
  this->movie_options[1].speed = 2;
  this->movie_options[2].menuitem = rtk_menuitem_create(this->movie_menu, "Speed x5", 1);
  this->movie_options[2].speed = 5;
  this->movie_options[3].menuitem = rtk_menuitem_create(this->movie_menu, "Speed x10", 1);
  this->movie_options[3].speed = 10;
  this->movie_option_count = 4;

  this->movie_count = 0;

  // Create the view menu
  this->view_menu = rtk_menu_create(this->canvas, "View");

  // create the view menu items and set their initial checked state
  this->grid_item = rtk_menuitem_create(this->view_menu, "Grid", 1);
  this->matrix_item = rtk_menuitem_create(this->view_menu, "Matrix", 1);
  this->objects_item = rtk_menuitem_create(this->view_menu, "Objects", 1);
  this->data_item = rtk_menuitem_create(this->view_menu, "Data", 1);
  
  rtk_menuitem_check(this->grid_item, showgrid);
  rtk_menuitem_check(this->matrix_item, 0);
  rtk_menuitem_check(this->objects_item, 1);
  rtk_menuitem_check(this->data_item, 1);
  
  // create the action menu
  this->action_menu = rtk_menu_create(this->canvas, "Action");
  this->subscribedonly_item = rtk_menuitem_create(this->action_menu, 
                                                  "Subscribe to all", 1);

  rtk_menuitem_check(this->subscribedonly_item, subscribedonly);

  /* BROKEN
  //zero the view menus
  memset( &device_menu,0,sizeof(stage_menu_t));
  memset( &data_menu,0,sizeof(stage_menu_t));

  // Create the view/device sub menu
  assert( this->data_menu.menu = 
          rtk_menu_create_sub(this->view_menu, "Data"));

  // Create the view/data sub menu
  assert( this->device_menu.menu = 
          rtk_menu_create_sub(this->view_menu, "Object"));

  // each device adds itself to the correct view menus in its rtkstartup()
  */
  
  // Create the grid
  this->fig_grid = rtk_fig_create(this->canvas, NULL, -49);
  if (minor > 0)
  {
    rtk_fig_color(this->fig_grid, 0.9, 0.9, 0.9);
    rtk_fig_grid(this->fig_grid, gridx/2, gridy/2, gridx, gridy, minor);
  }
  if (major > 0)
  {
    rtk_fig_color(this->fig_grid, 0.75, 0.75, 0.75);
    rtk_fig_grid(this->fig_grid, gridx/2, gridy/2, gridx, gridy, major);
  }
  rtk_fig_show(this->fig_grid, showgrid);
  
  return true;
}

// Save the GUI
bool RtkSave(CWorldFile *worldfile)
{
  int section = worldfile->LookupEntity("gui");
  if (section < 0)
  {
    PRINT_WARN("No gui entity in the world file; gui settings have not been saved.");
    return true;
  }

  // Size of canvas in pixels
  int sx, sy;
  rtk_canvas_get_size(this->canvas, &sx, &sy);
  worldfile->WriteTupleFloat(section, "size", 0, sx);
  worldfile->WriteTupleFloat(section, "size", 1, sy);

  // Origin of the canvas
  double ox, oy;
  rtk_canvas_get_origin(this->canvas, &ox, &oy);
  worldfile->WriteTupleLength(section, "origin", 0, ox);
  worldfile->WriteTupleLength(section, "origin", 1, oy);

  // Scale of the canvas
  double scale;
  rtk_canvas_get_scale(this->canvas, &scale, &scale);
  worldfile->WriteLength(section, "scale", scale);

  // Grid on/off
  int showgrid = rtk_menuitem_ischecked(this->grid_item);
  worldfile->WriteInt(section, "showgrid", showgrid);
  
  return true;
}


// Start the GUI
bool RtkStartup()
{
  // these must exist already
  assert( this->app );
  assert( this->app->canvas );
  
  // Initialise rtk
  rtk_app_main_init(this->app);

  // this rtkstarts all entities
  root->RtkStartup();
    
  return true;
}


// Stop the GUI
void RtkShutdown()
{
  assert( this->app );

  // Finalize rtk
  rtk_app_main_term(this->app);

  return;
}


// Update the GUI
void RtkUpdate()
{
  //PRINT_DEBUG( "updating gui" );

  // Process events
  rtk_app_main_loop(this->app);

  // Refresh the gui at a fixed rate (in simulator time).
  if (this->m_sim_time < this->rtk_update_time + 1 / this->rtk_update_rate)
    return;
  this->rtk_update_time = 
    this->m_sim_time - fmod(this->m_sim_time, 1 / this->rtk_update_rate);
  
  // Process menus
  RtkMenuHandling();      
  
  // Update the object tree
  root->RtkUpdate();

  // Render the canvas
  rtk_canvas_render(this->canvas);

  return;
}


// Update the GUI
void RtkMenuHandling()
{
  char filename[128];
    
  // See if we need to quit the program
  if (rtk_menuitem_isactivated(this->exit_menuitem))
    ::quit = 1;
  if (rtk_canvas_isclosed(this->canvas))
    ::quit = 1;

  // Save the world file
  if (rtk_menuitem_isactivated(this->save_menuitem))
    Save();
  
  // Start/stop export (jpeg)
  if (rtk_menuitem_isactivated(this->stills_jpeg_menuitem))
  {
    if (rtk_menuitem_ischecked(this->stills_jpeg_menuitem))
    {
      this->stills_series++;
      rtk_menuitem_enable(this->stills_ppm_menuitem, 0);
    }
    else
      rtk_menuitem_enable(this->stills_ppm_menuitem, 1);      
  }
  if (rtk_menuitem_ischecked(this->stills_jpeg_menuitem))
  {
    snprintf(filename, sizeof(filename), "stage-%03d-%04d.jpg",
             this->stills_series, this->stills_count++);
    printf("saving [%s]\n", filename);
    rtk_canvas_export_image(this->canvas, filename, RTK_IMAGE_FORMAT_JPEG);
  }

  // Start/stop export (ppm)
  if (rtk_menuitem_isactivated(this->stills_ppm_menuitem))
  {
    if (rtk_menuitem_ischecked(this->stills_ppm_menuitem))
    {
      this->stills_series++;
      rtk_menuitem_enable(this->stills_jpeg_menuitem, 0);
    }
    else
      rtk_menuitem_enable(this->stills_jpeg_menuitem, 1);      
  }
  if (rtk_menuitem_ischecked(this->stills_ppm_menuitem))
  {
    snprintf(filename, sizeof(filename), "stage-%03d-%04d.ppm",
             this->stills_series, this->stills_count++);
    printf("saving [%s]\n", filename);
    rtk_canvas_export_image(this->canvas, filename, RTK_IMAGE_FORMAT_PPM);
  }

  // Update movie menu
  this->RtkUpdateMovieMenu();
  
  // Show or hide the grid
  if (rtk_menuitem_ischecked(this->grid_item))
    rtk_fig_show(this->fig_grid, 1);
  else
    rtk_fig_show(this->fig_grid, 0);

  // clear any matrix rendering, then redraw if the emnu item is checked
  this->matrix->unrender();
  if (rtk_menuitem_ischecked(this->matrix_item))
    this->matrix->render( this );
  
  // enable/disable subscriptions to show sensor data
  static bool lasttime = rtk_menuitem_ischecked(this->subscribedonly_item);
  bool thistime = rtk_menuitem_ischecked(this->subscribedonly_item);

  // for now i check if the menu item changed
  if( thistime != lasttime )
  {
    if( thistime )  // change the subscription counts of any player-capable ent
      root->FamilySubscribe();
    else
      root->FamilyUnsubscribe();
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
  
  for (i = 0; i < this->movie_option_count; i++)
  {
    option = this->movie_options + i;
    
    if (rtk_menuitem_isactivated(option->menuitem))
    {
      if (rtk_menuitem_ischecked(option->menuitem))
      {
        // Start the movie capture
        snprintf(filename, sizeof(filename), "stage-%03d-sp%02d.mpg",
                 this->movie_count++, option->speed);
        rtk_canvas_movie_start(this->canvas, filename,
                               this->rtk_update_rate, option->speed);

        // Disable all other capture options
        for (j = 0; j < this->movie_option_count; j++)
          rtk_menuitem_enable(this->movie_options[j].menuitem, i == j);
      }
      else
      {
        // Stop movie capture
        rtk_canvas_movie_stop(this->canvas);

        // Enable all capture options
        for (j = 0; j < this->movie_option_count; j++)
          rtk_menuitem_enable(this->movie_options[j].menuitem, 1);
      }
    }

    // Export the frame
    if (rtk_menuitem_ischecked(option->menuitem))
      rtk_canvas_movie_frame(this->canvas);
  }
  return;
}

// this functionality was previously in CEntity::SetProperty - this is
// a lot cleaner, though I didn't want to shoehorn it into the world
// class so it's sitting out here as a regular function. - rtv
void RtkEntityPropertyChange( CEntity* ent, EntityProperty prop )
{
  assert( ent );
  assert( prop > 0 );
  assert( prop < ENTITY_LAST_PROPERTY );
  
  if( !ent->fig )
    return; 
  
  double px, py, pa;

  switch( prop )
    {
      // these require a complete redraw
    case PropSizeX:
    case PropSizeY:
    case PropOriginX:
    case PropOriginY:
    case PropName:
    case PropColor:
    case PropShape:
      ent->RtkShutdown();
      ent->RtkStartup();
      break;
      
      // these require just moving the figure
    case PropPoseX:
    case PropPoseY:
    case PropPoseTh:
      ent->GetPose( px, py, pa );
      rtk_fig_origin(ent->fig, px, py, pa );
      break;

    default:
      break;
    }  
}


#endif

