/** @defgroup model

    The basic model simulates an object with basic properties; position,
    size, velocity, color, visibility to various sensors, etc. The basic
    model also has a body made up of a list of lines. Internally, the
    basic model is used as the base class for all other model types. You
    can use the basic model to simulate environmental objects.

    API: Stg::Model 

    <h2>Worldfile properties</h2>

    @par Summary and default values

    @verbatim
    model
    (
    pose [ 0.0 0.0 0.0 0.0 ]
    size [ 0.1 0.1 0.1 ]
    origin [ 0.0 0.0 0.0 0.0 ]
    velocity [ 0.0 0.0 0.0 0.0 ]

    color "red"
    color_rgba [ 0.0 0.0 0.0 1.0 ]
    bitmap ""
    ctrl ""

    # determine how the model appears in various sensors
    fiducial_return 0
    fiducial_key 0
    obstacle_return 1
    ranger_return 1
    blob_return 1
    laser_return LaserVisible
    gripper_return 0
    gravity_return 0
    sticky_return 0

    # GUI properties
    gui_nose 0
    gui_grid 0
    gui_outline 1
    gui_move 0 (1 if the model has no parents);

    boundary 0
    mass 10.0
    map_resolution 0.1
    say ""
    alwayson 0
    )
    @endverbatim

    @par Details
 
    - pose [ x:<float> y:<float> z:<float> heading:<float> ] \n
    specify the pose of the model in its parent's coordinate system
    - size [ x:<float> y:<float> z:<float> ]\n
    specify the size of the model in each dimension
    - origin [ x:<float> y:<float> z:<float> heading:<float> ]\n
    specify the position of the object's center, relative to its pose
    - velocity [ x:<float> y:<float> z:<float> heading:<float> omega:<float> ]\n
    Specify the initial velocity of the model. Note that if the model hits an obstacle, its velocity will be set to zero.
 
    - color <string>\n
    specify the color of the object using a color name from the X11 database (rgb.txt)
    - bitmap filename:<string>\n
    Draw the model by interpreting the lines in a bitmap (bmp, jpeg, gif, png supported). The file is opened and parsed into a set of lines.  The lines are scaled to fit inside the rectangle defined by the model's current size.
    - ctrl <string>\n
    specify the controller module for the model
 
    - fiducial_return fiducial_id:<int>\n
    if non-zero, this model is detected by fiducialfinder sensors. The value is used as the fiducial ID.
    - fiducial_key <int>
    models are only detected by fiducialfinders if the fiducial_key values of model and fiducialfinder match. This allows you to have several independent types of fiducial in the same environment, each type only showing up in fiducialfinders that are "tuned" for it.
    - obstacle_return <int>\n
    if 1, this model can collide with other models that have this property set 
    - ranger_return <int>\n
    if 1, this model can be detected by ranger sensors
    - blob_return <int>\n
    if 1, this model can be detected in the blob_finder (depending on its color)
    - laser_return <int>\n
    if 0, this model is not detected by laser sensors. if 1, the model shows up in a laser sensor with normal (0) reflectance. If 2, it shows up with high (1) reflectance.
    - gripper_return <int>\n
    iff 1, this model can be gripped by a gripper and can be pushed around by collisions with anything that has a non-zero obstacle_return.
 
    - gui_nose <int>\n
    if 1, draw a nose on the model showing its heading (positive X axis)
    - gui_grid <int>\n
    if 1, draw a scaling grid over the model
    - gui_outline <int>\n
    if 1, draw a bounding box around the model, indicating its size
    - gui_move <int>\n
    if 1, the model can be moved by the mouse in the GUI window

    - friction <float>\n
    Determines the proportion of velocity lost per second. For example, 0.1 would mean that the object would lose 10% of its speed due to friction per second. A value of zero (the default) means this model can not be pushed around (infinite friction).
*/

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <map>

