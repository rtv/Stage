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
 * Desc: Simulates a simple box 
 * Author: Andrew Howard
 * Date: 7 Dec 2000
 * CVS info: $Id: box.cc,v 1.3 2002-11-11 08:21:39 rtv Exp $
 */

#include "world.hh"
#include "box.hh"

///////////////////////////////////////////////////////////////////////////
// Default constructor
CBox::CBox(LibraryItem* libit, CWorld *world, CEntity *parent)
        : CEntity(libit, world, parent)
{
  if(parent)
    this->color = parent->color;
  else
    this->color = libit->color;

  this->shape = ShapeRect;
  this->size_x = 1.0;
  this->size_y = 1.0;
  
  vision_return = true; 
  laser_return = LaserVisible;
  sonar_return = true;
  obstacle_return = true;
  idar_return = IDARReflect;
}


///////////////////////////////////////////////////////////////////////////
// Update the laser data
void CBox::Update( double simtime )
{
  CEntity::Update(simtime);
  
  double x, y, th;
  GetGlobalPose( x,y,th );

  ReMap(x, y, th);
}




