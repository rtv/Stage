//#include <pair>
#include <vector>
#include <string>


#include <math.h>
#include <assert.h>

#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif 

// STL container iterator macros - __typeof is a gcc extension
// WARNING: assumes container is not modified during iteration
#ifndef FOR_EACH
#define VAR(V,init) __typeof(init) V=(init)
#define FOR_EACH(I,C) for(VAR(I,(C).begin());I!=(C).end();++I) 
#endif

const unsigned int VCOUNT = 32;
const unsigned int WCOUNT = 32;
const double obstacle_dist_max = 2.0; // TODO XX

// implement signum
template<typename T> int sgn(T val)
{
    return (val > T(0)) - (val < T(0));
}

class Vec;
Vec operator*( double s, const Vec& v) ;

class Vec : public std::pair<double, double>
{
public:
  Vec( double a=0, double b=0 )
  {
    first = a;
    second = b;    
  }
  
  double Dot( const Vec& v) const
  { return( first*v.first + second*v.second); }
  
  Vec operator-(const Vec& v) const
  { return Vec( first-v.first, second-v.second); }
  
  Vec operator+(const Vec& v ) const
  { return Vec( first+v.first, second+v.second);}
  
  Vec operator/( const double s ) const
  { return Vec( first/s, second/s ); }
  
  Vec operator*( double s ) 
  { return Vec( s*first, s*second); }

  bool operator<( const Vec& v )
  { return( LengthSqr() < v.LengthSqr() ); }
  
  double LengthSqr() const
  { return Dot(*this); }

  double Length() const
  { return sqrt(LengthSqr()); }

  Vec Normalize() const
  { return( ( 1.0 / Length()) * *this ); }

  std::string Str(void)
  {
    char buf[1024];
    snprintf( buf, sizeof(buf), "[%.2f,%.2f]", first, second );    
    return std::string( buf );
  }    
};

Vec operator*( double s, const Vec& v) 
{ return Vec( s*v.first, s*v.second); }


class Circle
{
public:
  Vec c;
  double r;
  
  Circle( const Vec& c, double r ) : c(c), r(r)
  {}

  Circle() : c(0,0), r(0) 
  {}
  
  void Draw()
  {
    glBegin( GL_LINE_LOOP );    
    for( double a=0; a<2.0*M_PI; a+=M_PI/32.0 )
      glVertex2f( c.first + r*cos(a), c.second + r*sin(a) );
    glEnd();    
  }  
};

  int circle_circle_intersection(double x0, double y0, double r0,
				 double x1, double y1, double r1,
				 double *xi, double *yi,
				 double *xi_prime, double *yi_prime)
  {
    //double a, dx, dy, d, h, rx, ry;
    //double x2, y2;
    
    /* dx and dy are the vertical and horizontal distances between
     * the circle centers.
     */
    const double dx = x1 - x0;
    const double dy = y1 - y0;
    
    /* Determine the straight-line distance between the centers. */
    const double d = hypot(dx,dy); 
    
    /* Check for solvability. */
    if (d > (r0 + r1))
      {
	/* no solution. circles do not intersect. */
	return 0;
      }
    if (d < fabs(r0 - r1))
      {
	/* no solution. one circle is contained in the other */
	return 0;
      }
    
    /* 'point 2' is the point where the line through the circle
     * intersection points crosses the line between the circle
     * centers.  
     */
    
    /* Determine the distance from point 0 to point 2. */
    const double a = ((r0*r0) - (r1*r1) + (d*d)) / (2.0 * d) ;
    
    /* Determine the coordinates of point 2. */
    const double x2 = x0 + (dx * a/d);
    const double y2 = y0 + (dy * a/d);
    
    /* Determine the distance from point 2 to either of the
     * intersection points.
     */
    const double h = sqrt((r0*r0) - (a*a));
    
    /* Now determine the offsets of the intersection points from
     * point 2.
     */
    const double rx = -dy * (h/d);
    const double ry = dx * (h/d);
    
    /* Determine the absolute intersection points. */
    *xi = x2 + rx;
    *xi_prime = x2 - rx;
    
    *yi = y2 + ry;
    *yi_prime = y2 - ry;
    
    return 1;
  }


