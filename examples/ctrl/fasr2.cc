#include "stage.hh"
using namespace Stg;

#include "astar/astar.h"

const bool verbose = false;

// navigation control params
const double cruisespeed = 0.4; 
const double avoidspeed = 0.05; 
const double avoidturn = 0.5;
const double minfrontdistance = 0.7;  
const double stopdist = 0.5;
const int avoidduration = 10;
//const int workduration = 20;
const int PAYLOAD = 1;

//const int TRAIL_LENGTH_MAX = 500;

typedef enum {
  MODE_WORK=0,
  MODE_DOCK,
  MODE_UNDOCK
} nav_mode_t;




class Edge;

class Node
{
public:
  Pose pose;
  double value;  
  std::vector<Edge*> edges;
  
  Node( const Pose& pose )
	 : pose(pose), value(0), edges() {}
  
  ~Node();
  
  void AddEdge( Edge* edge ) 
  { 
	 assert(edge);
	 edges.push_back( edge ); 	 
  }   
  
  void Draw() const;
}; 

class Edge
{
public:
  Node* to;
  double cost;
  
  Edge( Node* to, double cost=1.0 ) 
	 : to(to), cost(cost) {}  
};

class Graph
{
public:
  std::vector<Node*> nodes;
  
  Graph(){}
  ~Graph() { FOR_EACH( it, nodes ){ delete *it; }}
  
  void AddNode( Node* node ){ nodes.push_back( node ); }
  
  void PopFront()
  {
	 const std::vector<Node*>::iterator& it = nodes.begin();
	 delete *it;
	 nodes.erase( it );
  }

  void Draw() const
  { 
	 glPointSize(4);
	 FOR_EACH( it, nodes ){ (*it)->Draw(); }
  }

  bool GoodDirection( const Pose& pose, stg_meters_t range, stg_radians_t& heading_result )
  {
	 // find the node with the lowest value within range of the given
	 // pose and return the absolute heading from pose that node 
	 
	 if( nodes.empty() )
		return 0; // a null guess
	 
	 
	 Node* best_node = NULL;
	 
	 // find the closest node
	 FOR_EACH( it, nodes )
		{
		  Node* node = *it;
		  double dist = hypot( node->pose.x - pose.x, node->pose.y - pose.y );
		  
		  // if it's in range, and either its the first we have found,
		  // or it has a lower value than the current best
		  if( dist < range &&  
				( best_node == NULL || node->value < best_node->value ))
			 {
				best_node = node;
			 }
		}
	 
	 if( best_node == NULL )
		{
		  puts( "warning: no nodes in range" );
		  return false;
		}
	 
	 //else
	 heading_result = atan2( best_node->pose.y - pose.y,
									 best_node->pose.x - pose.x );

	 return true;
  }
};


class GraphVis : public Visualizer
{
public:
  Graph& graph;
  
  GraphVis( Graph& graph ) 
	 : Visualizer( "graph", "vis_graph" ), graph(graph) {}
  virtual ~GraphVis(){}
  
  virtual void Visualize( Model* mod, Camera* cam )
  {

	 glPushMatrix();

	 Gl::pose_inverse_shift( mod->GetGlobalPose() );

	 //mod->PushColor( 1,0,0,1 );

	 mod->PushColor( mod->GetColor() );
	 graph.Draw();
	 mod->PopColor();

	 glPopMatrix();
  }
};


Node::~Node() 
{ 
  FOR_EACH( it, edges )
	 { delete *it; }
}

void Node::Draw() const
{
  // print value
  //char buf[32];
  //snprintf( buf, 32, "%.0f", value );
  //Gl::draw_string( pose.x, pose.y+0.2, 0.1, buf );

  glBegin( GL_POINTS );
  glVertex2f( pose.x, pose.y );
  glEnd();
  
  glBegin( GL_LINES );
  FOR_EACH( it, edges )
	 { 
		glVertex2f( pose.x, pose.y );
		glVertex2f( (*it)->to->pose.x, (*it)->to->pose.y );
	 }
  glEnd();
}

