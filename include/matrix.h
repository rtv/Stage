// ==================================================================
// Filename:	CMatrix.h
//
// $Id: matrix.h,v 1.2.2.5 2001-08-24 18:38:04 vaughan Exp $
// RTV
// ==================================================================

#ifndef _MATRIX_CLASS_
#define _MATRIX_CLASS_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "image.h" // for Rect
#include "entity.hh"


enum MatrixMode { mode_set, mode_unset };




class CMatrix
{
public:
        int	width;
	int	height;
	
	CEntity***  data;
	int* current_slot;
	int* available_slots;

	char*	win_title;

	int initial_buf_size;

	void dump( void );

	MatrixMode mode;
	void PrintCell( int cell );

	//CMatrix();
	CMatrix(int w,int h);
	//CMatrix(CMatrix*);

	inline	int	get_width(void)		{return width;}
	inline	int	get_height(void)	{return height;}
	inline	int	get_size(void)		{return width*height;}

	// MUST BE IN-BOUNDS!
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

	inline void set_cell(int x, int y, CEntity* ent );
	inline void unset_cell(int x, int y, CEntity* ent );

	inline void SetMode( MatrixMode m ) { mode = m; };
	inline MatrixMode GetMode( void ) { return mode; };

	//inline	CEntity** *get_data(void)	{return data;}

	void	copy_from(CMatrix* img);

	//void    bgfill( int x, int y, CEntity* ent );
	//void	draw_box(int,int,int,int,CEntity* ent);
	void	draw_circle(int x,int y,int r,CEntity* ent);
	void	draw_line(int x1,int y1,int x2,int y2,CEntity* ent);

	// new 1 Dec 2000 - RTV
	void	draw_rect( const Rect& t, CEntity* ent );

	//void	draw_line_detect(int x1,int y1,int x2,int y2,int c);	
	CEntity** line_detect(int x1,int y1,int x2,int y2);
	CEntity** rect_detect( const Rect& r);

	void	clear( void );
    
	~CMatrix(void);
};

enum LineIteratorMode { PointToPoint=0, PointToBearingRange };

class CLineIterator
{
 private:
  double m_x, m_y;
  
  double m_remaining_range;
  double m_max_range;
  double m_angle;
  
  CEntity** m_ent;
  int m_index;

  double m_ppm;
  CMatrix* m_matrix;

 public:
  CLineIterator( double x, double y, double a, double b, 
		 double ppm, CMatrix* matrix, LineIteratorMode pmode );
  
  CEntity* GetNextEntity( void );

  inline void GetPos( double& x, double& y )
    {
      x = m_x / m_ppm;
      y = m_y / m_ppm;
    };
  
  inline double GetRange( void )
    {
      return( (m_max_range - m_remaining_range) / m_ppm );
    };

  inline CEntity** RayTrace( double &px, double &py, double pth, 
			     double &remaining_range );


  void PrintArray( CEntity** ent );
  
};

class CCircleIterator : public CLineIterator
{
  double m_radius;

 public:
  CCircleIterator( double x, double y, double r, 
		   double ppm, CMatrix* matrix );
  
  
  inline CEntity** RayTrace( double &px, double &py, double pr, 
			     double &remaining_angle );
  
  
}

class CRectangleIterator
{
 private:
  CLineIterator* lits[4];
  
  double corners[4][2];
  
 public:

  CRectangleIterator( double x, double y, double th,
		      double w, double h,  
		      double ppm, CMatrix* matrix );
  
  CEntity* GetNextEntity( void );  
};


#endif
