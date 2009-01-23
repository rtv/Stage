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
#include "canvas.hh"
#include "texture_manager.hh"
using namespace Stg;

// speech bubble colors
static const stg_color_t BUBBLE_FILL = 0xFFC8C8FF; // light blue/grey
static const stg_color_t BUBBLE_BORDER = 0xFF000000; // black
static const stg_color_t BUBBLE_TEXT = 0xFF000000; // black

// static members
uint32_t Model::count = 0;
GHashTable* Model::modelsbyid = g_hash_table_new( NULL, NULL );

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
    callbacks( g_hash_table_new( g_int_hash, g_int_equal ) ),
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
    map_caches_are_invalid( true ),
    map_resolution(0.1),
    mass(0),
    on_update_list( false ),
    on_velocity_list( false ),
    parent(parent),
    pose(),
	 power_pack( NULL ),
    props(NULL),
    rebuild_displaylist(true),
    say_string(NULL),
    stall(false),	 
    subs(0),
    thread_safe( false ),
    trail( g_array_new( false, false, sizeof(stg_trail_item_t) )),
    type(type),	
    used(false),
    velocity(),
    watts(0),
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
  map_caches_are_invalid = true;
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
    flag_list = g_list_append( this->flag_list, flag );
}

void Model::RemoveFlag( Flag* flag )
{
  if( flag )
    flag_list = g_list_remove( this->flag_list, flag );
}

void Model::PushFlag( Flag* flag )
{
  if( flag )
    flag_list = g_list_prepend( flag_list, flag);
}

Flag* Model::PopFlag()
{
  if( flag_list == NULL )
    return NULL;

  Flag* flag = (Flag*)flag_list->data;
  flag_list = g_list_remove( flag_list, flag );

  return flag;
}


