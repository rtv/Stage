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

    # GUI properties
    gui_nose 0
    gui_grid 0
    gui_outline 1
    gui_movemask <0 if top level or (STG_MOVE_TRANS | STG_MOVE_ROT)>;

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
    - gui_movemask <int>\n
    define how the model can be moved by the mouse in the GUI window

*/

/*
  TODO

  - friction float [WARNING: Friction model is not yet implemented;
  - details may change] if > 0 the model can be pushed around by other
  - moving objects. The value determines the proportion of velocity
  - lost per second. For example, 0.1 would mean that the object would
  - lose 10% of its speed due to friction per second. A value of zero
  - (the default) means this model can not be pushed around (infinite
  - friction).
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
GHashTable* Model::modelsbyid = g_hash_table_new( NULL, NULL );


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

void Visibility::Load( Worldfile* wf, int wf_entity )
{
  blob_return = wf->ReadInt( wf_entity, "blob_return", blob_return);    
  fiducial_key = wf->ReadInt( wf_entity, "fiducial_key", fiducial_key);
  fiducial_return = wf->ReadInt( wf_entity, "fiducial_return", fiducial_return);
  gripper_return = wf->ReadInt( wf_entity, "gripper_return", gripper_return);    
  laser_return = (stg_laser_return_t)wf->ReadInt( wf_entity, "laser_return", laser_return);    
  obstacle_return = wf->ReadInt( wf_entity, "obstacle_return", obstacle_return);    
  ranger_return = wf->ReadInt( wf_entity, "ranger_return", ranger_return);    
}    

GuiState:: GuiState() :
  grid( false ),
  mask( 0 ),
  nose( false ),
  outline( false )
{ /* nothing to do */}

void GuiState::Load( Worldfile* wf, int wf_entity )
{
  nose = wf->ReadInt( wf_entity, "gui_nose", nose);    
  grid = wf->ReadInt( wf_entity, "gui_grid", grid);    
  outline = wf->ReadInt( wf_entity, "gui_outline", outline);    
  mask = wf->ReadInt( wf_entity, "gui_movemask", mask);    
}    


// constructor
Model::Model( World* world,
	      Model* parent,
				  const stg_model_type_t type )
  : Ancestor(), 	 
    access_mutex(NULL),
    blinkenlights( g_ptr_array_new() ),
    blockgroup(),
    blocks_dl(0),
    boundary(false),
    callbacks( g_hash_table_new( g_direct_hash, g_direct_equal ) ),
    color( 0xFFFF0000 ), // red
    data_fresh(false),
    disabled(false),
	 custom_visual_list( NULL ),
    flag_list(NULL),
    geom(),
    has_default_block( true ),
    id( Model::count++ ),
    initfunc(NULL),
    interval((stg_usec_t)1e4), // 10msec
    last_update(0),
	 log_state(false),
    map_resolution(0.1),
    mass(0),
    on_update_list( false ),
    on_velocity_list( false ),
    parent(parent),
    pose(),
	 power_pack( NULL ),
	 pps_charging(NULL),
    props(NULL),
	 rastervis(),
    rebuild_displaylist(true),
    say_string(NULL),
    stall(false),	 
    subs(0),
    thread_safe( false ),
    trail( g_array_new( false, false, sizeof(stg_trail_item_t) )),
    type(type),	
    used(false),
    velocity(),
    watts(0.0),
	 watts_give(0.0),
	 watts_take(0.0),	 
    wf(NULL),
    wf_entity(0),
    world(world),
	 world_gui( dynamic_cast< WorldGui* >( world ) )
{
  assert( modelsbyid );
  assert( world );
  
  PRINT_DEBUG3( "Constructing model world: %s parent: %s type: %d ",
		world->Token(), 
		parent ? parent->Token() : "(null)",
		type );
  
  g_hash_table_insert( modelsbyid, (void*)id, this );
  
  // Adding this model to its ancestor also gives this model a
  // sensible default name
  if ( parent ) 
    parent->AddChild( this );
  else
    {
      world->AddChild( this );
      // top level models are draggable in the GUI
      gui.mask = (STG_MOVE_TRANS | STG_MOVE_ROT);
    }

  world->AddModel( this );
  
  g_datalist_init( &this->props );
  
  // now we can add the basic square shape
  AddBlockRect( -0.5, -0.5, 1.0, 1.0, 1.0 );

  AddVisualizer( &rastervis, false );

  PRINT_DEBUG2( "finished model %s @ %p", this->token, this );
}

Model::~Model( void )
{
  UnMap(); // remove from the raytrace bitmap

  // children are removed in ancestor class
  
  // remove from parent, if there is one
  if( parent ) 
    parent->children = g_list_remove( parent->children, this );
  else
    world->children = g_list_remove( world->children, this );
  
  if( callbacks ) g_hash_table_destroy( callbacks );
	
  g_datalist_clear( &props );

  g_hash_table_remove( Model::modelsbyid, (void*)id );

  world->RemoveModel( this );
}

void Model::StartUpdating()
{
  if( ! on_update_list )
    {
      on_update_list = true;
      world->StartUpdatingModel( this );
    }
}

