/*
 *  RTK2 : A GUI toolkit for robotics
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
 * CVS: $Id: rtk_region.c,v 1.4 2005-03-11 21:56:57 rtv Exp $
 *
 * TODO:
 *  - Use GdkRegions instead of GdkRectangles.
 */

#include <stdlib.h>
#include "rtk.h"


// Create a new region.
stg_rtk_region_t *stg_rtk_region_create()
{
  stg_rtk_region_t *region;

  region = calloc(1, sizeof(stg_rtk_region_t));
  stg_rtk_region_set_empty(region);

  return region;
}


// Destroy a region.
void stg_rtk_region_destroy(stg_rtk_region_t *region)
{
  free(region);
  return;
}


// Set a region to empty.
void stg_rtk_region_set_empty(stg_rtk_region_t *region)
{
  region->rect.x = region->rect.y = 0;
  region->rect.width = region->rect.height = 0;
  return;
}


// Set the region to the union of the two given regions.
void stg_rtk_region_set_union(stg_rtk_region_t *regiona, stg_rtk_region_t *regionb)
{
  GdkRectangle rectc;

  if (!stg_rtk_region_test_empty(regionb))
  {
    if (stg_rtk_region_test_empty(regiona))
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
void stg_rtk_region_set_union_rect(stg_rtk_region_t *region, int ax, int ay, int bx, int by)
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
    if (stg_rtk_region_test_empty(region))
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
void stg_rtk_region_get_brect(stg_rtk_region_t *region, GdkRectangle *rect)
{
  *rect = region->rect;
  return;
}


// Test to see if a region is empty.
int stg_rtk_region_test_empty(stg_rtk_region_t *region)
{
  if (region->rect.width * region->rect.height == 0)
    return 1;
  return 0;
}


// Test for intersection betweenr regions.
int stg_rtk_region_test_intersect(stg_rtk_region_t *regiona, stg_rtk_region_t *regionb)
{
  GdkRectangle recta, rectb, rectc;

  recta = regiona->rect;
  recta.width += 1;
  rectb = regionb->rect;
  rectb.width += 1;
  
  return gdk_rectangle_intersect(&regiona->rect, &regionb->rect, &rectc);
}