void Model::ClearBlocks( void )
{
  UnMap();
  blockgroup.Clear();
  map_caches_are_invalid = true;

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


void Model::AddBlockRect( stg_meters_t x, 
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
  
  blockgroup.AppendBlock( new Block( this,
				     pts, 4, 
				     0, dz, 
				     color,
				     true ) );
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
Pose Model::GlobalToLocal( Pose pose )
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
bool Model::IsAntecedent( Model* testmod )
{
  if( parent == NULL )
    return false;
  
  if( parent == testmod )
    return true;
  
  return parent->IsAntecedent( testmod );
}

// returns true iff model [testmod] is a descendent of this model
bool Model::IsDescendent( Model* testmod )
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

bool Model::IsRelated( Model* that )
{
  // is it me?
  if( this == that )
 	 return true;
  
  // wind up to top-level object
  Model* candidate = this;
  while( candidate->parent )
	 candidate = candidate->parent;
  
  // and recurse down the tree    
  return candidate->IsDescendent( that );
}


// get the model's velocity in the global frame
Velocity Model::GetGlobalVelocity()
{
  Pose gpose = GetGlobalPose();

  double cosa = cos( gpose.a );
  double sina = sin( gpose.a );

  Velocity gv;
  gv.x = velocity.x * cosa - velocity.y * sina;
  gv.y = velocity.x * sina + velocity.y * cosa;
  gv.a = velocity.a;

  //printf( "local velocity %.2f %.2f %.2f\nglobal velocity %.2f %.2f %.2f\n",
  //  mod->velocity.x, mod->velocity.y, mod->velocity.a,
  //  gvel->x, gvel->y, gvel->a );

  return gv;
}

// set the model's velocity in the global frame
void Model::SetGlobalVelocity( Velocity gv )
{
  Pose gpose = GetGlobalPose();

  double cosa = cos( gpose.a );
  double sina = sin( gpose.a );

  Velocity lv;
  lv.x = gv.x * cosa + gv.y * sina;
  lv.y = -gv.x * sina + gv.y * cosa;
  lv.a = gv.a;

  this->SetVelocity( lv );
}

// get the model's position in the global frame
Pose Model::GetGlobalPose()
{ 
  // if I'm a top level model, my global pose is my local pose
  if( parent == NULL )
    return pose;
  
  // otherwise  
  
  Pose global_pose = pose_sum( parent->GetGlobalPose(), pose );		
  
  // we are on top of our parent
  global_pose.z += parent->geom.size.z;
  
  //   PRINT_DEBUG4( "GET GLOBAL POSE [x:%.2f y:%.2f z:%.2f a:%.2f]",
  // 		global_pose.x,
  // 		global_pose.y,
  // 		global_pose.z,
  // 		global_pose.a );

  return global_pose;
}


inline Pose Model::LocalToGlobal( Pose pose )
{  
  return pose_sum( pose_sum( GetGlobalPose(), geom.pose ), pose );
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
  map_caches_are_invalid = false;
} 


void Model::Subscribe( void )
{
  subs++;
  world->total_subs++;

  //printf( "subscribe to %s %d\n", token, subs );

  // if this is the first sub, call startup
  if( this->subs == 1 )
    this->Startup();
}

void Model::Unsubscribe( void )
{
  subs--;
  world->total_subs--;

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

void Model::Print( char* prefix )
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

const char* Model::PrintWithPose()
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
    Update();
}
  
bool Model::UpdateDue( void )
{
  return( world->sim_time  >= (last_update + interval) );
}
 
void Model::Update( void )
{
  //   printf( "[%llu] %s update (%d subs)\n", 
  // 			 this->world->sim_time, this->token, this->subs );
  
  // f we're drawing current and a power pack has been installed
  if( power_pack )
	 {
		if( watts > 0 )
		  {
			 // consume  energy stored in the power pack
			 stg_joules_t consumed =  watts * (world->interval_sim * 1e-6); 
			 power_pack->stored -= consumed;
			 
			 /*		
						printf ( "%s current %.2f consumed %.6f ppack @ %p [ %.2f/%.2f (%.0f)\n",
						token, 
						watts, 
						consumed, 
						power_pack, 
						power_pack->stored, 
						power_pack->capacity, 
						power_pack->stored / power_pack->capacity * 100.0 );
			 */
		  }
		
		// I own this power pack, see if the world wants to recharge it */
		if( power_pack->mod == this )
		  world->TryCharge( power_pack, GetGlobalPose() );
	 }

  CallCallbacks( &hooks.update );
  last_update = world->sim_time;
}

void Model::DrawSelected()
{
  glPushMatrix();
  
  glTranslatef( pose.x, pose.y, pose.z+0.01 ); // tiny Z offset raises rect above grid
  
  Pose gpose = GetGlobalPose();
  
  char buf[64];
  snprintf( buf, 63, "%s [%.2f %.2f %.2f %.2f]", 
	    token, gpose.x, gpose.y, gpose.z, rtod(gpose.a) );
  
  PushColor( 0,0,0,1 ); // text color black
  Gl::draw_string( 0.5,0.5,0.5, buf );
  
  glRotatef( rtod(pose.a), 0,0,1 );
  
  Gl::pose_shift( geom.pose );
  
  double dx = geom.size.x / 2.0 * 1.6;
  double dy = geom.size.y / 2.0 * 1.6;
  
  PopColor();

  PushColor( 0,1,0,0.4 ); // highlight color blue
  glRectf( -dx, -dy, dx, dy );
  PopColor();

  PushColor( 0,1,0,0.8 ); // highlight color blue
  glLineWidth( 1 );
  glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
  glRectf( -dx, -dy, dx, dy );
  glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
  PopColor();

  glPopMatrix();
}


void Model::DrawTrailFootprint()
{
  double r,g,b,a;

  for( int i=trail->len-1; i>=0; i-- )
    {
      stg_trail_item_t* checkpoint = & g_array_index( trail, stg_trail_item_t, i );

      glPushMatrix();
		Gl::pose_shift( checkpoint->pose );
		Gl::pose_shift( geom.pose );

      stg_color_unpack( checkpoint->color, &r, &g, &b, &a );
      PushColor( r, g, b, 0.1 );

      blockgroup.DrawFootPrint( geom );

      glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
      PushColor( r/2, g/2, b/2, 0.1 );

      blockgroup.DrawFootPrint( geom );

      PopColor();
      PopColor();
      glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
      glPopMatrix();
    }
}

void Model::DrawTrailBlocks()
{
  double timescale = 0.0000001;

  for( int i=trail->len-1; i>=0; i-- )
    {
      stg_trail_item_t* checkpoint = & g_array_index( trail, stg_trail_item_t, i );

      Pose pose;
      memcpy( &pose, &checkpoint->pose, sizeof(pose));
      pose.z =  (world->sim_time - checkpoint->time) * timescale;

      PushLocalCoords();
      //blockgroup.Draw( this );

      DrawBlocksTree();
      PopCoords();
    }
}

void Model::DrawTrailArrows()
{
  double r,g,b,a;

  double dx = 0.2;
  double dy = 0.07;
  double timescale = 0.0000001;

  for( unsigned int i=0; i<trail->len; i++ )
    {
      stg_trail_item_t* checkpoint = & g_array_index( trail, stg_trail_item_t, i );

      Pose pose;
      memcpy( &pose, &checkpoint->pose, sizeof(pose));
      pose.z =  (world->sim_time - checkpoint->time) * timescale;

      PushLocalCoords();

      // set the height proportional to age

      PushColor( checkpoint->color );

      glEnable(GL_POLYGON_OFFSET_FILL);
      glPolygonOffset(1.0, 1.0);

      glBegin( GL_TRIANGLES );
      glVertex3f( 0, -dy, 0);
      glVertex3f( dx, 0, 0 );
      glVertex3f( 0, +dy, 0 );
      glEnd();
      glDisable(GL_POLYGON_OFFSET_FILL);

      glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );

      stg_color_unpack( checkpoint->color, &r, &g, &b, &a );
      PushColor( r/2, g/2, b/2, 1 ); // darker color

      glDepthMask(GL_FALSE);
      glBegin( GL_TRIANGLES );
      glVertex3f( 0, -dy, 0);
      glVertex3f( dx, 0, 0 );
      glVertex3f( 0, +dy, 0 );
      glEnd();
      glDepthMask(GL_TRUE);

      glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
      PopColor();
      PopColor();
      PopCoords();
    }
}

void Model::DrawOriginTree()
{
  DrawPose( GetGlobalPose() );  
  for( GList* it=children; it; it=it->next )
    ((Model*)it->data)->DrawOriginTree();
}
 

void Model::DrawBlocksTree( )
{
  PushLocalCoords();
  LISTMETHOD( children, Model*, DrawBlocksTree );
  DrawBlocks();  
  PopCoords();
}
  
void Model::DrawPose( Pose pose )
{
  PushColor( 0,0,0,1 );
  glPointSize( 4 );
  
  glBegin( GL_POINTS );
  glVertex3f( pose.x, pose.y, pose.z );
  glEnd();
  
  PopColor();  
}

void Model::DrawBlocks( )
{ 
  blockgroup.CallDisplayList( this );
}

void Model::DrawBoundingBoxTree()
{
  PushLocalCoords();
  LISTMETHOD( children, Model*, DrawBoundingBoxTree );
  DrawBoundingBox();
  PopCoords();
}

void Model::DrawBoundingBox()
{
  Gl::pose_shift( geom.pose );  

  PushColor( color );
  
  glBegin( GL_QUAD_STRIP );
  
  glVertex3f( -geom.size.x/2.0, -geom.size.y/2.0, geom.size.z );
  glVertex3f( -geom.size.x/2.0, -geom.size.y/2.0, 0 );
 
  glVertex3f( +geom.size.x/2.0, -geom.size.y/2.0, geom.size.z );
  glVertex3f( +geom.size.x/2.0, -geom.size.y/2.0, 0 );
 
  glVertex3f( +geom.size.x/2.0, +geom.size.y/2.0, geom.size.z );
  glVertex3f( +geom.size.x/2.0, +geom.size.y/2.0, 0 );

  glVertex3f( +geom.size.x/2.0, +geom.size.y/2.0, geom.size.z );
  glVertex3f( +geom.size.x/2.0, +geom.size.y/2.0, 0 );

  glVertex3f( -geom.size.x/2.0, +geom.size.y/2.0, geom.size.z );
  glVertex3f( -geom.size.x/2.0, +geom.size.y/2.0, 0 );

  glVertex3f( -geom.size.x/2.0, -geom.size.y/2.0, geom.size.z );
  glVertex3f( -geom.size.x/2.0, -geom.size.y/2.0, 0 );

  glEnd();

  glBegin( GL_LINES );
  glVertex2f( -0.02, 0 ); 
  glVertex2f( +0.02, 0 ); 

  glVertex2f( 0, -0.02 ); 
  glVertex2f( 0, +0.02 ); 
  glEnd();

  PopColor();
}

// move into this model's local coordinate frame
void Model::PushLocalCoords()
{
  glPushMatrix();  
  
  if( parent )
    glTranslatef( 0,0, parent->geom.size.z );
  
  Gl::pose_shift( pose );
}

void Model::PopCoords()
{
  glPopMatrix();
}

void Model::AddCustomVisualizer( CustomVisualizer* custom_visual )
{
	if( !custom_visual )
		return;

	//Visualizations can only be added to stage when run in a GUI
	if( world_gui == NULL ) {
		printf( "Unable to add custom visualization - it must be run with a GUI world\n" );
		return;
	}

	//save visual instance
	custom_visual_list = g_list_append(custom_visual_list, custom_visual );

	//register option for all instances which share the same name
	Canvas* canvas = world_gui->GetCanvas();
	std::map< std::string, Option* >::iterator i = canvas->_custom_options.find( custom_visual->name() );
	if( i == canvas->_custom_options.end() ) {
		Option* op = new Option( custom_visual->name(), custom_visual->name(), "", true, world_gui );
		canvas->_custom_options[ custom_visual->name() ] = op;
		registerOption( op );
	}
}

void Model::RemoveCustomVisualizer( CustomVisualizer* custom_visual )
{
	if( custom_visual )
		custom_visual_list = g_list_remove(custom_visual_list, custom_visual );

	//TODO unregister option - tricky because there might still be instances attached to different models which have the same name
}


void Model::DrawStatusTree( Camera* cam ) 
{
  PushLocalCoords();
  DrawStatus( cam );
  LISTMETHODARG( children, Model*, DrawStatusTree, cam );  
  PopCoords();
}

void Model::DrawStatus( Camera* cam ) 
{
  

  if( say_string || power_pack )	  
    {
      float yaw, pitch;
      pitch = - cam->pitch();
      yaw = - cam->yaw();			
      
		Pose gpz = GetGlobalPose();

      float robotAngle = -rtod(gpz.a);
      glPushMatrix();
		
		
      // move above the robot
      glTranslatef( 0, 0, 0.5 );		
		
      // rotate to face screen
      glRotatef( robotAngle - yaw, 0,0,1 );
      glRotatef( -pitch, 1,0,0 );


		//if( ! parent )
		// glRectf( 0,0,1,1 );
		
		if( power_pack && (power_pack->mod == this) )
		  power_pack->Visualize( cam );

		if( say_string )
		  {
			 //get raster positition, add gl_width, then project back to world coords
			 glRasterPos3f( 0, 0, 0 );
			 GLfloat pos[ 4 ];
			 glGetFloatv(GL_CURRENT_RASTER_POSITION, pos);
			 
			 GLboolean valid;
			 glGetBooleanv( GL_CURRENT_RASTER_POSITION_VALID, &valid );
			 
			 if( valid ) 
				{
				  
				  fl_font( FL_HELVETICA, 12 );
				  float w = gl_width( this->say_string ); // scaled text width
				  float h = gl_height(); // scaled text height
				  
				  GLdouble wx, wy, wz;
				  GLint viewport[4];
				  glGetIntegerv(GL_VIEWPORT, viewport);
				  
				  GLdouble modelview[16];
				  glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
				  
				  GLdouble projection[16];	
				  glGetDoublev(GL_PROJECTION_MATRIX, projection);
				  
				  //get width and height in world coords
				  gluUnProject( pos[0] + w, pos[1], pos[2], modelview, projection, viewport, &wx, &wy, &wz );
				  w = wx;
				  gluUnProject( pos[0], pos[1] + h, pos[2], modelview, projection, viewport, &wx, &wy, &wz );
				  h = wy;
				  
				  // calculate speech bubble margin
				  const float m = h/10;
				  
				  // draw inside of bubble
				  PushColor( BUBBLE_FILL );
				  glPushAttrib( GL_POLYGON_BIT | GL_LINE_BIT );
				  glPolygonMode( GL_FRONT, GL_FILL );
				  glEnable( GL_POLYGON_OFFSET_FILL );
				  glPolygonOffset( 1.0, 1.0 );
				  Gl::draw_octagon( w, h, m );
				  glDisable( GL_POLYGON_OFFSET_FILL );
				  PopColor();
				  
				  // draw outline of bubble
				  PushColor( BUBBLE_BORDER );
				  glLineWidth( 1 );
				  glEnable( GL_LINE_SMOOTH );
				  glPolygonMode( GL_FRONT, GL_LINE );
				  Gl::draw_octagon( w, h, m );
				  glPopAttrib();
				  PopColor();
				  
				  PushColor( BUBBLE_TEXT );
				  // draw text inside the bubble
				  Gl::draw_string( 2.5*m, 2.5*m, 0, this->say_string );
				  PopColor();			
				}
		  }
		glPopMatrix();
	 }
  
  if( stall )
    {
      DrawImage( TextureManager::getInstance()._stall_texture_id, cam, 0.85 );
    }
}

stg_meters_t Model::ModelHeight()
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

void Model::DrawImage( uint32_t texture_id, Camera* cam, float alpha )
{
  float yaw, pitch;
  pitch = - cam->pitch();
  yaw = - cam->yaw();
  float robotAngle = -rtod(pose.a);

  glEnable(GL_TEXTURE_2D);
  glBindTexture( GL_TEXTURE_2D, texture_id );

  glColor4f( 1.0, 1.0, 1.0, alpha );
  glPushMatrix();

  //position image above the robot
  glTranslatef( 0.0, 0.0, ModelHeight() + 0.3 );

  // rotate to face screen
  glRotatef( robotAngle - yaw, 0,0,1 );
  glRotatef( -pitch - 90, 1,0,0 );

  //draw a square, with the textured image
  glBegin(GL_QUADS);
  glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.25f, 0, -0.25f );
  glTexCoord2f(1.0f, 0.0f); glVertex3f( 0.25f, 0, -0.25f );
  glTexCoord2f(1.0f, 1.0f); glVertex3f( 0.25f, 0,  0.25f );
  glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.25f, 0,  0.25f );
  glEnd();

  glPopMatrix();
  glBindTexture( GL_TEXTURE_2D, 0 );
  glDisable(GL_TEXTURE_2D);
}

