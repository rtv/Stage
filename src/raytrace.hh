/*
 *  Stage : a multi-robot simulator.
 *  Copyright (C) 2001, 2002 Richard Vaughan, Andrew Howard and Brian Gerkey.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
// ==================================================================
// Filename:	raytrace.hh
// $Id: raytrace.hh,v 1.2.6.1 2003-02-01 02:14:30 rtv Exp $
// RTV
// ==================================================================


#ifndef RAYTRACE_HH
#define RAYTRACE_HH

#include <math.h>
#include "matrix.hh"

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

  inline double GetAngle( void )
  {
    return( m_angle );
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

// scans an arc of a circle
class CArcIterator
{
protected:
  double m_radius;
  double m_px, m_py;
  double m_center_x, m_center_y;
  double m_angle;
  double m_dangle;

  double m_max_angle;

  int m_index;

  CEntity** m_ent;
  double m_ppm;
  CMatrix* m_matrix;

 public:
  CArcIterator( double x, double y, double th,  double scan_r, double scan_th,
		   double ppm, CMatrix* matrix );
  
  
  inline CEntity** ArcTrace( double &remaining_angle );
  
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

// a circle iterator is an arc iterator with a hard-coded 2*PI max_angle 
class CCircleIterator : public CArcIterator
{
public: 
  CCircleIterator( double x, double y, double r, 
		   double ppm, CMatrix* matrix );
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
  
  inline void GetPos( double& x, double& y )
  {
    for( int i=0; i<4; i++ )
      if( lits[i] != 0 ) // find the current line iterator
	{
	  lits[i]->GetPos( x, y );
	  break;
	}
  };

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

#endif // RAYTRACE_HH

