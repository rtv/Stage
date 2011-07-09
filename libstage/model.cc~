/** 
	 $Rev$ 
**/

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

		update_interval 100

    color "red"
    color_rgba [ 0.0 0.0 0.0 1.0 ]
    bitmap ""
    ctrl ""

    # determine how the model appears in various sensors
    fiducial_return 0
    fiducial_key 0
    obstacle_return 1
    ranger_return 1.0
    blob_return 1
    ranger_return 1.0
    gripper_return 0

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

    stack_children 1
    )
    @endverbatim

    @par Details
 
    - pose [ x:<float> y:<float> z:<float> heading:<float> ] \n
    specify the pose of the model in its parent's coordinate system

    - size [ x:<float> y:<float> z:<float> ]\n specify the size of the
    model in each dimension

    - origin [ x:<float> y:<float> z:<float> heading:<float> ]\n
    specify the position of the object's center, relative to its pose

    - velocity [ x:<float> y:<float> z:<float> heading:<float>
    omega:<float> ]\n Specify the initial velocity of the model. Note
    that if the model hits an obstacle, its velocity will be set to
    zero.
 
    - update_interval int (defaults to 100) The amount of simulated
      time in milliseconds between calls to Model::Update(). Controls
      the frequency with which this model's data is generated.

		- velocity_enable int (defaults to 0)\n Most models ignore their
		velocity state. This saves on processing time since most models
		never have their velocity set to anything but zero. Some
		subclasses (e.g. ModelPosition) change this default as they are
		expecting to move. Users can specify a non-zero value here to
		enable velocity control of this model. This achieves the same as
		calling Model::VelocityEnable()

    - color <string>\n specify the color of the object using a color
    name from the X11 database (rgb.txt)

    - bitmap filename:<string>\n Draw the model by interpreting the
    lines in a bitmap (bmp, jpeg, gif, png supported). The file is
    opened and parsed into a set of lines.  The lines are scaled to
    fit inside the rectangle defined by the model's current size.

    - ctrl <string>\n Specify the controller module for the model, and
    its argument string. For example, the string "foo bar bash" will
    load libfoo.so, which will have its Init() function called with
    the entire string as an argument (including the library name). It
    is up to the controller to parse the string if it needs
    arguments."
 
    - fiducial_return fiducial_id:<int>\n if non-zero, this model is
    detected by fiducialfinder sensors. The value is used as the
    fiducial ID.

    - fiducial_key <int> models are only detected by fiducialfinders
    if the fiducial_key values of model and fiducialfinder match. This
    allows you to have several independent types of fiducial in the
    same environment, each type only showing up in fiducialfinders
    that are "tuned" for it.

    - obstacle_return <int>\n if 1, this model can collide with other
    models that have this property set

    - blob_return <int>\n if 1, this model can be detected in the
    blob_finder (depending on its color)

    - ranger_return <float>\n This model is detected with this
    reflectance value by ranger_model sensors. If negative, this model
    is invisible to ranger sensors, and does not block propogation of
    range-sensing rays. This models an idealized reflectance sensor,
    and replaces the normal/bright reflectance of deprecated laser
    model. Defaults to 1.0.

    - gripper_return <int>\n iff 1, this model can be gripped by a
    gripper and can be pushed around by collisions with anything that
    has a non-zero obstacle_return.
 
    - gui_nose <int>\n if 1, draw a nose on the model showing its
      heading (positive X axis)

    - gui_grid <int>\n if 1, draw a scaling grid over the model

    - gui_outline <int>\n if 1, draw a bounding box around the model,
    indicating its size

    - gui_move <int>\n if 1, the model can be moved by the mouse in
    the GUI window

    - stack_children <int>\n If non-zero (the default), the coordinate
      system of child models is offset in z so that its origin is on
      _top_ of this model, making it easy to stack models together. If
      zero, the child coordinate system is not offset in z, making it
      easy to define objects in a single local coordinate system.
*/