void Model::DrawFlagList( void )
{	
  if( flag_list  == NULL )
    return;
  
  PushLocalCoords();
  
  GLUquadric* quadric = gluNewQuadric();
  glTranslatef(0,0,1); // jump up
  Pose gpose = GetGlobalPose();
  glRotatef( 180 + rtod(-gpose.a),0,0,1 );
  

  GList* list = g_list_copy( flag_list );
  list = g_list_reverse(list);
  
  for( GList* item = list; item; item = item->next )
    {
		
      Flag* flag = (Flag*)item->data;
		
      glTranslatef( 0, 0, flag->size/2.0 );
				
      PushColor( flag->color );
		

      glEnable(GL_POLYGON_OFFSET_FILL);
      glPolygonOffset(1.0, 1.0);
      gluQuadricDrawStyle( quadric, GLU_FILL );
      gluSphere( quadric, flag->size/2.0, 4,2  );
		glDisable(GL_POLYGON_OFFSET_FILL);

      // draw the edges darker version of the same color
      double r,g,b,a;
      stg_color_unpack( flag->color, &r, &g, &b, &a );
      PushColor( stg_color_pack( r/2.0, g/2.0, b/2.0, a ));
		
      gluQuadricDrawStyle( quadric, GLU_LINE );
      gluSphere( quadric, flag->size/2.0, 4,2 );
		
      PopColor();
      PopColor();
		
      glTranslatef( 0, 0, flag->size/2.0 );
    }
  
  g_list_free( list );
  
  gluDeleteQuadric( quadric );
  
  PopCoords();
}


