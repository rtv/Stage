#include "stage.hh"
using namespace Stg;

#include <FL/Fl_Shared_Image.H>
#include <libstandalone_drivers/plan.h>

int read_map_from_image(int* size_x, int* size_y, char** mapdata, 
			const char* fname, int negate );




const bool verbose = false;

// navigation control params
const double cruisespeed = 0.4; 
const double avoidspeed = 0.05; 
const double avoidturn = 0.5;
const double minfrontdistance = 0.4;  
const double stopdist = 0.3;
const int avoidduration = 10;
const int workduration = 20;
const int payload = 1;

typedef enum {
  MODE_WORK=0,
  MODE_DOCK,
  MODE_UNDOCK
} nav_mode_t;


#define MAP_IDX(sx, i, j) ((sx) * (j) + (i))


static guchar* pb_get_pixel( Fl_Shared_Image* img, int x, int y )
{
  guchar* pixels = (guchar*)(img->data()[0]);
  unsigned int index = (y * img->w() * img->d()) + (x * img->d());
  return( pixels + index );
}

// returns true if the value in the first channel is above threshold
static gboolean pb_pixel_is_set( Fl_Shared_Image* img, int x, int y, int threshold )
{
  guchar* pixel = pb_get_pixel( img,x,y );
  return( pixel[0] > threshold );
}

void print_og( plan_t* plan )
{
  const int sx = plan->size_x;
  const int sy = plan->size_y;

  char c;
  for(int j=0;j<sy;j++)
    {
      for(int i=0;i<sx;i++)
	{	  
	  switch( plan->cells[i+j*sx].occ_state )
	    {
	    case 1: c = 'O'; break;
	    case 0: c = '?'; break;
	    case -1: c = '.'; break;
	    default: c = 'X'; break;
	    }
	  
	  printf( "%c", c );
	}
      puts( "" );
    }
}

void print_cspace( plan_t* plan )
{
  const int sx = plan->size_x;
  const int sy = plan->size_y;

  char c;
  for(int j=0;j<sy;j++)
    {
      for(int i=0;i<sx;i++)
	{	  
	  printf( "%.2f ", plan->cells[i+j*sx].occ_dist  );
	}
      puts( "" );
    }
}
  
plan_t* SetupPlan( double robot_radius, 
		   double safety_dist,
		   double max_radius,
		   double dist_penalty,
		   const char* fname,
		   double widthm,
		   double originx,
		   double originy,
		   int threshold )
{  
  Fl_Shared_Image *img = Fl_Shared_Image::get(fname);
  assert( img );
 
  int sx = img->w();
  int sy = img->h();

  plan_t* plan = NULL;
  if( ! (plan = plan_alloc( robot_radius + safety_dist, 
			    robot_radius + safety_dist,
			    max_radius, 
			    dist_penalty,				 
			    0.5)) )
    {
      puts("failed to allocate plan");
    }
  
  // map geometry
  plan->scale = widthm / sx;
  plan->size_x = sx;
  plan->size_y = sy;
  plan->origin_x = originx;
  plan->origin_y = originy;
  
  // allocate space for map cells
  assert(plan->cells == NULL);
  plan->cells = (plan_cell_t*)
    realloc(plan->cells, 
	    sx * sy * sizeof(plan_cell_t));
  
  assert(plan->cells);
  
  printf( "plan->max_radius %.3f\n", plan->max_radius );

  // Copy over obstacle information from the image data that we read
  for(int j=0;j<sy;j++)
    for(int i=0;i<sx;i++)
      {
	plan_cell_t* cell = &plan->cells[i+j*sx];
	cell->occ_dist = plan->max_radius;

	cell->occ_state = 
	  pb_pixel_is_set( img,i,sy-j-1, threshold) ? -1 : 1;	

	if( cell->occ_state >= 0 )
	  cell->occ_dist = 0;
      }
  
  img->release(); // frees all image resources
  
  plan_init(plan);
  plan_compute_cspace(plan);

  return plan;
}


class PointsVis : public CustomVisualizer 
{
public:
  PointsVis( Model* mod ) : 
	 mod( mod ),
	 pts( NULL ),
	 pt_count( 0 ),
	 CustomVisualizer() 
  { /* nothing to do */ };
  
