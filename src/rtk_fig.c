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
 * Desc: Stk fig functions
 * Author: Andrew Howard
 * Contributors: Richard Vaughan
 * CVS: $Id: rtk_fig.c,v 1.16 2005-09-11 21:13:26 rtv Exp $
 *
 * Notes:
 *   Some of this is a horrible hack, particular the xfig stuff.
 */

#define _GNU_SOURCE

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "rtk.h"
#include "rtkprivate.h"

//#include <libgnomecanvas/libgnomecanvas.h>

// Draw the selection
void stg_rtk_fig_render_selection(stg_rtk_fig_t *fig);

// Create a point stroke
void stg_rtk_fig_point_alloc(stg_rtk_fig_t *fig, double ox, double oy);

// Create a polygon
void stg_rtk_fig_polygon_alloc(stg_rtk_fig_t *fig, double ox, double oy, double oa,
                           int closed, int filled, int point_count, stg_rtk_point_t *points);

// Create a text stroke
void stg_rtk_fig_text_alloc(stg_rtk_fig_t *fig, double ox, double oy, double oa, const char *text);

// Create an image stroke.
void stg_rtk_fig_image_alloc(stg_rtk_fig_t *fig, double ox, double oy, double oa,
                         double scale, int width, int height, int bpp,
                         const void *image, const void *mask);


// Convert from local to global coords
#define GX(x, y) (fig->dox + (x) * fig->dsx * fig->dcos - (y) * fig->dsy * fig->dsin)
#define GY(x, y) (fig->doy + (x) * fig->dsx * fig->dsin + (y) * fig->dsy * fig->dcos)
#define GA(a) (fig->doa + (a))

// Convert from global to local coords
#define GX_TO_LX(x, y) (+((x) - fig->dox) / fig->dsx * fig->dcos + \
                    ((y) - fig->doy) / fig->dsy * fig->dsin)
#define GY_TO_LY(x, y) (-((x) - fig->dox) / fig->dsx * fig->dsin + \
                    ((y) - fig->doy) / fig->dsy * fig->dcos)

// Convert from global to device coords
#define GX_TO_DX(x) (fig->canvas->sizex / 2 + ((x) - fig->canvas->ox) / fig->canvas->sx)
#define GY_TO_DY(y) (fig->canvas->sizey / 2 - ((y) - fig->canvas->oy) / fig->canvas->sy)

// Convert from device to global coords
#define DX_TO_GX(x) ((+(x) - fig->canvas->sizex / 2) * fig->canvas->sx + fig->canvas->ox)
#define DY_TO_GY(y) ((-(y) + fig->canvas->sizey / 2) * fig->canvas->sy + fig->canvas->oy)

// Convert from global to paper coords
#define PX(x) (GX_TO_DX(x) * 1200 * 6 / fig->canvas->sizex)
#define PY(y) (GY_TO_DY(y) * 1200 * 6 / fig->canvas->sizex)
#define PA(a) (fig->canvas->sy < 0 ? -(a) : (a))

// Convert from local to global coords (to point structure)
#define LTOG(p, fx, fy) {p.x = GX(fx, fy); p.y = GY(fx, fy);}

// Convert from global to device coords (point structure)
#define GTOD(p, q) {p.x = GX_TO_DX(q.x); p.y = GY_TO_DY(q.y);}

// Convert from local to device coords (to point structure)
#define LTOD(p, fx, fy) {p.x = GX_TO_DX(GX(fx, fy)); p.y = GY_TO_DY(GY(fx, fy));}

// Convert from local to paper coords
#define GTOP(p, fx, fy) {p.x = PX(GX(fx, fy)); p.y = PY(GY(fx, fy));}

// Set a point
#define SETP(p, fx, fy) {p.x = fx; p.y = fy;}

// See if a point in paper coords is outside the crop region
#define CROP(p) (p.x < 0 || p.x >= 1200 * 6 || \
                 p.y < 0 || p.y >= 1200 * 6)


gboolean test_callback( void* data )
{
  puts( "TEST_CALLBACK" );
  
  return TRUE;
}


// Create a figure
stg_rtk_fig_t *stg_rtk_fig_create(stg_rtk_canvas_t *canvas, stg_rtk_fig_t *parent, int layer ) 
{
  stg_rtk_fig_t *fig;
  stg_rtk_fig_t *nfig;

  // Make sure the layer is valid
  if (layer <= -STK_CANVAS_LAYERS || layer >= STK_CANVAS_LAYERS)
    return NULL;

  // Create fig
  fig = calloc(1,sizeof(stg_rtk_fig_t));

  fig->canvas = canvas;
  fig->parent = parent;
  fig->layer = layer;
  fig->show = TRUE;
  fig->region = stg_rtk_region_create();
  fig->movemask = 0;
  fig->ox = fig->oy = fig->oa = 0.0;
  fig->cos = 1.0;
  fig->sin = 0.0;
  fig->sx = 1.0;
  fig->sy = 1.0;
  fig->stroke_size = 1024;
  fig->stroke_count = 0;
  fig->strokes = malloc(fig->stroke_size * sizeof(fig->strokes[0]));
  fig->dc_color.red = 0;
  fig->dc_color.green = 0;
  fig->dc_color.blue = 0;
  //fig->dc_xfig_color = 0;
  fig->dc_linewidth = 1;


  // If the figure is parentless, add it to the list of canvas
  // figures.  Otherwise, if the figure has a parent, at it to the
  // parent's list
  if (parent == NULL)
  {
    STK_LIST_APPENDX(canvas->fig, sibling, fig);
  }
  else
  {
    STK_LIST_APPENDX(parent->child, sibling, fig);
  }

  // Insert figure into layer list.  Figures appear in list in
  // increasing layer order.
  for (nfig = canvas->layer_fig; nfig != NULL; nfig = nfig->layer_next)
  {
    if (layer < nfig->layer)
    {
      fig->layer_prev = nfig->layer_prev;
      fig->layer_next = nfig;
      if (nfig->layer_prev)
        nfig->layer_prev->layer_next = fig;
      else
        canvas->layer_fig = fig;
      nfig->layer_prev = fig;
      break;
    }
  }
  if (nfig == NULL)
  {
    STK_LIST_APPENDX(canvas->layer_fig, layer, fig);
  }

  // Determine global coords for fig
  stg_rtk_fig_calc(fig);
  
  return fig;
}

// Create a figure, setting the user data (rtv)
stg_rtk_fig_t *stg_rtk_fig_create_ex(stg_rtk_canvas_t *canvas, stg_rtk_fig_t *parent, 
			     int layer, void* userdata )
{
  stg_rtk_fig_t* fig = stg_rtk_fig_create(canvas, parent, layer );
  fig->userdata = userdata;
  return fig;
}

