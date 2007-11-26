//extern GList* dl_list;

/** @ingroup stage 
    @{ 
*/

/** @defgroup model Model
    
The basic model simulates an object with basic properties; position,
size, velocity, color, visibility to various sensors, etc. The basic
model also has a body made up of a list of lines. Internally, the
basic model is used base class for all other model types. You can use
the basic model to simulate environmental objects.

<h2>Worldfile properties</h2>

@par Summary and default values

@verbatim
model
(
  pose [0 0 0]
  size [1.0 1.0]
  origin [0 0 0]
  velocity [0 0 0]

  # body color
  color "red" 

  # determine how the model appears in various sensors

  obstacle_return 1
  laser_return 1
  ranger_return 1
  blobfinder_return 1
  fiducial_return 1
  gripper_return 0

  fiducial_key 0

  # GUI properties
  gui_nose 0
  gui_grid 0
  gui_boundary 0
  gui_movemask ?

  bitmap ""
)
@endverbatim

@par Details
- pose [x_pos:float y_pos:float heading:float]
  - specify the pose of the model in its parent's coordinate system
- size [x_size:float ysize:float]
  - specify the size of the model
- origin [x_pos:float y_pos:float heading:float]
  - specify the position of the object's center, relative to its pose
- velocity [x_speed:float y_speed:float rotation_speed:float]
  - specify the initial velocity of the model. Not that if the model hits an obstacle, its velocity will be set to zero.
- color [colorname:string]
  - specify the color of the object using a color name from the X11 database (rgb.txt)
- line_count [int]
  - specify the number of lines that make up the model's body
- line[index] [x1:float y1:float x2:float y2:float]
  - creates a line from (x1,y1) to (x2,y2). A set of line_count lines defines the robot's body for the purposes of collision detection and rendering in the GUI window.
- bitmap [filename:string}
  - alternative way to set the model's line_count and lines. The file must be a bitmap recognized by libgtkpixbuf (most popular formats are supported). The file is opened and parsed into a set of lines. Unless the bitmap_resolution option is used, the lines are scaled to fit inside the rectangle defined by the model's current size.
- gui_nose [bool]
  - if 1, draw a nose on the model showing its heading (positive X axis)
- gui_grid [bool]
  - if 1, draw a scaling grid over the model
- gui_movemask [int]
  - define how the model can be moved by the mouse in the GUI window
- gui_boundary [bool]
  - if 1, draw a bounding box around the model, indicating its size
- obstacle_return [bool]
  - if 1, this model can collide with other models that have this property set
- blob_return [bool]
  - if 1, this model can be detected in the blob_finder (depending on its color)
- ranger_return [bool]
  - if 1, this model can be detected by ranger sensors
- laser_return [int]
  - if 0, this model is not detected by laser sensors. if 1, the model shows up in a laser sensor with normal (0) reflectance. If 2, it shows up with high (1) reflectance.
- fiducial_return [fiducial_id:int]
  - if non-zero, this model is detected by fiducialfinder sensors. The value is used as the fiducial ID.
- fiducial_key [int]
  - models are only detected by fiducialfinders if the fiducial_key values of model and fiducialfinder match. This allows you to have several independent types of fiducial in the same environment, each type only showing up in fiducialfinders that are "tuned" for it.
- ranger_return [bool]
   - iff 1, this model can be detected by a ranger.
- gripper_return [bool]
   - iff 1, this model can be gripped by a gripper and can be pushed around by collisions with anything that has a non-zero obstacle_return.

*/

/** @} */  

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
  
#define _GNU_SOURCE

#include <limits.h> 
//#include <assert.h>
//#include <math.h>
//#include <GL/gl.h>
//#include <glib.h>

//#define DEBUG
#include "stage.hh"

// basic model
#define STG_DEFAULT_BLOBRETURN true
#define STG_DEFAULT_BOUNDARY true
#define STG_DEFAULT_COLOR (0xFF0000) // red
#define STG_DEFAULT_ENERGY_CAPACITY 1000.0
#define STG_DEFAULT_ENERGY_CHARGEENABLE 1
#define STG_DEFAULT_ENERGY_GIVERATE 0.0
#define STG_DEFAULT_ENERGY_PROBERANGE 0.0
#define STG_DEFAULT_ENERGY_TRICKLERATE 0.1
#define STG_DEFAULT_GEOM_SIZEX 0.10 // 1m square by default
#define STG_DEFAULT_GEOM_SIZEY 0.10
#define STG_DEFAULT_GEOM_SIZEZ 0.10
#define STG_DEFAULT_GRID false
#define STG_DEFAULT_GRIPPERRETURN false
#define STG_DEFAULT_LASERRETURN LaserVisible
#define STG_DEFAULT_MAP_RESOLUTION 0.1
#define STG_DEFAULT_MASK (STG_MOVE_TRANS | STG_MOVE_ROT)
#define STG_DEFAULT_MASS 10.0  // kg
#define STG_DEFAULT_NOSE false
#define STG_DEFAULT_OBSTACLERETURN true
#define STG_DEFAULT_OUTLINE true
#define STG_DEFAULT_RANGERRETURN true


