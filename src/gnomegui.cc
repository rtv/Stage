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
 * Desc: Gnome GUI world components for CWorld and CEntity
 * Author: Richard Vaughan, Andrew Howard
 * Date: 7 Dec 2000
 * CVS info: $Id: gnomegui.cc,v 1.1 2002-09-07 02:05:23 rtv Exp $
 */


#if HAVE_CONFIG_H
  #include <config.h>
#endif
#if HAVE_STRINGS_H
  #include <strings.h>
#endif

// if stage was configured to use the experimental GNOME GUI
#ifdef RTVG

//#undef DEBUG
//#undef VERBOSE
//#define DEBUG 
//#define VERBOSE

#include "world.hh"
#include "menus.hh"

// CWORLD ////////////////////////////////////////////////////////////////////////////////

/* a callback for the about menu item, it will display a simple "About"                 
 * dialog box standard to all gnome applications                        
 */                     

void CWorld::GuiAboutBox(GtkWidget *widget, gpointer data)                   
{                       
  GtkWidget* box = gnome_about_new(/*title: */ "Stage",                
				   /*version: */VERSION,
				   /*copyright: */ "(C) the Authors, 2000-2002",
				   /*authors: */stage_authors,
				   /*other comments: */
				   "Funding & facilites:\n\tDARPA\n"
				   "\tUniversity of Southern California\n"
				   "\tHRL Laboratories LLC\n"
				   "\nHomepage: http://playerstage.sourceforge.net\n"
				   "\n\"All the World's a stage "
				   "and all the men and women merely players\"",
				   NULL);
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

  // bind quit function
  gtk_signal_connect( GTK_OBJECT(g_app), "delete_event",
		      GTK_SIGNAL_FUNC(gtk_main_quit), NULL );

  // add menus  
  gnome_app_create_menus( this->g_app, main_menu );
  
  // setup appbar
  this->g_appbar = GNOME_APPBAR(gnome_appbar_new(FALSE, TRUE, GNOME_PREFERENCES_USER));
  gnome_appbar_set_default(  this->g_appbar, "No selection" );
  gnome_app_set_statusbar(GNOME_APP(this->g_app), GTK_WIDGET(this->g_appbar));
  
  // add menu hints to appbar
  //gnome_app_install_menu_hints(GNOME_APP(this->g_app), main_menumenubar);
  
  // create a canvas
  gtk_widget_push_visual(gdk_rgb_get_visual());
  gtk_widget_push_colormap(gdk_rgb_get_cmap());
  // TODO - read which type to use from world file
  this->g_canvas = GNOME_CANVAS(gnome_canvas_new()); // Xlib - fast
  //this->g_canvas = GNOME_CANVAS(gnome_canvas_new_aa()); //  anti-aliased, slow
  gtk_widget_pop_colormap();
  gtk_widget_pop_visual();

  
  // connect a signal handler for button events
  
  // set up the scrolling
  GtkWidget* sw = gtk_scrolled_window_new(NULL, NULL);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw),
                                 GTK_POLICY_AUTOMATIC,
                                 GTK_POLICY_AUTOMATIC);
  
  GtkWidget*  hruler = gtk_hruler_new();
  GtkWidget*  vruler = gtk_vruler_new();

  gtk_container_add(GTK_CONTAINER(sw), GTK_WIDGET(this->g_canvas));
  //gtk_container_add(GTK_CONTAINER(sw), hruler );
  //gtk_container_add(GTK_CONTAINER(sw), vruler );

  // set the widget size
  // TODO
  //gtk_window_set_default_size(GTK_WINDOW(sw), 300, 300);

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
  
  gtk_widget_show_all(GTK_WIDGET(g_app));
}

enum MotionMode 
{
  Nothing = 0, Dragging, Spinning, Zooming
};