// Destroy a figure
void stg_rtk_fig_destroy(stg_rtk_fig_t *fig)
{
  //printf( "destroying fig %p\n", fig );
  

  // remove any glib sources that might access this figure (such as
  // a blink timer) - rtv
  while( g_source_remove_by_user_data( fig ) )
    {} // empty loop  - the call is repeated until it returns FALSE.

  // Remove from the list of canvas figures, or from the parent's list
  // of figures.
  if (fig->parent == NULL)
  {
    STK_LIST_REMOVEX(fig->canvas->fig, sibling, fig);
  }
  else
  {
    STK_LIST_REMOVEX(fig->parent->child, sibling, fig);
  }

  // Remove from the layer list.
  STK_LIST_REMOVEX(fig->canvas->layer_fig, layer, fig);

    // Free the strokes
  stg_rtk_fig_clear(fig);
  free(fig->strokes);
  
  // Clear the dirty regions
  stg_rtk_region_destroy(fig->region);
  
  free(fig);
}

// Recursively free a whole tree of figures (rtv)
void stg_rtk_fig_and_descendents_destroy( stg_rtk_fig_t* fig )
{
  while( fig->child )
    stg_rtk_fig_and_descendents_destroy( fig->child );

  stg_rtk_fig_destroy( fig );
}


// Set the mouse event callback function.
void stg_rtk_fig_add_mouse_handler(stg_rtk_fig_t *fig, stg_rtk_mouse_fn_t callback)
{
  // TODO: have a list of callbacks
  fig->mouse_fn = callback;
  return;
}


// Unset the mouse event callback function.
void stg_rtk_fig_remove_mouse_handler(stg_rtk_fig_t *fig, stg_rtk_mouse_fn_t callback)
{
  // TODO: have a list of callbacks
  fig->mouse_fn = NULL;
  return;
}

// Clear all strokes from the figure
void stg_rtk_fig_clear(stg_rtk_fig_t *fig)
{
  if( fig == NULL )
    return;

  int i;
  stg_rtk_stroke_t *stroke;

  // Add the old region to the canvas dirty region.
  stg_rtk_fig_dirty(fig);

  // Free all the strokes
  for (i = 0; i < fig->stroke_count; i++)
  {
    stroke = fig->strokes[i];
    if (stroke->freefn)
      (*stroke->freefn) (fig, stroke);
  }
  fig->stroke_count = 0;

  // Reset the figure region
  stg_rtk_region_set_empty(fig->region);  
}

// Show or hide the figure
void stg_rtk_fig_show(stg_rtk_fig_t *fig, int show)
{
  if (show != fig->show)
  {
    fig->show = show;
    stg_rtk_fig_calc(fig);
  }
}

// Set the movement mask
void stg_rtk_fig_movemask(stg_rtk_fig_t *fig, int mask)
{
  fig->movemask = mask;
}


// See if the mouse is over this figure
int stg_rtk_fig_mouse_over(stg_rtk_fig_t *fig)
{
  if (fig->canvas->mouse_over_fig == fig)
    return TRUE;
  return FALSE;
}


// See if the figure has been selected
int stg_rtk_fig_mouse_selected(stg_rtk_fig_t *fig)
{
  if (fig->canvas->mouse_selected_fig == fig)
    return TRUE;
  return FALSE;
}


// Change the origin of a figure
// Coords are relative to parent
void stg_rtk_fig_origin(stg_rtk_fig_t *fig, double ox, double oy, double oa)
{    
  if (fig->ox != ox || fig->oy != oy || fig->oa != oa)
  {
    // Set coords relative to parent
    fig->ox = ox;
    fig->oy = oy;
    fig->oa = oa;
    fig->cos = cos(oa);
    fig->sin = sin(oa);

    // Determine global coords for fig
    stg_rtk_fig_calc(fig);
  }
}


// Change the origin of a figure
// Coords are global
void stg_rtk_fig_origin_global(stg_rtk_fig_t *fig, double ox, double oy, double oa)
{    
  if (fig->parent)
  {
    // Set coords relative to parent
    fig->ox = +(ox - fig->parent->dox) / fig->parent->dsx * fig->parent->dcos
      + (oy - fig->parent->doy) / fig->parent->dsy * fig->parent->dsin;
    fig->oy = -(ox - fig->parent->dox) / fig->parent->dsx * fig->parent->dsin
      + (oy - fig->parent->doy) / fig->parent->dsy * fig->parent->dcos;
    fig->oa = oa - fig->parent->doa;
    fig->cos = cos(fig->oa);
    fig->sin = sin(fig->oa);
  }
  else
  {
    // Top level figure -- use global coords directly
    fig->ox = ox;
    fig->oy = oy;
    fig->oa = oa;
    fig->cos = cos(fig->oa);
    fig->sin = sin(fig->oa);

  }

  // Determine global coords for fig
  stg_rtk_fig_calc(fig);  
  return;
}


// Get the current figure origin
void stg_rtk_fig_get_origin(stg_rtk_fig_t *fig, double *ox, double *oy, double *oa)
{
  *ox = fig->ox;
  *oy = fig->oy;
  *oa = fig->oa;
}


// Change the scale of a figure
void stg_rtk_fig_scale(stg_rtk_fig_t *fig, double scale)
{
  // Set scale
  fig->sy = scale * fig->sy / fig->sx;
  fig->sx = scale;

  // Recompute global coords
  // (for child figures in particular)
  stg_rtk_fig_calc(fig);
}


// Set the color
void stg_rtk_fig_color(stg_rtk_fig_t *fig, double r, double g, double b)
{ 
  fig->dc_color.red = (int) (r * 0xFFFF);
  fig->dc_color.green = (int) (g * 0xFFFF);
  fig->dc_color.blue = (int) (b * 0xFFFF);
}


// Set the color for strokes.
// Color is specified as an RGB32 value (8 bits per color).
void stg_rtk_fig_color_rgb32(stg_rtk_fig_t *fig, int color)
{
  fig->dc_color.red = (((color >> 16) & 0xFF) << 8);
  fig->dc_color.green = (((color >> 8) & 0xFF) << 8);
  fig->dc_color.blue = ((color & 0xFF) << 8);
}


 // Set the color for strokes.  Color is specified as an xfig color.
/*void stg_rtk_fig_color_xfig(stg_rtk_fig_t *fig, int color)
{
  fig->dc_xfig_color = color;
}
*/


// Set the line width.
void stg_rtk_fig_linewidth(stg_rtk_fig_t *fig, int width)
{
  fig->dc_linewidth = width;
}


// Mark the figure as dirty
void stg_rtk_fig_dirty(stg_rtk_fig_t *fig)
{
  if (fig->layer < 0)
  {
    fig->canvas->bg_dirty = TRUE;
  }
  else
  {
    fig->canvas->fg_dirty = TRUE;
    stg_rtk_region_set_union(fig->canvas->fg_dirty_region, fig->region);
  }
  return;
}


