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
 * Desc: Gnome GUI 
 * Author: Richard Vaughan
 * Date: 7 Dec 2000
 * CVS info: $Id: gnomegui.cc,v 1.3 2002-10-25 22:48:09 rtv Exp $
 */


#if HAVE_CONFIG_H
  #include <config.h>
#endif
#if HAVE_STRINGS_H
  #include <strings.h>
#endif

#ifdef USE_GNOME2

#undef DEBUG
//#undef VERBOSE
//#define DEBUG 
//#define VERBOSE

#include "world.hh"
#include "gnomegui.hh"
#include "playerdevice.hh"
#include "bitmap.hh"
#include "worldfile.hh"
#include <gnome.h>

// include the logo XPM
#include "stage.xpm"



void StageQuit( void ); // declare quit func

// IMPLEMENT THE INTERFACE HOOKS
// MAP THE STANDARD HOOKS ONTO GNOME SPECIFIC FUNCTIONS

void GuiInit( int argc, char** argv )
{ 
  GnomeInit( argc, argv );
}

//void GuiEnterMainLoop( 

// WORLD HOOKS /////////////////////////////
void GuiWorldStartup( CWorld* world )
{ 
  GnomeWorldStartup( world );
}

void GuiWorldShutdown( CWorld* world )
{ 
  /* do nothing */ 
}

void GuiWorldUpdate( CWorld* world )
{ 
  // allow gtk to do some work
  while( gtk_events_pending () )
    gtk_main_iteration();      
}

// ENTITY HOOKS ///////////////////////////
void GuiEntityStartup( CEntity* ent )
{ 
  GnomeEntityStartup( ent ); 
}

void GuiEntityShutdown( CEntity* ent )
{ 
  /* do nothing */ 
}

void GuiLoad( CWorld* world )
{ 
  /* do nothing */ 
}

void GuiSave( CWorld* world )
{ 
  /* do nothing */ 
}

void GuiEntityUpdate( CEntity* ent )
{
  GnomeCheckSub( ent ); // check to see if we're still subscribed
}

void GuiEntityPropertyChange( CEntity* ent, EntityProperty prop )
{ 
  GnomeEntityPropertyChange( ent, prop );
}


// GUI CONSTANTS /////////////////////////////////////////////////////////////////////////////

const char* stage_name = "Stage";
const char* stage_authors[] = {"Richard Vaughan <vaughan@hrl.com>", 
			       "Andrew Howard <ahoward@usc.edu>", 
			       "Brian Gerkey <gerkey@usc.edu>", 
			       "",
			       "Supported by:",
			       "\tDARPA IPTO",
			       "\tThe University of Southern California",
			       "\tHRL Laboratories LLC", 
			       NULL };

const char* stage_copyright = "Copyright Player/Stage Project, 2000-2002";
const char** stage_documenters = NULL;
const char* stage_translators = NULL;
const char* stage_comments = 
"A robot device simulator\n\nhttp://playerstage.sourceforge.net\n\n\"\"All the World's a stage,\nand all the men and women merely players\"\n (Shakespeare - As You Like It) ";

// GUI GLOBALS ////////////////////////////////////////////////////////////////////////////

CEntity* g_watched = NULL; // the user-selected entity
bool  g_click_respond = false; // controls whether the g_watched entity responds to mouse events

GnomeCanvasGroup* g_select_rect = NULL; // group for the selection image
GnomeCanvasItem* g_select_ellipse = NULL; // visually indicates the selected entity
GtkProgressBar* g_pbar = NULL; // clock and progress indicator widget

GnomeApp* g_app; // application
GnomeCanvas* g_canvas; //main drawing window
GtkMenuBar* g_menubar; // menu bar
GnomeAppBar* g_appbar; // status bar
GtkToolbar* g_tbar; // toolbar

// MENU DEFINITIONS ///////////////////////////////////////////////////////////////////////

// calback functions
#define new_app_cb NULL
#define open_cd NULL
#define save_as_cb NULL
//#define close_cb NULL
#define exit_cb StageQuit
#define open_cb NULL
#define about_cb GnomeAboutBox

// menus
static GnomeUIInfo file_menu[] = {
  GNOMEUIINFO_MENU_NEW_ITEM(N_("_New Window"),
                            N_("Create a new text viewer window"), 
                            new_app_cb, NULL),
  GNOMEUIINFO_MENU_OPEN_ITEM(open_cb,NULL),
  GNOMEUIINFO_MENU_SAVE_AS_ITEM(save_as_cb,NULL),
  GNOMEUIINFO_SEPARATOR,
  //  GNOMEUIINFO_MENU_CLOSE_ITEM(close_cb,NULL),
  GNOMEUIINFO_MENU_EXIT_ITEM(exit_cb,NULL),
  GNOMEUIINFO_END
};

