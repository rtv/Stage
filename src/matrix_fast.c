/*************************************************************************
 * RTV
 * $Id: matrix_fast.c,v 1.2 2004-09-28 05:28:43 rtv Exp $
 ************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h> // for memcpy(3)

#include "stage.h"


//#define DEBUG

// if this is defined, empty matrix cells are deleted, saving memory
// at the cost of a little time. If you turn this off - Stage may use
// a LOT of memory if it runs a long time in large worlds.
#define STG_DELETE_EMPTY_CELLS 1

// basic integer drawing functions (not intended for external use)
void draw_line( stg_matrix_t* matrix,
		int x1, int y1, int x2, int y2, void* object, int add);


stg_matrix_t* stg_matrix_create( double ppm, double medppm, double bigppm )
{
  stg_matrix_t* matrix = (stg_matrix_t*)calloc(1,sizeof(stg_matrix_t));
  
  
  int base_size = 1000;
  
  matrix->array_width = base_size;
  matrix->array_height = base_size;
  size_t len = matrix->array_width * matrix->array_height;

  matrix->array = g_ptr_array_sized_new( len );  // allocate the VAST fast array
  
  // create a pointer array at every cell in the array
  size_t i;
  for( i=0; i<len; i++ )
      g_ptr_array_add( matrix->array, g_ptr_array_new() );
    
  matrix->ppm = ppm;
  matrix->medppm = medppm;  
  matrix->bigppm = bigppm;

  return matrix;
}

// frees all memory allocated by the matrix; first the cells, then the
// cell array.
void stg_matrix_destroy( stg_matrix_t* matrix )
{
  g_ptr_array_free( matrix->array, TRUE );
  free( matrix );
}

// removes all pointers from every cell in the matrix
void stg_matrix_clear( stg_matrix_t* matrix )
{  
  //g_hash_table_destroy( matrix->table );
  //matrix->table = g_hash_table_new_full( matrix_hash, matrix_equal,
  //				 matrix_key_destroy, matrix_value_destroy );
}


GPtrArray* stg_matrix_table_add_cell( GHashTable* table, glong x, glong y )
{
  stg_matrix_coord_t* coord = calloc( sizeof(stg_matrix_coord_t), 1 );
  coord->x = x;
  coord->y = y;
  
  GPtrArray* cell = g_ptr_array_new();
  g_hash_table_insert( table, coord, cell );
  
  return cell;
}


// get the array of pointers in cell y*width+x, specified in meters
GPtrArray* stg_matrix_bigcell_get( stg_matrix_t* matrix, double x, double y)
{
  return stg_matrix_cell_get( matrix, x, y );
}

// get the array of pointers in cell y*width+x, specified in meters
GPtrArray* stg_matrix_medcell_get( stg_matrix_t* matrix, double x, double y)
{
  return stg_matrix_cell_get( matrix, x, y );
}


// get the array of pointers in cell y*width+x, specified in meters
GPtrArray* stg_matrix_cell_get( stg_matrix_t* matrix, double x, double y)
{
  guint xcell = floor(x*matrix->ppm) + matrix->array_width/2;
  guint ycell = floor(y*matrix->ppm) + matrix->array_height/2;

  // bounds check
  if( xcell < 0 || xcell >= matrix->array_width )
    return NULL;

  if( ycell < 0 || ycell > matrix->array_height )
    return NULL;

  guint index =  xcell + matrix->array_width * ycell;
  
  //printf( "returning array at index: %d\n", index );

  return g_ptr_array_index( matrix->array, index );
}

// append the pointer <object> to the pointer array at the cell
void stg_matrix_cell_append(  stg_matrix_t* matrix, 
			      double x, double y, void* object )
{
  GPtrArray* cell = stg_matrix_cell_get( matrix, x, y );  
  
  if( cell  )
    {
      //printf( "adding an object to cell at (%.2f %.2f)\n", x, y );
      
      // add us to the cell, but only once
      g_ptr_array_remove_fast( cell, object );
      g_ptr_array_add( cell, object );  
      
    }
  else
    PRINT_ERR2( "BOUNDS ERROR at %.2f %.2f", x, y  );

  // todo - record a timestamp for matrix mods so devices can see if
  //the matrix has changed since they last peeked into it
  // matrix->last_mod =

  //printf( "appending %p to matrix at %d %d. %d pointers now cell\n", 
  //  object, (int)x, (int)y, cell->len );
}

// if <object> appears in the cell's array, remove it
void stg_matrix_cell_remove( stg_matrix_t* matrix, 
			     double x, double y, void* object )
{ 
  GPtrArray* cell = stg_matrix_cell_get( matrix, x, y );
  
  if( cell )
    {
      g_ptr_array_remove_fast( cell, object );
    }
  else
    PRINT_ERR2( "BOUNDS ERROR at %.2f %.2f", x, y  );
}

// draw or erase a straight line between x1,y1 and x2,y2.
void stg_matrix_line( stg_matrix_t* matrix,
		      double x1,double y1,double x2,double y2, 
		      void* object, int add )
{
  double xdiff = x2 - x1;
  double ydiff = y2 - y1;

  double angle = atan2( ydiff, xdiff );
  double len = hypot( ydiff, xdiff );

  double cosa = cos(angle);
  double sina = sin(angle);
  
  double xincr = cosa * 1.0 / matrix->ppm;
  double yincr = sina * 1.0 / matrix->ppm;
  
  double xx = 0;
  double yy = 0;
  
  double finalx, finaly;

  while( len > hypot( yy,xx ) )
    {
      finalx = x1+xx;
      finaly = y1+yy;
      
      // bounds check
      //if( finalx >= 0 && finaly >= 0 )
	{
	  if( add ) 
	    stg_matrix_cell_append( matrix, finalx, finaly, object ); 
	  else
	    stg_matrix_cell_remove( matrix, finalx, finaly, object ); 
	}
      
      xx += xincr;
      yy += yincr;      
    }
}

void stg_matrix_lines( stg_matrix_t* matrix, 
		       stg_line_t* lines, int num_lines,
		       void* object, int add )
{
  int l;
  for( l=0; l<num_lines; l++ )
    stg_matrix_line( matrix, 
		     lines[l].x1, lines[l].y1, 
		     lines[l].x2, lines[l].y2, 
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


