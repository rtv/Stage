/*
 *  STK2 : A GUI toolkit for robotics
 *  Copyright (C) 2001  Andrew Howard  ahoward@usc.edu
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
 * Desc: Rtk canvas functions
 * Author: Andrew Howard, Richard Vaughan
 * CVS: $Id: rtk_canvas.c,v 1.19 2005-09-11 21:13:26 rtv Exp $
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#define _GNU_SOURCE

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <gdk/gdkkeysyms.h>

#if HAVE_JPEGLIB_H
#include <jpeglib.h>
#endif

#define DEBUG
#include "rtk.h"
#include "rtkprivate.h"

// Declare some local functions
static gboolean stg_rtk_on_destroy(GtkWidget *widget, stg_rtk_canvas_t *canvas);
static void stg_rtk_on_configure(GtkWidget *widget, GdkEventConfigure *event, stg_rtk_canvas_t *canvas);
static void stg_rtk_on_expose(GtkWidget *widget, GdkEventExpose *event, stg_rtk_canvas_t *canvas);
static void stg_rtk_on_press(GtkWidget *widget, GdkEventButton *event, stg_rtk_canvas_t *canvas);
static void stg_rtk_on_motion(GtkWidget *widget, GdkEventMotion *event, stg_rtk_canvas_t *canvas);
static void stg_rtk_on_release(GtkWidget *widget, GdkEventButton *event, stg_rtk_canvas_t *canvas);
static void stg_rtk_on_key_press(GtkWidget *widget, GdkEventKey *event, stg_rtk_canvas_t *canvas);
static void stg_rtk_canvas_mouse(stg_rtk_canvas_t *canvas, int event, int button, int x, int y);

// Mouse modes 
enum {MOUSE_NONE, MOUSE_PAN, MOUSE_ZOOM, MOUSE_TRANS, MOUSE_ROT, MOUSE_SCALE};

// Mouse events
enum {EVENT_PRESS, EVENT_MOTION, EVENT_RELEASE};

// Convert from device to logical coords
#define LX(x) (canvas->ox + (+(x) - canvas->sizex / 2) * canvas->sx)
#define LY(y) (canvas->oy + (-(y) + canvas->sizey / 2) * canvas->sy)

// Convert from logical to device coords
#define DX(x) (canvas->sizex / 2 + ((x) - canvas->ox) / canvas->sx)
#define DY(y) (canvas->sizey / 2 - ((y) - canvas->oy) / canvas->sy)


gboolean    stest                  (GtkWidget *widget,
				   GdkEventConfigure *event,
				   gpointer user_data)
{
  printf( "EVENT\n" );
  return FALSE;
} 


// Create a canvas
stg_rtk_canvas_t *stg_rtk_canvas_create(stg_rtk_app_t *app)
{
  int l;
  stg_rtk_canvas_t *canvas;

  // Create canvas
  canvas = calloc(1, sizeof(stg_rtk_canvas_t));

  // Append canvas to linked list
  STK_LIST_APPEND(app->canvas, canvas);
    
  canvas->app = app;
  canvas->sizex = 0;
  canvas->sizey = 0;
  canvas->ox = 0.0;
  canvas->oy = 0.0;
  canvas->sx = 0.01;
  canvas->sy = 0.01;
  canvas->destroyed = FALSE;
  canvas->bg_dirty = TRUE;
  canvas->fg_dirty = TRUE;
  canvas->fg_dirty_region = stg_rtk_region_create();
  canvas->calc_deferred = 0;
  canvas->movemask = STK_MOVE_PAN | STK_MOVE_ZOOM;

  canvas->fig = NULL;
  canvas->layer_fig = NULL;

  // Initialise mouse handling
  canvas->zoom_fig = NULL;
  canvas->mouse_mode = MOUSE_NONE;
  canvas->mouse_over_fig = NULL;  
  canvas->mouse_selected_fig = NULL;
  canvas->mouse_selected_fig_last = NULL;

  
  // Create gtk drawing area
  canvas->canvas = gtk_drawing_area_new();
  //gtk_widget_set_size_request( canvas->canvas, 1000,1000 );

  //gtk_layout_put( GTK_LAYOUT(canvas->gcanvas), canvas->canvas, 0,0 );

  //  gtk_widget_show( canvas->canvas );
  //gtk_widget_show( canvas->gcanvas );
  

  //gtk_layout_put( GTK_LAYOUT(canvas->layout), GTK_WIDGET(hbox), 0,0 );
  //gtk_layout_put( GTK_LAYOUT(canvas->layout), canvas->canvas, 0,200 );
  //gtk_container_add( GTK_CONTAINER(canvas->layout), GTK_WIDGET(hbox) );
  //gtk_container_add( GTK_CONTAINER(canvas->layout), canvas->canvas );



  //gtk_box_pack_end(GTK_BOX(canvas->layout), scrolled_win, TRUE, TRUE, 0);

  canvas->bg_pixmap = NULL;
  canvas->fg_pixmap = NULL;
  canvas->gc = NULL;
  canvas->colormap = NULL;

  // Set default drawing attributes
  canvas->fontname = strdup("-adobe-helvetica-medium-r-*-*-*-120-*-*-*-*-*-*");
  canvas->font = gdk_font_load(canvas->fontname);
  canvas->bgcolor.red = 0xFFFF;
  canvas->bgcolor.green = 0xFFFF;
  canvas->bgcolor.blue = 0xFFFF;
  canvas->linewidth = 0;

  // Movie stuff
  //canvas->movie_context = NULL;

  // Connect gtk signal handlers
  g_signal_connect(GTK_OBJECT(canvas->canvas), "configure_event", 
		   GTK_SIGNAL_FUNC(stg_rtk_on_configure), canvas);
  g_signal_connect(GTK_OBJECT(canvas->canvas), "expose_event", 
		   GTK_SIGNAL_FUNC(stg_rtk_on_expose), canvas);
  g_signal_connect(GTK_OBJECT(canvas->canvas), "button_press_event", 
		   GTK_SIGNAL_FUNC(stg_rtk_on_press), canvas);
  g_signal_connect(GTK_OBJECT(canvas->canvas), "motion_notify_event", 
		   GTK_SIGNAL_FUNC(stg_rtk_on_motion), canvas);
  g_signal_connect(GTK_OBJECT(canvas->canvas), "button_release_event", 
		   GTK_SIGNAL_FUNC(stg_rtk_on_release), canvas);

  // this seems not to receive the arrow keys...
  //g_signal_connect_after(GTK_OBJECT(canvas->frame), "key-press-event", 
  //		 GTK_SIGNAL_FUNC(stg_rtk_on_key_press), canvas);
  
  // Set the event mask
  gtk_widget_set_events(canvas->canvas,
                        GDK_EXPOSURE_MASK |
                        GDK_BUTTON_PRESS_MASK |
                        GDK_BUTTON_RELEASE_MASK |
			GDK_KEY_PRESS_MASK |
                        GDK_POINTER_MOTION_MASK );

  // rtv - support for motion hints would be handy - todo?
  //GDK_POINTER_MOTION_HINT_MASK);

  // show all layers by default
  for( l=0; l<STK_CANVAS_LAYERS; l++ )
    canvas->layer_show[l] = 1;

  return canvas;
}

// Delete the canvas
void stg_rtk_canvas_destroy(stg_rtk_canvas_t *canvas)
{
  int count;

  // Get rid of any figures we still have
  count = 0;
  while (canvas->fig)
  {
    stg_rtk_fig_destroy(canvas->fig);
    count++;
  }
  if (count > 0)
    PRINT_WARN1("garbage collected %d figures", count);

  // Clear the dirty regions
  stg_rtk_region_destroy(canvas->fg_dirty_region);
  
  // Remove ourself from the linked list in the app
  STK_LIST_REMOVE(canvas->app->canvas, canvas);

  // Destroy the frame
  gtk_widget_hide(GTK_WIDGET(canvas->canvas));
  gtk_widget_destroy(GTK_WIDGET(canvas->canvas));
  
  // fontname was strdup()ed 
  if (canvas->fontname)
    free(canvas->fontname);

  // Free ourself
  free(canvas);

  return;
}

// See if the canvas has been closed
int stg_rtk_canvas_isclosed(stg_rtk_canvas_t *canvas)
{
  return canvas->destroyed;
}


// Set the canvas title
void stg_rtk_canvas_title(stg_rtk_canvas_t *canvas, const char *title)
{
  //gtk_window_set_title(GTK_WINDOW(canvas->frame), title);
}


// Set the size of a canvas
// (sizex, sizey) is the width and height of the canvas, in pixels.
void stg_rtk_canvas_size(stg_rtk_canvas_t *canvas, int sizex, int sizey)
{
  //gtk_window_set_default_size(GTK_WINDOW(canvas->frame), sizex, sizey);
  return;
}

// Get the canvas size.
// (sizex, sizey) is the width and height of the canvas, in pixels.
void stg_rtk_canvas_get_size(stg_rtk_canvas_t *canvas, int *sizex, int *sizey)
{
  *sizex = canvas->sizex;
  *sizey = canvas->sizey;
  return;
}

// Set the origin of a canvas
// (ox, oy) specifies the logical point that maps to the center of the
// canvas.
void stg_rtk_canvas_origin(stg_rtk_canvas_t *canvas, double ox, double oy)
{
  canvas->ox = ox;
  canvas->oy = oy;

  // Re-calculate all the figures.
  stg_rtk_canvas_calc(canvas);
}


// Get the origin of a canvas
// (ox, oy) specifies the logical point that maps to the center of the
// canvas.
void stg_rtk_canvas_get_origin(stg_rtk_canvas_t *canvas, double *ox, double *oy)
{
  *ox = canvas->ox;
  *oy = canvas->oy;
}


// Scale a canvas
// Sets the pixel width and height in logical units
void stg_rtk_canvas_scale(stg_rtk_canvas_t *canvas, double sx, double sy)
{
  canvas->sx = sx;
  canvas->sy = sy;
  
  // Re-calculate all the figures.
  stg_rtk_canvas_calc(canvas);
}


// Get the scale of the canvas
// (sx, sy) are the pixel with and height in logical units
void stg_rtk_canvas_get_scale(stg_rtk_canvas_t *canvas, double *sx, double *sy)
{
  *sx = canvas->sx;
  *sy = canvas->sy;
}


// Set the movement mask
// Set the mask to a bitwise combination of STK_MOVE_TRANS, STK_MOVE_SCALE.
// to enable user manipulation of the canvas.
void stg_rtk_canvas_movemask(stg_rtk_canvas_t *canvas, int mask)
{
  canvas->movemask = mask;
}


// Set the default font for text strokes
void stg_rtk_canvas_font(stg_rtk_canvas_t *canvas, const char *fontname)
{
  if (canvas->font)
  {
    gdk_font_unref(canvas->font);
    canvas->font = NULL;
  }

  if (canvas->fontname)
  {
    free(canvas->fontname);
    canvas->fontname = strdup(fontname);
  }

  // Load the default font
  if (canvas->font == NULL)
    canvas->font = gdk_font_load(canvas->fontname);

  // Text extents will have changed, so recalc everything.
  stg_rtk_canvas_calc(canvas);
}


// Set the canvas backround color
void stg_rtk_canvas_bgcolor(stg_rtk_canvas_t *canvas, double r, double g, double b)
{
  canvas->bgcolor.red = (int) (r * 0xFFFF);
  canvas->bgcolor.green = (int) (g * 0xFFFF);
  canvas->bgcolor.blue = (int) (b * 0xFFFF);
  return;
}


// Set the default line width.
void stg_rtk_canvas_linewidth(stg_rtk_canvas_t *canvas, int width)
{
  canvas->linewidth = width;
}

///
// rtv experimental feature
//

// the figure will be shown until stg_rtk_canvas_flash_update() is called
// [duration] times. if [kill] is non-zero, the fig will
// also be destroyed when its counter expires.
void stg_rtk_canvas_flash( stg_rtk_canvas_t* canvas, stg_rtk_fig_t* fig, int duration, 
		       int kill )
{
  stg_rtk_flasher_t* flasher = malloc( sizeof(stg_rtk_flasher_t) );
  
  // force the fig visible
  stg_rtk_fig_show( fig, 1 );
  
  flasher->fig = fig;
  flasher->duration = duration;
  flasher->kill = kill;
  
  STK_LIST_APPEND( canvas->flashers, flasher );
}

void stg_rtk_canvas_flash_update( stg_rtk_canvas_t* canvas )
{
  stg_rtk_flasher_t* flasher = canvas->flashers; 
  
  while( flasher != NULL )
    {
      //stg_rtk_fig_t* fig = flasher->fig;      
      flasher->duration--;
      
      // if it's time to flip, flip
      if( flasher->duration < 1 )
	{
	  stg_rtk_flasher_t* doomed = flasher;
	  
	  // force the fig invisible
	  if( doomed->kill )
	    stg_rtk_fig_and_descendents_destroy( doomed->fig );
	  else
	    stg_rtk_fig_show( doomed->fig, 0);
	  
	  flasher = flasher->next;	  
	  STK_LIST_REMOVE( canvas->flashers, doomed );
	  continue;
	}
      
      flasher = flasher->next;
    }
}

void stg_rtk_canvas_layer_show( stg_rtk_canvas_t* canvas, int layer, char show )
{
  canvas->layer_show[layer] = show;
  
  // invalidate the whole window 
  stg_rtk_canvas_calc( canvas );
} 

// end rtv experimental

// Re-calculate all the figures (private).
void stg_rtk_canvas_calc(stg_rtk_canvas_t *canvas)
{
  stg_rtk_fig_t *fig;
  
  // The whole window is dirty
  canvas->bg_dirty = TRUE;
  canvas->fg_dirty = TRUE;

  // Add the whole window to the dirty region
  stg_rtk_region_set_union_rect(canvas->fg_dirty_region, 0, 0, canvas->sizex, canvas->sizey);

  // Update all the figures
  for (fig = canvas->fig; fig != NULL; fig = fig->sibling_next)
    stg_rtk_fig_calc(fig);
 
  return;
}


// Render the figures in the canvas
void stg_rtk_canvas_render(stg_rtk_canvas_t *canvas)
{
  //static int c=0;  
  //if( c++ % 20 != 0 )
  //return;

  int bg_count;
  stg_rtk_fig_t *fig;
  GdkColor color;

  int rcount;
  GdkRectangle clipbox;

  if (canvas->destroyed)
    return;

  // If there were deferred computations, do them now.
  if (canvas->calc_deferred)
  {
    stg_rtk_canvas_calc(canvas);
    canvas->calc_deferred = 0;
  }
  
  // Set the canvas color
  gdk_color_alloc(canvas->colormap, &canvas->bgcolor);
  gdk_gc_set_foreground(canvas->gc, &canvas->bgcolor);
  
  // See if there is anything in the background.
  // TODO: optimize
  bg_count = 0;
  for (fig = canvas->layer_fig; fig != NULL; fig = fig->layer_next)
  {
    if (fig->layer < 0 && fig->show)
    {
      bg_count++;
      break;
    }
  }

  // Render the background
  if (canvas->bg_dirty)
  {
    // Clear background pixmap
    gdk_draw_rectangle(canvas->bg_pixmap, canvas->gc, TRUE,
                       0, 0, canvas->sizex, canvas->sizey);

    // Render all figures, in order of layer
    for (fig = canvas->layer_fig; fig != NULL; fig = fig->layer_next)
    {
      if (fig->layer < 0)
        stg_rtk_fig_render(fig);
    }

    // Copy background pixmap to foreground pixmap
    gdk_draw_pixmap(canvas->fg_pixmap, canvas->gc, canvas->bg_pixmap,
                    0, 0, 0, 0, canvas->sizex, canvas->sizey);

    // The entire forground needs redrawing
    stg_rtk_region_set_union_rect(canvas->fg_dirty_region,
                             0, 0, canvas->sizex, canvas->sizey);
  }

  // Render the foreground
  if (canvas->bg_dirty || canvas->fg_dirty)
  {
    // Clip drawing to the dirty region
    stg_rtk_region_get_brect(canvas->fg_dirty_region, &clipbox);
    gdk_gc_set_clip_rectangle(canvas->gc, &clipbox);

    // Copy background pixmap to foreground pixmap
    gdk_draw_pixmap(canvas->fg_pixmap, canvas->gc, canvas->bg_pixmap,
                    clipbox.x, clipbox.y, clipbox.x, clipbox.y,
                    clipbox.width, clipbox.height);
    rcount = 0;
    
    // Render all figures, in order of layer
    for (fig = canvas->layer_fig; fig != NULL; fig = fig->layer_next)
    {
      // Make sure the figure is in the foreground
      //if (fig->layer >= 0)
      if (fig->layer >= 0 && canvas->layer_show[fig->layer] > 0 )
      {
        // Only draw figures in the dirty region
        if (stg_rtk_region_test_intersect(canvas->fg_dirty_region, fig->region))
        {
          rcount++;
          stg_rtk_fig_render(fig);
        }
      }
    }

    // Now copy foreground pixmap to screen
    gdk_draw_pixmap(canvas->canvas->window, canvas->gc, canvas->fg_pixmap,
                    clipbox.x, clipbox.y, clipbox.x, clipbox.y,
                    clipbox.width, clipbox.height);
    gdk_gc_set_clip_rectangle(canvas->gc, NULL);
  }

  // Reset the dirty regions
  canvas->bg_dirty = FALSE;
  canvas->fg_dirty = FALSE;
  stg_rtk_region_set_empty(canvas->fg_dirty_region);

  gdk_colormap_free_colors(canvas->colormap, &canvas->bgcolor, 1);
}


// Export an image.
// [filename] is the name of the file to save.
// [format] is the image file format (STK_IMAGE_FORMAT_JPEG, STK_IMAGE_FORMAT_PPM).
void stg_rtk_canvas_export_image(stg_rtk_canvas_t *canvas, const char *filename, int format)
{
  GdkPixbuf* buf = gdk_pixbuf_get_from_drawable( NULL, 
						  canvas->fg_pixmap,
						  canvas->colormap,
						  0,0,0,0,
						  canvas->sizex,
						  canvas->sizey );
  
  switch( format )
    {
    case STK_IMAGE_FORMAT_JPEG:
      gdk_pixbuf_save( buf, filename, "jpeg", NULL,
		       "quality", "100", NULL );
      break;

    case STK_IMAGE_FORMAT_PPM:
      gdk_pixbuf_save( buf, filename, "ppm", NULL, NULL );
      break;

    case STK_IMAGE_FORMAT_PNG:
      gdk_pixbuf_save( buf, filename, "png", NULL, NULL );
      break;
    case STK_IMAGE_FORMAT_PNM:
      gdk_pixbuf_save( buf, filename, "pnm", NULL, NULL );
      break;

    default: 
      puts( "unrecognized image format" );
      break;
    }
      
}	
  
// Pixel tolerances for moving stuff
#define TOL_MOVE 15

// See if there is a moveable figure close to the given device point.
stg_rtk_fig_t *stg_rtk_canvas_pick_fig(stg_rtk_canvas_t *canvas, int x, int y)
{
  int maxlayer;
  stg_rtk_fig_t *fig, *maxfig;

  maxfig = NULL;
  maxlayer = INT_MIN;

  // TODO: optimize
  for (fig = canvas->layer_fig; fig != NULL; fig = fig->layer_next)
  {
    if (!fig->show)
      continue;
    if (fig->movemask == 0)
      continue;
    if (stg_rtk_fig_hittest(fig, x, y))
    {
      if (fig->layer >= maxlayer)
      {
        maxfig = fig;
        maxlayer = fig->layer;
      }
    }
  }
  return maxfig;
}

// Do mouse stuff
void stg_rtk_canvas_mouse(stg_rtk_canvas_t *canvas, int event, int button, int x, int y)
{
  double px, py, pa, rd, rl;
  stg_rtk_fig_t *fig;
    
  if (event == EVENT_PRESS)
  {        
    // See of there are any moveable figures at this point
    fig = stg_rtk_canvas_pick_fig(canvas, x, y);

    // If there are moveable figures...
    if (fig)
    {
      if (button == 1 && (fig->movemask & STK_MOVE_TRANS))
      {
        canvas->mouse_mode = MOUSE_TRANS;
        canvas->mouse_start_x = LX(x) - fig->dox;
        canvas->mouse_start_y = LY(y) - fig->doy;
        canvas->mouse_selected_fig = fig;
        stg_rtk_fig_dirty(fig);
        stg_rtk_fig_on_mouse(canvas->mouse_selected_fig, STK_EVENT_PRESS, canvas->mouse_mode);
      }
      else if (button == 3 && (fig->movemask & STK_MOVE_ROT))
      {
        canvas->mouse_mode = MOUSE_ROT;
        px = LX(x) - fig->dox;
        py = LY(y) - fig->doy;
        canvas->mouse_start_a = atan2(py, px) - fig->doa;
        canvas->mouse_selected_fig = fig;
        stg_rtk_fig_dirty(fig);
        stg_rtk_fig_on_mouse(canvas->mouse_selected_fig, STK_EVENT_PRESS, canvas->mouse_mode);
      }

      // rtv - fixed and reinstated scroll wheel support 1/7/03
      // rtv - handle the mouse scroll wheel for rotating objects 
      else if( button == 4 && (fig->movemask & STK_MOVE_ROT))
	{
	  stg_rtk_fig_dirty(fig);
	  stg_rtk_fig_origin_global(fig, fig->dox, fig->doy, fig->doa + 0.2 );
	  stg_rtk_fig_on_mouse(fig, STK_EVENT_PRESS, canvas->mouse_mode);
	  
	  return;
	}
      else if( button == 5 && (fig->movemask & STK_MOVE_ROT))
	{
	  stg_rtk_fig_dirty(fig);
	  stg_rtk_fig_origin_global(fig, fig->dox, fig->doy, fig->doa - 0.2 );
	  stg_rtk_fig_on_mouse(fig, STK_EVENT_PRESS, canvas->mouse_mode);
	  return;
	}
      
    }

    // Else translate and scale the canvas...
    else
    {
      if (button == 1 && (canvas->movemask & STK_MOVE_PAN))
      {
        // Store the logical coordinate of the start point
        canvas->mouse_mode = MOUSE_PAN;
        canvas->mouse_start_x = LX(x);
        canvas->mouse_start_y = LY(y);
      }
      else if (button == 3 && (canvas->movemask & STK_MOVE_ZOOM))
      {
	if( canvas->zoom_fig == NULL )
	  {
	    // Store the logical coordinate of the start point
	    canvas->mouse_mode = MOUSE_ZOOM;
	    canvas->mouse_start_x = LX(x);
	    canvas->mouse_start_y = LY(y);
	    
	    // Create a figure for showing the zoom
	    //assert(canvas->zoom_fig == NULL);
	   	    
	    canvas->zoom_fig = stg_rtk_fig_create(canvas, NULL, STK_CANVAS_LAYERS-1);
	    px = LX(canvas->sizex / 2);
	    py = LY(canvas->sizey / 2);
	    rl = 2 * sqrt((LX(x) - px) * (LX(x) - px) + (LY(y) - py) * (LY(y) - py));
	    stg_rtk_fig_ellipse(canvas->zoom_fig, px, py, 0, rl, rl, 0);
	  }
      }

      /* BROKEN
      // handle the mouse scroll wheel for cancas zooming
      else if (button == 4 && (canvas->movemask & STK_MOVE_ZOOM))
      {
        canvas->mouse_mode = MOUSE_ZOOM;
        canvas->sx *= 0.9;
        canvas->sy *= 0.9;
      }
      else if (button == 5 && (canvas->movemask & STK_MOVE_ZOOM))
      {
        canvas->mouse_mode = MOUSE_ZOOM;
        canvas->sx *= 1.1;
        canvas->sy *= 1.1;
      }
      */
    }
  }

  if (event == EVENT_MOTION)
  {            
    if (canvas->mouse_mode == MOUSE_TRANS)
    {
      // Translate the selected figure
      fig = canvas->mouse_selected_fig;
      px = LX(x) - canvas->mouse_start_x;
      py = LY(y) - canvas->mouse_start_y;
      stg_rtk_fig_dirty(fig);
      stg_rtk_fig_origin_global(fig, px, py, fig->doa);
      stg_rtk_fig_on_mouse(canvas->mouse_selected_fig, STK_EVENT_MOTION, canvas->mouse_mode);
    }
    else if (canvas->mouse_mode == MOUSE_ROT)
    {
      // Rotate the selected figure 
      fig = canvas->mouse_selected_fig;
      px = LX(x) - fig->dox;
      py = LY(y) - fig->doy;
      pa = atan2(py, px) - canvas->mouse_start_a;
      stg_rtk_fig_dirty(fig);
      stg_rtk_fig_origin_global(fig, fig->dox, fig->doy, pa);
      stg_rtk_fig_on_mouse(canvas->mouse_selected_fig, STK_EVENT_MOTION, canvas->mouse_mode);
    }
    else if (canvas->mouse_mode == MOUSE_PAN)
    {
      // Infer the translation that will map the current physical mouse
      // point to the original logical mouse point.        
      canvas->ox = canvas->mouse_start_x -
        (+x - canvas->sizex / 2) * canvas->sx;
      canvas->oy = canvas->mouse_start_y -
        (-y + canvas->sizey / 2) * canvas->sy;
    }
    else if (canvas->mouse_mode == MOUSE_ZOOM)
    {
      // Compute scale change that will map original logical point
      // onto circle intersecting current mouse point.
      px = x - canvas->sizex / 2;
      py = y - canvas->sizey / 2;
      rd = sqrt(px * px + py * py) + 1;
      px = canvas->mouse_start_x - canvas->ox;
      py = canvas->mouse_start_y - canvas->oy;
      rl = sqrt(px * px + py * py);
      canvas->sy = rl / rd * canvas->sy / canvas->sx;        
      canvas->sx = rl / rd;
    }
    else if (canvas->mouse_mode == MOUSE_NONE)
    {
      // See of there are any moveable figures at this point
      fig = stg_rtk_canvas_pick_fig(canvas, x, y);
      if (fig)
        stg_rtk_fig_dirty(fig);
      if (canvas->mouse_over_fig)
        stg_rtk_fig_dirty(canvas->mouse_over_fig);
      if (fig != canvas->mouse_over_fig)
      {
        if (canvas->mouse_over_fig)
          stg_rtk_fig_on_mouse(canvas->mouse_over_fig, STK_EVENT_MOUSE_NOT_OVER, canvas->mouse_mode);
        if (fig)
          stg_rtk_fig_on_mouse(fig, STK_EVENT_MOUSE_OVER, canvas->mouse_mode);
      }
      canvas->mouse_over_fig = fig;

      if( fig )
	canvas->mouse_selected_fig_last = fig;
    }

    if (canvas->mouse_mode == MOUSE_PAN || canvas->mouse_mode == MOUSE_ZOOM)
    {
      // Re-compute figures (deferred, will be done on next render).
      canvas->calc_deferred++;
    }
  }

  if (event == EVENT_RELEASE)
  {
    if (canvas->mouse_mode == MOUSE_PAN || canvas->mouse_mode == MOUSE_ZOOM)
    {
      // Re-compute figures (deferred, will be done on next render).
      canvas->calc_deferred++;

      // Delete zoom figure
      if (canvas->zoom_fig != NULL)
      {
        stg_rtk_fig_destroy(canvas->zoom_fig);
        canvas->zoom_fig = NULL;
      }
  
      // Reset mouse mode
      canvas->mouse_mode = MOUSE_NONE;
      canvas->mouse_selected_fig = NULL;
    }
    else
    {
      fig = canvas->mouse_selected_fig;
            
      // Reset mouse mode
      canvas->mouse_mode = MOUSE_NONE;
      canvas->mouse_selected_fig = NULL;
      
      // Do callbacks
      if (fig)
      {
        stg_rtk_fig_dirty(fig);
        stg_rtk_fig_on_mouse(fig, STK_EVENT_RELEASE, canvas->mouse_mode);
      }
    }
  }
}