//  args:
//
//  int    polySides  =  how many corners the polygon has
//  float  polyX[]    =  horizontal coordinates of corners
//  float  polyY[]    =  vertical coordinates of corners
//  float  x, y       =  point to be tested
//
//  (Globals are used in this example for purposes of speed.  Change as
//  desired.)
//
//  The function will return YES if the point x,y is inside the polygon, or
//  NO if it is not.  If the point is exactly on the edge of the polygon,
//  then the function may return YES or NO.
//
//  Note that division by zero is avoided because the division is protected
//  by the "if" clause which surrounds it.

bool PointInPolygon( int polySides, float polyX[], float polyY[], float x, float y ) 
{
  int   i, j=polySides-1 ;
  bool  oddNodes=false;
  
  for (i=0; i<polySides; i++) {
    if (((polyY[i]< y && polyY[j]>=y)
	 ||   (polyY[j]< y && polyY[i]>=y))
	&&  (polyX[i]<=x || polyX[j]<=x)) {
      oddNodes^=(polyX[i]+(y-polyY[i])/(polyY[j]-polyY[i])*(polyX[j]-polyX[i])<x); }
    j=i; }
  
  return oddNodes; 
}


/*
  The I-DWA scores for speed and turn rate
  args: 
  v: current forward speed
  vmax: maximum speed
  w: current turn rate
  wmax: maximum turn rate
  roh: distance to goal point (distance error)
  alpha: angle to goal point (heading error)
  dist: distance to nearest obstacle after one timestep if we drive at this speed
*/
std::pair<double,double> IDWAScores( double v, double vmax, 
				     double w, double wmax,
				     double roh, double alpha )
{
  // gains must be positive - values from the paper
  const double k_v = 1.0; // distance error weight
  const double k_alpha = 0.59; // angular error weight
  const double k_roh = 3.0; // slow on goal approach weight
  
  // I-DWA Equation 13
  // ideal forward speed to arrive at the goal
  const double vi = k_v * vmax * cos(alpha) * tanh( roh / k_roh );
  
  // I-DWA Equation 14
  // ideal turn speed to arrive at the goal
  const double wi = k_alpha * alpha + vi * sin(alpha) / roh;
  
  // I-DWA Equation 17 
  // weighted comparison of current speeds with the
  // ideal speeds taking into account the distance from the goal point
  // and the distance to obstacles.

  const double vs = 1.0 - fabs(v-vi) / (2.0*vmax );					  
  double ws = 1.0 - fabs(w-wi) / (2.0*wmax );
  
  // printf( "w %.2f wi %.2f fabs(w-wi) %.2f wmax %.6f  ws %.2f\n", 
  // 	  w, wi, fabs(w-wi), wmax, ws );

  // ws is going slighty below zero for some reason. Force it not to.
  if( ws < 0.0 )
    ws = 0.0;

  assert( vs >= 0.0 );
  assert( vs <= 1.0 );
  assert( ws >= 0.0 );
  assert( ws <= 1.0 );
    
  return std::pair<double,double>( vs, ws );				  
}


// return weighted sum of scores, gains chosen to normalize [0..1]
double ObjectiveFunc( double so, double go,
		      double sv, double gv,
		      double sw, double gw )  
{
  // scores must be normalized to the range [0..1]
  assert( so >= 0.0 );
  assert( so <= 1.0 );
  assert( sv >= 0.0 );
  assert( sv <= 1.0 );
  assert( sw >= 0.0 );
  assert( sw <= 1.0 );

  // gains must be positive and sum to 1.0
  assert( go >= 0.0 );
  assert( go <= 1.0 );
  assert( gv >= 0.0 );
  assert( gv <= 1.0 );
  assert( gw >= 0.0 );
  assert( gw <= 1.0 );
  assert( fabs( (gv + gw + go ) - 1.0) < 0.0001 );

  //printf( "sv %.2f sw %.2f\n", sv, sw );  
  return( gv * sv + gw * sw + go * so );
}


class Vel
{
public:
  double v, w;
  double score;

  double score_speed;
  double score_heading;
  double score_obstacle;

  bool admissible;
  
  bool inwindow;

  // distance to nearest obstacle
  double obstacle_dist;
  
  // offset of new pose after moving with velocity (v,w) for 1 timestep
  double dx, dy;
  
  // loction of nearest obstacle
  double ox, oy;
  
