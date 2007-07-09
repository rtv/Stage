
#define _GNU_SOURCE

#include <limits.h> 
#include <assert.h>
#include <math.h>
//#include <string.h> // for strdup(3)

//#define DEBUG

#include "stage_internal.h"
#include "model.hh"
#include "gui.h"
#include <GL/gl.h>

  // basic model
#define STG_DEFAULT_MASS 10.0  // kg
#define STG_DEFAULT_GEOM_SIZEX 1.0 // 1m square by default
#define STG_DEFAULT_GEOM_SIZEY 1.0
#define STG_DEFAULT_GEOM_SIZEZ 1.0
#define STG_DEFAULT_OBSTACLERETURN TRUE
#define STG_DEFAULT_LASERRETURN LaserVisible
#define STG_DEFAULT_RANGERRETURN TRUE
#define STG_DEFAULT_BLOBRETURN TRUE
#define STG_DEFAULT_COLOR (0xFF0000) // red
#define STG_DEFAULT_ENERGY_CAPACITY 1000.0
#define STG_DEFAULT_ENERGY_CHARGEENABLE 1
#define STG_DEFAULT_ENERGY_PROBERANGE 0.0
#define STG_DEFAULT_ENERGY_GIVERATE 0.0
#define STG_DEFAULT_ENERGY_TRICKLERATE 0.1
#define STG_DEFAULT_MASK (STG_MOVE_TRANS | STG_MOVE_ROT)
#define STG_DEFAULT_NOSE FALSE
#define STG_DEFAULT_GRID FALSE
#define STG_DEFAULT_OUTLINE TRUE
#define STG_DEFAULT_MAP_RESOLUTION 0.1


//extern int _stg_disable_gui;

extern int dl_debug;

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
  