// Recalculate global coords for figure
void stg_rtk_fig_calc(stg_rtk_fig_t *fig)
{
  int i;
  stg_rtk_fig_t *child;
  stg_rtk_stroke_t *stroke;

  if (fig->parent)
  {        
    fig->dox = fig->parent->dox
      + fig->ox * fig->parent->dsx * fig->parent->dcos
      - fig->oy * fig->parent->dsy * fig->parent->dsin;
    fig->doy = fig->parent->doy
      + fig->ox * fig->parent->dsx * fig->parent->dsin
      + fig->oy * fig->parent->dsy * fig->parent->dcos;
    fig->doa = fig->parent->doa + fig->oa;
    fig->dcos = cos(fig->doa);
    fig->dsin = sin(fig->doa);
    fig->dsx = fig->parent->dsx * fig->sx;
    fig->dsy = fig->parent->dsy * fig->sy;
  }
  else
  {
    fig->dox = fig->ox;
    fig->doy = fig->oy;
    fig->doa = fig->oa;
    fig->dcos = fig->cos;
    fig->dsin = fig->sin;
    fig->dsx = fig->sx;
    fig->dsy = fig->sy;
  }
  
  // Add the old region to the canvas dirty region.
  stg_rtk_region_set_union(fig->canvas->fg_dirty_region, fig->region);

  // Reset the figure region
  stg_rtk_region_set_empty(fig->region);

  // Reset the figure bounding box.
  fig->min_x = +DBL_MAX/2;
  fig->min_y = +DBL_MAX/2;  
  fig->max_x = -DBL_MAX/2;
  fig->max_y = -DBL_MAX/2;

  // Recalculate strokes
  for (i = 0; i < fig->stroke_count; i++)
  {
    stroke = fig->strokes[i];
    (*stroke->calcfn) (fig, stroke);
  }

  // Add the new region to the canvas dirty region.
  stg_rtk_region_set_union(fig->canvas->fg_dirty_region, fig->region);

  // Update all our children.
  for (child = fig->child; child != NULL; child = child->sibling_next)
  {
    assert(child->parent == fig);
    stg_rtk_fig_calc(child);
  }

  // The modified fig has not been rendered
  stg_rtk_fig_dirty(fig);

  return;
}


// Render the figure
void stg_rtk_fig_render(stg_rtk_fig_t *fig)
{
  int i;
  stg_rtk_stroke_t *stroke;
  GdkDrawable *drawable;
  GdkGC *gc;
  GdkColormap *colormap;
  GdkColor color;
  int linewidth;

  // Dont render if we're not supposed to show it
  if (!fig->show)
    return;

  drawable = (fig->layer < 0 ? fig->canvas->bg_pixmap : fig->canvas->fg_pixmap);
  gc = fig->canvas->gc;
  colormap = fig->canvas->colormap;

  color.red = 0x0;
  color.green = 0x0;
  color.blue = 0x0;
  gdk_color_alloc(colormap, &color);
  gdk_gc_set_foreground(gc, &color);
  gdk_gc_set_function(gc, GDK_COPY);

  // Set the default line properties.
  linewidth = fig->canvas->linewidth;
  gdk_gc_set_line_attributes(gc, linewidth,
                             GDK_LINE_SOLID, GDK_CAP_NOT_LAST, GDK_JOIN_MITER);

  for (i = 0; i < fig->stroke_count; i++)
  {
    stroke = fig->strokes[i];

    // Set the color
    if (stroke->color.red != color.red ||
        stroke->color.green != color.green ||
        stroke->color.blue != color.blue)
    {
      gdk_colormap_free_colors(colormap, &color, 1);
      color.red = stroke->color.red;
      color.green = stroke->color.green;
      color.blue = stroke->color.blue;
      gdk_color_alloc(colormap, &color);
      gdk_gc_set_foreground(gc, &color);
    }

    // Set the line attributes
    if (stroke->linewidth != linewidth)
    {
      linewidth = stroke->linewidth;
      gdk_gc_set_line_attributes(gc, linewidth,
                                 GDK_LINE_SOLID, GDK_CAP_NOT_LAST, GDK_JOIN_MITER);
    }

    // Draw the stroke.
    (*stroke->drawfn) (fig, stroke); 
  }

  // Free any colors we have allocated
  gdk_colormap_free_colors(colormap, &color, 1);

  // Draw the selection box
  if (stg_rtk_fig_mouse_over(fig) || stg_rtk_fig_mouse_selected(fig))
    stg_rtk_fig_render_selection(fig);
    

  return;
}


// Draw the selection
void stg_rtk_fig_render_selection(stg_rtk_fig_t *fig)
{
  GdkDrawable *drawable;
  GdkGC *gc;
  GdkColormap *colormap;
  GdkColor color;
  GdkPoint points[4];

  drawable = (fig->layer < 0 ? fig->canvas->bg_pixmap : fig->canvas->fg_pixmap);
  gc = fig->canvas->gc;
  colormap = fig->canvas->colormap;
  
  color.red = 0x0000;
  color.green = 0x8000;
  color.blue = 0x8000;
  gdk_color_alloc(colormap, &color);
  gdk_gc_set_foreground(gc, &color);
  gdk_gc_set_function(gc, GDK_XOR);
  gdk_gc_set_line_attributes(gc, 4,
                             GDK_LINE_ON_OFF_DASH, GDK_CAP_NOT_LAST, GDK_JOIN_MITER);
                             //GDK_LINE_SOLID, GDK_CAP_NOT_LAST, GDK_JOIN_MITER);

  // Draw the bounding box
  LTOD(points[0], fig->min_x, fig->max_y);
  LTOD(points[1], fig->max_x, fig->max_y);
  LTOD(points[2], fig->max_x, fig->min_y);
  LTOD(points[3], fig->min_x, fig->min_y);
  gdk_draw_polygon(drawable, fig->canvas->gc, FALSE, points, 4);

  // Draw cross-hairs
  /* This doesnt really work, since the figure origin may
    lie outside the figure bounding box.
  LTOD(points[0], fig->min_x, 0);
  LTOD(points[1], fig->max_x, 0);
  LTOD(points[2], 0, fig->min_y);
  LTOD(points[3], 0, fig->max_y);
  gdk_draw_lines(drawable, fig->canvas->gc, points + 0, 2);
  gdk_draw_lines(drawable, fig->canvas->gc, points + 2, 2);
  */

  // Free any colors we have allocated
  gdk_colormap_free_colors(colormap, &color, 1);
  gdk_gc_set_function(gc, GDK_COPY);
  return;
}

/*
// Render the figure to xfig
void stg_rtk_fig_render_xfig(stg_rtk_fig_t *fig)
{
  int i;
  stg_rtk_stroke_t *stroke;

  for (i = 0; i < fig->stroke_count; i++)
  {
    stroke = fig->strokes[i];
    if (stroke->xfigfn)
      (*stroke->xfigfn)(fig, stroke);
  }
}
*/


