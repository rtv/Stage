// ==================================================================
// Filename:	CMatrix.h
//
// $Id: matrix.hh,v 1.3 2001-09-20 18:29:22 vaughan Exp $
// RTV
// ==================================================================

#ifndef _MATRIX_CLASS_
#define _MATRIX_CLASS_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "image.hh" // for Rect
#include "entity.hh"


enum MatrixMode { mode_set, mode_unset };

class CMatrix
{
public:
  int	width;
  int	height;
  
  CEntity***  data;
  unsigned char* current_slot;
  unsigned char* available_slots;
  
  char*	win_title;
  
  int initial_buf_size;
  
  void dump( void );
  
  MatrixMode mode;
  void PrintCell( int cell );
  
  //CMatrix();
  CMatrix(int w,int h);
  //CMatrix(CMatrix*);
  
  inline int get_width(void) {return width;}
  inline int get_height(void) {return height;}
  inline int get_size(void) {return width*height;}
  
  // MUST BE IN-BOUNDS! -- turned off checking for speed

  // get a pixel color by its x,y coordinate
  inline CEntity** get_cell(int x, int y)
  { 
    //if (x<0 || x>=width || y<0 || y>=height) return 0;
    return data[x+(y*width)]; 
  }
  
  // get a pixel color by its position in the array
  inline CEntity** get_cell( int i)
  { 
    //if( i<0 || i > width*height ) return 0;
    return data[i]; 
  }
  
  // is there an object of this type here?
  inline bool is_type( int x, int y, StageType type )
  { 
    //if( i<0 || i > width*height ) return 0;
    
    CEntity** cell = data[x+(y*width)];
    
    while( *cell )
      {
	if( (*cell)->m_stage_type == type ) return true;
	cell++;
      }
    
    return false;
  }
  

  inline void set_cell(int x, int y, CEntity* ent );
  inline void unset_cell(int x, int y, CEntity* ent );
  
  inline void SetMode( MatrixMode m ) { mode = m; };
  inline MatrixMode GetMode( void ) { return mode; };
  
  void	copy_from(CMatrix* img);
  
  void	draw_circle(int x,int y,int r,CEntity* ent);
  void	draw_line(int x1,int y1,int x2,int y2,CEntity* ent);
  
  // new 1 Dec 2000 - RTV
  void	draw_rect( const Rect& t, CEntity* ent );
  
  CEntity** line_detect(int x1,int y1,int x2,int y2);
  CEntity** rect_detect( const Rect& r);
  
  void	clear( void );
  
  ~CMatrix(void);
};


#endif
