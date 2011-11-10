#include "stage.hh"
using namespace Stg;

const bool verbose = false;

// navigation control params
const double cruisespeed = 0.4; 
const double avoidspeed = 0.05; 
const double avoidturn = 0.5;
const double minfrontdistance = 0.7;  
const double stopdist = 0.5;
const int avoidduration = 10;
const int workduration = 20;
const unsigned int payload = 1;


double refuel[4][4] = {
  {  0, 0, 45, 120 },
  { 0,-90, -60, -160 },
  { -90, -90, 180, 180 },
  { -90, -180, -90, -90 }
};

typedef enum {
  MODE_WORK=0,
  MODE_DOCK,
  MODE_UNDOCK
} nav_mode_t;

class Crumb 
{
public:
  Pose pose;
  int goal;
  double value;
  uint64_t expires;

  Crumb( const Pose& pose, int goal, double value, uint64_t expires ) 
    : pose(pose), goal(goal), value(value), expires(expires)
  {}
  
  double Distance( const Pose& from )
  {
    return pose.Distance2D( from );
  }  
};

class Robot
{
private:
  ModelPosition* pos;
  ModelRanger* laser;
  ModelRanger* ranger;
  ModelFiducial* fiducial;
  Model *source, *sink;
  int avoidcount, randcount;
  int work_get, work_put;
  bool charger_ahoy;
  double charger_bearing;
  double charger_range;
  double charger_heading;
  nav_mode_t mode;
  bool at_dest;
  double wander_goal;

public:

  static std::vector<Crumb> crumbs[2];
  std::vector<Crumb*> hood;
  std::vector<Crumb> trail;

  uint64_t crumb_interval;
  uint64_t crumb_ttl;
  double crumb_radius;
  //  double travel_time;
  Crumb* best_crumb;

  class LostVis : public Visualizer
  {
  public:    
    Robot& rob;
    
    LostVis( Robot& rob ) 
      : Visualizer( "LOST", "vis_lost" ), rob(rob) {}
    virtual ~LostVis(){}
    
    virtual void Visualize( Model* mod, Camera* cam )
    {
      const Pose& gp = mod->GetPose();

      mod->PushColor( Color( 0,1,0,0.5 ));

      // draw our read radius
      glBegin(GL_LINE_LOOP);
      for( double a=0; a<2.0*M_PI; a+= 0.2 )
	glVertex2f( rob.crumb_radius * cos(a),
		    rob.crumb_radius * sin(a) );
      
      glEnd();
	

      glPushMatrix();
      
      Gl::pose_inverse_shift( gp );
      
      glPointSize(3.0);

      
      double len = 0.1;

      // draw the local trail
      glColor3f( 1,0,1 );
      glBegin( GL_LINES );
      FOR_EACH( it, rob.trail )
	{	
	  glVertex2f( it->pose.x, it->pose.y );
	  glVertex2f( it->pose.x + len * cos(it->pose.a),
		      it->pose.y + len * sin(it->pose.a) );	  
	}
      glEnd();
      
      glBegin( GL_POINTS );      
      FOR_EACH( it, rob.trail )
	glVertex2f( it->pose.x, it->pose.y );
      glEnd();
      

      // draw the shared trails
      const Color cols[2] = { Color( 1,0,0 ), Color( 0,0,1 ) };      
      for( int g=0; g<2; g++ )
	{
	  glColor4f( cols[g].r,  cols[g].g,  cols[g].b,  cols[g].a );

	  glBegin( GL_LINES );
	  FOR_EACH( it, rob.crumbs[g] )
	    {	
	      glVertex2f( it->pose.x, it->pose.y );
	      glVertex2f( it->pose.x + len * cos(it->pose.a),
			  it->pose.y + len * sin(it->pose.a) );	  
	    }
	  glEnd();
	  	  
	  glBegin( GL_POINTS );      
	  FOR_EACH( it, rob.crumbs[g] )
	    glVertex2f( it->pose.x, it->pose.y );
	  glEnd();
	  
	  // char buf[32];
	  // FOR_EACH( it, rob.crumbs[g] )
	  //   {
	  //     snprintf( buf, 32, "%.0f", it->value );
	  //     Gl::draw_string( it->pose.x, it->pose.y, 0.0, buf );
	  //   }		  
	}
      

      // draw lines to all crumbs in the hood
      glColor3f( 0,1,0 );
      glBegin( GL_LINES );      
      FOR_EACH( it, rob.hood )
       	{
       	  glVertex2f( gp.x, gp.y );
       	  glVertex2f( (*it)->pose.x, (*it)->pose.y );
       	}
      glEnd();
      
      //mod->PopColor();
      
      if( rob.best_crumb )
      	{
	  glColor3f( 0,0,0 );
      	  glBegin( GL_LINES );            
      	    glVertex3f( gp.x, gp.y, 0.02 );
      	    glVertex3f( rob.best_crumb->pose.x, rob.best_crumb->pose.y, 0.02 );
      	  glEnd();
      	}
      
      glPopMatrix();
      mod->PopColor();
    }
  };
  
public:
  Robot( ModelPosition* pos, 
	 Model* source,
	 Model* sink ) 
    : pos(pos), 
      laser( (ModelRanger*)pos->GetChild( "ranger:1" )),
      ranger( (ModelRanger*)pos->GetChild( "ranger:0" )),
      fiducial( (ModelFiducial*)pos->GetUnusedModelOfType( "fiducial" )),	
      source(source), 
      sink(sink), 
      avoidcount(0), 
      randcount(0), 
      work_get(0), 
      work_put(0),
      charger_ahoy(false),
      charger_bearing(0),
      charger_range(0),
      charger_heading(0),
      mode(MODE_WORK),
      at_dest( false ),
      wander_goal(0),
      crumb_interval( 20 ),
      crumb_ttl( 5000 ),
      crumb_radius( 1.5 ),
      best_crumb(NULL)
  {
    // need at least these models to get any work done
    // (pos must be good, as we used it in the initialization list)
    assert( laser );
    assert( source );
    assert( sink );

    //	 pos->GetUnusedModelOfType( "laser" );
	 
    // PositionUpdate() checks to see if we reached source or sink
    pos->AddCallback( Model::CB_UPDATE, (model_callback_t)PositionUpdate, this );
    pos->Subscribe();

    // LaserUpdate() controls the robot, by reading from laser and
    // writing to position
    laser->AddCallback( Model::CB_UPDATE, (model_callback_t)LaserUpdate, this );
    laser->Subscribe();

    fiducial->AddCallback( Model::CB_UPDATE, (model_callback_t)FiducialUpdate, this );	 	 
    fiducial->Subscribe();
	 
    pos->AddCallback( Model::CB_FLAGINCR, (model_callback_t)Arrive, this );
    pos->AddCallback( Model::CB_FLAGDECR, (model_callback_t)Arrive, this );

    pos->AddVisualizer( new LostVis(*this), true );
		
  }

