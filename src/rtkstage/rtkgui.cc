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
 * Desc: top level class that contains everything
 * Author: Richard Vaughan, Andrew Howard
 * Date: 7 Dec 2000
 * CVS info: $Id: rtkgui.cc,v 1.2 2002-10-30 01:45:39 gerkey Exp $
 */


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

// TODO - unwrap RTK code from around the place and have it work
// through the GUI hooks

// WORLD HOOKS /////////////////////////////
void GuiInit( int argc, char** argv )
{ 
  rtk_init(&argc, &argv);
}

void GuiWorldStartup( CWorld* world )
{ 
  world->RtkStartup();
}

void GuiWorldShutdown( CWorld* world )
{ 
  /* do nothing */ 
}

void GuiWorldUpdate( CWorld* world )
{ 
  world->RtkUpdate();

  // allow gtk to do some work
  while( gtk_events_pending () )
    gtk_main_iteration();      
}


void GuiLoad( CWorld* world )
{
  world->RtkLoad( world->worldfile );
}

void GuiSave( CWorld* world )
{
  world->RtkSave( world->worldfile );
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
  /* do nothing */ 
}

////////////////////////////////////////////

void CWorld::AddToMenu( stage_menu_t* menu, CEntity* ent, int check )
{
  assert( menu );
  assert( ent );

  // if there's no menu item for this type yet
  if( menu->items[ ent->stage_type ] == NULL )
    // create a new menu item
    assert( menu->items[ ent->stage_type ] =  
	    rtk_menuitem_create( menu->menu, 
				 this->lib->
				 TokenFromType(ent->stage_type),1));
  
  
  rtk_menuitem_check( menu->items[ ent->stage_type ], check );
}

void CWorld::AddToDataMenu(  CEntity* ent, int check )
{
  assert( ent );
  AddToMenu( &this->data_menu, ent, check );
}

void CWorld::AddToDeviceMenu(  CEntity* ent, int check )
{
  assert( ent );
  AddToMenu( &this->device_menu, ent, check );
}

  // devices check this to see if they should display their data
bool CWorld::ShowDeviceData( StageType devtype )
{ 
  rtk_menuitem_t* menu_item = data_menu.items[ devtype ];
  
  if( menu_item )
    return( rtk_menuitem_ischecked( menu_item ) );  
  else // if there's no option in the menu, display this data
    return true;
}

bool CWorld::ShowDeviceBody( StageType devtype )
{
  rtk_menuitem_t* menu_item = device_menu.items[ devtype ];
  
  if( menu_item )
    return( rtk_menuitem_ischecked( menu_item ) );  
  else // if there's no option in the menu, display this data
    return true;    
}