  // all the points that this trajectory intersects with the obstacle cloud
  std::vector<Vec> cloud_hits;
  
  Vel( double v, double w ) 
    : v(v), 
      w( fabs(w) == 0 ? 0.01 : w ), // disallow zero rotation 
      score(0), 
      admissible(true),
      obstacle_dist(0),
    //      dx( interval * v * cos(interval*w) ),
    //  dy( interval * v * sin(interval*w) ),
      ox(0), 
      oy(0),
      cloud_hits()
  {
  }

  Vel() 
    : v(0), 
      w(0), 
      score(0), 
      admissible(true),
      obstacle_dist(0),
      ox(0), 
      oy(0),
      cloud_hits()
  {
  }
  
  
  void Evaluate( double goal_dist, 
		 double goal_angle,
		 double vmax, 
		 double wmax, 
		 const std::vector<Circle>& cloud )
  {
    double radius = v / w; 


    // compute where the trajectory collides with the obstacle cloud
    cloud_hits.clear();	    	
    
    Vec circle( 0, radius ); // center of circle        
    
    Vec hit1, hit2;	       
    FOR_EACH( it, cloud )
      if( circle_circle_intersection( 0, radius, fabs(radius), 
				      it->c.first, it->c.second, fabs(it->r), 
				      &hit1.first, &hit1.second,
				      &hit2.first, &hit2.second ))
	{
	  cloud_hits.push_back( hit1 );	
	  cloud_hits.push_back( hit2 );
	}    
    
    // default value will be overridden if this trajectory hits an obstacle
    obstacle_dist = 5.0; // TOOD
    
    //    if( fabs(w) >  0.01 ) 
      {
	
	if( cloud_hits.size() )
	  {		
	    // find the closest cloud hit
	    Vec closest = *min_element( cloud_hits.begin(), cloud_hits.end() );	    
	    
	    ox = closest.first;
	    oy = closest.second;	    
	    
	    // find the distance along the circle until we hit the obstacle
	    Vec a( 0, -radius );
	    Vec b( closest.first, closest.second - radius );	    
	    double g = acos( a.Dot(b) / (a.Length() * b.Length() ));	    
	    
	    //printf( "g %.2f r %.2f\n", g, radius );
	    
	    obstacle_dist = std::min( obstacle_dist, fabs( g * radius ) );
	    //it->obstacle_dist = fabs( g*radius );
	    
	    assert( obstacle_dist > 0.0 );
	    assert( obstacle_dist <= obstacle_dist_max );
	    
	    // printf( "v %.2f w %.2f hit (%.2f,%.2f) hypot %.2f dist %.2f\n",
	    // 	it->v, it->w, closest.first, closest.second, 
	    // 	hypot(closest.first, fabs(closest.second)), it->obstacle_dist );	    	    
	  }
      }
    
    // normalize the score
    score_obstacle = obstacle_dist / obstacle_dist_max;
    
    //    printf( "(%.2f,%2.f) obstacle_dist %.2f max %.2f score %.2f\n",
    //	    v, w, obstacle_dist, obstacle_dist_max, score_obstacle );    
    
    assert( score_obstacle >=0 );
    assert( score_obstacle <= 1.0 );
    
    std::pair<double,double> scores = IDWAScores(  v, vmax, 
						   w, wmax,
						   goal_dist, 
						   goal_angle );
    
    score_speed = scores.first;
    score_heading = scores.second;
    
    // values from the I-DWA paper
    double dist_gain = 7.0/13.0;
    double speed_gain = 3.0/13.0;
    double turn_gain = 3.0/13.0;
    
    
    score = ObjectiveFunc( score_obstacle, dist_gain,
			   score_speed, speed_gain,
			   score_heading, turn_gain );
  }
  


  void Print()
  {
    printf( "[v %.3f w %.3f @(%.2f,%.2f) admit %s obstacle %.2f @(%.2f,%.3f) score %.3f]\n", 
	    v, w, 
	    dx, dy, 
	    admissible ? "T" : "F",
	    obstacle_dist,
	    ox, oy,
	    score );
  }      
};

/* // convert a ranger scan into a vector of line segments */
/* std::vector<Circle> ObstacleCloud( const ModelRanger::Sensor& sensor,  */
/* 				   const double safety_radius ) */
/* {       */
/*   std::vector<Circle> cloud( sensor.sample_count ); */
  
