// ==================================================================
// Filename:	raytrace.cc
// $Id: raytrace.cc,v 1.3 2001-09-28 21:55:42 gerkey Exp $
// RTV
// ==================================================================

#include "raytrace.hh"


CEntity* g_nullp = 0;

CLineIterator::CLineIterator( double x, double y, double a, double b, 
			      double ppm, CMatrix* matrix, LineIteratorMode pmode )
{   
  m_matrix = matrix;
  m_ppm = ppm;       
  
  m_x = x * m_ppm;
  m_y = y * m_ppm;

  m_ent = 0;
  m_index = 0;

  switch( pmode )
    {
    case PointToBearingRange:
      {
	double range = b;
	double bearing = a;
	
	m_angle = bearing;
	m_max_range = m_remaining_range = range * m_ppm;
      }
      break;
    case PointToPoint:
      {
	double x1 = a;
	double y1 = b;
	
	//printf( "From %.2f,%.2f to %.2f,%.2f\n", x,y,x1,y1 );

	m_angle = atan2( y1-y, x1-x );
	m_max_range = m_remaining_range = hypot( x1-x, y1-y ) * m_ppm;
      }
      break;
    default:
      puts( "Stage Warning: unknown LineIterator mode" );
    }

  //printf( "angle = %.2f remaining_range = %.2f\n", m_angle, m_remaining_range );
  //fflush( stdout );
};


CEntity* CLineIterator::GetNextEntity( void )
{
  //PrintArray( m_ent );

  if( m_remaining_range <= 0 ) return 0;

  //printf( "current ptr: %p index: %d\n", m_ent[m_index], index );

  if( !(m_ent && m_ent[m_index]) ) // if the ent array is null or empty
    {
      //printf( "before %d,%d\n", (int)m_x, (int)m_y );
      // get a new array of pointers
      m_ent = RayTrace( m_x, m_y, m_angle, m_remaining_range );
      //PrintArray( m_ent );
      //printf( "after %d,%d\n", (int)m_x, (int)m_y );
      m_index = 0; // back to the start of the array
    }
  
  assert( m_ent != 0 ); // should be a valid array, even if empty
 
  //PrintArray( m_ent );
  //printf( "returning %p (index: %d) at: %d %d rng: %d\n",  
  //  m_ent[m_index], m_index, (int)m_x, (int)m_y, (int)m_remaining_range );

  return m_ent[m_index++]; // return the next CEntity* (it may be null)
}

inline CEntity** CLineIterator::RayTrace( double &px, double &py, double pth, 
				   double &remaining_range )
{
  int range = 0;
  
  //printf( "Ray from %.2f,%.2f angle: %.2f remaining_range %.2f\n", 
  //  px, py, RTOD(pth), remaining_range );
  
  // hops along each axis
  double cospth = cos( pth );
  double sinpth = sin( pth );
  
  CEntity** ent = 0;
  
  // Look along scan line for obstacles
  for( range = 0; range < remaining_range; range++ )
    {
      px += cospth;
      py += sinpth;
      
      // stop this ray if we're out of bounds
      if( px < 0 ) 
	{ 
	  px = 0; 
	  remaining_range = 0.0;
	}
      if( px >= m_matrix->width ) 
	{ 
	  px = m_matrix->width-1; 
	  remaining_range = 0.0;
	}
      if( py < 0 ) 
	{ 
	  py = 0; 
	  remaining_range = 0.0;
	}
      if( py >= m_matrix->height ) 
	{ 
	  py = m_matrix->height-1; 
	  remaining_range = 0.0;
	}
      
      //printf( "looking in %d,%d\n", (int)px, (int)py );
      
      ent = m_matrix->get_cell( (int)px,(int)py );
      if( !ent[0] && px+1 < m_matrix->width) 
	ent = m_matrix->get_cell( (int)px+1,(int)py );
      
	  if( ent[0] ) break;// we hit something!
    }
  
  remaining_range -= (double)range+1; // we have this much left to go
  
  //if( ent && ent[0] )
  //printf( "Hit %p at %.2f,%.2f with %.2f to go\n", 
  //    ent[0], px, py, remaining_range );
  //else
  //printf( "hit nothing. remaining range = %.2f\n", remaining_range );
  // fflush( stdout );
  
  return ent; // we hit these entities
}; 

void CLineIterator::PrintArray( CEntity** ent )
{
  printf( "Array: " );
  
  if( !ent ) 
    {
      printf( "(null)\n" ); 
      return;
    }
  
  //puts( "foo" );

  printf( "[ " );
  int p = 0;
  while( ent[p] ) printf( "%p ", (ent[p++]) );
  
  printf( " ]\n" );
  
  //fflush( stdout );
}


