/*************************************************************************
 * RTV
 * $Id: matrix.cc,v 1.15.6.4 2003-02-07 05:30:34 rtv Exp $
 ************************************************************************/

#include <math.h>
#include <iostream>
#include <fstream>

//#if INCLUDE_ZLIB
//#include <zlib.h>
//#endif

#include "matrix.hh"
#include "library.hh"

const int BUFFER_ALLOC_SIZE = 1;

//#define DEBUG
    
// construct from width / height 
CMatrix::CMatrix( double width_meters, double height_meters, double ppm,  int default_buf_size)
{
  this->ppm = ppm;
  this->width = (int)(width_meters * ppm) + 1; 
  this->height = (int)(height_meters * ppm) + 1;
  this->default_buf_size = default_buf_size;
  
  AllocateStorage();
}

void CMatrix::AllocateStorage()
{
  assert( data 	= new CEntity**[width*height] );
  assert( used_slots = new unsigned char[ width*height ] );
  assert( available_slots = new unsigned char[ width*height ] );
  
  for( int p=0; p< width * height; p++ )
    {
      // create the pointer "strings"
      assert( data[p] = new CEntity*[ default_buf_size + 1] );
      // zero them out
      memset( data[p], 0, (default_buf_size + 1) * sizeof( CEntity* ) );
      
      used_slots[p] = 0;
      available_slots[p] = default_buf_size;
    }
}

void CMatrix::DeallocateStorage()
{
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
  
  data = NULL;
  available_slots = NULL;
  used_slots = NULL;
}

void CMatrix::Resize(  double width_meters, double height_meters, double ppm )
{
  DeallocateStorage();
  
  this->width = (int)(width_meters * ppm) + 1; 
  this->height = (int)(height_meters * ppm) + 1;
  this->ppm = ppm;
  
  AllocateStorage();
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



// get a pixel color by its x,y coordinate
CEntity** CMatrix::get_cell(int x, int y)
{ 
  //if (x<0 || x>=width || y<0 || y>=height) 
  //{
  //fputs("Stage: WARNING: CEntity::get_cell(int,int) out-of-bounds\n",
  //stderr);
  //return 0;
  //}
  
  return data[x+(y*width)]; 
}

// get a pixel color by its position in the array
CEntity** CMatrix::get_cell( int i)
{ 
  if( i<0 || i > width*height ) 
    {
      fputs("Stage: WARNING: CEntity::get_cell(int) out-of-bounds\n",stderr);
      return 0;
    }
  return data[i]; 
}

// is there an object of this type here?
inline bool CMatrix::is_type( int x, int y, int type )
{ 
  //if( i<0 || i > width*height ) return 0;
  
  CEntity** cell = data[x+(y*width)];
  
  while( *cell )
    {
      if( (*cell)->lib_entry->type_num == type ) return true;
      cell++;
    }
    
  return false;
}


// useful debug function allows plotting the world externally
void CMatrix::dump( void )
{
  std::ofstream out( "world.dump" );
  
  for( int y=0; y<height; y++ )
  {
    for( int x=0; x<width; x++ )
    {
	  
      CEntity** ent = get_cell( x,y );
	  
      //while( *ent )
      if( *ent )
	    {   
	      out << x << ' ' << y;
	      out << ' ' << (int)*ent << std::endl;
	    }
    }
  }

  out.close();

  puts( "DUMPED" );
}


// Draw a rectangle
void CMatrix::draw_rect( const stage_rect_t& t, CEntity* ent, bool add)
{
  draw_line( t.toplx, t.toply, t.toprx, t.topry, ent, add);
  draw_line( t.toprx, t.topry, t.botrx, t.botry, ent, add);
  draw_line( t.botrx, t.botry, t.botlx, t.botly, ent, add);
  draw_line( t.botlx, t.botly, t.toplx, t.toply, ent, add);
}


// draws (2*PI)/0.1 = 62 little lines to form a circle
void CMatrix::draw_circle(int x,int y,int r, CEntity* ent, bool add)
{
  double i,cx,cy;
  int x1,y1,x2,y2;
  
  x1=x;y1=y+r;	
  
  for (i=0;i<2.0*M_PI;i+=0.1)
  {
    cx = (double) x + (double (r) * sin(i));
    cy = (double) y + (double (r) * cos(i));
    x2=(int)cx; y2=(int)cy;
    draw_line(x1, y1, x2, y2, ent, add);
    x1=x2; y1=y2;
  }	
  
  draw_line(x1, y1, x, y+r, ent, add);
}


// Draw a line from (x1, y1) to (x2, y2) inclusive
void CMatrix::draw_line(int x1,int y1,int x2,int y2, CEntity* ent, bool add)
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
    if (add)
      set_cell(x, y, ent);
    else
      unset_cell(x, y, ent);

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
      if (add)
        set_cell(x, y, ent);
      else
        unset_cell(x, y, ent);
  	}

    x = x2;
    y = y2;
    if (add)
      set_cell(x, y, ent);
    else
      unset_cell(x, y, ent);
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
    if (add)
      set_cell(x, y, ent);
    else
      unset_cell(x, y, ent);

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
      if (add)
        set_cell(x, y, ent);
      else
        unset_cell(x, y, ent);
  	}

    x = x2;
    y = y2;
    if (add)
      set_cell(x, y, ent);
    else
      unset_cell(x, y, ent);
  }
}


void CMatrix::clear( void )
{
  memset(this->data,0,width*height*sizeof(CEntity**));
  memset(this->used_slots,0,width*height*sizeof(this->used_slots[0])); 
}


inline void CMatrix::set_cell(int x, int y, CEntity* ent )
{
  if( ent == 0 )
    return;
  if (x<0 || x>=width || y<0 || y>=height) 
    return;
  
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
    return;
  
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

///////////////////////////////////////////////////////////////////////////
// Set a rectangle in the world grid
void CMatrix::SetRectangle(double px, double py, double pth,
			   double dx, double dy, 
			   CEntity* ent, bool add)
{
  stage_rect_t rect;

  dx /= 2.0;
  dy /= 2.0;

  double cx = dx * cos(pth);
  double cy = dy * cos(pth);
  double sx = dx * sin(pth);
  double sy = dy * sin(pth);
    
  rect.toplx = (int) ((px + cx - sy) * ppm);
  rect.toply = (int) ((py + sx + cy) * ppm);

  rect.toprx = (int) ((px + cx + sy) * ppm);
  rect.topry = (int) ((py + sx - cy) * ppm);

  rect.botlx = (int) ((px - cx - sy) * ppm);
  rect.botly = (int) ((py - sx + cy) * ppm);

  rect.botrx = (int) ((px - cx + sy) * ppm);
  rect.botry = (int) ((py - sx - cy) * ppm);
    
  //printf( "draw_rect %d,%d %d,%d %d,%d %d,%d\n",
  //  rect.toplx, rect.toply,
  //  rect.toprx, rect.topry,
  //  rect.botlx, rect.botly,
  //  rect.botrx, rect.botry );

  draw_rect( rect, ent, add );
}


///////////////////////////////////////////////////////////////////////////
// Set a circle in the world grid
void CMatrix::SetCircle(double px, double py, double pr, 
			CEntity* ent, bool add )
{
  // Convert from world to image coords
  int x = (int) (px * ppm);
  int y = (int) (py * ppm);
  int r = (int) (pr * ppm);
    
  draw_circle( x,y,r,ent, add);
}