gint CWorld::GuiEvent(GnomeCanvasItem *item, GdkEvent *event, gpointer data)
{
  assert( item );
  assert( data );

  GdkCursor *cursor = NULL; // change cursor to indicate context
  CWorld* world = (CWorld*)data; // cast the data ptr for convenience
 
  static MotionMode mode = Nothing;
  
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
	//puts( "BUTTON" );
	
	if( selected ) // if something was selected, unselect it
	  { 
	    selected->GuiUnselect();
	    selected = NULL;
	  }
	
	// selected may be null if we clicked too far from anything
	selected = world->GetNearestChildWithinRange( item_x, item_y, 
						      1.0, world->root );
	if( selected ) // if something was selected, unselect it
	  selected->GuiSelect();	
	
	switch(event->button.button) 
	  {
	  case 1: // start dragging
	    if( selected )
	      {
		//puts( "START DRAG" );
		cursor = gdk_cursor_new(GDK_FLEUR);
		mode = Dragging;
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
	      selected->SetProperty( -1, PropPoseX, &item_x, sizeof(item_x) );
	      selected->SetProperty( -1, PropPoseY, &item_y, sizeof(item_y) );
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
      break;
      
    case GDK_LEAVE_NOTIFY:
      break;

    default:
      break;
    }
        
  return TRUE;
}

// CENTITY ////////////////////////////////////////////////////////////////////////////////

void CEntity::GuiStartup( void )
{
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
  
  GnomeCanvasPoints *points1;

  // draw a shape
  switch( this->shape )
    {
    case ShapeRect:
    case ShapeCircle: // TODO - implement circle
      //case ShapeNone:
      
      points1 = gnome_canvas_points_new(5);
      points1->coords[0] =  - size_x/2.0;
      points1->coords[1] =  - size_y/2.0;
      points1->coords[2] =  + size_x/2.0;
      points1->coords[3] =  - size_y/2.0;
      points1->coords[4] =  + size_x/2.0;
      points1->coords[5] =  + size_y/2.0;
      points1->coords[6] =  - size_x/2.0;
      points1->coords[7] =  + size_y/2.0;
      points1->coords[8] =  points1->coords[0]; 
      points1->coords[9] =  points1->coords[1]; 

      this->g_body =  gnome_canvas_item_new(g_group,
					   GNOME_TYPE_CANVAS_LINE,
					   "points", points1,
					   "fill_color_rgba", (this->color << 8)+255,
					   "width_pixels", 1,
					   NULL);
      break;
      /*this->g_body = 
	gnome_canvas_item_new (g_group,
			       // TODO - REPLACE THIS WITH ELLIPSE
			       //gnome_canvas_rect_get_type(),
			       // ELLIPSE DOESN'T HANDLE ROTATION
			       //gnome_canvas_ellipse_get_type(),
			       "x1", - size_x/2.0,
			       "y1", - size_y/2.0,
			       "x2", + size_x/2.0,
			       "y2", + size_y/2.0,
			       "fill_color_rgba", (this->color<<8)+255,
			       "width_pixels", 1,
			       NULL );
      break;
      */
    case ShapeNone: // draw nothin'.
      break;
    }      

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
			       "fill_color_rgba", (::LookupColor(BACKGROUND_COLOR)<< 8)+255,
			       "outline_color", "black",
			       "width_pixels", 1,
			       NULL );

      this->GuiRenderGrid( 0.2, ::LookupColor(GRID_MINOR_COLOR) );
      this->GuiRenderGrid( 1.0, ::LookupColor(GRID_MAJOR_COLOR) );
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
			     "fill_color_rgba", (color<<8) + 255,
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
			     "fill_color_rgba", (color<<8) + 255,
			     "width_pixels", 1,
			     NULL );
    }
}

void CEntity::GuiSelect( void )
{
  printf( "select: %d \n", this->stage_type ); 

  // make our outline glow yellow (if we have a body figure)
  if( this->g_body )
    gnome_canvas_item_set( this->g_body,
			   //"outline_color_rgba", 0xFFFF00BB,
			   "fill_color_rgba", 0xFFFF00FF,
			   "width_pixels", 3,
			   NULL );

  gnome_appbar_push(  this->m_world->g_appbar, "Select" );
  this->GuiStatus();

}

void CEntity::GuiUnselect( void )
{
  // put the outline back to normal (if we have a body figure)
  if( this->g_body )
    gnome_canvas_item_set( this->g_body,
			   //"outline_color_rgba", (this->color << 8)+255,
			   "fill_color_rgba", (this->color << 8)+255,
			   "width_pixels", 1,
			   NULL );
  
  // remove my status from the appbar
  gnome_appbar_pop(  this->m_world->g_appbar );  
}

void CEntity::GuiStatus( void )
{
  // describe my status on the appbar
  char buf[64];
  double x, y, th;
  this->GetGlobalPose( x, y, th );

  sprintf( buf, "Type: %d pose: X: %.2f Y: %.2f TH: %.2f",
	   this->stage_type, x, y, th );
	   
  
  //gnome_appbar_set_status(  this->m_world->g_appbar, buf );  

}

                      
#endif

