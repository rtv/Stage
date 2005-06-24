
#define PACKPOSE(P,X,Y,A) {P->x=X; P->y=Y; P->a=A;}

#include <limits.h> // for PATH_MAX
#include <assert.h>
#include <math.h>

//#define DEBUG
//#undef DEBUG

#define PPPOSE "pose"

#include "stage_internal.h"
#include "gui.h"

  // basic model
#define STG_DEFAULT_MASS 10.0  // kg
#define STG_DEFAULT_POSEX 0.0  // start at the origin by default
#define STG_DEFAULT_POSEY 0.0
#define STG_DEFAULT_POSEA 0.0
#define STG_DEFAULT_GEOM_POSEX 0.0 // no origin offset by default
#define STG_DEFAULT_GEOM_POSEY 0.0
#define STG_DEFAULT_GEOM_POSEA 0.0
#define STG_DEFAULT_GEOM_SIZEX 1.0 // 1m square by default
#define STG_DEFAULT_GEOM_SIZEY 1.0
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
#define STG_DEFAULT_GUI_MOVEMASK (STG_MOVE_TRANS | STG_MOVE_ROT)
#define STG_DEFAULT_GUI_NOSE FALSE
#define STG_DEFAULT_GUI_GRID FALSE
#define STG_DEFAULT_GUI_BOUNDARY FALSE

//extern int _stg_disable_gui;

/** @defgroup basic Basic model
    
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
  size [0 0]
  origin [0 0 0]
  velocity [0 0 0]

  color "red" # body color

  # determine how the model appears in various sensors
  obstacle_return 1
  laser_return 1
  ranger_return 1
  blobfinder_return 1
  fiducial_return 1

  # GUI properties
  gui_nose 0
  gui_grid 0
  gui_boundary 0
  gui_movemask ?

  # body shape
  line_count 4
  line[0][0 0 1 0]
  line[1][1 0 1 1]
  line[2][1 1 0 1]
  line[3][0 1 0 0]

  bitmap ""
  bitmap_resolution 0
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
- bitmap_resolution [meters:float]
 - (TODO: fix this - it messes up the object placement a little) alternative way to set the model's size. Used with the bitmap option, this sets the model's size according to the size of the bitmap file, by multiplying the width and height of the bitmap, measured in pixels, by this scaling factor. 
- gui_nose [bool]
  - if 1, draw a nose on the model showing its heading (positive X axis)
- gui_grid [bool]
  - if 1, draw a scaling grid over the model
- gui_movemask [bool]
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
- ranger_return [bool]
   - iff 1, this model can be detected by a ranger.
*/

/*
TODO

- friction float
  - [WARNING: Friction model is not yet implemented; details may change] if > 0 the model can be pushed around by other moving objects. The value determines the proportion of velocity lost per second. For example, 0.1 would mean that the object would lose 10% of its speed due to friction per second. A value of zero (the default) means this model can not be pushed around (infinite friction). 
*/


  



/// convert a global pose into the model's local coordinate system
void stg_model_global_to_local( stg_model_t* mod, stg_pose_t* pose )
{
  //printf( "g2l global pose %.2f %.2f %.2f\n",
  //  pose->x, pose->y, pose->a );

  // get model's global pose
  stg_pose_t org;
  stg_model_get_global_pose( mod, &org );

  //printf( "g2l global origin %.2f %.2f %.2f\n",
  //  org.x, org.y, org.a );

  // compute global pose in local coords
  double sx =  (pose->x - org.x) * cos(org.a) + (pose->y - org.y) * sin(org.a);
  double sy = -(pose->x - org.x) * sin(org.a) + (pose->y - org.y) * cos(org.a);
  double sa = pose->a - org.a;

  PACKPOSE(pose,sx,sy,sa);

  //printf( "g2l local pose %.2f %.2f %.2f\n",
  //  pose->x, pose->y, pose->a );
}


/// generate the default name for a model, based on the name of its
/// parent, and its type. This can be overridden in the world file.
int stg_model_create_name( stg_model_t* mod )
{
  assert( mod );
  assert( mod->token );
  
  PRINT_DEBUG1( "model's default name is %s",
		mod->token );
  if( mod->parent )
    PRINT_DEBUG1( "model's parent's name is %s",
		  mod->parent->token );

  char buf[512];  
  if( mod->parent == NULL )
    snprintf( buf, 255, "%s:%d", 
	      mod->token, 
	      mod->world->child_type_count[mod->type] );
  else
    snprintf( buf, 255, "%s.%s:%d", 
	      mod->parent->token,
	      mod->token, 
	      mod->parent->child_type_count[mod->type] );
  
  free( mod->token );
  // return a new string just long enough
  mod->token = strdup(buf);

  return 0; //ok
}


stg_polygon_t* unit_polygon_create( void )
{
  stg_point_t pts[4];
  pts[0].x = 0;
  pts[0].y = 0;
  pts[1].x = 1;
  pts[1].y = 0;
  pts[2].x = 1;
  pts[2].y = 1;
  pts[3].x = 0;
  pts[3].y = 1;
  
  stg_polygon_t* poly = stg_polygon_create();
  stg_polygon_set_points( poly, pts, 4 );
  
  return poly;
}

/*
/// grow a model by adding [sz] bytes to the end of its struct
stg_model_t* stg_model_extend( stg_model_t* mod, size_t sz )
{
  stg_model_map_with_children( mod, 0 );

  // grow the model to allow for our type-specific extensions
  mod = realloc( mod, sizeof(stg_model_t)+sz );

  // hack! repair our model callback pointer following the realloc
  if( mod->world->win )
    mod->gui.top->userdata = mod;

  // zero our extension data
  memset( mod->extend, 0, sz );

  stg_model_map_with_children( mod, 1 );

  return mod;
}

/// create a model with [extra_len] bytes allocated at the end of the
/// struct, accessible via the 'extend' field.
stg_model_t* stg_model_create_extended( stg_world_t* world, 
					stg_model_t* parent,
					stg_id_t id, 
					stg_model_type_t type,
					char* token,
					size_t extra_len )
{
  return stg_model_extend( stg_model_create( world, parent, 
					     id, type, token ),
			   extra_len );
}

*/

void test_storage( stg_model_t* mod, stg_property_t* prop, 
		   void* data, size_t len )
{
  puts( "override storage" );
  prop->data = realloc( prop->data, len );
  prop->len = len;
  memcpy( prop->data, data, len );
}


