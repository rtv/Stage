/*************************************************************************
 * RTV
 * $Id: matrix.cc,v 1.2.2.4 2001-08-24 03:42:15 vaughan Exp $
 ************************************************************************/

#include <math.h>
#include <iostream.h>
#include <fstream.h>

#if INCLUDE_ZLIB
#include <zlib.h>
#endif

#include "matrix.h"

//#define DEBUG

typedef short           short_triple[3];

void CMatrix::PrintCell( int cell )
{
  printf( "data[ %d ] ", cell );
  int p = 0;
  while( data[cell][p] )
    printf( "\t%p", data[cell][p++] );
  
  printf( "\t %d/%d\n", current_slot[cell], available_slots[cell] );
  
  fflush( stdout );
}
  
  
// construct from width / height 
CMatrix::CMatrix(int w,int h)
{
#ifdef DEBUG
  cout << "Image: " << width << 'x' << height << flush;
#endif
  
  initial_buf_size = 4;

  mode = mode_set;

  width = w; height = h;
  data 	= new CEntity**[width*height];
  current_slot = new int[ width*height ];
  available_slots = new int[ width*height ];

  for( int p=0; p< width * height; p++ )
    {
      // create the pointer "strings"
      data[p] = new CEntity*[ initial_buf_size + 1];
      // zero them out
      memset( data[p], 0, (initial_buf_size + 1) * sizeof( CEntity* ) );

      current_slot[p] = 0;
      available_slots[p] = initial_buf_size;
    }

}


//destruct
CMatrix::~CMatrix(void) 	
{
	if (data) delete [] data;
	data = NULL;
}

void CMatrix::copy_from(CMatrix* img)
{
  if ((width != img->width)||(height !=img->height))
    {
      fprintf(stderr,"CMatrix:: mismatch in size (copy_from)\n");
      exit(0);
    }
  
  memcpy(data,img->data,width*height*sizeof(CEntity*) );
}


void CMatrix::dump( void )
{
  ofstream out( "world.dump" );
  
  for( int y=0; y<height; y++ )
    {
      for( int x=0; x<width; x++ )
	{
	  
	  CEntity** ent = get_cell( x,y );
	  
	  //while( *ent )
	  if( *ent )
	    {   
	      out << x << ' ' << y;
	      out << ' ' << (int)*ent << endl;
	    }
	  //else

	  //out << ' ' << 0;
	  
	  //out << endl;
	  
	}
      
      //out << endl; // blank line
    }

  out.close();

  cout << "DUMPED" << endl;
}

void CMatrix::draw_circle(int x,int y,int r, CEntity* ent )
{
  double i,cx,cy;
  int x1,y1,x2,y2;
  
  x1=x;y1=y+r;	
  
  for (i=0;i<2.0*M_PI;i+=0.1)
    {
      cx = (double) x + (double (r) * sin(i));
      cy = (double) y + (double (r) * cos(i));
      x2=(int)cx;y2=(int)cy;
      draw_line(x1,y1,x2,y2,ent);
      x1=x2;y1=y2;
    }	
  
  draw_line(x1,y1,x,y+r,ent);
}


void CMatrix::draw_rect( const Rect& t, CEntity* ent )
{
  draw_line( t.toplx, t.toply, t.toprx, t.topry, ent );
  draw_line( t.toprx, t.topry, t.botlx, t.botly, ent );
  draw_line( t.botlx, t.botly, t.botrx, t.botry, ent );
  draw_line( t.botrx, t.botry, t.toplx, t.toply, ent );
}


