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
 */
// ==================================================================
// Filename:	raytrace.cc
// Author:      Richard Vaughan
// $Id: raytrace.cc,v 1.10.6.5 2003-08-09 01:25:20 rtv Exp $
// ==================================================================

#include <assert.h>
#include "raytrace.hh"

CEntity* g_nullp = 0;

CLineIterator::CLineIterator( double x, double y, double a, double b, 
			      CMatrix* matrix, LineIteratorMode pmode )
{   
  m_matrix = matrix;
  
  m_x = x * m_matrix->ppm;
  m_y = y * m_matrix->ppm;

  m_ent = 0;
  m_index = 0;

  switch( pmode )
  {
    case PointToBearingRange:
    {
      double range = b;
      double bearing = a;
	
      m_angle = bearing;
      m_max_range = m_remaining_range = range * m_matrix->ppm;
    }
    break;
    case PointToPoint:
    {
      double x1 = a;
      double y1 = b;
	
      //printf( "From %.2f,%.2f to %.2f,%.2f\n", x,y,x1,y1 );

      m_angle = atan2( y1-y, x1-x );
      m_max_range = m_remaining_range = hypot( x1-x, y1-y ) * m_matrix->ppm;
    }
    break;
    default:
      puts( "Stage Warning: unknown LineIterator mode" );
  }

  //printf( "angle = %.2f remaining_range = %.2f\n", m_angle, m_remaining_range );
  //fflush( stdout );
};


CEntity* CLineIterator::GetNextEntity( void )
{
  //PrintArray( m_ent );

  if( m_remaining_range <= 0 ) return 0;

  //printf( "current ptr: %p index: %d\n", m_ent[m_index], index );

  if( !(m_ent && m_ent[m_index]) ) // if the ent array is null or empty
    {
      //printf( "before %d,%d\n", (int)m_x, (int)m_y );
      // get a new array of pointers
      m_ent = RayTrace( m_x, m_y, m_angle, m_remaining_range );
      //PrintArray( m_ent );
      //printf( "after %d,%d\n", (int)m_x, (int)m_y );
      m_index = 0; // back to the start of the array
    }
  
  assert( m_ent != 0 ); // should be a valid array, even if empty
 
  //PrintArray( m_ent );
  //printf( "returning %p (index: %d) at: %d %d rng: %d\n",  
  //  m_ent[m_index], m_index, (int)m_x, (int)m_y, (int)m_remaining_range );

  return m_ent[m_index++]; // return the next CEntity* (it may be null)
}

inline CEntity** CLineIterator::RayTrace( double &px, double &py, double pth, 
				   double &remaining_range )
{
  int range = 0;
  
  //printf( "Ray from %.2f,%.2f angle: %.2f remaining_range %.2f\n", 
  //  px, py, RTOD(pth), remaining_range );
  
  // hops along each axis 1 cell at a time
  double cospth = cos( pth );
  double sinpth = sin( pth );
  
  CEntity** ent = 0;
  
  // Look along scan line for obstacles
  for( range = 0; range < remaining_range; range++ )
  {
    px += cospth;
    py += sinpth;
      
    // stop this ray if we're out of bounds
    if( px < 0 ) 
    { 
      px = 0; 
      remaining_range = 0.0;
    }
    if( px >= m_matrix->width ) 
    { 
      px = m_matrix->width-1; 
      remaining_range = 0.0;
    }
    if( py < 0 ) 
    { 
      py = 0; 
      remaining_range = 0.0;
    }
    if( py >= m_matrix->height ) 
    { 
      py = m_matrix->height-1; 
      remaining_range = 0.0;
    }
      
    //printf( "looking in %d,%d\n", (int)px, (int)py );
      
    ent = m_matrix->get_cell( (int)px,(int)py );
    if( !ent[0] && px+1 < m_matrix->width) 
      ent = m_matrix->get_cell( (int)px+1,(int)py );
      
	  if( ent[0] ) break;// we hit something!
  }
  
  remaining_range -= (double)range+1; // we have this much left to go
  
  //if( ent && ent[0] )
  //printf( "Hit %p (%d) at %.2f,%.2f with %.2f to go\n", 
  //    ent[0], ent[0]->type_num, px, py, remaining_range );
  //else
  //printf( "hit nothing. remaining range = %.2f\n", remaining_range );
  // fflush( stdout );
  
  return ent; // we hit these entities
}; 

