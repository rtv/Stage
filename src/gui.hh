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
 * Desc: GUI interface definitions.
 * Author: Richard Vaughan
 * Date: 6 Oct 2002
 * CVS info: $Id: gui.hh,v 1.4.2.3 2003-02-05 03:59:49 rtv Exp $
 */

// GUI hooks - the Stage GUI implements these functions called by Stage
//
// This is a new model and RTK does not yet follow it
// see the gnome2 gui for a working example - rtv

#ifndef STAGE_GUI_HH
#define STAGE_GUI_HH

#include "stage.h"

typedef struct
{
  char token[ STG_TOKEN_MAX ]; // string identifying the GUI library
  // a bunch of function pointers
  int (*init_func)(int argc, char** argv);
  int (*load_func)( stage_gui_config_t* cfg );
  int (*update_func)(void);
  int (*createmodel_func)( CEntity* ent );
  int (*destroymodel_func)( CEntity* ent );
  int (*tweakmodel_func)( CEntity* ent, stage_prop_id_t prop );
  // save_func
  // prop_func
  /// etc.
} stage_gui_library_item_t;



void GuiLoad( stage_gui_config_t* cfg );
void GuiSave( stage_gui_config_t* cfg );

int GuiUpdate( void );

int GuiInit( int argc, char** argv );

int GuiEntityStartup( CEntity* ent );
int GuiEntityShutdown( CEntity* ent );
int GuiEntityUpdate( CEntity* ent );
int GuiEntityPropertyChange( CEntity* ent, stage_prop_id_t prop );


#endif //STAGE_GUI_HH