void CMatrix::draw_line(int x1,int y1,int x2,int y2, CEntity* ent)
{
  int delta_x, delta_y;
  int delta, incE, incNE;
  int x, y;
  int neg_slope = 0;
  
  if (x1 > x2)
    {
      delta_x = x1 - x2;
      if (y1 > y2)	delta_y = y1 - y2;
      else		delta_y = y2 - y1;
      
      if (delta_y <= delta_x)	draw_line(x2, y2, x1, y1, ent);
    }
  if (y1 > y2)
    {
      delta_y = y1 - y2;
      if (x1 > x2)	delta_x = x1 - x2;
      else		delta_x = x2 - x1;
      
      if (delta_y > delta_x)	draw_line(x2, y2, x1, y1, ent);
    }
  
  if (x1 > x2)
    {
      neg_slope = 1;
      delta_x = x1 - x2;
    }
  else
    delta_x = x2 - x1;
  
  if (y1 > y2)
    {
      neg_slope = 1;
      delta_y = y1 - y2;
    }
  else
    delta_y = y2 - y1;
  
  x = x1;
  y = y1;
  
  set_cell(x,y,ent);
  
  if (delta_y <= delta_x)
    {
      delta = 2 * delta_y - delta_x;
      incE = 2 * delta_y;
      incNE = 2 * (delta_y - delta_x);
      
      while (x < x2)
	{
	  if (delta <= 0)
	    {
	      delta = delta + incE;
	      x++;
	    }
	  else
	    {
	      delta = delta + incNE;
	      x++;
	      if (neg_slope)	y--;
	      else		y++;
	    }
	  set_cell(x,y,ent);
	}
    }
  else
    {
      delta = 2 * delta_x - delta_y;
      incE = 2 * delta_x;
      incNE = 2 * (delta_x - delta_y);
      
      while (y < y2) 
	{
	  if (delta <= 0)
	    {
	      delta = delta + incE;
	      y++;
	    }
	  else
	    {
	      delta = delta + incNE;
	      y++;
	      if (neg_slope)	x--;
	      else		x++;
	    }
	  set_cell(x,y,ent);
	}
    }
}

CEntity** CMatrix::rect_detect( const Rect& r)
{

  CEntity** hit;
  
  if( (hit = line_detect( r.toplx, r.toply, r.toprx, r.topry)) > 0 ) 
    return hit;
  
  if( (hit = line_detect( r.toprx, r.topry, r.botlx, r.botly)) > 0 )
    return hit;
  
  if( (hit = line_detect( r.botlx, r.botly, r.botrx, r.botry)) > 0 )
    return hit;
  
  if( (hit = line_detect( r.botrx, r.botry, r.toplx, r.toply)) > 0 )
    return hit;

  return 0;
}

CEntity** CMatrix::line_detect(int x1,int y1,int x2,int y2)
{
  int delta_x, delta_y;
  int delta, incE, incNE;
  int x, y;
  int neg_slope = 0;
  CEntity** cell;
  
  if (x1 > x2)
    {
      delta_x = x1 - x2;
      if (y1 > y2)	delta_y = y1 - y2;
      else		delta_y = y2 - y1;
      
      if (delta_y <= delta_x) return( line_detect(x2, y2, x1, y1));
    }
  if (y1 > y2)
    {
      delta_y = y1 - y2;
      if (x1 > x2)	delta_x = x1 - x2;
      else		delta_x = x2 - x1;
      
      if (delta_y > delta_x) return( line_detect(x2, y2, x1, y1));
    }
  
  if (x1 > x2)
    {
      neg_slope = 1;
      delta_x = x1 - x2;
    }
  else
    delta_x = x2 - x1;
  
  if (y1 > y2)
    {
      neg_slope = 1;
      delta_y = y1 - y2;
    }
  else
    delta_y = y2 - y1;
  
  x = x1;
  y = y1;
  
  //check to see if this pixel is an obstacle
  cell = get_cell( x,y );
  if( cell[0] != 0)
   { 
     return cell;
   }

  //set_cell(x,y,col);
  
  if (delta_y <= delta_x)
    {
      delta = 2 * delta_y - delta_x;
      incE = 2 * delta_y;
      incNE = 2 * (delta_y - delta_x);
      
      while (x < x2)
	{
	  if (delta <= 0)
	    {
	      delta = delta + incE;
	      x++;
	    }
	  else
	    {
	      delta = delta + incNE;
	      x++;
	      if (neg_slope)	y--;
	      else		y++;
	    }

	  //check to see if this pixel is an obstacle
	  cell = get_cell( x,y );
	  if( cell[0] != 0)
	    { 
	      return cell;
	    }
	  
	  //set_cell(x,y,col);
	}
    }
  else
    {
      delta = 2 * delta_x - delta_y;
      incE = 2 * delta_x;
      incNE = 2 * (delta_x - delta_y);
      
      while (y < y2) 
	{
	  if (delta <= 0)
	    {
	      delta = delta + incE;
	      y++;
	    }
	  else
	    {
	      delta = delta + incNE;
	      y++;
	      if (neg_slope)	x--;
	      else		x++;
	    }
	  //check to see if this pixel is an obstacle
	  cell = get_cell( x,y );
	  if( cell[0] != 0)
	    { 
	      return cell;
	    }
	  //set_cell(x,y,col);
	}
    }
  return 0;
}