// Test to see if the given device point lies within the figure
int stg_rtk_fig_hittest(stg_rtk_fig_t *fig, int dx, int dy)
{
  double gx, gy, lx, ly;
  
  // Compute local coords of device point
  gx = DX_TO_GX(dx);
  gy = DY_TO_GY(dy);
  lx = GX_TO_LX(gx, gy);
  ly = GY_TO_LY(gx, gy);

  //printf("%.3f %.3f -> %.3f %.3f -> %.3f %.3f\n",
  //       gx, gy, lx, ly, GX(lx, ly), GY(lx, ly));
  //printf("%d %d %f %f %.3f %.3f | %.3f %.3f %.3f %.3f\n",
  //       dx, dy, gx, gy, lx, ly, fig->min_x, fig->min_y, fig->max_x, fig->max_y);

  if (lx < fig->min_x || lx > fig->max_x)
    return 0;
  if (ly < fig->min_y || ly > fig->max_y)
    return 0;
  
  return 1;
}


// Process mouse events
void stg_rtk_fig_on_mouse(stg_rtk_fig_t *fig, int event, int mode)
{
  if (fig->mouse_fn)
    (*fig->mouse_fn) (fig, event, mode);
  
  return;
}


/***************************************************************************
 * High-level stoke functions
 **************************************************************************/


// Create a point
void stg_rtk_fig_point(stg_rtk_fig_t *fig, double ox, double oy)
{
  stg_rtk_fig_point_alloc(fig, ox, oy);
}


// Create a line
void stg_rtk_fig_line(stg_rtk_fig_t *fig, double ax, double ay, double bx, double by)
{
  stg_rtk_point_t points[2];

  points[0].x = ax;
  points[0].y = ay;
  points[1].x = bx;
  points[1].y = by;

  stg_rtk_fig_polygon_alloc(fig, 0, 0, 0, 0, 0, 2, points);
  return;
}


// Draw a line centered on the given point.
void stg_rtk_fig_line_ex(stg_rtk_fig_t *fig, double ox, double oy, double oa, double size)
{
  double ax, ay, bx, by;

  ax = ox + size / 2 * cos(oa + M_PI);
  ay = oy + size / 2 * sin(oa + M_PI);
  bx = ox + size / 2 * cos(oa);
  by = oy + size / 2 * sin(oa);
  
  stg_rtk_fig_line(fig, ax, ay, bx, by);
  return;
}


// create a fancy arrow that can be filled
void stg_rtk_fig_arrow_fancy(stg_rtk_fig_t *fig, 
			     double ox, double oy, double oa,
			     double len, double head, double thickness, 
			     int filled )
{
  stg_rtk_point_t points[7];
  
  double t = thickness/2.0;

  points[0].x = 0;
  points[0].y = t;
  points[1].x = len - head;
  points[1].y = t;
  points[2].x = len - head;
  points[2].y = head * sin(0.8*M_PI);
  points[3].x = len;
  points[3].y = 0;
  points[4].x = len - head;
  points[4].y = head * -sin(0.8*M_PI);
  points[5].x = len - head;
  points[5].y = -t;
  points[6].x = 0;
  points[6].y = -t;

  stg_rtk_fig_polygon_alloc(fig, ox, oy, oa, 1, filled, 7, points);
  return;
}

// Create an arrow
void stg_rtk_fig_arrow(stg_rtk_fig_t *fig, double ox, double oy, double oa,
                   double len, double head)
{
  stg_rtk_point_t points[5];
  
  points[0].x = 0;
  points[0].y = 0;
  points[1].x = len;
  points[1].y = 0;
  points[2].x = len + head * cos(0.80 * M_PI);
  points[2].y = 0 + head * sin(0.80 * M_PI);
  points[3].x = len + head * cos(-0.80 * M_PI);
  points[3].y = 0 + head * sin(-0.80 * M_PI);
  points[4].x = len;
  points[4].y = 0;

  stg_rtk_fig_polygon_alloc(fig, ox, oy, oa, 0, 0, 5, points);
  return;
}


// Create an arrow
void stg_rtk_fig_arrow_ex(stg_rtk_fig_t *fig, double ax, double ay,
                      double bx, double by, double head)
{
  double dx, dy, oa, len;

  dx = bx - ax;
  dy = by - ay;
  oa = atan2(dy, dx);
  len = sqrt((dx * dx) + (dy * dy));

  stg_rtk_fig_arrow(fig, ax, ay, oa, len, head);
  return;
}


// Draw a rectangle.
void stg_rtk_fig_rectangle(stg_rtk_fig_t *fig, double ox, double oy, double oa,
                       double sx, double sy, int filled)
{
  stg_rtk_point_t points[4];

  points[0].x = -sx / 2;
  points[0].y = -sy / 2;
  points[1].x = +sx / 2;
  points[1].y = -sy / 2;
  points[2].x = +sx / 2;
  points[2].y = +sy / 2;
  points[3].x = -sx / 2;
  points[3].y = +sy / 2;

  stg_rtk_fig_polygon_alloc(fig, ox, oy, oa, 1, filled, 4, points);
  return;
}


// Create an ellipse
void stg_rtk_fig_ellipse(stg_rtk_fig_t *fig, double ox, double oy, double oa,
                     double sx, double sy, int filled)
{
  int i;
  stg_rtk_point_t points[32];
  
  for (i = 0; i < 32; i++)
  {
    points[i].x = sx / 2 * cos(i * M_PI / 16);
    points[i].y = sy / 2 * sin(i * M_PI / 16);
  }

  stg_rtk_fig_polygon_alloc(fig, ox, oy, oa, 1, filled, 32, points);
  return;
}

// Create an arc on an ellipse
void stg_rtk_fig_ellipse_arc( stg_rtk_fig_t *fig, double ox, double oy, double oa,
			  double sx, double sy, double min_th, double max_th)
{
  int i;
  stg_rtk_point_t points[33];
  
  double dth = (max_th - min_th) / 32;

  for (i = 0; i < 33; i++)
  {
    points[i].x = sx/2.0 * cos( min_th + i * dth );
    points[i].y = sy/2.0 * sin( min_th + i * dth );
  }

  stg_rtk_fig_polygon_alloc(fig, ox, oy, oa, 0, 0, 33, points);
  return;
}


// Create a polygon
void stg_rtk_fig_polygon(stg_rtk_fig_t *fig, double ox, double oy, double oa,
			 int point_count, double points[][2], int filled)
{
  int i;
  stg_rtk_point_t *npoints;

  npoints = malloc(point_count * sizeof(stg_rtk_point_t));
  
  //printf( "created a polygon of %d points: ", point_count );

  for (i = 0; i < point_count; i++)
  {
    npoints[i].x = points[i][0];
    npoints[i].y = points[i][1];

    //printf( "(%.2f,%.2f) ", npoints[i].x, npoints[i].y );
  }

  //

  stg_rtk_fig_polygon_alloc(fig, ox, oy, oa, 1, filled, point_count, npoints);
  free(npoints);
  
  
  return;
}