void CLineIterator::PrintArray( CEntity** ent )
{
  printf( "Array: " );
  
  if( !ent ) 
  {
    printf( "(null)\n" ); 
    return;
  }
  
  //puts( "foo" );

  printf( "[ " );
  int p = 0;
  while( ent[p] ) printf( "%p ", (ent[p++]) );
  
  printf( " ]\n" );
  
  //fflush( stdout );
}

// an isoceles triangle is specified from it's point, by the bearing
// and length of it's left side, and by the angle between the equal
// sides. the triangle is scanned from point to base in increasingly
// long lines perpendicular to the base. skip give the distance
// increment between scan lines.
CTriangleAreaIterator::
CTriangleAreaIterator( double x, double y, 
		       double bearing, double range, double angle,
		       double skip,
		       CMatrix* matrix )
{
  //printf( "new CTriangleAreaIterator( %.2f, %.2f, %.2f, %.2f, %.2f )\n",
  //  x, y, bearing, range, angle, skip ); 
  
  m_matrix = matrix;
  
  m_x = x;
  m_y = y;
  m_angle = angle;

  m_bearing = bearing;

  m_skip = skip;
  m_max_range = range;
  m_range_so_far = 0; 


  // the angle of the lines to iterate over is 
  // perpendicular to the center line of the triangle.
  m_li_angle = (m_bearing -angle/2.0) - M_PI_2;

  li = 0; // initially null line iterator

  assert( m_skip > 0 );
}

CTriangleAreaIterator::~CTriangleAreaIterator( void )
{
  if( li ) delete li;
}

CEntity* CTriangleAreaIterator::GetNextEntity( void )
{
  CEntity* ent;

  while( m_range_so_far < m_max_range )
  {
    // if there's an iterator and it's still working, use it.
    if( li && (ent = li->GetNextEntity()))
      return ent;
      
    // otherwise we need to create a new iterator
    // after deleting the old one of course
    if( li ) delete li; 
      
    //printf( "x: %.2f y: %.2f + skip: %.2f = ", m_x, m_y, m_skip );

    // move from the current position down the left edge of the triangle
    m_x += m_skip * cos(m_bearing);
    m_y += m_skip * sin(m_bearing);

    //printf( "x: %.2f y: %.2f ", m_x, m_y );
      
    m_range_so_far += m_skip;
      
    //printf( "range so far: %.2f\n", m_range_so_far );
    
    // figure the length of the scan line
    double li_len = m_range_so_far * sin( m_angle );
      
    //printf( "new CLineIterator( %.2f, %.2f, %.2f, %.2f )\n",
    //      m_x, m_y, m_li_angle, li_len ); 
      

    li = new CLineIterator( m_x, m_y, m_li_angle, li_len, 
                            m_matrix, PointToBearingRange );
  }
  return 0;
}

double CTriangleAreaIterator::GetRange( void )
{
  // find the height of the triangle so far
  return( m_range_so_far * cos( m_angle/2.0 ) );
}


CRectangleIterator::CRectangleIterator( double x, double y, double th,
					double w, double h,  
					CMatrix* matrix )
{
  // calculate the corners of our body

  double cx = (w/2.0) * cos(th);
  double cy = (h/2.0) * cos(th);
  double sx = (w/2.0) * sin(th);
  double sy = (h/2.0) * sin(th);
  
  corners[0][0] = x + cx - sy;
  corners[0][1] = y + sx + cy;
    
  corners[1][0] = x - cx - sy;
  corners[1][1] = y - sx + cy;
    
  corners[2][0] = x - cx + sy;
  corners[2][1] = y - sx - cy;
   
  corners[3][0] = x + cx + sy;
  corners[3][1] = y + sx - cy;
  
  
  lits[0] = new CLineIterator( corners[0][0], corners[0][1], 
                               corners[1][0], corners[1][1],
                               matrix, PointToPoint );
  
  lits[1] = new CLineIterator( corners[1][0], corners[1][1], 
                               corners[2][0], corners[2][1],
                               matrix, PointToPoint );

  lits[2] = new CLineIterator( corners[2][0], corners[2][1], 
                               corners[3][0], corners[3][1],
                               matrix, PointToPoint );
  
  lits[3] = new CLineIterator( corners[3][0], corners[3][1], 
                               corners[0][0], corners[0][1],
                               matrix, PointToPoint );
  
} 

CRectangleIterator::~CRectangleIterator( void )
{
  for( int f=0; f<4; f++ )
    if( lits[f] ) delete lits[f];
}

