#include "stage.hh"
using namespace Stg;

// implement signum
template<typename T> int sgn(T val)
{
    return (val > T(0)) - (val < T(0));
}

// number of different velocities and turn rates to consider
const double vres = 10.0;
const double wres = 10.0;

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
  
  //  static Vec Scale( double s, const Vec& v) 
  //  { return Vec( s*v.first, s*v.second); }

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


bool IntersectCircle( const Vec& E, 
		      const Vec& L, 
		      const Vec& C, // center of circle
		      const double r,
		      Vec& hit1,
		      Vec& hit2)
{ 
  const Vec d = L - E; // the obstacle line vector
  const Vec f = E - C; // vector from circle center to ray start
  
  // solve quadratic equation
  const double a = d.Dot(d);
  const double b = 2.0 * f.Dot(d);
  const double c = f.Dot(f) - r*r;
  double discriminant = b*b - 4*a*c;
  
  if( discriminant < 0 )
    return false;   //  no intersections  
  // else
  
  discriminant = sqrt( discriminant );
  const double t1 = (-b + discriminant)/(2.0*a);
  const double t2 = (-b - discriminant)/(2.0*a);
  
  //  printf( "C %.2f %.2f r %.2f t1 %.2f t2 %.2f\n",
  //	  C.first, C.second, r, t1, t2 )
  
  if( t1 >= 0 && t1 <= 1 )
      {
	// solution on is ON THE RAY.
	hit1 = t1 * d + E;	
      }
  else
    {
      // solution "out of range" of ray
    }
  
  if( t2 >= 0 && t2 <= 1 )
    {
      hit2 = t2 * d + E;	
    }
  else
    {
      // solution "out of range" of ray
    }
  
  return true;
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
      w(w), 
      score(0), 
      admissible(true),
      obstacle_dist(1e12),
      dx(0),
      dy(0),
      ox(0), 
      oy(0),
      cloud_hits()
  {
    dx = interval * v * cos( interval * w );
    dy = interval * v * sin( interval * w );
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
  
  std::vector<std::pair<Vec,Vec> > cloud;
  
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
	vspace.push_back( Vel( v, w ) );
  }
  
  
  // convert a ranger scan into a vector of line segments
  void ObstacleCloud( const ModelRanger::Sensor& sensor, 
		      double safety_radius, 
		      std::vector<std::pair<Vec,Vec> >& cloud )
  {
    //    std::vector<std::pair<Vec,Vec> > cloud;
        
    cloud.resize( sensor.sample_count );
    
    for( unsigned int i=0; i<sensor.sample_count; i++ )
      {
	double r = sensor.ranges[i];
	double a = sensor.bearings[i];
	double cosa = cos(a);
	double sina = sin(a);
		
	Vec hit( r * cosa, r * sina );			
	
	//const double len = 0.5 * M_PI/sensor.sample_count * r;		
	// length of line segment (not too small)
	double len = std::max( M_PI/sensor.sample_count * r, 0.2 );
	


	Vec offset( len * cos( M_PI/2.0 + a ),
		    len * sin( M_PI/2.0 + a ));	
		    

	cloud[i].first  = Vec( hit + offset );
	cloud[i].second = Vec( hit - offset );
      }
  }

  
  
  void AddCloudHits( double v, double w, std::vector<Vec>& hits )
  {
    // todo: get rid of this
    if( w == 0 )
      w = 0.01;	   	   	   
    
    double radius = v / w;       	       
    Vec circle( 0, radius ); // center of circle    
    
    Vec hit1, hit2;	       
    FOR_EACH( it, cloud )
      {	
	IntersectCircle( it->first, it->second,  circle, radius, hit1, hit2 );
		
	if( (hit1.first!=0.0) || (hit1.second!=0.0) ) // non zero vector
	  hits.push_back( hit1 );
	
	if( (hit2.first!=0.0) || (hit2.second!=0.0) ) // non zero vector
	  hits.push_back( hit2 );
      }
  }
  


  void EvaluateVSpace()
  {
    // get a cloud of obstacle line segments from the ranger
    ObstacleCloud( laser->GetSensors()[0], 0.5, cloud );
    
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
	//it->obstacle_dist = 0.0;//10.0;	

	it->admissible = 
	  PointsInsideRangerScan( laser->GetSensors()[0], *it );	
	
	//if( it->obstacle_dist < radius )
	//it->admissible = false;

	//it->admissible = true;
	

	// it's always ok to turn on the spot
	//if( fabs( it->v ) < 0.0001 )
	//it->admissible = true;
	
	if( it->admissible  )
	  {
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
	    else
	      {  // no hits - set to a sensible maximumum
		it->obstacle_dist = 8.0;
	      }
	    				
  
	    it->score = ObjectiveFunc( it->v, vmax, 
				       it->w, wmax,
				       goal_dist, 
				       goal_angle,
				       it->obstacle_dist );
	  }
	else
	  it->score = 0.0;	
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
      

      glPointSize( 6.0 );
 
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

       glColor3f( 0, 0, 0 );

       glBegin( GL_LINES );       
       FOR_EACH( it, rob.cloud )
	 {	   
	   glVertex2f( it->first.first, it->first.second );
	   glVertex2f( it->second.first, it->second.second );
	 }
       glEnd();
	   
                   
       // draw each trajectory as a faint circle
       glColor4f( 0,0,0,0.2 );
       double steps = 100;
       FOR_EACH( it, rob.vspace )
	 {	   	   	   
	   glBegin( GL_LINE_LOOP );
	   double radius = it->v / it->w;	   
	   double a = (2.0 * M_PI) / steps;	       
	   for( int i=0; i<steps; i++ )
	     glVertex2f( fabs(radius) * cos( i*a ), radius + fabs(radius) * sin( i*a ) );	   
	   glEnd();
	 }
       
       // draw the cloud hits
       glColor3f( 0,1,0 );	   
       glBegin( GL_POINTS );
       FOR_EACH( it, rob.vspace )
	 {
	   //	   printf( "cloud huts len %d\n", (int)it->cloud_hits.size() );
	   
	   FOR_EACH( it2, it->cloud_hits )
	     glVertex2f( it2->first, it2->second );
	 }
       glEnd();	 	   
       

       glColor3f( 1,0,1 ); // magenta
       // show the computation of arc length
       
       // hit point to circle center
       FOR_EACH( it, rob.vspace )
	 //FOR_EACH( it2, it->cloud_hits )
	 {
	   // find the distance along the circle until we hit the obstacle
	    double radius = it->v / it->w; 
	    double dist = it->obstacle_dist;
	    
	    //Vec a( 0, -radius );
	    //Vec b( it->ox, it->oy - radius );	    
	   // double g = acos( a.Dot(b) / (a.Length() * b.Length() ));	    
	   // double dist = fabs( g*radius );

	    if( dist < 10 )
	      {		
		// sanity check
		glBegin( GL_LINE_STRIP );
		for( double d=0.01; d<dist; d+=0.1 )
		  {	       
		    double w = M_PI -d / radius;	       
		    double x = radius * sin(w);	       
		    double y = radius + radius * cos(w);
		    glVertex3f( x, y, 0.5 );
		  }
		glEnd();
	      }
	    
	 }       


       
       

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