static GnomeUIInfo help_menu [] = {                     
  /* load the helpfiles for this application if available */              
  GNOMEUIINFO_HELP("Stage"),                
  /* the standard about item */           
  GNOMEUIINFO_MENU_ABOUT_ITEM(about_cb,NULL),          
  GNOMEUIINFO_END         
};                      

static GnomeUIInfo view_menu [] = {                     
  /* load the helpfiles for this application if available */              
  GNOMEUIINFO_HELP("Stage"),                
  /* the standard about item */           
  GNOMEUIINFO_MENU_ABOUT_ITEM(about_cb,NULL),          
  GNOMEUIINFO_END         
};                      

static GnomeUIInfo main_menu[] = {
  GNOMEUIINFO_MENU_FILE_TREE(file_menu),
  GNOMEUIINFO_MENU_HELP_TREE(help_menu),
  GNOMEUIINFO_END
};


static GnomeUIInfo toolbar [] = {
  //GNOMEUIINFO_ITEM_STOCK("Run", "Run the simulation",
  //		      NULL, GNOME_STOCK_PIXMAP_FORWARD),
	GNOMEUIINFO_ITEM_STOCK("Stop", "Pause the simulation",
			      NULL, GNOME_STOCK_PIXMAP_STOP),
	//	GNOMEUIINFO_ITEM_STOCK("Zoom In", "Zoom In",
	//	      NULL, GNOME_STOCK_ZOOM_IN),
	//GNOMEUIINFO_ITEM_STOCK("Zoom Out", "Zoom Out",
	//	      NULL, GNOME_STOCK_ZOOM_OUT),
	//GNOMEUIINFO_ITEM_STOCK("Zoom 1:1", "Zoom 1;1",
	//	      NULL, GNOME_STOCK_ZOOM_100),
	GNOMEUIINFO_END
};


// utility functions for accessing entity data ////////////////////////////////////
 
GnomeCanvasGroup* GetOrigin( CEntity* ent )
{
  return( ((gui_entity_data*)ent->gui_data)->origin );
}

GnomeCanvasGroup* GetGroup( CEntity* ent )
{
  return( ((gui_entity_data*)ent->gui_data)->group );
}

GnomeCanvasItem* GetData( CEntity* ent )
{
  return( ((gui_entity_data*)ent->gui_data)->data );
}

GnomeCanvasItem* GetBody( CEntity* ent )
{
  return( ((gui_entity_data*)ent->gui_data)->body );
}

bool GetClickSub( CEntity* ent )
{
  return( ((gui_entity_data*)ent->gui_data)->click_subscribed );
}

void SetOrigin( CEntity* ent, GnomeCanvasGroup* origin )
{
  ((gui_entity_data*)ent->gui_data)->origin = origin;
}

void SetGroup( CEntity* ent, GnomeCanvasGroup* group )
{
  ((gui_entity_data*)ent->gui_data)->group= group;
}

void SetData( CEntity* ent, GnomeCanvasItem* data )
{
  ((gui_entity_data*)ent->gui_data)->data = data;
}

void SetBody( CEntity* ent, GnomeCanvasItem* body )
{
  ((gui_entity_data*)ent->gui_data)->body = body;
}

void SetClickSub( CEntity* ent, bool cs )
{
  ((gui_entity_data*)ent->gui_data)->click_subscribed = cs;
}


// //////////////////////////////////////////////////////////////////////////////