// constructor
StgModel::StgModel( StgWorld* world,
		    StgModel* parent,
		    stg_id_t id,
		    char* typestr )
  : StgAncestor()
{    
  PRINT_DEBUG4( "Constructing model world: %s parent: %s type: %s id: %d",
		world->Token(), 
		parent ? parent->Token() : "(null)",
		typestr,
		id );

  this->id = id;
  this->typestr = typestr;
  this->parent = parent; 
  this->world = world;

  this->debug = false;

  // generate a name. This might be overwritten if the "name" property
  // is set in the worldfile when StgModel::Load() is called
  
  StgAncestor* anc = parent ? (StgAncestor*)parent : (StgAncestor*)world;

  unsigned int cnt = anc->GetNumChildrenOfType( typestr );
  char* buf = new char[STG_TOKEN_MAX];

  snprintf( buf, STG_TOKEN_MAX, "%s.%s:%d", 
	    anc->Token(), typestr, cnt ); 

  this->token = strdup( buf );
  delete buf;
  
  PRINT_DEBUG2( "model has token \"%s\" and typestr \"%s\"", 
		this->token, this->typestr );  

  anc->AddChild( this );
  world->AddModel( this );
  
  memset( &pose, 0, sizeof(pose));
  memset( &global_pose, 0, sizeof(global_pose));
  


  this->data_fresh = false;
  this->disabled = false;
  this->d_list = NULL;
  this->blocks = NULL;
  this->body_dirty = true;
  this->data_dirty = true;
  this->gpose_dirty = true;
  this->say_string = NULL;
  this->subs = 0;
  
  this->geom.size.x = STG_DEFAULT_GEOM_SIZEX;
  this->geom.size.y = STG_DEFAULT_GEOM_SIZEX;
  this->geom.size.z = STG_DEFAULT_GEOM_SIZEX;
  memset( &this->geom.pose, 0, sizeof(this->geom.pose));

  this->obstacle_return = STG_DEFAULT_OBSTACLERETURN;
  this->ranger_return = STG_DEFAULT_RANGERRETURN;
  this->blob_return = STG_DEFAULT_BLOBRETURN;
  this->laser_return = STG_DEFAULT_LASERRETURN;
  this->gripper_return = STG_DEFAULT_GRIPPERRETURN;
  this->boundary = STG_DEFAULT_BOUNDARY;
  this->color = STG_DEFAULT_COLOR;
  this->map_resolution = STG_DEFAULT_MAP_RESOLUTION; // meters
  this->gui_nose = STG_DEFAULT_NOSE;
  this->gui_grid = STG_DEFAULT_GRID;
  this->gui_outline = STG_DEFAULT_OUTLINE;
  this->gui_mask = this->parent ? 0 : STG_DEFAULT_MASK;

  this->callbacks = g_hash_table_new( g_int_hash, g_int_equal );

  bzero( &this->velocity, sizeof(velocity));
  this->on_velocity_list = false;

  //this->velocity.a = 0.2;
  //SetVelocity( &this->velocity );

  this->last_update = 0;
  this->interval = 1e4; // 10msec
  
  // now we can add the basic square shape
  this->AddBlockRect( -0.5,-0.5,1,1 );
  
  PRINT_DEBUG2( "finished model %s (%d).", 
		this->token,
		this->id );
}

StgModel::~StgModel( void )
{
  // remove from parent, if there is one
  if( parent ) 
    parent->children = g_list_remove( parent->children, this );
    
  if( callbacks ) g_hash_table_destroy( callbacks );

  world->RemoveModel( this );  
}


void StgModel::AddBlock( stg_point_t* pts, 
			 size_t pt_count,
			 stg_meters_t zmin,
			 stg_meters_t zmax,
			 stg_color_t col,
			 bool inherit_color )
{
  //UnMap();
  
  blocks = 
    g_list_prepend( blocks, new StgBlock( this, pts, pt_count, 
					  zmin, zmax, 
					  col, inherit_color ));  
  
  // force recreation of display lists before drawing
  body_dirty = true;

  //Map();
}


void StgModel::ClearBlocks( void )
{
  stg_block_list_destroy( blocks );
  blocks = NULL;
  body_dirty = true;
}