// Draw a grid.  Grid is centered on (ox, oy) with size (dx, dy) with
// spacing (sp).
void stg_rtk_fig_grid(stg_rtk_fig_t *fig, double ox, double oy,
                  double dx, double dy, double sp)
{
  int i, nx, ny;

  nx = (int) ceil((dx/2.0) / sp);
  ny = (int) ceil((dy/2.0) / sp);

  if( nx == 0 ) nx = 1.0;
  if( ny == 0 ) ny = 1.0;


  // draw the bounding box first
  stg_rtk_fig_rectangle( fig, ox,oy,0, dx, dy, 0 );

  char str[64];

  for (i = -nx+1; i < nx; i++)
  {
    //stg_rtk_fig_line(fig, ox - dx/2 + i * sp, oy - dy/2,
    //           ox - dx/2 + i * sp, oy - dy/2 + ny * sp);

    stg_rtk_fig_line( fig, 
		  ox + i * sp, 
		  oy - dy/2,
                  ox + i * sp,
		  oy + dy/2 );

    //stg_rtk_fig_line( fig, 
    //	  ox - i * sp, 
    //	  oy - dy/2,
    //            ox - i * sp,
    //	  oy + dy/2 );

    
    snprintf( str, 64, "%d", (int)i );
    stg_rtk_fig_text( fig, -0.2 + (ox + i * sp), -0.2 , 0, str );
  }

  for (i = -ny+1; i < ny; i++)
    {
      stg_rtk_fig_line( fig, 
		    ox - dx/2, 
		    oy + i * sp,
		    ox + dx/2,
		    oy + i * sp );
      
      snprintf( str, 64, "%d", (int)i );
      stg_rtk_fig_text( fig, -0.2, -0.2 + (oy + i * sp) , 0, str );
    }
  
  // draw the axis origin lines
  //stg_rtk_fig_color_rgb32( fig, 0 );
  //stg_rtk_fig_line( fig, ox-dx/2, 0, ox+dx/2, 0 );
  //stg_rtk_fig_line( fig, 0, oy-dy/2, 0, oy+dy/2 );

  return;
}


// Create a text stroke
void stg_rtk_fig_text(stg_rtk_fig_t *fig, double ox, double oy, double oa, const char *text)
{
  stg_rtk_fig_text_alloc(fig, ox, oy, oa, text);
  return;
}


// Create an image stroke.
void stg_rtk_fig_image(stg_rtk_fig_t *fig, double ox, double oy, double oa,
                   double scale, int width, int height, int bpp, void *image, void *mask)
{
  stg_rtk_fig_image_alloc(fig, ox, oy, oa, scale, width, height, bpp, image, mask);
  return;
}



/***************************************************************************
 * Generic initialization for all strokes
 **************************************************************************/

// Add a stroke to the figure.
void stg_rtk_fig_stroke_add(stg_rtk_fig_t *fig, stg_rtk_stroke_t *stroke)
{
  // Increase size of stroke list if necessary
  if (fig->stroke_count == fig->stroke_size)
  {
    fig->stroke_size *= 2;
    fig->strokes = realloc(fig->strokes, fig->stroke_size * sizeof(fig->strokes[0]));
  }

  fig->strokes[fig->stroke_count++] = stroke;

  stroke->color = fig->dc_color;
  //stroke->xfig_color = fig->dc_xfig_color;
  stroke->linewidth = fig->dc_linewidth;
  stroke->drawfn = NULL;
  //stroke->xfigfn = NULL;
  stroke->calcfn = NULL;
  stroke->freefn = NULL;

  return;
}


/***************************************************************************
 * Point stroke
 **************************************************************************/


// Function prototypes
void stg_rtk_fig_point_free(stg_rtk_fig_t *fig, stg_rtk_point_stroke_t *data);
void stg_rtk_fig_point_calc(stg_rtk_fig_t *fig, stg_rtk_point_stroke_t *data);
void stg_rtk_fig_point_draw(stg_rtk_fig_t *fig, stg_rtk_point_stroke_t *data);
//void stg_rtk_fig_point_xfig(stg_rtk_fig_t *fig, stg_rtk_point_stroke_t *data);


// Create a point stroke
void stg_rtk_fig_point_alloc(stg_rtk_fig_t *fig, double ox, double oy)
{
  stg_rtk_point_stroke_t *data;

  data = calloc(1, sizeof(stg_rtk_point_stroke_t));
  stg_rtk_fig_stroke_add(fig, (stg_rtk_stroke_t*) data);
  data->stroke.freefn = (stg_rtk_stroke_fn_t) stg_rtk_fig_point_free;
  data->stroke.calcfn = (stg_rtk_stroke_fn_t) stg_rtk_fig_point_calc;
  data->stroke.drawfn = (stg_rtk_stroke_fn_t) stg_rtk_fig_point_draw;
  //  data->stroke.xfigfn = (stg_rtk_stroke_fn_t) stg_rtk_fig_point_xfig;

  data->ox = ox;
  data->oy = oy;
  (*data->stroke.calcfn) (fig, (stg_rtk_stroke_t*) data);

  // This will make sure the new stroke gets drawn
  stg_rtk_fig_dirty(fig);

  return;
}


void stg_rtk_fig_point_free(stg_rtk_fig_t *fig, stg_rtk_point_stroke_t *data)
{
  free(data);
  return;
}


// Update a point
void stg_rtk_fig_point_calc(stg_rtk_fig_t *fig, stg_rtk_point_stroke_t *data)
{
  LTOD(data->point, data->ox, data->oy);

  // Update the figure's bounding region.
  stg_rtk_region_set_union_rect(fig->region, data->point.x - 1, data->point.y - 1,
                            data->point.x + 1, data->point.y + 1);
  
  return;
}


// Render a point
void stg_rtk_fig_point_draw(stg_rtk_fig_t *fig, stg_rtk_point_stroke_t *data)
{
  GdkDrawable *drawable;    

  drawable = (fig->layer < 0 ? fig->canvas->bg_pixmap : fig->canvas->fg_pixmap);
  gdk_draw_point(drawable, fig->canvas->gc, data->point.x, data->point.y);
  return;
}

/*
// Render stroke to xfig
void stg_rtk_fig_point_xfig(stg_rtk_fig_t *fig, stg_rtk_point_stroke_t *data)
{
  // TODO
  return;
}
*/


/***************************************************************************
 * Polygon/polyline stroke
 **************************************************************************/


// Function prototypes
void stg_rtk_fig_polygon_free(stg_rtk_fig_t *fig, stg_rtk_polygon_stroke_t *data);
void stg_rtk_fig_polygon_calc(stg_rtk_fig_t *fig, stg_rtk_polygon_stroke_t *data);
void stg_rtk_fig_polygon_draw(stg_rtk_fig_t *fig, stg_rtk_polygon_stroke_t *data);
//void stg_rtk_fig_polygon_xfig(stg_rtk_fig_t *fig, stg_rtk_polygon_stroke_t *data);


