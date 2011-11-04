#include <pthread.h>

#include "stage.hh"
using namespace Stg;

// generic planner implementation
#include "astar/astar.h"

static const bool verbose = false;

// navigation control params
static const double cruisespeed = 0.4; 
static const double avoidspeed = 0.05; 
static const double avoidturn = 0.5;
static const double minfrontdistance = 0.7;  
static const double stopdist = 0.5;
static const unsigned int avoidduration = 10;
static const unsigned int PAYLOAD = 1;


static unsigned int 
MetersToCell( meters_t m,  meters_t size_m, unsigned int size_c )
{
  m += size_m / 2.0; // shift to local coords
  m /= size_m / (float)size_c; // scale
  return (unsigned int)floor(m); // quantize 
}

static meters_t 
CellToMeters( unsigned int c,  meters_t size_m, unsigned int size_c )
{
  meters_t cell_size = size_m/(float)size_c; 
  meters_t m = c * cell_size; // scale
  m -= size_m / 2.0; // shift to local coords

  return m + cell_size/2.0; // offset to cell center
}


class Robot
{
public: 
	class Task
	{
	public:
		Model* source;
		Model* sink;
		unsigned int participants;
		
		Task( Model* source, Model* sink ) 
			: source(source), sink(sink), participants(0) 
		{}
	};
	
  static std::vector<Task> tasks;

private:
	class Node;
	
	class Edge
	{
	public:
		Node* to;
		double cost;
		
		Edge( Node* to, double cost=1.0 ) 
			: to(to), cost(cost) {}  
	};
	
	class Node
	{
	public:
		Pose pose;
		double value;  
		std::vector<Edge*> edges;
		
		Node( Pose pose, double value=0 )
			: pose(pose), value(value), edges() {}
		
		~Node() 
		{ 
			FOR_EACH( it, edges )
				{ delete *it; }
		}
		
		void AddEdge( Edge* edge ) 
		{ 
			assert(edge);
			edges.push_back( edge ); 	 
		}   
		
		void Draw() const;
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
			glPointSize(3);
			FOR_EACH( it, nodes ){ (*it)->Draw(); }
		}
		
		bool GoodDirection( const Pose& pose, meters_t range, radians_t& heading_result )
		{
			// find the node with the lowest value within range of the given
			// pose and return the absolute heading from pose to that node 
			
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
					fprintf( stderr, "FASR warning: no nodes in range" );
					return false;
				}

			//else
			heading_result = atan2( best_node->pose.y - pose.y,
															best_node->pose.x - pose.x );
			
			// add a little bias to one side (the left) - creates two lanes
			// of robot traffic
			heading_result = normalize( heading_result + 0.25 );

			return true;
		}
	};

	class GraphVis : public Visualizer
	{
	public:
		Graph** graphpp;
		
		GraphVis( Graph** graphpp ) 
			: Visualizer( "graph", "vis_graph" ), graphpp(graphpp) {}
		virtual ~GraphVis(){}
		
		virtual void Visualize( Model* mod, Camera* cam )
		{
			if( *graphpp == NULL )
				return;
			
			glPushMatrix();
			
			Gl::pose_inverse_shift( mod->GetGlobalPose() );
			
			//mod->PushColor( 1,0,0,1 );
			
			Color c = mod->GetColor();
			c.a = 0.4;
			
			mod->PushColor( c );
			(*graphpp)->Draw();
			mod->PopColor();
			
			glPopMatrix();
		}
	};
	

private:
  long int wait_started_at;
  
  ModelPosition* pos;
  ModelRanger* laser;
  ModelRanger* sonar;
  ModelFiducial* fiducial;
  
  unsigned int task;

  Model* fuel_zone;
  Model* pool_zone;
  int avoidcount, randcount;
  int work_get, work_put;
  bool charger_ahoy;
  double charger_bearing;
  double charger_range;
  double charger_heading;
	
	enum {
		MODE_WORK=0,
		MODE_DOCK,
		MODE_UNDOCK,
		MODE_QUEUE
	} mode;
	
  radians_t docked_angle;

  static pthread_mutex_t planner_mutex;

  Model* goal;
  Pose cached_goal_pose;

  Graph* graphp;
  GraphVis graphvis;
  unsigned int node_interval;
  unsigned int node_interval_countdown;
  
  static const unsigned int map_width;
  static const unsigned int map_height;
  static uint8_t* map_data;
  static Model* map_model;
	 
  bool fiducial_sub;
  bool laser_sub;
  bool sonar_sub;

  bool force_recharge;