/// create a new model
stg_model_t* stg_model_create( stg_world_t* world, 
			       stg_model_t* parent,
			       stg_id_t id, 
			       stg_model_type_t type,
			       char* token )
{  
  stg_model_t* mod = calloc( sizeof(stg_model_t),1 );
  
  assert( pthread_mutex_init( &mod->data_mutex, NULL ) == 0 );
  assert( pthread_mutex_init( &mod->cmd_mutex, NULL ) == 0 );
  assert( pthread_mutex_init( &mod->cfg_mutex, NULL ) == 0 );
  
  mod->id = id;

  mod->disabled = FALSE;
  mod->world = world;
  mod->parent = parent; 
  mod->type = type;
  mod->token = strdup(token); // this will be immediately replaced by
			      // model_create_name  
  // create a default name for the model that's derived from its
  // ancestors' names and its worldfile token
  //model_create_name( mod );

  PRINT_DEBUG4( "creating model %d.%d(%s) type %s", 
		mod->world->id, mod->id, 
		mod->token, stg_model_type_string(mod->type) );
  
  PRINT_DEBUG1( "original token: %s", token );

  // add this model to its parent's list of children (if any)
  if( parent) g_ptr_array_add( parent->children, mod );       
  
  // having installed the paremt, it's safe to creat the GUI components
  if( mod->world->win )
    gui_model_create( mod );

  // create this model's empty list of children
  mod->children = g_ptr_array_new();

  mod->data = NULL;
  mod->data_len = 0;
  
  mod->cmd = NULL;
  mod->cmd_len = 0;
  
  mod->cfg = NULL;
  mod->cfg_len = 0;

  mod->laser_return = LaserVisible;
  mod->obstacle_return = TRUE;
  mod->fiducial_return = FiducialNone;
  mod->ranger_return = TRUE;
  
  // sensible defaults
  mod->stall = FALSE;

  mod->pose.x = STG_DEFAULT_POSEX;
  mod->pose.y = STG_DEFAULT_POSEY;
  mod->pose.a = STG_DEFAULT_POSEA;
  
  stg_model_set_prop_storage( mod, PPPOSE, test_storage );
  stg_model_set_prop( mod, PPPOSE, &mod->pose, sizeof(stg_pose_t) );

  mod->geom.pose.x = STG_DEFAULT_GEOM_POSEX;
  mod->geom.pose.y = STG_DEFAULT_GEOM_POSEY;
  mod->geom.pose.a = STG_DEFAULT_GEOM_POSEA;
  mod->geom.size.x = STG_DEFAULT_GEOM_SIZEX;
  mod->geom.size.y = STG_DEFAULT_GEOM_SIZEY;

  mod->boundary = FALSE;
  
  stg_guifeatures_t gf;
  gf.show_data = 1;
  gf.show_cmd = 0;
  gf.show_cfg = 0;
  gf.nose =  STG_DEFAULT_GUI_NOSE;
  gf.grid = STG_DEFAULT_GUI_GRID;
  gf.outline = 1;
  gf.movemask = STG_DEFAULT_GUI_MOVEMASK;  
  stg_model_set_guifeatures( mod, &gf );

  mod->blob_return = TRUE;

  // zero velocity
  memset( &mod->velocity, 0, sizeof(mod->velocity) );

  mod->color = stg_lookup_color( "red" );
  
  mod->polygons = g_array_new( FALSE, FALSE, sizeof(stg_polygon_t) );
  
  stg_point_t pts[4];
  pts[0].x = 0;
  pts[0].y = 0;
  pts[1].x = 1;
  pts[1].y = 0;
  pts[2].x = 1;
  pts[2].y = 1;
  pts[3].x = 0;
  pts[3].y = 1;
  
  stg_polygon_t* poly = stg_polygon_create();
  stg_polygon_set_points( poly, pts, 4 );
  stg_model_set_polygons( mod, poly, 1 );
  free(poly);
  
  // init the arbitrary datalist structure
  g_datalist_init( &mod->props);
	
  // install the default functions
  mod->f_startup = NULL;
  mod->f_shutdown = NULL;
  mod->f_render_data = NULL;
  mod->f_render_cmd = NULL;
  mod->f_render_cfg = NULL;
  mod->f_update = _model_update;

  // possible extensions?
  //mod->f_set_data = _model_set_data;
  //mod->f_get_data =  _model_get_data;
  //mod->f_set_command = _model_set_cmd;
  //mod->f_get_command = _model_get_cmd;
  //mod->f_set_config = _model_set_cfg;
  //mod->f_get_config = _model_get_cfg;

  // TODO

  // mod->friction = 0.0;
  
  PRINT_DEBUG4( "finished model %d.%d(%s) type %s", 
		mod->world->id, mod->id, 
		mod->token, stg_model_type_string(mod->type) );
  return mod;
}

//void* stg_model_get_prop( stg_model_t* mod, char* name )
//{
//return g_datalist_get_data( &mod->props, name );
//}

//void stg_model_set_prop( stg_model_t* mod, char* name, void* data )
//{
// g_datalist_set_data( &mod->props, name, data );
//}

/// free the memory allocated for a model
void stg_model_destroy( stg_model_t* mod )
{
  assert( mod );
  
  if( mod->world->win )
    gui_model_destroy( mod );

  if( mod->parent && mod->parent->children ) g_ptr_array_remove( mod->parent->children, mod );
  if( mod->children ) g_ptr_array_free( mod->children, FALSE );
  if( mod->data ) free( mod->data );
  if( mod->cmd ) free( mod->cmd );
  if( mod->cfg ) free( mod->cfg );
  if( mod->token ) free( mod->token );

  if( mod->polygons->data ) 
    stg_polygons_destroy( (stg_polygon_t*)mod->polygons->data, mod->polygons->len ); 
  
  free( mod );
}


void model_destroy_cb( gpointer mod )
{
  stg_model_destroy( (stg_model_t*)mod );
}

int stg_model_is_antecedent( stg_model_t* mod, stg_model_t* testmod )
{
  if( mod == NULL )
    return FALSE;

  if( mod == testmod )
    return TRUE;
  
  if( stg_model_is_antecedent( mod->parent, testmod ))
    return TRUE;
  
  // neither mod nor a child of mod matches testmod
  return FALSE;
}

/// returns TRUE if model [testmod] is a descendent of model [mod]
int stg_model_is_descendent( stg_model_t* mod, stg_model_t* testmod )
{
  if( mod == testmod )
    return TRUE;

  assert( mod->children );

  int ch;
  for(ch=0; ch < mod->children->len; ch++ )
    {
      stg_model_t* child = g_ptr_array_index( mod->children, ch );
      if( stg_model_is_descendent( child, testmod ))
	return TRUE;
    }
  
  // neither mod nor a child of mod matches testmod
  return FALSE;
}

/// returns 1 if model [mod1] and [mod2] are in the same model tree
int stg_model_is_related( stg_model_t* mod1, stg_model_t* mod2 )
{
  if( mod1 == mod2 )
    return TRUE;

  // find the top-level model above mod1;
  stg_model_t* t = mod1;
  while( t->parent )
    t = t->parent;

  // now seek mod2 below t
  return stg_model_is_descendent( t, mod2 );
}

