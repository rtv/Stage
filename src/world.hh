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
 * CVS info: $Id: world.hh,v 1.33 2003-10-16 02:05:14 rtv Exp $
 */

#ifndef _WORLD_HH
#define _WORLD_HH

#include <glib.h>
#include "stage.h"
#include "rtkgui.hh"
class CMatrix;

typedef struct
{
  pid_t pid;
  guint source_in;
  guint source_hup;
  GIOChannel *channel;
  GList *worlds; // list of the worlds created by this client
  GList *subs;
} stg_client_data_t;

typedef struct stg_world
{
  int id;
  bool running;
  GList *models;
  GString *name, *token;  
  CMatrix* matrix;
  double time; // simulation time in seconds
  double interval; // interval between clock updates in seconds
  guint clock_tag;
  double width, height;
  double ppm; // the resolution of the world model in pixels per meter
  stg_gui_window_t* win;   // each world has a GUI window of it's own
  stg_client_data_t* client; // the client that created this world
} stg_world_t;


stg_world_t* stg_world_create( stg_client_data_t* client,
			       stg_id_t id, 
			       stg_world_create_t* rc );

int stg_world_destroy( stg_world_t* world );
CMatrix* stg_world_create_matrix( stg_world_t* world );
void stg_world_destroy_matrix( stg_world_t* world );
void stg_world_update( stg_world_t* world );
void stg_world_save( stg_world_t* world );

#ifdef DEBUG
#define WORLD_DEBUG(W,S) printf("[%d %s %p] "S" (%s %s)\n",W->id,W->name->str,W,__FILE__,__FUNCTION__);
#define WORLD_DEBUG1(W,S,A) printf("[%d %s %p] "S" (%s %s)\n",W->id,W->name->str,W,A,__FILE__,__FUNCTION__);
#define WORLD_DEBUG2(W,S,A,B) printf("[%d %s %p] "S" (%s %s)\n",W->id,W->name->str,W,A,B,__FILE__,__FUNCTION__);
#define WORLD_DEBUG3(W,S,A,B,C) printf("[%d %s %p] "S" (%s %s)\n",W->id,W->name->str,W,A,B,C,__FILE__,__FUNCTION__);
#else
#define WORLD_DEBUG(W,S)
#define WORLD_DEBUG1(W,S,A)
#define WORLD_DEBUG2(W,S,A,B)
#define WORLD_DEBUG3(W,S,A,B,C)
#endif


#endif

