// ==================================================================
// Filename:	CMatrix.h
//
// $Id: matrix.hh,v 1.4.6.7 2003-08-09 01:25:20 rtv Exp $
// RTV
// ==================================================================

#ifndef _MATRIX_CLASS_
#define _MATRIX_CLASS_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stage.h"
#include "entity.hh"

class CMatrix
{
  public:
  int	width;
  int	height;

  private:
  CEntity***  data;
  unsigned char* used_slots;
  unsigned char* available_slots;
  int default_buf_size;
  public:

  CMatrix( double width_meters, double height_meters, double ppm, int default_buf_size);
  ~CMatrix(void);
  
  inline int get_width(void) {return width;}
  inline int get_height(void) {return height;}
  inline int get_size(void) {return width*height;}
  
  double ppm; // pixels per meter - matrix does its own scaling now.

  // MUST BE IN-BOUNDS!

  // get a pixel color by its x,y coordinate
  CEntity** get_cell(int x, int y);

  // get a pixel color by its position in the array
  CEntity** get_cell( int i);

  // is there an object of this type here?
  bool is_type( int x, int y, int type );

  void set_cell(int x, int y, CEntity* ent );
  void unset_cell(int x, int y, CEntity* ent );
  
  void	copy_from(CMatrix* img);

  void	draw_line(int x1, int y1, int x2, int y2, CEntity* ent, bool add);
  void	draw_rect( const stg_rect_t& t, CEntity* ent, bool add );
  void	draw_circle(int x, int y, int r, CEntity* ent, bool add);
  
  void	clear( void );

// utilities for visualizing the matrix
  void dump( void );

  void PrintCell( int cell );
  void CheckCell( int cell );

  // these have been moved from CWorld - they draw shapes measured in
  // meters about a center point
  void SetRectangle(double px, double py, double pth,
		    double dx, double dy, 
		    CEntity* ent, bool add);

  void SetCircle(double px, double py, double pr, 
		 CEntity* ent, bool add );
  
  void AllocateStorage();
  void DeallocateStorage();
  void Resize(  double width_meters, double height_meters, double ppm );
};


#endif
