/*************************************************************************
 * image.cc - bitmap image class CMatrix with processing functions
 *            originally by Neil Sumpter and others at U.Leeds, UK.
 * RTV
 * $Id: matrix.cc,v 1.2.2.1 2001-08-17 21:00:16 vaughan Exp $
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
  
  puts( "" );
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

  //cout << "constructed NImage" << endl;
}

// construct from width / height with existing data
//  CMatrix::CMatrix(unsigned char* array, int w,int h)
//  {
//  #ifdef DEBUG
//    cout << "Image: " << width << 'x' << height << flush;
//  #endif

//    width 	= w;
//    height 	= h;
//    data 		= array;

//  #ifdef DEBUG
//    cout << '@' << data << endl;
//  #endif
//  }

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


void CMatrix::draw_rect( const Rect t, CEntity* ent )
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
  int slots = available_slots[ cell ];
  
  printf( "SET %p\n", ent );
  printf( "before " );
  PrintCell( cell );
  
  // if it's already here do nothing
  for( int s=0; s<=slot; s++ )
    if( data[cell][s] == ent ) return;
  
  // old sticky bit stuff
  //unsigned char *p = data + x + y * width;
  //*p = (*p & 0x80) | c;
  
  if( slot < slots )
    {
      //printf( "Putting %p in cell:%d slot:%d slots:%d\n",
      //ent, cell, slot, slots );
      
      data[ cell ][ slot ] = ent;
      
      current_slot[ cell ]++;
      
      //printf( "Now cell:%d slot:%d slots:%d\n",
      //cell, current_slot[ cell ], available_slots[ cell ] );
    }
  else
    printf( "out of slots! cell:%d slot:%d slots:%d\n",
	    cell, slot, slots );
  
  printf( "after " );
  PrintCell( cell );
}

inline void CMatrix::unset_cell(int x, int y, CEntity* ent )
{
  if( ent == 0 ) return;
  if (x<0 || x>=width || y<0 || y>=height) return;
  
  
  int cell = x + y * width;
  
  printf( "UNSET %p\n", ent );
  printf( "before " );
  PrintCell( cell );
  
  for( int slot = 0; slot < current_slot[ cell ]; slot++ )
    if( data[cell][slot] == ent )
      {
	// we've found it! now delete by
	// copying the next slot over this one
	// and repeat until we're at the end
	// the last copy copies over the zero'd pointer
	while( data[cell][slot] )
	  {
	    data[cell][slot] = data[cell][++slot];
	    //data[cell][slot] = 0;
	  }		  
	
	current_slot[ cell ] = slot; // zero the new end slot
	
	break;break;
      }
  
  printf( "after " );
  PrintCell( cell );  
}
