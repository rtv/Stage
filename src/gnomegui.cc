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
 * Desc: Gnome GUI world components (all methods begin with 'Gui')
 * Author: Richard Vaughan
 * Date: 7 Dec 2000
 * CVS info: $Id: gnomegui.cc,v 1.10 2002-09-26 07:22:38 rtv Exp $
 */


#if HAVE_CONFIG_H
  #include <config.h>
#endif
#if HAVE_STRINGS_H
  #include <strings.h>
#endif

// if stage was configured to use the experimental GNOME GUI
#ifdef USE_GNOME2

//#undef DEBUG
//#undef VERBOSE
//#define DEBUG 
//#define VERBOSE

#include "world.hh"
#include "gnomegui.hh"
#include "playerdevice.hh"

// include the logo XPM
#include "stage.xpm"

const double select_range = 0.6;

void StageQuit( void ); // declare quit func


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


// MENU DEFINITIONS ////////////////////////////////////////////////////////////////////////////

// calback functions
#define new_app_cb NULL
#define open_cd NULL
#define save_as_cb NULL
//#define close_cb NULL
#define exit_cb StageQuit
#define open_cb NULL
#define about_cb CWorld::GuiAboutBox

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
	GNOMEUIINFO_ITEM_STOCK("Run", "Run the simulation",
			      NULL, GNOME_STOCK_PIXMAP_FORWARD),
	GNOMEUIINFO_ITEM_STOCK("Stop", "Stop the simulation",
			      NULL, GNOME_STOCK_PIXMAP_STOP),
	GNOMEUIINFO_END
};

// CWORLD ////////////////////////////////////////////////////////////////////////////////

/* a callback for the about menu item, it will display a simple "About"                 
 * dialog box standard to all gnome applications                        
 */                     

void CWorld::GuiAboutBox(GtkWidget *widget, gpointer data)                   
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

void CWorld::GuiSubscribeToAll(GtkWidget *widget, gpointer data)                   
{
  // recursively subscribe/unsubscribe to everything
  static bool sub = false;
  sub = !sub; // invert sub
  sub ? CWorld::root->FamilySubscribe() : CWorld::root->FamilyUnsubscribe();
}