//#define DEBUG 0
#include "stage.hh"
#include "worldfile.hh"
using namespace Stg;

// static members
uint32_t Model::count = 0;
std::map<stg_id_t,Model*> Model::modelsbyid;

std::map<std::string, creator_t> Model::name_map;

void Size::Load( Worldfile* wf, int section, const char* keyword )
{
  x = wf->ReadTupleLength( section, keyword, 0, x );
  y = wf->ReadTupleLength( section, keyword, 1, y );
  z = wf->ReadTupleLength( section, keyword, 2, z );
}

void Size::Save( Worldfile* wf, int section, const char* keyword )
{
  wf->WriteTupleLength( section, keyword, 0, x );
  wf->WriteTupleLength( section, keyword, 1, y );
  wf->WriteTupleLength( section, keyword, 2, z );
}

void Pose::Load( Worldfile* wf, int section, const char* keyword )
{
  x = wf->ReadTupleLength( section, keyword, 0, x );
  y = wf->ReadTupleLength( section, keyword, 1, y );
  z = wf->ReadTupleLength( section, keyword, 2, z );
  a = wf->ReadTupleAngle(  section, keyword, 3, a );
}

void Pose::Save( Worldfile* wf, int section, const char* keyword )
{
  wf->WriteTupleLength( section, keyword, 0, x );
  wf->WriteTupleLength( section, keyword, 1, y );
  wf->WriteTupleLength( section, keyword, 2, z );
  wf->WriteTupleAngle(  section, keyword, 3, a );
}

Visibility::Visibility() : 
  blob_return( true ),
  fiducial_key( 0 ),
  fiducial_return( 0 ),
  gripper_return( false ),
  laser_return( LaserVisible ),
  obstacle_return( true ),
  ranger_return( true )
{ /* nothing do do */ }

//static const members
static const double DEFAULT_FRICTION = 0.0;

void Visibility::Load( Worldfile* wf, int wf_entity )
{
  blob_return = wf->ReadInt( wf_entity, "blob_return", blob_return);    
  fiducial_key = wf->ReadInt( wf_entity, "fiducial_key", fiducial_key);
  fiducial_return = wf->ReadInt( wf_entity, "fiducial_return", fiducial_return);
  gripper_return = wf->ReadInt( wf_entity, "gripper_return", gripper_return);    
  laser_return = (stg_laser_return_t)wf->ReadInt( wf_entity, "laser_return", laser_return);    
  obstacle_return = wf->ReadInt( wf_entity, "obstacle_return", obstacle_return);    
  ranger_return = wf->ReadInt( wf_entity, "ranger_return", ranger_return);    
  gravity_return = wf->ReadInt( wf_entity, "gravity_return", gravity_return);
  sticky_return = wf->ReadInt( wf_entity, "sticky_return", sticky_return);
}    

GuiState:: GuiState() :
  grid( false ),
  move( false ),  
  nose( false ),
  outline( false )
{ /* nothing to do */}

void GuiState::Load( Worldfile* wf, int wf_entity )
{
  nose = wf->ReadInt( wf_entity, "gui_nose", nose);    
  grid = wf->ReadInt( wf_entity, "gui_grid", grid);    
  outline = wf->ReadInt( wf_entity, "gui_outline", outline);    
  move = wf->ReadInt( wf_entity, "gui_move", move );    
}    

