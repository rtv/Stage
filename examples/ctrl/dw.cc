#include "stage.hh"
using namespace Stg;

// implement signum
template<typename T> int sgn(T val)
{
    return (val > T(0)) - (val < T(0));
}

// number of different velocities and turn rates to consider
const double vres = 14.0;
const double wres = 14.0;

const double safety = 0.7;


// time interval between updates
double interval = 0.1;

// distance from the robot to the goal in X and Y
double gdx = 0.0;
double gdy = 0.0;
		
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
    if ((polyY[i]< y && polyY[j]>=y
	 ||   polyY[j]< y && polyY[i]>=y)
	&&  (polyX[i]<=x || polyX[j]<=x)) {
      oddNodes^=(polyX[i]+(y-polyY[i])/(polyY[j]-polyY[i])*(polyX[j]-polyX[i])<x); }
    j=i; }
  
  return oddNodes; 
}


/*
  The I-DWA cost function
  args: 
  v: current forward speed
  vmax: maximum speed
  w: current turn rate
  wmax: maximum turn rate
  roh: distance to goal point (distance error)
  alpha: angle to goal point (heading error)
  dist: distance to nearest obstacle after one timestep if we drive at this speed
*/
double ObjectiveFunc( double v, double vmax, 
		      double w, double wmax,
		      double roh, double alpha,
		      double dist )
{
  // gains must be positive - values from the paper
  double k_v = 1.0; // distance error weight
  double k_alpha = 0.59; // angular error weight
  double k_roh = 3.0; // slow on goal approach weight
  
  // I-DWA Equation 13
  // ideal forward speed to arrive at the goal
  double vi = k_v * vmax * cos(alpha) * tanh( roh / k_roh );
  
  // I-DWA Equation 14
  // ideal turn speed to arrive at the goal
  double wi = k_alpha * alpha + vi * sin(alpha) / roh;
  
  // I-DWA Equation 17 
  // weighted comparison of current speeds with the
  // ideal speeds taking into account the distance from the goal point
  // and the distance to obstacles.

  // gains must be positive and sum to 1.0 : values from the paper
  double speed_gain = 3.0/13.0;
  double turn_gain = 3.0/13.0;
  double dist_gain = 7.0/13.0;
  assert( fabs( (speed_gain + turn_gain + dist_gain ) - 1.0) < 0.001 );
    
  return ( speed_gain * ( 1.0 - fabs( v - vi ) / (2.0 * vmax )) +
	   turn_gain  * ( 1.0 - fabs( w - wi ) / (2.0 * wmax )) +
	   dist_gain  * dist );
}

class Vel;

bool PointsInsideRangerScan( const ModelRanger::Sensor& sensor, Vel& v );