// HOOK - system init
void GnomeInit( int argc, char** argv )
{
  PRINT_DEBUG( "" );

  /* initialize gnome */          
   gnome_init( "Stage", (char*)VERSION,  argc, argv);
  
  // Allow rgb image handling
  gdk_rgb_init();
  // create the app
  assert( g_app = GNOME_APP(gnome_app_new( "stage", "Stage" )) );
  
  // make user-resizable
  gtk_window_set_policy(GTK_WINDOW(g_app), FALSE, TRUE, FALSE);

  // add menus  
  gnome_app_create_menus( g_app, main_menu );
  
  // setup appbar
  assert( g_appbar = GNOME_APPBAR(gnome_appbar_new(FALSE, TRUE, GNOME_PREFERENCES_USER)) );
  assert( g_appbar );
  gnome_appbar_set_default(  g_appbar, "No selection" );
  gnome_app_set_statusbar(GNOME_APP(g_app), GTK_WIDGET(g_appbar)); 

 // make a toolbar and fill it from the static structure
  assert( g_tbar = GTK_TOOLBAR(gtk_toolbar_new()) );
  gnome_app_fill_toolbar( g_tbar, toolbar, NULL );

  //GtkWidget* tbar = gtk_toolbar_new();
  //gtk_toolbar_set_orientation (GTK_TOOLBAR (tbar), 
  //		       GTK_ORIENTATION_HORIZONTAL);
  // gtk_toolbar_set_style (GTK_TOOLBAR (tbar), GTK_TOOLBAR_BOTH);
  //  gtk_container_set_border_width (GTK_CONTAINER (tbar), 5);
  //  gtk_toolbar_set_space_size (GTK_TOOLBAR (tbar), 5);
//gtk_container_add (GTK_CONTAINER (handlebox), tbar);

  
  assert( g_pbar = GTK_PROGRESS_BAR(gtk_progress_bar_new()) );
  gtk_progress_bar_set_text( GTK_PROGRESS_BAR(g_pbar), "time" );
  gtk_widget_set_size_request( GTK_WIDGET(g_pbar), 75, 30 );

  // create a canvas
  gtk_widget_push_visual(gdk_rgb_get_visual());
  gtk_widget_push_colormap(gdk_rgb_get_cmap());

  // TODO - read which type to use from world file
  //g_canvas = GNOME_CANVAS(gnome_canvas_new()); // Xlib - fast
  g_canvas = GNOME_CANVAS(gnome_canvas_new_aa()); //  anti-aliased, slow

  gtk_widget_pop_colormap();
  gtk_widget_pop_visual();


  // set up the scrolling
  GtkWidget* sw = gtk_scrolled_window_new(NULL, NULL);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw),
                                 GTK_POLICY_AUTOMATIC,
                                 GTK_POLICY_AUTOMATIC);
  
  //GtkWidget*  hruler = gtk_hruler_new();
  //GtkWidget*  vruler = gtk_vruler_new();

  gtk_container_add(GTK_CONTAINER(sw), GTK_WIDGET(g_canvas));
  //gtk_container_add(GTK_CONTAINER(sw), hruler );
  //gtk_container_add(GTK_CONTAINER(sw), vruler );

  // bind quit function to the top-level window
  gtk_signal_connect(GTK_OBJECT(g_app),
                     "delete_event",
                     GTK_SIGNAL_FUNC(StageQuit),
                     NULL);

  
  // set the app's main window to be the scrolled window
  gnome_app_set_contents( GNOME_APP(g_app), sw );

  // install the toolbar
  gnome_app_set_toolbar(GNOME_APP( g_app), GTK_TOOLBAR(g_tbar));


  //GtkWidget* play = 
    //gtk_toggle_button_new_with_label( "play" );
    //gtk_check_button_new_with_label( "Run" ); 
  //gtk_button_new_from_stock(GNOME_STOCK_PIXMAP_STOP);
  //GtkWidget* scaler = gtk_hscale_new_with_range( 0, 100, 5 );
  
  // add menu hints to appbar
  //gnome_app_install_menu_hints(GNOME_APP(this->g_app), main_menumenubar);
}

// HOOK - startup
void GnomeWorldStartup( CWorld* world  )
{ 
  PRINT_DEBUG( "" );
   
  //GtkWidget* spinner_label = gtk_label_new( "  PPM " );
  GtkWidget* spinner = gtk_spin_button_new_with_range  ( 1, 1000, 1 );

  gtk_spin_button_set_value( GTK_SPIN_BUTTON(spinner), world->ppm );

  //gtk_toolbar_append_widget (GTK_TOOLBAR(tbar), play, "zoom", "zoom");
  gtk_toolbar_append_widget( GTK_TOOLBAR(g_tbar), GTK_WIDGET(g_pbar), "zoom", "zoom");
  gtk_toolbar_append_space( GTK_TOOLBAR(g_tbar) );
  //gtk_toolbar_append_widget (GTK_TOOLBAR(tbar), spinner_label, NULL.);
  gtk_toolbar_append_widget (GTK_TOOLBAR(g_tbar), spinner, "pixels per meter", "ppm");
  //gtk_toolbar_append_widget (GTK_TOOLBAR(tbar), scaler, "zoom", "zoom");
  
  //gtk_widget_show (spinner_label);
  gtk_widget_show (spinner);
  gtk_widget_show (GTK_WIDGET(g_pbar));
  //gtk_widget_show (scaler);
   
  // set the starting size - just a quick hack for starters
  int initwidth = world->matrix->get_width() + 40;
  int initheight = world->matrix->get_height() + 100;

  // sensible bounds check
  if( initwidth > gdk_screen_width() ) initwidth = gdk_screen_width();
  if( initheight > gdk_screen_height() ) initheight = gdk_screen_height();
  
  gtk_window_set_default_size(GTK_WINDOW(g_app), initwidth, initheight );

  // connect a signal handler for button events
    
  // set scaling and scroll viewport
  gnome_canvas_set_pixels_per_unit( g_canvas, world->ppm );
  gnome_canvas_set_scroll_region( g_canvas, 
				  0, 0,
				  world->matrix->width / world->ppm,
				  world->matrix->height / world->ppm );

  // NO - entity gui startup happens in the CEntity::Startup
  // this creates the GUI elements in the entity tree
  //world->root->GuiStartup();
 
  // flip the coordinate system, so that y increases upwards
  //GnomeCanvasGroup *root = gnome_canvas_root(GNOME_CANVAS(canvas)); 
  //double flip[6] = { 1, 0, 0, -1, 0, 0 }; 
  //gnome_canvas_item_affine_relative(GNOME_CANVAS_ITEM(root), flip); 
  
  
  // attach a callback to the zoom widget
  gtk_signal_connect(GTK_OBJECT(spinner), "value-changed",
		     (GtkSignalFunc) GnomeZoomCallback, world );
  
  // attach a callback for events to the root group in the canvas
  gtk_signal_connect(GTK_OBJECT(gnome_canvas_root(g_canvas)), "event",
		     (GtkSignalFunc) GnomeEventCanvas, world );


  // start a callback to display the simultion time in the status bar
  /* Add a timer callback to update the value of the progress bar */
  gtk_timeout_add (100, GnomeProgressTimeout, world );

  // check every now and again for dead subscription
  //gtk_timeout_add (1000, CWorld::GuiProgressTimeout, this );
  gtk_widget_show_all(GTK_WIDGET(g_app));

}