void Model::DrawBlinkenlights()
{
  PushLocalCoords();

  GLUquadric* quadric = gluNewQuadric();
  //glTranslatef(0,0,1); // jump up
  //Pose gpose = GetGlobalPose();
  //glRotatef( 180 + rtod(-gpose.a),0,0,1 );

  for( unsigned int i=0; i<blinkenlights->len; i++ )
    {
      stg_blinkenlight_t* b = 
	(stg_blinkenlight_t*)g_ptr_array_index( blinkenlights, i );
      assert(b);

      glTranslatef( b->pose.x, b->pose.y, b->pose.z );

      PushColor( b->color );

      if( b->enabled )
	gluQuadricDrawStyle( quadric, GLU_FILL );
      else
	gluQuadricDrawStyle( quadric, GLU_LINE );

      gluSphere( quadric, b->size/2.0, 8,8  );

      PopColor();
    }

  gluDeleteQuadric( quadric );

  PopCoords();
}

void Model::DrawPicker( void )
{
  //PRINT_DEBUG1( "Drawing %s", token );
  PushLocalCoords();

  // draw the boxes
  blockgroup.DrawSolid( geom );

  // recursively draw the tree below this model 
  LISTMETHOD( this->children, Model*, DrawPicker );

  PopCoords();
}