// constructor
Model::Model( World* world,
				  Model* parent,
				  const std::string& type ) :
  Ancestor(), 	 
  access_mutex(),
  alwayson(false),
  blockgroup(),
  blocks_dl(0),
  boundary(false),
  callbacks(),
  color( 1,0,0 ), // red
  data_fresh(false),
  disabled(false),
	 cv_list(),
    flag_list(),
    geom(),
    has_default_block( true ),
    id( Model::count++ ),
    interval((stg_usec_t)1e5), // 100msec
    interval_energy((stg_usec_t)1e5), // 100msec
    interval_pose((stg_usec_t)1e5), // 100msec
    last_update(0),
	 log_state(false),
    map_resolution(0.1),
    mass(0),
    parent(parent),
    pose(),
	 power_pack( NULL ),
	 pps_charging(),
    props(),
	 rastervis(),
    rebuild_displaylist(true),
    say_string(NULL),
    stall(false),	 
    subs(0),
    thread_safe( false ),
    trail(),
	 trail_length(50),
	 trail_interval(5),
    type(type),	
	 event_queue_num( -1 ),
    used(false),
    velocity(),
    watts(0.0),
	 watts_give(0.0),
	 watts_take(0.0),	 
    wf(NULL),
    wf_entity(0),
    world(world),
	 world_gui( dynamic_cast<WorldGui*>( world ) )
{
  //assert( modelsbyid );
  assert( world );
  
  PRINT_DEBUG3( "Constructing model world: %s parent: %s type: %d ",
		world->Token(), 
		parent ? parent->Token() : "(null)",
		type );
  
  modelsbyid[id] = this;
  
  // Adding this model to its ancestor also gives this model a
  // sensible default name
  if ( parent ) 
    parent->AddChild( this );
  else
    {
      world->AddChild( this );
      // top level models are draggable in the GUI
      gui.move = true;
    }

  world->AddModel( this );
  
  this->friction = DEFAULT_FRICTION;
    
  // now we can add the basic square shape
  AddBlockRect( -0.5, -0.5, 1.0, 1.0, 1.0 );

  AddVisualizer( &rastervis, false );

  PRINT_DEBUG2( "finished model %s @ %p", this->token, this );
}

Model::~Model( void )
{
  UnMap(); // remove from the raytrace bitmap

  // children are removed in ancestor class
  
  // remove myself from my parent's child list, or the world's child
  // list if I have no parent
  ModelPtrVec& vec  = parent ? parent->children : world->children;
  vec.erase( std::remove( vec.begin(), vec.end(), this ));

  modelsbyid.erase(id);

  world->RemoveModel( this );
}

// this should be called after all models have loaded from the
// worldfile - it's a chance to do any setup now that all models are
// in existence
void Model::Init()
{
  // init is called after the model is loaded
  blockgroup.CalcSize();

  UnMap(); // remove any old cruft rendered during startup
  Map();

  if( FindPowerPack() )
	 world->Enqueue( 0, World::Event::ENERGY, interval_energy, this );
  
  // find the queue for update events: zero if thread safe, else we
  // ask the world to assign us to a queue  
  if( event_queue_num < 1 ) 		
	 event_queue_num = thread_safe ? world->GetEventQueue( this ) : 0;
  
  CallCallbacks( &hooks.init );

  if( alwayson )
	 Subscribe();
}  

void Model::InitRecursive()
{
  // init children first
  FOR_EACH( it, children )
	 (*it)->InitRecursive();

  Init();
}  

void Model::AddFlag( Flag* flag )
{
  if( flag )
	 {
		flag_list.push_back( flag );		
		CallCallbacks( &hooks.flag_incr );		
	 }
}

void Model::RemoveFlag( Flag* flag )
{
  if( flag )
	 {
		flag_list.erase( remove( flag_list.begin(), flag_list.end(), flag ));		
		CallCallbacks( &hooks.flag_decr );
	 }
}

void Model::PushFlag( Flag* flag )
{
  if( flag )
	 {
		flag_list.push_front( flag);
		CallCallbacks( &hooks.flag_incr );
	 }
}

Flag* Model::PopFlag()
{
  if( flag_list.size() == 0 )
    return NULL;
  
  Flag* flag = flag_list.front();
  flag_list.pop_front();
  
  CallCallbacks( &hooks.flag_decr );
  
  return flag;
}

void Model::ClearBlocks( void )
{
  UnMap();
  blockgroup.Clear();
  //no need to Map() -  we have no blocks
  NeedRedraw();
}