public:  

  Robot( ModelPosition* pos, 
				 Model* fuel,
				 Model* pool ) 
		: 
		wait_started_at(-1),
		pos(pos), 
		laser( (ModelRanger*)pos->GetChild( "ranger:1" )),
		sonar( (ModelRanger*)pos->GetChild( "ranger:0" )),
		fiducial( (ModelFiducial*)pos->GetUnusedModelOfType( "fiducial" )),	
		task(random() % tasks.size() ), // choose a task at random
		fuel_zone(fuel),
		pool_zone(pool),
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
		goal(tasks[task].source),
		cached_goal_pose(),
		graphp(NULL),
		graphvis( &graphp ),
		node_interval( 20 ),
		node_interval_countdown( node_interval ),
		fiducial_sub(false),		
		laser_sub(false),
		sonar_sub(false),
		force_recharge( false )
  {
		// need at least these models to get any work done
		// (pos must be good, as we used it in the initialization list)
		assert( laser );
		assert( fiducial );
		assert( sonar );
		assert( goal );

		// match the color of my destination
		pos->SetColor( tasks[task].source->GetColor() );
		
		tasks[task].participants++;
		
		EnableLaser(true);

		// we access all other data from the position callback
		pos->AddCallback(  Model::CB_UPDATE, (model_callback_t)UpdateCallback, this );
		pos->Subscribe();

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
		  
				// fix the node costs for astar: 0=>1, 1=>9
		  
				unsigned int sz = map_width * map_height;	 
				for( unsigned int i=0; i<sz; i++ )
					{
						if( map_data[i] == 0 )
							map_data[i] = 1;
						else if( map_data[i] == 1 )
							map_data[i] = 9;
						else
							printf( "FASR: bad value %d in map at index %d\n", (int)map_data[i], (int)i );
				
						assert( (map_data[i] == 1) || (map_data[i] == 9) );		  
					}    	 
			}
	 
		//( goal );
		//puts("");
  }
 
	
	void Enable( Stg::Model* model, bool& sub, bool on )
	{
		if( on && !sub )
			{ 
				sub = true;
				model->Subscribe();
			}
		
		if( !on && sub )
			{
				sub = false;
				model->Unsubscribe();
			}
	}
	
  void EnableSonar( bool on )
  { 
		Enable( sonar, sonar_sub, on );
  }

  void EnableLaser( bool on )
  { 
		Enable( laser, laser_sub, on );
  }

  void EnableFiducial( bool on )
  { 
		Enable( fiducial, fiducial_sub, on );
  }

	static std::map< std::pair<uint64_t,uint64_t>, Graph*> plancache;
	

	uint64_t Pt64( const ast::point_t& pt)
	{
		// quantize the position a bit to reduce planning frequency
		uint64_t x = pt.x / 1;
		uint64_t y = pt.y / 1;
		
		return (x<<32) + y;
	}
	
	void CachePlan( const ast::point_t& start, const ast::point_t& goal, Graph* graph )
	{
		std::pair<uint64_t,uint64_t> key( Pt64(start),Pt64(goal));
		plancache[key] = graph;
	}
	
	Graph* LookupPlan( const ast::point_t& start, const ast::point_t& goal )
	{
		std::pair<uint64_t,uint64_t> key(Pt64(start),Pt64(goal));
		return plancache[key];		
	}


 void Plan( Pose sp )
  {
		static float hits = 0;
		static float misses = 0;

		// change my color to that of my destination
		//pos->SetColor( dest->GetColor() );
		
		Pose pose = pos->GetPose();
		Geom g = map_model->GetGeom();
		
		ast::point_t start( MetersToCell(pose.x, g.size.x, map_width), 
									 MetersToCell(pose.y, g.size.y, map_height) );
		ast::point_t goal( MetersToCell(sp.x, g.size.x, map_width),
									MetersToCell(sp.y, g.size.y, map_height) );
		
		//printf( "searching from (%.2f, %.2f) [%d, %d]\n", pose.x, pose.y, start.x, start.y );
		//printf( "searching to   (%.2f, %.2f) [%d, %d]\n", sp.x, sp.y, goal.x, goal.y );
		
		// astar() is not reentrant, so we protect it thus
		
		//graph.nodes.clear(); // whatever happens, we clear the old plan
		

		//pthread_mutex_lock( &planner_mutex );
		
		// check to see if we have a path planned for these positions already
		//printf( "plancache @ %p size %d\n", &plancache, (int)plancache.size() );
		
		//if( graphp )
		//delete graphp;

		graphp = LookupPlan( start, goal );

		if( ! graphp ) // no plan cached
			{
				misses++;
				
				std::vector<ast::point_t> path;
				bool result = ast::astar( map_data, 
																		map_width, 
																		map_height, 
																		start,
																		goal,
																		path );				
				
				if( ! result )
					printf( "FASR2 warning: plan failed to find path from (%.2f,%.2f) to (%.2f,%.2f)\n",
									pose.x, pose.y, sp.x, sp.y );				
				
				graphp = new Graph();
				
				unsigned int dist = 0;
				
				Node* last_node = NULL;

				for( std::vector<ast::point_t>::reverse_iterator rit = path.rbegin();
						 rit != path.rend();
						 ++rit )			
					{	
						//printf( "%d, %d\n", it->x, it->y );
						
						Node* node = new Node( Pose( CellToMeters(rit->x, g.size.x, map_width ),
																				 CellToMeters(rit->y, g.size.y, map_height),
																				 0, 0 ),
																	 dist++ ); // value stored at node
						
						graphp->AddNode( node );
						
 						if( last_node )
 							last_node->AddEdge( new Edge( node ) );
						
 						last_node = node;
					}	 
				
				CachePlan( start, goal, graphp );
			}
		else
			{
				hits++;
				//puts( "FOUND CACHED PLAN" );
			}
				
		//printf( "hits/misses %.2f\n", hits/misses );
		//pthread_mutex_unlock( &planner_mutex );
	}
	
		
  void Dock()
  {
		const meters_t creep_distance = 0.5;
	 
		if( charger_ahoy )
			{
				// drive toward the charger
				// orient term helps to align with the way the charger is pointing
				const double orient = normalize( M_PI - (charger_bearing - charger_heading) );
				const double a_goal = normalize( charger_bearing - 2.0 * orient );
		  		  
				if( charger_range > creep_distance )
					{
						if( !ObstacleAvoid() )
							{
								pos->SetXSpeed( cruisespeed );	  					 
								pos->SetTurnSpeed( a_goal );
							}
					}
				else	// we are very close!
					{			
						pos->SetTurnSpeed( a_goal );
						pos->SetXSpeed( 0.02 );	// creep towards it				 
				
						if( charger_range < 0.08 ) // close enough
							{
								pos->Stop();
								docked_angle = pos->GetPose().a;
								EnableLaser( false );
							}
				
						if( pos->Stalled() ) // touching
							pos->SetXSpeed( -0.01 ); // back off a bit			 				
					}			 
			}			  
		else
			{
				//printf( "FASR: %s docking but can't see a charger\n", pos->Token() );
				pos->Stop();
				EnableFiducial( false );
				mode = MODE_WORK; // should get us back on track eventually
			}

		// if the battery is charged, go back to work
		if( Full() )
			{
				//printf( "fully charged, now back to work\n" );		  
				mode = MODE_UNDOCK;
				EnableSonar(true); // enable the sonar to see behind us
				EnableLaser(true);
				EnableFiducial(true);
				force_recharge = false;		  
			}	 
  }

  void SetTask( unsigned int t )
  {
		task = t;
		tasks[task].participants++;
  }

  void UnDock()
  {
		const meters_t back_off_distance = 0.2;
		const meters_t back_off_speed = -0.02;
		const radians_t undock_rotate_speed = 0.3;
		const meters_t wait_distance = 0.2;
		const unsigned int BACK_SENSOR_FIRST = 10;
		const unsigned int BACK_SENSOR_LAST = 13;
	 
		// make sure the required sensors are running
		assert( sonar_sub );
		assert( fiducial_sub );

		if( charger_range < back_off_distance )
			{
				pos->SetXSpeed( back_off_speed );

				pos->Say( "" );
		  
				// stay put while anything is close behind 
				const std::vector<ModelRanger::Sensor>& sensors = sonar->GetSensors();

				for( unsigned int s = BACK_SENSOR_FIRST; s <= BACK_SENSOR_LAST; ++s )
					if( sensors[s].ranges.size() < 1 || sensors[s].ranges[0] < wait_distance) 
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
				
						// if we're not working on a drop-off
						if( pos->GetFlagCount() == 0 )
							{
								// pick a new task at random
								SetTask( random() % tasks.size() );
								SetGoal( tasks[task].source );
							}
						else
							SetGoal( tasks[task].sink );

				
						EnableFiducial(false);
						EnableSonar(false);
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
		//const std::vector<ModelLaser::Sample>& scan = laser->GetSamples();

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

  
  void SetGoal( Model* goal )
  {	
		if( goal != this->goal )
			{
				this->goal = goal;
				Plan( goal->GetPose() );
				pos->SetColor( goal->GetColor() );
			}
  }
	 
  void Work()
  {
		if( ! ObstacleAvoid() )
			{
				if( verbose ) 
					puts( "Cruise" );
				
				if( Hungry() )
					SetGoal( fuel_zone );
				
				const Pose pose = pos->GetPose();		
				double a_goal = 0;
				
				// if the graph fails to offer advice or the goal has moved a
				// ways since last time we planned
				if( graphp == NULL || 
						(graphp->GoodDirection( pose, 5.0, a_goal ) == 0) || 
						(goal->GetPose().Distance2D( cached_goal_pose ) > 0.5) )
					{
						//printf( "%s replanning from (%.2f,%.2f) to %s at (%.2f,%.2f) in Work()\n", 
						//	  pos->Token(),
						//	  pose.x, pose.y,
						//	  goal->Token(),
						//	  goal->GetPose().x,
						//	  goal->GetPose().y );
						Plan( goal->GetPose() );
						cached_goal_pose = goal->GetPose();				
					}
		  
				// if we are low on juice - find the direction to the recharger instead
				if( Hungry() )		 
					{ 
						EnableFiducial(true);
				
						while( graphp->GoodDirection( pose, 5.0, a_goal ) == 0 )
							{
								printf( "%s replanning in Work()\n", pos->Token() );
							}
				
				
						if( charger_ahoy ) // I see a charger while hungry!
							mode = MODE_DOCK;
					}
				
				const double a_error = normalize( a_goal - pose.a );
				pos->SetTurnSpeed(  a_error );			
				pos->SetXSpeed( cruisespeed );
			}  
  }


  bool Hungry()
  {
		return( force_recharge || pos->FindPowerPack()->ProportionRemaining() < 0.2 );
  }	 

  bool Full()
  {
		return( pos->FindPowerPack()->ProportionRemaining() > 0.95 );
  }	 
  
  // static callback wrapper
  static int UpdateCallback( ModelRanger* laser, Robot* robot )
  {  
		return robot->Update();
  }
  
  int Update( void )
  {
		if( strcmp( pos->Token(), "position:0") == 0 )
			{
				static int seconds = 0;
		  
				int timenow = pos->GetWorld()->SimTimeNow() / 1e6;
		  
				if( timenow - seconds > 5 )
					{
						seconds = timenow;
				
						// report the task participation		  
						//						printf( "time: %d sec\n", seconds ); 
						  
// 						unsigned int sz = tasks.size(); 
// 						for( unsigned int i=0; i<sz; ++i )
// 							printf( "\t task[%u] %3u (%s)\n", 
// 											i, tasks[i].participants, tasks[i].source->Token() );			 				
					}
			}
	 
		Pose pose = pos->GetPose();

	 
		// assume we can't see the charger
		charger_ahoy = false;
	 
		// if the fiducial is enabled
		if( fiducial_sub ) 
			{	 
				std::vector<ModelFiducial::Fiducial>& fids = fiducial->GetFiducials();
		  
				if( fids.size() > 0 )
					{
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
								charger_ahoy = true;
					 
								//printf( "AHOY %s\n", pos->Token() );
					 
								charger_bearing = closest->bearing;
								charger_range = closest->range;
								charger_heading = closest->geom.a;			 
							}
					}
			}


		//printf( "Pose: [%.2f %.2f %.2f %.2f]\n",
		//  pose.x, pose.y, pose.z, pose.a );
  
		//pose.z += 0.0001;
		//pos->SetPose( pose );
	 
		if( goal == tasks[task].source )
			{
				// if we're close to the source we get a flag
				Pose sourcepose = goal->GetPose();
				Geom sourcegeom = goal->GetGeom();
		  
				meters_t dest_dist = hypot( sourcepose.x-pose.x, sourcepose.y-pose.y );
		  
				// when we get close enough, we start waiting
				if( dest_dist < sourcegeom.size.x/2.0 ) // nearby
					if( wait_started_at < 0 && pos->GetFlagCount() == 0 ) 
						{
							wait_started_at = pos->GetWorld()->SimTimeNow() / 1e6; // usec to seconds
							//printf( "%s begain waiting at %ld seconds\n", pos->Token(), wait_started_at );
						}
		  
				if( wait_started_at > 0 )
					{
						//long int waited = (pos->GetWorld()->SimTimeNow() / 1e6) - wait_started_at;
				
						// leave with small probability
						if( drand48() < 0.0005 )
							{
								//printf( "%s abandoning task %s after waiting %ld seconds\n",
								//		pos->Token(), goal->Token(), waited );
					 
								force_recharge = true; // forces hungry to return true
								tasks[task].participants--;
								wait_started_at = -1;
								return 0;
							}
					}

				// when we get onto the square
				if( dest_dist < sourcegeom.size.x/2.0 &&				
						pos->GetFlagCount() < PAYLOAD )
					{
				
						// transfer a chunk from source to robot
						pos->PushFlag( goal->PopFlag() );

						if( pos->GetFlagCount() == 1 && wait_started_at > 0 )
							{
								// stop waiting, since we have received our first flag
								wait_started_at = -1;
							} 			 

						if( pos->GetFlagCount() == PAYLOAD )
							SetGoal( tasks[task].sink ); // we're done working
					}
			}	 
		else if( goal == tasks[task].sink )
			{			 
				// if we're close to the sink we lose a flag
				Pose sinkpose = goal->GetPose();
				Geom sinkgeom = goal->GetGeom();
		  
				if( hypot( sinkpose.x-pose.x, sinkpose.y-pose.y ) < sinkgeom.size.x/2.0 && 
						pos->GetFlagCount() > 0 )
					{
						//puts( "dropping" );
						// transfer a chunk between robot and goal

						Model::Flag* f = pos->PopFlag();
						f->SetColor( Color(0,1,0) ); // delivered flags are green
						goal->PushFlag( f );		  
				
						if( pos->GetFlagCount() == 0 ) 
							SetGoal( tasks[task].source ); // we're done dropping off
					}
			}
		else
			{
				assert( goal == fuel_zone || goal == pool_zone );
			}
		  
		switch( mode )
			{
			case MODE_DOCK:
				//puts( "DOCK" );
				Dock();
				break;
		
			case MODE_WORK:
				//puts( "WORK" );
				Work();
				break;

			case MODE_UNDOCK:
				//puts( "UNDOCK" );
				UnDock();
				break;
		
			default:
				printf( "unrecognized mode %u\n", mode );		
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


void Robot::Node::Draw() const
{
  // print value
  //char buf[32];
  //snprintf( buf, 32, "%.0f", value );
  //Gl::draw_string( pose.x, pose.y+0.2, 0.1, buf );

  //glBegin( GL_POINTS );
  //glVertex2f( pose.x, pose.y );
  //glEnd();

	glBegin( GL_LINES );
	FOR_EACH( it, edges )
		{ 
			glVertex2f( pose.x, pose.y );
			glVertex2f( (*it)->to->pose.x, (*it)->to->pose.y );
		}
	glEnd();
}


// STATIC VARS
pthread_mutex_t Robot::planner_mutex = PTHREAD_MUTEX_INITIALIZER;
const unsigned int Robot::map_width( 32 );
const unsigned int Robot::map_height( 32 );
uint8_t* Robot::map_data( NULL );
Model* Robot::map_model( NULL );

std::map< std::pair<uint64_t,uint64_t>, Robot::Graph*> Robot::plancache;

std::vector<Robot::Task> Robot::tasks;

void split( const std::string& text, const std::string& separators, std::vector<std::string>& words)
{
  const int n = text.length();
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
extern "C" int Init( ModelPosition* mod, CtrlArgs* args )
{  
  // some init for only the first controller
  if( Robot::tasks.size() == 0 )
		{
			srandom( time(NULL) );
			
			// tokenize the worldfile ctrl argument string into words
			std::vector<std::string> words;
			split( args->worldfile, std::string(" \t"), words );			
			// expecting at least one task color name 
			assert( words.size() > 1 );
			
			const World* w = mod->GetWorld();
			
			// make an array of tasks by reading the controller arguments
			for( unsigned int s=1; s<words.size(); s++ )
				Robot::tasks.push_back( Robot::Task( w->GetModel( words[s] + "_source"),
																						 w->GetModel( words[s] + "_sink") ) );			 
		}
	
  new Robot( mod,
						 mod->GetWorld()->GetModel( "fuel_zone" ),
						 mod->GetWorld()->GetModel( "pool_zone" ) );
	
  return 0; //ok
}