// todo
//     - friction <float>\n Determines the proportion of velocity lost
//     per second. For example, 0.1 would mean that the object would lose
//     10% of its speed due to friction per second. A value of zero (the
//     default) means this model can not be pushed around (infinite
//     friction).

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <map>

//#define DEBUG 0
#include "stage.hh"
#include "worldfile.hh"
using namespace Stg;

// static members
uint32_t Model::count(0);
uint32_t Model::trail_length(50);
uint64_t Model::trail_interval(5);
std::map<Stg::id_t,Model*> Model::modelsbyid;
std::map<std::string, creator_t> Model::name_map;

//static const members
static const double DEFAULT_FRICTION = 0.0;

void Bounds::Load( Worldfile* wf, const int section, const char* keyword )
{
	if( CProperty* prop = wf->GetProperty( section, keyword ) )	
		{
			if( prop->values.size() != 2 )
				{
					puts( "" ); // newline
					PRINT_ERR1( "Loading 1D bounds. Need a vector of length 2: found %d.\n", 
											(int)prop->values.size() ); 
					exit(-1);
				}
			
			min = atof( wf->GetPropertyValue( prop, 0 )) * wf->unit_length;
			max = atof( wf->GetPropertyValue( prop, 1 )) * wf->unit_length;
		}
}

bool Color::Load( Worldfile* wf, const int section )
{
  if( wf->PropertyExists( section, "color" ))
		{      
			const std::string& colorstr = wf->ReadString( section, "color", "" );
			if( colorstr != "" )
				{
					if( colorstr == "random" )
						{
							r = drand48();
							g = drand48();
							b = drand48();
							a = 1.0;
						}
					else
						{
							Color c = Color( colorstr );
							r = c.r;
							g = c.g;
							b = c.b;
							a = c.a;
						}
				}
			return true;
		}        
	
  if( wf->PropertyExists( section, "color_rgba" ))
    {      
      if (wf->GetProperty(section,"color_rgba")->values.size() < 4)
				{
					PRINT_ERR1( "color_rgba requires 4 values, found %d\n", 
											(int)wf->GetProperty(section,"color_rgba")->values.size() );
					exit(-1);
				}
      else
				{
					r = wf->ReadTupleFloat( section, "color_rgba", 0, r );
					g = wf->ReadTupleFloat( section, "color_rgba", 1, g );
					b = wf->ReadTupleFloat( section, "color_rgba", 2, b );
					a = wf->ReadTupleFloat( section, "color_rgba", 3, a );
				}  
			
			return true;
    }
	
	return false;
}

void Size::Load( Worldfile* wf, const int section, const char* keyword )
{
	if( CProperty* prop = wf->GetProperty( section, keyword ) )	
		{
			if( prop->values.size() != 3 )
				{
					puts( "" ); // newline
					PRINT_ERR1( "Loading size. Need a vector of length 3: found %d.\n", 
											(int)prop->values.size() ); 
					exit(-1);
				}
			
			x = atof( wf->GetPropertyValue( prop, 0 )) * wf->unit_length;
			y = atof( wf->GetPropertyValue( prop, 1 )) * wf->unit_length;
			z = atof( wf->GetPropertyValue( prop, 2 )) * wf->unit_length;
		}
}

void Size::Save( Worldfile* wf, int section, const char* keyword ) const
{
  wf->WriteTupleLength( section, keyword, 0, x );
  wf->WriteTupleLength( section, keyword, 1, y );
  wf->WriteTupleLength( section, keyword, 2, z );
}

void Pose::Load( Worldfile* wf, const int section, const char* keyword )
{
	CProperty* prop = wf->GetProperty( section, keyword );	
	
	if( prop )
		{
			if( prop->values.size() != 4 )
				{
					puts( "" ); // newline
					PRINT_ERR1( "Loading pose. Need a vector of length 4: found %d.\n", 
											(int)prop->values.size() ); 
					exit(-1);
				}
			
			x = atof( wf->GetPropertyValue( prop, 0 )) * wf->unit_length;
			y = atof( wf->GetPropertyValue( prop, 1 )) * wf->unit_length;
			z = atof( wf->GetPropertyValue( prop, 2 )) * wf->unit_length;
			a = atof( wf->GetPropertyValue( prop, 3 )) * wf->unit_angle;
		}
}