void GnomeZoomCallback( GtkSpinButton *spinner, gpointer world )
{
  assert( spinner );
  double ppm = gtk_spin_button_get_value( spinner );     
  gnome_canvas_set_pixels_per_unit( g_canvas, ppm );//((CWorld*)world)->ppm );
}


/* Update the clock in the progress bar, and check for dead subscriptions */
gboolean GnomeProgressTimeout( gpointer data )
{
  CWorld* world = (CWorld*)data;

  // set the text to be the current time
  char seconds_buf[128];
  snprintf( seconds_buf, 128, "%.2f", world->GetTime() );
  gtk_progress_bar_set_text( g_pbar, seconds_buf );

  
  // if stage is counting down to a stop time, show the fraction of time used up
  if( world->GetStopTime() )
    gtk_progress_bar_set_fraction( g_pbar, world->GetTime() / world->GetStopTime() );
  
    /* As this is a timeout function, return TRUE so that it
   * continues to get called */
  return TRUE;
} 

gint GnomeEventCanvas(GnomeCanvasItem *item, GdkEvent *event, gpointer data)
{
  //PRINT_DEBUG("");

  assert( item );
  assert( data );

  GdkCursor *cursor = NULL; // change cursor to indicate context
  CWorld* world = (CWorld*)data; // cast the data ptr for convenience
 
  static MotionMode mode = Nothing;
  
  static double drag_offset_x;
  static double drag_offset_y;

  static double spin_start_th;
  static double spin_start_x;
  //static double zoom = 1.0;
  static double last_x = 0.0;

  double item_x, item_y; // pointer pos in local coords
  double win_x = 0, win_y = 0; // pointer pos in window coords


  item_x = event->button.x;
  item_y = event->button.y;


  gnome_canvas_item_w2i(GNOME_CANVAS_ITEM( GetOrigin(world->root)), 
			&item_x, &item_y);
  
  gnome_canvas_world_to_window( g_canvas, event->button.x, event->button.y,
				&win_x, &win_y );
  

    //printf( "EVENT g_watched = %p\n", g_watched );

  if( g_watched )
  switch (event->type) 
    {
    case GDK_BUTTON_PRESS: // begin a drag or spin
      if( !g_click_respond ) // we're out of range of the selected entity
	{
	  // our click deselects the entity
	  GnomeUnselect( g_watched );
	  g_watched = NULL;
	}
      else // we're manipulating the selected entity
	{
	  double dummy;

	  switch(event->button.button) 
	    {
	    case 1: // start dragging
	      //puts( "START DRAG" );
	      cursor = gdk_cursor_new(GDK_FLEUR);
	      mode = Dragging;
	      
	      g_watched->GetPose( drag_offset_x, drag_offset_y, dummy );
	      
	      // subtract the cursor position to get the offsets
	      drag_offset_x -= item_x;
	      drag_offset_y -= item_y;
	      break;
	      
	    case 3: // start spinning
	      //puts( "START SPIN" );
	      cursor = gdk_cursor_new(GDK_EXCHANGE);
	      mode = Spinning;
	     
	      g_watched->GetPose( dummy, dummy, spin_start_th );
	      spin_start_x = win_x;
	      break;
	      
	    case 2: 
	      //puts( "TOGGLE VIEW DATA" );
	      
	      GetClickSub(g_watched) ? 
		g_watched->FamilyUnsubscribe() : g_watched->FamilySubscribe();
	      
	      SetClickSub( g_watched, !GetClickSub(g_watched) ); // invert value	      
	      break;
	      
	    default:
	      break;
	    }
	  
	  if( cursor ) // install the cursor for the job
	    {
	      gnome_canvas_item_grab(item,
				     GDK_POINTER_MOTION_MASK | 
				     //GDK_POINTER_MOTION_HINT_MASK | 
				     GDK_BUTTON_RELEASE_MASK |
				     GDK_BUTTON_PRESS_MASK,
				     cursor,
				     event->button.time);
	      gdk_cursor_destroy(cursor); // once it's set, we don't need this structure
	      cursor = NULL;
	    }     
	}
      break;
      
    case GDK_MOTION_NOTIFY: // perform a drag, spin or zoom
      //printf( "MOTION mode %d\n", mode );
      
      switch( mode )
	{
	case Nothing:
	  //puts( "NOTHING" );
	  break;
	  
	case Dragging:
	  if( g_watched )
	    {		  
	      //puts( "DRAGGING" );
	      // move the entity
	      double newpos_x = item_x + drag_offset_x;
	      double newpos_y = item_y + drag_offset_y;

	      g_watched->SetProperty( -1, PropPoseX, &newpos_x, sizeof(item_x) );
	      g_watched->SetProperty( -1, PropPoseY, &newpos_y, sizeof(item_y) );
	    }
	  break;
	  
	case Spinning:
	  if( g_watched )
	    {
	      //puts( "SPINNING" );
	      // rotate the entity
	      double mouse_delta = win_x - spin_start_x;
	      double angle = spin_start_th - (mouse_delta/20.0);
	      g_watched->SetProperty( -1, PropPoseTh, &angle, sizeof(angle) );
	    }
	  break;
	  
	default:
	  PRINT_ERR1( "invalid mouse mode %d\n", mode );
	  break;
	  
	}
      break;
      
    case GDK_BUTTON_RELEASE: // end a drag, spin or zoom
      if( mode != Nothing )
	{
	  //puts( "RELEASE" );
	  mode = Nothing;
	  gnome_canvas_item_ungrab(item, event->button.time);      
	}
      break;
      
    case GDK_2BUTTON_PRESS: // double-click sub/unsubscribes
      break;

    case GDK_ENTER_NOTIFY:
      //puts( "enter" );
      break;
      
    case GDK_LEAVE_NOTIFY:
      // puts( "leave" );
      break;

    default:
      break;
    }
        
  return TRUE;
}


