// ==================================================================
// Filename:	raytrace.hh
// $Id: raytrace.hh,v 1.3 2001-12-20 03:11:46 vaughan Exp $
// RTV
// ==================================================================


#ifndef RAYTRACE_HH
#define RAYTRACE_HH

#include <math.h>
#include "matrix.hh"
#include "world.hh"

enum LineIteratorMode { PointToPoint=0, PointToBearingRange };

// these classes do not inherit from each other, but they have similar
// interfaces and use each other extensively

// scans the points along a line
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

// scans the AREA of a triangle
class CTriangleAreaIterator
{
private:
  double m_x, m_y;
  
  double m_bearing;
  double m_range_so_far;
  double m_max_range;
  double m_angle;
  double m_skip;
  

  double m_li_angle;

  double m_ppm;

  CMatrix* m_matrix;

  CLineIterator* li; // we use LineIterators to do the work

 public:

  // an isoceles triangle is specified from it's point, by the bearing
  // and length of it's left side, and by the angle between the equal
  // sides. the triangle is scanned from point to base in increasingly
  // long lines perpendicular to the base. skip give the distance
  // increment between scan lines.
  CTriangleAreaIterator( double x, double y, 
			 double bearing, double range, double angle,
			 double skip,
			 double ppm, CMatrix* matrix );
  
  ~CTriangleAreaIterator();

  CEntity* GetNextEntity( void );

//    inline void GetPos( double& x, double& y )
//      {
//        x = m_x / m_ppm;
//        y = m_y / m_ppm;
//      };
  
  double GetRange( void );

  //void PrintArray( CEntity** ent ); 
};

// scans the circumference of a circle
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
  
  void GetPos( double& x, double& y );
};


// given a pointer to an array of entity pointers, this iterator will
// return one entity* at a time. when it runs out of entities, it
// returns null.

class CDatabaseIterator
{

private:
  CEntity** database;
  int index;
  int len;

public:
  CDatabaseIterator( CEntity** database, int len )
  {
    this->database = database;
    this->len = len;

    index = 0;
  };

  CEntity* GetNextEntity( void )
  {
    if( index < len )
      return database[index++];
    else
      return (CEntity*)0;
  }
};

#endif // _ITERATORS_H