void Model::LoadBlock( Worldfile* wf, int entity )
{
  if( has_default_block )
    {
      blockgroup.Clear(); 
      has_default_block = false;
    }
  
  blockgroup.LoadBlock( this, wf, entity );  
}


Block* Model::AddBlockRect( stg_meters_t x, 
									 stg_meters_t y, 
									 stg_meters_t dx, 
									 stg_meters_t dy,
									 stg_meters_t dz )
{  
  UnMap();

  stg_point_t pts[4];
  pts[0].x = x;
  pts[0].y = y;
  pts[1].x = x + dx;
  pts[1].y = y;
  pts[2].x = x + dx;
  pts[2].y = y + dy;
  pts[3].x = x;
  pts[3].y = y + dy;
  
  Block* newblock =  new Block( this,
									 pts, 4, 
									 0, dz, 
									 color,
									 true );

  blockgroup.AppendBlock( newblock );

  return newblock;
}


stg_raytrace_result_t Model::Raytrace( const Pose &pose,
				       const stg_meters_t range, 
				       const stg_ray_test_func_t func,
				       const void* arg,
				       const bool ztest )
{
  return world->Raytrace( LocalToGlobal(pose),
			  range,
			  func,
			  this,
			  arg,
			  ztest );
}

stg_raytrace_result_t Model::Raytrace( const stg_radians_t bearing,
				       const stg_meters_t range, 
				       const stg_ray_test_func_t func,
				       const void* arg,
				       const bool ztest )
{
  Pose raystart;
  bzero( &raystart, sizeof(raystart));
  raystart.a = bearing;

  return world->Raytrace( LocalToGlobal(raystart),
			  range,
			  func,
			  this,
			  arg,
			  ztest );
}


void Model::Raytrace( const stg_radians_t bearing,
		      const stg_meters_t range, 
		      const stg_radians_t fov,
		      const stg_ray_test_func_t func,
		      const void* arg,
		      stg_raytrace_result_t* samples,
		      const uint32_t sample_count,
		      const bool ztest )
{
  Pose raystart;
  bzero( &raystart, sizeof(raystart));
  raystart.a = bearing;

  world->Raytrace( LocalToGlobal(raystart),
		   range,		   
		   fov,
		   func,
		   this,
		   arg,
		   samples,
		   sample_count,
		   ztest );
}

// convert a global pose into the model's local coordinate system
Pose Model::GlobalToLocal( const Pose& pose ) const
{
  // get model's global pose
  Pose org = GetGlobalPose();
  
  // compute global pose in local coords
  double sx =  (pose.x - org.x) * cos(org.a) + (pose.y - org.y) * sin(org.a);
  double sy = -(pose.x - org.x) * sin(org.a) + (pose.y - org.y) * cos(org.a);
  double sz = pose.z - org.z;
  double sa = pose.a - org.a;
  
  org.x = sx;
  org.y = sy;
  org.z = sz;
  org.a = sa;

  return org;
}


void Model::Say( const char* str )
{
  if( say_string )
    free( say_string );
  say_string = strdup( str );
}

// returns true iff model [testmod] is an antecedent of this model
bool Model::IsAntecedent( const Model* testmod ) const
{
  if( parent == NULL )
    return false;
  
  if( parent == testmod )
    return true;
  
  return parent->IsAntecedent( testmod );
}

// returns true iff model [testmod] is a descendent of this model
bool Model::IsDescendent( const Model* testmod ) const
{
  if( this == testmod )
	 return true;
  
  FOR_EACH( it, children )
	 if( (*it)->IsDescendent( testmod ) )
		return true;
  
  // neither mod nor a child of this matches testmod
  return false;
}

bool Model::IsRelated( const Model* that ) const
{
  // is it me?
  if( this == that )
 	 return true;
  
  // wind up to top-level object
  Model* candidate = (Model*)this;
  while( candidate->parent )
	 candidate = candidate->parent;
  
  // and recurse down the tree    
  return candidate->IsDescendent( that );
}

