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
 * CVS info: $Id: nogui.cc,v 1.1.4.1 2003-05-24 22:33:13 gerkey Exp $
 */


#include "world.hh"
#include "entity.hh"
#include "gui.hh"

#if !INCLUDE_RTK2

void GuiInit( int argc, char** argv ) {}
void GuiWorldStartup( CWorld* world ) {}
void GuiWorldShutdown( CWorld* world ) {}
void GuiLoad( CWorld* world ) {}
void GuiSave( CWorld* world ) {}
void GuiWorldUpdate( CWorld* world ) {}
void GuiEntityStartup( CEntity* ent ) {}
void GuiEntityShutdown( CEntity* ent ) {}
void GuiEntityUpdate( CEntity* ent ) {}
void GuiEntityPropertyChange( CEntity* ent, EntityProperty prop ) {}

#endif
