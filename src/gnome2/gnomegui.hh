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
 * Desc: Gnome GUI header
 * Author: Richard Vaughan
 * Date: 20 September 2002
 * CVS info: $Id: gnomegui.hh,v 1.3 2003-01-10 03:45:38 rtv Exp $
 */

#if HAVE_CONFIG_H
  #include <config.h>
#endif
#if HAVE_STRINGS_H
  #include <strings.h>
#endif

#ifdef USE_GNOME2

#ifndef GNOMEGUI_HH
#define GNOMEGUI_HH

#include "world.hh"
#include "entity.hh"
#include "playerdevice.hh"
#include "stage_types.hh"

#include <gnome.h>

// need the device types for the render function args
#include "laserdevice.hh"
#include "sonardevice.hh"
#include "visiondevice.hh"

// handy macros ////////////////////////////////////////////////////////

//  compose a 4-byte red/green/blue/alpha color from
// a normal 3-byte color and a 1-byte alpha value
#define RGBA(C,A) (C<<8)+A

enum MotionMode 
{
  Nothing = 0, Dragging, Spinning, Zooming
};

// one of these structures is allocatd for each entity, and accessed through
// the CEntity::gui_data member
typedef struct _gui_entity_data
{
  GnomeCanvasGroup* origin; // sets our location on the canvas
  GnomeCanvasGroup* group; // inside g_origin - contains our body figure g_fig
  GnomeCanvasItem* data; // inside g_group - contains our data figures if any
  GnomeCanvasItem* body; // inside g_group - contains our body figures if any
  bool click_subscribed;
} gui_entity_data;
 
// this sets the type of the gui data pointer in CEntity.hh
//#define ENTITY_GUI_DATA_TYPE gui_entity_data


// FUNCTIONS ////////////////////////////////////////////////////////////

// get the entity's generic gui_data pointer cast to the right type.
//gui_entity_data* GetData( CEntity* ent )
//{ return( (gui_entity_data*)(ent->gui_data) ); }

GnomeCanvasGroup* GetOrigin( CEntity* ent );
GnomeCanvasGroup* GetGroup( CEntity* ent );
GnomeCanvasItem* GetData( CEntity* ent );
GnomeCanvasItem* GetBody( CEntity* ent );
bool GetClickSub( CEntity* ent );
void SetOrigin( CEntity* ent, GnomeCanvasGroup* origin );
void SetGroup( CEntity* ent, GnomeCanvasGroup* group );
void SetData( CEntity* ent, GnomeCanvasItem* data );
void SetBody( CEntity* ent, GnomeCanvasItem* body );
void SetClickSub( CEntity* ent, bool cs );

// system init
void GnomeInit( int argc, char **argv );
 
// startup
void GnomeWorldStartup( CWorld* world  );
void GnomeEntityStartup( CEntity* entity );

// no shutdown code yet


// Update the value of the progress bar 
gboolean GnomeProgressTimeout( gpointer data );

// event-handling callbacks

// for mouse on the canvas
gint GnomeEventCanvas(GnomeCanvasItem *item, GdkEvent *event, gpointer data);
gint GnomeEventRoot(GnomeCanvasItem *item, GdkEvent *event, gpointer data);
gint GnomeEventBody(GnomeCanvasItem *item, GdkEvent *event, gpointer data);

// for toolbar buttons
void GnomeZoomCallback( GtkSpinButton *spinner, gpointer world );
void GnomeAboutBox(GtkWidget *widget, gpointer data);


gint GnomeEventSelection(GnomeCanvasItem *item, GdkEvent *event, gpointer data);
void GnomeRenderGrid( CWorld* world, double spacing, StageColor color );
void GnomeSelectionDull( void );
void GnomeSelectionBright( void );
void GnomeSelect( CEntity* ent );
void GnomeUnselect( CEntity* ent );
void GnomeStatus( CEntity* ent );
void GnomeEntityPropertyChange( CEntity* ent, EntityProperty prop );
void GnomeEntityRenderData( CPlayerEntity* ent );
void GnomeEntityMove( CEntity* ent );
void GnomeSubscribeToAll(GtkWidget *widget, gpointer data);

// check the device for subscriptions. if none, destroy any existing
// data figure
void GnomeCheckSub( CEntity* ent );

// device data renderers
void GnomeEntityRenderDataLaser( CLaserDevice* ent );
void GnomeEntityRenderDataSonar( CSonarDevice* ent );
void GnomeEntityRenderDataBlobfinder( CVisionDevice* ent );

#endif // GNOMEGUI_HH
#endif // USE_GNOME2