  virtual void DataVisualize( Camera* cam )
  {
	 if( pt_count < 1 )
		return;
	 
	 glPushMatrix();

	 Gl::pose_inverse_shift( mod->GetGlobalPose() );
	 
	 glColor3f( 0,1,0 );

	 glBegin( GL_POINTS );
	 
	 for( int i=0; i< pt_count; i++ )
		glVertex2f( pts[2*i], pts[2*i+1] );
	 
	 glEnd();	 

	 glPopMatrix();
  }
  
  // rtv - surely a static string member would be easier here?
  //must return a name for visualization (careful not to return stack-memory)	
  
  static const std::string n;
  virtual const std::string& name() { return n; } ; 
  
  void SetPoints( double* pts, int pt_count )
  {
	 this->pts = pts;
	 this->pt_count = pt_count;
  }
				
private:
  Model* mod;
  double* pts;
  int pt_count;
};


const std::string PointsVis::n = "PointsVisName";

class Robot
{
private:
  ModelPosition* pos;
  ModelLaser* laser;
  ModelRanger* ranger;
  ModelFiducial* fiducial;
  ModelBlobfinder* blobfinder;
  ModelGripper* gripper;
  Model *source, *sink;
  int avoidcount, randcount;
  int work_get, work_put;
  bool charger_ahoy;
  double charger_bearing;
  double charger_range;
  double charger_heading;
  nav_mode_t mode;
  bool at_dest;

  plan_t* plan;
  bool plan_done;

  Model* goal;

  GQueue* laser_history;
  double* pts;
  
  PointsVis* vis;
  bool havepath;

public:
  Robot( ModelPosition* pos, 
	 Model* source,
	 Model* sink ) 
    : pos(pos), 
      laser( (ModelLaser*)pos->GetUnusedModelOfType( MODEL_TYPE_LASER )),
      ranger( (ModelRanger*)pos->GetUnusedModelOfType( MODEL_TYPE_RANGER )),
      fiducial( (ModelFiducial*)pos->GetUnusedModelOfType( MODEL_TYPE_FIDUCIAL )),	
      blobfinder( (ModelBlobfinder*)pos->GetUnusedModelOfType( MODEL_TYPE_BLOBFINDER )),
      gripper( (ModelGripper*)pos->GetUnusedModelOfType( MODEL_TYPE_GRIPPER )),
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
      plan( NULL ),
      plan_done( false ),
		goal( NULL ),
		laser_history( g_queue_new() ),
		pts( NULL ),
		vis( new PointsVis( laser ) ),
		havepath( false )
  {
    // need at least these models to get any work done
    // (pos must be good, as we used it in the initialization list)
    assert( laser );
    assert( source );
    assert( sink );
	 
    // PositionUpdate() checks to see if we reached source or sink
    pos->AddUpdateCallback( (stg_model_callback_t)PositionUpdate, this );
    pos->Subscribe();

    // LaserUpdate() controls the robot, by reading from laser and
    // writing to position
    laser->AddUpdateCallback( (stg_model_callback_t)LaserUpdate, this );
    laser->Subscribe();

    fiducial->AddUpdateCallback( (stg_model_callback_t)FiducialUpdate, this );	 	 
    fiducial->Subscribe();
	 
    //gripper->AddUpdateCallback( (stg_model_callback_t)GripperUpdate, this );	 	 
    gripper->Subscribe();
	 
    if( blobfinder ) // optional
      {
	blobfinder->AddUpdateCallback( (stg_model_callback_t)BlobFinderUpdate, this );
	blobfinder->Subscribe();
      }
    
    //planif( !plan )
      plan = SetupPlan( 0.3, // robot radius 
			0.1, // safety dist
			0.6, // max radius 
			1.0, // dist penalty
			"/Users/vaughan/PS/stage/trunk/worlds/bitmaps/cave_compact.png",
			16.0, -8.0, -8.0, 250 );

		laser->AddCustomVisualizer( vis );
  }
  