void Pose::Save( Worldfile* wf, const int section, const char* keyword )
{
  wf->WriteTupleLength( section, keyword, 0, x );
  wf->WriteTupleLength( section, keyword, 1, y );
  wf->WriteTupleLength( section, keyword, 2, z );
  wf->WriteTupleAngle(  section, keyword, 3, a );
}

Model::Visibility::Visibility() : 
  blob_return( true ),
  fiducial_key( 0 ),
  fiducial_return( 0 ),
  gripper_return( false ),
  obstacle_return( true ),
  ranger_return( 1.0 )
{ /* nothing to do */ }


void Model::Visibility::Load( Worldfile* wf, int wf_entity )
{
  blob_return = wf->ReadInt( wf_entity, "blob_return", blob_return);    
  fiducial_key = wf->ReadInt( wf_entity, "fiducial_key", fiducial_key);
  fiducial_return = wf->ReadInt( wf_entity, "fiducial_return", fiducial_return);
  gripper_return = wf->ReadInt( wf_entity, "gripper_return", gripper_return);    
  obstacle_return = wf->ReadInt( wf_entity, "obstacle_return", obstacle_return);    
  ranger_return = wf->ReadFloat( wf_entity, "ranger_return", ranger_return);    
}    

void Model::Visibility::Save( Worldfile* wf, int wf_entity )
{
  wf->WriteInt( wf_entity, "blob_return", blob_return);    
  wf->WriteInt( wf_entity, "fiducial_key", fiducial_key);
  wf->WriteInt( wf_entity, "fiducial_return", fiducial_return);
  wf->WriteInt( wf_entity, "gripper_return", gripper_return);    
  wf->WriteInt( wf_entity, "obstacle_return", obstacle_return);    
  wf->WriteFloat( wf_entity, "ranger_return", ranger_return);    
}    


Model::GuiState::GuiState() :
  grid( false ),
  move( false ),  
  nose( false ),
  outline( false )
{ /* nothing to do */}

void Model::GuiState::Load( Worldfile* wf, int wf_entity )
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
  mapped(false),
  drawOptions(),
  alwayson(false),
  blockgroup(),
  blocks_dl(0),
  boundary(false),
  callbacks(__CB_TYPE_COUNT), // one slot in the vector for each type
  color( 1,0,0 ), // red
  data_fresh(false),
  disabled(false),
  cv_list(),
  flag_list(),
	friction(DEFAULT_FRICTION),
  geom(),
  has_default_block( true ),
  id( Model::count++ ),
  interval((usec_t)1e5), // 100msec
  interval_energy((usec_t)1e5), // 100msec
  interval_pose((usec_t)1e5), // 100msec
  last_update(0),
  log_state(false),
  map_resolution(0.1),
  mass(0),
  parent(parent),
  pose(),
  power_pack( NULL ),
  pps_charging(),
  rastervis(),
  rebuild_displaylist(true),
  say_string(),
  stack_children( true ),
  stall(false),	 
  subs(0),
  thread_safe( false ),
  trail(trail_length),
  trail_index(0),
  type(type),	
  event_queue_num( 0 ),
  used(false),
  velocity(),
  velocity_enable( false ),
  watts(0.0),
  watts_give(0.0),
  watts_take(0.0),	 
  wf(NULL),
  wf_entity(0),
  world(world),
  world_gui( dynamic_cast<WorldGui*>( world ) )
{
  assert( world );
  
  PRINT_DEBUG3( "Constructing model world: %s parent: %s type: %s \n",
					 world->Token(), 
					 parent ? parent->Token() : "(null)",
					 type.c_str() );
  
  modelsbyid[id] = this;
  
  // Adding this model to its ancestor also gives this model a
  // sensible default name
  if ( parent ) 
    parent->AddChild( this );
  else
    {
      world->AddChild( this );
      // top level models are draggable in the GUI by default
      gui.move = true;
    }

  world->AddModel( this );
      
  // now we can add the basic square shape
  AddBlockRect( -0.5, -0.5, 1.0, 1.0, 1.0 );

  AddVisualizer( &rastervis, false );

  PRINT_DEBUG2( "finished model %s @ %p", this->Token(), this );
}