CRectangleIterator::CRectangleIterator( double x, double y, double th,
					double w, double h,  
					double ppm, CMatrix* matrix )
{
  // calculate the corners of our body

  double cx = (w/2.0) * cos(th);
  double cy = (h/2.0) * cos(th);
  double sx = (w/2.0) * sin(th);
  double sy = (h/2.0) * sin(th);
  
  corners[0][0] = x + cx - sy;
  corners[0][1] = y + sx + cy;
    
  corners[1][0] = x - cx - sy;
  corners[1][1] = y - sx + cy;
    
  corners[2][0] = x - cx + sy;
  corners[2][1] = y - sx - cy;
   
  corners[3][0] = x + cx + sy;
  corners[3][1] = y + sx - cy;
  
  
  lits[0] = new CLineIterator( corners[0][0], corners[0][1], 
			       corners[1][0], corners[1][1],
			       ppm, matrix, PointToPoint );
  
  lits[1] = new CLineIterator( corners[1][0], corners[1][1], 
			       corners[2][0], corners[2][1],
			       ppm, matrix, PointToPoint );

  lits[2] = new CLineIterator( corners[2][0], corners[2][1], 
			       corners[3][0], corners[3][1],
			       ppm, matrix, PointToPoint );
  
  lits[3] = new CLineIterator( corners[3][0], corners[3][1], 
			       corners[0][0], corners[0][1],
			       ppm, matrix, PointToPoint );
  
} 

CRectangleIterator::~CRectangleIterator( void )
{
  for( int f=0; f<4; f++ )
    if( lits[f] ) delete lits[f];
}

CEntity* CRectangleIterator::GetNextEntity( void )
{
  CEntity* ent = 0;
  
  for( int i=0; i<4; i++ )
    if( (ent = lits[i]->GetNextEntity() ) != 0 )
      break;
  
  return ent;
}

CCircleIterator::CCircleIterator( double x, double y, double r, 
				  double ppm, CMatrix* matrix )
{   
  m_matrix = matrix;
  m_ppm = ppm;       

  m_radius = r * m_ppm;
  m_center_x = x * m_ppm;
  m_center_y = y * m_ppm;
  m_angle = 0;

  m_ent = 0;
  m_index = 0;

  m_px = 0;
  m_py = 0;

  // the angle increment - moves us by 1 pixel
  m_dangle = atan2( 1, m_radius );

  //printf( "Construct circle iterator: x=%.2f y=%.2f r=%.2f a=%.2f dangle=%.2f \n",
  //  m_center_x, m_center_y, m_radius, m_angle, m_dangle );

};


CEntity* CCircleIterator::GetNextEntity( void )
{
  if( m_angle >= 2.0 * M_PI ) return 0; // done!
  
  //printf( "current ptr: %p index: %d\n", m_ent[m_index], index );

  if( !(m_ent && m_ent[m_index]) ) // if the ent array is null or empty
    {
      //printf( "before %d,%d\n", (int)m_x, (int)m_y );
      // get a new array of pointers
      m_ent = CircleTrace( m_angle );
      //PrintArray( m_ent );
      //printf( "after %d,%d\n", (int)m_x, (int)m_y );
      m_index = 0; // back to the start of the array
    }
  
  assert( m_ent != 0 ); // should be a valid array, even if empty
 
  //PrintArray( m_ent );
  //printf( "returning %p (index: %d) at: %d %d rng: %d\n",  
  //m_ent[m_index], m_index, (int)m_x, (int)m_y, (int)m_angle );

  return m_ent[m_index++]; // return the next CEntity* (it may be null)  
}


CEntity** CCircleIterator::CircleTrace( double &remaining_angle )
{
 CEntity** ent = 0;
   
 while( remaining_angle < (M_PI * 2.0) )
   { 
     m_px = m_center_x + m_radius * cos( remaining_angle );
     m_py = m_center_y + m_radius * sin( remaining_angle );
     
     //printf( "looking in %d,%d  angle: %.2f\n", 
     //     (int)px, (int)py, remaining_angle );
     
     remaining_angle += m_dangle;
     
     // bounds check
     if( m_px < 0 || m_px >= m_matrix->width-1 || 
	 m_py < 0 || m_py >= m_matrix->height ) 
     {
       // if we're out of bounds, then return a pointer to NULL,
       // indicating free space
       ent = &g_nullp;
     }
     else
     {
       ent = m_matrix->get_cell( (int)m_px,(int)m_py );
       if( !ent[0] )//&& (px+1 < m_matrix->width) ) 
         ent = m_matrix->get_cell( (int)m_px+1,(int)m_py );
     }

     if( ent[0] ) break;// we hit something!
   }
 
 //if( ent && ent[0] )
 //printf( "Hit %p at %.2f,%.2f with %.2f to go\n", 
 //   ent[0], px, py, remaining_angle );
 //else
 //printf( "hit nothing. remaining angle = %.2f\n", remaining_angle );
 //fflush( stdout );
 
 return ent; // we hit these entities
}

void CCircleIterator::PrintArray( CEntity** ent )
{
  printf( "Array: " );
  
  if( !ent ) 
    {
      printf( "(null)\n" ); 
      return;
    }
  
  //puts( "foo" );
  
  printf( "[ " );
  int p = 0;
  while( ent[p] ) printf( "%p ", (ent[p++]) );
  
  printf( " ]\n" );
  
  //fflush( stdout );
}
