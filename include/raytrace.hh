// ==================================================================
// Filename:	raytrace.hh
// $Id: raytrace.hh,v 1.1.2.2 2001-09-04 20:36:31 vaughan Exp $
// RTV
// ==================================================================


#ifndef RAYTRACE_HH
#define RAYTRACE_HH

#include <math.h>
#include "matrix.hh"
#include "entity.hh"

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

class CCircleIterator
{
 private:
  double m_radius;
  double m_px, m_py;
  double m_center_x, m_center_y;
  double m_angle;
  double m_dangle;

  int m_index;

  CEntity** m_ent;
  double m_ppm;
  CMatrix* m_matrix;

 public:
  CCircleIterator( double x, double y, double r, 
		   double ppm, CMatrix* matrix );
  
  
  inline CEntity** CircleTrace( double &remaining_angle );
  
  CEntity* GetNextEntity( void );

  void PrintArray( CEntity** ent );

  inline void GetPos( double& x, double& y )
    {
      x = m_px / m_ppm;
      y = m_py / m_ppm;
    };
  
  inline double GetRange( void )
    {
      return( hypot( m_px - m_center_x, m_py - m_center_y ) / m_ppm );
    };

  inline double GetAngle( void )
  {
    return( m_angle );
  };

  
};

class CRectangleIterator
{
 private:
  CLineIterator* lits[4];
 
  double corners[4][2];
  
 public:

  CRectangleIterator( double x, double y, double th,
		      double w, double h,  
		      double ppm, CMatrix* matrix );

  ~CRectangleIterator( void );
 
  CEntity* GetNextEntity( void );  
};


#endif // _ITERATORS_H