/// get the model's velocity in the global frame
void stg_model_global_velocity( stg_model_t* mod, stg_velocity_t* gvel )
{
  stg_pose_t gpose;
  stg_model_get_global_pose( mod, &gpose );

  double cosa = cos( gpose.a );
  double sina = sin( gpose.a );
  
  gvel->x = mod->velocity.x * cosa - mod->velocity.y * sina;
  gvel->y = mod->velocity.x * sina + mod->velocity.y * cosa;
  gvel->a = mod->velocity.a;

  //printf( "local velocity %.2f %.2f %.2f\nglobal velocity %.2f %.2f %.2f\n",
  //  mod->velocity.x, mod->velocity.y, mod->velocity.a,
  //  gvel->x, gvel->y, gvel->a );
}

// set the model's velocity in the global frame
/* void stg_model_set_global_velocity( stg_model_t* mod, stg_velocity_t* gvel ) */
/* { */
/*   // TODO - do this properly */

/*   //stg_pose_t gpose; */
  
/*   stg_velocity_t lvel; */
/*   lvel.x = gvel->x; */
/*   lvel.y = gvel->y; */
/*   lvel.a = gvel->a; */

/*   stg_model_set_velocity( mod, &lvel ); */
/* } */

// get the model's position in the global frame
void  stg_model_get_global_pose( stg_model_t* mod, stg_pose_t* gpose )
{ 
  stg_pose_t parent_pose;
  
  // find my parent's pose
  if( mod->parent )
    {
      stg_model_get_global_pose( mod->parent, &parent_pose );
      
      gpose->x = parent_pose.x + mod->pose.x * cos(parent_pose.a) 
	- mod->pose.y * sin(parent_pose.a);
      gpose->y = parent_pose.y + mod->pose.x * sin(parent_pose.a) 
	+ mod->pose.y * cos(parent_pose.a);
      gpose->a = NORMALIZE(parent_pose.a + mod->pose.a);
    }
  else
    memcpy( gpose, &mod->pose, sizeof(mod->pose));
}
    
    
// should one day do all this with affine transforms for neatness

/** convert a pose in this model's local coordinates into global
    coordinates */
void stg_model_local_to_global( stg_model_t* mod, stg_pose_t* pose )
{  
  stg_pose_t origin;   
  stg_model_get_global_pose( mod, &origin );
  stg_pose_sum( pose, &origin, pose );
}


/// recursively map a model and all it's descendents
void stg_model_map_with_children(  stg_model_t* mod, gboolean render )
{
  // call this function for all the model's children
  int ch;
  for( ch=0; ch<mod->children->len; ch++ )
    stg_model_map_with_children( (stg_model_t*)g_ptr_array_index(mod->children, ch), 
				 render);  
  // now map the model
  stg_model_map( mod, render );
}

/// if render is true, render the model into the matrix, else unrender
/// the model
void stg_model_map( stg_model_t* mod, gboolean render )
{
  assert( mod );
  
  size_t count=0;
  stg_polygon_t* polys = stg_model_get_polygons(mod, &count);
  
  if( render )
    {      
      // get model's global pose
      stg_pose_t org;
      memcpy( &org, &mod->geom.pose, sizeof(org));
      stg_model_local_to_global( mod, &org );
      
      if( count && polys )    
	stg_matrix_polygons( mod->world->matrix, 
			     org.x, org.y, org.a,
			     polys, count, 
			     mod );  
      else
	PRINT_ERR1( "expecting %d polygons but have no data", (int)count );
      
      if( mod->boundary )    
	stg_matrix_rectangle( mod->world->matrix,
			      org.x, org.y, org.a,
			      mod->geom.size.x,
			      mod->geom.size.y,
			      mod ); 
    }
  else
    stg_matrix_remove_object( mod->world->matrix, mod );
}



void model_update_cb( gpointer key, gpointer value, gpointer user )
{
  stg_model_update( (stg_model_t*)value );
}


void stg_model_subscribe( stg_model_t* mod )
{
  mod->subs++;
  
  //printf( "subscribe %d\n", mod->subs );
  
  // if this is the first sub, call startup
  if( mod->subs == 1 )
    stg_model_startup(mod);
}

void stg_model_unsubscribe( stg_model_t* mod )
{
  mod->subs--;
  
  //printf( "unsubscribe %d\n", mod->subs );

  // if this is the last sub, call shutdown
  if( mod->subs < 1 )
    stg_model_shutdown(mod);
}


void pose_invert( stg_pose_t* pose )
{
  pose->x = -pose->x;
  pose->y = -pose->y;
  pose->a = pose->a;
}

void stg_model_print( stg_model_t* mod )
{
  printf( "   model %d:%d:%s\n", mod->world->id, mod->id, mod->token );
}

void model_print_cb( gpointer key, gpointer value, gpointer user )
{
  stg_model_print( (stg_model_t*)value );
}



void stg_get_default_pose( stg_pose_t* pose )
{
  assert(pose);
  pose->x = STG_DEFAULT_GEOM_POSEX;
  pose->y = STG_DEFAULT_GEOM_POSEY;
  pose->a = STG_DEFAULT_GEOM_POSEA;
}

void stg_get_default_geom( stg_geom_t* geom )
{
  assert(geom);
  geom->pose.x = STG_DEFAULT_GEOM_POSEX;
  geom->pose.y = STG_DEFAULT_GEOM_POSEY;
  geom->pose.a = STG_DEFAULT_GEOM_POSEA;  
  geom->size.x = STG_DEFAULT_GEOM_SIZEX;
  geom->size.y = STG_DEFAULT_GEOM_SIZEY;
}


// DATA/COMMAND/CONFIG ----------------------------------------------------
// these set and get the generic buffers for derived types
//

int _get_something( void* dest, size_t dest_len, 
		    void* src, size_t src_len, 
		    pthread_mutex_t* mutex )
{
  size_t copy_len = MIN( dest_len, src_len );
  pthread_mutex_lock( mutex );
  memcpy( dest, src, copy_len );
  pthread_mutex_unlock( mutex );
  return copy_len;
}

int stg_model_get_data( stg_model_t* mod, void* dest, size_t len )
{
  return _get_something( dest, len, mod->data, mod->data_len, &mod->data_mutex );
}

int stg_model_get_command( stg_model_t* mod, void* dest, size_t len )
{
  return _get_something( dest, len, mod->cmd, mod->cmd_len, &mod->cmd_mutex );
}

int stg_model_get_config( stg_model_t* mod, void* dest, size_t len )
{
  return _get_something( dest, len, mod->cfg, mod->cfg_len, &mod->cfg_mutex );
}

