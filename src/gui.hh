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
 * CVS info: $Id: gui.hh,v 1.4.2.2 2003-02-04 03:35:38 rtv Exp $
 */

// GUI hooks - the Stage GUI implements these functions called by Stage
//
// This is a new model and RTK does not yet follow it
// see the gnome2 gui for a working example - rtv

#ifndef STAGE_GUI_HH
#define STAGE_GUI_HH


// called from  CStageServer::CStageServer()
void GuiInit( int argc, char** argv );

// called from  CStageServer::CStageServer()
void GuiUpdate( void );

// called from CWorld::Load()
void GuiLoad( CEntity* ent );

// called from CWorld::Save();
void GuiSave( CEntity* ent );

// called from CEntity::Startup()
void GuiEntityStartup( CEntity* ent );

// called from CEntity::Shutdown()
void GuiEntityShutdown( CEntity* ent );

// called from CEntity::Update()
void GuiEntityUpdate( CEntity* ent );

// called from CEntity::SetProperty()
void GuiEntityPropertyChange( CEntity* ent, stage_prop_id_t prop );

#endif //STAGE_GUI_HH