  void Dock()
  {
    // close the grippers so they can be pushed into the charger
    ModelGripper::config_t gripper_data = gripper->GetConfig();
  
    if( gripper_data.paddles != ModelGripper::PADDLE_CLOSED )
      gripper->CommandClose();
    else  if( gripper_data.lift != ModelGripper::LIFT_UP )
      gripper->CommandUp();  

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
    const stg_meters_t gripper_distance = 0.2;
    const stg_meters_t back_off_distance = 0.3;
    const stg_meters_t back_off_speed = -0.05;

    // back up a bit
    if( charger_range < back_off_distance )
      pos->SetXSpeed( back_off_speed );
    else
      pos->SetXSpeed( 0.0 );
  
    // once we have backed off a bit, open and lower the gripper
    ModelGripper::config_t gripper_data = gripper->GetConfig();
    if( charger_range > gripper_distance )
      {
	if( gripper_data.paddles != ModelGripper::PADDLE_OPEN )
	  gripper->CommandOpen();
	else if( gripper_data.lift != ModelGripper::LIFT_DOWN )
	  gripper->CommandDown();  
      }
    
    // if the gripper is down and open and we're away from the charger, undock is finished
    if( gripper_data.paddles == ModelGripper::PADDLE_OPEN &&
	gripper_data.lift == ModelGripper::LIFT_DOWN &&
	charger_range > back_off_distance )	 
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
  
    // Get the data
    uint32_t sample_count=0;
    stg_laser_sample_t* scan = laser->GetSamples( &sample_count );
    
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
	  minleft = MIN( minleft, scan[i].range );
	else      
	  minright = MIN( minright, scan[i].range );
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
	 Pose pose = pos->GetPose();
	 plan_update_waypoints( plan, pose.x, pose.y );

    if( ObstacleAvoid() )
		{
		  //pos->SetSpeed( 0,0,0 );
		  
		  Pose gpose = goal->GetGlobalPose();		  

		  if( plan_do_global( plan, pose.x, pose.y, gpose.x, gpose.y ) < 0)
			 {
				havepath = false;
			 }		 
		}
	 else
      {
		  if( verbose ) puts( "Cruise" );
		  
		  if( havepath )
			 pos->SetXSpeed( cruisespeed );	  
		  else
			 pos->SetXSpeed( 0 );
		  
		  Pose pose = pos->GetPose();
		  
		  double gx,gy;
		  plan_convert_waypoint( plan, plan->waypoints[1], &gx, &gy );
		  
		  double a_goal = atan2( gy-pose.y, gx-pose.x );
		  
		  // printf( "goal heading %.2f\n", a_goal );
		  
		  // if we are low on juice - find the direction to the recharger instead
		  if( Hungry() )		 
			 { 
				//puts( "hungry - using refuel map" );
				
				// use the refuel map
				//a_goal = dtor( refuel[y][x] );
				
				//if( charger_ahoy ) // I see a charger while hungry!
				  //mode = MODE_DOCK;
			 }
		  
		  double a_error = normalize( a_goal - pose.a );		  
		  pos->SetTurnSpeed(  a_error );
      }  
  }
  

  typedef struct
  {
	 stg_laser_sample_t* scan;
	 Pose pose;
  } scan_record_t;

