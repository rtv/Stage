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
 * Desc: A root device model - replaces the CWorld class
 * Author: Richard Vaughan
 * Date: 31 Jan 2003
 * CVS info: $Id: root.cc,v 1.1.2.3 2003-02-05 03:59:49 rtv Exp $
 */


#include <math.h>

#include "root.hh"
#include "matrix.hh"
#include <stdio.h>

#define DEBUG

// constructor
CRootDevice::CRootDevice( LibraryItem* libit )
  : CEntity( libit, 0, NULL )    
{
  PRINT_DEBUG( "Creating root model" );
  
  CEntity::ppm = 100; // default 1cm world resolution
  CEntity::root = this; // phear me!

  shape = ShapeRect;
  size_x = 10.0; // a 10m world by default
  size_y = 10.0;
  
  vision_return = false; 
  laser_return = LaserVisible;
  sonar_return = true;
  obstacle_return = true;
  idar_return = IDARReflect;
}

///////////////////////////////////////////////////////////////////////////
// Startup routine
//
bool CRootDevice::Startup()
{
  PRINT_DEBUG( "Startup root model" );

  // create the matrix for everyone to play in
  
  // find out how big the matrix has to be
  // the bounding box routine stretches the given dimensions to fit
  // the entity and its children
  double xmin, ymin, xmax, ymax;
  xmin = ymin = 9999999.9;
  xmax = ymax = 0.0;
  
  this->GetBoundingBox( xmin, ymin, xmax, ymax );
  
  PRINT_DEBUG4( "The world seems to be from %.2fx%.2f to %.2fx%.2f\n",
	       xmin, ymin, xmax, ymax );
  
  // Initialise the matrix, now that we know how big it has to be
  int w = (int)( ceil( xmax * ppm) - xmin*ppm);
  int h = (int)( ceil( ymax * ppm) - ymin*ppm);  
  
  PRINT_DEBUG3( "Creating a matrix %dx%d pixels at %.2f ppm\n",
	       w, h, ppm );
  
  assert( matrix = new CMatrix(w, h, 1) );

  //if( CEntity::enable_gui ) GuiInit();
 
  if (!CEntity::Startup())
    return false;

  return true;
}


///////////////////////////////////////////////////////////////////////////
// Update everything
void CRootDevice::Update( double sim_time ) 
{
  //PRINT_DEBUG( "Update root model" );

  CEntity::Update( sim_time );
  
  double x, y, th;
  GetGlobalPose( x,y,th );
  
  ReMap(x, y, th);

  return;
}
