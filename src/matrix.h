// ==================================================================
// Filename:	CMatrix.h
//
// $Id: matrix.h,v 1.1 2004-02-29 04:06:33 rtv Exp $
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

//#include "stage.h"


typedef struct 
{
  int width, height;
  double ppm;
  GPtrArray* data;
} stg_matrix_t;


stg_matrix_t* stg_matrix_create( int width, int height, double ppm );

// frees all memory allocated by the matrix; first the cells, then the
// cell array.
void stg_matrix_destroy( stg_matrix_t* matrix );

// removes all pointers from every cell in the matrix
void stg_matrix_clear( stg_matrix_t* matrix );

// get the array of pointers in cell y*width+x
GPtrArray* stg_matrix_cell( stg_matrix_t* matrix, int x, int y);

// append the pointer <object> to the pointer array at the cell
void stg_matrix_cell_append(  stg_matrix_t* matrix, 
			      int x, int y, void* object );

// if <object> appears in the cell's array, remove it
void stg_matrix_cell_remove(  stg_matrix_t* matrix,
			      int x, int y, void* object );

// these append to the <object> pointer to the cells on the edge of a
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

//void stg_matrix_circle( stg_matrix_t* matrix,
//		double px, double py, double pr, 
//		void* object, bool add );
//};

#ifdef __cplusplus
  }
#endif 

#endif
