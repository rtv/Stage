// ==================================================================
// Filename:	CMatrix.h
//
// $Id: matrix.h,v 1.4 2004-06-09 02:32:08 rtv Exp $
// RTV
// ==================================================================

#ifndef _MATRIX
#define _MATRIX

#ifdef __cplusplus
extern "C" {
#endif 
  
  //#include <stdio.h>
  //#include <stdlib.h>
  //#include <string.h>
  
#include <glib.h>
  
#include "stage.h"
   

typedef struct 
{
  double ppm; // pixels per meter (1/resolution)
  GHashTable* table;
  
  double medppm;
  GHashTable* medtable;

  double bigppm;
  GHashTable* bigtable;

} stg_matrix_t;
  
typedef struct
{
  gulong x; // address a very large space of cells
  gulong y;
} stg_matrix_coord_t;


stg_matrix_t* stg_matrix_create( double ppm, double medppm, double bigppm );

// frees all memory allocated by the matrix; first the cells, then the
// cell array.
void stg_matrix_destroy( stg_matrix_t* matrix );

// removes all pointers from every cell in the matrix
void stg_matrix_clear( stg_matrix_t* matrix );

// get the array of pointers in cell y*width+x
//GPtrArray* stg_matrix_cell( stg_matrix_t* matrix, gulong x, gulong y);

GPtrArray* stg_matrix_cell_get( stg_matrix_t* matrix, double x, double y);
GPtrArray* stg_matrix_bigcell_get( stg_matrix_t* matrix, double x, double y);

// append the [object] to the pointer array at the cell
//void stg_matrix_cell_append(  stg_matrix_t* matrix, 
//		      gulong x, gulong y, void* object );
void stg_matrix_cell_append(  stg_matrix_t* matrix, 
			      double x, double y, void* object );

// if [object] appears in the cell's array, remove it
void stg_matrix_cell_remove(  stg_matrix_t* matrix,
			      double x, double y, void* object );

// these append to the [object] pointer to the cells on the edge of a
// shape. shapes are described in meters about a center point. They
// use the matrix.ppm value to scale from meters to matrix pixels.
void stg_matrix_rectangle( stg_matrix_t* matrix,
			   double px, double py, double pth,
			   double dx, double dy, 
			   void* object, int add );

void stg_matrix_line( stg_matrix_t* matrix, 
		      double x1, double y1, 
		      double x2, double y2,
		       void* object, int add );

void stg_matrix_lines( stg_matrix_t* matrix, 
		       stg_line_t* lines, int num_lines,
		       void* object, int add );

//void stg_matrix_circle( stg_matrix_t* matrix,
//		double px, double py, double pr, 
//		void* object, bool add );
//};

#ifdef __cplusplus
  }
#endif 

#endif