void Model::DataVisualize( Camera* cam )
{  
//   if( power_pack )
// 	 {
// 		// back into global coords to get rid of my rotation
// 		glPushMatrix();  
// 		gl_pose_inverse_shift( GetGlobalPose() );

// 		// shift to the top left corner of the model (roughly)
// 		glTranslatef( pose.x - geom.size.x/2.0, 
// 						  pose.y + geom.size.y/2.0, 
// 						  pose.z + geom.size.z );

// 		power_pack->Visualize( cam );
// 		glPopMatrix();
// 	 }
}

void Model::DataVisualizeTree( Camera* cam )
{
  PushLocalCoords();
  DataVisualize( cam ); // virtual function overridden by most model types  

  CustomVisualizer* vis;
  for( GList* item = custom_visual_list; item; item = item->next ) {
    vis = static_cast< CustomVisualizer* >( item->data );
	if( world_gui->GetCanvas()->_custom_options[ vis->name() ]->isEnabled() )
		vis->DataVisualize( cam );
  }


  // and draw the children
  LISTMETHODARG( children, Model*, DataVisualizeTree, cam );

  PopCoords();
}

void Model::DrawGrid( void )
{
  if ( gui.grid ) 
    {
      PushLocalCoords();
		
      stg_bounds3d_t vol;
      vol.x.min = -geom.size.x/2.0;
      vol.x.max =  geom.size.x/2.0;
      vol.y.min = -geom.size.y/2.0;
      vol.y.max =  geom.size.y/2.0;
      vol.z.min = 0;
      vol.z.max = geom.size.z;
		 
      PushColor( 0,0,1,0.4 );
		Gl::draw_grid(vol);
      PopColor();		 
      PopCoords();
    }
}