unsigned int MetersToCell( stg_meters_t m,  stg_meters_t size_m, unsigned int size_c )
{
  m += size_m / 2.0; // shift to local coords
  m /= size_m / (float)size_c; // scale
  return (unsigned int)floor(m); // quantize 
}

stg_meters_t CellToMeters( unsigned int c,  stg_meters_t size_m, unsigned int size_c )
{
  stg_meters_t cell_size = size_m/(float)size_c; 
  stg_meters_t m = c * cell_size; // scale
  m -= size_m / 2.0; // shift to local coords
  

  return m + cell_size/2.0; // offset to cell center
}


#include <pthread.h>

class Robot
{
private:
  ModelPosition* pos;
  ModelLaser* laser;
  ModelRanger* ranger;
  ModelFiducial* fiducial;
  Model *source, *sink;
  Model* fuel_zone;
  int avoidcount, randcount;
  int work_get, work_put;
  bool charger_ahoy;
  double charger_bearing;
  double charger_range;
  double charger_heading;
  nav_mode_t mode;

  stg_radians_t docked_angle;

  static pthread_mutex_t planner_mutex;

  Model* goal;

  Graph graph;
  GraphVis graphvis;
  Node* last_node;
  unsigned int node_interval;
  unsigned int node_interval_countdown;
  
  static unsigned int map_width;
  static unsigned int map_height;
  static uint8_t* map_data;
  static Model* map_model;
	 
  bool fiducial_sub;
  bool ranger_sub;
  

public:
  Robot( ModelPosition* pos, 
			Model* source,
			Model* sink,
			Model* fuel) 
	 : pos(pos), 
		laser( (ModelLaser*)pos->GetUnusedModelOfType( "laser" )),
		ranger( (ModelRanger*)pos->GetUnusedModelOfType( "ranger" )),
		fiducial( (ModelFiducial*)pos->GetUnusedModelOfType( "fiducial" )),	
		source(source), 
		sink(sink), 
		fuel_zone(fuel),
		avoidcount(0), 
		randcount(0), 
		work_get(0), 
		work_put(0),
		charger_ahoy(false),
		charger_bearing(0),
		charger_range(0),
		charger_heading(0),
		mode(MODE_WORK),
		docked_angle(0),
		goal(source),
		graph(),
		graphvis( graph ),
		last_node( NULL ),
		node_interval( 20 ),
		node_interval_countdown( node_interval ),
		fiducial_sub(false),
		ranger_sub(false)
  {
	 // need at least these models to get any work done
	 // (pos must be good, as we used it in the initialization list)
	 assert( laser );
	 assert( fiducial );
	 assert( ranger );
	 assert( source );
	 assert( sink );

	 //	 pos->GetUnusedModelOfType( "laser" );
	 
	 // PositionUpdate() checks to see if we reached source or sink
	 pos->AddUpdateCallback( (stg_model_callback_t)PositionUpdate, this );
	 pos->Subscribe();

	 // LaserUpdate() controls the robot, by reading from laser and
	 // writing to position
	 laser->AddUpdateCallback( (stg_model_callback_t)LaserUpdate, this );
	 laser->Subscribe();

	 // we subscribe to the fiducial device only while docking
	 fiducial->AddUpdateCallback( (stg_model_callback_t)FiducialUpdate, this );	 	 
	 
	 // we subscribe to the ranger device only while undocking (i.e. backing up)
	 //ranger->AddUpdateCallback( (stg_model_callback_t)RangerUpdate, this );	 	 	 
	 
	 pos->AddVisualizer( &graphvis, true );
	 
	 if( map_data == NULL )
		{
		  map_data = new uint8_t[map_width*map_height*2];
		  
		  // MUST clear the data, since Model::Rasterize() only enters
		  // non-zero pixels
		  memset( map_data, 0, sizeof(uint8_t) * map_width * map_height);
		  
		  // get the map
		  map_model = pos->GetWorld()->GetModel( "cave" );
		  assert(map_model);
		  Geom g = map_model->GetGeom();
		  
		  map_model->Rasterize( map_data, 
										map_width, 
										map_height, 
										g.size.x/(float)map_width, 
										g.size.y/(float)map_height );
		  
		  // 	 putchar( '\n' );
		  // 	 for( unsigned int y=0; y<map_height; y++ )
		  // 		{
		  // 		  for( unsigned int x=0; x<map_width; x++ )
		  // 			 printf( "%3d,", map_data[x + ((map_height-y-1)*map_width)] ? 999 : 1 );
		  
		  // 		  puts("");
		  // 		}    	 
		  
		  // fix the node costs for astar: 0=>1, 1=>9
		  
		  unsigned int sz = map_width * map_height;	 
		  for( unsigned int i=0; i<sz; i++ )
			 {
				if( map_data[i] == 0 )
				  map_data[i] = 1;
				else if( map_data[i] == 1 )
				  map_data[i] = 9;
				else
				  printf( "bad value %d in map at index %d\n", (int)map_data[i], (int)i );
				
				assert( (map_data[i] == 1) || (map_data[i] == 9) );		  
			 }    	 
		}
	 
	 //( goal );
	 //puts("");
  }
  