/*   for( unsigned int i=0; i<sensor.sample_count; i++ ) */
/*     { */
/*       double r = sensor.ranges[i]; */
/*       double a = sensor.bearings[i]; */
      
/*       cloud[i].c.first = r * cos(a); */
/*       cloud[i].c.second = r * sin(a); */
/*       cloud[i].r = std::max( safety_radius, 0.7 * M_PI/sensor.sample_count * r );	 */
/*     } */
  
/*   return cloud; */
/* } */

// convert a ranger scan into a vector of line segments
std::vector<Circle> Cloud( std::vector<double> ranges, 
			   std::vector<double> bearings,
			   const double safety_radius )
{      
  assert( ranges.size() == bearings.size() );
  
  unsigned int num = ranges.size();

  std::vector<Circle> cloud( num );
  
  for( unsigned int i=0; i<num; ++i )
    {
      double r = ranges[i];
      double a = bearings[i];
      
      cloud[i].c.first = r * cos(a);
      cloud[i].c.second = r * sin(a);
      cloud[i].r = std::max( safety_radius, 0.7 * M_PI/num * r );	
    }
  
  return cloud;
}

class VelSpace
{ 
public:
  std::vector<Vel> vels;
  unsigned int vcount, wcount;
  std::pair<double,double> vbounds, wbounds, dvbounds, dwbounds;
  std::vector<Circle> cloud;
  
  Vel& GetVel( unsigned int v, unsigned int w ) 
  { return vels[ v + w*wcount ]; }  
  
  const Vel&   GetVel( unsigned int v, unsigned int w ) const 
  { return vels[ v + w*wcount ]; }
    
  VelSpace( const unsigned int vcount, 
	    const unsigned int wcount,
	    const std::pair<double,double>& vbounds,
	    const std::pair<double,double>& wbounds,
	    const std::pair<double,double>& dvbounds,
	    const std::pair<double,double>& dwbounds
	    ) :
    vcount(vcount),
    wcount(wcount),
    vbounds(vbounds),
    wbounds(wbounds),
    dvbounds(dvbounds),
    dwbounds(dwbounds),
    cloud()
  {    
    vels.resize( vcount * wcount );

    for( unsigned int vi=0; vi<vcount; ++vi )
      for( unsigned int wi=0; wi<wcount; ++wi )
	{
	  Vel& vel = GetVel( vi, wi );
	  
	  const double vrange = vbounds.second - vbounds.first;
	  const double wrange = wbounds.second - wbounds.first;
	  
	  const double vincr = vrange / (double)vcount;
	  const double wincr = wrange / (double)wcount;
	  
	  vel.v = vbounds.first + vi * vincr;
	  vel.w = wbounds.first + wi * wincr;	  
	  
	  // disallow zero w
	  if( fabs(vel.w) < 0.001 )
	    vel.w = 0.001;
	}
  }   
  
  

  
  Vel& Best( double x, double y, double a,
	     double v, double w,
	     double r, double b,
	     const std::vector<double>& ranges,
	     const std::vector<double>& bearings,
	     const double radius,
	     const double interval )
  {        
    // range of achievable speeds at the nexttimestep is the dynamic window
    const double v1max = std::min( v + dvbounds.second * interval, vbounds.second );
    const double v1min = std::max( v + dvbounds.first * interval, vbounds.first );    
    
    const double w1max = std::min( w + dwbounds.second * interval, wbounds.second );
    const double w1min = std::max( w + dwbounds.first * interval, wbounds.first );
    
    //    printf( "Reachable bounds (%.3f to %.3f) (%.3f %.3f)\n",
    //	    v1min, v1max, w1min, w1max );

    // 
    cloud = Cloud( ranges, bearings, radius );
    
    unsigned int best_index = 0;
    double best_score = 0.0;
    
    // evaluate each candidate (v,w) pair
    for( unsigned int i=0; i<vels.size(); ++i )
      {
	Vel& vel = vels[i];

	vel.Evaluate( r, b, vbounds.second, wbounds.second, cloud);
	
	if( vel.v <= v1max && 
	    vel.v >= v1min &&
	    vel.w <= w1max && 
	    vel.w >= w1min )	    
	  {
	    vel.inwindow = true;
	  }
	else
	  vel.inwindow = false;

	if( vel.inwindow && (vel.score > best_score) )
	  {
	    best_score = vel.score;
	    best_index = i;
	  }	
      }            
    return vels[best_index];
  }
  