  void Dock()
  {

    if( charger_ahoy )
      {
	double a_goal = normalize( charger_bearing );				  
		
	// 		if( pos->Stalled() )
	//  		  {
	// 			 puts( "stalled. stopping" );
	//  			 pos->Stop();
	//		  }
	// 		else

	if( charger_range > 0.5 )
	  {
	    if( !ObstacleAvoid() )
	      {
		pos->SetXSpeed( cruisespeed );	  					 
		pos->SetTurnSpeed( a_goal );
	      }
	  }
	else	
	  {			
	    pos->SetTurnSpeed( a_goal );
	    pos->SetXSpeed( 0.02 );	// creep towards it				 

	    if( charger_range < 0.08 ) // close enough
	      pos->Stop();

	    if( pos->Stalled() ) // touching
	      pos->SetXSpeed( -0.01 ); // back off a bit			 

	  }			 
      }			  
    else
      {
	//printf( "docking but can't see a charger\n" );
	pos->Stop();
	mode = MODE_WORK; // should get us back on track eventually
      }

    // if the battery is charged, go back to work
    if( Full() )
      {
	//printf( "fully charged, now back to work\n" );
	mode = MODE_UNDOCK;
      }
  }


  void UnDock()
  {
    const meters_t back_off_distance = 0.3;
    const meters_t back_off_speed = -0.05;

    // back up a bit
    if( charger_range < back_off_distance )
      pos->SetXSpeed( back_off_speed );
    else
      pos->SetXSpeed( 0.0 );
  
    // if we're away from the charger, undock is finished: back to work
    if( charger_range > back_off_distance )	 
      mode = MODE_WORK;  
  }

