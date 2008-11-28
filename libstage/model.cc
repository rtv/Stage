/** @defgroup model

The basic model simulates an object with basic properties; position,
size, velocity, color, visibility to various sensors, etc. The basic
model also has a body made up of a list of lines. Internally, the
basic model is used as the base class for all other model types. You
can use the basic model to simulate environmental objects.

API: Stg::StgModel 

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

  blocks 0
  block[0].points 0
  block[0].point[0] [ 0.0 0.0 ]
  block[0].z [ 0.0 1.0 ]
  block[0].color "<color>"

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

//#define DEBUG 0
#include "stage_internal.hh"
#include "texture_manager.hh"
//#include <limits.h> 


//static const members
static const bool DEFAULT_BOUNDARY = false;
static const stg_color_t DEFAULT_COLOR = (0xFFFF0000); // solid red
static const stg_joules_t DEFAULT_ENERGY_CAPACITY = 1000.0;
static const bool DEFAULT_ENERGY_CHARGEENABLE = true;
static const stg_watts_t DEFAULT_ENERGY_GIVERATE =  0.0;
static const stg_meters_t DEFAULT_ENERGY_PROBERANGE = 0.0;
static const stg_watts_t DEFAULT_ENERGY_TRICKLERATE = 0.1;
static const bool DEFAULT_GRID = false;
static const bool DEFAULT_GRIPPERRETURN = false;
static const stg_laser_return_t DEFAULT_LASERRETURN = LaserVisible;
static const stg_meters_t DEFAULT_MAP_RESOLUTION = 0.1;
static const stg_movemask_t DEFAULT_MASK = (STG_MOVE_TRANS | STG_MOVE_ROT);
static const stg_kg_t DEFAULT_MASS = 10.0;
static const bool DEFAULT_NOSE = false;
static const bool DEFAULT_OBSTACLERETURN = true;
static const bool DEFAULT_BLOBRETURN = true;
static const bool DEFAULT_OUTLINE = true;
static const bool DEFAULT_RANGERRETURN = true;

// speech bubble colors
static const stg_color_t BUBBLE_FILL = 0xFFC8C8FF; // light blue/grey
static const stg_color_t BUBBLE_BORDER = 0xFF000000; // black
static const stg_color_t BUBBLE_TEXT = 0xFF000000; // black

// static members
uint32_t StgModel::count = 0;
GHashTable* StgModel::modelsbyid = g_hash_table_new( NULL, NULL );


// constructor
StgModel::StgModel( StgWorld* world,
						  StgModel* parent,
						  const stg_model_type_t type )
  : StgAncestor(), 
	 world(world),
	 parent(parent),
	 type(type),
    id( StgModel::count++ ),
	 trail( g_array_new( false, false, sizeof(stg_trail_item_t) )),
	 blocks_dl(0),
	 data_fresh(false),
	 disabled(false),
	 rebuild_displaylist(true),
	 say_string(NULL),
	 subs(0),
	 used(false),
	 stall(false),	 
	 obstacle_return(DEFAULT_OBSTACLERETURN),
	 ranger_return(DEFAULT_RANGERRETURN),
	 blob_return(DEFAULT_BLOBRETURN),
	 laser_return(DEFAULT_LASERRETURN),
	 gripper_return(DEFAULT_GRIPPERRETURN),
	 fiducial_return(0),
	 fiducial_key(0),
	 boundary(DEFAULT_BOUNDARY),
	 color(DEFAULT_COLOR),
	 map_resolution(DEFAULT_MAP_RESOLUTION),
	 gui_nose(DEFAULT_NOSE),
	 gui_grid(DEFAULT_GRID),
	 gui_outline(DEFAULT_OUTLINE),
	 gui_mask( parent ? 0 : DEFAULT_MASK),
	 callbacks( g_hash_table_new( g_int_hash, g_int_equal ) ),
	 flag_list(NULL),
	 blinkenlights( g_ptr_array_new() ),
	 last_update(0),
	 interval((stg_usec_t)1e4), // 10msec
	 initfunc(NULL),
	 wf(NULL),
	 on_velocity_list( false ),
	 on_update_list( false ),
	 wf_entity(0),
	 has_default_block( true ),
	 map_caches_are_invalid( true ),
	 thread_safe( false )
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
    world->AddChild( this );
  
  world->AddModel( this );
  
  g_datalist_init( &this->props );
  
  // now we can add the basic square shape
  AddBlockRect( -0.5, -0.5, 1.0, 1.0, 1.0 );

  PRINT_DEBUG2( "finished model %s @ %p", 
					 this->token, this );
}

StgModel::~StgModel( void )
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

  g_hash_table_remove( StgModel::modelsbyid, (void*)id );

  world->RemoveModel( this );
}

void StgModel::StartUpdating()
{
  if( ! on_update_list )
	 {
		on_update_list = true;
		world->StartUpdatingModel( this );
	 }
}

void StgModel::StopUpdating()
{
  on_update_list = false;
  world->StopUpdatingModel( this );
}

// this should be called after all models have loaded from the
// worldfile - it's a chance to do any setup now that all models are
// in existence
void StgModel::Init()
{
  // init is called after the model is loaded
  blockgroup.CalcSize();

  UnMap(); // remove any old cruft rendered during startup
  map_caches_are_invalid = true;
  Map();

  if( initfunc )
    Subscribe();
}  

void StgModel::InitRecursive()
{
  // init children first
  LISTMETHOD( children, StgModel*, InitRecursive );
  Init();
}  

void StgModel::AddFlag( StgFlag* flag )
{
  if( flag )
    flag_list = g_list_append( this->flag_list, flag );
}

void StgModel::RemoveFlag( StgFlag* flag )
{
  if( flag )
    flag_list = g_list_remove( this->flag_list, flag );
}

void StgModel::PushFlag( StgFlag* flag )
{
  if( flag )
    flag_list = g_list_prepend( flag_list, flag);
}

StgFlag* StgModel::PopFlag()
{
  if( flag_list == NULL )
    return NULL;

  StgFlag* flag = (StgFlag*)flag_list->data;
  flag_list = g_list_remove( flag_list, flag );

  return flag;
}


void StgModel::ClearBlocks( void )
{
  UnMap();
  blockgroup.Clear();
  map_caches_are_invalid = true;

  //no need to Map() -  we have no blocks
  NeedRedraw();
}

void StgModel::LoadBlock( Worldfile* wf, int entity )
{
  if( has_default_block )
	 {
		blockgroup.Clear(); 
		has_default_block = false;
	 }
  
  blockgroup.LoadBlock( this, wf, entity );  
}


void StgModel::AddBlockRect( stg_meters_t x, 
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
  
  blockgroup.AppendBlock( new StgBlock( this,
													 pts, 4, 
													 0, dz, 
													 color,
													 true ) );
}


stg_raytrace_result_t StgModel::Raytrace( const stg_pose_t &pose,
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

stg_raytrace_result_t StgModel::Raytrace( const stg_radians_t bearing,
														const stg_meters_t range, 
														const stg_ray_test_func_t func,
														const void* arg,
														const bool ztest )
{
  stg_pose_t raystart;
  bzero( &raystart, sizeof(raystart));
  raystart.a = bearing;

  return world->Raytrace( LocalToGlobal(raystart),
								  range,
								  func,
								  this,
								  arg,
								  ztest );
}


void StgModel::Raytrace( const stg_radians_t bearing,
								 const stg_meters_t range, 
								 const stg_radians_t fov,
								 const stg_ray_test_func_t func,
								 const void* arg,
								 stg_raytrace_result_t* samples,
								 const uint32_t sample_count,
								 const bool ztest )
{
  stg_pose_t raystart;
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

// // convert a global pose into the model's local coordinate system
// void StgModel::GlobalToLocal( stg_pose_t* pose )
// {
//   // get model's global pose
//   stg_pose_t org = GetGlobalPose();

//   //printf( "g2l global origin %.2f %.2f %.2f\n",
//   //  org.x, org.y, org.a );

//   // compute global pose in local coords
//   double sx =  (pose->x - org.x) * cos(org.a) + (pose->y - org.y) * sin(org.a);
//   double sy = -(pose->x - org.x) * sin(org.a) + (pose->y - org.y) * cos(org.a);
//   double sa = pose->a - org.a;

//   pose->x = sx;
//   pose->y = sy;
//   pose->a = sa;
// }

// convert a global pose into the model's local coordinate system
stg_pose_t StgModel::GlobalToLocal( stg_pose_t pose )
{
  // get model's global pose
  stg_pose_t org = GetGlobalPose();
  
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


void StgModel::Say( const char* str )
{
  if( say_string )
    free( say_string );
  say_string = strdup( str );
}

// returns true iff model [testmod] is an antecedent of this model
bool StgModel::IsAntecedent( StgModel* testmod )
{
  if( parent == NULL )
	 return false;
  
  if( parent == testmod )
	 return true;
  
  return parent->IsAntecedent( testmod );
}

// returns true iff model [testmod] is a descendent of this model
bool StgModel::IsDescendent( StgModel* testmod )
{
  for( GList* it=this->children; it; it=it->next )
    {
      StgModel* child = (StgModel*)it->data;

		if( child == testmod )
		  return true;
		
		if( child->IsDescendent( testmod ) )
		  return true;
    }

  // neither mod nor a child of this matches testmod
  return false;
}

// returns true iff model [mod1] and [mod2] are in the same model tree
bool StgModel::IsRelated( StgModel* mod2 )
{
  return( (this == mod2) || IsAntecedent( mod2 ) || IsDescendent( mod2 ) );
}

// get the model's velocity in the global frame
stg_velocity_t StgModel::GetGlobalVelocity()
{
  stg_pose_t gpose = GetGlobalPose();

  double cosa = cos( gpose.a );
  double sina = sin( gpose.a );

  stg_velocity_t gv;
  gv.x = velocity.x * cosa - velocity.y * sina;
  gv.y = velocity.x * sina + velocity.y * cosa;
  gv.a = velocity.a;

  //printf( "local velocity %.2f %.2f %.2f\nglobal velocity %.2f %.2f %.2f\n",
  //  mod->velocity.x, mod->velocity.y, mod->velocity.a,
  //  gvel->x, gvel->y, gvel->a );

  return gv;
}

// set the model's velocity in the global frame
void StgModel::SetGlobalVelocity( stg_velocity_t gv )
{
  stg_pose_t gpose = GetGlobalPose();

  double cosa = cos( gpose.a );
  double sina = sin( gpose.a );

  stg_velocity_t lv;
  lv.x = gv.x * cosa + gv.y * sina;
  lv.y = -gv.x * sina + gv.y * cosa;
  lv.a = gv.a;

  this->SetVelocity( lv );
}

// get the model's position in the global frame
stg_pose_t StgModel::GetGlobalPose()
{ 
  // if I'm a top level model, my global pose is my local pose
  if( parent == NULL )
	 return pose;
  
  // otherwise  
  
  stg_pose_t global_pose = pose_sum( parent->GetGlobalPose(), pose );		
  
  // we are on top of our parent
  global_pose.z += parent->geom.size.z;
  
  //   PRINT_DEBUG4( "GET GLOBAL POSE [x:%.2f y:%.2f z:%.2f a:%.2f]",
  // 		global_pose.x,
  // 		global_pose.y,
  // 		global_pose.z,
  // 		global_pose.a );

  return global_pose;
}


// convert a pose in this model's local coordinates into global
// coordinates
// should one day do all this with affine transforms for neatness?
inline stg_pose_t StgModel::LocalToGlobal( stg_pose_t pose )
{  
  return pose_sum( pose_sum( GetGlobalPose(), geom.pose ), pose );
}

void StgModel::MapWithChildren()
{
  UnMap();
  Map();

  // recursive call for all the model's children
  for( GList* it=children; it; it=it->next )
	 ((StgModel*)it->data)->MapWithChildren();
}

void StgModel::UnMapWithChildren()
{
  UnMap();

  // recursive call for all the model's children
  for( GList* it=children; it; it=it->next )
    ((StgModel*)it->data)->UnMapWithChildren();
}

void StgModel::Map()
{
  //PRINT_DEBUG1( "%s.Map()", token );

  // render all blocks in the group at my global pose and size
  blockgroup.Map();
  map_caches_are_invalid = false;
} 


void StgModel::Subscribe( void )
{
  subs++;
  world->total_subs++;

  //printf( "subscribe to %s %d\n", token, subs );

  // if this is the first sub, call startup
  if( this->subs == 1 )
    this->Startup();
}

void StgModel::Unsubscribe( void )
{
  subs--;
  world->total_subs--;

  printf( "unsubscribe from %s %d\n", token, subs );

  // if this is the last sub, call shutdown
  if( subs < 1 )
    this->Shutdown();
}


void pose_invert( stg_pose_t* pose )
{
  pose->x = -pose->x;
  pose->y = -pose->y;
  pose->a = pose->a;
}

void StgModel::Print( char* prefix )
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
    ((StgModel*)it->data)->Print( prefix );
}

const char* StgModel::PrintWithPose()
{
  stg_pose_t gpose = GetGlobalPose();

  static char txt[256];
  snprintf(txt, sizeof(txt), "%s @ [%.2f,%.2f,%.2f,%.2f]",  
			  token, 
			  gpose.x, gpose.y, gpose.z, gpose.a  );

  return txt;
}

void StgModel::Startup( void )
{
  //printf( "Startup model %s\n", this->token );

  // TODO:  this could be a callback
  if( initfunc )
    initfunc( this );

  StartUpdating();

  CallCallbacks( &startup_hook );
}

void StgModel::Shutdown( void )
{
  //printf( "Shutdown model %s\n", this->token );

  StopUpdating();

  CallCallbacks( &shutdown_hook );
}

void StgModel::UpdateIfDue( void )
{
  if( UpdateDue() )
	 Update();
}
  
bool StgModel::UpdateDue( void )
{
  return( world->sim_time  >= (last_update + interval) );
}
 
void StgModel::Update( void )
{
  //   printf( "[%llu] %s update (%d subs)\n", 
  // 			 this->world->sim_time, this->token, this->subs );

  CallCallbacks( &update_hook );
  last_update = world->sim_time;
}

void StgModel::DrawSelected()
{
  glPushMatrix();
  
  glTranslatef( pose.x, pose.y, pose.z+0.01 ); // tiny Z offset raises rect above grid
  
  stg_pose_t gpose = GetGlobalPose();
  
  char buf[64];
  snprintf( buf, 63, "%s [%.2f %.2f %.2f %.2f]", 
				token, gpose.x, gpose.y, gpose.z, rtod(gpose.a) );
  
  PushColor( 0,0,0,1 ); // text color black
  gl_draw_string( 0.5,0.5,0.5, buf );
  
  glRotatef( rtod(pose.a), 0,0,1 );
  
  gl_pose_shift( geom.pose );
  
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


void StgModel::DrawTrailFootprint()
{
  double r,g,b,a;

  for( int i=trail->len-1; i>=0; i-- )
    {
      stg_trail_item_t* checkpoint = & g_array_index( trail, stg_trail_item_t, i );

      glPushMatrix();
      gl_pose_shift( checkpoint->pose );
      gl_pose_shift( geom.pose );

      stg_color_unpack( checkpoint->color, &r, &g, &b, &a );
      PushColor( r, g, b, 0.1 );

		blockgroup.DrawFootPrint();

      glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
      PushColor( r/2, g/2, b/2, 0.1 );

		blockgroup.DrawFootPrint();

      PopColor();
      PopColor();
	  glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
      glPopMatrix();
    }
}

void StgModel::DrawTrailBlocks()
{
  double timescale = 0.0000001;

  for( int i=trail->len-1; i>=0; i-- )
    {
      stg_trail_item_t* checkpoint = & g_array_index( trail, stg_trail_item_t, i );

      stg_pose_t pose;
      memcpy( &pose, &checkpoint->pose, sizeof(pose));
      pose.z =  (world->sim_time - checkpoint->time) * timescale;

      PushLocalCoords();
      //blockgroup.Draw( this );

		DrawBlocksTree();
      PopCoords();
    }
}

void StgModel::DrawTrailArrows()
{
  double r,g,b,a;

  double dx = 0.2;
  double dy = 0.07;
  double timescale = 0.0000001;

  for( unsigned int i=0; i<trail->len; i++ )
    {
      stg_trail_item_t* checkpoint = & g_array_index( trail, stg_trail_item_t, i );

      stg_pose_t pose;
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

void StgModel::DrawOriginTree()
{
  DrawPose( GetGlobalPose() );  
  for( GList* it=children; it; it=it->next )
	 ((StgModel*)it->data)->DrawOriginTree();
}
 

void StgModel::DrawBlocksTree( )
{
  PushLocalCoords();
  LISTMETHOD( children, StgModel*, DrawBlocksTree );
  DrawBlocks();  
  PopCoords();
}
  
void StgModel::DrawPose( stg_pose_t pose )
{
  PushColor( 0,0,0,1 );
  glPointSize( 4 );
  
  glBegin( GL_POINTS );
  glVertex3f( pose.x, pose.y, pose.z );
  glEnd();
  
  PopColor();  
}

void StgModel::DrawBlocks( )
{ 
  gl_pose_shift( geom.pose );
  blockgroup.CallDisplayList( this );
}

void StgModel::DrawBoundingBoxTree()
{
  PushLocalCoords();
  LISTMETHOD( children, StgModel*, DrawBoundingBoxTree );
  DrawBoundingBox();
  PopCoords();
}

void StgModel::DrawBoundingBox()
{
  gl_pose_shift( geom.pose );  

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
void StgModel::PushLocalCoords()
{
  glPushMatrix();  
  
  if( parent )
	 glTranslatef( 0,0, parent->geom.size.z );
  
  gl_pose_shift( pose );
}

void StgModel::PopCoords()
{
  glPopMatrix();
}

void StgModel::DrawStatusTree( Camera* cam ) 
{
  PushLocalCoords();
  DrawStatus( cam );
  LISTMETHODARG( children, StgModel*, DrawStatusTree, cam );  
  PopCoords();
}

void StgModel::DrawStatus( Camera* cam ) 
{
  if( say_string )	  
	 {
		float yaw, pitch;
		pitch = - cam->pitch();
		yaw = - cam->yaw();			
		
		float robotAngle = -rtod(pose.a);
		glPushMatrix();
		
		fl_font( FL_HELVETICA, 12 );
		float w = gl_width( this->say_string ); // scaled text width
		float h = gl_height(); // scaled text height
		
		// move above the robot
		glTranslatef( 0, 0, 0.5 );		
		
		// rotate to face screen
		glRotatef( robotAngle - yaw, 0,0,1 );
		glRotatef( -pitch, 1,0,0 );
		
		//get raster positition, add gl_width, then project back to world coords
		glRasterPos3f( 0, 0, 0 );
		GLfloat pos[ 4 ];
		glGetFloatv(GL_CURRENT_RASTER_POSITION, pos);
		
		GLboolean valid;
		glGetBooleanv( GL_CURRENT_RASTER_POSITION_VALID, &valid );
		if( valid ) 
		  {
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
			 gl_draw_octagon( w, h, m );
			 glDisable( GL_POLYGON_OFFSET_FILL );
			 PopColor();
			 
			 // draw outline of bubble
			 PushColor( BUBBLE_BORDER );
			 glLineWidth( 1 );
			 glEnable( GL_LINE_SMOOTH );
			 glPolygonMode( GL_FRONT, GL_LINE );
			 gl_draw_octagon( w, h, m );
			 glPopAttrib();
			 PopColor();
			 
			 PushColor( BUBBLE_TEXT );
			 // draw text inside the bubble
			 gl_draw_string( 2.5*m, 2.5*m, 0, this->say_string );
			 PopColor();			
		  }
		glPopMatrix();
	 }
  
  if( stall )
	 {
		DrawImage( TextureManager::getInstance()._stall_texture_id, cam, 0.85 );
	 }
}

stg_meters_t StgModel::ModelHeight()
{	
	stg_meters_t m_child = 0; //max size of any child
	for( GList* it=this->children; it; it=it->next )
    {
		StgModel* child = (StgModel*)it->data;
		stg_meters_t tmp_h = child->ModelHeight();
		if( tmp_h > m_child )
			m_child = tmp_h;
    }
	
	//height of model + max( child height )
	return geom.size.z + m_child;
}

void StgModel::DrawImage( uint32_t texture_id, Camera* cam, float alpha )
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

void StgModel::DrawFlagList( void )
{	
  if( flag_list  == NULL )
	 return;
  
  PushLocalCoords();
  
  GLUquadric* quadric = gluNewQuadric();
  glTranslatef(0,0,1); // jump up
  stg_pose_t gpose = GetGlobalPose();
  glRotatef( 180 + rtod(-gpose.a),0,0,1 );
  

  GList* list = g_list_copy( flag_list );
  list = g_list_reverse(list);
  
  for( GList* item = list; item; item = item->next )
	 {
		
		StgFlag* flag = (StgFlag*)item->data;
		
		glTranslatef( 0, 0, flag->size/2.0 );
				
		PushColor( flag->color );
		
		gluQuadricDrawStyle( quadric, GLU_FILL );
		gluSphere( quadric, flag->size/2.0, 4,2  );
		
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


void StgModel::DrawBlinkenlights()
{
  PushLocalCoords();

  GLUquadric* quadric = gluNewQuadric();
  //glTranslatef(0,0,1); // jump up
  //stg_pose_t gpose = GetGlobalPose();
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

void StgModel::DrawPicker( void )
{
  //PRINT_DEBUG1( "Drawing %s", token );
  PushLocalCoords();

  // draw the boxes
  blockgroup.DrawSolid();

  // recursively draw the tree below this model 
  LISTMETHOD( this->children, StgModel*, DrawPicker );

  PopCoords();
}

void StgModel::DataVisualize( Camera* cam )
{  
  // do nothing
}

void StgModel::DataVisualizeTree( Camera* cam )
{
  PushLocalCoords();
  DataVisualize( cam ); // virtual function overridden by most model types  

  // and draw the children
  LISTMETHODARG( children, StgModel*, DataVisualizeTree, cam );

  PopCoords();
}

void StgModel::DrawGrid( void )
{
  if ( gui_grid ) 
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
      gl_draw_grid(vol);
      PopColor();		 
      PopCoords();
    }
}

inline bool velocity_is_zero( stg_velocity_t& v )
{
  return( !(v.x || v.y || v.z || v.a) );
}

void StgModel::SetVelocity( stg_velocity_t vel )
{
//   printf( "SetVelocity %.2f %.2f %.2f %.2f\n", 
// 			 vel.x,
// 			 vel.y,
// 			 vel.z,
// 			 vel.a );

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

void StgModel::NeedRedraw( void )
{
  this->rebuild_displaylist = true;

  if( parent )
    parent->NeedRedraw();
  else
	 world->NeedRedraw();
}

void StgModel::SetPose( stg_pose_t pose )
{
  //PRINT_DEBUG5( "%s.SetPose(%.2f %.2f %.2f %.2f)", 
  //	this->token, pose->x, pose->y, pose->z, pose->a );

  // if the pose has changed, we need to do some work
  if( memcmp( &this->pose, &pose, sizeof(stg_pose_t) ) != 0 )
    {
		//puts( "SETPOSE" );
	  
		//      UnMapWithChildren();


      pose.a = normalize( pose.a );
      this->pose = pose;
      this->pose.a = normalize(this->pose.a);

      this->NeedRedraw();

		this->map_caches_are_invalid = true;
      MapWithChildren();

		world->dirty = true;
    }

  // register a model change even if the pose didn't actually change
  CallCallbacks( &this->pose );
}

void StgModel::AddToPose( double dx, double dy, double dz, double da )
{
  if( dx || dy || dz || da )
    {
      stg_pose_t pose = GetPose();
      pose.x += dx;
      pose.y += dy;
      pose.z += dz;
      pose.a += da;

      SetPose( pose );
    }
}

void StgModel::AddToPose( stg_pose_t pose )
{
  this->AddToPose( pose.x, pose.y, pose.z, pose.a );
}

void StgModel::SetGeom( stg_geom_t geom )
{
  //printf( "MODEL \"%s\" SET GEOM (%.2f %.2f %.2f)[%.2f %.2f]\n",
  //  this->token,
  //  geom->pose.x, geom->pose.y, geom->pose.a,
  //  geom->size.x, geom->size.y );

  UnMapWithChildren();
  
  this->geom = geom;
  
  //blockgroup.CalcSize();
  
  NeedRedraw();

  MapWithChildren();

  CallCallbacks( &this->geom );
}

void StgModel::SetColor( stg_color_t col )
{
  this->color = col;
  NeedRedraw();
  CallCallbacks( &this->color );
}

void StgModel::SetMass( stg_kg_t mass )
{
  this->mass = mass;
  CallCallbacks( &this->mass );
}

void StgModel::SetStall( stg_bool_t stall )
{
  this->stall = stall;
  CallCallbacks( &this->stall );
}

void StgModel::SetGripperReturn( int val )
{
  this->gripper_return = val;
  CallCallbacks( &this->gripper_return );
}

void StgModel::SetFiducialReturn(  int val )
{
  this->fiducial_return = val;
  CallCallbacks( &this->fiducial_return );
}

void StgModel::SetFiducialKey( int key )
{
  this->fiducial_key = key;
  CallCallbacks( &this->fiducial_key );
}

void StgModel::SetLaserReturn( stg_laser_return_t val )
{
  this->laser_return = val;
  CallCallbacks( &this->laser_return );
}

void StgModel::SetObstacleReturn( int val )
{
  this->obstacle_return = val;
  CallCallbacks( &this->obstacle_return );
}

void StgModel::SetBlobReturn( int val )
{
  this->blob_return = val;
  CallCallbacks( &this->blob_return );
}

void StgModel::SetRangerReturn( int val )
{
  this->ranger_return = val;
  CallCallbacks( &this->ranger_return );
}

void StgModel::SetBoundary( int val )
{
  this->boundary = val;
  CallCallbacks( &this->boundary );
}

void StgModel::SetGuiNose(  int val )
{
  this->gui_nose = val;
  CallCallbacks( &this->gui_nose );
}

void StgModel::SetGuiMask( int val )
{
  this->gui_mask = val;
  CallCallbacks( &this->gui_mask );
}

void StgModel::SetGuiGrid(  int val )
{
  this->gui_grid = val;
  CallCallbacks( &this->gui_grid );
}

void StgModel::SetGuiOutline( int val )
{
  this->gui_outline = val;
  CallCallbacks( &this->gui_outline );
}

void StgModel::SetWatts(  stg_watts_t watts )
{
  this->watts = watts;
  CallCallbacks( &this->watts );
}

void StgModel::SetMapResolution(  stg_meters_t res )
{
  this->map_resolution = res;
  CallCallbacks( &this->map_resolution );
}

// set the pose of model in global coordinates 
void StgModel::SetGlobalPose( stg_pose_t gpose )
{
  SetPose( parent ? parent->GlobalToLocal( gpose ) : gpose );

  //printf( "setting global pose %.2f %.2f %.2f = local pose %.2f %.2f %.2f\n",
  //      gpose->x, gpose->y, gpose->a, lpose.x, lpose.y, lpose.a );
}

int StgModel::SetParent(  StgModel* newparent)
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


void StgModel::PlaceInFreeSpace( stg_meters_t xmin, stg_meters_t xmax, 
											stg_meters_t ymin, stg_meters_t ymax )
{
  while( TestCollision() )
    SetPose( stg_pose_t::Random( xmin,xmax, ymin, ymax ));		
}


StgModel* StgModel::TestCollision()
{
  //printf( "mod %s test collision...\n", token );

  StgModel* hitmod = blockgroup.TestCollision();
  
  if( hitmod == NULL ) 
    for( GList* it = children; it; it=it->next ) 
		{ 
		  hitmod = ((StgModel*)it->data)->TestCollision();
		  if( hitmod )
			 break;
		}
  
  //printf( "mod %s test collision done.\n", token );
  return hitmod;  
}

void StgModel::CommitTestedPose()
{
  for( GList* it = children; it; it=it->next ) 
	 ((StgModel*)it->data)->CommitTestedPose();
  
  blockgroup.SwitchToTestedCells();
}
  
StgModel* StgModel::ConditionalMove( stg_pose_t newpose )
{ 
  stg_pose_t startpose = pose;
  pose = newpose; // do the move provisionally - we might undo it below
   
  StgModel* hitmod = TestCollision();
 
  if( hitmod )
	 pose = startpose; // move failed - put me back where I started
  else
	 {
		CommitTestedPose(); // shift anyrecursively commit to blocks to the new pose 
		world->dirty = true; // need redraw
	 }

  return hitmod;
}

void StgModel::UpdatePose( void )
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
  stg_pose_t p;
  p.x = velocity.x * interval;
  p.y = velocity.y * interval;
  p.z = velocity.z * interval;
  p.a = velocity.a * interval;
    
  // attempts to move to the new pose. If the move fails because we'd
  // hit another model, that model is returned.
  StgModel* hitthing = ConditionalMove( pose_sum( pose, p ) );

  SetStall( hitthing ? true : false );
}


int StgModel::TreeToPtrArray( GPtrArray* array )
{
  g_ptr_array_add( array, this );

  //printf( " added %s to array at %p\n", root->token, array );

  int added = 1;

  for(GList* it=children; it; it=it->next )
    added += ((StgModel*)it->data)->TreeToPtrArray( array );

  return added;
}

StgModel* StgModel::GetUnsubscribedModelOfType( stg_model_type_t type )
{
  printf( "searching for type %d in model %s type %d\n", type, token, this->type );
  
  if( (this->type == type) && (this->subs == 0) )
    return this;

  // this model is no use. try children recursively
  for( GList* it = children; it; it=it->next )
    {
      StgModel* child = (StgModel*)it->data;

      StgModel* found = child->GetUnsubscribedModelOfType( type );
      if( found )
		  return found;
    }

  // nothing matching below this model
  return NULL;
}

StgModel* StgModel::GetUnusedModelOfType( stg_model_type_t type )
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
      StgModel* child = (StgModel*)it->data;

      StgModel* found = child->GetUnusedModelOfType( type );
      if( found )
		  return found;
    }

  // nothing matching below this model
  return NULL;
}


StgModel* StgModel::GetModel( const char* modelname )
{
  // construct the full model name and look it up
  char* buf = new char[TOKEN_MAX];
  snprintf( buf, TOKEN_MAX, "%s.%s", this->token, modelname );

  StgModel* mod = world->GetModel( buf );

  if( mod == NULL )
    PRINT_WARN1( "Model %s not found", buf );

  delete[] buf;

  return mod;
}

void StgModel::UnMap()
{
  blockgroup.UnMap();
}