  void EnableRanger( bool on )
  { 
	 if( on && !ranger_sub )
		{ 
		  ranger_sub = true;
		  ranger->Subscribe();
		}
	 
	 if( !on && ranger_sub )
		{
		  ranger_sub = false;
		  ranger->Unsubscribe();
		}
  }

  void EnableFiducial( bool on )
  { 
	 if( on && !fiducial_sub )
		{ 
		  fiducial_sub = true;
		  fiducial->Subscribe();
		}
	 
	 if( !on && fiducial_sub )
		{
		  fiducial_sub = false;
		  fiducial->Unsubscribe();
		}
  }


  void Plan( Model* dest )
  {
	 Pose pose = pos->GetPose();
	 Pose sp = dest->GetPose();
	 Geom g = map_model->GetGeom();
	 
	 point_t start( MetersToCell(pose.x, g.size.x, map_width), 
						 MetersToCell(pose.y, g.size.y, map_height) );
	 point_t goal( MetersToCell(sp.x, g.size.x, map_width),
						MetersToCell(sp.y, g.size.y, map_height) );
	 
	 //printf( "searching from (%.2f, %.2f) [%d, %d]\n", pose.x, pose.y, start.x, start.y );
	 //printf( "searching to   (%.2f, %.2f) [%d, %d]\n", sp.x, sp.y, goal.x, goal.y );

	 // astar() is not reentrant, so we protect it thus
	 pthread_mutex_lock( &planner_mutex );

	 std::vector<point_t> path;
	 bool result = astar( map_data, 
								 map_width, 
								 map_height, 
								 start,
								 goal,
								 path );

	 pthread_mutex_unlock( &planner_mutex );

	 //printf( "#%s:\n", result ? "PATH" : "NOPATH" );

	 graph.nodes.clear();
	 
	 unsigned int dist = 0;
	 //FOR_EACH( it, path )
	 for( std::vector<point_t>::reverse_iterator rit = path.rbegin();
			rit != path.rend();
			++rit )			
	 {	
		//printf( "%d, %d\n", it->x, it->y );
		
		Node* node = new Node( Pose( CellToMeters(rit->x, g.size.x, map_width ),
											  CellToMeters(rit->y, g.size.y, map_height),
											  0, 0 ) );
		
		node->value = dist++;
		
		graph.AddNode( node );
		
		if( last_node )
		  last_node->AddEdge( new Edge( node ) );
		
		last_node = node;
	 }	 
  }
  