// Create a polygon
void stg_rtk_fig_polygon_alloc(stg_rtk_fig_t *fig,
                           double ox, double oy, double oa,
                           int closed, int filled,
                           int point_count, stg_rtk_point_t *points)
{
  stg_rtk_polygon_stroke_t *data;
  int i;

  data = calloc(1, sizeof(stg_rtk_polygon_stroke_t));
  stg_rtk_fig_stroke_add(fig, (stg_rtk_stroke_t*) data);
  data->stroke.freefn = (stg_rtk_stroke_fn_t) stg_rtk_fig_polygon_free;
  data->stroke.calcfn = (stg_rtk_stroke_fn_t) stg_rtk_fig_polygon_calc;
  data->stroke.drawfn = (stg_rtk_stroke_fn_t) stg_rtk_fig_polygon_draw;
  //data->stroke.xfigfn = (stg_rtk_stroke_fn_t) stg_rtk_fig_polygon_xfig;
    
  data->ox = ox;
  data->oy = oy;
  data->oa = oa;
  data->closed = closed;
  data->filled = filled;

  data->point_count = point_count;
  data->lpoints = calloc(point_count, sizeof(data->lpoints[0]));
  for (i = 0; i < point_count; i++)
    data->lpoints[i] = points[i];
  data->ppoints = calloc(point_count, sizeof(data->ppoints[0]));

  // Call the calculation function just for this stroke
  (*data->stroke.calcfn) (fig, (stg_rtk_stroke_t*) data);
  
  // This will make sure the new stroke gets drawn  
  stg_rtk_fig_dirty(fig);
    

  return;
}


// Free a polygon
void stg_rtk_fig_polygon_free(stg_rtk_fig_t *fig, stg_rtk_polygon_stroke_t *data)
{
  free(data->lpoints);
  free(data->ppoints);
  free(data);
  return;
}


// Update a polygon
void stg_rtk_fig_polygon_calc(stg_rtk_fig_t *fig, stg_rtk_polygon_stroke_t *data)
{
  int i;
  double cosa, sina;
  double lx, ly, gx, gy;
  stg_rtk_point_t *lpoint;
  GdkPoint *ppoint;
  int minx, miny, maxx, maxy;

  minx = miny = INT_MAX / 2;
  maxx = maxy = -1;
  
  cosa = cos(data->oa);
  sina = sin(data->oa);

  // Update the physical polygon coordinates.
  for (i = 0; i < data->point_count; i++)
  {
    lpoint = data->lpoints + i;
    ppoint = data->ppoints + i;

    lx = data->ox + lpoint->x * cosa - lpoint->y * sina;
    ly = data->oy + lpoint->x * sina + lpoint->y * cosa;

    fig->min_x = MIN(fig->min_x, lx);
    fig->min_y = MIN(fig->min_y, ly);
    fig->max_x = MAX(fig->max_x, lx);
    fig->max_y = MAX(fig->max_y, ly);
    
    gx = GX(lx, ly);
    gy = GY(lx, ly);
    
    ppoint->x = GX_TO_DX(gx);
    ppoint->y = GY_TO_DY(gy);

    if (ppoint->x < minx)
      minx = ppoint->x;
    if (ppoint->y < miny)
      miny = ppoint->y;
    if (ppoint->x > maxx)
      maxx = ppoint->x;
    if (ppoint->y > maxy)
      maxy = ppoint->y;
  }

  // Allow for the selection indicator, which may run over a bit.
  minx -= 3;
  miny -= 3;
  maxx += 3;
  maxy += 3;

  // Update the figure's bounding region.
  stg_rtk_region_set_union_rect(fig->region, minx, miny, maxx, maxy);

  return;
}


// Render a polygon
void stg_rtk_fig_polygon_draw(stg_rtk_fig_t *fig, stg_rtk_polygon_stroke_t *data)
{
  GdkDrawable *drawable;
  drawable = (fig->layer < 0 ? fig->canvas->bg_pixmap : fig->canvas->fg_pixmap);

  // Draw polygons
  if (data->closed)
  {
    if (data->filled)
      gdk_draw_polygon(drawable, fig->canvas->gc, TRUE,
                       data->ppoints, data->point_count);
    gdk_draw_polygon(drawable, fig->canvas->gc, FALSE,
                     data->ppoints, data->point_count);
  }

  // Draw polylines
  else
  {
    gdk_draw_lines(drawable, fig->canvas->gc, data->ppoints, data->point_count);    
  }

  

  return;
}


/***************************************************************************
 * Text stroke
 ***************************************************************************/


// Function prototypes
void stg_rtk_fig_text_free(stg_rtk_fig_t *fig, stg_rtk_text_stroke_t *data);
void stg_rtk_fig_text_calc(stg_rtk_fig_t *fig, stg_rtk_text_stroke_t *data);
void stg_rtk_fig_text_draw(stg_rtk_fig_t *fig, stg_rtk_text_stroke_t *data);
//void stg_rtk_fig_text_xfig(stg_rtk_fig_t *fig, stg_rtk_text_stroke_t *data);


// Create a text stroke
void stg_rtk_fig_text_alloc(stg_rtk_fig_t *fig, double ox, double oy, double oa, const char *text)
{
  stg_rtk_text_stroke_t *data;

  data = calloc(1, sizeof(stg_rtk_text_stroke_t));
  stg_rtk_fig_stroke_add(fig, (stg_rtk_stroke_t*) data);
  data->stroke.freefn = (stg_rtk_stroke_fn_t) stg_rtk_fig_text_free;
  data->stroke.calcfn = (stg_rtk_stroke_fn_t) stg_rtk_fig_text_calc;
  data->stroke.drawfn = (stg_rtk_stroke_fn_t) stg_rtk_fig_text_draw;
  //  data->stroke.xfigfn = (stg_rtk_stroke_fn_t) stg_rtk_fig_text_xfig;

  data->ox = ox;
  data->oy = oy;
  data->oa = oa;
  assert(text);
  data->text = strdup(text);

  (*data->stroke.calcfn) (fig, data);

  // This will make sure the new stroke gets drawn
  stg_rtk_fig_dirty(fig);
    
  return;
}


// Cleanup text stroke
void stg_rtk_fig_text_free(stg_rtk_fig_t *fig, stg_rtk_text_stroke_t *data)
{
  assert(data->text);
  free(data->text);
  free(data);
  return;
}


// Update text
void stg_rtk_fig_text_calc(stg_rtk_fig_t *fig, stg_rtk_text_stroke_t *data)
{
  int i, len, baseline;
  int width, ascent, descent;
  
  LTOD(data->point, data->ox, data->oy);

  // Find the line-breaks compute the bounding box
  i = 0;
  baseline = data->point.y;
  while (i < strlen(data->text))
  {
    len = strcspn(data->text + i, "\n");
  
    // Compute the bounding box for the text
    gdk_text_extents(fig->canvas->font, data->text + i, len,
                     NULL, NULL, &width, &ascent, &descent);

    // Update the figure's bounding region.
    stg_rtk_region_set_union_rect(fig->region, data->point.x, baseline - ascent,
                              data->point.x + width, baseline + descent);

    // Compute the baseline for the next line of text
    baseline += 14 * (ascent + descent) / 10;
    i += len + 1;
  }

  return;
}