void Model::StopUpdating()
{
  on_update_list = false;
  world->StopUpdatingModel( this );
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

  if( initfunc )
    Subscribe();
}  

void Model::InitRecursive()
{
  // init children first
  LISTMETHOD( children, Model*, InitRecursive );
  Init();
}  

void Model::AddFlag( Flag* flag )
{
  if( flag )
	 {
		flag_list = g_list_append( this->flag_list, flag );		
		CallCallbacks( &hooks.flag_incr );		
	 }
}

void Model::RemoveFlag( Flag* flag )
{
  if( flag )
	 {
		flag_list = g_list_remove( this->flag_list, flag );
		CallCallbacks( &hooks.flag_decr );
	 }
}

void Model::PushFlag( Flag* flag )
{
  if( flag )
	 {
		flag_list = g_list_prepend( flag_list, flag);
		CallCallbacks( &hooks.flag_incr );
	 }
}

Flag* Model::PopFlag()
{
  if( flag_list == NULL )
    return NULL;

  Flag* flag = (Flag*)flag_list->data;
  flag_list = g_list_remove( flag_list, flag );

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

// utility for g_free()ing everything in a list
void list_gfree( GList* list )
{
  // free all the data in the list
  LISTFUNCTION( list, gpointer, g_free );

  // free the list elements themselves
  g_list_free( list );
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
  
  for( GList* it=this->children; it; it=it->next )
    {
      Model* child = (Model*)it->data;		
      if( child->IsDescendent( testmod ) )
		  return true;
    }
  
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

inline Pose Model::LocalToGlobal( const Pose& pose ) const
{  
  return pose_sum( pose_sum( GetGlobalPose(), geom.pose ), pose );
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
  for( GList* it=children; it; it=it->next )
    ((Model*)it->data)->MapWithChildren();
}

void Model::UnMapWithChildren()
{
  UnMap();

  // recursive call for all the model's children
  for( GList* it=children; it; it=it->next )
    ((Model*)it->data)->UnMapWithChildren();
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

  printf( "unsubscribe from %s %d\n", token, subs );

  // if this is the last sub, call shutdown
  if( subs < 1 )
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

  for( GList* it=children; it; it=it->next )
    ((Model*)it->data)->Print( prefix );
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

  // TODO:  this could be a callback
  if( initfunc )
    initfunc( this );

  StartUpdating();

  CallCallbacks( &hooks.startup );
}

void Model::Shutdown( void )
{
  //printf( "Shutdown model %s\n", this->token );

  StopUpdating();

  CallCallbacks( &hooks.shutdown );
}

void Model::UpdateIfDue( void )
{
  if( UpdateDue() )
	 {
		Update();
		CallUpdateCallbacks();
	 }
}
  
bool Model::UpdateDue( void )
{
  return( (last_update == 0) || (world->sim_time  >= (last_update + interval)) );
}

void Model::Update( void )
{
  // NOTE - update callbacks are NOT called from here, as this method
  //  may be called in multiple threads, and callbacks may not be
  //  reentrant
  
  //   printf( "[%llu] %s update (%d subs)\n", 
  // 			 this->world->sim_time, this->token, this->subs );
  
  // if we're drawing current and a power pack has been installed
  
  PowerPack* pp = FindPowerPack();
  if( pp && ( watts > 0 ))
    {
      // consume  energy stored in the power pack
      stg_joules_t consumed =  watts * (world->interval_sim * 1e-6); 
      pp->Dissipate( consumed, GetGlobalPose() );      
    }

  last_update = world->sim_time;
  
  if( log_state )
	 world->Log( this );
}

void Model::CallUpdateCallbacks( void )
{
  // if we were updated this timestep, call the callbacks
  if( last_update == world->sim_time ) 
	 CallCallbacks( &hooks.update );
}

stg_meters_t Model::ModelHeight() const
{	
  stg_meters_t m_child = 0; //max size of any child
  for( GList* it=this->children; it; it=it->next )
    {
      Model* child = (Model*)it->data;
      stg_meters_t tmp_h = child->ModelHeight();
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


GList* Model::AppendTouchingModels( GList* list )
{
  return blockgroup.AppendTouchingModels( list );
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
    for( GList* it = children; it; it=it->next ) 
      { 
		  hitmod = ((Model*)it->data)->TestCollisionTree();
		  if( hitmod )
			 break;
      }
  
  //printf( "mod %s test collision done.\n", token );
  return hitmod;  
}  

void Model::UpdateCharge()
{
  assert( watts_give > 0 );
  
  PowerPack* mypp = FindPowerPack();
  assert( mypp );
  
  // detach charger from all the packs charged last time
  for( GList* it = pps_charging; it; it = it->next )
	 ((PowerPack*)it->data)->ChargeStop();
  g_list_free( pps_charging );
  pps_charging = NULL;
  
  // run through and update all appropriate touchers
  for( GList* touchers = AppendTouchingModels( NULL );
		 touchers;
		 touchers = touchers->next )
	 {
		Model* toucher = (Model*)touchers->data;		
 		PowerPack* hispp =toucher->FindPowerPack();		
		
 		if( hispp && toucher->watts_take > 0.0) 
 		  {		
 			 //printf( "   toucher %s can take up to %.2f wats\n", 
			 //		toucher->Token(), toucher->watts_take );
			 
 			 stg_watts_t rate = MIN( watts_give, toucher->watts_take );
 			 stg_joules_t amount =  rate * (world->interval_sim * 1e-6);
			 
			 //printf ( "moving %.2f joules from %s to %s\n",
			 //		 amount, token, toucher->token );
			 
			 // set his charging flag
			 hispp->ChargeStart();
			 
 			 // move some joules from me to him
 			 mypp->TransferTo( hispp, amount );
			 
			 // remember who we are charging so we can detatch next time
			 pps_charging = g_list_prepend( pps_charging, hispp );
 		  }
 	 }
}

void Model::CommitTestedPose()
{
  for( GList* it = children; it; it=it->next ) 
    ((Model*)it->data)->CommitTestedPose();
  
  blockgroup.SwitchToTestedCells();
}
  
Model* Model::ConditionalMove( const Pose& newpose )
{ 
  assert( newpose.a >= -M_PI );
  assert( newpose.a <=  M_PI );

  Pose startpose = pose;
  pose = newpose; // do the move provisionally - we might undo it below
   
  Model* hitmod = TestCollisionTree();
 
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
  
  // TODO - control this properly, and maybe do it faster
  //if( 0 )
  //if( (world->updates % 10 == 0) )
  //     {
  //       stg_trail_item_t checkpoint;
  //       memcpy( &checkpoint.pose, &pose, sizeof(pose));
  //       checkpoint.color = color;
  //       checkpoint.time = world->sim_time;

  //       if( trail->len > 100 )
  // 		  g_array_remove_index( trail, 0 );

  //       g_array_append_val( this->trail, checkpoint );
  //     }

  // convert usec to sec
  double interval = (double)world->interval_sim / 1e6;

  // find the change of pose due to our velocity vector
  Pose p;
  p.x = velocity.x * interval;
  p.y = velocity.y * interval;
  p.z = velocity.z * interval;
  p.a = normalize( velocity.a * interval );
    
  //if( isnan( p.x ) || isnan( p.y )  || isnan( p.z )  || isnan( p.a ) )
  //printf( "UpdatePose bad vel %s [%.2f %.2f %.2f %.2f]\n",
  //   token, p.x, p.y, p.z, p.a );

  // attempts to move to the new pose. If the move fails because we'd
  // hit another model, that model is returned.
  Pose q = pose_sum( pose, p );
  assert( q.a >= -M_PI );
  assert( q.a <=  M_PI );

  Model* hitthing = ConditionalMove( q );

  SetStall( hitthing ? true : false );
}


int Model::TreeToPtrArray( GPtrArray* array ) const
{
  g_ptr_array_add( array, (void*)this );

  //printf( " added %s to array at %p\n", root->token, array );

  int added = 1;

  for(GList* it=children; it; it=it->next )
    added += ((Model*)it->data)->TreeToPtrArray( array );

  return added;
}

Model* Model::GetUnsubscribedModelOfType( stg_model_type_t type ) const
{  
  if( (this->type == type) && (this->subs == 0) )
    return (Model*)this; // discard const

  // this model is no use. try children recursively
  for( GList* it = children; it; it=it->next )
    {
      Model* child = (Model*)it->data;

      Model* found = child->GetUnsubscribedModelOfType( type );
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

Model* Model::GetUnusedModelOfType( stg_model_type_t type )
{
  //printf( "searching for type %d in model %s type %d\n", type, token, this->type );

  if( (this->type == type) && (!this->used ) )
    {
      this->used = true;
      return this; // discard const
    }

  // this model is no use. try children recursively
  for( GList* it = children; it; it=it->next )
    {
      Model* child = (Model*)it->data;

      Model* found = child->GetUnusedModelOfType( type );
      if( found )
	return found;
    }

  // nothing matching below this model
  return NULL;
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
	 pts(NULL)
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

  
  if( pts )
	 {
		glPushMatrix();
		Size sz = mod->blockgroup.GetSize();
		//glTranslatef( -mod->geom.size.x / 2.0, -mod->geom.size.y/2.0, 0 );
		//glScalef( mod->geom.size.x / sz.x, mod->geom.size.y / sz.y, 1 );
		
		// now we're in world meters coordinates
		glPointSize( 4 );
		glBegin( GL_POINTS );
		for( GList* it=pts; it; it=it->next )
		  {
			 stg_point_t* pt = (stg_point_t*)it->data;
			 assert( pt );
			 glVertex2f( pt->x, pt->y );
			 
			 char buf[128];
			 snprintf( buf, 127, "[%.2f x %.2f]", pt->x, pt->y );
			 Gl::draw_string( pt->x, pt->y, 0, buf );		  
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
  pts = g_list_prepend( pts, new stg_point_t( x, y ) );
}

void Model::RasterVis::ClearPts()
{
  if( pts )
	 for( GList* it=pts; it; it=it->next )
		if( it->data )
		  delete (stg_point_t*)it->data;

  g_list_free( pts );
  pts = NULL;
}