void StgModel::AddBlockRect( double x, double y, 
			     double width, double height )
{  
  stg_point_t pts[4];
  pts[0].x = x;
  pts[0].y = y;
  pts[1].x = x + width;
  pts[1].y = y;
  pts[2].x = x + width;
  pts[2].y = y + height;
  pts[3].x = x;
  pts[3].y = y + height;

  // todo - fix this
  AddBlock( pts, 4, 0, 1, 0, true );	      
}

stg_meters_t StgModel::Raytrace( stg_radians_t angle,
				 stg_meters_t range, 
				 stg_block_match_func_t func,
				 const void* arg,
				 StgModel** hitmod )
  
{
  stg_pose_t gpose;
  GetGlobalPose( &gpose );
  gpose.a += angle;
  
  return world->Raytrace( this, 
			  &gpose,
			  range,
			  func,
			  arg,
			  hitmod );
}

stg_meters_t StgModel::Raytrace( stg_pose_t* pz,
				 stg_meters_t range, 
				 stg_block_match_func_t func,
				 const void* arg,
				 StgModel** hitmod )

{
  stg_pose_t gpose;
  memcpy( &gpose, pz, sizeof(stg_pose_t) );
  LocalToGlobal( &gpose );
  
  return world->Raytrace( this,
			  &gpose,
			  range,
			  func,
			  arg,
			  hitmod );
}




stg_d_draw_t* stg_d_draw_create( stg_d_type_t type,
				 stg_vertex_t* verts,
				 size_t vert_count )
{
  size_t vert_mem_size = vert_count * sizeof(stg_vertex_t);
  
  // allocate space for the draw structure and the vertex data behind it
  stg_d_draw_t* d = (stg_d_draw_t*)
    g_malloc( sizeof(stg_d_draw_t) + vert_mem_size );
  
  d->type = type;
  d->vert_count = vert_count;
  
  // copy the vertex data behind the draw structure
  memcpy( d->verts, verts, vert_mem_size );
	     
  return d;
}