int stg_model_set_data( stg_model_t* mod, void* data, size_t len )
{
  pthread_mutex_lock( &mod->data_mutex );
  mod->data = realloc( mod->data, len );
  memcpy( mod->data, data, len );    
  mod->data_len = len;
  pthread_mutex_unlock( &mod->data_mutex );

  // if a callback was registered, call it
  if( mod->data_notify )
    (*mod->data_notify)(mod->data_notify_arg, data, len );
  
  if( mod->world->win )
    {
      if( mod->world->win->render_data_flag[ mod->type ] && 
	  !mod->world->win->disable_data )
	gui_model_render_data( mod );
      else
	if( mod->gui.data  )
	  {
	    stg_rtk_fig_clear(mod->gui.data);
	    stg_rtk_fig_destroy( mod->gui.data);
	    mod->gui.data = NULL;
	  }
    }
  
  PRINT_DEBUG3( "model %d(%s) put data of %d bytes",
		mod->id, mod->token, (int)mod->data_len);
  return 0; //ok
}

int stg_model_set_command( stg_model_t* mod, void* cmd, size_t len )
{
  pthread_mutex_lock( &mod->cmd_mutex );  
  mod->cmd = realloc( mod->cmd, len );
  memcpy( mod->cmd, cmd, len );    
  mod->cmd_len = len;  
  pthread_mutex_unlock( &mod->cmd_mutex );
  
  if( mod->world->win )
    gui_model_render_command( mod );
  
  PRINT_DEBUG3( "model %d(%s) put command of %d bytes",
		mod->id, mod->token, (int)mod->cmd_len);
  
  return 0; //ok
}

int stg_model_set_config( stg_model_t* mod, void* cfg, size_t len )
{ 
  pthread_mutex_lock( &mod->cfg_mutex );  
  mod->cfg = realloc( mod->cfg, len );
  memcpy( mod->cfg, cfg, len );    
  mod->cfg_len = len;
  pthread_mutex_unlock( &mod->cfg_mutex );  

  if( mod->world->win )
    gui_model_render_config( mod );
  
  PRINT_DEBUG3( "model %d(%s) put command of %d bytes",
		mod->id, mod->token, (int)mod->cfg_len);
  return 0; //ok
}

// default update function that implements velocities for all objects
int _model_update( stg_model_t* mod )
{
  mod->interval_elapsed = 0;
  
  // now move the model if it has any velocity
  if( (mod->velocity.x || mod->velocity.y || mod->velocity.a ) )
    stg_model_update_pose( mod );

  return 0; //ok
}

int stg_model_update( stg_model_t* mod )
{
  return( mod->f_update ? mod->f_update(mod) : 0 );
}

int stg_model_startup( stg_model_t* mod )
{
  if( mod->f_startup == NULL )
    PRINT_ERR1( "model %s has no startup function registered", mod->token ); 
  
  //assert(mod->f_startup);

  if( mod->f_startup )
    return mod->f_startup(mod);
  
  return 0; //ok
}

int stg_model_shutdown( stg_model_t* mod )
{
  if( mod->f_shutdown == NULL )
    PRINT_ERR1( "model %s has no shutdown function registered", mod->token ); 
  
  //assert(mod->f_shutdown );
  
  if( mod->f_shutdown )
    return mod->f_shutdown(mod);
  
  return 0; //ok
}



//------------------------------------------------------------------------
// basic model properties



void stg_model_get_pose( stg_model_t* model, stg_pose_t* pose )
{
  assert(model);
  assert(pose);
  memcpy( pose, &model->pose, sizeof(stg_pose_t));
}

void stg_model_get_velocity( stg_model_t* mod, stg_velocity_t* dest )
{
  assert(mod);
  assert(dest);
  memcpy( dest, &mod->velocity, sizeof(mod->velocity));
}

void stg_model_get_geom( stg_model_t* mod, stg_geom_t* dest )
{
  assert(mod);
  assert(dest);
  memcpy( dest, &mod->geom, sizeof(mod->geom));
}

void stg_model_get_color( stg_model_t* mod, stg_color_t* dest )
{
  assert(mod);
  assert(dest);
  memcpy( dest, &mod->color, sizeof(mod->color));
}

void stg_model_get_mass( stg_model_t* mod, stg_kg_t* dest )
{
  assert(mod);
  assert(dest);
  memcpy( dest, &mod->obstacle_return, sizeof(mod->obstacle_return));
}

void stg_model_get_boundary( stg_model_t* mod, stg_bool_t* dest )
{
  assert(mod);
  assert(dest);
  memcpy( dest, &mod->boundary, sizeof(mod->boundary));
}

void stg_model_get_guifeatures( stg_model_t* mod, stg_guifeatures_t* dest )
{
  assert(mod);
  assert(dest);
  memcpy( dest, &mod->guifeatures, sizeof(mod->guifeatures));
}

void stg_model_get_laserreturn( stg_model_t* mod, stg_laser_return_t* dest )
{
  assert(mod);
  assert(dest);
  memcpy( dest, &mod->laser_return, sizeof(mod->laser_return));
}

void stg_model_get_gripperreturn( stg_model_t* mod, stg_gripper_return_t* dest )
{
  assert(mod);
  assert(dest);
  memcpy( dest, &mod->gripper_return, sizeof(mod->gripper_return));
}

void stg_model_get_rangerreturn( stg_model_t* mod, stg_ranger_return_t* dest )
{
  assert(mod);
  assert(dest);
  memcpy( dest, &mod->ranger_return, sizeof(mod->ranger_return));
}

void stg_model_get_fiducialreturn( stg_model_t* mod,stg_fiducial_return_t* dest )
{
  assert(mod);
  assert(dest);
  memcpy( dest, &mod->fiducial_return, sizeof(mod->fiducial_return));
}

//void stg_model_get_friction( stg_model_t* mod,stg_friction_t* dest )
//{
//assert(mod);
//assert(dest);
//memcpy( dest, &mod->friction, sizeof(mod->friction));
//}


void stg_model_get_obstaclereturn( stg_model_t* mod, stg_obstacle_return_t* dest )
{
  assert(mod);
  assert(dest);
  memcpy( dest, &mod->obstacle_return, sizeof(mod->obstacle_return));
}


void stg_model_get_blobreturn( stg_model_t* mod, stg_blob_return_t* dest )
{
  assert(mod);
  assert(dest);
  memcpy( dest, &mod->blob_return, sizeof(mod->blob_return));
}

stg_polygon_t* stg_model_get_polygons( stg_model_t* mod, size_t* poly_count )
{
  *poly_count = mod->polygons->len;
  return (stg_polygon_t*)mod->polygons->data;
}

// create a new, zeroed property
stg_property_t* stg_property_create( void )
{
  //puts( "creating new property" );
  return((stg_property_t*)calloc(sizeof(stg_property_t),1));
}

void stg_property_destroy( stg_property_t* prop )
{
  // empty the list
  if( prop->callbacks )
    g_list_free( prop->callbacks );
  
  free( prop );
}