inline bool velocity_is_zero( Velocity& v )
{
  return( !(v.x || v.y || v.z || v.a) );
}

void Model::SetVelocity( Velocity vel )
{
  //   printf( "SetVelocity %.2f %.2f %.2f %.2f\n", 
  // 			 vel.x,
  // 			 vel.y,
  // 			 vel.z,
  // 			 vel.a );

  assert( ! isnan(vel.x) );
  assert( ! isnan(vel.y) );
  assert( ! isnan(vel.z) );
  assert( ! isnan(vel.a) );

  this->velocity = vel;
  
  if( on_velocity_list && velocity_is_zero( vel ) ) 	 
    {
      world->StopUpdatingModelPose( this );
      on_velocity_list = false;
    }

  if( (!on_velocity_list) && (!velocity_is_zero( vel )) ) 	 
    {
      world->StartUpdatingModelPose( this );
      on_velocity_list = true;
    }

  CallCallbacks( &this->velocity );
}

void Model::NeedRedraw( void )
{
  this->rebuild_displaylist = true;

  if( parent )
    parent->NeedRedraw();
  else
    world->NeedRedraw();
}

void Model::SetPose( Pose newpose )
{
  //PRINT_DEBUG5( "%s.SetPose(%.2f %.2f %.2f %.2f)", 
  //	this->token, pose->x, pose->y, pose->z, pose->a );

  // if the pose has changed, we need to do some work
  if( memcmp( &pose, &newpose, sizeof(Pose) ) != 0 )
    {
      pose = newpose;
      pose.a = normalize(pose.a);

      if( isnan( pose.a ) )
	printf( "SetPose bad angle %s [%.2f %.2f %.2f %.2f]\n",
		token, pose.x, pose.y, pose.z, pose.a );
		
      NeedRedraw();
      map_caches_are_invalid = true;
      MapWithChildren();
      world->dirty = true;
    }

  // register a model change even if the pose didn't actually change
  CallCallbacks( &this->pose );
}