stg_point_t Model::LocalToGlobal( const stg_point_t& pt) const
 {  
   Pose gpose = LocalToGlobal( Pose( pt.x, pt.y, 0, 0 ) );
   return stg_point_t( gpose.x, gpose.y );
 }

void Model::MapWithChildren()
{
  UnMap();
  Map();

  // recursive call for all the model's children
  FOR_EACH( it, children )
    (*it)->MapWithChildren();
}

void Model::MapFromRoot()
{
	Root()->MapWithChildren();
}

void Model::UnMapWithChildren()
{
  UnMap();

  // recursive call for all the model's children
  FOR_EACH( it, children )
    (*it)->UnMapWithChildren();
}

void Model::UnMapFromRoot()
{
	Root()->UnMapWithChildren();
}

void Model::Map()
{
  //PRINT_DEBUG1( "%s.Map()", token );

  // render all blocks in the group at my global pose and size
  blockgroup.Map();
} 


void Model::Subscribe( void )
{
  subs++;
  world->total_subs++;
  world->dirty = true; // need redraw
  
  //printf( "subscribe to %s %d\n", token, subs );
  
  // if this is the first sub, call startup
  if( subs == 1 )
    Startup();
}

void Model::Unsubscribe( void )
{
  subs--;
  world->total_subs--;
  world->dirty = true; // need redraw

  //printf( "unsubscribe from %s %d\n", token, subs );

  // if this is the last sub, call shutdown
  if( subs == 0 )
    this->Shutdown();
}


void pose_invert( Pose* pose )
{
  pose->x = -pose->x;
  pose->y = -pose->y;
  pose->a = pose->a;
}

void Model::Print( char* prefix ) const
{
  if( prefix )
    printf( "%s model ", prefix );
  else
    printf( "Model ");

  printf( "%s:%s\n", 
	  //			id, 
	  world->Token(), 
	  token );

  FOR_EACH( it, children )
    (*it)->Print( prefix );
}

const char* Model::PrintWithPose() const
{
  Pose gpose = GetGlobalPose();

  static char txt[256];
  snprintf(txt, sizeof(txt), "%s @ [%.2f,%.2f,%.2f,%.2f]",  
	   token, 
	   gpose.x, gpose.y, gpose.z, gpose.a  );

  return txt;
}

void Model::Startup( void )
{
  //printf( "Startup model %s\n", this->token );  
  //printf( "model %s using queue %d\n", token, event_queue_num );
  
  // put my first events in the world's queue
  world->Enqueue( event_queue_num, World::Event::UPDATE, interval, this );
  world->Enqueue( 0, World::Event::POSE, interval_pose, this );
  
  CallCallbacks( &hooks.startup );
}

void Model::Shutdown( void )
{
  //printf( "Shutdown model %s\n", this->token );
  CallCallbacks( &hooks.shutdown );

  // allows data visualizations to be cleared.
  NeedRedraw();
}


void Model::Update( void )
{ 
  CallCallbacks( &hooks.update );

  last_update = world->sim_time;
  
  if( subs > 0 )
	 world->Enqueue( event_queue_num, World::Event::UPDATE, interval, this );
}


stg_meters_t Model::ModelHeight() const
{	
  stg_meters_t m_child = 0; //max size of any child
  FOR_EACH( it, children )
    {
      stg_meters_t tmp_h = (*it)->ModelHeight();
      if( tmp_h > m_child )
		  m_child = tmp_h;
    }
	
  //height of model + max( child height )
  return geom.size.z + m_child;
}

void Model::AddToPose( double dx, double dy, double dz, double da )
{
  Pose pose = this->GetPose();
  pose.x += dx;
  pose.y += dy;
  pose.z += dz;
  pose.a += da;
  
  this->SetPose( pose );
}