  void Dock()
  {
	 const stg_meters_t creep_distance = 0.5;
	 
	 if( charger_ahoy )
		{
		  double a_goal = normalize( charger_bearing );				  
		  
		  double orient = normalize( M_PI - (charger_bearing - charger_heading) );
		  //printf( "val %.2f\n", orient );
		  
		  //if( fabs(orient) < M_PI/4.0 )
		  a_goal = normalize( a_goal - 2.0 * orient );
		  
		  
		  // 		if( pos->Stalled() )
		  //  		  {
		  // 			 puts( "stalled. stopping" );
		  //  			 pos->Stop();
		  //		  }
		  // 		else
		  
		  // a_goal *= 2.0;
		  
		  if( charger_range > creep_distance )
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
				  {
					 pos->Stop();
					 docked_angle = pos->GetPose().a;
				  }
				
				if( pos->Stalled() ) // touching
				  pos->SetXSpeed( -0.01 ); // back off a bit			 				
			 }			 
		}			  
	 else
		{
		  printf( "%s docking but can't see a charger\n", pos->Token() );
		  pos->Stop();
		  EnableFiducial( false );
		  mode = MODE_WORK; // should get us back on track eventually
		}

	 // if the battery is charged, go back to work
	 if( Full() )
		{
		  //printf( "fully charged, now back to work\n" );		  
		  mode = MODE_UNDOCK;
		  EnableRanger(true); // enable the sonar to see behind us
		  //EnableFiducial(false);
		}
  }


  void UnDock()
  {
	 const stg_meters_t back_off_distance = 0.2;
	 const stg_meters_t back_off_speed = -0.02;
	 const stg_radians_t undock_rotate_speed = 0.3;
	 const stg_meters_t wait_distance = 0.2;
	 const unsigned int BACK_SENSOR_FIRST = 10;
	 const unsigned int BACK_SENSOR_LAST = 13;
	 
	 
	 if( charger_range < back_off_distance )
		{
		  pos->SetXSpeed( back_off_speed );

		  pos->Say( "" );
		  
		  // stay put while anything is close behind 
		  for( unsigned int s = BACK_SENSOR_FIRST; s <= BACK_SENSOR_LAST; ++s )
			 if( ranger->sensors[s].range < wait_distance) 
				{
				  pos->Say( "Waiting..." );
				  pos->SetXSpeed( 0.0 );
				  return;
				}	
		}
	 else
		{ // we've backed up enough
		  
		  double heading_error = normalize( pos->GetPose().a - (docked_angle + M_PI ) );
		  
		  if( fabs( heading_error ) > 0.05 ) 
			 {
				// turn
				pos->SetXSpeed( 0 );
				pos->SetTurnSpeed( undock_rotate_speed * sgn(-heading_error) );
			 }
		  else
			 {
				// we're pointing the right way, so we're done
				mode = MODE_WORK;  
				SetGoal( pos->GetFlagCount() ? sink : source );
				
				EnableFiducial(false);
				EnableRanger(false);
			 }
		}
  }
	
  bool ObstacleAvoid()
  {
	 bool obstruction = false;
	 bool stop = false;
  
	 // find the closest distance to the left and right and check if
	 // there's anything in front
	 double minleft = 1e6;
	 double minright = 1e6;
  
	 // Get the data
	 uint32_t sample_count=0;
	 ModelLaser::Sample* scan = laser->GetSamples( &sample_count );
    
	 for (uint32_t i = 0; i < sample_count; i++)
		{		
		  if( verbose ) printf( "%.3f ", scan[i].range );
		
		  if( (i > (sample_count/4)) 
				&& (i < (sample_count - (sample_count/4))) 
				&& scan[i].range < minfrontdistance)
			 {
				if( verbose ) puts( "  obstruction!" );
				obstruction = true;
			 }
		
		  if( scan[i].range < stopdist )
			 {
				if( verbose ) puts( "  stopping!" );
				stop = true;
			 }
		
		  if( i > sample_count/2 )
				minleft = std::min( minleft, scan[i].range );
		  else      
				minright = std::min( minright, scan[i].range );
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

  
  void SetGoal( Model* goal )
  {
	 if( goal != this->goal )
		{
		  this->goal = goal;
		  Plan( goal );
		}
  }
	 
  void Work()
  {
	 if( ! ObstacleAvoid() )
		{
		  if( verbose ) puts( "Cruise" );
		
		  //avoidcount = 0;
		
		  Pose pose = pos->GetPose();
		
		

		  double a_goal = 0;
		  
			 // dtor( ( pos->GetFlagCount() ) ? have[y][x] : need[y][x] );	// map
			 //atan2( gp.y - pose.y, gp.x - pose.x ); // crow flies
			 // use direction of lowest value node within range in graph 

		  
		  //		  Model* goal = fuel_zone;
		  
		  if( Hungry() )
			 SetGoal( fuel_zone );
		  
		  while( graph.GoodDirection( pose, 4.0, a_goal ) == 0 )
			 {
				//printf( "%s replanning from (%.2f,%.2f) to %s at (%.2f,%.2f) in Work()\n", 
				//	  pos->Token(),
				//	  pose.x, pose.y,
				//	  goal->Token(),
				//	  goal->GetPose().x,
				//	  goal->GetPose().y );
				Plan( goal );
			 }
		  
		  // if we are low on juice - find the direction to the recharger instead
		  if( Hungry() )		 
			 { 
				EnableFiducial(true);

				//puts( "hungry - using refuel map" );
				
				// use the refuel map
				//a_goal = dtor( refuel[y][x] );
				
				while( graph.GoodDirection( pose, 4.0, a_goal ) == 0 )
				  {
					 printf( "%s replanning in Work()\n", pos->Token() );
				  }
				
				
				if( charger_ahoy ) // I see a charger while hungry!
				  mode = MODE_DOCK;
			 }
		  
		  double a_error = normalize( a_goal - pose.a );
		  pos->SetTurnSpeed(  a_error );
			
			//double a_error_size = fabs(a_error);
			
			//if( a_error_size < 1.0 )
			//pos->SetXSpeed( (1.0 - a_error_size) *  cruisespeed );	  
			//else
			//pos->SetXSpeed( 0.0 );
			pos->SetXSpeed( cruisespeed );
		}  
  }

  
  static int RangerUpdate( ModelRanger* ranger, Robot* robot )
  {
		//printf( "%s RANGER UPDATE", ranger->Token() );
	 return 0;
  }


  // inspect the laser data and decide what to do
  static int LaserUpdate( ModelLaser* laser, Robot* robot )
  {
	 //   if( laser->power_pack && laser->power_pack->charging )
	 // 	 printf( "model %s power pack @%p is charging\n",
	 // 				laser->Token(), laser->power_pack );
  
	 if( laser->GetSamples(NULL) == NULL )
		return 0;

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
	 return( pos->FindPowerPack()->ProportionRemaining() < 0.2 );
  }	 

  bool Full()
  {
	 return( pos->FindPowerPack()->ProportionRemaining() > 0.95 );
  }	 

  static int PositionUpdate( ModelPosition* pos, Robot* robot )
  {  
	 Pose pose = pos->GetPose();

#if 0	 
	 // when countdown reaches zero
	 if( --robot->node_interval_countdown == 0 )
		{
		  // reset countdown
		  robot->node_interval_countdown = robot->node_interval;
		  
		  Node* node = new Node( pose );
		  robot->graph.AddNode( node );
		  
		  if( robot->last_node )
			 robot->last_node->AddEdge( new Edge( node ) );
		  
		  robot->last_node = node;
		  
		  // limit the number of nodes
		  while( robot->graph.nodes.size() > TRAIL_LENGTH_MAX )
			 robot->graph.PopFront();			 
		}
#endif  

	 //printf( "Pose: [%.2f %.2f %.2f %.2f]\n",
	 //  pose.x, pose.y, pose.z, pose.a );
  
	 //pose.z += 0.0001;
	 //robot->pos->SetPose( pose );
	 
	 // if we're close to the source we get a flag
	 Pose sourcepose = robot->source->GetPose();
	 Geom sourcegeom = robot->source->GetGeom();

	 if( hypot( sourcepose.x-pose.x, sourcepose.y-pose.y ) < sourcegeom.size.x/2.0 &&
		  pos->GetFlagCount() < PAYLOAD )
		{
		  // transfer a chunk from source to robot
		  pos->PushFlag( robot->source->PopFlag() );
		  
		  if( pos->GetFlagCount() == PAYLOAD )
		  	 robot->SetGoal( robot->sink ); // we're done working
		}
	 
	 // if we're close to the sink we lose a flag
	 Pose sinkpose = robot->sink->GetPose();
	 Geom sinkgeom = robot->sink->GetGeom();
	 
	 if( hypot( sinkpose.x-pose.x, sinkpose.y-pose.y ) < sinkgeom.size.x/2.0 && 
		  pos->GetFlagCount() > 0 )
		{
		  //puts( "dropping" );
		  // transfer a chunk between robot and goal
		  robot->sink->PushFlag( pos->PopFlag() );		  
		  
		  if( pos->GetFlagCount() == 0 ) 
			 robot->SetGoal( robot->source ); // we're done dropping off
		}
  

	 //printf( "diss: %.2f\n", pos->FindPowerPack()->GetDissipated() );
  
	 return 0; // run again
  }



  static int FiducialUpdate( ModelFiducial* mod, Robot* robot )
  {    
	 robot->charger_ahoy = false;
	 
	 std::vector<ModelFiducial::Fiducial>& fids = mod->GetFiducials();
	 
	 ModelFiducial::Fiducial* closest = NULL;		
	 
	 for( unsigned int i = 0; i < fids.size(); i++ )
		{				
		  //printf( "fiducial %d is %d at %.2f m %.2f radians\n",
		  //	  i, f->id, f->range, f->bearing );		
		  
		  ModelFiducial::Fiducial* f = &fids[i];
		  
		  // find the closest 
		  if( f->id == 2 && ( closest == NULL || f->range < closest->range )) // I see a charging station
			 closest = f; 
		}						  
	 
	 if( closest )
		{			 // record that I've seen it and where it is
		  robot->charger_ahoy = true;
		
		  //printf( "AHOY %s\n", robot->pos->Token() );

		  robot->charger_bearing = closest->bearing;
		  robot->charger_range = closest->range;
		  robot->charger_heading = closest->geom.a;			 
		}
	 
	 return 0; // run again
  }
  
  
  static int FlagIncr( Model* mod, Robot* robot )
 {
	 printf( "model %s collected flag\n", mod->Token() );
	 return 0;
  }

  static int FlagDecr( Model* mod, Robot* robot )
  {
	 printf( "model %s dropped flag\n", mod->Token() );
	 return 0;
  }
};


// STATIC VARS
pthread_mutex_t Robot::planner_mutex = PTHREAD_MUTEX_INITIALIZER;
unsigned int Robot::map_width( 60 );
unsigned int Robot::map_height( 60 );
uint8_t* Robot::map_data( NULL );
Model* Robot::map_model( NULL );

void split( const std::string& text, const std::string& separators, std::vector<std::string>& words)
{
  int n = text.length();
  int start, stop;
  start = text.find_first_not_of(separators);
  while ((start >= 0) && (start < n))
	 {
		stop = text.find_first_of(separators, start);
		if ((stop < 0) || (stop > n)) stop = n;
		words.push_back(text.substr(start, stop - start));
		start = text.find_first_not_of(separators, stop+1);
	 }
}

// Stage calls this when the model starts up
extern "C" int Init( Model* mod, CtrlArgs* args )
{  
  //printf( "%s args: %s\n", mod->Token(), args->worldfile.c_str() );
  
  // tokenize the argument string into words
  std::vector<std::string> words;
  split( args->worldfile, std::string(" \t"), words );
  
  //printf( "words size %u\n", words.size() );  
  //FOR_EACH( it, words )
  //printf( "word: %s\n", it->c_str() );
  //puts( "" );
  
  // expecting a task color name as the 1th argument
  assert( words.size() == 2 );
  assert( words[1].size() > 0 );
  
  new Robot( (ModelPosition*)mod,
				 mod->GetWorld()->GetModel( words[1] + "_source" ),
				 mod->GetWorld()->GetModel( words[1] + "_sink" ),
				 mod->GetWorld()->GetModel( "fuel_zone" ) );

  return 0; //ok
}