void Model::AddToPose( double dx, double dy, double dz, double da )
{
  if( dx || dy || dz || da )
    {
      Pose pose = GetPose();
      pose.x += dx;
      pose.y += dy;
      pose.z += dz;
      pose.a += da;

      SetPose( pose );
    }
}

void Model::AddToPose( Pose pose )
{
  this->AddToPose( pose.x, pose.y, pose.z, pose.a );
}

void Model::SetGeom( Geom geom )
{
  //printf( "MODEL \"%s\" SET GEOM (%.2f %.2f %.2f)[%.2f %.2f]\n",
  //  this->token,
  //  geom->pose.x, geom->pose.y, geom->pose.a,
  //  geom->size.x, geom->size.y );

  UnMapWithChildren();
  
  this->geom = geom;
  
  blockgroup.CalcSize();
  
  NeedRedraw();

  MapWithChildren();
  
  CallCallbacks( &geom );
}

void Model::SetColor( stg_color_t col )
{
  this->color = col;
  NeedRedraw();
  CallCallbacks( &color );
}

void Model::SetMass( stg_kg_t mass )
{
  this->mass = mass;
  CallCallbacks( &mass );
}

void Model::SetStall( stg_bool_t stall )
{
  this->stall = stall;
  CallCallbacks( &stall );
}

void Model::SetGripperReturn( int val )
{
  vis.gripper_return = val;
  CallCallbacks( &vis.gripper_return );
}

void Model::SetFiducialReturn(  int val )
{
  vis.fiducial_return = val;
  CallCallbacks( &vis.fiducial_return );
}

void Model::SetFiducialKey( int key )
{
  vis.fiducial_key = key;
  CallCallbacks( &vis.fiducial_key );
}

void Model::SetLaserReturn( stg_laser_return_t val )
{
  vis.laser_return = val;
  CallCallbacks( &vis.laser_return );
}

void Model::SetObstacleReturn( int val )
{
  vis.obstacle_return = val;
  CallCallbacks( &vis.obstacle_return );
}

void Model::SetBlobReturn( int val )
{
  vis.blob_return = val;
  CallCallbacks( &vis.blob_return );
}

void Model::SetRangerReturn( int val )
{
  vis.ranger_return = val;
  CallCallbacks( &vis.ranger_return );
}

void Model::SetBoundary( int val )
{
  boundary = val;
  CallCallbacks( &boundary );
}

void Model::SetGuiNose(  int val )
{
  gui.nose = val;
  CallCallbacks( &gui.nose );
}

void Model::SetGuiMask( int val )
{
  gui.mask = val;
  CallCallbacks( &gui.mask );
}

void Model::SetGuiGrid(  int val )
{
  gui.grid = val;
  CallCallbacks( &this->gui.grid );
}

void Model::SetGuiOutline( int val )
{
  gui.outline = val;
  CallCallbacks( &gui.outline );
}

void Model::SetWatts(  stg_watts_t watts )
{
  watts = watts;
  CallCallbacks( &watts );
}

void Model::SetMapResolution(  stg_meters_t res )
{
  map_resolution = res;
  CallCallbacks( &map_resolution );
}