void StgModel::AddBlock( stg_point_t* pts, 
			 size_t pt_count,
			 stg_meters_t zmin,
			 stg_meters_t zmax,
			 stg_color_t col,
			 bool inherit_color )
{
  this->blocks = 
    g_list_prepend( this->blocks, 
		    stg_block_create( this, pts, pt_count, 
				      zmin, zmax, 
				      col, inherit_color ));  
  
  // force recreation of display lists before drawing
  this->dirty = true;
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
  this->AddBlock( pts, 4, 0, 1, 0, true );	      
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
  this->GetGlobalPose( &org );

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


// constructor
StgModel::StgModel( stg_world_t* world, 
		    StgModel* parent,
		    stg_id_t id,
		    CWorldFile* wf )
{    
  this->wf = wf;
  this->id = id; // id is also our worldfile entity number, always unique
  this->disabled = FALSE;
  this->world = world;
  this->parent = parent; 

  this->d_list = NULL;
  this->blocks = NULL;
  this->dirty = true;

  this->typestr = wf->GetEntityType( this->id );
  this->child_type_count = g_hash_table_new( g_str_hash, g_str_equal );
  
  PRINT_DEBUG3( "Constructing model %d.%d(%s)\n", 
		this->world->id,
		this->id,
		typestr );
  
  // add this model to its parent's list of children (if any)
  if( parent)
    parent->children = g_list_append( parent->children, this );      
  
  // create this model's empty list of children
  this->children = NULL;
  
  
  // add one to to the parent's count of objects of this type
  GHashTable* tbl = NULL;

  if( parent )
    tbl = parent->child_type_count;
  else
    tbl = world->child_type_count;
  
  int typecnt = (int)g_hash_table_lookup( tbl, typestr);

  // generate a name. This might be overwritten if the "name" property
  // is set in the worldfile when StgModel::Load() is called
  
  if( parent )
    snprintf( this->token, STG_TOKEN_MAX, "%s.%s:%d", 
	      parent->Token(), typestr, typecnt ); 
  else
    snprintf( this->token, STG_TOKEN_MAX, "%s:%d", 
	      typestr, typecnt ); 

  //printf( "model has token \"%s\" and typestr \"%s\"\n", 
  //  this->token, this->typestr );
	
      
  g_hash_table_insert( tbl, (gpointer)typestr, (gpointer)++typecnt);
  

  // initialize the table of callbacks that are triggered when a
  // model's fields change
  this->callbacks = g_hash_table_new( g_int_hash, g_int_equal );

  this->geom.size.x = STG_DEFAULT_GEOM_SIZEX;
  this->geom.size.y = STG_DEFAULT_GEOM_SIZEX;
  this->geom.size.z = STG_DEFAULT_GEOM_SIZEX;
  
  this->obstacle_return = 1;
  this->ranger_return = 1;
  this->blob_return = 1;
  this->laser_return = LaserVisible;
  this->gripper_return = 0;
  this->boundary = 0;
  this->color = 0xFF0000; // red;  
  this->map_resolution = 0.1; // meters
  this->gui_nose = STG_DEFAULT_NOSE;
  this->gui_grid = STG_DEFAULT_GRID;
  this->gui_outline = STG_DEFAULT_OUTLINE;
  this->gui_mask = this->parent ? 0 : STG_DEFAULT_MASK;

  // now we can add the basic square shape
  this->AddBlockRect( -0.5,-0.5,1,1 );

  this->data = this->cfg = this->cmd = NULL;
  
  this->subs = 0;

  // TODO!
  // now it's safe to create the GUI components
  //this->GuiInit();

  // GL
  this->dl_body = glGenLists( 1 );
  this->dl_data = glGenLists( 1 );

  // exterimental: creates a menu of models
  // gui_add_tree_item( mod );
  
  PRINT_DEBUG3( "finished model %d.%d (%s)", 
		this->world->id, 
		this->id, 
		this->token );  
}

StgModel::~StgModel( void )
{
  // remove from parent, if there is one
  if( parent && parent->children ) 
    parent->children = g_list_remove( parent->children, this );
  
  // free all local stuff 
  // TODO - make sure this is up to date - check leaks with valgrind
  if( children ) g_list_free( children );
  if( callbacks ) g_hash_table_destroy( callbacks );
  if( data ) g_free( data );
  if( cmd ) g_free( cmd );
  if( cfg ) g_free( cfg );
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

// returns TRUE if model [testmod] is a descendent of model [mod]
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
void  StgModel::GetGlobalPose( stg_pose_t* gpose )
{ 
  // todo - compute this only once and cache between calls.

  stg_pose_t parent_pose;  
  
  // find my parent's pose
  if( this->parent )
    {
      parent->GetGlobalPose( &parent_pose );
      
      gpose->x = parent_pose.x + pose.x * cos(parent_pose.a) 
	- pose.y * sin(parent_pose.a);
      gpose->y = parent_pose.y + pose.x * sin(parent_pose.a) 
	+ pose.y * cos(parent_pose.a);
      gpose->a = NORMALIZE(parent_pose.a + pose.a);

      // no 3Dg geometry, as we can only rotate about the z axis (yaw)
      gpose->z = parent_pose.z + this->parent->geom.size.z + pose.z;
    }
  else
    memcpy( gpose, &pose, sizeof(stg_pose_t));
}
    
  
// convert a pose in this model's local coordinates into global
// coordinates
// should one day do all this with affine transforms for neatness?
void StgModel::LocalToGlobal( stg_pose_t* pose )
{  
  stg_pose_t origin;   
  this->GetGlobalPose( &origin );
  stg_pose_sum( pose, &origin, pose );
}


// recursively map a model and all it's descendents
void StgModel::MapWithChildren( bool render )
{
  // call this function for all the model's children
  for( GList* it=children; it; it=it->next )
    ((StgModel*)it->data)->MapWithChildren(render);  
  
  // now map the model
  this->Map( render );
}

// if render is true, render the model into the matrix, else unrender
// the model
void StgModel::Map( bool render )
{
  // bail out if we have no blocks
  if( blocks == NULL )
    return;
  
  if( render )
    {
      // get model's global pose
      stg_pose_t org;
      memcpy( &org, &geom.pose, sizeof(org));
      this->LocalToGlobal( &org );

      // render the blocks in the matrix here
      for( GList* it=blocks; it; it=it->next )
	stg_matrix_block( this->world->matrix,
			  &org,
			  (stg_block_t*)it->data );      
    }
  else 
    // remove all the blcks from the matrix
    if( blocks )
      for( GList* it=blocks; it; it=it->next )
	stg_matrix_remove_object( this->world->matrix, it->data ); 
} 


void StgModel::Subscribe( void )
{
  subs++;
  world->subs++;
  
  printf( "subscribe to %s %d\n", token, subs );
  
  // if this is the first sub, call startup
  if( subs == 1 )
    this->Startup();
}

void StgModel::Unsubscribe( void )
{
  subs--;
  world->subs--;
  
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
  printf( "%s model %d:%d:%s\n", 
	  prefix ? prefix : "", 
	  world->id, 
	  id, 
	  token );
}


// // a virtual method init after the constructor
// void StgModel::Initialize( void )
// {
//   printf( "Initialize model %s\n", this->token );

//   // TODO 
//   // GUI init
//   // other init?

// }


void StgModel::Startup( void )
{
  printf( "Startup model %s\n", this->token );
  CallCallbacks( &startup );
}

void StgModel::Shutdown( void )
{
  printf( "Shutdown model %s\n", this->token );
  CallCallbacks( &shutdown );
}

void StgModel::Update( void )
{
  //printf( "[%lu] %s update (%d subs)\n", 
  //  this->world->sim_time, this->token, this->subs );
  
  this->CallCallbacks( &update );

  LISTMETHOD( this->children, StgModel*, Update );
}
 

void StgModel::Draw( void )
{
  //printf( "%s.Draw()\n",
  //  this->token );

  glPushMatrix();
  
  // move into this model's local coordinate frame
  gl_pose_shift( &this->pose );
  gl_pose_shift( &this->geom.pose );

  // todo - we don't need to do this so often

  if( this->dirty )
    {
      this->GuiGenerateBody();
      this->dirty = false;
    }
  
  // Call my various display lists
  //if( win->show_grid ) && this->gui_grid )
  //glCallList( this->dl_grid );
  
  glCallList( this->dl_body );
  glCallList( this->dl_data );
  glCallList( this->dl_data );
  
  // shift up the CS to the top of this model
  gl_coord_shift(  0,0, this->geom.size.z, 0 );
  
  // recursively draw the tree below this model 
  LISTMETHOD( this->children, StgModel*, Draw );

  glPopMatrix(); // drop out of local coords
}

// call this when the local physical appearance has changed
void StgModel::GuiGenerateBody( void )
{
  printf( "%s::GuiGenerateBody()\n", this->token );
  
  glNewList( this->dl_body, GL_COMPILE );
  
  // draw blocks
  LISTFUNCTION( this->blocks, stg_block_t*, stg_block_render );

  glEndList();
}
  
void StgModel::GuiGenerateGrid( void )
{
  glNewList( this->dl_grid, GL_COMPILE );

  push_color_rgba( 0.8,0.8,0.8,0.8 );

  double dx = this->geom.size.x;
  double dy = this->geom.size.y;
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

  int i;
  for (i = -nx+1; i < nx; i++)
    {
      glVertex2f(  i * sp,  - dy/2 );
      glVertex2f(  i * sp,  + dy/2 );
      //snprintf( str, 64, "%d", (int)i );
      //stg_rtk_fig_text( fig, -0.2 + (ox + i * sp), -0.2 , 0, str );
    }
  
  for (i = -ny+1; i < ny; i++)
    {
      glVertex2f( - dx/2, i * sp );
      glVertex2f( + dx/2,  i * sp );
      //snprintf( str, 64, "%d", (int)i );
      //stg_rtk_fig_text( fig, -0.2, -0.2 + (oy + i * sp) , 0, str );
    }
  
  glEnd();
  
  pop_color();
  glEndList();
}



//------------------------------------------------------------------------
// basic model properties

void StgModel::SetData( void* data, size_t len )
{
  this->data = g_realloc( this->data, len );
  memcpy( this->data, data, len );
  this->data_len = len;

  CallCallbacks( &this->data );
}

void StgModel::SetCmd( void* cmd, size_t len )
{
  this->cmd = g_realloc( this->cmd, len );
  memcpy( this->cmd, cmd, len );
  this->cmd_len = len;

  CallCallbacks( &this->cmd );
}

void StgModel::SetCfg( void* cfg, size_t len )
{
  this->cfg = g_realloc( this->cfg, len );
  memcpy( this->cfg, cfg, len );
  this->cfg_len = len;

  CallCallbacks( &this->cmd );
}

void* StgModel::GetCfg( size_t* lenp )
{
  if(lenp) *lenp = this->cfg_len;
  return this->cfg;
}

void* StgModel::GetData( size_t* lenp )
{
  if(lenp) *lenp = this->data_len;
  return this->data;
}

void* StgModel::GetCmd( size_t* lenp )
{
  if(lenp) *lenp = this->cmd_len;
  return this->cmd;
}

void StgModel::GetVelocity( stg_velocity_t* dest )
{
  assert(dest);
  memcpy( dest, &this->velocity, sizeof(stg_velocity_t));
}

void StgModel::SetVelocity( stg_velocity_t* vel )
{
  assert(vel);  
  memcpy( &this->velocity, vel, sizeof(stg_velocity_t));

  CallCallbacks( &this->velocity );
}


void StgModel::SetPose( stg_pose_t* pose )
{
/*   printf( "MODEL \"%s\" SET POSE (%.2f %.2f %.2f)\n", */
/* 	  this->token, pose->x, pose->y, pose->a ); */

  assert(pose);

  // if the pose has changed, we need to do some work
  if( memcmp( &this->pose, pose, sizeof(stg_pose_t) ) != 0 )
    {
      // unrender from the matrix
      MapWithChildren( 0 );
      
      memcpy( &this->pose, pose, sizeof(stg_pose_t));
      
      //double hitx, hity;
      //stg_model_test_collision2( mod, &hitx, &hity );

      // render in the matrix
      MapWithChildren( 1 );
    }

  // register a model change even if the pose didn't actually change
  CallCallbacks( &this->pose );
}

void StgModel::AddToPose( double dx, double dy, double dz, double da )
{
  if( dx || dy || dz || da )
    {
      // unrender from the matrix
      MapWithChildren( 0 );
      
      this->pose.x += dx;
      this->pose.y += dy;
      this->pose.z += dz;
      this->pose.a += da;

      // render in the matrix
      MapWithChildren( 1 );
    }
  
  // register a model change even if the pose didn't actually change
  CallCallbacks( &this->pose );
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
 
  // unrender from the matrix
  Map( 0 );
  
  memcpy( &this->geom, geom, sizeof(stg_geom_t));
  
  stg_block_list_scale( blocks,
			&geom->size );
  
  // re-render int the matrix
  Map( 1 );  

  CallCallbacks( &this->geom );
}

void StgModel::SetColor( stg_color_t col )
{
  this->color = col;
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



/* int lines_raytrace_match( stg_model_t* mod, stg_model_t* hitmod ) */
/* { */
/*   // Ignore invisible things, myself, my children, and my ancestors. */
/*   if(  hitmod->obstacle_return  && (!stg_model_is_related(mod,hitmod)) )  */
/*     return 1; */
  
/*   return 0; // no match */
/* }	 */



// Check to see if the given pose will yield a collision with obstacles.
// Returns a pointer to the first entity we are in collision with, and stores
// the location of the hit in hitx,hity (if non-null)
// Returns NULL if not collisions.
// This function is useful for writing position devices.
/* stg_model_t* stg_model_test_collision( stg_model_t* mod,  */
/* 				       //stg_pose_t* pose,  */
/* 				       double* hitx, double* hity ) */
/* { */
/*   return 0; */
  
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
  
/*   // raytrace along all our rectangles. expensive, but most vehicles */
/*   // will just be a single rect, grippers 3 rects, etc. not too bad. */
  
/*   size_t count=0; */
/*   stg_polygon_t* polys = stg_model_get_polygons(mod, &count); */

/*   // no body? no collision */
/*   if( count < 1 ) */
/*     return NULL; */

/*   // loop over all polygons */
/*   int q; */
/*   for( q=0; q<count; q++ ) */
/*     { */
/*       stg_polygon_t* poly = &polys[q]; */
      
/*       int point_count = poly->points->len; */

/*       // loop over all points in this polygon */
/*       int p; */
/*       for( p=0; p<point_count; p++ ) */
/* 	{ */
/* 	  stg_point_t* pt1 = &g_array_index( poly->points, stg_point_t, p );	   */
/* 	  stg_point_t* pt2 = &g_array_index( poly->points, stg_point_t, (p+1) % point_count); */
	  
/* 	  stg_pose_t pp1; */
/* 	  pp1.x = pt1->x; */
/* 	  pp1.y = pt1->y; */
/* 	  pp1.a = 0; */
	  
/* 	  stg_pose_t pp2; */
/* 	  pp2.x = pt2->x; */
/* 	  pp2.y = pt2->y; */
/* 	  pp2.a = 0; */
	  
/* 	  stg_pose_t p1; */
/* 	  stg_pose_t p2; */
	  
/* 	  // shift the line points into the global coordinate system */
/* 	  stg_pose_sum( &p1, &pose, &pp1 ); */
/* 	  stg_pose_sum( &p2, &pose, &pp2 ); */
	  
/* 	  //printf( "tracing %.2f %.2f   %.2f %.2f\n",  p1.x, p1.y, p2.x, p2.y ); */
	  
/* 	  itl_t* itl = itl_create( p1.x, p1.y, p2.x, p2.y,  */
/* 				   mod->world->matrix,  */
/* 				   PointToPoint ); */
	  
/* 	  //stg_rtk_fig_t* fig = stg_model_fig_create( mod, "foo", NULL, STG_LAYER_POSITIONDATA ); */
/* 	  //stg_rtk_fig_line( fig, p1.x, p1.y, p2.x, p2.y ); */
	  
/* 	  stg_model_t* hitmod = itl_first_matching( itl, lines_raytrace_match, mod ); */
	  
	  
/* 	  if( hitmod ) */
/* 	    { */
/* 	      if( hitx ) *hitx = itl->x; // report them */
/* 	      if( hity ) *hity = itl->y;	   */
/* 	      itl_destroy( itl ); */
/* 	      return hitmod; // we hit this object! stop raytracing */
/* 	    } */

/* 	  itl_destroy( itl ); */
/* 	} */
/*     } */

/*   return NULL;  // done  */
//}



/* typedef struct */
/* { */
/*   stg_model_t* mod; */
/*   GList** list; */
/* } _modlist_t; */


/* int bboxes_overlap( stg_bbox3d_t* box1, stg_bbox3d_t* box2 ) */
/* { */
/*   // return TRUE iff any corner of box 2 is contained within box 1 */
/*   return( box1->x.min <= box2->x.max && */
/* 	  box2->x.min <= box1->x.max && */
/* 	  box1->y.min <= box2->y.max && */
/* 	  box2->y.min <= box1->y.max ); */
/* } */


/* GList* stg_model_overlappers( stg_model_t* mod, GList* models ) */
/* { */
/*   GList *it; */
/*   stg_model_t* candidate; */
/*   GList *overlappers = NULL; */

/*   //printf( "\nfinding overlappers of %s\n", mod->token ); */

/*   for( it=models; it; it=it->next ) */
/*     { */
/*       candidate = (stg_model_t*)it->data; */

/*       if( candidate == mod ) */
/* 	continue; */

/*       printf( "model %s tested against %s...", mod->token, candidate->token ); */
      
/*       if( bboxes_overlap( &mod->bbox, &candidate->bbox ) ) */
/* 	{ */
/* 	  overlappers = g_list_append( overlappers, candidate ); */
/* 	  puts(" OVERLAP!" ); */
/* 	} */
/*       else */
/* 	puts( "" ); */

/*       if( candidate->children ) */
/* 	overlappers = g_list_concat( overlappers,  */
/* 				     stg_model_overlappers( mod, candidate->children )); */
/*     } */
  
/*   return overlappers; */
/* } */


// add any line segments contained by this model to the list of line
// segments in world coordinates.
/* GList* model_append_segments( stg_model_t* mod, GList* segments ) */
/* { */
/*   stg_pose_t gpose; */
/*   stg_model_get_global_pose( mod, &gpose ); */
/*   double x = gpose.x; */
/*   double y = gpose.y; */
/*   double a = gpose.a; */

/*   int p; */
/*   for( p=0; p<mod->polygons_count; p++ ) */
/*     { */
/*       stg_polygon_t* poly =  &mod->polygons[p]; */

/*       // need at least three points for a meaningful polygon */
/*       if( poly->points->len > 2 ) */
/* 	{ */
/* 	  int count = poly->points->len; */
/* 	  int p; */
/* 	  for( p=0; p<count; p++ ) // for */
/* 	    { */
/* 	      stg_point_t* pt1 = &g_array_index( poly->points, stg_point_t, p );	   */
/* 	      stg_point_t* pt2 = &g_array_index( poly->points, stg_point_t, (p+1) % count); */

/* 	      stg_line_t *line = malloc(sizeof */
/* 	      line.x1 = x + pt1->x * cos(a) - pt1->y * sin(a); */
/* 	      line.y1 = y + pt1->x * sin(a) + pt1->y * cos(a);  */
	      
/* 	      line.x2 = x + pt2->x * cos(a) - pt2->y * sin(a); */
/* 	      line.y2 = y + pt2->x * sin(a) + pt2->y * cos(a);  */
	      
/* 	      //stg_matrix_lines( matrix, &line, 1, object ); */
/* 	    } */
/* 	} */
/*       //else */
/*       //PRINT_WARN( "attempted to matrix render a polygon with less than 3 points" );  */
/*     } */
      



/* } */

/* stg_model_t* stg_model_test_collision2( stg_model_t* mod,  */
/* 					//stg_pose_t* pose,  */
/* 					double* hitx, double* hity ) */
/* { */
/*  /\*  int ch; *\/ */
/* /\*   stg_model_t* child_hit = NULL; *\/ */
/* /\*   for(ch=0; ch < mod->children->len; ch++ ) *\/ */
/* /\*     { *\/ */
/* /\*       stg_model_t* child = g_ptr_array_index( mod->children, ch ); *\/ */
/* /\*       child_hit = stg_model_test_collision2( child, hitx, hity ); *\/ */
/* /\*       if( child_hit ) *\/ */
/* /\* 	return child_hit; *\/ */
/* /\*     } *\/ */
  

/*   stg_pose_t pose; */
/*   memcpy( &pose, &mod->geom.pose, sizeof(pose)); */
/*   stg_model_local_to_global( mod, &pose ); */
  
/*   //return NULL; */
  
/*   // raytrace along all our rectangles. expensive, but most vehicles */
/*   // will just be a single rect, grippers 3 rects, etc. not too bad. */
  
/*   //size_t count=0; */
/*   //stg_polygon_t* polys = stg_model_get_polygons(mod, &count); */

/*   // no body? no collision */
/*   //if( count < 1 ) */
/*   //return NULL; */

/*   // first find the models whose bounding box overlaps with this one */

/* /\*   printf ("COLLISION TESTING MODEL %s\n", mod->token ); *\/ */
/*   GList* candidates = NULL;//stg_model_overlappers( mod, mod->world->children ); *\/ */

/* /\*   if( candidates ) *\/ */
/* /\*     printf( "mod %s overlappers: %d\n", mod->token, g_list_length( candidates )); *\/ */
/* /\*   else *\/ */
/* /\*     printf( "mod %s has NO overlappers\n", mod->token ); *\/ */


/*   // now test every line segment in the candidates for intersection */
/*   // and record the closest hitpoint */
/* /\*   GList* model_segments = model_append_segments( mod, NULL ); *\/ */
  
/* /\*   GList* obstacle_segments = NULL; *\/ */
  
/* /\*   GList* it; *\/ */
/* /\*   for( it=candidates; it; it=it->next ) *\/ */
/* /\*     obstacle_segments =  *\/ */
/* /\*       model_append_segments( (stg_model_t*)it->data, obstacle_segments ); *\/ */
 

/*   if( candidates ) */
/*     g_list_free( candidates ); */
	  


/*   return NULL;  // done  */
/* } */


// int StgModel::UpdatePose( void )
// {
//   printf( "model %s update pose\n", token );
//   // do nothing yet
// }

// int stg_model_update_pose( stg_model_t* mod )
// { 
//   PRINT_DEBUG4( "pose update model %d (vel %.2f, %.2f %.2f)", 
// 		mod->id, mod->velocity.x, mod->velocity.y, mod->velocity.a );
 
//   stg_velocity_t gvel;
//   stg_model_get_global_velocity( mod, &gvel );
      
//   stg_pose_t gpose;
//   stg_model_get_global_pose( mod, &gpose );

//   // convert msec to sec
//   double interval = (double)mod->world->sim_interval / 1000.0;
  

//   stg_pose_t old_pose;
//   memcpy( &old_pose, &mod->pose, sizeof(old_pose));

//   // compute new pose
//   //gpose.x += gvel.x * interval;
//   //gpose.y += gvel.y * interval;
//   //gpose.a += gvel.a * interval;
//   mod->pose.x += (mod->velocity.x * cos(mod->pose.a) - mod->velocity.y * sin(mod->pose.a)) * interval;
//   mod->pose.y += (mod->velocity.x * sin(mod->pose.a) + mod->velocity.y * cos(mod->pose.a)) * interval;
//   mod->pose.a += mod->velocity.a * interval;

//   // check this model and all it's children at the new pose
//   double hitx=0, hity=0, hita=0;
//   stg_model_t* hitthing =
//     //NULL; //stg_model_test_collision( mod, &hitx, &hity );
//     stg_model_test_collision2( mod, &hitx, &hity );

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
// }



// find the top-level model above mod;
StgModel* StgModel::RootModel( void )
{
  while( this->parent )
    return( this->parent->RootModel() );
  return this;
}

int stg_model_tree_to_ptr_array( StgModel* root, GPtrArray* array )
{
  g_ptr_array_add( array, root );
  
  //printf( " added %s to array at %p\n", root->token, array );
  
  int added = 1;
  
  for(GList* it=root->Children(); it; it=it->next )
    added += stg_model_tree_to_ptr_array( (StgModel*)it->data, array );
  
  return added;
}