Model::~Model( void )
{
  // children are removed in ancestor class
  
  if( world ) // if I'm not a worldless dummy model
	 {
		UnMap(0); // remove from the movable model array
		UnMap(1); // remove from the moveable model array
	 		
		// remove myself from my parent's child list, or the world's child
		// list if I have no parent		
		EraseAll( this, parent ? parent->children : world->children );			
		
		// erase from the static map of all models
		modelsbyid.erase(id);			
				
		world->RemoveModel( this );
	 }
}


void Model::InitControllers()
{
  CallCallbacks( CB_INIT );
}


void Model::AddFlag( Flag* flag )
{
  if( flag )
	 {
		flag_list.push_back( flag );		
		CallCallbacks( CB_FLAGINCR );
	 }
}

void Model::RemoveFlag( Flag* flag )
{
  if( flag )
	 {
		EraseAll( flag, flag_list );
		CallCallbacks( CB_FLAGDECR );
	 }
}

void Model::PushFlag( Flag* flag )
{
  if( flag )
		{
			flag_list.push_front( flag);
			CallCallbacks( CB_FLAGINCR );
		}
}

Model::Flag* Model::PopFlag()
{
  if( flag_list.size() == 0 )
    return NULL;
  
  Flag* flag = flag_list.front();
  flag_list.pop_front();
  
	CallCallbacks( CB_FLAGDECR );

  return flag;
}