void CWorld::GuiStartup( void )
{ 
  // Allow rgb image handling
  gdk_rgb_init();
  
  // create the app
  this->g_app = GNOME_APP(gnome_app_new( "stage", "Stage" ));

  // make user-resizable
  gtk_window_set_policy(GTK_WINDOW(g_app), FALSE, TRUE, FALSE);

  // add menus  
  gnome_app_create_menus( this->g_app, main_menu );
  
  // setup appbar
  this->g_appbar = GNOME_APPBAR(gnome_appbar_new(TRUE, TRUE, GNOME_PREFERENCES_USER));
  assert( g_appbar );
  gnome_appbar_set_default(  this->g_appbar, "No selection" );
  gtk_progress_bar_set_text( gnome_appbar_get_progress( this->g_appbar ), "time" );
  gnome_app_set_statusbar(GNOME_APP(this->g_app), GTK_WIDGET(this->g_appbar));
  gnome_app_create_toolbar(GNOME_APP(this->g_app), toolbar);

  // add menu hints to appbar
  //gnome_app_install_menu_hints(GNOME_APP(this->g_app), main_menumenubar);
  
  // create a canvas
  gtk_widget_push_visual(gdk_rgb_get_visual());
  gtk_widget_push_colormap(gdk_rgb_get_cmap());
  // TODO - read which type to use from world file & menus
  //this->g_canvas = GNOME_CANVAS(gnome_canvas_new()); // Xlib - fast
  this->g_canvas = GNOME_CANVAS(gnome_canvas_new_aa()); //  anti-aliased, slow
  gtk_widget_pop_colormap();
  gtk_widget_pop_visual();

  // set the starting size - just a quick hack for starters
  int initwidth = this->matrix->get_width() + 40;
  int initheight = this->matrix->get_height() + 70;

  // sensible bounds check
  if( initwidth > gdk_screen_width() ) initwidth = gdk_screen_width();
  if( initheight > gdk_screen_height() ) initheight = gdk_screen_height();
  
  gtk_window_set_default_size(GTK_WINDOW(g_app), initwidth, initheight );

  
  // connect a signal handler for button events
  
  // set up the scrolling
  GtkWidget* sw = gtk_scrolled_window_new(NULL, NULL);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw),
                                 GTK_POLICY_AUTOMATIC,
                                 GTK_POLICY_AUTOMATIC);
  
  //GtkWidget*  hruler = gtk_hruler_new();
  //GtkWidget*  vruler = gtk_vruler_new();

  gtk_container_add(GTK_CONTAINER(sw), GTK_WIDGET(this->g_canvas));
  //gtk_container_add(GTK_CONTAINER(sw), hruler );
  //gtk_container_add(GTK_CONTAINER(sw), vruler );

  // bind quit function to the top-level window
  gtk_signal_connect(GTK_OBJECT(g_app),
                     "delete_event",
                     GTK_SIGNAL_FUNC(StageQuit),
                     NULL);
  
  // set the app's main window to be the scrolled window
  gnome_app_set_contents( GNOME_APP(g_app), sw );
  
  // set scaling and scroll viewport
  gnome_canvas_set_pixels_per_unit( g_canvas, ppm );
  gnome_canvas_set_scroll_region( g_canvas, 
				  0, 0,
				  matrix->width / ppm,
				  matrix->height / ppm );

  // this creates the GUI elements in the entity tree
  root->GuiStartup();
 
  // flip the coordinate system, so that y increases upwards
  //GnomeCanvasGroup *root = gnome_canvas_root(GNOME_CANVAS(canvas)); 
  //double flip[6] = { 1, 0, 0, -1, 0, 0 }; 
  //gnome_canvas_item_affine_relative(GNOME_CANVAS_ITEM(root), flip); 
  
  // attach a callback for events to the root group in the canvas
  gtk_signal_connect(GTK_OBJECT(gnome_canvas_root(this->g_canvas)), "event",
		     (GtkSignalFunc) CWorld::GuiEvent,
		     this );
  
  // start a callback to display the simultion time in the status bar
  /* Add a timer callback to update the value of the progress bar */
  gtk_timeout_add (100, CWorld::GuiProgressTimeout, this );

  // check every now and again for dead subscription
  //gtk_timeout_add (1000, CWorld::GuiProgressTimeout, this );

  gtk_widget_show_all(GTK_WIDGET(g_app));
}

enum MotionMode 
{
  Nothing = 0, Dragging, Spinning, Zooming
};

/* Update the value of the progress bar so that we get
 * some movement */
gboolean CWorld::GuiProgressTimeout( gpointer data )
{
  CWorld* world = (CWorld*)data;

  // set the text to be the current time
  char seconds_buf[128];
  snprintf( seconds_buf, 128, "%.2f", world->GetTime() );
  gtk_progress_bar_set_text( gnome_appbar_get_progress(world->g_appbar), seconds_buf );

  
  // if stage is counting down to a stop time, show the fraction of time used up
  if( world->m_stoptime )
    gtk_progress_bar_set_fraction( gnome_appbar_get_progress(world->g_appbar), 
				     world->GetTime() / world->GetStopTime() );
  
  /* As this is a timeout function, return TRUE so that it
   * continues to get called */
  return TRUE;
} 

