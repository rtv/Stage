/*************************************************************************
 * RTV
 * $Id: matrix.cc,v 1.9.2.1 2001-12-17 18:11:28 ahoward Exp $
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
    
// construct from width / height 
CMatrix::CMatrix(int w,int h)
{
  initial_buf_size = BUFFER_ALLOC_SIZE;
  //initial_buf_size = 0;

  mode = mode_set;

  width = w; height = h;
  data 	= new CEntity**[width*height];
  used_slots = new unsigned char[ width*height ];
  available_slots = new unsigned char[ width*height ];

  for( int p=0; p< width * height; p++ )
  {
    // create the pointer "strings"
    data[p] = new CEntity*[ initial_buf_size + 1];
    // zero them out
    memset( data[p], 0, (initial_buf_size + 1) * sizeof( CEntity* ) );

    used_slots[p] = 0;
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
      if( data[p] )
        delete[] data[p];
      
    // delete the main array
    delete [] data;
  }
  
  if( available_slots )
    delete [] available_slots;
  if( used_slots )
    delete [] used_slots;
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


// Draw a rectangle
void CMatrix::draw_rect( const Rect& t, CEntity* ent )
{
  draw_line( t.toplx, t.toply, t.toprx, t.topry, ent );
  draw_line( t.toprx, t.topry, t.botrx, t.botry, ent );
  draw_line( t.botrx, t.botry, t.botlx, t.botly, ent );
  draw_line( t.botlx, t.botly, t.toplx, t.toply, ent );
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


// Draw a line from (x1, y1) to (x2, y2) inclusive
void CMatrix::draw_line(int x1,int y1,int x2,int y2, CEntity* ent)
{
  int dx, dy;
  int x, y;
  int incE, incNE;
  int t, delta;
  int neg_slope = 0;
  
  dx = x2 - x1;
  dy = y2 - y1;

  // Draw lines with slope < 45 degrees
  if (abs(dx) > abs(dy))
  {
    if (x1 > x2)
    {
      t = x1; x1 = x2; x2 = t;
      t = y1; y1 = y2; y2 = t;
      dx = -dx;
      dy = -dy;
    }

    if (y1 > y2)
    {
      neg_slope = 1;
      dy = -dy;
    }

    delta = 2 * dy - dx;
    incE = 2 * dy;
    incNE = 2 * (dy - dx);

    x = x1;
    y = y1;
    set_cell(x, y, ent);

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
        if (neg_slope)
          y--;
        else
          y++;
      }
  	  set_cell(x,y,ent);
  	}

    x = x2;
    y = y2;
    set_cell(x, y, ent);
  }

  // Draw lines with slope > 45 degrees
  else
  {
    if (y1 > y2)
    {      t = x1; x1 = x2; x2 = t;
      t = y1; y1 = y2; y2 = t;
      dx = -dx;
      dy = -dy;
    }

    if (x1 > x2)
    {
      neg_slope = 1;
      dx = -dx;
    }

    delta = 2 * dx - dy;
    incE = 2 * dx;
    incNE = 2 * (dx - dy);

    x = x1;
    y = y1;
    set_cell(x, y, ent);

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
        if (neg_slope)
          x--;
        else
          x++;
      }
  	  set_cell(x,y,ent);
  	}

    x = x2;
    y = y2;
    set_cell(x, y, ent);
  }
}


void CMatrix::clear( void )
{
  //cout << "Clear: " << data << ',' << ent << ',' << width*height << endl;
  memset(this->data,0,width*height*sizeof(CEntity**));
  memset(this->used_slots,0,width*height*sizeof(this->used_slots[0])); 
}


inline void CMatrix::set_cell(int x, int y, CEntity* ent )
{    
  if( mode == mode_unset ) // HACK! 
  {
    unset_cell( x, y, ent );
    return;
  }

  if( ent == 0 )
    return;
  if (x<0 || x>=width || y<0 || y>=height) 
  {
    //fputs("Stage: WARNING: CMatrix::set_cell() out of bounds!\n",stderr);
    return;
  }
  
  int cell = x + y * width;
  int used_slots = this->used_slots[cell];
  
  // See it we already have this entity
  for (int slot = 0; slot < this->used_slots[cell]; slot++)
    if (this->data[cell][slot] == ent)
      return;

  // Make sure we have room to add the entity
  if (used_slots >= this->available_slots[cell])
  {
    int oldlen = this->available_slots[cell];
    int newlen = oldlen + BUFFER_ALLOC_SIZE;

    // make the new buffer - has one spare null pointer to mark the end
    CEntity** longer = new CEntity*[ newlen + 1 ];

    // zero the buffer
    // copy the existing data into the new longer buffer
    memset( longer, 0, (newlen+1) * sizeof( CEntity* ) );
    memcpy( longer, this->data[cell], oldlen * sizeof( CEntity* ) );
      
    //delete the old buffer
    delete[] this->data[cell];
      
    // and insert the new buffer
    this->data[cell] = longer;
      
    // remember how many slots we have in this buffer
    this->available_slots[ cell ] = newlen;
  }

  // do the insertion
  data[cell][used_slots] = ent;
  this->used_slots[cell]++;

  // Debugging
  //CheckCell(cell);
  return;
}


inline void CMatrix::unset_cell(int x, int y, CEntity* ent )
{
  if( ent == 0 )
    return;
  if (x<0 || x>=width || y<0 || y>=height) 
  {
    //fputs("Stage: WARNING: CMatrix::unset_cell() out of bounds!\n",stderr);
    //fprintf(stderr,"Stage: WARNING: x,y: %d,%d\t w,h: %d,%d\n",
    //      x,y,width,height);
    return;
  }
  
  int cell = x + y * width;
  int used_slots = this->used_slots[cell];
  
  for (int slot = 0; slot < used_slots; slot++)
  {
    if (this->data[cell][slot] == ent)
    {
      // Move everything down to eliminate this slot.
      // Note that this assumes that there is an end-marker,
      // so we dont need to zero anything.
      memmove(this->data[cell] + slot, this->data[cell] + slot + 1,
              (this->available_slots[cell] - slot) * sizeof(CEntity*));
      this->used_slots[cell]--;
      break;
    }
  }

  // Debugging
  //CheckCell(cell);
}


// Print the cell contents
void CMatrix::PrintCell( int cell )
{
  printf( "data[ %d ] ", cell );
  for (int slot = 0; slot < available_slots[cell]; slot++)
    printf( " %p", data[cell][slot] );
  printf( "\t %d/%d\n", used_slots[cell], available_slots[cell] );
  
  fflush( stdout );
}


// Sanity check: check that each entity is present exactly once
void CMatrix::CheckCell( int cell )
{
  for (int i = 0; i < available_slots[cell]; i++)
  {
    if (data[cell][i] == NULL)
      continue;
    for (int j = i + 1; j < available_slots[cell]; j++)
    {
      if (data[cell][i] == data[cell][j])
      {
        printf("sanity check failed : dupilcate entry\n");
        PrintCell(cell);
      }
    }
  }
  if (data[cell][used_slots[cell]] != NULL)
    printf("sanity check failed : no end marker\n");

}
