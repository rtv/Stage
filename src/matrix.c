/*************************************************************************
 * RTV
 * $Id: matrix.c,v 1.2 2004-04-05 03:00:26 rtv Exp $
 ************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h> // for memcpy(3)

#include "matrix.h"

//#define DEBUG

// basic integer drawing functions (not intended for external use)
void draw_line( stg_matrix_t* matrix,
		int x1, int y1, int x2, int y2, void* object, int add);
//void draw_rect( const stg_rect_t& t, void* object, int add );
//void draw_circle(int x, int y, int r, void* object, int add);

void matrix_key_destroy( gpointer key )
{
  free( key );
}

void matrix_value_destroy( gpointer value )
{
  g_ptr_array_free( (GPtrArray*)value, TRUE );
}

// compress the two coordinates into the two halves of a single guint
guint matrix_hash( gconstpointer key )
{
  stg_matrix_coord_t* coord = (stg_matrix_coord_t*)key;
  
  gushort x = (gushort)coord->x; // ignore overflow
  gushort y = (gushort)coord->y; // ignore overflow
  
  //printf( " %d %d hash %d\n", x, y, ( (x << 16) | y ) );

  return( (x << 16) | y );
}

// returns TRUE if the two coordinates are the same, else FALSE
gboolean matrix_equal( gconstpointer c1, gconstpointer c2 )
{
  stg_matrix_coord_t* mc1 = (stg_matrix_coord_t*)c1;
  stg_matrix_coord_t* mc2 = (stg_matrix_coord_t*)c2;
  
  return( mc1->x == mc2->x && mc1->y == mc2->y );  
}

stg_matrix_t* stg_matrix_create( double ppm )
{
  stg_matrix_t* matrix = (stg_matrix_t*)calloc(1,sizeof(stg_matrix_t));
  
  matrix->table = g_hash_table_new_full( matrix_hash, matrix_equal,
					 matrix_key_destroy, matrix_value_destroy );  
  matrix->ppm = ppm;
  return matrix;
}

// frees all memory allocated by the matrix; first the cells, then the
// cell array.
void stg_matrix_destroy( stg_matrix_t* matrix )
{
  g_hash_table_destroy( matrix->table );
  free( matrix );
}

// removes all pointers from every cell in the matrix
void stg_matrix_clear( stg_matrix_t* matrix )
{  
  g_hash_table_destroy( matrix->table );
  matrix->table = g_hash_table_new_full( matrix_hash, matrix_equal,
					 matrix_key_destroy, matrix_value_destroy );
}

// get the array of pointers in cell y*width+x
GPtrArray* stg_matrix_cell( stg_matrix_t* matrix, gulong x, gulong y)
{
  stg_matrix_coord_t coord;
  coord.x = x;
  coord.y = y;
   
  return g_hash_table_lookup( matrix->table, &coord );
}

// get the array of pointers in cell y*width+x, specified in meters
GPtrArray* stg_matrix_cell_m( stg_matrix_t* matrix, double x, double y)
{
  stg_matrix_coord_t coord;
  coord.x = (gulong)(x * matrix->ppm);
  coord.y = (gulong)(y * matrix->ppm);
   
  //printf( "inspecting %d,%d contents %p\n", 
  //  coord.x, coord.y,  g_hash_table_lookup( matrix->table, &coord ) );
  
  return g_hash_table_lookup( matrix->table, &coord );
}


void stg_matrix_cell_append_m( stg_matrix_t* matrix, 
			       double x, double y, void* object )
{
  stg_matrix_cell_append( matrix, 
			  (gulong)(x*matrix->ppm),
			  (gulong)(y*matrix->ppm), 
			  object );
}

// append the pointer <object> to the pointer array at the cell
void stg_matrix_cell_append(  stg_matrix_t* matrix, 
			      gulong x, gulong y, void* object )
{
  GPtrArray* cell = stg_matrix_cell( matrix, x, y );
  
  if( cell == NULL )
    {
      stg_matrix_coord_t* coord = calloc( sizeof(stg_matrix_coord_t), 1 );
      coord->x = x;
      coord->y = y;

      cell = g_ptr_array_new();
      g_hash_table_insert( matrix->table, coord, cell );
    }
  else // make sure we're only in the cell once 
    g_ptr_array_remove_fast( cell, object );
  
  g_ptr_array_add( cell, object );  
  
  //printf( "appending %p to matrix at %d %d. %d pointers now cell\n", 
  //  object, (int)x, (int)y, cell->len );
}

void stg_matrix_cell_remove_m( stg_matrix_t* matrix, 
			       double x, double y, void* object )
{
  stg_matrix_cell_remove( matrix, 
			  (gulong)(x*matrix->ppm),
			  (gulong)(y*matrix->ppm), 
			  object );
}

// if <object> appears in the cell's array, remove it
void stg_matrix_cell_remove(  stg_matrix_t* matrix,
			      gulong x, gulong y, void* object )
{ 
  GPtrArray* cell = stg_matrix_cell( matrix, x, y );
  
  if( cell )
    {
      //printf( "removing %p from %d,%d. %d pointers remain here\n", 
      //      object, x, y, cell->len );

      g_ptr_array_remove_fast( cell, object );

      if( cell->len == 0 )
	{
	  stg_matrix_coord_t coord;
	  coord.x = x;
	  coord.y = y;
	  
	  g_hash_table_remove( matrix->table, &coord );
	}
    }
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