void Model::AddToPose( const Pose& pose )
{
  this->AddToPose( pose.x, pose.y, pose.z, pose.a );
}

void Model::PlaceInFreeSpace( stg_meters_t xmin, stg_meters_t xmax, 
			      stg_meters_t ymin, stg_meters_t ymax )
{
  while( TestCollisionTree() )
    SetPose( Pose::Random( xmin,xmax, ymin, ymax ));		
}

void Model::AppendTouchingModels( ModelPtrSet& touchers )
{
  blockgroup.AppendTouchingModels( touchers );
}

Model* Model::TestCollision()
{
  //printf( "mod %s test collision...\n", token );  
  return( blockgroup.TestCollision() );
}

Model* Model::TestCollisionTree()
{  
  Model* hitmod = TestCollision();
  
  if( hitmod == NULL ) 	 
	 FOR_EACH( it, children )
      { 
		  hitmod = (*it)->TestCollisionTree();
		  if( hitmod )
			 break;
      }
  
  //printf( "mod %s test collision done.\n", token );
  return hitmod;  
}  

void Model::UpdateCharge()
{  
  PowerPack* mypp = FindPowerPack();
  assert( mypp );
  
  if( watts > 0 ) // dissipation rate
	 {
		// consume  energy stored in the power pack
		stg_joules_t consumed =  watts * (interval_energy * 1e-6); 
		mypp->Dissipate( consumed, GetGlobalPose() );      
	 }  
  
  if( watts_give > 0 ) // transmission to other powerpacks max rate
	 {  
		// detach charger from all the packs charged last time
		FOR_EACH( it, pps_charging )
		  (*it)->ChargeStop();
		pps_charging.clear();
		
		// run through and update all appropriate touchers
		ModelPtrSet touchers;
		AppendTouchingModels( touchers );
		
		FOR_EACH( it, touchers )
		  {
			 Model* toucher = (*it);
			 PowerPack* hispp =toucher->FindPowerPack();		
			 
			 if( hispp && toucher->watts_take > 0.0) 
				{		
				  //printf( "   toucher %s can take up to %.2f wats\n", 
				  //		toucher->Token(), toucher->watts_take );
				  
				  stg_watts_t rate = std::min( watts_give, toucher->watts_take );
				  stg_joules_t amount =  rate * interval_energy * 1e-6;
				  
				  //printf ( "moving %.2f joules from %s to %s\n",
				  //		 amount, token, toucher->token );
				  
				  // set his charging flag
				  hispp->ChargeStart();
				  
				  // move some joules from me to him
				  mypp->TransferTo( hispp, amount );
				  
				  // remember who we are charging so we can detatch next time
				  pps_charging.push_front( hispp );
				}
		  }
	 }
  
  // set up the next event
  if( subs > 0 ) // TODO XX ? 
  world->Enqueue( 0, World::Event::ENERGY, interval_energy, this );
}

void Model::CommitTestedPose()
{
  FOR_EACH( it, children )
    (*it)->CommitTestedPose();
  
  blockgroup.SwitchToTestedCells();
}
  
Model* Model::ConditionalMove( const Pose& newpose )
{ 
  assert( newpose.a >= -M_PI );
  assert( newpose.a <=  M_PI );

  Pose startpose( pose );
  pose = newpose; // do the move provisionally - we might undo it below
     
  Model* hitmod( TestCollisionTree() );
 
  if( hitmod )
    pose = startpose; // move failed - put me back where I started
  else
    {
      CommitTestedPose(); // shift anyrecursively commit to blocks to the new pose 
      world->dirty = true; // need redraw
    }
  
  return hitmod;
}


