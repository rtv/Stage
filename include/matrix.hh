// ==================================================================
// Filename:	CMatrix.h
//
// $Id: matrix.hh,v 1.10 2002-08-22 02:04:38 rtv Exp $
// RTV
// ==================================================================

#ifndef _MATRIX_CLASS_
#define _MATRIX_CLASS_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "image.hh" // for Rect
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
  
  public:

  CMatrix(int w, int h, int default_buf_size);
  ~CMatrix(void);
    
  inline int get_width(void) {return width;}
  inline int get_height(void) {return height;}
  inline int get_size(void) {return width*height;}
  
  // MUST BE IN-BOUNDS!

  // get a pixel color by its x,y coordinate
  inline CEntity** get_cell(int x, int y)
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
  inline CEntity** get_cell( int i)
    { 
      if( i<0 || i > width*height ) 
      {
        fputs("Stage: WARNING: CEntity::get_cell(int) out-of-bounds\n",stderr);
        return 0;
      }
      return data[i]; 
    }
  
  // is there an object of this type here?
  inline bool is_type( int x, int y, StageType type )
    { 
      //if( i<0 || i > width*height ) return 0;
    
      CEntity** cell = data[x+(y*width)];
    
      while( *cell )
      {
        if( (*cell)->stage_type == type ) return true;
        cell++;
      }
    
      return false;
    }
  

  inline void set_cell(int x, int y, CEntity* ent );
  inline void unset_cell(int x, int y, CEntity* ent );
  
  void	copy_from(CMatrix* img);

  void	draw_line(int x1, int y1, int x2, int y2, CEntity* ent, bool add);
  void	draw_rect( const Rect& t, CEntity* ent, bool add );
  void	draw_circle(int x, int y, int r, CEntity* ent, bool add);
  
  CEntity** line_detect(int x1,int y1,int x2,int y2);
  CEntity** rect_detect( const Rect& r);
  
  void	clear( void );

// utilities for visualizing the matrix
  void dump( void );

  rtk_fig_t* fig;
  void render( CWorld* world );
  void unrender( void );

  void PrintCell( int cell );
  void CheckCell( int cell );
  
};


#endif