void GnomeEntityStartup( CEntity* entity )
{ 
  PRINT_DEBUG( "" );

  // allocte storage for gui data, zero it, and stow it in the entity
  gui_entity_data* data = new gui_entity_data();
  memset( data, 0, sizeof(gui_entity_data) );
  entity->gui_data = (void*)data;

  //gui_entity_data* ged = (gui_entity_data*)entity->gui_data;

  // find our parent group - if we're root it's the gnome_canvas_root
  GnomeCanvasGroup* parent_group;
  if( entity->m_parent_entity )
      assert( parent_group = GetGroup( entity->m_parent_entity ) );
  else
    {
      // sanity check - only root should have no parent 
      assert( entity == entity->m_world->root );
      assert( parent_group = gnome_canvas_root( g_canvas ) );
    }

  // set the origin for this entities' coordinate system
  double x, y, th;
  entity->GetPose( x, y, th );
  
  SetOrigin( entity,  
	     (GnomeCanvasGroup*)gnome_canvas_item_new( parent_group,
						       gnome_canvas_group_get_type(),
						       "x", x, 
						       "y", y,
						       NULL)
	     );

  // we rotate this group about the origin
  SetGroup( entity,
	    (GnomeCanvasGroup*)gnome_canvas_item_new( GetOrigin(entity),
						      gnome_canvas_group_get_type(),
						      "x", 0,
						      "y", 0,
						      NULL)
	     );
  
  // specify the affine matrix for the rectangle, to do the rotation
  double affine[6];
  art_affine_rotate( affine, RTOD(th) );
  gnome_canvas_item_affine_relative( GNOME_CANVAS_ITEM(GetGroup(entity)), affine );
	

  if( entity->m_parent_entity == NULL ) // we're root
    {
      // TODO - can this overwrite the above SetBody? that would cause a leak...
      
      // draw a rectangle around the root entity as a background for the world
      SetBody( entity, 
	       gnome_canvas_item_new ( GetGroup(entity),
				       gnome_canvas_rect_get_type(),
				       "x1", 0.0,
				       "y1", 0.0,
				       "x2", entity->m_world->matrix->width / entity->m_world->ppm,
				       "y2", entity->m_world->matrix->width / entity->m_world->ppm,
				       "fill_color_rgba", RGBA(::LookupColor(BACKGROUND_COLOR),255),
				       "outline_color", "black",
				       "width_pixels", 1,
				       NULL )
	       );
      
      gtk_signal_connect(GTK_OBJECT( GetBody(entity) ), "event",
			 (GtkSignalFunc) GnomeEventRoot, entity );
    
      
      GnomeRenderGrid( entity->m_world, 0.2, ::LookupColor(GRID_MINOR_COLOR) );
      GnomeRenderGrid( entity->m_world, 1.0, ::LookupColor(GRID_MAJOR_COLOR) );
    }
  else // we're not root
    {
      switch( entity->stage_type )
	{
	case WallType: // it's a bitmap
	  {
	    PRINT_DEBUG( "making a bitmap gui thing" );

	    GnomeCanvasPoints *gcp = gnome_canvas_points_new(5);
	    
	    PRINT_DEBUG1( "rects: %d\n", ((CBitmap*)entity)->bitmap_rects.size() );

	    // add the figure we pre-computed in CBitmap::Startup()  
	    for(
		std::vector<bitmap_rectangle_t>::iterator it = ((CBitmap*)entity)->bitmap_rects.begin();
		it != ((CBitmap*)entity)->bitmap_rects.end();
		it++ )  
	      {
		// find the rectangle bounding box, as these are
		// originally specced in center,offset style
		gcp->coords[0] = it->x - it->w/2.0;
		gcp->coords[1] = it->y - it->h/2.0;
		gcp->coords[2] = gcp->coords[0] + it->w;
		gcp->coords[3] = gcp->coords[1];
		gcp->coords[4] = gcp->coords[2];
		gcp->coords[5] = gcp->coords[1] + it->h;
		gcp->coords[6] = gcp->coords[0];
		gcp->coords[7] = gcp->coords[5];
		gcp->coords[8] = gcp->coords[0];
		gcp->coords[9] = gcp->coords[1];
		
		
		// add the rectangle   
		/*
		  // use rectangle in aa mode
		  gnome_canvas_item_new (g_group,
		  gnome_canvas_rect_get_type(),
		  "x1", x,
		  "y1", y,
		  "x2", a,
		  "y2", b,
		  // alpha blending is slow...
		  //"fill_color_rgba", (color << 8)+128,
		  //"fill_color_rgba", (color << 8)+255,
		  "fill_color", "black",
		  "outline_color", NULL,
		  "width_pixels", 1,
		  NULL );            
		*/
		
		// use lines in X mode
		gnome_canvas_item_new ( GetGroup(entity),
					gnome_canvas_line_get_type(),
					"points", gcp,
					"fill_color_rgba", (entity->color<<8)+255,
					"width_pixels", 1,//0.001,
					NULL );
	      }
	    
	    gnome_canvas_points_free(gcp);	    
	  }
	  break;
	  
	default:  // most things are just rectangles or ellipses
	  {
	    GtkType shape = 0;
	    
	    // choose a shape
	    switch( entity->shape )
	      {
	      case ShapeRect:
		shape = gnome_canvas_rect_get_type();
		break;
	      case ShapeCircle: 
		shape = gnome_canvas_ellipse_get_type();
		break;
	      case ShapeNone: // draw nothin'.
		shape = 0;
		break;
	      }      
	    
	    if( shape != 0 )
	      SetBody( entity, 
		       gnome_canvas_item_new ( GetGroup(entity), 
					       shape,
					       "x1", - entity->size_x/2.0,
					       "y1", - entity->size_y/2.0,
					       "x2", + entity->size_x/2.0,
					       "y2", + entity->size_y/2.0,
					       "fill_color_rgba", RGBA(entity->color,255),
					       "outline_color_rgba", RGBA(entity->color,255),
					       "width_pixels", 1,
					       NULL ) 
		       );
	  

	  // register an event if this is a lop level thingie
	  // so we can be selected when the mouse enters us
	  // attach a callback for events to the root group in the canvas
	  
	  if( entity->m_parent_entity == entity->m_world->root )
	    gtk_signal_connect(GTK_OBJECT( GetBody(entity) ), "event",
			       (GtkSignalFunc) GnomeEventBody, entity );  
	  }
	  break;
	}
    }
  
  SetClickSub( entity, false ); // we haven't been clicked on
}