void Model::UpdatePose( void )
{
  if( disabled )
    return;
  
  // convert usec to sec
  double interval( (double)interval_pose / 1e6 );
  
  // find the change of pose due to our velocity vector
  Pose p( velocity.x * interval,
			 velocity.y * interval,
			 velocity.z * interval,
			 normalize( velocity.a * interval ));
  
  // attempts to move to the new pose. If the move fails because we'd
  // hit another model, that model is returned.	
	// ConditionalMove() returns a pointer to the model we hit, or
	// NULL. We use this as a boolean for SetStall()
  SetStall( ConditionalMove( pose + p ) );
  
  if( trail_length > 0 && world->updates % trail_interval == 0 )
	 {
		trail.push_back( TrailItem( world->sim_time, GetGlobalPose(), color ) ); 
		
		if( trail.size() > trail_length )
		  trail.pop_front();
	 }	            

  //if( ! velocity.IsZero() )

  if( subs > 0 )// TODO XX ? 
	 world->Enqueue( 0, World::Event::POSE, interval_pose, this );
}

Model* Model::GetUnsubscribedModelOfType( const std::string& type ) const
{  
  if( (this->type == type) && (this->subs == 0) )
    return const_cast<Model*> (this); // discard const

  // this model is no use. try children recursively
  FOR_EACH( it, children )
	 {
		Model* found = (*it)->GetUnsubscribedModelOfType( type );
		if( found )
		  return found;
    }
  
  // nothing matching below this model
  return NULL;
}

void Model::NeedRedraw( void )
{
  this->rebuild_displaylist = true;

  if( parent )
    parent->NeedRedraw();
  else
    world->NeedRedraw();
}

Model* Model::GetUnusedModelOfType( const std::string& type )
{
  //printf( "searching for type %d in model %s type %d\n", type, token, this->type );
  
  if( (this->type == type) && (!this->used ) )
    {
      this->used = true;
      return this;
    }

  // this model is no use. try children recursively
  FOR_EACH( it, children )
    {
      Model* found = (*it)->GetUnusedModelOfType( type );
      if( found )
		  return found;
    }
  
  // nothing matching below this model
  if( ! parent )  PRINT_WARN1( "Request for unused model of type %s failed", type.c_str() );
  return NULL;
}

stg_kg_t Model::GetTotalMass()
{
  stg_kg_t sum = mass;
  
  FOR_EACH( it, children )
	 sum += (*it)->GetTotalMass();

  return sum;
}

stg_kg_t Model::GetMassOfChildren()
{
  return( GetTotalMass() - mass);
}

Model* Model::GetModel( const char* modelname ) const
{
  // construct the full model name and look it up
  char* buf = new char[TOKEN_MAX];
  snprintf( buf, TOKEN_MAX, "%s.%s", this->token, modelname );

  Model* mod = world->GetModel( buf );

  if( mod == NULL )
    PRINT_WARN1( "Model %s not found", buf );

  delete[] buf;

  return mod;
}

void Model::UnMap()
{
  blockgroup.UnMap();
}

void Model::BecomeParentOf( Model* child )
{
  if( child->parent )
	 child->parent->RemoveChild( child );
  else
	 world->RemoveChild( child );
  
  child->parent = this;
  
  this->AddChild( child );
  
  world->dirty = true; 
}

PowerPack* Model::FindPowerPack() const
{
  if( power_pack )
	 return power_pack;

  if( parent )
	 return parent->FindPowerPack();

  return NULL;
}

void Model::RegisterOption( Option* opt )
{ 
  //drawOptions.push_back( opt ); 
  
  //if( world->IsGUI() )
  world->RegisterOption( opt );
}


void Model::Rasterize( uint8_t* data, 
		       unsigned int width, 
		       unsigned int height, 
		       stg_meters_t cellwidth,
		       stg_meters_t cellheight )
{
  rastervis.ClearPts();
  blockgroup.Rasterize( data, width, height, cellwidth, cellheight );
  rastervis.SetData( data, width, height, cellwidth, cellheight );
}

void Model::SetFriction( double friction )
{
  this->friction = friction;
  CallCallbacks( &this->friction );
}