// Render text
void stg_rtk_fig_text_draw(stg_rtk_fig_t *fig, stg_rtk_text_stroke_t *data)
{
  int i, len, baseline;
  int width, ascent, descent;
  GdkDrawable *drawable;

  drawable = (fig->layer < 0 ? fig->canvas->bg_pixmap : fig->canvas->fg_pixmap);

  // Find the line-breaks compute the bounding box
  i = 0;
  baseline = data->point.y;
  while (i < strlen(data->text))
  {
    len = strcspn(data->text + i, "\n");
    
    // Compute the bounding box for the text
    gdk_text_extents(fig->canvas->font, data->text + i, len,
                     NULL, NULL, &width, &ascent, &descent);

    // Draw the text
    gdk_draw_text(drawable, fig->canvas->font, fig->canvas->gc,
                data->point.x, baseline, data->text + i, len);

    // Compute the baseline for the next line of text
    baseline += 14 * (ascent + descent) / 10;
    i += len + 1;
  }

  return;
}

/*
// Render stroke to xfig
void stg_rtk_fig_text_xfig(stg_rtk_fig_t *fig, stg_rtk_text_stroke_t *data)
{
  int ox, oy, sx, sy;
  int fontsize;

  // Compute origin
  ox = PX(GX(data->ox, data->oy));
  oy = PY(GY(data->ox, data->oy));

  // Compute extent
  sx = 0;
  sy = 0;
  
  // Compute font size
  // HACK
  fontsize = 12; // * 6 * 1200 / fig->canvas->sizex;

  fprintf(fig->canvas->file, "4 0 0 50 0 0 %d 0 4 ", fontsize);
  fprintf(fig->canvas->file, "%d %d ", sx, sy);
  fprintf(fig->canvas->file, "%d %d ", ox, oy);
  fprintf(fig->canvas->file, "%s\\001\n", data->text);

  return;
}
*/



/***************************************************************************
 * Image stroke
 ***************************************************************************/


// Function prototypes
void stg_rtk_fig_image_free(stg_rtk_fig_t *fig, stg_rtk_image_stroke_t *data);
void stg_rtk_fig_image_calc(stg_rtk_fig_t *fig, stg_rtk_image_stroke_t *data);
void stg_rtk_fig_image_draw(stg_rtk_fig_t *fig, stg_rtk_image_stroke_t *data);
//void stg_rtk_fig_image_xfig(stg_rtk_fig_t *fig, stg_rtk_image_stroke_t *data);


// Create a image stroke
void stg_rtk_fig_image_alloc(stg_rtk_fig_t *fig, double ox, double oy, double oa,
                         double scale, int width, int height, int bpp,
                         const void *image, const void *mask)
{
  int bytes;
  stg_rtk_image_stroke_t *data;

  data = calloc(1, sizeof(stg_rtk_image_stroke_t));
  stg_rtk_fig_stroke_add(fig, (stg_rtk_stroke_t*) data);
  data->stroke.freefn = (stg_rtk_stroke_fn_t) stg_rtk_fig_image_free;
  data->stroke.calcfn = (stg_rtk_stroke_fn_t) stg_rtk_fig_image_calc;
  data->stroke.drawfn = (stg_rtk_stroke_fn_t) stg_rtk_fig_image_draw;
  //  data->stroke.xfigfn = (stg_rtk_stroke_fn_t) stg_rtk_fig_image_xfig;

  data->ox = ox;
  data->oy = oy;
  data->oa = oa;

  data->width = width;
  data->height = height;
  data->scale = scale;
  data->bpp = bpp;
    
  // Make our own copy of the image
  bytes = width * height * bpp / 8;
  data->image = malloc(bytes);
  memcpy(data->image, image, bytes);
  
  // Make our own copy of the mask
  if (mask)
  {
    data->mask = malloc(bytes);
    memcpy(data->mask, mask, bytes);
  }
  else
    data->mask = NULL;

  (*data->stroke.calcfn) (fig, data);

  // This will make sure the new stroke gets drawn
  stg_rtk_fig_dirty(fig);
    
  return;
}


// Cleanup image stroke
void stg_rtk_fig_image_free(stg_rtk_fig_t *fig, stg_rtk_image_stroke_t *data)
{
  if (data->mask)
    free(data->mask);
  free(data->image);
  return;
}


// Update image
void stg_rtk_fig_image_calc(stg_rtk_fig_t *fig, stg_rtk_image_stroke_t *data)
{
  int i;//, j;
  int minx, miny, maxx, maxy;
  int px, py;
  double dx, dy;
  double lx, ly;
  double rx, ry;

  minx = miny = INT_MAX / 2;
  maxx = maxy = -1;
  
  dx = data->width * data->scale / 2;
  dy = data->height * data->scale / 2;

  // Determine the bounding rectangle for the image.
  for (i = 0; i < 4; i++)
  {
    rx = cos(i * M_PI/2 - 3 * M_PI/4) / cos(M_PI/4);
    ry = sin(i * M_PI/2 - 3 * M_PI/4) / sin(M_PI/4);
    
    lx = data->ox + rx * dx * cos(data->oa) - ry * dy * sin(data->oa);
    ly = data->oy + rx * dx * sin(data->oa) + ry * dy * cos(data->oa);

    px = GX_TO_DX(GX(lx, ly));
    py = GY_TO_DY(GY(lx, ly));

    fig->min_x = MIN(fig->min_x, lx);
    fig->min_y = MIN(fig->min_y, ly);
    fig->max_x = MAX(fig->max_x, lx);
    fig->max_y = MAX(fig->max_y, ly);

    data->points[i][0] = px;
    data->points[i][1] = py;

    if (px < minx)
      minx = px;
    if (py < miny)
      miny = py;
    if (px > maxx)
      maxx = px;
    if (py > maxy)
      maxy = py;
  }

  // Allow for the selection indicator, which may run over a bit.
  minx -= 1;
  miny -= 1;
  maxx += 1;
  maxy += 1;

  //printf("%.3f %.3f %.3f %.3f\n",
  //       fig->min_x, fig->min_y, fig->max_x, fig->max_y);
  //printf("%d %d %d %d\n", minx, miny, maxx, maxy);
  
  stg_rtk_region_set_union_rect(fig->region, minx, miny, maxx, maxy);
  return;
}