gint CWorld::GuiEvent(GnomeCanvasItem *item, GdkEvent *event, gpointer data)
{
  assert( item );
  assert( data );

  GdkCursor *cursor = NULL; // change cursor to indicate context
  CWorld* world = (CWorld*)data; // cast the data ptr for convenience
 
  static MotionMode mode = Nothing;
  
  static double drag_offset_x;
  static double drag_offset_y;

  static double spin_start_th;
  static double spin_start_x;
  static double zoom = 1.0;
  static double last_x = 0.0;

  double item_x, item_y; // pointer pos in local coords
  double win_x = 0, win_y = 0; // pointer pos in window coords


  item_x = event->button.x;
  item_y = event->button.y;

  gnome_canvas_item_w2i(GNOME_CANVAS_ITEM(world->root->g_origin), &item_x, &item_y);

  gnome_canvas_world_to_window( world->g_canvas, event->button.x, event->button.y,
				&win_x, &win_y );

  //printf( "pointer %.2f %.2f \n", item_x, item_y );

  static CEntity* selected = NULL;

  switch (event->type) 
    {
    case GDK_BUTTON_PRESS: // begin a drag, spin or zoom
      {
	if( event->button.button == 1 || event->button.button == 3 )
	  {
	    CEntity* nearby = world->GetNearestChildWithinRange( item_x, item_y, select_range, world->root );
	    // selected may be null if we clicked too far from anything
	    //selected = world->GetNearestChildWithinRange( item_x, item_y, 
	    //					  1.0, world->root );
	    
	    if( selected && nearby == NULL ) // clicked away from everything - cancel selection
	      {
		selected->GuiUnselect();
		selected->GuiUnwatch();
		selected = NULL;
	      }
	    else if( selected &&  nearby == selected ) // we clicked on the same thing again - do nothing
	      {
		selected->GuiUnwatch();
		//selected->GuiUnselect();
		selected->GuiSelect();
	      }
	    else if( !selected && nearby ) // no previous selection, select nearby
	      { 
		selected = nearby;
		selected->GuiSelect();
	      }
	    else if( selected && nearby ) // previous selection, now select nearby
	      {
		selected->GuiUnselect();
		selected->GuiUnwatch();
		selected = nearby;
		selected->GuiSelect();
	      }		 
	  }
	
	switch(event->button.button) 
	  {
	  case 1: // start dragging
	    if( selected )
	      {
		//puts( "START DRAG" );
		cursor = gdk_cursor_new(GDK_FLEUR);
		mode = Dragging;
		
		double dummy;
		selected->GetPose( drag_offset_x, drag_offset_y, dummy );
		
		// subtract the cursor position to get the offsets
		drag_offset_x -= item_x;
		drag_offset_y -= item_y;

	      }
	    break;
	    
	  case 3: // start spinning
	    if( selected )
	      {
		//puts( "START SPIN" );
		cursor = gdk_cursor_new(GDK_EXCHANGE);
		mode = Spinning;
		
		double dummy;
		selected->GetPose( dummy, dummy, spin_start_th );
		spin_start_x = win_x;
	      }
	    break;
	    
	  case 2: // start zooming
	    puts( "START ZOOM" );
	    cursor = gdk_cursor_new(GDK_SB_H_DOUBLE_ARROW);
	    mode = Zooming;
	    last_x = win_x;
	    break;
	    
	  case 4:
	    //puts( "ZOOM IN" );
	    break;
	    
	  case 5:
	    //puts( "ZOOM OUT" );
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
	  if( selected )
	    {		  
	      //puts( "DRAGGING" );
	      // move the entity
	      double newpos_x = item_x + drag_offset_x;
	      double newpos_y = item_y + drag_offset_y;

	      selected->SetProperty( -1, PropPoseX, &newpos_x, sizeof(item_x) );
	      selected->SetProperty( -1, PropPoseY, &newpos_y, sizeof(item_y) );
	    }
	  break;
	  
	case Spinning:
	  if( selected )
	    {
	      //puts( "SPINNING" );
	      // rotate the entity
	      double mouse_delta = win_x - spin_start_x;
	      double angle = spin_start_th - (mouse_delta/20.0);
	      selected->SetProperty( -1, PropPoseTh, &angle, sizeof(angle) );
	    }
	  break;
	  
	case Zooming:
	  // for efficiency, we only redraw when the mouse has moved a fair way
	  if( fabs( win_x - last_x ) > 10 )
	    {
	      //puts( "ZOOMING" );
	      win_x > last_x ? zoom *= 1.05 : zoom *= 0.95;
	      //printf( "win_x: %.2f last_x %.2f zoom: %.2f\n", win_x, last_x, zoom );
	      if( zoom < 0.1 ) zoom = 0.1;
	      if( zoom > 10.0 ) zoom = 10.0;
	      
	      gnome_canvas_set_pixels_per_unit( world->g_canvas, 
						zoom * world->ppm );
	      last_x = win_x;
	    }
	  break;
	  
	default:
	  PRINT_ERR1( "invalid mouse mode %d\n", mode );
	  break;
	  
	}
      break;
      
    case GDK_BUTTON_RELEASE: // end a drag, spin or zoom

      switch(event->button.button) 
	{
	case 1: // handle selection and dragging
	case 3:

	  if( selected )
	    {
	      selected->GuiUnselect();
	      selected->GuiWatch();
	      //selected = NULL;
	    }
	  break;
	  
	case 2: 
	    break;
	  
	case 4:
	  //puts( "ZOOM IN" );
	  break;
	  
	case 5:
	  //puts( "ZOOM OUT" );
	  break;
	  
	default:
	  break;
	}
      
      if( mode != Nothing )
	{
	  //puts( "RELEASE" );
	  mode = Nothing;
	  gnome_canvas_item_ungrab(item, event->button.time);      
	}
      break;
      
    case GDK_2BUTTON_PRESS: // double-click sub/unsubscribes
      //puts( "DOUBLECLICK - TOGGLE SUBS" );
      if( selected )
	{
	  selected->click_subscribed ? 
	    selected->FamilyUnsubscribe() : selected->FamilySubscribe();
	  
	  selected->click_subscribed = !selected->click_subscribed; // invert 
	}
    case GDK_ENTER_NOTIFY:
      //puts( "enter" );
      break;
      
    case GDK_LEAVE_NOTIFY:
      //puts( "leave" );
      break;

    default:
      break;
    }
        
  return TRUE;
}

// CENTITY ////////////////////////////////////////////////////////////////////////////////

void CEntity::GuiStartup( void )
{
  this->g_origin = NULL;
  this->g_group = NULL;
  this->g_data = NULL;
  this->g_body= NULL;

  // find our parent group - if we're root it's the gnome_canvas_root
  GnomeCanvasGroup* parent_group;
  if( m_parent_entity )
    parent_group = m_parent_entity->g_group;
  else
    parent_group = gnome_canvas_root( m_world->g_canvas );
  
  // make a group for this entity
  this->g_origin = 
    (GnomeCanvasGroup*)gnome_canvas_item_new( parent_group,
					      gnome_canvas_group_get_type(),
					      "x", this->local_px,
					      "y", this->local_py,
					      NULL);
  
  this->g_group = 
    (GnomeCanvasGroup*)gnome_canvas_item_new( g_origin,
					      gnome_canvas_group_get_type(),
					      "x", 0,
					      "y", 0,
					      NULL);

  // specify the affine matrix for the rectangle, to do the rotation
  double affine[6];
  art_affine_rotate( affine, RTOD(local_pth) );
  gnome_canvas_item_affine_relative( GNOME_CANVAS_ITEM(g_group), affine );
  
  GtkType shape = 0;

  // choose a shape
  switch( this->shape )
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
    this->g_body =  gnome_canvas_item_new (g_group, shape,
					   "x1", - size_x/2.0,
					   "y1", - size_y/2.0,
					   "x2", + size_x/2.0,
					   "y2", + size_y/2.0,
					   "fill_color_rgba", RGBA(this->color,255),
					   "outline_color_rgba", RGBA(this->color,255),
					   "width_pixels", 1,
					   NULL );
  
  if( m_parent_entity == NULL ) // we're root
    {
      // draw a rectangle around the root entity as a background for the world
      this->g_body = 
	gnome_canvas_item_new (g_group,
			       gnome_canvas_rect_get_type(),
			       "x1", 0.0,
			       "y1", 0.0,
			       "x2", m_world->matrix->width / m_world->ppm,
			       "y2", m_world->matrix->width / m_world->ppm,
			       "fill_color_rgba", RGBA(::LookupColor(BACKGROUND_COLOR),255),
			       "outline_color", "black",
			       "width_pixels", 1,
			       NULL );

      //this->GuiRenderGrid( 0.2, ::LookupColor(GRID_MINOR_COLOR) );
      //this->GuiRenderGrid( 1.0, ::LookupColor(GRID_MAJOR_COLOR) );
    }
	
  
  this->click_subscribed = false; // we haven't been clicked on

  CHILDLOOP( ch )
    ch->GuiStartup();
}

void CEntity::GuiRenderGrid( double spacing, StageColor color )
{
  // add a m/5 grid
  GnomeCanvasPoints *gcp = gnome_canvas_points_new(2);
  
  double width = m_world->matrix->width / m_world->ppm;
  double height = m_world->matrix->height/ m_world->ppm;

  for( double xx = 0.0; xx < width; xx+=spacing )
    {
      gcp->coords[0] = xx;
      gcp->coords[1] = 0;
      gcp->coords[2] = xx;
      gcp->coords[3] = height;
      
      gnome_canvas_item_new (g_group, gnome_canvas_line_get_type(),
			     "points", gcp,
			     "fill_color_rgba", RGBA(color,255),
			     "width_pixels", 1,
			     NULL );
    }
  
  for( double yy = 0.0; yy < m_world->matrix->height / m_world->ppm; yy+=spacing )
    {
      gcp->coords[0] = 0;
      gcp->coords[1] = yy;
      gcp->coords[2] = width;
      gcp->coords[3] = yy;
      
      gnome_canvas_item_new (g_group, gnome_canvas_line_get_type(),
			     "points", gcp,
			     "fill_color_rgba", RGBA(color,255),
			     "width_pixels", 1,
			     NULL );
    }
}


GnomeCanvasGroup* watch_rect = NULL;


void CEntity::GuiWatch( void )
{
  printf( "watch: %d \n", this->stage_type ); 

   double border = select_range/2.0;
   double noselen = 0.2;
   double nosewid = 0.1;
   
   watch_rect = 
     (GnomeCanvasGroup*)gnome_canvas_item_new( this->g_group,
					       gnome_canvas_group_get_type(),
					       "x", 0,
					       "y", 0,
					       NULL);

   // a yellow ellipse
   gnome_canvas_item_new( watch_rect,
			  gnome_canvas_ellipse_get_type(),
			  "x1", -(this->size_x/2.0 + border),
			  "y1", -(this->size_y/2.0 + border),
			  "x2", +(this->size_x/2.0 + border),
			  "y2", +(this->size_x/2.0 + border),
			  "fill_color_rgba", RGBA(RGB(255,255,0),64),
			  "outline_color_rgba", RGBA(0,128),
			  "width_pixels", 1,
			  NULL );

   // a heading indicator triangle
   GnomeCanvasPoints *pts = gnome_canvas_points_new(3);
   
   pts->coords[0] =  +(this->size_x/2.0 + border) - noselen/2.0;
   pts->coords[1] =  -nosewid;
   pts->coords[2] = pts->coords[0];
   pts->coords[3] = +nosewid;
   pts->coords[4] =  +(this->size_x/2.0 + border) + noselen/2.0;
   pts->coords[5] =  0;
   
   gnome_canvas_item_new( watch_rect,
			  gnome_canvas_polygon_get_type(),
			  "points", pts,
			  "fill_color_rgba", RGBA(RGB(255,255,0),64),
			  "outline_color_rgba", RGBA(0,128),
			  "width_pixels", 1,
			  NULL );

   gnome_canvas_item_lower( (GnomeCanvasItem*)watch_rect, 99 );
   
   this->watched = true;

   gnome_appbar_push(  this->m_world->g_appbar, "Watch" );
   this->GuiStatus();   
}

void CEntity::GuiUnwatch( void )
{
  if( watch_rect )
    {
      gtk_object_destroy(GTK_OBJECT(watch_rect));
      watch_rect = NULL;
    }

   this->watched = false;
      
  // remove my status from the appbar
   assert( this->m_world->g_appbar );
   gnome_appbar_pop(  this->m_world->g_appbar );  
}

GnomeCanvasGroup* select_rect = NULL;


void CEntity::GuiSelect( void )
{
  printf( "select: %d \n", this->stage_type ); 

   double border = select_range;
   double noselen = 0.2;
   double nosewid = 0.1;
   
   select_rect = 
     (GnomeCanvasGroup*)gnome_canvas_item_new( this->g_group,
					       gnome_canvas_group_get_type(),
					       "x", 0,
					       "y", 0,
					       NULL);

   // a yellow ellipse
   gnome_canvas_item_new( select_rect,
			  gnome_canvas_ellipse_get_type(),
			  "x1", -(this->size_x/2.0 + border),
			  "y1", -(this->size_y/2.0 + border),
			  "x2", +(this->size_x/2.0 + border),
			  "y2", +(this->size_x/2.0 + border),
			  "fill_color_rgba", RGBA(RGB(255,255,0),64),
			  "outline_color", "black",
			  "width_pixels", 1,
			  NULL );

   // a heading indicator triangle
   GnomeCanvasPoints *pts = gnome_canvas_points_new(3);
   
   pts->coords[0] =  +(this->size_x/2.0 + border) - noselen/2.0;
   pts->coords[1] =  -nosewid;
   pts->coords[2] = pts->coords[0];
   pts->coords[3] = +nosewid;
   pts->coords[4] =  +(this->size_x/2.0 + border) + noselen/2.0;
   pts->coords[5] =  0;
   
   gnome_canvas_item_new( select_rect,
			  gnome_canvas_polygon_get_type(),
			  "points", pts,
			  "fill_color_rgba", RGBA(RGB(255,255,0),255),
			  "outline_color", "black",
			  "width_pixels", 1,
			  NULL );



   gnome_canvas_item_lower( (GnomeCanvasItem*)select_rect, 99 );
   
   this->watched = true;

   gnome_appbar_push(  this->m_world->g_appbar, "Select" );
   this->GuiStatus();
   
}

void CEntity::GuiUnselect( void )
{
  if( select_rect )
    {
      gtk_object_destroy(GTK_OBJECT(select_rect));
      select_rect = NULL;
    }
      
   this->watched = false;

  // remove my status from the appbar
  assert( this->m_world->g_appbar );
  gnome_appbar_pop(  this->m_world->g_appbar );  
}

void CEntity::GuiStatus( void )
{
  // describe my status on the appbar
  const int buflen = 256;
  char buf[buflen];
  double x, y, th;
  this->GetGlobalPose( x, y, th );

  // check for overflow
  assert( -1 != 
	  snprintf( buf, buflen, 
		   "Pose(%.2f,%.2f,%.2f) Stage(%d:%d)",
		   x, y, th, 
		    this->stage_id,
		    this->stage_type ) );
  
  assert( this->m_world->g_appbar );
  gnome_appbar_set_status(  this->m_world->g_appbar, buf );  
}

void CPlayerEntity::GuiStatus( void )
{
  // describe my status on the appbar
  const int buflen = 256;
  char buf[buflen];
  double x, y, th;
  this->GetGlobalPose( x, y, th );
  
  // check for overflow
  assert( -1 !=
	  snprintf( buf, buflen, 
		    "Pose(%.2f,%.2f,%.2f) Player(%d:%d:%d) Stage(%d:%d)",
		    x, 
		    y, 
		    th,
		    this->m_player.port, 
		    this->m_player.code, 
		    this->m_player.index, 
		    this->stage_id,
		    this->stage_type
		    ) );
  
  assert( this->m_world->g_appbar );
  gnome_appbar_set_status( this->m_world->g_appbar, buf );  
}

                  
void CEntity::GuiMove( void )
{
  // if the GUI is not up and running, do nothing
  if( !this->m_world->g_canvas ) 
    return; 
  
  assert( this->g_origin );
  assert( this->g_group );
  
  double rotate[6], translate[6];
  art_affine_translate( translate, local_px, local_py );
  art_affine_rotate( rotate, RTOD(local_pth) );
  
  // translate the origin
  gnome_canvas_item_affine_absolute( GNOME_CANVAS_ITEM(g_origin), translate);
  
  // rotate the group about the origin
  gnome_canvas_item_affine_absolute( GNOME_CANVAS_ITEM(g_group), rotate);

  // if this item is watched we update the status bar
  if( this->watched )
    this->GuiStatus();
}

#endif

