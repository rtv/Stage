/*************************************************************************
 * RTV
 * $Id: matrix.cc,v 1.9 2001-09-29 00:47:12 vaughan Exp $
 ************************************************************************/

#include <math.h>
#include <iostream.h>
#include <fstream.h>

//#if INCLUDE_ZLIB
//#include <zlib.h>
//#endif

#include "matrix.hh"

const int BUFFER_ALLOC_SIZE = 1;

//#define DEBUG

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
  initial_buf_size = BUFFER_ALLOC_SIZE;
  //initial_buf_size = 0;

  mode = mode_set;

  width = w; height = h;
  data 	= new CEntity**[width*height];
  current_slot = new unsigned char[ width*height ];
  available_slots = new unsigned char[ width*height ];

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
  // if we allocated the array
  if (data)
    {
      // delete the storage in each cell
      for( int p=0; p< width * height; p++ )
	if( data[p] ) delete[] data[p];
      
      // delete the main array
      delete [] data;
    }
  
  if( available_slots )  delete [] available_slots;
  if( current_slot )    delete [] current_slot;
}


// useful debug function allows plotting the world externally
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
	}
    }

  out.close();

  puts( "DUMPED" );
}


// draws (2*PI)/0.1 = 62 little lines to form a circle
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
  draw_line( t.toprx, t.topry, t.botrx, t.botry, ent );
  draw_line( t.botrx, t.botry, t.botlx, t.botly, ent );
  draw_line( t.botlx, t.botly, t.toplx, t.toply, ent );
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

  if (x<0 || x>=width || y<0 || y>=height) 
  {
    //fputs("Stage: WARNING: CMatrix::set_cell() out of bounds!\n",stderr);
    return;
  }
  
  int cell = x + y * width;
  int slot = current_slot[ cell ];
  
  //printf( "SET %d %p (%d)\n", cell, ent, ent->m_stage_type );
  //printf( "before " );
  //PrintCell( cell );
  
  // if it's already here do nothing
  for( int s=0; s<=slot; s++ )
    if( data[cell][s] == ent ) return;
  
  // if we're out of room. make some more space
  if( !(slot < available_slots[ cell ]) )
    {
      //printf( "out of slots! cell:%d slot:%d slots:%d\n",
      //      cell, slot, available_slots[cell] );
      
      // set the new buffer size
      int oldlen = available_slots[cell];
      int newlen = oldlen + BUFFER_ALLOC_SIZE;
      //printf("resizing old:%d\tnew:%d\n", oldlen,newlen);
      
      // make the new buffer - has one spare null pointer to mark the end
      CEntity** longer = new CEntity*[ newlen + 1 ];
      
      // zero the buffer
      memset( longer, 0, (newlen+1) * sizeof( CEntity* ) );
      
      //copy the existing data into the new longer buffer
      memcpy( longer, data[cell], oldlen * sizeof( CEntity* ) );
      
      //delete the old buffer
      delete[] data[cell];
      
      // and insert the new buffer
      data[cell] = longer;
      
      // remember how many slots we have in this buffer
      available_slots[ cell ] = newlen;
    }

  // do the insertion
  data[ cell ][ slot ] = ent;
  current_slot[ cell ]++;
 
  //printf( "after " );
  //PrintCell( cell );
}

inline void CMatrix::unset_cell(int x, int y, CEntity* ent )
{
  if( ent == 0 ) return;
  if (x<0 || x>=width || y<0 || y>=height) 
  {
    //fputs("Stage: WARNING: CMatrix::unset_cell() out of bounds!\n",stderr);
    //fprintf(stderr,"Stage: WARNING: x,y: %d,%d\t w,h: %d,%d\n",
    //      x,y,width,height);
    return;
  }
  
  int cell = x + y * width;
  
  //printf( "UNSET %d %p (%d)\n", cell, ent, ent->m_stage_type );
  //printf( "before " );
  //PrintCell( cell );
  
  int current =  current_slot[ cell ];
  int end = available_slots[ cell ];

  for( int slot = 0; slot < current; slot++ )
    if( data[cell][slot] == ent )
      {
	for( int s=slot; slot<end; slot++ )
	  data[cell][s] = data[cell][s+1];
	
	current--; // reduce the slot count
	
	// zero from the current slot to the end
	for( int h = current; h < end; h++ )
	  data[cell][h] = NULL;
	
	// set current again;
	current_slot[ cell ] = current;
	
	break; // dump out
      }

  
  //printf( "after " );
  //PrintCell( cell );  
}


