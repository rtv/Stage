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
 * Desc: a container for models 
 * Author: Richard Vaughan
 * Date: 24 July 2003
 * CVS info: $Id: world.hh,v 1.22.6.3 2003-08-09 01:30:16 rtv Exp $
 */

#ifndef _WORLD_HH
#define _WORLD_HH

#include <glib.h>
#include "stage.h"
#include "rtkgui.hh"
class CMatrix;

class CWorld
{
public: 

  int id;
  bool running;
  GNode* node;  
  GString *name, *token;

  CMatrix* matrix;
  
  GIOChannel* channel;
  double width, height;
  
  CWorld( GIOChannel* channel, stg_world_create_t* rc );
  ~CWorld( void );
  
  double ppm; // the resolution of the world model in pixels per meter
  
  double simtime; // the simulation time in seconds
  //double timestep; // the duration of one update in seconds
  
  // Initialise world  
  int Startup( void ); 
  // Finalize world
  int Shutdown();  

  // each world has a GUI window of it's own
  stg_gui_window_t* win;
  
private: 
  int CreateMatrix( void );
};


/*  

//I'd prefer to move all this to C one of these days - like this:
typedef struct
{
  int id;
  bool running;
  GNode* node;  
  GString *name, *token;  
  CMatrix* matrix;
  GIOChannel* channel;
  double width, height;
  double ppm; // the resolution of the world model in pixels per meter
  double simtime; // the simulation time in seconds
  //double timestep; // the duration of one update in seconds
  stg_gui_window_t* win;   // each world has a GUI window of it's own
  } stg_world_t;
  
  
  stg_world_t* stg_world_create( GIOChannel* channel, stg_world_create_t* rc );
  int stg_world_destroy( stg_world_t* world );
  int stg_world_startup( stg_world_t* world );
  int stg_world_shutdown( stg_world_t* world );
  
  CMatrix* stg_create_matrix( stg_world_t* world );
*/

#endif