int _property_store( stg_model_t* mod, char* propname, 
		     void* data, size_t len, void* user)
{
  printf( "model %s property %s changed with %d bytes user %p\n",
	  mod->token, propname, (int)len, user );

  return 0;
}

void stg_property_print_cb( GQuark key, gpointer data, gpointer user )
{
  printf( "%s%s %d bytes\n", 
	  (char*)user, 
	  g_quark_to_string(key), 
	  ((stg_property_t*)data)->len );
}

//const size_t STG_PROPNAME_MAX = 128;
#define STG_PROPNAME_MAX 128

typedef struct 
{
  stg_model_t* mod;
  char propname[STG_PROPNAME_MAX];
  void* data;
  size_t len;
  void* user;
} stg_property_callback_args_t;
    

// call a property callback
void stg_property_callback( stg_property_callback_t cb, 
			    stg_property_callback_args_t* args )
{
  //printf( "calling %p(%p, %s, %p, %d, %p )\n",
  //  cb, args->mod, args->propname, args->data, args->len, args->user );
  
  cb( args->mod, args->propname, args->data, args->len, args->user );
}

// wrapper for stg_property_callback
void stg_property_callback_cb( gpointer data, gpointer user )
{
  stg_property_callback( (stg_property_callback_t)data,
			 (stg_property_callback_args_t*)user );  
}


// call every property callback listed  in a property
void stg_property_callback_list( char* propname, 
				 stg_property_t* prop,
				 stg_property_callback_args_t* args )
{  
  //printf( "checking callbacks for %s:%s\n", 
  //  args->mod->token, 
  //  propname );
  
  if( prop->callbacks ) 
    {
      strncpy( args->propname, propname, STG_PROPNAME_MAX); 
      g_list_foreach( prop->callbacks, stg_property_callback_cb, args );
    }
}

// wrapper for stg_property_callback_list()
void stg_property_callback_list_cb( GQuark key, gpointer data, gpointer user )
{
  stg_property_callback_list( g_quark_to_string(key),
			      (stg_property_t*)data,
			      (stg_property_callback_args_t*)user );
}



int stg_model_set_prop_storage( stg_model_t* mod, char* propname, 
				stg_property_storage_func_t storage )
{
  stg_property_t* prop = g_datalist_get_data( &mod->props, propname );
  
  if( prop == NULL )
    {
      // create a property and stash it in the model with the right name
      printf( "* set_prop_storage creating a new prop %s:%s\n", mod->token, propname );
      prop = stg_property_create();
      g_datalist_set_data( &mod->props, propname, prop );
    }
  
  prop->storage_func = storage;
}


int stg_model_set_prop( stg_model_t* mod, char* propname, 
			void* data, size_t len )
{
  stg_property_t* prop = g_datalist_get_data( &mod->props, propname );
  
  if( prop == NULL )
    {
      // create a property and stash it in the model with the right name
      printf( "* set_prop creating a new prop %s:%s\n", mod->token, propname );
      prop = stg_property_create();
      g_datalist_set_data( &mod->props, propname, prop );
    }
  
  // if there's a special storage function registered, call it
  if( prop->storage_func )
    prop->storage_func( mod, prop, data, len );
  else
    {
      // just stash the data in the property structure
      prop->data = realloc(prop->data,len);
      memcpy( prop->data, data, len );
      prop->len = len;
    }
  
  // temp
  //printf( "setting %s:%s %d bytes\n", mod->token, propname, len );
  //printf( "model %s now has properties:\n", mod->token );
  //g_datalist_foreach( &mod->props, stg_property_print_cb, "  " );
	  
  // call any callbacks
  stg_property_callback_args_t args;
  args.mod = mod;
  args.data = data;
  args.len = len;
  args.user = prop->user;
  g_datalist_foreach( &mod->props, stg_property_callback_list_cb, &args );

  return 0; // ok
}


// Get the data of the named property. A pointer to the data is set in
// [data]. returns the size of the data in bytes, or negative error
// code if failed.
int stg_model_get_prop( stg_model_t* mod, char* propname, void** data )
{
  stg_property_t* prop = g_datalist_get_data( &mod->props, propname );
  
  if( prop )
    {
      *data = prop->data;
      return prop->len;
    }
  
  // didn't find the property
  PRINT_WARN2( "attempting to get a onexistent property (%s:%s)",
	       mod->token, propname );
  
  *data = NULL;
  return -1; // error
}

int stg_model_add_prop_callback( stg_model_t* mod, char* propname, 
				 stg_property_callback_t callback,
				 void* user )
{
  assert(mod);
  assert(propname);
  stg_property_t* prop = g_datalist_get_data( &mod->props, propname );
  
  
  if( prop == NULL )
    {
      PRINT_WARN2( "attempting to add a callback to a nonexistent property (%s:%s)",
		   mod->token, propname );
      return 1; // error
    }
  
  // else
  prop->callbacks = g_list_append( prop->callbacks, callback );
  prop->user = user;
  return 0; //ok
}

int stg_model_remove_prop_callback( stg_model_t* mod, char* propname, stg_property_callback_t callback )
{
  stg_property_t* prop = g_datalist_get_data( &mod->props, propname );
  
  if( prop == NULL )
    {
      PRINT_WARN2( "attempting to remove a callback from a nonexistent property (%s:%s)",
		   mod->token, propname );
      return 1; // error
    }
  
  // else
  prop->callbacks = g_list_remove( prop->callbacks, callback );
  return 0; //ok
}



int stg_model_remove_prop_callbacks( stg_model_t* mod, char* propname )
{
  stg_property_t* prop = g_datalist_get_data( &mod->props, propname );
  
  if( prop == NULL )
    {
      PRINT_WARN2( "attempting to remove all callbacks from a nonexistent property (%s:%s)",
		   mod->token, propname );
      return 1; // error
    }
  
  // else
  g_list_free( prop->callbacks );
  prop->callbacks = NULL;
  return 0; //ok
}

/** set the pose of a model in its parent's CS */
int stg_model_set_pose( stg_model_t* mod, stg_pose_t* pose )
{
  // if the new pose is different
  if( memcmp( &mod->pose, pose, sizeof(stg_pose_t) ))
    {
      // unrender from the matrix
      stg_model_map_with_children( mod, 0 );
      
      // copy the new pose
      memcpy( &mod->pose, pose, sizeof(mod->pose) );
      
      // render in the matrix
      stg_model_map_with_children( mod, 1 );
      
      // move the stg_rtk figure to match
      if( mod->world->win )
	gui_model_move( mod );      
    }
  
  
  stg_model_set_prop( mod, PPPOSE, pose, sizeof(stg_pose_t) );

  return 0; // OK
}