gint GnomeEventRoot(GnomeCanvasItem *item, GdkEvent *event, gpointer data)
{
  assert( item );
  assert( data );

  GdkCursor *cursor = NULL; // change cursor to indicate context
  CEntity* entity = (CEntity*)data; // cast the data ptr for convenience

  // we should be the root object
  assert( entity == entity->m_world->root );
 
  switch (event->type) 
    {
    case GDK_BUTTON_PRESS: // begin a drag or spin
      break;
      
    case GDK_MOTION_NOTIFY: // perform a drag, spin or zoom
      break;
      
    case GDK_BUTTON_RELEASE: // end a drag, spin or zoom
      break;
      
    case GDK_2BUTTON_PRESS: // double-click sub/unsubscribes
      break;

    case GDK_ENTER_NOTIFY:
      //puts( "enter root - GuiSelectionDull" );
      if( g_watched ) GnomeSelectionDull();
      break;
      
    case GDK_LEAVE_NOTIFY:
      //puts( "leave root" );
        break;

    default:
      break;
    }
        
  return FALSE;
}

gint GnomeEventBody(GnomeCanvasItem *item, GdkEvent *event, gpointer data)
{
  assert( item );
  assert( data );

  CEntity* entity = (CEntity*)data; // cast the data ptr for convenience
 
  switch (event->type) 
    {
    case GDK_ENTER_NOTIFY:
      //puts( "enter top level entity - GuiSelect" );
      GnomeSelect( entity );
      break;
      
    case GDK_LEAVE_NOTIFY:
      //puts( "leave top level entity" );
        break;

    default:
      break;
    }
        
  return FALSE;
}

