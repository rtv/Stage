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
 * CVS info: $Id: root.cc,v 1.1.2.6 2003-02-23 08:01:37 rtv Exp $
 */


#include <math.h>

#include "root.hh"
#include "matrix.hh"
#include <stdio.h>

#define DEBUG

// constructor
CRootDevice::CRootDevice( )
  : CEntity( 0, "root", "red", NULL )    
{
  PRINT_DEBUG( "Creating root model" );
  
  CEntity::root = this; // phear me!
  
  size_x = 10.0; // a 10m world by default
  size_y = 10.0;
  ppm = 10; // default 10cm resolution passed into matrix (DEBUG)
  //this->ppm = 100; // default 10cm resolution passed into matrix

  // the global origin is the bottom left corner of the root object
  origin_x = size_x/2.0;
  origin_y = size_y/2.0;

  vision_return = false; 
  laser_return = LaserVisible;
  sonar_return = true;
  obstacle_return = true;
  idar_return = IDARReflect;
  
  PRINT_DEBUG3( "Creating a matrix [%.2fx%.2f]m at %.2f ppm",
		size_x, size_y, ppm );
  
  //if( matrix ) delete matrix;
  // make the matrix 1 pixel wider than the world
  assert( matrix = new CMatrix( size_x, size_y, ppm, 1) );
  

}

int CRootDevice::SetProperty( int con, stage_prop_id_t property, 
			      void* value, size_t len, stage_buffer_t reply )
{
  PRINT_DEBUG3( "setting prop %d (%d bytes) for ROOT ent %p",
		property, len, this );
  
  assert( value );
  assert( len > 0 );
  assert( (int)len < MAX_PROPERTY_DATA_LEN );

  // get the inherited behavior first
  CEntity::SetProperty( con, property, value, len );
  
  double x, y, th;
  
  switch( property )
    {      
      // we'll tackle this one, 'cos it means creating a new matrix
    case STG_PROP_ENTITY_SIZE:
      PRINT_WARN( "setting size of ROOT" );
      assert( len == sizeof( stage_size_t ) );
      matrix->Resize( size_x, size_y, ppm );      
      this->MapFamily();
      break;
      
    case STG_PROP_ROOT_PPM:
      PRINT_WARN( "setting PPM of ROOT" );
      assert(len == sizeof(ppm) );
      memcpy( &(ppm), value, sizeof(ppm) );
      
      matrix->Resize( size_x, size_y, ppm );      
      this->MapFamily();
      break;

    case STG_PROP_ROOT_GUI:
      


    default: 
      break;
    }


  return 0;
}