/** set the pose of model in global coordinates */
int stg_model_set_global_pose( stg_model_t* mod, stg_pose_t* gpose )
{

  if( mod->parent == NULL )
    {
      //printf( "setting pose directly\n");
      stg_model_set_pose( mod, gpose );
    }  
  else
    {
      stg_pose_t lpose;
      memcpy( &lpose, gpose, sizeof(lpose) );
      stg_model_global_to_local( mod->parent, &lpose );
      stg_model_set_pose( mod, &lpose );
    }

  //printf( "setting global pose %.2f %.2f %.2f = local pose %.2f %.2f %.2f\n",
  //      gpose->x, gpose->y, gpose->a, lpose.x, lpose.y, lpose.a );

  return 0; //ok
}

int stg_model_set_geom( stg_model_t* mod, stg_geom_t* geom )
{ 
  // unrender from the matrix
  stg_model_map( mod, 0 );
  
  // store the geom
  memcpy( &mod->geom, geom, sizeof(mod->geom) );
  
  // we probably need to scale and re-render our polygons
  stg_normalize_polygons( (stg_polygon_t*)mod->polygons->data, mod->polygons->len, 
			  mod->geom.size.x, mod->geom.size.y );
  
  if( mod->world->win )
    stg_model_render_polygons( mod );
  
  // we may need to re-render our nose
  if( mod->world->win )
    gui_model_features(mod);
  
  // re-render int the matrix
  stg_model_map( mod, 1 );

  return 0;
}


int stg_model_set_polygons( stg_model_t* mod, 
			    stg_polygon_t* polys, 
			    size_t poly_count )
{
  assert(mod);
  if( poly_count > 0 ) assert( polys );
  
  // background (used e.g for laser scan fill)
  PRINT_DEBUG3( "model %d(%s) received %d polygons", 
		mod->id, mod->token, (int)poly_count );

  // normalize the polygons to fit exactly in the model's body rectangle
  stg_normalize_polygons( polys, poly_count, mod->geom.size.x, mod->geom.size.y );
  
  stg_model_map( mod, 0 ); // unmap the model from the matrix
  
  // remove the old polygons
  g_array_set_size( mod->polygons, 0 );
    
  // append the new polys
  g_array_append_vals( mod->polygons, polys, poly_count );
  
  if( mod->world->win )
    stg_model_render_polygons( mod );

  stg_model_map( mod, 1 ); // map the model into the matrix with the new polys
  
  return 0;
} 

int stg_model_set_parent( stg_model_t* mod, stg_model_t* newparent)
{
  // remove the model from its old parent (if it has one)
  if( mod->parent )
    g_ptr_array_remove( mod->parent->children, mod );

  if( newparent )
    g_ptr_array_add( newparent->children, mod );

  // link from the model to its new parent
  mod->parent = newparent;

  // completely rebuild the GUI elements - it's too complex to patch up the tree herea
  gui_model_destroy( mod );
  gui_model_create( mod );
  stg_model_render_polygons( mod );


  return 0; //ok
}


int stg_model_set_laserreturn( stg_model_t* mod, stg_laser_return_t* val )
{
  memcpy( &mod->laser_return, val, sizeof(mod->laser_return));

  stg_model_set_prop( mod, "laser_return", val, sizeof(stg_laser_return_t) );
  return 0;
}

int stg_model_set_gripperreturn( stg_model_t* mod, stg_gripper_return_t* val )
{
  memcpy( &mod->gripper_return, val, sizeof(mod->gripper_return));
  stg_model_set_prop( mod, "gripper_return", val, sizeof(stg_gripper_return_t) );
  return 0;
}

int stg_model_set_rangerreturn( stg_model_t* mod, stg_ranger_return_t* val )
{
  memcpy( &mod->ranger_return, val, sizeof(mod->ranger_return));
  stg_model_set_prop( mod, "ranger_return", val, sizeof(stg_ranger_return_t));
  return 0;
}

int stg_model_set_obstaclereturn( stg_model_t* mod, stg_obstacle_return_t* val )
{
  memcpy( &mod->obstacle_return, val, sizeof(mod->obstacle_return));
  return 0;
}


int stg_model_set_velocity( stg_model_t* mod, stg_velocity_t* val )
{
  memcpy( &mod->velocity, val, sizeof(mod->velocity) );
  return 0;
}

int stg_model_set_blobreturn( stg_model_t* mod, stg_blob_return_t* val )
{
  memcpy( &mod->blob_return, val, sizeof(mod->blob_return) );
  return 0;
}

int stg_model_set_color( stg_model_t* mod, stg_color_t* col )
{
  memcpy( &mod->color, col, sizeof(mod->color) );

  // redraw my image
  if( mod->world->win )
    stg_model_render_polygons( mod );
  
  return 0; // OK
}

int stg_model_set_mass( stg_model_t* mod, stg_kg_t* mass )
{
  memcpy( &mod->mass, mass, sizeof(mod->mass) );
  return 0;
}

int stg_model_set_boundary( stg_model_t* mod, stg_bool_t* b )
{
  memcpy( &mod->boundary, b, sizeof(mod->boundary) );
  stg_model_map( mod, TRUE );
  return 0;
}

int stg_model_set_guifeatures( stg_model_t* mod, stg_guifeatures_t* gf )
{
  memcpy( &mod->guifeatures, gf, sizeof(mod->guifeatures));
  
  // override the movemask flags - only top-level objects are moveable
  if( mod->parent )
    mod->guifeatures.movemask = 0;

  // redraw the fancy features
  if( mod->world->win )
    gui_model_features( mod );
  return 0;
}

int stg_model_set_fiducialreturn( stg_model_t* mod, stg_fiducial_return_t* val )     
{
  memcpy( &mod->fiducial_return, val, sizeof(mod->fiducial_return) );
  return 0;
}


extern stg_rtk_fig_t* fig_debug_rays; 

int lines_raytrace_match( stg_model_t* mod, stg_model_t* hitmod )
{
  // Ignore myself, my children, and my ancestors.
  if( (!stg_model_is_related(mod,hitmod))  &&  
      hitmod->obstacle_return ) 
    return 1;
  
  return 0; // no match
}	