  bool ObstacleAvoid()
  {
    bool obstruction = false;
    bool stop = false;
  
    // find the closest distance to the left and right and check if
    // there's anything in front
    double minleft = 1e6;
    double minright = 1e6;
	 
    // Get the data from the first sensor of the laser 
    const std::vector<meters_t>& scan = laser->GetSensors()[0].ranges;
	 
    uint32_t sample_count = scan.size();
  
    for (uint32_t i = 0; i < sample_count; i++)
      {		
	if( verbose ) printf( "%.3f ", scan[i] );
		
	if( (i > (sample_count/4)) 
	    && (i < (sample_count - (sample_count/4))) 
	    && scan[i] < minfrontdistance)
	  {
	    if( verbose ) puts( "  obstruction!" );
	    obstruction = true;
	  }
		
	if( scan[i] < stopdist )
	  {
	    if( verbose ) puts( "  stopping!" );
	    stop = true;
	  }
		
	if( i > sample_count/2 )
	  minleft = std::min( minleft, scan[i] );
	else      
	  minright = std::min( minright, scan[i] );
      }
  
    if( verbose ) 
      {
	puts( "" );
	printf( "minleft %.3f \n", minleft );
	printf( "minright %.3f\n ", minright );
      }
  
    if( obstruction || stop || (avoidcount>0) )
      {
	if( verbose ) printf( "Avoid %d\n", avoidcount );
		
	pos->SetXSpeed( stop ? 0.0 : avoidspeed );      
		
	/* once we start avoiding, select a turn direction and stick
	   with it for a few iterations */
	if( avoidcount < 1 )
	  {
	    if( verbose ) puts( "Avoid START" );
	    avoidcount = random() % avoidduration + avoidduration;
			 
	    if( minleft < minright  )
	      {
		pos->SetTurnSpeed( -avoidturn );
		if( verbose ) printf( "turning right %.2f\n", -avoidturn );
	      }
	    else
	      {
		pos->SetTurnSpeed( +avoidturn );
		if( verbose ) printf( "turning left %2f\n", +avoidturn );
	      }
	  }			  
		
	avoidcount--;

	return true; // busy avoding obstacles
      }
  
    return false; // didn't have to avoid anything
  }


  void Work()
  {
    if( ! ObstacleAvoid() )
      {
	if( verbose ) puts( "Cruise" );
		
	//avoidcount = 0;
	pos->SetXSpeed( cruisespeed );	  
		
	Pose pose = pos->GetPose();
		
	int x = (pose.x + 8) / 4;
	int y = (pose.y + 8) / 4;
		
	// oh what an awful bug - 5 hours to track this down. When using
	// this controller in a world larger than 8*8 meters, a_goal can
	// sometimes be NAN. Causing trouble WAY upstream. 
	if( x > 3 ) x = 3;
	if( y > 3 ) y = 3;
	if( x < 0 ) x = 0;
	if( y < 0 ) y = 0;

	
	if( best_crumb )
	  {
	    double a_goal = best_crumb->pose.a;	    

	    //dtor( ( pos->GetFlagCount() ) ? have[y][x] : need[y][x] );
	    
	    // if we are low on juice - find the direction to the recharger instead
	    if( Hungry() )		 
	      { 
		//puts( "hungry - using refuel map" );
		
		// use the refuel map
		a_goal = dtor( refuel[y][x] );
		
		if( charger_ahoy ) // I see a charger while hungry!
		  mode = MODE_DOCK;
	      }
	    
	    double a_error = normalize( a_goal - pose.a );
	    pos->SetTurnSpeed(  a_error );
	  }  	
	else
	  {
	    pos->SetTurnSpeed(0);
	  }
      }
}


  // inspect the laser data and decide what to do
  static int LaserUpdate( ModelRanger* laser, Robot* robot )
  {
    //   if( laser->power_pack && laser->power_pack->charging )
    // 	 printf( "model %s power pack @%p is charging\n",
    // 				laser->Token(), laser->power_pack );
		
    //assert( laser->GetSamples().size() > 0 );
		
    switch( robot->mode )
      {
      case MODE_DOCK:
	//puts( "DOCK" );
	robot->Dock();
	break;
		
      case MODE_WORK:
	//puts( "WORK" );
	robot->Work();
	break;

      case MODE_UNDOCK:
	//puts( "UNDOCK" );
	robot->UnDock();
	break;
		
      default:
	printf( "unrecognized mode %u\n", robot->mode );		
      }
  
    return 0;
  }

  bool Hungry()
  {
    return( pos->FindPowerPack()->ProportionRemaining() < 0.25 );
  }	 

  bool Full()
  {
    return( pos->FindPowerPack()->ProportionRemaining() > 0.95 );
  }	 