// Handle destroy events
gboolean stg_rtk_on_destroy(GtkWidget *widget, stg_rtk_canvas_t *canvas)
{
  canvas->destroyed = TRUE;
  return FALSE;
}


// Process configure events
void stg_rtk_on_configure(GtkWidget *widget, GdkEventConfigure *event, stg_rtk_canvas_t *canvas)
{
  GdkColor color;
  
  //printf( "event width %d height %d\n", event->width, event->height );

  canvas->sizex = event->width;
  canvas->sizey = event->height;

  //printf( "canvas width %d height %d\n", canvas->sizex, canvas->sizey );

  if (canvas->gc == NULL)
    canvas->gc = gdk_gc_new(canvas->canvas->window);
  if (canvas->colormap == NULL)
    canvas->colormap = gdk_colormap_get_system();
  
  // Create offscreen pixmaps
  if (canvas->bg_pixmap != NULL)
    gdk_pixmap_unref(canvas->bg_pixmap);
  if (canvas->fg_pixmap != NULL)
    gdk_pixmap_unref(canvas->fg_pixmap);
  canvas->bg_pixmap = gdk_pixmap_new(canvas->canvas->window,
                                     canvas->sizex, canvas->sizey, -1);
  canvas->fg_pixmap = gdk_pixmap_new(canvas->canvas->window,
                                     canvas->sizex, canvas->sizey, -1);

  // Make sure we redraw with a white background
  gdk_color_white(canvas->colormap, &color);
  gdk_gc_set_foreground(canvas->gc, &color);

  //Clear pixmaps
  gdk_draw_rectangle(canvas->bg_pixmap, canvas->gc, TRUE,
                  0, 0, canvas->sizex, canvas->sizey);
  gdk_draw_rectangle(canvas->fg_pixmap, canvas->gc, TRUE,
                  0, 0, canvas->sizex, canvas->sizey);
    
  // Re-calculate all the figures since the coord transform has
  // changed.
  canvas->calc_deferred++;

}


