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
 * Desc: Simulates a simple box obstacle
 * Author: Andrew Howard
 * Date: 7 Dec 2000
 * CVS info: $Id: boxobstacle.cc,v 1.23 2002-09-07 02:05:23 rtv Exp $
 */

#include "world.hh"
#include "boxobstacle.hh"

// register this device type with the Library
CEntity box_bootstrap( "box", 
		       BoxType, 
		       (void*)&CBoxObstacle::Creator ); 

///////////////////////////////////////////////////////////////////////////
// Default constructor
CBoxObstacle::CBoxObstacle(CWorld *world, CEntity *parent)
        : CEntity(world, parent)
{
  this->stage_type = BoxType;
  this->color = ::LookupColor(BOX_COLOR);

  this->shape = ShapeRect;
  this->size_x = 1.0;
  this->size_y = 1.0;
  
  vision_return = true; 
  laser_return = LaserVisible;
  sonar_return = true;
  obstacle_return = true;
}


///////////////////////////////////////////////////////////////////////////
// Update the laser data
void CBoxObstacle::Update( double simtime )
{
  CEntity::Update(simtime);
  
  double x, y, th;
  GetGlobalPose( x,y,th );

  ReMap(x, y, th);
}