void CMatrix::clear( void )
{
  //cout << "Clear: " << data << ',' << ent << ',' << width*height << endl;
  memset(data,0,width*height*sizeof(CEntity**));
}

inline void CMatrix::set_cell(int x, int y, CEntity* ent )
{    
  if( mode == mode_unset ) // HACK! 
    {
      unset_cell( x, y, ent );
      return;
    }
  
  if( ent == 0 ) return;

  if (x<0 || x>=width || y<0 || y>=height) return;
  
  int cell = x + y * width;
  int slot = current_slot[ cell ];
  
  //printf( "SET %d %p (%d)\n", cell, ent, ent->m_stage_type );
  //printf( "before " );
  //PrintCell( cell );
  
  // if it's already here do nothing
  for( int s=0; s<=slot; s++ )
    if( data[cell][s] == ent ) return;
  
  if( slot < available_slots[ cell ] )
    {
      data[ cell ][ slot ] = ent;
      current_slot[ cell ]++;
    }
  else
    printf( "out of slots! cell:%d slot:%d slots:%d\n",
	    cell, slot, available_slots[cell] );
  
  //printf( "after " );
  //PrintCell( cell );
}

inline void CMatrix::unset_cell(int x, int y, CEntity* ent )
{
  if( ent == 0 ) return;
  if (x<0 || x>=width || y<0 || y>=height) return;
  
  int cell = x + y * width;
  
  //printf( "UNSET %d %p (%d)\n", cell, ent, ent->m_stage_type );
  //printf( "before " );
  //PrintCell( cell );
  
  for( int slot = 0; slot < current_slot[ cell ]; slot++ )
    if( data[cell][slot] == ent )
      {
	// we've found it! now delete by
	// copying the next slot over this one
	// and repeat until we're at the end
	// the last copy copies over the zero'd pointer
	while( data[cell][slot] )
	    data[cell][slot] = data[cell][++slot];
	
	current_slot[ cell ] = slot-1; // zero the new end slot
	
	break;break;
      }
  
  //printf( "after " );
  //PrintCell( cell );  
}



CLineIterator::CLineIterator( double x, double y, double a, double b, 
			      double ppm, CMatrix* matrix, LineIteratorMode pmode )
{   
  m_matrix = matrix;
  m_ppm = ppm;       
  
  m_x = x * m_ppm;
  m_y = y * m_ppm;

  m_ent = 0;
  m_index = 0;

  switch( pmode )
    {
    case PointToBearingRange:
      {
	double range = b;
	double bearing = a;
	
	m_angle = bearing;
	m_max_range = m_remaining_range = range * m_ppm;
      }
      break;
    case PointToPoint:
      {
	double x1 = a;
	double y1 = b;
	
	//printf( "From %.2f,%.2f to %.2f,%.2f\n", x,y,x1,y1 );

	m_angle = atan2( y1-y, x1-x );
	m_max_range = m_remaining_range = hypot( x1-x, y1-y ) * m_ppm;
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
  
  // hops along each axis
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
  //printf( "Hit %p at %.2f,%.2f with %.2f to go\n", 
  //    ent[0], px, py, remaining_range );
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


CRectangleIterator::CRectangleIterator( double x, double y, double th,
					double w, double h,  
					double ppm, CMatrix* matrix )
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
			       ppm, matrix, PointToPoint );
  
  lits[1] = new CLineIterator( corners[1][0], corners[1][1], 
			       corners[2][0], corners[2][1],
			       ppm, matrix, PointToPoint );

  lits[2] = new CLineIterator( corners[2][0], corners[2][1], 
			       corners[3][0], corners[3][1],
			       ppm, matrix, PointToPoint );
  
  lits[3] = new CLineIterator( corners[3][0], corners[3][1], 
			       corners[0][0], corners[0][1],
			       ppm, matrix, PointToPoint );
  
} 


CEntity* CRectangleIterator::GetNextEntity( void )
{
  CEntity* ent = 0;
  
  for( int i=0; i<4; i++ )
    if( (ent = lits[i]->GetNextEntity() ) != 0 )
      break;
  
  return ent;
}