void stg_d_draw_destroy( stg_d_draw_t* d )
{
  g_free( d );
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
void StgModel::GlobalToLocal( stg_pose_t* pose )
{
  //printf( "g2l global pose %.2f %.2f %.2f\n",
  //  pose->x, pose->y, pose->a );

  // get model's global pose
  stg_pose_t org;
  GetGlobalPose( &org );

  //printf( "g2l global origin %.2f %.2f %.2f\n",
  //  org.x, org.y, org.a );

  // compute global pose in local coords
  double sx =  (pose->x - org.x) * cos(org.a) + (pose->y - org.y) * sin(org.a);
  double sy = -(pose->x - org.x) * sin(org.a) + (pose->y - org.y) * cos(org.a);
  double sa = pose->a - org.a;
  
  pose->x = sx;
  pose->y = sy;
  pose->a = sa;
  
  //printf( "g2l local pose %.2f %.2f %.2f\n",
  //  pose->x, pose->y, pose->a );
}

// // default update function that implements velocities for all objects
// void StgModel::MoveDueToVelocity( void )
// {
//   // now move the model if it has any velocity
//   if( velocity.x || velocity.y || velocity.z || velocity.a )
//     this->UpdatePoses( void );
// }


// StgModel* StgModel::Create( stg_world_t* world, StgModel* parent, 
// 			    stg_id_t id, CWorldFile* wf )
// { 
//   return new StgModel( world, parent, id, wf ); 
// }    



void StgModel::Say( char* str )
{
  if( say_string )
    free( say_string );
  say_string = strdup( str );
}

bool StgModel::IsAntecedent( StgModel* testmod )
{
  if( this == testmod )
    return true;
  
  if( this->Parent()->IsAntecedent( testmod ) )
    return true;
  
  // neither mod nor a child of mod matches testmod
  return false;
}

// returns true if model [testmod] is a descendent of model [mod]
bool StgModel::IsDescendent( StgModel* testmod )
{
  if( this == testmod )
    return true;

  for( GList* it=this->children; it; it=it->next )
    {
      StgModel* child = (StgModel*)it->data;
      if( child->IsDescendent( testmod ) )
	return true;
    }
  
  // neither mod nor a child of mod matches testmod
  return false;
}

// returns 1 if model [mod1] and [mod2] are in the same model tree
bool StgModel::IsRelated( StgModel* mod2 )
{
  if( this == mod2 )
    return true;

  // find the top-level model above mod1;
  StgModel* t = this;
  while( t->Parent() )
    t = t->Parent();

  // now seek mod2 below t
  return t->IsDescendent( mod2 );
}

// get the model's velocity in the global frame
void StgModel::GetGlobalVelocity( stg_velocity_t* gv )
{
  stg_pose_t gpose;
  this->GetGlobalPose( &gpose );
  
  double cosa = cos( gpose.a );
  double sina = sin( gpose.a );
  
  gv->x = velocity.x * cosa - velocity.y * sina;
  gv->y = velocity.x * sina + velocity.y * cosa;
  gv->a = velocity.a;
      
  //printf( "local velocity %.2f %.2f %.2f\nglobal velocity %.2f %.2f %.2f\n",
  //  mod->velocity.x, mod->velocity.y, mod->velocity.a,
  //  gvel->x, gvel->y, gvel->a );
}

// set the model's velocity in the global frame
void StgModel::SetGlobalVelocity( stg_velocity_t* gv )
{
  stg_pose_t gpose;
  this->GetGlobalPose( &gpose );
  
  double cosa = cos( gpose.a );
  double sina = sin( gpose.a );

  stg_velocity_t lv;
  lv.x = gv->x * cosa + gv->y * sina; 
  lv.y = -gv->x * sina + gv->y * cosa; 
  lv.a = gv->a;

  this->SetVelocity( &lv );
}

// get the model's position in the global frame
void StgModel::GetGlobalPose( stg_pose_t* gpose )
{ 
  //printf( "model %s global pose ", token );

  if( this->gpose_dirty )
    {
      stg_pose_t parent_pose;  
      
      // find my parent's pose
      if( this->parent )
	{
	  parent->GetGlobalPose( &parent_pose );
	  
	  global_pose.x = parent_pose.x + pose.x * cos(parent_pose.a) 
	    - pose.y * sin(parent_pose.a);
	  global_pose.y = parent_pose.y + pose.x * sin(parent_pose.a) 
	    + pose.y * cos(parent_pose.a);
	  global_pose.a = NORMALIZE(parent_pose.a + pose.a);
	  
	  // no 3Dg geometry, as we can only rotate about the z axis (yaw)
	  // but we are on top of our parent
	  global_pose.z = parent_pose.z + this->parent->geom.size.z + pose.z;
	}
      else
	memcpy( &global_pose, &pose, sizeof(stg_pose_t));
  
      this->gpose_dirty = false;
      //printf( " WORK " );

    }
  //else
  //printf( " CACHED " );
  
//   PRINT_DEBUG4( "GET GLOBAL POSE [x:%.2f y:%.2f z:%.2f a:%.2f]",
// 		global_pose.x,
// 		global_pose.y,
// 		global_pose.z,
// 		global_pose.a );


  memcpy( gpose, &global_pose, sizeof(stg_pose_t));
}
    
  
// convert a pose in this model's local coordinates into global
// coordinates
// should one day do all this with affine transforms for neatness?
void StgModel::LocalToGlobal( stg_pose_t* pose )
{  
  stg_pose_t pz;
  memcpy( &pz, pose, sizeof(stg_pose_t));
  
  stg_pose_t origin;   
  this->GetGlobalPose( &origin );
  
  stg_pose_sum( pose, &origin, &pz );
}


void StgModel::MapWithChildren()
{
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

  if( world->graphics && this->debug )
    {
      double scale = 1.0 / world->ppm;
      glPushMatrix();
      glTranslatef( 0,0,1 );
      glScalef( scale,scale,scale );
    }
  
  for( GList* it=blocks; it; it=it->next )
    ((StgBlock*)it->data)->Map();

  if( world->graphics && this->debug )
    glPopMatrix();
} 

void StgModel::UnMap()
{
  //PRINT_DEBUG1( "%s.UnMap()", token );

  for( GList* it=blocks; it; it=it->next )
    ((StgBlock*)it->data)->UnMap();
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
  
  printf( "%d %s:%s\n", 
	  id, 
	  world->Token(), 
	  token );
}

const char* StgModel::PrintWithPose()
{
  stg_pose_t gpose;
  GetGlobalPose( &gpose );
  
  static char txt[256];  
  snprintf(txt, sizeof(txt), "%s @ [%.2f,%.2f,%.2f,%.2f]",  
	   token, 
	   gpose.x, gpose.y, gpose.z, gpose.a  );
  
  return txt;
}



void StgModel::Startup( void )
{
  //printf( "Startup model %s\n", this->token );
  
  world->StartUpdatingModel( this );
  
  CallCallbacks( &startup );
}

void StgModel::Shutdown( void )
{
  //printf( "Shutdown model %s\n", this->token );
  
  world->StopUpdatingModel( this );

  CallCallbacks( &shutdown );
}

void StgModel::UpdateIfDue( void )
{
  if(  world->sim_time  >= 
       (last_update + interval) )
    this->Update();
}

// void StgModel::UpdateTree( void )
// {
//   LISTMETHOD( this->children, StgModel*, UpdateTree );
// }

void StgModel::Update( void )
{
  //printf( "[%lu] %s update (%d subs)\n", 
  //  this->world->sim_time_ms, this->token, this->subs );
  
  CallCallbacks( &update );

  last_update = world->sim_time;
}
 
void StgModel::DrawSelected()
{
  glPushMatrix();
 
  glTranslatef( pose.x, pose.y, pose.z );

  stg_pose_t gpose;
  GetGlobalPose(&gpose);
  
  char buf[64];
  snprintf( buf, 63, "%s [%.2f,%.2f,%.2f]", token, gpose.x, gpose.y, RTOD(gpose.a) );
  
  PushColor( 0,0,0,1 ); // text color black
  gl_draw_string( 0.5,0.5,0.5, buf );

  glRotatef( RTOD(pose.a), 0,0,1 );
 
  gl_pose_shift( &geom.pose );
  
  double dx = geom.size.x / 2.0 * 1.6;
  double dy = geom.size.y / 2.0 * 1.6;
  
  PopColor();
  PushColor( 1,0,0,0.8 ); // highlight color red
  glRectf( -dx, -dy, dx, dy );

  PopColor();
  glPopMatrix();
}

void StgModel::Draw( uint32_t flags )
{
  //PRINT_DEBUG1( "Drawing %s", token );

  glPushMatrix();

  // move into this model's local coordinate frame
  gl_pose_shift( &this->pose );
  gl_pose_shift( &this->geom.pose );

  // draw all the blocks
  if( flags & STG_SHOW_BLOCKS )
    LISTMETHOD( this->blocks, StgBlock*, Draw );
  
  if( flags & STG_SHOW_DATA )
    this->DataVisualize();

  //if( flags & STG_SHOW_GEOM )
    //this->DataVisualize();

  // etc

  //if( this->say_string )
  // gl_speech_bubble( 0,0,0, this->say_string );
  
  
  if( gui_grid && (flags & STG_SHOW_GRID) )
    DrawGrid();

  // shift up the CS to the top of this model
  gl_coord_shift(  0,0, this->geom.size.z, 0 );
  
  // recursively draw the tree below this model 
  //LISTMETHOD( this->children, StgModel*, Draw );
  for( GList* it=children; it; it=it->next )
    ((StgModel*)it->data)->Draw( flags );

  glPopMatrix(); // drop out of local coords
}

void StgModel::DrawPicker( void )
{
  //PRINT_DEBUG1( "Drawing %s", token );

  glPushMatrix();

  // move into this model's local coordinate frame
  gl_pose_shift( &this->pose );
  gl_pose_shift( &this->geom.pose );
  
  // draw the boxes
  for( GList* it=blocks; it; it=it->next )
    ((StgBlock*)it->data)->DrawSolid() ;
  
  // shift up the CS to the top of this model
  gl_coord_shift(  0,0, this->geom.size.z, 0 );
  
  // recursively draw the tree below this model 
  LISTMETHOD( this->children, StgModel*, DrawPicker );

  glPopMatrix(); // drop out of local coords
}


void StgModel::DataVisualize( void )
{
  // do nothing
}

void StgModel::DrawGrid( void )
{
  PushColor( 0,0,0,0.1 );

  double dx = geom.size.x;
  double dy = geom.size.y;
  double sp = 1.0;
 
  int nx = (int) ceil((dx/2.0) / sp);
  int ny = (int) ceil((dy/2.0) / sp);
  
  if( nx == 0 ) nx = 1.0;
  if( ny == 0 ) ny = 1.0;
  
  glBegin(GL_LINES);

  // draw the bounding box first
  glVertex2f( -nx, -ny );
  glVertex2f(  nx, -ny );

  glVertex2f( -nx, ny );
  glVertex2f(  nx, ny );

  glVertex2f( nx, -ny );
  glVertex2f( nx,  ny );

  glVertex2f( -nx,-ny );
  glVertex2f( -nx, ny );

  char str[16];
  int i;
  for (i = -nx+1; i < nx; i++)
    {
      glVertex2f(  i * sp,  - dy/2 );
      glVertex2f(  i * sp,  + dy/2 );
      snprintf( str, 16, "%d", (int)i );
      //gl_draw( str, -0.2 + (nx + i * sp), -0.2 , 1 );
    }
  
  for (i = -ny+1; i < ny; i++)
    {
      glVertex2f( - dx/2, i * sp );
      glVertex2f( + dx/2,  i * sp );
      snprintf( str, 16, "%d", (int)i );
      //gl_draw( str, -0.2, -0.2 + (ny + i * sp) , 1 );
    }
  
  glEnd();
  
  PopColor();
}

void StgModel::GetVelocity( stg_velocity_t* dest )
{
  assert(dest);
  memcpy( dest, &this->velocity, sizeof(stg_velocity_t));
}

bool velocity_is_nonzero( stg_velocity_t* v )
{
  return( v->x || v->y || v->z || v->a );
}

void StgModel::SetVelocity( stg_velocity_t* vel )
{
  assert(vel);  
  memcpy( &this->velocity, vel, sizeof(stg_velocity_t));
  
  if( ! this->on_velocity_list && velocity_is_nonzero( & this->velocity ) )
    {
      this->world->velocity_list = g_list_prepend( this->world->velocity_list, this );
      this->on_velocity_list = true;
    }
  
  if( this->on_velocity_list && ! velocity_is_nonzero( &this->velocity ))
    {
      this->world->velocity_list = g_list_remove( this->world->velocity_list, this );
      this->on_velocity_list = false;
    }    

  CallCallbacks( &this->velocity );
}

void StgModel::NeedRedraw( void )
{
  this->world->dirty = true;
}

void StgModel::GPoseDirtyTree( void )
{
  this->gpose_dirty = true; // our global pose may have changed
  
  for( GList* it = this->children; it; it=it->next )
    ((StgModel*)it->data)->GPoseDirtyTree();      
}

void StgModel::SetPose( stg_pose_t* pose )
{
  //PRINT_DEBUG5( "%s.SetPose(%.2f %.2f %.2f %.2f)", 
  //	this->token, pose->x, pose->y, pose->z, pose->a ); 
  
  assert(pose);

  // if the pose has changed, we need to do some work
  if( memcmp( &this->pose, pose, sizeof(stg_pose_t) ) != 0 )
    {
      UnMapWithChildren();
      
      memcpy( &this->pose, pose, sizeof(stg_pose_t));

      this->pose.a = NORMALIZE(this->pose.a);
		            
      //double hitx, hity;
      //stg_model_test_collision2( mod, &hitx, &hity );

      this->NeedRedraw();
      this->GPoseDirtyTree(); // global pose may have changed

      MapWithChildren();
    }

  // register a model change even if the pose didn't actually change
  CallCallbacks( &this->pose );
}

void StgModel::AddToPose( double dx, double dy, double dz, double da )
{
  if( dx || dy || dz || da )
    {
      stg_pose_t pose;
      GetPose( &pose );
      pose.x += dx;
      pose.y += dy;
      pose.z += dz;
      pose.a += da;

      SetPose( &pose );
    }
}

void StgModel::AddToPose( stg_pose_t* pose )
{
  assert( pose );
  this->AddToPose( pose->x, pose->y, pose->z, pose->a );
}


void StgModel::GetGeom( stg_geom_t* dest )
{
  assert(dest);
  memcpy( dest, &this->geom, sizeof(stg_geom_t));
}

void StgModel::SetGeom( stg_geom_t* geom )
{
  //printf( "MODEL \"%s\" SET GEOM (%.2f %.2f %.2f)[%.2f %.2f]\n",
  //  this->token,
  //  geom->pose.x, geom->pose.y, geom->pose.a,
  //  geom->size.x, geom->size.y );

  assert(geom);
 
  UnMap();
  
  memcpy( &this->geom, geom, sizeof(stg_geom_t));
  
  stg_block_list_scale( blocks,
			&geom->size );
  
  body_dirty = true;

  Map();  

  CallCallbacks( &this->geom );
}

void StgModel::SetColor( stg_color_t col )
{
  this->color = col;  
  body_dirty = true;
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

void StgModel::SetLaserReturn( int val )
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

void StgModel::GetPose( stg_pose_t* dest )
{
  assert(dest);
  memcpy( dest, &this->pose, sizeof(stg_pose_t));
}

// set the pose of model in global coordinates 
void StgModel::SetGlobalPose( stg_pose_t* gpose )
{
  if( this->parent == NULL )
    {
      //printf( "setting pose directly\n");
      this->SetPose( gpose );
    }  
  else
    {
      stg_pose_t lpose;
      memcpy( &lpose, gpose, sizeof(lpose) );
      this->parent->GlobalToLocal( &lpose );
      this->SetPose( &lpose );
    }

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
  
  // HACK - completely rebuild the GUI elements - it's too complex to patch up the tree
  //gui_model_destroy( mod );
  //gui_model_create( mod );

  CallCallbacks( &this->parent );

  return 0; //ok
}



int lines_raytrace_match( StgModel* mod, StgModel* hitmod ) 
 { 
   // Ignore invisible things, myself, my children, and my ancestors. 
   if(  hitmod->ObstacleReturn() && !(mod == hitmod) )//&& (!stg_model_is_related(mod,hitmod)) )  
     return 1; 
  
   return 0; // no match 
 }	 



// Check to see if the given pose will yield a collision with obstacles.
// Returns a pointer to the first entity we are in collision with, and stores
// the location of the hit in hitx,hity (if non-null)
// Returns NULL if not collisions.
// This function is useful for writing position devices.

StgModel* StgModel::TestCollision( stg_pose_t* pose, 
				   double* hitx, double* hity ) 
{ 
  
/*  stg_model_t* child_hit = NULL; */

/*   GList* it; */
/*   for(it=mod->children; it; it=it->next ) */
/*     { */
/*       stg_model_t* child = (stg_model_t*)it->data; */
/*       child_hit = stg_model_test_collision( child, hitx, hity ); */
/*       if( child_hit ) */
/* 	return child_hit; */
/*     } */
  

/*   stg_pose_t pose; */
/*   memcpy( &pose, &mod->geom.pose, sizeof(pose)); */
/*   stg_model_local_to_global( mod, &pose ); */
  
/*   //return NULL; */
  
  // raytrace along all our blocks. expensive, but most vehicles 
  // will just be a few blocks, grippers 3 blocks, etc. not too bad. 

  // no blocks, no hit!
  if( this->blocks == NULL )
    return NULL;

  // unrender myself - avoids a lot of self-hits
  this->UnMap();

  // add the local geom offset
  stg_pose_t local;
  stg_pose_sum( &local, pose, &this->geom.pose );
  
  //glNewList( this->dl_debug, GL_COMPILE );
  //glPushMatrix();
  //glTranslatef( 0,0,1.0 );  
  //push_color_rgb( 0,0,1 );
  //glBegin( GL_LINES );

  // loop over all blocks 
  for( GList* it = this->blocks; it; it=it->next )
    {
      StgBlock* b = (StgBlock*)it->data;
      
      // loop over all edges of the block
      for( unsigned int p=0; p<b->pt_count; p++ ) 
	{ 
	  stg_point_t* pt1 = &b->pts[p];
	  stg_point_t* pt2 = &b->pts[(p+1) % b->pt_count]; 

	  stg_pose_t pp1; 
 	  pp1.x = pt1->x; 
 	  pp1.y = pt1->y; 
 	  pp1.a = 0; 
	  
 	  stg_pose_t pp2; 
 	  pp2.x = pt2->x; 
 	  pp2.y = pt2->y; 
 	  pp2.a = 0; 
	  
 	  stg_pose_t p1; 
 	  stg_pose_t p2; 
	  
 	  // shift the line points into the global coordinate system 
 	  stg_pose_sum( &p1, &local, &pp1 ); 
 	  stg_pose_sum( &p2, &local, &pp2 ); 
	  
	  //glVertex2f( p1.x, p1.y );
	  //glVertex2f( p2.x, p2.y );

 	  //printf( "tracing %.2f %.2f   %.2f %.2f\n",  p1.x, p1.y, p2.x, p2.y ); 
	  
	  //double dx = p2.x - p1.x;
	  //double dy = p2.y - p1.y;

//  	  itl_t* itl = itl_create( p1.x, p1.y, 0.2,
// 				   p2.x, p2.y,  				   
//  				   root->matrix,  
//  				   PointToPoint ); 
	  	  
//  	  StgModel* hitmod = itl_first_matching( itl, lines_raytrace_match, this ); 
	  
//  	  if( hitmod ) 
//  	    { 
//  	      if( hitx ) *hitx = itl->x; // report them 
//  	      if( hity ) *hity = itl->y;	   
//  	      itl_destroy( itl ); 

// 	      this->Map(1);

// 	      //pop_color();
// 	      //glEnd();
// 	      //glPopMatrix();
// 	      //glEndList();
	      
//  	      return hitmod; // we hit this object! stop raytracing 
//  	    } 

//  	  itl_destroy( itl ); 
 	} 
    } 
  
  // re-render myself
  this->Map();


  //pop_color();
  //glEnd();
  //glPopMatrix();
  //glEndList();


   return NULL;  // done  
}



void StgModel::UpdatePose( void )
{
  if( disabled )
    return;

   //stg_velocity_t gvel;
   //this->GetGlostg_model_get_global_velocity( mod, &gvel );
      
   //stg_pose_t gpose;
   //stg_model_get_global_pose( mod, &gpose );
   
   // convert msec to sec
   double interval = (double)world->interval_sim / 1e6;
  

   //stg_pose_t old_pose;
   //memcpy( &old_pose, &mod->pose, sizeof(old_pose));

   stg_velocity_t v;
   this->GetVelocity( &v );

   stg_pose_t p;
   this->GetPose( &p );

   p.x += (v.x * cos(p.a) - v.y * sin(p.a)) * interval;
   p.y += (v.x * sin(p.a) + v.y * cos(p.a)) * interval;
   p.a += v.a * interval;

   //this->SetPose( &p );

   // convert the new pose to global coords so we can see what it might hit

   stg_pose_t gp;
   memcpy( &gp, &p, sizeof(stg_pose_t));

   //this->LocalToGlobal( &gp );

   // check this model and all it's children at the new pose
   double hitx=0, hity=0;
   StgModel* hitthing = this->TestCollision( &gp, &hitx, &hity );

   if( hitthing )
     {
       printf( "hit %s at %.2f %.2f\n",
	       hitthing->Token(), hitx, hity );
     }
   else
     this->SetPose( &p );
     

//   int stall = 0;
      
//   if( hitthing )
//     {
//       // grippable objects move when we hit them
//       if(  hitthing->gripper_return ) 
// 	{
// 	  //PRINT_WARN( "HIT something grippable!" );
	  
// 	  stg_velocity_t hitvel;
// 	  //hitvel.x = gvel.x;
// 	  //hitvel.y = gvel.y;
// 	  //hitvel.a = 0.0;

// 	  stg_pose_t hitpose;
// 	  stg_model_get_global_pose( hitthing, &hitpose );
// 	  double hita = atan2( hitpose.y - hity, hitpose.x - hitx );
// 	  double mag = 0.3;
// 	  hitvel.x =  mag * cos(hita);
// 	  hitvel.y = mag * sin(hita);
// 	  hitvel.a = 0;

// 	  stg_model_set_global_velocity( hitthing, &hitvel );
	  
// 	  stall = 0;
// 	  // don't make the move - reset the pose
// 	  //memcpy( &mod->pose, &old_pose, sizeof(mod->pose));
// 	}
//       else // other objects block us totally
// 	{
// 	  hitthing = NULL;
// 	  // move back to where we started
// 	  memcpy( &mod->pose, &old_pose, sizeof(mod->pose));
// 	  interval = 0.2 * interval; // slow down time
	  
// 	  // inch forward until we collide
// 	  do
// 	    {
// 	      memcpy( &old_pose, &mod->pose, sizeof(old_pose));
	      
// 	      mod->pose.x += 
// 		(mod->velocity.x * cos(mod->pose.a) - mod->velocity.y * sin(mod->pose.a)) * interval;
// 	      mod->pose.y += 
// 		(mod->velocity.x * sin(mod->pose.a) + mod->velocity.y * cos(mod->pose.a)) * interval;
// 	      mod->pose.a += mod->velocity.a * interval;
	      
// 	      hitthing = stg_model_test_collision( mod, &hitx, &hity );
	      
// 	    } while( hitthing == NULL );
	  
// 	  //PRINT_WARN( "HIT something immovable!" );
	  
// 	  stall = 1;
	  
// 	  // set velocity to zero
// 	  stg_velocity_t zero_v;
// 	  memset( &zero_v, 0, sizeof(zero_v));
// 	  stg_model_set_velocity( mod, &zero_v );

// 	  // don't make the move - reset the pose
// 	  memcpy( &mod->pose, &old_pose, sizeof(mod->pose));

// 	}
//     }

//   // if the pose changed, we need to set it.
//   if( memcmp( &old_pose, &mod->pose, sizeof(old_pose) ))
//       stg_model_set_pose( mod, &mod->pose );

//   stg_model_set_stall( mod, stall );
  
//   /* trails */
//   //if( fig_trails )
//   //gui_model_trail()


//   // if I'm a pucky thing, slow down for next time - simulates friction
//   if( mod->gripper_return )
//     {
//       double slow = 0.08;

//       if( mod->velocity.x > 0 )
// 	{
// 	  mod->velocity.x -= slow;
// 	  mod->velocity.x = MAX( 0, mod->velocity.x );
// 	}
//       else if ( mod->velocity.x < 0 )
// 	{
// 	  mod->velocity.x += slow;
// 	  mod->velocity.x = MIN( 0, mod->velocity.x );
// 	}
      
//       if( mod->velocity.y > 0 )
// 	{
// 	  mod->velocity.y -= slow;
// 	  mod->velocity.y = MAX( 0, mod->velocity.y );
// 	}
//       else if( mod->velocity.y < 0 )
// 	{
// 	  mod->velocity.y += slow;
// 	  mod->velocity.y = MIN( 0, mod->velocity.y );
// 	}

//       CallCallbacks( &this->velocity );
//     }
  
//   return 0; // ok
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