void Model::ClearBlocks()
{
  blockgroup.UnMap(0);
  blockgroup.UnMap(1);  
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


Block* Model::AddBlockRect( meters_t x, 
														meters_t y, 
														meters_t dx, 
														meters_t dy,
														meters_t dz )
{  
  UnMap(0);
  UnMap(1);
  
  std::vector<point_t> pts(4);
  pts[0].x = x;
  pts[0].y = y;
  pts[1].x = x + dx;
  pts[1].y = y;
  pts[2].x = x + dx;
  pts[2].y = y + dy;
  pts[3].x = x;
  pts[3].y = y + dy;
  
  Block* newblock =  new Block( this,
										  pts,
										  0, dz, 
										  color,
										  true, 
										  false );
  
  blockgroup.AppendBlock( newblock );
  
  Map(0);
  Map(1);
  
  return newblock;
}


RaytraceResult Model::Raytrace( const Pose &pose,
				       const meters_t range, 
				       const ray_test_func_t func,
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

RaytraceResult Model::Raytrace( const radians_t bearing,
																const meters_t range, 
																const ray_test_func_t func,
																const void* arg,
																const bool ztest )
{
  return world->Raytrace( LocalToGlobal(Pose(0,0,0,bearing)),
													range,
													func,
													this,
													arg,
													ztest );
}


void Model::Raytrace( const radians_t bearing,
											const meters_t range, 
											const radians_t fov,
											const ray_test_func_t func,
											const void* arg,
											RaytraceResult* samples,
											const uint32_t sample_count,
											const bool ztest )
{
  world->Raytrace( LocalToGlobal(Pose( 0,0,0,bearing)),
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
  const Pose org( GetGlobalPose() );
  
  // compute global pose in local coords
  const double sx =  (pose.x - org.x) * cos(org.a) + (pose.y - org.y) * sin(org.a);
  const double sy = -(pose.x - org.x) * sin(org.a) + (pose.y - org.y) * cos(org.a);
  const double sz = pose.z - org.z;
  const double sa = pose.a - org.a;
  
  return Pose( sx, sy, sz, sa );
}

void Model::Say( const std::string& str )
{
  say_string = str;
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

point_t Model::LocalToGlobal( const point_t& pt) const
{  
	const Pose gpose = LocalToGlobal( Pose( pt.x, pt.y, 0, 0 ) );
	return point_t( gpose.x, gpose.y );
}


void Model::LocalToPixels( const std::vector<point_t>& local,
													 std::vector<point_int_t>& global) const
{
	const Pose gpose = GetGlobalPose() + geom.pose;
	
	FOR_EACH( it, local )
		{
			Pose ptpose = gpose + Pose( it->x, it->y, 0, 0 );
			global.push_back( point_int_t( (int32_t)floor( ptpose.x * world->ppm) ,
																		 (int32_t)floor( ptpose.y * world->ppm) ));
		}
}

void Model::MapWithChildren( unsigned int layer )
{
  UnMap(layer);
  Map(layer);

  // recursive call for all the model's children
  FOR_EACH( it, children )
    (*it)->MapWithChildren(layer);
}

void Model::MapFromRoot( unsigned int layer )
{
	Root()->MapWithChildren(layer);
}

void Model::UnMapWithChildren(unsigned int layer)
{
  UnMap(layer);

  // recursive call for all the model's children
  FOR_EACH( it, children )
    (*it)->UnMapWithChildren(layer);
}

void Model::UnMapFromRoot(unsigned int layer)
{
	Root()->UnMapWithChildren(layer);
}

void Model::Subscribe( void )
{
  subs++;
  world->total_subs++;
  world->dirty = true; // need redraw
  
  //printf( "subscribe to %s %d\n", Token(), subs );
  
  // if this is the first sub, call startup
  if( subs == 1 )
    Startup();
}

void Model::Unsubscribe( void )
{
  subs--;
  world->total_subs--;
  world->dirty = true; // need redraw

  //printf( "unsubscribe from %s %d\n", Token(), subs );

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
			 token.c_str() );
  
  FOR_EACH( it, children )
    (*it)->Print( prefix );
}

const char* Model::PrintWithPose() const
{
  const Pose gpose = GetGlobalPose();
	
  static char txt[256];
  snprintf(txt, sizeof(txt), "%s @ [%.2f,%.2f,%.2f,%.2f]",  
			  token.c_str(), 
			  gpose.x, gpose.y, gpose.z, gpose.a  );

  return txt;
}

void Model::Startup( void )
{
  //printf( "Startup model %s\n", this->token );  
  //printf( "model %s using queue %d\n", token, event_queue_num );
	
  // if we're thread safe, we can use an event queue >0  
	event_queue_num = world->GetEventQueue( this );
	
  // put my first update request in the world's queue
	if( thread_safe )
		world->Enqueue( event_queue_num, interval, this, UpdateWrapper, NULL );
	else
		world->Enqueue( 0, interval, this, UpdateWrapper, NULL );
	
	if( velocity_enable )
	  world->active_velocity.insert(this);
	
  if( FindPowerPack() )
		world->active_energy.insert( this );
  
	CallCallbacks( CB_STARTUP );
}

void Model::Shutdown( void )
{
  //printf( "Shutdown model %s\n", this->token );
  CallCallbacks( CB_SHUTDOWN );
  
  world->active_energy.erase( this );
  world->active_velocity.erase( this );
	velocity_enable = false;

  // allows data visualizations to be cleared.
  NeedRedraw();
}


void Model::Update( void )
{ 
	//printf( "Q%d model %p %s update\n", event_queue_num, this, Token() );

	//	CallCallbacks( CB_UPDATE );
	
  last_update = world->sim_time;  
	
	if( subs > 0 ) // no subscriptions means we don't need to be updated
		world->Enqueue( event_queue_num, interval, this, UpdateWrapper, NULL );
	
	// if we updated the model then it needs to have its update
	// callback called in series back in the main thread. It's
	// not safe to run user callbacks in a worker thread, as
	// they may make OpenGL calls or unsafe Stage API calls,
	// etc. We queue up the callback into a queue specific to

	if( ! callbacks[Model::CB_UPDATE].empty() )
		world->pending_update_callbacks[event_queue_num].push(this);					
}

void Model::CallUpdateCallbacks( void )
{
	CallCallbacks( CB_UPDATE );
}

meters_t Model::ModelHeight() const
{	
  meters_t m_child = 0; //max size of any child
  FOR_EACH( it, children )
		m_child = std::max( m_child, (*it)->ModelHeight() );
	
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

void Model::PlaceInFreeSpace( meters_t xmin, meters_t xmax, 
			      meters_t ymin, meters_t ymax )
{
  while( TestCollision() )
    SetPose( Pose::Random( xmin,xmax, ymin, ymax ));		
}

void Model::AppendTouchingModels( ModelPtrSet& touchers )
{
  blockgroup.AppendTouchingModels( touchers );
}


Model* Model::TestCollision()
{  
  Model* hitmod = blockgroup.TestCollision();
  
  if( hitmod == NULL ) 	 
	 FOR_EACH( it, children )
		 { 
			 hitmod = (*it)->TestCollision();
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
			mypp->Dissipate( watts * (interval_energy * 1e-6), GetGlobalPose() );      
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
				  
				  const watts_t rate = std::min( watts_give, toucher->watts_take );
				  const joules_t amount =  rate * interval_energy * 1e-6;
				  
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
}


void Model::Move( void )
{  
  if( velocity.IsZero() )
	 return;

  if( disabled )
	 return;

  // convert usec to sec
  const double interval( (double)world->sim_interval / 1e6 );
  
  // find the change of pose due to our velocity vector
  const Pose p( velocity.x * interval,
					 velocity.y * interval,
					 velocity.z * interval,
					 normalize( velocity.a * interval ));
  
  // the pose we're trying to achieve (unless something stops us)
  const Pose newpose( pose + p );
  
  // stash the original pose so we can put things back if we hit
  const Pose startpose( pose );
  
  pose = newpose; // do the move provisionally - we might undo it below
  
  //const unsigned int layer( world->updates%2 );
  
  const unsigned int layer( world->updates%2 );
  
  UnMapWithChildren( layer ); // remove from all blocks
  MapWithChildren( layer ); // render into new blocks
  
  if( TestCollision() ) // crunch!
	 {
		// put things back the way they were
		// this is expensive, but it happens _very_ rarely for most people
		pose = startpose;
		UnMapWithChildren( layer );
		MapWithChildren( layer );
		SetStall(true);
	 }
  else
	 {
		world->dirty = true; // need redraw	
		SetStall(false);
	 }
}


void Model::UpdateTrail()
{
	// get the current item and increment the counter
	TrailItem* item = &trail[trail_index++];
	
	// record the current info
	item->time = world->sim_time;
	item->pose = GetGlobalPose();
	item->color = color;

	// wrap around ring buffer
	trail_index %= trail_length;
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

void Model::Redraw( void )
{
	world->Redraw();
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

kg_t Model::GetTotalMass() const
{
  kg_t sum = mass;
  
  FOR_EACH( it, children )
	 sum += (*it)->GetTotalMass();

  return sum;
}

kg_t Model::GetMassOfChildren() const
{
  return( GetTotalMass() - mass);
}

void Model::Map( unsigned int layer )
{
  if( ! mapped )
	 {
		// render all blocks in the group at my global pose and size
		blockgroup.Map( layer );
		mapped = true;
	 }
} 

void Model::UnMap( unsigned int layer )
{
  if( mapped )
	 {
		blockgroup.UnMap(layer);
		mapped = false;
	 }
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
  world->RegisterOption( opt );
}


void Model::Rasterize( uint8_t* data, 
		       unsigned int width, 
		       unsigned int height, 
		       meters_t cellwidth,
		       meters_t cellheight )
{
  rastervis.ClearPts();
  blockgroup.Rasterize( data, width, height, cellwidth, cellheight );
  rastervis.SetData( data, width, height, cellwidth, cellheight );
}

void Model::SetFriction( double friction )
{
  this->friction = friction;
}

Model* Model::GetChild( const std::string& modelname ) const
{
  // construct the full model name and look it up
  
  const std::string fullname = token + "." + modelname;
  
  Model* mod = world->GetModel( fullname );
  
  if( mod == NULL )
	 PRINT_WARN1( "Model %s not found", fullname.c_str() );
  
   return mod;
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
  (void)cam; // avoid warning about unused var

  if( data == NULL )
	 return;

  // go into world coordinates  
  glPushMatrix();
  mod->PushColor( 1,0,0,0.5 );

  Gl::pose_inverse_shift( mod->GetGlobalPose() );
  
  if( pts.size() > 0 )
	 {
		glPushMatrix();
		//Size sz = mod->blockgroup.GetSize();
		//glTranslatef( -mod->geom.size.x / 2.0, -mod->geom.size.y/2.0, 0 );
		//glScalef( mod->geom.size.x / sz.x, mod->geom.size.y / sz.y, 1 );
		
		// now we're in world meters coordinates
		glPointSize( 4 );
		glBegin( GL_POINTS );
		
		FOR_EACH( it, pts )
		  {
			 point_t& pt = *it;
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
  for( unsigned int y=0; y<height; ++y )
	 for( unsigned int x=0; x<width; ++x )
		{
		  // printf( "[%u %u] ", x, y );
		  if( data[ x + y*width ] )
			 glRectf( x, y, x+1, y+1 );
		}

  glTranslatef( 0,0,0.01 );

  mod->PushColor( 0,0,0,1 );
  glPolygonMode( GL_FRONT, GL_LINE );
  for( unsigned int y=0; y<height; ++y )
	 for( unsigned int x=0; x<width; ++x )
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
																const unsigned int width, 
																const unsigned int height,
																const meters_t cellwidth, 
																const meters_t cellheight )
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


void Model::RasterVis::AddPoint( const meters_t x, const meters_t y )
{
  pts.push_back( point_t( x, y ) );
}

void Model::RasterVis::ClearPts()
{
  pts.clear();
}


Model::Flag::Flag( const Color color, const double size ) 
	: color(color), size(size), displaylist(0)
{ 
}

Model::Flag* Model::Flag::Nibble( double chunk )
{
	Flag* piece = NULL;

	if( size > 0 )
	{
		chunk = std::min( chunk, this->size );
		piece = new Flag( this->color, chunk );
		this->size -= chunk;
	}

	return piece;
}


void Model::Flag::SetColor( const Color& c )
{
	color = c;
	
	if( displaylist )
		{
			// force recreation of list
			glDeleteLists( displaylist, 1 );
			displaylist = 0;
		}
}

void Model::Flag::SetSize( double sz )
{
	size = sz;
	
	if( displaylist )
		{
			// force recreation of list
			glDeleteLists( displaylist, 1 );
			displaylist = 0;
		}
}


void Model::Flag::Draw(  GLUquadric* quadric )
{
	if( displaylist == 0 )
		{
			displaylist = glGenLists(1);
			assert( displaylist > 0 );
			
			glNewList( displaylist, GL_COMPILE );	
			
			glColor4f( color.r, color.g, color.b, color.a );
			
			glEnable(GL_POLYGON_OFFSET_FILL);
			glPolygonOffset(1.0, 1.0);
			gluQuadricDrawStyle( quadric, GLU_FILL );
			gluSphere( quadric, size/2.0, 4,2  );
			glDisable(GL_POLYGON_OFFSET_FILL);
			
			// draw the edges darker version of the same color
			glColor4f( color.r/2.0, color.g/2.0, color.b/2.0, color.a/2.0 );
			
			gluQuadricDrawStyle( quadric, GLU_LINE );
			gluSphere( quadric, size/2.0, 4,2 );

			glEndList();
		}

	glCallList( displaylist );
}