CEntity* CRectangleIterator::GetNextEntity( void )
{
  CEntity* ent = 0;
  
  for( int i=0; i<4; i++ )
  {
    if (!lits[i])
      continue;
    if( (ent = lits[i]->GetNextEntity()) == 0 )
    {
      delete lits[i]; // don't need that line any more
      lits[i] = 0; // mark it as deleted
    }
    else
      break; // we'll take this entity
  }
  
  return ent;
}


// a circle iterator is an arc iterator with a 2PI scan range
CCircleIterator::CCircleIterator( double x, double y, double r, CMatrix* matrix )
  : CArcIterator( x, y, 0, r, M_PI*2.0, matrix )
{
}

CArcIterator::CArcIterator( double x, double y, double th,
			    double scan_r, double scan_th, 
			    CMatrix* matrix )
{   
  m_matrix = matrix;

  m_center_x = x * m_matrix->ppm;
  m_center_y = y * m_matrix->ppm;
  m_angle = th - scan_th/2.0;

  m_radius = scan_r * m_matrix->ppm;
  m_max_angle = m_angle + scan_th; // this is the angle length of the scan in radians

  m_ent = 0;
  m_index = 0;

  m_px = 0;
  m_py = 0;

  // the angle increment - moves us by 1 pixel
  m_dangle = atan2( 1, m_radius );

  //printf( "Construct circle iterator: x=%.2f y=%.2f r=%.2f a=%.2f dangle=%.2f \n",
  //  m_center_x, m_center_y, m_radius, m_angle, m_dangle );

};


CEntity* CArcIterator::GetNextEntity( void )
{
  if( m_angle >= m_max_angle ) return 0; // done!
  
  //printf( "current ptr: %p index: %d\n", m_ent[m_index], index );

  if( !(m_ent && m_ent[m_index]) ) // if the ent array is null or empty
  {
    //printf( "before %d,%d\n", (int)m_x, (int)m_y );
    // get a new array of pointers
    m_ent = ArcTrace( m_angle );
    //PrintArray( m_ent );
    //printf( "after %d,%d\n", (int)m_x, (int)m_y );
    m_index = 0; // back to the start of the array
  }
  
  assert( m_ent != 0 ); // should be a valid array, even if empty
 
  //PrintArray( m_ent );
  //printf( "returning %p (index: %d) at: %d %d rng: %d\n",  
  //m_ent[m_index], m_index, (int)m_x, (int)m_y, (int)m_angle );

  return m_ent[m_index++]; // return the next CEntity* (it may be null)  
}


CEntity** CArcIterator::ArcTrace( double &remaining_angle )
{
  CEntity** ent = 0;
   
  while( remaining_angle < m_max_angle )
  { 
    m_px = m_center_x + m_radius * cos( remaining_angle );
    m_py = m_center_y + m_radius * sin( remaining_angle );
     
    //printf( "looking in %d,%d  angle: %.2f\n", 
    //     (int)px, (int)py, remaining_angle );
     
    remaining_angle += m_dangle;
     
    // bounds check
    if( m_px < 0 || m_px >= m_matrix->width-1 || 
        m_py < 0 || m_py >= m_matrix->height ) 
    {
      // if we're out of bounds, then return a pointer to NULL,
      // indicating free space
      ent = &g_nullp;
    }
    else
    {
      ent = m_matrix->get_cell( (int)m_px,(int)m_py );
      if( !ent[0] )//&& (px+1 < m_matrix->width) ) 
        ent = m_matrix->get_cell( (int)m_px+1,(int)m_py );
    }

    if( ent[0] ) break;// we hit something!
  }
 
  //if( ent && ent[0] )
  //printf( "Hit %p at %.2f,%.2f with %.2f to go\n", 
  //   ent[0], px, py, remaining_angle );
  //else
  //printf( "hit nothing. remaining angle = %.2f\n", remaining_angle );
  //fflush( stdout );
 
  return ent; // we hit these entities
}

void CArcIterator::PrintArray( CEntity** ent )
{
  printf( "Array: " );
  
  if( !ent ) 
  {
    printf( "(null)\n" ); 
    return;
  }
  
  //puts( "foo" );
  
  printf( "[ " );
  int p = 0;
  while( ent[p] ) printf( "%p ", (ent[p++]) );
  
  printf( " ]\n" );
  
  //fflush( stdout );
}
