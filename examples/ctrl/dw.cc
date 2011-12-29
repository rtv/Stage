#include "stage.hh"
#include "idw.h"

using namespace Stg;

const double safety = 0.7;

// time interval between updates
double interval = 0.1;

// distance from the robot to the goal in X and Y
double gdx = 0.0;
double gdy = 0.0;
		

// plug VS visualization into Stage
class Vis : public Visualizer
{
public:    
  
  VelSpace& vs;
  
  Vis( VelSpace& vs ) 
    : Visualizer( "velocity space", "vis_velocity_space" ), vs(vs) {}
  
  virtual ~Vis(){}
  
  virtual void Visualize( Model* mod, Camera* cam )
  { 
    mod->PushColor(Color(0,0,0));

    glPushMatrix();

    vs.Draw(); 

    mod->PopColor();
    glPopMatrix();
  }    
};


class Robot
{
public:  
  ModelPosition* pos;
  ModelRanger* laser;
  
  Pose goal;
  
  // describes circle that must not collide with obstacles. Must
  // contain the robot's body at least.
  double radius;  
  
  //  std::vector<Vel> vspace;
  Vel* best_vel;

  double v, w;
  
  std::vector<Circle> cloud;
  
  VelSpace vs;

  Robot( ModelPosition* pos, ModelRanger* laser ) : 
    pos(pos), 
    laser(laser), 
    goal( Pose( 7,3,0,0 )),
    radius(0.4),
    //    vspace(),
    best_vel(NULL),
    v(0.5),
    w(0),
    cloud(),
    vs( VCOUNT, WCOUNT,
	std::pair<double,double>( pos->velocity_bounds[0].min, 
				  pos->velocity_bounds[0].max ),
	std::pair<double,double>( pos->velocity_bounds[3].min, 
				  pos->velocity_bounds[3].max ),
	std::pair<double,double>( pos->acceleration_bounds[0].min, 
				  pos->acceleration_bounds[0].max ),
	std::pair<double,double>( pos->acceleration_bounds[3].min, 
				  pos->acceleration_bounds[3].max ) )	
  {
    pos->AddCallback( Model::CB_UPDATE, (model_callback_t)PositionUpdateCB, this );  
    pos->Subscribe(); // starts the position updates
    
    //laser->AddCallback( Model::CB_UPDATE, (model_callback_t)LaserUpdate, robot );
    laser->Subscribe(); // starts the ranger updates    
  }
    
  
  // // return the index of the vel with the highest score
  // Vel& BestVel( std::vector<Vel>& vels )
  // {
  //   unsigned int best_index = 0;
  //   double best_score = 0.0;
    
  //   for( unsigned int i=0; i<vels.size(); ++i )
  //     if( vels[i].inwindow && (vels[i].score > best_score) )
  // 	{
  // 	  best_score = vels[i].score;
  // 	  best_index = i;
  // 	}
    
  //   return vels[best_index];
  // }

  int PositionUpdate( void )
  {          
    
    // v = 0.5;
    //w = 0.0;

    // get a cloud of obstacle line segments from the ranger
    // cloud = ObstacleCloud( laser->GetSensors()[0], radius );
    
    // calculate the vspace array
    // EvaluateVSpace();          
    //best_vel = &BestVel( vspace ); 
    
    // goal range and bearing
    Pose gp = pos->GetGlobalPose();

    gdx = goal.x - gp.x;
    gdy = goal.y - gp.y;

    const double r = hypot( gdx, gdy );
    const double a = normalize( atan2( gdy, gdx ) - gp.a );
    
    const ModelRanger::Sensor& s = laser->GetSensors()[0];
    
    best_vel = &vs.Best( gp.x, gp.y, gp.a, v, w, r, a, s.ranges, s.bearings, radius, interval);
    
 
    //printf( "best vel %.2f %.2f\n", best_vel->v, best_vel->w );
    
    v  = best_vel->v;
    w =  best_vel->w;
    
  
    // and request those speeds
    pos->SetSpeed(v,0,w);
    
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




void Mesh( const std::vector<double>& arr, unsigned int w, unsigned int h, double sx, double sy, double sz )
{
  for( unsigned int x=0; x<w; x++ )
    {
      glBegin( GL_LINE_STRIP );  
      for( unsigned int y=0; y<h; y++ )      
	glVertex3f( sx*x, sy*y, sz*arr[y + x*w] );
      
      glEnd(); 
    }

  for( unsigned int y=0; y<h; y++ )
    {
      glBegin( GL_LINE_STRIP );  
      for( unsigned int x=0; x<w; x++ )      
	glVertex3f( sx*x, sy*y, sz*arr[y + x*w] );      
      glEnd(); 
    }  

  // draw the bouding cube

  double x = sx*w;
  double y = sy*h;

  glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );

  glBegin( GL_QUADS );

  glVertex3f( 0, 0,  0 );
  glVertex3f( x, 0,  0 );
  glVertex3f( x, y,  0 );
  glVertex3f( 0, y,  0 );

  glVertex3f( 0, 0,  1 );
  glVertex3f( x, 0,  1 );
  glVertex3f( x, y,  1 );
  glVertex3f( 0, y,  1 );

  glEnd();

  glBegin( GL_LINES );

  glVertex3f( 0, 0,  0 );
  glVertex3f( 0, 0,  1 );

  glVertex3f( x, 0,  0 );
  glVertex3f( x, 0,  1 );

  glVertex3f( x, y,  0 );
  glVertex3f( x, y,  1 );

  glVertex3f( 0, y,  0 );
  glVertex3f( 0, y,  1 );

  glEnd();


}

// void DrawMesh( double x, double y, double z, const std::vector<Vel>& vels, const Vel& best_vel )
// {
//   glPushMatrix();  
//   glTranslatef( x,y,z );
  
//   glPointSize( 2.0 );

//   double sx = 1.0 / vres;;
//   double sy = 1.0 / wres;
//   //double sz = 0.03;
  
//   int side = sqrt( vels.size() );  
  

//    std::vector<double> obstacles;   
//    FOR_EACH( it, vels )
//      obstacles.push_back( it->obstacle_dist / obstacle_dist_max ); 
//    glColor3f( 1,0,0 );      
//    Mesh( obstacles, side, side, sx, sy, 1.0 );

//    glTranslatef( 0,1,0 );

//    std::vector<double> heading;   
//    FOR_EACH( it, vels )
//      heading.push_back( it->score_heading ); 
//    glColor3f( 0,0,1 );      
//    Mesh( heading, side, side, sx, sy, 1.0 );

//    glTranslatef( 0,1,0 );

//    std::vector<double> speed;   
//    FOR_EACH( it, vels )
//      speed.push_back( it->score_speed ); 
//    glColor3f( 0,1,0 );      
//    Mesh( speed, side, side, sx, sy, 1.0 );
//    glTranslatef( 0,1,0 );

//    std::vector<double> overall;   
//    FOR_EACH( it, vels )
//      overall.push_back( it->score ); 
//    glColor3f( 0.5,  0.5, 0.5 );      
//    Mesh( overall, side, side, sx, sy, 1.0 );

//    glPopMatrix();    
// }



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

  //pos->AddVisualizer( new DWVis(*rob), true );

  pos->AddVisualizer( new Vis(rob->vs), true );

  return 0; //ok
}