  void Draw()
  {
   glPointSize( 8.0 );
 
      glPushMatrix();
      
      glLoadIdentity();
      glScalef( 0.1, 0.1, 0 );
      
      // glRotatef( -rtod(mod->GetGlobalPose().a), 0,0,1 );
      
      
      glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
      
      for( unsigned int v=0; v<vcount; ++v )
	for( unsigned int w=0; w<vcount; ++w )
	  {
	    const Vel& vel = GetVel( v, w );
	    
	    if( vel.inwindow )
	      glColor4f( vel.score, 0,0,0.7 );
	    else
	      glColor4f( 0, 0, vel.score, 0.7 );

	    glRectf( v, w, v+1, w+1 );
	  }
      glPopMatrix();
      
      // draw the obstacle cloud            
      glColor3f( 0, 1, 0 );
      FOR_EACH( it, cloud )
       	 it->Draw();       	   
                   
      // draw each trajectory as a faint circle
      // glColor4f( 0,0,0,0.2 );
      // const double steps = 100;
      // FOR_EACH( it, vels )
      //  	 {	   	   	   
      //  	   glBegin( GL_LINE_LOOP );
      //  	   double radius = it->v / it->w;	   
      //  	   double a = (2.0 * M_PI) / steps;	       
      //  	   for( int i=0; i<steps; i++ )
      // 	     glVertex2f( fabs(radius) * cos( i*a ), radius + fabs(radius) * sin( i*a ) );  
      // 	   glEnd();
      // 	 }
     
       // draw the cloud hits
      const double e=0.02;

      glColor4f( 0,0,1, 0.5 );	   
      
      glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
      glBegin( GL_QUADS );
      FOR_EACH( it, vels )
	{
	  //	   printf( "cloud huts len %d\n", (int)it->cloud_hits.size() );
	  //if( it->inwindow )
	  FOR_EACH( it2, it->cloud_hits )
	    {
	      glVertex2f( it2->first-e, it2->second-e );
	      glVertex2f( it2->first+e, it2->second-e );
	      glVertex2f( it2->first+e, it2->second+e );
	      glVertex2f( it2->first-e, it2->second+e );
	    }
	}
      glEnd();	 	
      
      FOR_EACH( it, vels )
	{
	  if( it->inwindow )
	    glColor4f( 0,0,0,0.8 ); // dark 
	  else
	    glColor4f( 0,0,0,0.15 ); // light	 


	    {
	      // if( rob.best_vel == &*it )	 
	      // 	{
	      // 	  //glLineWidth(3);       
	      // 	}
	      // else
	      // 	{
	      // 	  //glLineWidth(1);       
	      // 	}
	      
	      // show arcs until obstacles are hit
	      
	      const double radius = it->v / it->w; 
	      const double dist = it->obstacle_dist;
	      if( dist < 50 )
		{		
		  // sanity check
		  glBegin( GL_LINE_STRIP );
		  for( double d=0; d<fabs(dist); d+=0.05 )
		    {	       
		      const double w = M_PI - d/radius;	       
		      const double x = radius * sin(w);	       
		      const double y = radius + radius * cos(w);
		      
		      if( fabs(x) < 100 && fabs(y)<100 )
			glVertex2f( x, y  );
		    }
		  glEnd();
		}
	      else
		{
		  //printf( "LONG v %.2f w %.2f dist %.2f\n", it->v, it->w, dist );	       
		}
	    }	       
	}       
      
       // glLineWidth( 1.0 );
       

       // indicate the closest cloud hit
       // glColor3f( 1,0,0 );	   
       // glBegin( GL_LINES );
       // FOR_EACH( it, rob.vspace )
       // 	 {
       // 	   if( it->cloud_hits.size() )
       // 	     {
       // 	       glVertex2f( 0,0 );
       // 	       glVertex2f( it->ox, it->oy );
       // 	     }	   		
       // 	 }
       // glEnd();            
       
       glPopMatrix();       
  }
}; // class VelSpace
