/*************************************************************************
 * RTV
 * $Id: matrix.c,v 1.1 2004-02-29 04:06:33 rtv Exp $
 ************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "matrix.h"

//#define DEBUG

// basic integer drawing functions (not intended for external use)
void draw_line( stg_matrix_t* matrix,
		int x1, int y1, int x2, int y2, void* object, int add);
//void draw_rect( const stg_rect_t& t, void* object, int add );
//void draw_circle(int x, int y, int r, void* object, int add);



stg_matrix_t* stg_matrix_create( int width, int height, double ppm )
{
  stg_matrix_t* matrix = (stg_matrix_t*)calloc(1,sizeof(stg_matrix_t));

  matrix->ppm = ppm;
  
  matrix->width = width / ppm;
  matrix->height = height / ppm;
  
  matrix->cell_count = width*height;


    // the matrix is an array of (width*height) cells each containing a
  // pointer to a GPtrArray.
  matrix->data = g_ptr_array_sized_new( width*height );
  
  // create a pointer array at each pixel in the matrix
  int c;
  for( c=0; c<width*height; c++ )
    g_ptr_array_add( matrix->data, g_ptr_array_new() );

  return matrix;
}

// frees all memory allocated by the matrix; first the cells, then the
// cell array.
void stg_matrix_destroy( stg_matrix_t* matrix )
{
  int c;
  for( c=0; c<matrix->width*matrix->height; c++ )
    g_ptr_array_free( g_ptr_array_index(matrix->data,c), 0 );
  
  g_ptr_array_free( matrix->data, 0 );
  free( matrix );
}

// removes all pointers from every cell in the matrix
void stg_matrix_clear( stg_matrix_t* matrix )
{  int c;
 for( c=0; c<matrix->width*matrix->height; c++ )
   g_ptr_array_set_size( g_ptr_array_index(matrix->data,c), 0 ); 
}

// get the array of pointers in cell y*width+x
GPtrArray* stg_matrix_cell( stg_matrix_t* matrix, int x, int y)
{
  return( g_ptr_array_index( matrix->data, x+matrix->width*y ) );
}

// append the pointer <object> to the pointer array at the cell
void stg_matrix_cell_append(  stg_matrix_t* matrix, 
			      int x, int y, void* object )
{
  int index = x+matrix->width*y;
  
  if( index < 0 )
    {
      printf( "matrix index OOB < 0 %d\n", index );
      return;
    }
  
  if( index >= matrix->data->len ) 
    { 
      printf( "matrix index OOB >= %d %d\n", 
	      matrix->data->len, index );
      
      return;
    }
  
  GPtrArray** par =  &g_ptr_array_index( matrix->data, x+matrix->width*y );
  
  if( *par == NULL )
    *par = g_ptr_array_new();
  
  g_ptr_array_add( *par, object );
}

// if <object> appears in the cell's array, remove it
void stg_matrix_cell_remove(  stg_matrix_t* matrix,
			      int x, int y, void* object )
{ 
  int index = x+matrix->width*y;
  
  if( index < 0 )
    {
      printf( "matrix index OOB < 0 %d\n", index );
      return;
    }
  
  if( index >= matrix->data->len ) 
    { 
      printf( "matrix index OOB >= %d %d\n", 
	      matrix->data->len, index );
      
      return;
    }
  
  g_ptr_array_remove_fast( g_ptr_array_index(matrix->data, 
					     x + y*matrix->width), 
			   object );
}


// Draw a line from (x1, y1) to (x2, y2) inclusive
void draw_line( stg_matrix_t* matrix,
		int x1,int y1,int x2,int y2, 
		void* object, int add )
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
      stg_matrix_cell_append( matrix, x, y, object);
    else
      stg_matrix_cell_remove( matrix, x, y, object);

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
        stg_matrix_cell_append( matrix, x, y, object);
      else
        stg_matrix_cell_remove( matrix, x, y, object);
  	}

    x = x2;
    y = y2;
    if (add)
      stg_matrix_cell_append( matrix, x, y, object);
    else
      stg_matrix_cell_remove( matrix, x, y, object);
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
      stg_matrix_cell_append( matrix, x, y, object);
    else
      stg_matrix_cell_remove( matrix, x, y, object);

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
        stg_matrix_cell_append( matrix, x, y, object);
      else
        stg_matrix_cell_remove( matrix, x, y, object);
  	}

    x = x2;
    y = y2;
    if (add)
      stg_matrix_cell_append( matrix, x, y, object);
    else
      stg_matrix_cell_remove( matrix, x, y, object);
  }
}


void stg_matrix_line( stg_matrix_t* matrix, 
		      double x1, double y1,
		      double x2, double y2, 
		      void* object, int add )
{
  draw_line( matrix, 
	     (int)(x1*matrix->ppm), (int)(y1*matrix->ppm),
	     (int)(x2*matrix->ppm), (int)(y2*matrix->ppm),
	     object, add );
}

void stg_matrix_rectangle( stg_matrix_t* matrix, 
			   double px, double py, double pth,
			   double dx, double dy, 
			   void* object, int add )
{
  dx /= 2.0;
  dy /= 2.0;

  double cx = dx * cos(pth);
  double cy = dy * cos(pth);
  double sx = dx * sin(pth);
  double sy = dy * sin(pth);
    
  double toplx =  px + cx - sy;
  double toply =  py + sx + cy;

  double toprx =  px + cx + sy;
  double topry =  py + sx - cy;

  double botlx =  px - cx - sy;
  double botly =  py - sx + cy;

  double botrx =  px - cx + sy;
  double botry =  py - sx - cy;
    
  stg_matrix_line( matrix, toplx, toply, toprx, topry, object, add);
  stg_matrix_line( matrix, toprx, topry, botrx, botry, object, add);
  stg_matrix_line( matrix, botrx, botry, botlx, botly, object, add);
  stg_matrix_line( matrix, botlx, botly, toplx, toply, object, add);

  //printf( "SetRectangle drawing %d,%d %d,%d %d,%d %d,%d\n",
  //  toplx, toply,
  //  toprx, topry,
  //  botlx, botly,
  //  botrx, botry );
}


