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
 * Desc: Stk region manipulation
 * Author: Andrew Howard
 * CVS: $Id: rtk_region.c,v 1.3 2005-03-11 20:50:44 rtv Exp $
 *
 * TODO:
 *  - Use GdkRegions instead of GdkRectangles.
 */

#include <stdlib.h>
#include "rtk.h"


// Create a new region.
stk_region_t *stk_region_create()
{
  stk_region_t *region;

  region = calloc(1, sizeof(stk_region_t));
  stk_region_set_empty(region);

  return region;
}


// Destroy a region.
void stk_region_destroy(stk_region_t *region)
{
  free(region);
  return;
}


// Set a region to empty.
void stk_region_set_empty(stk_region_t *region)
{
  region->rect.x = region->rect.y = 0;
  region->rect.width = region->rect.height = 0;
  return;
}


// Set the region to the union of the two given regions.
void stk_region_set_union(stk_region_t *regiona, stk_region_t *regionb)
{
  GdkRectangle rectc;

  if (!stk_region_test_empty(regionb))
  {
    if (stk_region_test_empty(regiona))
      regiona->rect = regionb->rect;
    else 
    {
      gdk_rectangle_union(&regiona->rect, &regionb->rect, &rectc);
      regiona->rect = rectc;
    }
  }
  return;
}


// Set the region to the union of the region with a rectangle
void stk_region_set_union_rect(stk_region_t *region, int ax, int ay, int bx, int by)
{
  GdkRectangle rectb, rectc;

  if (ax <= 0)
    rectb.x = 0;
  else
    rectb.x = ax;

  if (ay <= 0)
    rectb.y = 0;
  else
    rectb.y = ay;

  if (bx - ax + 1 <= 0)
    rectb.width = 0;
  else
    rectb.width = bx - ax + 1;
  
  if (by - ay + 1 <= 0)
    rectb.height = 0;
  else
    rectb.height = by - ay + 1;

  if (rectb.width * rectb.height > 0)
  {
    if (stk_region_test_empty(region))
      region->rect = rectb;
    else
    {
      gdk_rectangle_union(&region->rect, &rectb, &rectc);
      region->rect = rectc;
    }
  }
  return;
}


// Get the bounding rectangle for the region.
void stk_region_get_brect(stk_region_t *region, GdkRectangle *rect)
{
  *rect = region->rect;
  return;
}


// Test to see if a region is empty.
int stk_region_test_empty(stk_region_t *region)
{
  if (region->rect.width * region->rect.height == 0)
    return 1;
  return 0;
}


// Test for intersection betweenr regions.
int stk_region_test_intersect(stk_region_t *regiona, stk_region_t *regionb)
{
  GdkRectangle recta, rectb, rectc;

  recta = regiona->rect;
  recta.width += 1;
  rectb = regionb->rect;
  rectb.width += 1;
  
  return gdk_rectangle_intersect(&regiona->rect, &regionb->rect, &rectc);
}