//***************************************************************
// Raster data visualizer
//
Model::RasterVis::RasterVis() 
  : Visualizer( "Rasterization", "raster_vis" ),
	 data(NULL),
	 width(0),
	 height(0),
	 cellwidth(0),
	 cellheight(0),
	 pts()
{
}

void Model::RasterVis::Visualize( Model* mod, Camera* cam ) 
{
  if( data == NULL )
	 return;

  // go into world coordinates  
  glPushMatrix();
  mod->PushColor( 1,0,0,0.5 );

  Gl::pose_inverse_shift( mod->GetGlobalPose() );

  
  if( pts.size() > 0 )
	 {
		glPushMatrix();
		Size sz = mod->blockgroup.GetSize();
		//glTranslatef( -mod->geom.size.x / 2.0, -mod->geom.size.y/2.0, 0 );
		//glScalef( mod->geom.size.x / sz.x, mod->geom.size.y / sz.y, 1 );
		
		// now we're in world meters coordinates
		glPointSize( 4 );
		glBegin( GL_POINTS );
		
		FOR_EACH( it, pts )
		  {
			 stg_point_t& pt = *it;
			 glVertex2f( pt.x, pt.y );
			 
			 char buf[128];
			 snprintf( buf, 127, "[%.2f x %.2f]", pt.x, pt.y );
			 Gl::draw_string( pt.x, pt.y, 0, buf );		  
		  }
		glEnd();
		
		mod->PopColor();
		
		glPopMatrix();
	 }

  // go into bitmap pixel coords
  glTranslatef( -mod->geom.size.x / 2.0, -mod->geom.size.y/2.0, 0 );
  //glScalef( mod->geom.size.x / width, mod->geom.size.y / height, 1 );

  glScalef( cellwidth, cellheight, 1 );

  mod->PushColor( 0,0,0,0.5 );
  glPolygonMode( GL_FRONT, GL_FILL );
  for( unsigned int y=0; y<height; y++ )
	 for( unsigned int x=0; x<width; x++ )
		{
		  // printf( "[%u %u] ", x, y );
		  if( data[ x + y*width ] )
			 glRectf( x, y, x+1, y+1 );
		}

  glTranslatef( 0,0,0.01 );

  mod->PushColor( 0,0,0,1 );
  glPolygonMode( GL_FRONT, GL_LINE );
  for( unsigned int y=0; y<height; y++ )
	 for( unsigned int x=0; x<width; x++ )
		{
		  if( data[ x + y*width ] )
			 glRectf( x, y, x+1, y+1 );
		  
// 		  char buf[128];
// 		  snprintf( buf, 127, "[%u x %u]", x, y );
// 		  Gl::draw_string( x, y, 0, buf );		  
		}

	
  glPolygonMode( GL_FRONT, GL_FILL );
	
  mod->PopColor();
  mod->PopColor();

  mod->PushColor( 0,0,0,1 );
  char buf[128];
  snprintf( buf, 127, "[%u x %u]", width, height );
  glTranslatef( 0,0,0.01 );
  Gl::draw_string( 1, height-1, 0, buf );
  
  mod->PopColor();

  glPopMatrix();
}

void Model::RasterVis::SetData( uint8_t* data, 
										  unsigned int width, 
										  unsigned int height,
										  stg_meters_t cellwidth, 
										  stg_meters_t cellheight )
{
  // copy the raster for test visualization
  if( this->data ) 
	 delete[] this->data;  
  size_t len = sizeof(uint8_t) * width * height;
  //printf( "allocating %lu bytes\n", len );
  this->data = new uint8_t[len];
  memcpy( this->data, data, len );
  this->width = width;
  this->height = height;
  this->cellwidth = cellwidth;
  this->cellheight = cellheight;
}


void Model::RasterVis::AddPoint( stg_meters_t x, stg_meters_t y )
{
  pts.push_back( stg_point_t( x, y ) );
}

void Model::RasterVis::ClearPts()
{
  pts.clear();
}