  static int PositionUpdate( ModelPosition* pos, Robot* robot )
  {  
    Pose pose = pos->GetPose();
  
    //printf( "Pose: [%.2f %.2f %.2f %.2f]\n",
    //  pose.x, pose.y, pose.z, pose.a );
  
    //pose.z += 0.0001;
    //robot->pos->SetPose( pose );
    
    Pose sourcep = robot->source->GetPose();
    Pose sinkp = robot->sink->GetPose();
    
    if( pos->GetFlagCount() < payload && 
	hypot( sourcep.x - pose.x, sourcep.y - pose.y ) < 2.0 )
      {
	if( ++robot->work_get > workduration )
	  {
	    // transfer a chunk from source to robot
	    pos->PushFlag( robot->source->PopFlag() );
	    robot->work_get = 0;	
	  }	  
      }
	 
    robot->at_dest = false;

    if( hypot( sinkp.x-pose.x, sinkp.y-pose.y ) < 2.0 )
      {
	robot->at_dest = true;

	if( ++robot->work_put > workduration )
	  {
	    //puts( "dropping" );
	    // transfer a chunk between robot and goal
	    robot->sink->PushFlag( pos->PopFlag() );
	    robot->work_put = 0;
	  }
      }
  

    //printf( "diss: %.2f\n", pos->FindPowerPack()->GetDissipated() );
  
    Pose p = pos->GetPose();

    uint64_t time =  pos->GetWorld()->UpdateCount();

    int goal = robot->pos->GetFlagCount() ? 1 : 0;

	
    // drop a crumb
    if( time % robot->crumb_interval == 0 )
	robot->trail.push_back( Crumb(p, goal, 0, time + robot->crumb_ttl ) );	   
	 
    // update the hood and TODO filter out old crumbs
    robot->hood.clear();	 
	 
    std::vector<Crumb>::iterator it = robot->crumbs[goal].begin();
	 
    robot->best_crumb = NULL;
    double best_dist = 1e99; // big
	 
    while( it != robot->crumbs[goal].end() )
      {
	// remove the crumb if it has expired
	if( time >= it->expires )
	  it = robot->crumbs[goal].erase( it );
	else
	  { 
	    double dist = it->Distance(p);

	    //printf( "dist %.2f best_dist %.2f\n", dist , best_dist );
		 
	    // is it in the hood?
	    if( dist < robot->crumb_radius )
	      {
		//printf( "< crumb radius %.2f\n", robot->crumb_radius );

		robot->hood.push_back( &(*it) );
		     
		// is this the best crumb yet seen?
		if( robot->best_crumb == NULL )
		  {
		    robot->best_crumb = &(*it);
		    best_dist = it->value;
		  }
		else
		  {
		    if( dist < best_dist )
		      {
			robot->best_crumb = &(*it);
			best_dist = it->value;

			// printf( "best dist %.2f crumb pose %.2f,%.2f\n", 
			// 	     best_dist, 
			// 	     robot->best_crumb->pose.x,
			// 	     robot->best_crumb->pose.y );
		      }
		  }			 		       
	      }	     
		 
	    ++it;
	  }
      }
  
    return 0; // run again
  }



  static int FiducialUpdate( ModelFiducial* mod, Robot* robot )
  {    
    robot->charger_ahoy = false;
		
    std::vector<ModelFiducial::Fiducial>& fids = mod->GetFiducials();
		
    for( unsigned int i = 0; i < fids.size(); i++ )
      {				
	//printf( "fiducial %d is %d at %.2f m %.2f radians\n",
	//	  i, f->id, f->range, f->bearing );		
		
	if( fids[i].id == 2 ) // I see a charging station
	  {
	    // record that I've seen it and where it is
	    robot->charger_ahoy = true;
	    robot->charger_bearing = fids[i].bearing;
	    robot->charger_range = fids[i].range;
	    robot->charger_heading = fids[i].geom.a;
					
	    //printf( "charger at %.2f radians\n", robot->charger_bearing );
	    break;
	  }
      }						  
 
    return 0; // run again
  }
  
  static int Arrive( Model* mod, Robot* robot )
  {
    //    robot->travel_time = 0;
    
    int goal = robot->pos->GetFlagCount() ? 0 : 1;

    //printf( "model %s arrives at goal %d\n", mod->Token(), goal );


    std::vector<Crumb>& global = robot->crumbs[goal];
    std::vector<Crumb>& local = robot->trail;

    // write the distances into the crumbs (reverse iteration)
    int dist = 1;
    for( std::vector<Crumb>::reverse_iterator rit = local.rbegin(); rit < local.rend(); ++rit )
      rit->value = dist++;
    
    // announce then clear local crumbs
    global.insert( global.end(), local.begin(), local.end() );
    local.clear();
		   
    return 0;
  }
  
};

// Stage calls this when the model starts up
extern "C" int Init( Model* mod, CtrlArgs* args )
{  
  new Robot( (ModelPosition*)mod,
	     mod->GetWorld()->GetModel( "source" ),
	     mod->GetWorld()->GetModel( "sink" ) );
  
  return 0; //ok
}



std::vector<Crumb> Robot::crumbs[2];