// Render image
void stg_rtk_fig_image_draw(stg_rtk_fig_t *fig, stg_rtk_image_stroke_t *data)
{
  int i, j;//, k;
  int fill;
  //double ix, iy, px, py;
  double sxx, sxy, syx, syy;
  double dxx, dxy, dyx, dyy;
  double ox, oy;//, cosa, sina;
  uint16_t *last_pixel, *pixel, *mask;
  double gpoints[4][2];
  GdkPoint points[4];
  GdkGCValues values;
  GdkColor color, oldcolor;
  GdkColormap *colormap;
  GdkDrawable *drawable;

  // TESTING: this should be implemented properly on a per-figure basis.
  // Dont render if the user is mousing around
  if (fig->canvas->mouse_mode != 0 && fig->canvas->mouse_selected_fig == NULL)
    return;
  
  drawable = (fig->layer < 0 ? fig->canvas->bg_pixmap : fig->canvas->fg_pixmap);

  gdk_gc_get_values(fig->canvas->gc, &values);
  colormap = fig->canvas->colormap;
  oldcolor = values.foreground;

  ox = data->points[0][0];
  oy = data->points[0][1];

  sxx = data->points[1][0] - data->points[0][0];
  sxy = data->points[1][1] - data->points[0][1];

  syx = data->points[3][0] - data->points[0][0];
  syy = data->points[3][1] - data->points[0][1];

  dxx = sxx / data->width;
  dxy = sxy / data->width;
  dyx = syx / data->height;
  dyy = syy / data->height;

  gpoints[0][0] = ox - dxx / 2;
  gpoints[0][1] = oy - dyy / 2;
  gpoints[1][0] = gpoints[0][0] + dyx;
  gpoints[1][1] = gpoints[0][1] + dyy;
  gpoints[2][0] = gpoints[1][0] + dxx;
  gpoints[2][1] = gpoints[1][1] + dxy;
  gpoints[3][0] = gpoints[2][0] - dyx;
  gpoints[3][1] = gpoints[2][1] - dyy;

  /* FIX
     gpoints[0][0] -= 1;
     gpoints[0][1] += 1;
     gpoints[1][0] -= 1;
     gpoints[1][1] -= 1;
     gpoints[2][0] += 1;
     gpoints[2][1] -= 1;
     gpoints[3][0] += 1;
     gpoints[3][1] += 1;
  */

  fill = 1;

  last_pixel = NULL;
  pixel = (uint16_t*) data->image;
  mask = (uint16_t*) data->mask;

  for (j = 0; j < data->height; j++)
  {
    for (i = 0; i < data->width; i++)
    {
      /*
        if (!mask || *mask > 0)
        {
        // Set polygon color
        color.red = STK_R_RGB16(*pixel) << 8;
        color.green = STK_G_RGB16(*pixel) << 8;
        color.blue = STK_B_RGB16(*pixel) << 8;
        gdk_color_alloc(colormap, &color);
        gdk_gc_set_foreground(fig->canvas->gc, &color);

        // Set start of polygon
        points[0].x = gpoints[0][0];
        points[0].y = gpoints[0][1];
        points[1].x = gpoints[1][0];
        points[1].y = gpoints[1][1];

        // Set end of polygon
        points[2].x = gpoints[2][0];
        points[2].y = gpoints[2][1];
        points[3].x = gpoints[3][0];
        points[3].y = gpoints[3][1];

        // Draw the polygon
        gdk_draw_polygon(drawable, fig->canvas->gc, fill, points, 4);
        }
      */

      if (last_pixel == NULL)
      {
        if (!mask || *mask > 0)
        {
          // Set polygon color
          color.red = STK_R_RGB16(*pixel) << 8;
          color.green = STK_G_RGB16(*pixel) << 8;
          color.blue = STK_B_RGB16(*pixel) << 8;
          gdk_color_alloc(colormap, &color);
          gdk_gc_set_foreground(fig->canvas->gc, &color);

          // Set start of new polygon
          points[0].x = gpoints[0][0];
          points[0].y = gpoints[0][1];
          points[1].x = gpoints[1][0];
          points[1].y = gpoints[1][1];
          points[2].x = gpoints[2][0];
          points[2].y = gpoints[2][1];
          points[3].x = gpoints[3][0];
          points[3].y = gpoints[3][1];
          
          last_pixel = pixel;
        }
      }
      else
      {
        if (!mask || *mask > 0)
        {
          if (*pixel == *last_pixel)
          {
            // Set end of polygon
            points[2].x = gpoints[2][0];
            points[2].y = gpoints[2][1];
            points[3].x = gpoints[3][0];
            points[3].y = gpoints[3][1];
          }
          else
          {
            // Draw the polygon
            gdk_draw_polygon(drawable, fig->canvas->gc, fill, points, 4);

            // Set new polygon color
            color.red = STK_R_RGB16(*pixel) << 8;
            color.green = STK_G_RGB16(*pixel) << 8;
            color.blue = STK_B_RGB16(*pixel) << 8;
            gdk_color_alloc(colormap, &color);
            gdk_gc_set_foreground(fig->canvas->gc, &color);

            // Set start of new polygon
            points[0].x = gpoints[0][0];
            points[0].y = gpoints[0][1];
            points[1].x = gpoints[1][0];
            points[1].y = gpoints[1][1];
            points[2].x = gpoints[2][0];
            points[2].y = gpoints[2][1];
            points[3].x = gpoints[3][0];
            points[3].y = gpoints[3][1];

            last_pixel = pixel;
          }
        }
        else
        {
          // Draw the polygon
          gdk_draw_polygon(drawable, fig->canvas->gc, fill, points, 4);

          last_pixel = NULL;
        }
      }

      pixel++;
      if (mask)
        mask++;

      // Move the pixel polygon along
      gpoints[0][0] += dxx;
      gpoints[0][1] += dxy;
      gpoints[1][0] += dxx;
      gpoints[1][1] += dxy;
      gpoints[2][0] += dxx;
      gpoints[2][1] += dxy;
      gpoints[3][0] += dxx;
      gpoints[3][1] += dxy;
    }

    if (last_pixel != NULL)
    {
      // Draw the polygon
      gdk_draw_polygon(drawable, fig->canvas->gc, fill, points, 4);

      last_pixel = NULL;
    }
    
    // Reset the pixel polygon to the start of the line
    gpoints[0][0] += -data->width * dxx + dyx;
    gpoints[0][1] += -data->width * dxy + dyy;
    gpoints[1][0] += -data->width * dxx + dyx;
    gpoints[1][1] += -data->width * dxy + dyy;
    gpoints[2][0] += -data->width * dxx + dyx;
    gpoints[2][1] += -data->width * dxy + dyy;
    gpoints[3][0] += -data->width * dxx + dyx;
    gpoints[3][1] += -data->width * dxy + dyy;
  }

  // Reset color
  gdk_gc_set_foreground(fig->canvas->gc, &oldcolor);

  return;
}

/*
// Render stroke to xfig
void stg_rtk_fig_image_xfig(stg_rtk_fig_t *fig, stg_rtk_image_stroke_t *data)
{
  return;
}
*/


gboolean stg_rtk_fig_blink_callback( void* data )
{
  stg_rtk_fig_t* fig;
  
  if( data == NULL )
    return FALSE;
  
  fig = (stg_rtk_fig_t*)data;
  stg_rtk_fig_show( fig, !fig->show );

  return TRUE;
}


void stg_rtk_fig_blink( stg_rtk_fig_t* fig, int interval_ms, int flag )
{
  assert( fig );  
  
  if( flag )
    g_timeout_add( (guint)interval_ms, stg_rtk_fig_blink_callback, fig );
  else
    g_source_remove_by_user_data( fig );
}