  // inspect the laser data and decide what to do
  static int LaserUpdate( ModelLaser* laser, Robot* robot )
  {
    //   if( laser->power_pack && laser->power_pack->charging )
    // 	 printf( "model %s power pack @%p is charging\n",
    // 				laser->Token(), laser->power_pack );
    
    uint32_t sample_count=0;
    stg_laser_sample_t* scan = laser->GetSamples( &sample_count );
	 
    if( scan == NULL )
      return 0;

	 scan_record_t* sr = new scan_record_t;
	 sr->pose = laser->GetGlobalPose();
	 
	 // deep copy the samples
	 sr->scan = new stg_laser_sample_t[sample_count];
	 memcpy( sr->scan, scan, sample_count * sizeof(stg_laser_sample_t));
	 
	 // add the copy to the list
	 g_queue_push_tail( robot->laser_history, sr );
	 
	 // take off the oldest scan and free it
	 if( robot->laser_history->length > 10 )
		{
		  scan_record_t* old = (scan_record_t*)g_queue_pop_head( robot->laser_history );
		  assert( old );
		  delete[] old->scan;
		  delete old;
		}
	 	 
    // fill the dynamic map
    robot->pts = (double*)realloc( robot->pts, sizeof(double) * 2*sample_count*robot->laser_history->length );
    
    // project hit points from each scan
    // scan = this->scans;
    
    int scan_points_count = 0;
    
    stg_laser_cfg_t cfg  = laser->GetConfig();
    
	 for( int i = 0; i < robot->laser_history->length; i++ )
		{
		  scan_record_t* scr = (scan_record_t*)g_queue_peek_nth( robot->laser_history, i );
		  stg_laser_sample_t* ascan = scr->scan;
		  Pose apose = scr->pose;
		  
		  double amin = -cfg.fov / 2.0; 
		  double aincr = cfg.fov / sample_count;
		  
		  for( unsigned int j=0; j<sample_count; j++ )
			 {
				if( ascan[j].range >= cfg.range_bounds.max )
				  continue;
				
				double a = amin + j * aincr;
				double cs,sn;
				cs = cos(apose.a+a);
				sn = sin(apose.a+a);
				
				double lx,ly;
				lx = apose.x + ascan[j].range*cs;
				ly = apose.y + ascan[j].range*sn;
				
				//assert(this->scan_points_count*2 < this->scan_points_size);
				robot->pts[2*scan_points_count  ] = lx;
				robot->pts[2*scan_points_count+1] = ly;
				scan_points_count++;
			 }
		}

	 //printf("setting %d hit points\n", scan_points_count);
	 plan_set_obstacles( robot->plan, robot->pts, scan_points_count);
	 
	 robot->vis->SetPoints( robot->pts, scan_points_count );

	 
 // 	 Waypoint* wps = new Waypoint[scan_points_count];
//      for( int j=0; j<scan_points_count; j++ )
//        {
//  		  wps[j].pose.x = pts[2*j];
//  		  wps[j].pose.y = pts[2*j+1];
//  		  wps[j].pose.z = 0;
//  		  wps[j].pose.a = 0;
//  		  wps[j].color = stg_color_pack( 0,1,0,0 );
// 		 }
	  
// 	  Waypoint* oldp = ((ModelPosition*)robot->pos->GetWorld()->GetModel( "dummy" ))->SetWaypoints( wps, scan_points_count );
// 	  if( oldp )
// 		 delete[] oldp;
	  
//	 delete[] pts;


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
  
    plan_t* plan = robot->plan;

    //printf( "Pose: [%.2f %.2f %.2f %.2f]\n",
    //  pose.x, pose.y, pose.z, pose.a );
    
    //printf( "planning from (%.2f %.2f) to (%.2f %.2f)\n",
    //    pose.x, pose.y, -4.0, 0.0 );

	 Model* nextgoal = NULL;
	 
	 if( pos->GetFlagCount() )
		nextgoal = robot->sink;
	 else
		nextgoal = robot->source;
	 
	 if( (robot->goal != nextgoal) )//|| !robot->plan_done)
		{
		  robot->goal = nextgoal;		  
		  
		  Pose gpose = robot->goal->GetGlobalPose();		  
 		  if( plan_do_global( plan, pose.x, pose.y, gpose.x, gpose.y ) < 0)
 			 {
 				puts( "no global path" );	  
 			 }
		}
		  
    //printf( "PATH_COUNT %d\n", plan->path_count );
	 
	 
// 	 double vx, va;
// 	 int rotatedir;
	 
// 	 plan_compute_diffdrive_cmds( plan,
// 											&vx, &va, 
// 											&rotatedir,
// 											pose.x, pose.y, pose.a, // lx, ly, la
//  											-7, -7, 0, // gx, gy, ga
// 											0.1, 0.1, // goal_d, goal_a
// 											1.0, 5.0, // maxd, dweight
// 											0.0, 0.2, // tvmin, tvmax
// 											0.0, 2, // avmin, avmax
// 											0, 1 ); // amin, amax

// // 	 double dx, da;
// // 	 plan_get_carrot( plan, &dx, &da,
// // 							pose.x, pose.y,
// // 							2.0, 10 );

// 	 pos->SetSpeed( vx, 0, va );

	 
// 	 if( plan_do_local( plan, pose.x, pose.y, 1.0 ) < 0 )
// 		{
// 		   puts( "no local path" );	  
		  
// 		  Pose gpose = robot->goal->GetGlobalPose();		  

// 		  if( plan_do_global( plan, pose.x, pose.y, gpose.x, gpose.y ) < 0)
// 			 {
// 				robot->havepath = false;
// 				puts( "no global path" );	  
// 			 }
// 		  else
// 			 robot->havepath = true;
// 		}
// 	 else
		robot->havepath = true;
	 
    plan_update_waypoints( plan, pose.x, pose.y );
	 
    //printf( "WAYPOINT_COUNT %d\n", plan->waypoint_count );
	 	 
    Waypoint* wps = NULL;    
    stg_color_t col = pos->GetColor();
	 
    if( plan->waypoint_count > 0 )
      {
		  wps = new Waypoint[plan->waypoint_count];
		  for( int i=0; i<plan->waypoint_count; i++ )
			 {
				//plan_convert_waypoint( plan, plan->path[i], 
				//		   &wps[i].pose.x,
				//		   &wps[i].pose.y );
				
				plan_convert_waypoint( plan, plan->waypoints[i], 
											  &wps[i].pose.x,
											  &wps[i].pose.y );
				
				wps[i].color = col;
			 }
      }

	 
	 Waypoint* old = pos->SetWaypoints( wps, plan->waypoint_count );	
	 if( old )
		delete[] old;
	 
    
    //pose.z += 0.0001;
    //robot->pos->SetPose( pose );
  
	 Pose psrc = robot->source->GetPose();
	 
    if( pos->GetFlagCount() < payload && 
		  hypot(psrc.x-pose.x, psrc.y-pose.y ) < 2.0 )
      {
		  if( ++robot->work_get > workduration )
			 {
				// protect source from concurrent access
				robot->source->Lock();
				
				// transfer a chunk from source to robot
				pos->PushFlag( robot->source->PopFlag() );
				robot->source->Unlock();
				
				robot->work_get = 0;	
			 }	  
      }
  
    robot->at_dest = false;

	 Pose psink = robot->sink->GetPose();

    if( hypot( psink.x-pose.x, psink.y-pose.y ) < 1.0 )
      {
	robot->at_dest = true;

	robot->gripper->CommandOpen();

	if( ++robot->work_put > workduration )
	  {
	    // protect sink from concurrent access
	    robot->sink->Lock();

	    //puts( "dropping" );
	    // transfer a chunk between robot and goal
	    robot->sink->PushFlag( pos->PopFlag() );
	    robot->sink->Unlock();

	    robot->work_put = 0;

	  }
      }
  
  
    return 0; // run again
  }