// Initialise the GUI
// TODO: fix this for client/server operation.
bool CWorld::RtkLoad(CWorldFile *worldfile)
{
  PRINT_DEBUG( "**********************************" );

  int sx, sy;
  double scale = 0.01;
  double dx, dy;
  double ox, oy;
  double gridx, gridy;
  double minor, major;
  bool showgrid;
  bool subscribedonly;

  if (worldfile != NULL)
    {
      int section = worldfile->LookupEntity("gui");

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

      gridx = ceil(gridx / major) * major;
      gridy = ceil(gridy / major) * major;
    }
  else
    {
      // Size of world in pixels
      sx = (int) this->matrix->width;
      sy = (int) this->matrix->height;

      // Grid size in meters
      gridx = sx / this->ppm;
      gridy = sy / this->ppm;

      // Place a hard limit, just to stop it going off the screen
      if (sx > 1024)
	sx = 1024;
      if (sy > 768)
	sy = 768;
    
      // Size in meters
      dx = sx * scale;
      dy = sy * scale;

      // Origin of the canvas
      ox = dx / 2;
      oy = dy / 2;


      // Grid spacing
      minor = 0.2;
      major = 1.0;
      showgrid = true;

      // default
      subscribedonly = true;

      gridx = ceil(gridx / major) * major;
      gridy = ceil(gridy / major) * major;
    }
  
  this->app = rtk_app_create();
  rtk_app_refresh_rate(this->app, 10);
  
  this->canvas = rtk_canvas_create(this->app);
  rtk_canvas_size(this->canvas, sx, sy);
  rtk_canvas_scale(this->canvas, scale, scale);
  rtk_canvas_origin(this->canvas, ox, oy);

  // Add some menu items
  this->file_menu = rtk_menu_create(this->canvas, "File");
  this->save_menuitem = rtk_menuitem_create(this->file_menu, "Save", 0);
  this->export_menuitem = rtk_menuitem_create(this->file_menu, "Export", 0);
  this->exit_menuitem = rtk_menuitem_create(this->file_menu, "Exit", 0);
  this->export_count = 0;

  // Create the view menu
  this->view_menu = rtk_menu_create(this->canvas, "View");

  // create the view menu items and set their initial checked state
  this->grid_item = rtk_menuitem_create(this->view_menu, "Grid", 1);
  rtk_menuitem_check(this->grid_item, showgrid);

  this->matrix_item = rtk_menuitem_create(this->view_menu, "Matrix", 1);
  rtk_menuitem_check(this->matrix_item, 0);

  // create the action menu
  this->action_menu = rtk_menu_create(this->canvas, "Action");
  this->subscribedonly_item = rtk_menuitem_create(this->action_menu, 
						  "Subscribe to all", 1);

  rtk_menuitem_check(this->subscribedonly_item, subscribedonly);

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
bool CWorld::RtkSave(CWorldFile *worldfile)
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
bool CWorld::RtkStartup()
{
  PRINT_DEBUG( "** STARTUP GUI **" );

  // don't call this
  //rtk_app_start(this->app);

  // create the main objects here
  rtk_canvas_t *canvas;
  rtk_table_t *table;

  // this rtkstarts all entities
  root->RtkStartup();

  // Display everything
  for (canvas = app->canvas; canvas != NULL; canvas = canvas->next)
    gtk_widget_show_all(canvas->frame);
  for (table = app->table; table != NULL; table = table->next)
    gtk_widget_show_all(table->frame);


  return true;
}


// Stop the GUI
void CWorld::RtkShutdown()
{
  rtk_app_stop(this->app);
}


// Update the GUI
void CWorld::RtkUpdate()
{
  RtkMenuHandling();      

  // this is from rtk_on_app_timer
  rtk_canvas_t *canvas;
  rtk_table_t *table;
  
  // Quit the app if we have been told we should
  // We first destroy in windows that are still open.
  if (app->must_quit)
    {
      for (canvas = app->canvas; canvas != NULL; canvas = canvas->next)
	if (!canvas->destroyed)
	  gtk_widget_destroy(canvas->frame);
      for (table = app->table; table != NULL; table = table->next)
	if (!table->destroyed)
	  gtk_widget_destroy(table->frame);
      gtk_main_quit();
    }
  
  // update the object tree
  root->RtkUpdate();
  
  
  // Update the display
  for (canvas = app->canvas; canvas != NULL; canvas = canvas->next)
    rtk_canvas_render(canvas);
    //rtk_canvas_render(canvas, FALSE, NULL);
  
  //struct timeval tv;
  //gettimeofday( &tv, NULL );
  //double start = tv.tv_sec + tv.tv_usec / 1000000.0;
  
  // allow gtk to do some work
  while( gtk_events_pending () )
    gtk_main_iteration ();
  
  //gettimeofday( &tv, NULL );

   //double duration = (tv.tv_sec + tv.tv_usec / 1000000.0) - start;

  //printf( "gtkloop: %.4f\n", duration );
}

// Update the GUI
void CWorld::RtkMenuHandling()
{
  // right now we have to check the state of all the menu items each
  // time around the loop. this is pretty unsatisfactory - callbacks
  // would be much better

  // See if we need to quit the program
  if (rtk_menuitem_isactivated(this->exit_menuitem))
    ::quit = 1;
  if (rtk_canvas_isclosed(this->canvas))
    ::quit = 1;

  // Save the world file
  if (rtk_menuitem_isactivated(this->save_menuitem))
    Save();

  // Handle export menu item
  // TODO - fold in XS's postscript and pnm export here or in rtk2
  if (rtk_menuitem_isactivated(this->export_menuitem))
  {
    char filename[128];
    snprintf(filename, sizeof(filename), 
             "rtkstage-%04d.fig", this->export_count++);
    PRINT_MSG1("exporting canvas to [%s]", filename);
    rtk_canvas_export_ppm(this->canvas, filename);
  }

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
      
  
}
#endif