gint GnomeEventSelection(GnomeCanvasItem *item, GdkEvent *event, gpointer data)
{
  assert( item );
  assert( event );
  assert( data );
  assert( g_watched );
 
  switch (event->type) 
    {
    case GDK_ENTER_NOTIFY:
      //puts( "enter select ellipse - GuiSelect" );
      GnomeSelectionBright();
      break;
      
    case GDK_LEAVE_NOTIFY:
      //puts( "leave select ellipse - GuiShrinkSelect" );
        break;

    default:
      break;
    }
        
  return FALSE;
}

void GnomeRenderGrid( CWorld* world, double spacing, StageColor color )
{
  // add a m/5 grid
  GnomeCanvasPoints *gcp = gnome_canvas_points_new(2);
  
  double width = world->matrix->width / world->ppm;
  double height = world->matrix->height/ world->ppm;

  for( double xx = 0.0; xx < width; xx+=spacing )
    {
      gcp->coords[0] = xx;
      gcp->coords[1] = 0;
      gcp->coords[2] = xx;
      gcp->coords[3] = height;
      
      gnome_canvas_item_new ( GetGroup(world->root), 
			      gnome_canvas_line_get_type(),
			      "points", gcp,
			      "fill_color_rgba", RGBA(color,255),
			      "width_pixels", 1,
			      NULL );
    }
  
  for( double yy = 0.0; yy < world->matrix->height / world->ppm; yy+=spacing )
    {
      gcp->coords[0] = 0;
      gcp->coords[1] = yy;
      gcp->coords[2] = width;
      gcp->coords[3] = yy;
      
      gnome_canvas_item_new( GetGroup( world->root), 
			     gnome_canvas_line_get_type(),
			     "points", gcp,
			     "fill_color_rgba", RGBA(color,255),
			     "width_pixels", 1,
			     NULL );
    }
}


void GnomeSelectionDull( void )
{
  assert( g_watched );

  // change the size and color of the ellipse
  gnome_canvas_item_set( g_select_ellipse,
			 "fill_color_rgba", RGBA(RGB(255,255,0),64),
			 "outline_color_rgba", RGBA(0,128),
			 "width_pixels", 1,
			 NULL );
  
  // disable button actions
  //SetClickSub( g_watched, false );
  g_click_respond = false;

   gnome_canvas_item_lower( g_select_ellipse, 99 );   
}

void GnomeSelectionBright( void )
{
  assert( g_watched );
   
   // change the appearance of the ellipse
   gnome_canvas_item_set( g_select_ellipse,
			  "fill_color_rgba", RGBA(RGB(255,255,0),96),
			  "outline_color", "black",
			  "width_pixels", 1,
			  NULL );
   
   //SetClickSub( g_watched, true );
  g_click_respond = true;

   gnome_canvas_item_lower( g_select_ellipse, 99 );   
}


void GnomeSelect( CEntity* ent )
{
  if( ent->m_parent_entity == NULL ) // i'm root! no selection
    return;
  
  if( g_watched == ent )
    return;

  if( g_watched ) GnomeUnselect( g_watched );

  g_watched = ent;
  
  //printf( "select: %d \n", ent->stage_type ); 
  
  double border = 0.7;
  double noselen = 0.2;
  double nosewid = 0.1;
  
  g_select_rect = 
    (GnomeCanvasGroup*)gnome_canvas_item_new( GetGroup(g_watched),
					      gnome_canvas_group_get_type(),
					      "x", 0,
					      "y", 0,
					      NULL);
  
   // a yellow ellipse
  g_select_ellipse = gnome_canvas_item_new( g_select_rect,
					    gnome_canvas_ellipse_get_type(),
					    "x1", -(g_watched->size_x/2.0 + border),
					    "y1", -(g_watched->size_y/2.0 + border),
					    "x2", +(g_watched->size_x/2.0 + border),
					    "y2", +(g_watched->size_x/2.0 + border),
					    NULL );

  GnomeSelectionBright();
  
  // attach a callback to the select group
  gtk_signal_connect(GTK_OBJECT( g_select_rect ), "event",
		     (GtkSignalFunc) GnomeEventSelection, ent );
  
  
   // a heading indicator triangle
   GnomeCanvasPoints *pts = gnome_canvas_points_new(3);
   
   pts->coords[0] =  +(ent->size_x/2.0 + border) - noselen/2.0;
   pts->coords[1] =  -nosewid;
   pts->coords[2] = pts->coords[0];
   pts->coords[3] = +nosewid;
   pts->coords[4] =  +(ent->size_x/2.0 + border) + noselen/2.0;
   pts->coords[5] =  0;
   
   gnome_canvas_item_new( g_select_rect,
			  gnome_canvas_polygon_get_type(),
			  "points", pts,
			  "fill_color_rgba", RGBA(RGB(255,255,0),255),
			  "outline_color", "black",
			  "width_pixels", 1,
			  NULL );
   
   
   
   gnome_canvas_item_lower( (GnomeCanvasItem*)g_select_rect, 99 );
   
   gnome_appbar_push( g_appbar, "Select" );
   GnomeStatus( ent );
 
}