class Vel
{
public:
  double v, w;
  double score;
  bool admissible;
  
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
      w( fabs(w) == 0 ? 0.001 : w ), // disallow zero rotation 
      score(0), 
      admissible(true),
      obstacle_dist(1e12),
      dx( interval * v * cos(interval*w) ),
      dy( interval * v * sin(interval*w) ),
      ox(0), 
      oy(0),
      cloud_hits()
  {
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

class Robot
{
public:
  ModelPosition* pos;
  ModelRanger* laser;
  
  Pose goal;
  double radius;  
  
  std::vector<Vel> vspace;
  Vel* best_vel;
  
  std::vector<Circle> cloud;
  
  Robot( ModelPosition* pos, ModelRanger* laser ) : 
    pos(pos), 
    laser(laser), 
    goal( Pose( 7,3,0,0 )),
    radius(0.4),
    vspace(),
    best_vel(NULL)
  {
    pos->AddCallback( Model::CB_UPDATE, (model_callback_t)PositionUpdateCB, this );  
    pos->Subscribe(); // starts the position updates
    
    //laser->AddCallback( Model::CB_UPDATE, (model_callback_t)LaserUpdate, robot );
    laser->Subscribe(); // starts the ranger updates

    // initialize vspace
    const double vmin = pos->velocity_bounds[0].min;
    const double vmax = pos->velocity_bounds[0].max;
    const double vrange = vmax - vmin;
    const double vincr = vrange / vres;
    
    const double wmin = pos->velocity_bounds[3].min;
    const double wmax = pos->velocity_bounds[3].max;
    const double wrange = wmax - wmin;
    const double wincr = wrange / wres;

    for( double v=vmin; v<=vmax; v += vincr )
      for( double w=wmin; w<=wmax; w += wincr )
	if( v || w ) vspace.push_back( Vel( v, w ) );
  }
  
  
  // convert a ranger scan into a vector of line segments
  void ObstacleCloud( const ModelRanger::Sensor& sensor, 
		      double safety_radius, 
		      std::vector<Circle>& cloud )
  {
    //    std::vector<std::pair<Vec,Vec> > cloud;
    
    cloud.resize( sensor.sample_count );
    
    for( unsigned int i=0; i<sensor.sample_count; i++ )
      {
	double r = sensor.ranges[i];
	double a = sensor.bearings[i];
	
	cloud[i].c.first = r * cos(a);
	cloud[i].c.second = r * sin(a);
	cloud[i].r = std::max( safety_radius, 0.7 * M_PI/sensor.sample_count * r );	
      }
  }

  
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
  
  void AddCloudHits( double v, double w, std::vector<Vec>& hits )
  {
    assert( w != 0.0 );
    
    double radius = v / w;       	       
    Vec circle( 0, radius ); // center of circle        
    
    Vec hit1, hit2;	       
    FOR_EACH( it, cloud )
      if( circle_circle_intersection( 0, radius, fabs(radius), 
				      it->c.first, it->c.second, fabs(it->r), 
				      &hit1.first, &hit1.second,
				      &hit2.first, &hit2.second ))
	{
	  hits.push_back( hit1 );	
	  hits.push_back( hit2 );
	}    
  }
  
  void EvaluateVSpace()
  {
    // get a cloud of obstacle line segments from the ranger
    ObstacleCloud( laser->GetSensors()[0], 0.4, cloud );
    
    const double vmax = pos->velocity_bounds[0].max;
    const double wmax = pos->velocity_bounds[3].max;
    
    // find the error in our current position wrt the goal
    const Pose& gpose = pos->GetGlobalPose();
    const double dx = goal.x - gpose.x;
    const double dy = goal.y - gpose.y;
    const double goal_dist = hypot( dx, dy );
    const double goal_angle = normalize( atan2( dy, dx ) - gpose.a );
    
    gdx = dx;
    gdy = dy;

    //usec_t timenow = pos->GetWorld()->SimTimeNow();
    //static usec_t lasttime = 0;
    //double interval = lasttime == 0 ? 0 : (timenow - lasttime) * 1e-4;
    //lasttime = timenow;
    
    //    printf( "%.4f\n", interval );


    // evaluate each candidate (v,w) pair
    FOR_EACH( it, vspace )
      {
	it->obstacle_dist = 3.0;//0.0;//10.0;	

	//it->admissible = 
	//PointsInsideRangerScan( laser->GetSensors()[0], *it );	
	
	// compute where the trajectory collides with the obstacle cloud
	it->cloud_hits.clear();	    	
	AddCloudHits( it->v, it->w, it->cloud_hits );	    
	    
	//printf( "cloud hits %d\n", (int)it->cloud_hits.size() );
	
	
	if( it->cloud_hits.size() )
	  {		
	    // find the closest cloud hit
	    Vec closest = *min_element( it->cloud_hits.begin(), it->cloud_hits.end() );	    
	    
	    it->ox = closest.first;
	    it->oy = closest.second;	    
	    
	    // find the distance along the circle until we hit the obstacle
	    double radius = it->v / it->w; 
	    Vec a( 0, -radius );
	    Vec b( closest.first, closest.second - radius );	    
	    double g = acos( a.Dot(b) / (a.Length() * b.Length() ));	    
	    
	    it->obstacle_dist = fabs( g*radius );
	    
	    
	    // printf( "v %.2f w %.2f hit (%.2f,%.2f) hypot %.2f dist %.2f\n",
	    // 	it->v, it->w, closest.first, closest.second, 
	    // 	hypot(closest.first, fabs(closest.second)), it->obstacle_dist );
	    
	    
	    //printf( "(%.2f,%2.f) -> (%.8f,%.8f)\n",
	    //	it->v, it->w, it->ox, it->oy );	    
	  }
	// else
	//   {  // no hits - set to a sensible maximumum
	// 	it->obstacle_dist = 8.0;
	//   }
	
	
	it->score = ObjectiveFunc( it->v, vmax, 
				   it->w, wmax,
				   goal_dist, 
				   goal_angle,
				   it->obstacle_dist );
	//}	  
	//else
	//it->score = 0.0;	
      }
  }
  
  // return the index of the vel with the highest score
  Vel& BestVel( std::vector<Vel>& vels )
  {
    unsigned int best_index = 0;
    double best_score = 0.0;
    
    for( unsigned int i=0; i<vels.size(); ++i )
      if( vels[i].score > best_score )
	{
	  best_score = vels[i].score;
	  best_index = i;
	}
    
    return vels[best_index];
  }

  int PositionUpdate( void )
  {          
    // figure out the best way to get there
    
    // calculate the vspace array
    EvaluateVSpace();          
    best_vel = &BestVel( vspace );

    printf( "best vel %.2f %.2f\n", best_vel->v, best_vel->w );
    

    // and request those speeds
    pos->SetSpeed( best_vel->v, 0, best_vel->w );
    
    return 0;
  }

  static int PositionUpdateCB( ModelPosition* pos, Robot* rob )
  {      
    return rob->PositionUpdate();
  }
  
};

bool PointsInsideRangerScan( const ModelRanger::Sensor& sensor, Vel& v )
{
  // pckage up the ranger hit points to define a bounding polygoin
  std::vector<float> polyX(sensor.sample_count+1);
  std::vector<float> polyY(sensor.sample_count+1);
  
  polyX[0] = 0.0;
  polyY[0] = 0.0;
  
  v.obstacle_dist = 1e12;// huge
  
  for( unsigned int i=0; i<sensor.sample_count; i++ )
    {
      polyX[i+1] = sensor.ranges[i] * cos( sensor.bearings[i] );
      polyY[i+1] = sensor.ranges[i] * sin( sensor.bearings[i] );

      //printf( "%d polypoint %.2f,%.2f  test point %.2f,%.2f\n", 
      //      (int)i, polyX[i+1], polyY[i+1], (double)x, (double)y );
      
      const double dist = hypot( polyX[i+1]-v.dx, polyY[i+1]-v.dy );
      if( dist < v.obstacle_dist )
	{
	  v.obstacle_dist = dist;  // record the range anf location of the cloest ranger hit
	  v.ox = polyX[i+1];
	  v.oy = polyY[i+1];
	}
    }  
  // puts("");

  return PointInPolygon( sensor.sample_count+1, &polyX[0], &polyY[0], v.dx, v.dy );
}


class DWVis : public Visualizer
  {
  public:    
    Robot& rob;
    
    DWVis( Robot& rob ) 
      : Visualizer( "dynamic window", "vis_dynamic_window" ), rob(rob) {}
    virtual ~DWVis(){}
    
    virtual void Visualize( Model* mod, Camera* cam )
    {
      //static int i=0;      
      // wrap around
      //i %= rob.vspace.size();
      

      glPointSize( 8.0 );
 
      glPushMatrix();
      
      glRotatef( -rtod(mod->GetGlobalPose().a), 0,0,1 );
      
      // draw a line to the goal
      mod->PushColor( Color( 0,0,1,0.3 ) );
      glBegin( GL_LINES );
      glVertex2f( 0, 0 );
      //glVertex2f( rob.goal.x - rob.pos->est_pose.x, 
      //	  rob.goal.y - rob.pos->est_pose.y );

      glVertex2f( gdx, gdy );
      glEnd();
      
      glPopMatrix();

      glPushMatrix();
      
      // shift up a little
      glTranslatef( 0,0,0.2 );      

      if( rob.best_vel )
	{
	  // draw a line to the best (v,w)
	  mod->PushColor( Color( 0,1,1,0.6 ) );
	  glBegin( GL_LINES );
	  glVertex2f( 0, 0 );
	  glVertex2f( rob.best_vel->dx, rob.best_vel->dy );
	  glEnd();
	}
      
      Color c[2] = { Color(1,0,0), Color(0,0,1,0.5) };
      
      glBegin (GL_POINTS );
      FOR_EACH( it, rob.vspace )
	{	  
       	  Color d = it->admissible ? c[0] : c[1]; 
       	  glColor4f( d.r, d.g, d.b, d.a );
	  glVertex2f( rob.best_vel->dx, rob.best_vel->dy );
       	}
       glEnd();
      
       // draw a line from each candidate point to the closest obstacle
      // glBegin (GL_LINES );
      // FOR_EACH( it, rob.vspace )
      // 	{	  
      // 	  glVertex3f( it->dx, it->dy, 0.2 );
      // 	  glVertex3f( it->ox, it->oy, 0.2 );

      // 	  it->Print();
      //  	}
      //  glEnd();
       
       

       // draw the obstacle cloud

       assert( rob.cloud.size() );

       glColor3f( 1, 0, 0 );
       FOR_EACH( it, rob.cloud )
	 it->Draw();       	   
                   
       // draw each trajectory as a faint circle
       // glColor4f( 0,0,0,0.2 );
       // double steps = 100;
       // FOR_EACH( it, rob.vspace )
       // 	 {	   	   	   
       // 	   glBegin( GL_LINE_LOOP );
       // 	   double radius = it->v / it->w;	   
       // 	   double a = (2.0 * M_PI) / steps;	       
       // 	   for( int i=0; i<steps; i++ )
       // 	     glVertex2f( fabs(radius) * cos( i*a ), radius + fabs(radius) * sin( i*a ) );	   
       // 	   glEnd();
       // 	 }
       
       // draw the cloud hits
       glColor3f( 0,0,0 );	   
       glBegin( GL_POINTS );
       FOR_EACH( it, rob.vspace )
	 {
	   //	   printf( "cloud huts len %d\n", (int)it->cloud_hits.size() );
	   
	   FOR_EACH( it2, it->cloud_hits )
	     glVertex2f( it2->first, it2->second );
	 }
       glEnd();	 	   
       

       // show the computation of arc length
       
       // hit point to circle center
       FOR_EACH( it, rob.vspace )
	 {
	   if( rob.best_vel == &*it )	 
	     {
	       glColor3f( 1,0,0 ); // red
	       glLineWidth(3);       
	     }
	   else
	     {
	       glColor4f( 0,0,0,0.15 ); // light gray	 
	       glLineWidth(1);       
	     }
	   
	   // show arcs until obstacles are hit
	   
	   double radius = it->v / it->w; 
	   double dist = it->obstacle_dist;
	   if( dist < 50 )
	     {		
	       // sanity check
	       glBegin( GL_LINE_STRIP );
	       for( double d=0; d<=dist; d+=dist/32.0 )
		 {	       
		   double w = M_PI - d/radius;	       
		   double x = radius * sin(w);	       
		   double y = radius + radius * cos(w);
		   glVertex2f( x, y  );
		 }
	       glEnd();
	     }
	   else
	     {
	       printf( "LONG v %.2f w %.2f dist %.2f\n", it->v, it->w, dist );	       
	     }
	       
	 }       
       glLineWidth( 1.0 );
       

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
       
       mod->PopColor();
    }
  };



// Stage calls this when the model starts up
extern "C" int Init( Model* mod, CtrlArgs* args )
{
  // local arguments
  /*  printf( "\nWander controller initialised with:\n"
      "\tworldfile string \"%s\"\n" 
      "\tcmdline string \"%s\"",
      args->worldfile.c_str(),
      args->cmdline.c_str() );
  */

  ModelPosition* pos = (ModelPosition*)mod;
  ModelRanger* laser = (ModelRanger*)mod->GetChild( "ranger:1" );
  
  assert( pos );
  assert( laser );

  Robot* rob = new Robot( pos, laser );

  pos->AddVisualizer( new DWVis(*rob), true );

  return 0; //ok
}