  static int FiducialUpdate( ModelFiducial* mod, Robot* robot )
  {    
    robot->charger_ahoy = false;
  
    for( unsigned int i = 0; i < mod->fiducial_count; i++ )
      {
	stg_fiducial_t* f = &mod->fiducials[i];
		
	//printf( "fiducial %d is %d at %.2f m %.2f radians\n",
	//	  i, f->id, f->range, f->bearing );		
		
	if( f->id == 2 ) // I see a charging station
	  {
	    // record that I've seen it and where it is
	    robot->charger_ahoy = true;
	    robot->charger_bearing = f->bearing;
	    robot->charger_range = f->range;
	    robot->charger_heading = f->geom.a;
			 
	    //printf( "charger at %.2f radians\n", robot->charger_bearing );
	    break;
	  }
      }						  
 
    return 0; // run again
  }

  static int BlobFinderUpdate( ModelBlobfinder* blobmod, Robot* robot )
  {  
    unsigned int blob_count = 0;
    stg_blobfinder_blob_t* blobs = blobmod->GetBlobs( &blob_count );

    if( blobs && (blob_count>0) )
      {
	printf( "%s sees %u blobs\n", blobmod->Token(), blob_count );
      }

    return 0;
  }

  static int GripperUpdate( ModelGripper* grip, Robot* robot )
  {
    ModelGripper::config_t gdata = grip->GetConfig();
	 
    printf( "BREAKBEAMS %s %s\n",
	    gdata.beam[0] ? gdata.beam[0]->Token() : "<null>", 
	    gdata.beam[1] ? gdata.beam[1]->Token() : "<null>" );

    printf( "CONTACTS %s %s\n",
	    gdata.contact[0] ? gdata.contact[0]->Token() : "<null>", 
	    gdata.contact[1] ? gdata.contact[1]->Token() : "<null>");


    return 0;
  }


};


      //plan_t* Robot::plan = NULL;

// Stage calls this when the model starts up
extern "C" int Init( Model* mod )
{  
  Robot* robot = new Robot( (ModelPosition*)mod,
			    mod->GetWorld()->GetModel( "source" ),
			    mod->GetWorld()->GetModel( "sink" ) );
    
  return 0; //ok
}