// Process expose events
void stg_rtk_on_expose(GtkWidget *widget, GdkEventExpose *event, stg_rtk_canvas_t *canvas)
{
 if (canvas->fg_pixmap)
  {
    // Copy foreground pixmap to screen
    gdk_draw_pixmap(canvas->canvas->window, canvas->gc, canvas->fg_pixmap,
                    0, 0, 0, 0, canvas->sizex, canvas->sizey);
  }
}


// Process keyboard events
void stg_rtk_on_key_press(GtkWidget *widget, GdkEventKey *event, stg_rtk_canvas_t *canvas)
{
  double scale, dx, dy;
  //PRINT_DEBUG3("key: %d %d %s", event->keyval, event->state, gdk_keyval_name(event->keyval));

  dx = canvas->sizex * canvas->sx;
  dy = canvas->sizey * canvas->sy;
  scale = 1.5;

  stg_rtk_fig_t *fig = NULL;
  
  switch (event->keyval)
    {
    case GDK_Left:
      if (event->state == 0)
        stg_rtk_canvas_origin(canvas, canvas->ox - 0.05 * dx, canvas->oy);
      else if (event->state == 4)
        stg_rtk_canvas_origin(canvas, canvas->ox - 0.5 * dx, canvas->oy);
      break;
    case GDK_Right:
      if (event->state == 0)
        stg_rtk_canvas_origin(canvas, canvas->ox + 0.05 * dx, canvas->oy);
      else if (event->state == 4)
        stg_rtk_canvas_origin(canvas, canvas->ox + 0.5 * dx, canvas->oy);
      break;
    case GDK_Up:
      if (event->state == 0)
        stg_rtk_canvas_origin(canvas, canvas->ox, canvas->oy + 0.05 * dy);
      else if (event->state == 4)
        stg_rtk_canvas_origin(canvas, canvas->ox, canvas->oy + 0.5 * dy);
      else if (event->state == 1)
        stg_rtk_canvas_scale(canvas, canvas->sx / scale, canvas->sy / scale);
      break;
    case GDK_Down:
      if (event->state == 0)
        stg_rtk_canvas_origin(canvas, canvas->ox, canvas->oy - 0.05 * dy);
      else if (event->state == 4)
        stg_rtk_canvas_origin(canvas, canvas->ox, canvas->oy - 0.5 * dy);
      else if (event->state == 1)
        stg_rtk_canvas_scale(canvas, canvas->sx * scale, canvas->sy * scale);
      break;
      
      // TODO: add support for moving objects with the keyboard
    case GDK_w:
    case GDK_W:
      fig = canvas->mouse_selected_fig_last;

      if( fig )
	{
	  stg_rtk_fig_dirty(fig);
	  stg_rtk_fig_origin_global(fig, 
				    fig->dox + 0.04 * cos(fig->doa),
				    fig->doy + 0.04 * sin(fig->doa), 
				    fig->doa );
	}
      break;
      
    case GDK_s:
    case GDK_S:
      fig = canvas->mouse_selected_fig_last;

      if( fig )
	{
	  stg_rtk_fig_dirty(fig);
	  stg_rtk_fig_origin_global(fig, 
				    fig->dox - 0.04 * cos(fig->doa),
				    fig->doy - 0.04 * sin(fig->doa), 
				    fig->doa );
	}
      break;

    case GDK_a:
    case GDK_A:
      fig = canvas->mouse_selected_fig_last;

      if( fig )
	{
	  stg_rtk_fig_dirty(fig);
	  stg_rtk_fig_origin_global(fig, 
				    fig->dox,
				    fig->doy, 
				    fig->doa + 0.1 );
	}
      break;

    case GDK_d:
    case GDK_D:
      fig = canvas->mouse_selected_fig_last;

      if( fig )
	{
	  stg_rtk_fig_dirty(fig);
	  stg_rtk_fig_origin_global(fig, 
				    fig->dox,
				    fig->doy, 
				    fig->doa - 0.1 );
	}
      break;

      
  }
}


// Process mouse press events
void stg_rtk_on_press(GtkWidget *widget, GdkEventButton *event, stg_rtk_canvas_t *canvas)
{
  stg_rtk_canvas_mouse(canvas, EVENT_PRESS, event->button, event->x, event->y);
}


// Process mouse motion events
void stg_rtk_on_motion(GtkWidget *widget, GdkEventMotion *event, stg_rtk_canvas_t *canvas)
{
  stg_rtk_canvas_mouse(canvas, EVENT_MOTION, 0, event->x, event->y);
}


// Process mouse release events
void stg_rtk_on_release(GtkWidget *widget, GdkEventButton *event, stg_rtk_canvas_t *canvas)
{
  stg_rtk_canvas_mouse(canvas, EVENT_RELEASE, event->button, event->x, event->y);
}