///////////////////////////////////////////////////////////////////////////
// Check to see if the given pose will yield a collision with obstacles.
// Returns a pointer to the first entity we are in collision with, and stores
// the location of the hit in hitx,hity (if non-null)
// Returns NULL if not collisions.
// This function is useful for writing position devices.
stg_model_t* stg_model_test_collision_at_pose( stg_model_t* mod, 
					   stg_pose_t* pose, 
					   double* hitx, double* hity )
{
  //return NULL;
  
  // raytrace along all our rectangles. expensive, but most vehicles
  // will just be a single rect, grippers 3 rects, etc. not too bad.
  
  size_t count=0;
  stg_polygon_t* polys = stg_model_get_polygons(mod, &count);

  // no body? no collision
  if( count < 1 )
    return NULL;

  if( fig_debug_rays ) stg_rtk_fig_clear( fig_debug_rays );

  // loop over all polygons
  int q;
  for( q=0; q<count; q++ )
    {
      stg_polygon_t* poly = &polys[q];
      
      int point_count = poly->points->len;

      // loop over all points in this polygon
      int p;
      for( p=0; p<point_count; p++ )
	{
	  stg_point_t* pt1 = &g_array_index( poly->points, stg_point_t, p );	  
	  stg_point_t* pt2 = &g_array_index( poly->points, stg_point_t, (p+1) % point_count);
	  
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
	  stg_pose_sum( &p1, pose, &pp1 );
	  stg_pose_sum( &p2, pose, &pp2 );
	  
	  //printf( "tracing %.2f %.2f   %.2f %.2f\n",  p1.x, p1.y, p2.x, p2.y );
	  
	  itl_t* itl = itl_create( p1.x, p1.y, p2.x, p2.y, 
				   mod->world->matrix, 
				   PointToPoint );
	  
	  stg_model_t* hitmod = itl_first_matching( itl, lines_raytrace_match, mod );
	  
	  
	  if( hitmod )
	    {
	      if( hitx ) *hitx = itl->x; // report them
	      if( hity ) *hity = itl->y;	  
	      itl_destroy( itl );
	      return hitmod; // we hit this object! stop raytracing
	    }

	  itl_destroy( itl );
	}
    }

  return NULL;  // done 
}



int stg_model_update_pose( stg_model_t* mod )
{ 
  PRINT_DEBUG4( "pose update model %d (vel %.2f, %.2f %.2f)", 
		mod->id, mod->velocity.x, mod->velocity.y, mod->velocity.a );
 
  stg_velocity_t gvel;
  stg_model_global_velocity( mod, &gvel );
      
  stg_pose_t gpose;
  stg_model_get_global_pose( mod, &gpose );

  // convert msec to sec
  double interval = (double)mod->world->sim_interval / 1000.0;
  
  // compute new global position
  gpose.x += gvel.x * interval;
  gpose.y += gvel.y * interval;
  gpose.a += gvel.a * interval;
      
  double hitx=0, hity=0;
  stg_model_t* hitthing =
    stg_model_test_collision_at_pose( mod, &gpose, &hitx, &hity );
      
  /*
   if( mod->friction )
    {
      // compute a new velocity, based on "friction"
      double vr = hypot( gvel.x, gvel.y );
      double va = atan2( gvel.y, gvel.x );
      vr -= vr * mod->friction;
      gvel.x = vr * cos(va);
      gvel.y = vr * sin(va);
      gvel.a -= gvel.a * mod->friction; 

      // lower bounds
      if( fabs(gvel.x) < 0.001 ) gvel.x = 0.0;
      if( fabs(gvel.y) < 0.001 ) gvel.y = 0.0;
      if( fabs(gvel.a) < 0.01 ) gvel.a = 0.0;
	  
    }
  */

  if( hitthing )
    {
      // TODO - friction simulation
      //if( hitthing->friction == 0 ) // hit an immovable thing
	{
	  PRINT_DEBUG( "HIT something immovable!" );
	  mod->stall = 1;

	  // set velocity to zero
	  stg_velocity_t zero_v;
	  memset( &zero_v, 9, sizeof(zero_v));
	  stg_model_set_velocity( mod, &zero_v );
	}
      /*
	  else
	{
	  puts( "hit something with non-zero friction" );

	  // Get the velocity of the thing we hit
	  //stg_velocity_t* vel = stg_model_get_velocity( hitthing );
	  double impact_vel = hypot( gvel.x, gvel.y );
	      
	  // TODO - use relative mass and velocity properly
	  //stg_kg_t* mass = stg_model_get_mass( hitthing );
	     
	  // Compute bearing from my center of mass to the impact point
	  double pth = atan2( hity-gpose.y, hitx-gpose.x );
	      
	  // Compute bearing TO impacted ent
	  //double pth2 = atan2( o.y-pose.y, o.x-pose.x );
	      
	  if( impact_vel )
	    {
	      double vr =  fabs(impact_vel);

	      stg_velocity_t given;
	      given.x = vr * cos(pth);
	      given.y = vr * sin(pth);
	      given.a = 0;//vr * sin(pth2);
		  
	      // get some velocity from the impact
	      //hitthing->velocity.x = vr * cos(pth);
	      //hitthing->velocity.y = vr * sin(pth);		  

	      printf( "gave %.2f %.2f vel\n",
		      given.x, given.y );

	      stg_model_set_global_velocity( hitthing, &given );
	    }
	      
	}
      */
	  
    }
  else	  
    {
      mod->stall = 0;

      // now set the new pose
      stg_model_set_global_pose( mod, &gpose );
  
      // ignore acceleration in energy model for now, we just pay
      // something to move.	
      //stg_kg_t mass = *stg_model_get_mass( mod );
      //stg_model_energy_consume( mod, STG_ENERGY_COST_MOTIONKG * mass ); 
    }      
  
  return 0; // ok
}


/*
void stg_model_reload( stg_model_t* mod )
{
  stg_model_load( mod );
}
*/

