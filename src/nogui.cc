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
 * Desc: dummy GUI hooks, when no GUI is linked in
 * Author: Brian Gerkey
 * Date: 30 Oct 2002
 * CVS info: $Id: nogui.cc,v 1.1.6.2 2003-02-04 03:35:38 rtv Exp $
 */


#include "entity.hh"
#include "gui.hh"

#if !USE_GNOME2 && !INCLUDE_RTK2

void GuiInit( int argc, char** argv ) {}
void GuiLoad( CEntity* ent ) {}
void GuiSave( CEntity* ent ) {}
void GuiEntityStartup( CEntity* ent ) {}
void GuiEntityShutdown( CEntity* ent ) {}
void GuiEntityUpdate( CEntity* ent ) {}
void GuiEntityPropertyChange( CEntity* ent, stage_prop_id_t prop ) {}

#endif