// set the pose of model in global coordinates 
void Model::SetGlobalPose( Pose gpose )
{
  SetPose( parent ? parent->GlobalToLocal( gpose ) : gpose );

  //printf( "setting global pose %.2f %.2f %.2f = local pose %.2f %.2f %.2f\n",
  //      gpose->x, gpose->y, gpose->a, lpose.x, lpose.y, lpose.a );
}

int Model::SetParent(  Model* newparent)
{
  // remove the model from its old parent (if it has one)
  if( this->parent )
    this->parent->children = g_list_remove( this->parent->children, this );

  if( newparent )
    newparent->children = g_list_append( newparent->children, this );

  // link from the model to its new parent
  this->parent = newparent;

  CallCallbacks( &this->parent );
  return 0; //ok
}


void Model::PlaceInFreeSpace( stg_meters_t xmin, stg_meters_t xmax, 
			      stg_meters_t ymin, stg_meters_t ymax )
{
  while( TestCollision() )
    SetPose( Pose::Random( xmin,xmax, ymin, ymax ));		
}


Model* Model::TestCollision()
{
  //printf( "mod %s test collision...\n", token );

  Model* hitmod = blockgroup.TestCollision();
  
  if( hitmod == NULL ) 
    for( GList* it = children; it; it=it->next ) 
      { 
	hitmod = ((Model*)it->data)->TestCollision();
	if( hitmod )
	  break;
      }
  
  //printf( "mod %s test collision done.\n", token );
  return hitmod;  
}

void Model::CommitTestedPose()
{
  for( GList* it = children; it; it=it->next ) 
    ((Model*)it->data)->CommitTestedPose();
  
  blockgroup.SwitchToTestedCells();
}
  
Model* Model::ConditionalMove( Pose newpose )
{ 
  if( isnan( pose.x ) || isnan( pose.y )  || isnan( pose.z )  || isnan( pose.a ) )
    printf( "ConditionalMove bad newpose %s [%.2f %.2f %.2f %.2f]\n",
	    token, newpose.x, newpose.y, newpose.z, newpose.a );

  Pose startpose = pose;
  pose = newpose; // do the move provisionally - we might undo it below
   
  Model* hitmod = TestCollision();
 
  if( hitmod )
    pose = startpose; // move failed - put me back where I started
  else
    {
      CommitTestedPose(); // shift anyrecursively commit to blocks to the new pose 
      world->dirty = true; // need redraw
    }

  
  if( isnan( pose.x ) || isnan( pose.y )  || isnan( pose.z )  || isnan( pose.a ) )
    printf( "ConditionalMove bad pose %s [%.2f %.2f %.2f %.2f]\n",
	    token, pose.x, pose.y, pose.z, pose.a );

  return hitmod;
}

void Model::UpdatePose( void )
{
  if( disabled )
    return;

  if( velocity.x == 0 && velocity.y == 0 && velocity.a == 0 && velocity.z == 0 )
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
  p.a = velocity.a * interval;
    
  if( isnan( p.x ) || isnan( p.y )  || isnan( p.z )  || isnan( p.a ) )
    printf( "UpdatePose bad vel %s [%.2f %.2f %.2f %.2f]\n",
	    token, p.x, p.y, p.z, p.a );

  // attempts to move to the new pose. If the move fails because we'd
  // hit another model, that model is returned.
  Model* hitthing = ConditionalMove( pose_sum( pose, p ) );

  SetStall( hitthing ? true : false );
}


int Model::TreeToPtrArray( GPtrArray* array )
{
  g_ptr_array_add( array, this );

  //printf( " added %s to array at %p\n", root->token, array );

  int added = 1;

  for(GList* it=children; it; it=it->next )
    added += ((Model*)it->data)->TreeToPtrArray( array );

  return added;
}

Model* Model::GetUnsubscribedModelOfType( stg_model_type_t type )
{
  printf( "searching for type %d in model %s type %d\n", type, token, this->type );
  
  if( (this->type == type) && (this->subs == 0) )
    return this;

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

Model* Model::GetUnusedModelOfType( stg_model_type_t type )
{
  printf( "searching for type %d in model %s type %d\n", type, token, this->type );

  if( (this->type == type) && (!this->used ) )
    {
      this->used = true;
      return this;
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


Model* Model::GetModel( const char* modelname )
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