void stg_model_load( stg_model_t* mod )
{
  stg_pose_t pose;
  stg_model_get_pose( mod, &pose );
  pose.x = wf_read_tuple_length(mod->id, "pose", 0, pose.x );
  pose.y = wf_read_tuple_length(mod->id, "pose", 1, pose.y ); 
  pose.a = wf_read_tuple_angle(mod->id, "pose", 2, pose.a );
  stg_model_set_pose( mod, &pose );
  
  stg_geom_t geom;
  stg_model_get_geom( mod, &geom );
  geom.pose.x = wf_read_tuple_length(mod->id, "origin", 0, geom.pose.x );
  geom.pose.y = wf_read_tuple_length(mod->id, "origin", 1, geom.pose.y );
  geom.pose.a = wf_read_tuple_length(mod->id, "origin", 2, geom.pose.a );
  geom.size.x = wf_read_tuple_length(mod->id, "size", 0, geom.size.x );
  geom.size.y = wf_read_tuple_length(mod->id, "size", 1, geom.size.y );
  stg_model_set_geom( mod, &geom );
  
  stg_velocity_t vel;
  vel.x = wf_read_tuple_length(mod->id, "velocity", 0, 0 );
  vel.y = wf_read_tuple_length(mod->id, "velocity", 1, 0 );
  vel.a = wf_read_tuple_angle(mod->id, "velocity", 2, 0 );      
  stg_model_set_velocity( mod, &vel );
    
  stg_kg_t mass = 
    wf_read_float(mod->id, "mass", mod->mass );
  stg_model_set_mass( mod, &mass );
  
  stg_obstacle_return_t obstacle = 
    wf_read_int( mod->id, "obstacle_return", mod->obstacle_return );
  stg_model_set_obstaclereturn( mod, &obstacle );
  
  stg_ranger_return_t ranger = 
    wf_read_int( mod->id, "ranger_return", mod->ranger_return );
  stg_model_set_rangerreturn( mod, &ranger );
  
  // laser visibility
  stg_laser_return_t laser_return = 
    wf_read_int(mod->id, "laser_return", mod->laser_return );      
  stg_model_set_laserreturn( mod, &laser_return );
  
  // ranger visibility
  stg_ranger_return_t ranger_return = 
    wf_read_int( mod->id, "ranger_return",mod->ranger_return );
  stg_model_set_rangerreturn( mod, &ranger_return );

  // grippability
  stg_gripper_return_t gripper_return = 
    wf_read_int( mod->id, "gripper_return",mod->gripper_return );
  stg_model_set_gripperreturn( mod, &gripper_return );
  
  // fiducial visibility
  int fid_return = 
    wf_read_int( mod->id, "fiducial_return", mod->fiducial_return );  
  stg_model_set_fiducialreturn( mod, &fid_return );

  // bounding box
  stg_bool_t boundary = 
    wf_read_int(mod->id, "boundary", mod->boundary ); 
  stg_model_set_boundary( mod, &boundary );

  // TODO - blob visibility
  //int blobvis = 
  //wf_read_int(mod->id, "blob_return", STG_DEFAULT_BLOBRETURN );        
  //stg_model_set_blobreturn( mod, &blobvis );
  
  // TODO - stg_friction_t friction;
  //friction = wf_read_float(mod->id, "friction", 0.0 );
  //stg_model_set_friction(mod, &friction );
  
  const char* colorstr = wf_read_string( mod->id, "color", NULL );
  if( colorstr )
    {
      stg_color_t color = stg_lookup_color( colorstr );
      PRINT_DEBUG2( "stage color %s = %X", colorstr, color );	  
      stg_model_set_color( mod, &color );
    }

  const char* bitmapfile = wf_read_string( mod->id, "bitmap", NULL );
  if( bitmapfile )
    {
      stg_rotrect_t* rects = NULL;
      int num_rects = 0;
      
      int image_width=0, image_height=0;
      
      char full[PATH_MAX];
      
      if( bitmapfile[0] == '/' )
	strcpy( full, bitmapfile );
      else
	{
	  char *tmp = strdup(wf_get_filename());
	  snprintf( full, PATH_MAX,
		    "%s/%s",  dirname(tmp), bitmapfile );
          free(tmp);
	}
      
#ifdef DEBUG
      printf( "attempting to load image %s\n",
	      full );
#endif
      
      if( stg_load_image( full, &rects, &num_rects, 
			  &image_width, &image_height ) )
	exit( -1 );
      
      double bitmap_resolution = 
	wf_read_float( mod->id, "bitmap_resolution", 0 );
      
      // if a bitmap resolution was specified, we override the object
      // geometry
      if( bitmap_resolution )
	{
	  geom.size.x = image_width *  bitmap_resolution;
	  geom.size.y = image_height *  bitmap_resolution; 	    
	  stg_model_set_geom( mod, &geom );	  
	}
	  
      // convert rects to an array of polygons and upload the polygons
      stg_polygon_t* polys = stg_rects_to_polygons( rects, num_rects );
      stg_model_set_polygons( mod, polys, num_rects );     

      // free rects, which was realloc()ed in stg_load_image
      free(rects);
      // free polys, since stg_model_set_polygons copied its contents
      free(polys);
    }
      
  int polycount = wf_read_int( mod->id, "polygons", 0 );
  if( polycount > 0 )
    {
      //printf( "expecting %d polygons\n", polycount );
      
      char key[256];
      stg_polygon_t* polys = stg_polygons_create( polycount );
      int l;
      for(l=0; l<polycount; l++ )
	{	  	  
	  snprintf(key, sizeof(key), "polygon[%d].points", l);
	  int pointcount = wf_read_int(mod->id,key,0);
	  
	  //printf( "expecting %d points in polygon %d\n",
	  //  pointcount, l );
	  
	  int p;
	  for( p=0; p<pointcount; p++ )
	    {
	      snprintf(key, sizeof(key), "polygon[%d].point[%d]", l, p );
	      
	      stg_point_t pt;	      
	      pt.x = wf_read_tuple_length(mod->id, key, 0, 0);
	      pt.y = wf_read_tuple_length(mod->id, key, 1, 0);

	      //printf( "key %s x: %.2f y: %.2f\n",
	      //      key, pt.x, pt.y );
	      
	      // append the point to the polygon
	      stg_polygon_append_points( &polys[l], &pt, 1 );
	    }
	}
      
      stg_model_set_polygons( mod, polys, polycount );
    }

  // if a type-specific load callback has been set
  if( mod->f_load )
    mod->f_load( mod ); // call the load function
  
  // do gui features last so we know exactly what we're supposed to
  // look like.
  stg_guifeatures_t gf;
  stg_model_get_guifeatures( mod, &gf );

  //gf.boundary = wf_read_int(mod->id, "gui_boundary", gf.boundary );
  gf.nose = wf_read_int(mod->id, "gui_nose", gf.nose );
  gf.grid = wf_read_int(mod->id, "gui_grid", gf.grid );
  gf.movemask = wf_read_int(mod->id, "gui_movemask", gf.movemask );
  gf.outline = wf_read_int(mod->id, "gui_outline", gf.outline );

  // install the new set of features 
  stg_model_set_guifeatures( mod, &gf );
}


void stg_model_save( stg_model_t* model )
{
  stg_pose_t pose;
  stg_model_get_pose(model, &pose);
  
  PRINT_DEBUG4( "saving model %s pose %.2f %.2f %.2f",
		model->token,
		pose.x,
		pose.y,
		pose.a );
  
  // right now we only save poses
  wf_write_tuple_length( model->id, "pose", 0, pose.x);
  wf_write_tuple_length( model->id, "pose", 1, pose.y);
  wf_write_tuple_angle( model->id, "pose", 2, pose.a);
}

// find the top-level model above mod;
stg_model_t* stg_model_root( stg_model_t* mod )
{
  while( mod->parent )
    mod = mod->parent;
  return mod;
}

int stg_model_tree_to_ptr_array( stg_model_t* root, GPtrArray* array )
{
  g_ptr_array_add( array, root );
  
  //printf( " added %s to array at %p\n", root->token, array );

  int added = 1;
  
  int ch;
  for(ch=0; ch < root->children->len; ch++ )
    {
      stg_model_t* child = g_ptr_array_index( root->children, ch );
      added += stg_model_tree_to_ptr_array( child, array );
    }
  
  return added;
}