void GnomeUnselect( CEntity* ent )
{
  if( ent->m_parent_entity == NULL ) // i'm root! no selection
    return;
  
  if( ent->m_parent_entity != ent-> m_world->root )
    {
      GnomeUnselect( ent->m_parent_entity );
      return;
    }
  
  if( g_select_rect )
    {
      gtk_object_destroy(GTK_OBJECT(g_select_rect));
      g_select_rect = NULL;
    }
      
  // remove my status from the appbar
  assert( g_appbar );
  gnome_appbar_pop(  g_appbar );  
}

void GnomeStatus( CEntity* ent )
{
  // describe my status on the appbar
  const int buflen = 256;
  char buf[buflen];

  ent->GetStatusString( buf, buflen );
  
  assert( g_appbar );
  gnome_appbar_set_status( g_appbar, buf );  
}


void GnomeEntityPropertyChange( CEntity* ent, EntityProperty prop )
{
  //PRINT_DEBUG( "" );
  assert( ent );
  assert( prop > 0 );
  assert( prop < ENTITY_LAST_PROPERTY );

 // if the GUI is not up and running, do nothing
  if( !g_canvas ) 
    return; 

  if( !ent->gui_data )
    return;

  switch( prop )
    {
    case PropPoseX: // if the entity moved
    case PropPoseY:
    case PropPoseTh:
      GnomeEntityMove( ent ); // move the gui figure
      break;
      
    case PropData:
      GnomeEntityRenderData( ent );
      break;
      
    case PropPlayerSubscriptions:
      // if we're no longer subscribed, delete the data figure
      //if( ent->Subscribed() < 1 && GetData( ent ) )
      //{
      //  gtk_object_destroy( GTK_OBJECT(GetData(ent)));
      //  SetData( ent, NULL );
      //}
      break;
      // TODO - more properties here
 
    default:
      break;
    }
}
  
void GnomeEntityRenderData( CEntity* ent )
{
  //PRINT_DEBUG( "" );

  switch( ent->stage_type )
    {
    case LaserTurretType:
      GnomeEntityRenderDataLaser(  (CLaserDevice*)ent );
      break;

    case SonarType:
      GnomeEntityRenderDataSonar( (CSonarDevice*)ent );
      break;

    default:
      break;
    }
}


void GnomeEntityMove( CEntity* ent )
{
  // if the GUI is not up and running, do nothing
  if( !g_canvas ) 
    return; 

  if( !ent->gui_data )
    return;
  
  assert( GetOrigin( ent ) );
  assert( GetGroup( ent ) );
  
  double rotate[6], translate[6];
  double x, y, th;
  ent->GetPose( x, y, th );

  art_affine_translate( translate, x, y );
  art_affine_rotate( rotate, RTOD(th) );
  
  // translate the origin
  gnome_canvas_item_affine_absolute( GNOME_CANVAS_ITEM(GetOrigin(ent)), translate);
  
  // rotate the group about the origin
  gnome_canvas_item_affine_absolute( GNOME_CANVAS_ITEM(GetGroup(ent)), rotate);
  
  // if this item is watched we update the status bar
  if( g_watched == ent  ) GnomeStatus( ent );
}


/* a callback for the about menu item, it will display a simple "About"                 
 * dialog box standard to all gnome applications                        
 */                     
void GnomeAboutBox(GtkWidget *widget, gpointer data)                   
{                       
  GdkPixbuf* logo = NULL;

  assert( logo = gdk_pixbuf_new_from_xpm_data( (const char**)stage_xpm ) );
  
  //gdk_pixbuf_new_from_file( "/home/vaughan/PS/stage/stage.jpg", NULL );
  //assert( logo );

  GtkWidget* box = gnome_about_new( stage_name,                
				    VERSION,
				    stage_copyright,
				    stage_comments,
				    stage_authors,
				    stage_documenters,
				    stage_translators,
				    logo );
  gtk_widget_show(box);           
} 

void GnomeSubscribeToAll(GtkWidget *widget, gpointer data)                   
{
  // recursively subscribe/unsubscribe to everything
  static bool sub = false;
  sub = !sub; // invert sub
  sub ? CWorld::root->FamilySubscribe() : CWorld::root->FamilyUnsubscribe();
}


void GnomeCheckSub( CEntity* ent )
{
  if( ent->Subscribed() < 1 && GetData( ent ) )
    {
      gtk_object_destroy( GTK_OBJECT(GetData(ent)) );
      SetData( ent, NULL );
    }
}


#endif


