
#define PACKPOSE(P,X,Y,A) {P->x=X; P->y=Y; P->a=A;}

#define _GNU_SOURCE

#include <limits.h> 
#include <assert.h>
#include <math.h>
#include <string.h> // for strdup(3)

//#define DEBUG
//#undef DEBUG

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
  mod->token = strndup(buf,512);

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

void storage_ordinary( stg_property_t* prop, 
		       void* data, size_t len )
{
  prop->data = realloc( prop->data, len );
  prop->len = len;
  memcpy( prop->data, data, len );
}



void storage_guifeatures( stg_property_t* prop, 
			  void* data, size_t len )
{
  stg_guifeatures_t* gf  = (stg_guifeatures_t*)data;
  
  storage_ordinary( prop, data, len );
  
  // override the movemask flags - only top-level objects are moveable
  if( prop->mod->parent )
    gf->movemask = 0;
  
  // redraw the fancy features
  if( prop->mod->world->win )
    gui_model_features( prop->mod );
}


void storage_polygons( stg_property_t* prop, 
		       void* data, size_t len ) 
{
  assert(prop);
  if( len > 0 ) assert(data);
  
  stg_polygon_t* polys = (stg_polygon_t*)data;
  size_t count = len / sizeof(stg_polygon_t);
  
  stg_geom_t* geom = 
    stg_model_get_property_fixed( prop->mod, "geom", sizeof(stg_geom_t));
    
  //printf( "model %d(%s) received %d polygons\n", 
  //  prop->mod->id, prop->mod->token, (int)count );
  
  // normalize the polygons to fit exactly in the model's body
  // rectangle (if specified)
  if( geom )
    stg_normalize_polygons( polys, count, geom->size.x, geom->size.y );
  
  stg_model_map( prop->mod, 0 ); // unmap the model from the matrix
  
  storage_ordinary( prop, polys, len );
  
  stg_model_map( prop->mod, 1 ); // map the model into the matrix with the new polys

  if( prop->mod->world->win )
    stg_model_render_polygons( prop->mod );

} 

void storage_geom( stg_property_t* prop, 
		   void* data, size_t len )
{ 
  assert( len == sizeof(stg_geom_t));

  // unrender from the matrix
  stg_model_map( prop->mod, 0 );
  
  storage_ordinary( prop, data, len );
  
  // we probably need to scale and re-render our polygons
  stg_model_property_refresh( prop->mod, "polygons" );
  
  // re-render int the matrix
  stg_model_map( prop->mod, 1 );
  
  // we may need to re-render our nose, border, etc.
  if( prop->mod->world->win )
    gui_model_features(prop->mod);  
}



void storage_color( stg_property_t* prop,
		    void* data, size_t len )
{
  assert( len == sizeof(stg_color_t));

  storage_ordinary( prop, data, len );
  
  // redraw my image
  if( prop->mod->world->win )
    stg_model_render_polygons( prop->mod );
}


/** set the pose of a model in its parent's CS */
void storage_pose( stg_property_t* prop,
		   void* data, size_t len )
{
  assert( len == sizeof(stg_pose_t) );  
  assert( data );
  
  //if( memcmp( prop->data, data, sizeof(stg_pose_t) )) 
  { 

    // unrender from the matrix
    stg_model_map_with_children( prop->mod, 0 );
    
    storage_ordinary( prop, data, len );
    
    // render in the matrix
    stg_model_map_with_children( prop->mod, 1 );
    
    // move the stg_rtk figure to match
    if( prop->mod->world->win )
      gui_model_move( prop->mod );
  }
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
  
  // create this model's empty list of children
  mod->children = g_ptr_array_new();

  // install the default functions
  mod->f_startup = NULL;
  mod->f_shutdown = NULL;
  mod->f_update = _model_update;

  stg_geom_t geom;
  memset( &geom, 0, sizeof(geom));
  geom.size.x = 1.0;
  geom.size.y = 1.0;
  stg_model_set_property_ex( mod, "geom", &geom, sizeof(geom), storage_geom );
  
  stg_pose_t pose;
  memset( &pose, 0, sizeof(pose));
  stg_model_set_property_ex( mod, "pose", &pose, sizeof(pose), storage_pose );
  
  stg_obstacle_return_t obs = 1;
  stg_model_set_property( mod, "obstacle_return", &obs, sizeof(obs));
  
  stg_ranger_return_t rng = 1;
  stg_model_set_property( mod, "ranger_return", &rng, sizeof(rng));
  
  stg_blob_return_t blb = 1;
  stg_model_set_property( mod, "blob_return", &blb, sizeof(blb));
  
  stg_laser_return_t lsr = LaserVisible;
  stg_model_set_property( mod, "laser_return", &lsr, sizeof(lsr));
  
  stg_gripper_return_t grp = 0;
  stg_model_set_property( mod, "gripper_return", &grp, sizeof(grp));
  
  stg_bool_t bdy = 0;
  stg_model_set_property( mod, "boundary", &bdy, sizeof(bdy));
  
  // body color
  stg_color_t col = 0xFF0000; // red;  
  stg_model_set_property_ex( mod, "color", &col, sizeof(col), storage_color );
  
  stg_polygon_t* square = stg_unit_polygon_create();
  stg_model_set_property_ex( mod, "polygons", square, sizeof(stg_polygon_t), storage_polygons );
  //stg_model_set_property_ex( mod, "polygons", NULL, 0, storage_polygons );
      
  // do gui features last so we know what we're supposed to look like.
  stg_guifeatures_t gf;
  gf.nose = 0;
  gf.grid = 0;
  gf.movemask = mod->parent ? 0 : STG_MOVE_TRANS | STG_MOVE_ROT ;
  gf.outline = 1;
  stg_model_set_property_ex( mod, "gui_features", &gf, sizeof(gf), storage_guifeatures );
  
  // now it's safe to create the GUI components
  if( mod->world->win )
    gui_model_create( mod );
  
  // exterimental: creates a menu of models
  gui_add_tree_item( mod )


  PRINT_DEBUG4( "finished model %d.%d(%s) type %s", 
		mod->world->id, mod->id, 
		mod->token, stg_model_type_string(mod->type) );
  return mod;
}

void stg_property_print_cb( GQuark key, gpointer data, gpointer user )
{
  stg_property_t* prop = (stg_property_t*)data;
  
  printf( "%s key: %s name: %s len: %d func: %p\n", 
	  user ? (char*)user : "", 
	  g_quark_to_string(key), 	  
	  prop->name,
	  (int)prop->len,
	  prop->storage_func );
}

void stg_model_print_properties( stg_model_t* mod )
{
  printf( "Model \"%s\" has  properties:\n",
	  mod->token );
  
  g_datalist_foreach( &mod->props, stg_property_print_cb, "   " );
  puts( "   <end>" );  
}


/// free the memory allocated for a model
void stg_model_destroy( stg_model_t* mod )
{
  assert( mod );
  
  if( mod->world->win )
    gui_model_destroy( mod );

  if( mod->parent && mod->parent->children ) g_ptr_array_remove( mod->parent->children, mod );
  if( mod->children ) g_ptr_array_free( mod->children, FALSE );
  //if( mod->data ) free( mod->data );
  //if( mod->cmd ) free( mod->cmd );
  //if( mod->cfg ) free( mod->cfg );
  if( mod->token ) free( mod->token );

  // TODO - free all property data

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
  
  stg_velocity_t* lvel = 
    stg_model_get_property_fixed( mod, "velocity", sizeof(stg_velocity_t));
  
  if( lvel )
    {
      gvel->x = lvel->x * cosa - lvel->y * sina;
      gvel->y = lvel->x * sina + lvel->y * cosa;
      gvel->a = lvel->a;
    }
  else // no velocity property - we're not moving anywhere
    memset( gvel, 0, sizeof(stg_velocity_t));
      
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
  
  stg_pose_t* pose = 
    stg_model_get_property_fixed( mod, "pose", sizeof(stg_pose_t));
  
  // find my parent's pose
  if( mod->parent )
    {
      stg_model_get_global_pose( mod->parent, &parent_pose );
      
      gpose->x = parent_pose.x + pose->x * cos(parent_pose.a) 
	- pose->y * sin(parent_pose.a);
      gpose->y = parent_pose.y + pose->x * sin(parent_pose.a) 
	+ pose->y * cos(parent_pose.a);
      gpose->a = NORMALIZE(parent_pose.a + pose->a);
    }
  else
    memcpy( gpose, pose, sizeof(stg_pose_t));
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
  
  stg_geom_t geom;
  stg_model_get_geom( mod, &geom );
  
  if( polys )
    {
      if( render && count )
	{      
	  // get model's global pose
	  stg_pose_t org;
	  memcpy( &org, &geom.pose, sizeof(org));
	  stg_model_local_to_global( mod, &org );
	  
	  if( count && polys )    
	    stg_matrix_polygons( mod->world->matrix, 
				 org.x, org.y, org.a,
				 polys, count, 
				 mod );  
	  else
	    PRINT_ERR1( "expecting %d polygons but have no data", (int)count );
	  
	  stg_bool_t* boundary = 
	    stg_model_get_property_fixed( mod, "boundary", sizeof(stg_bool_t));
	  if( *boundary )    
	    stg_matrix_rectangle( mod->world->matrix,
				  org.x, org.y, org.a,
				  geom.size.x,
				  geom.size.y,
				  mod ); 
	}
      else
	stg_matrix_remove_object( mod->world->matrix, mod );
    }
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


// default update function that implements velocities for all objects
int _model_update( stg_model_t* mod )
{
  mod->interval_elapsed = 0;

  stg_velocity_t* vel = 
    stg_model_get_property_fixed( mod, "velocity", sizeof(stg_velocity_t));
  
  // now move the model if it has any velocity
  if( vel && (vel->x || vel->y || vel->a ) )
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


/*
void stg_model_get_pose( stg_model_t* mod, stg_pose_t* pose )
{
  assert(mod);
  assert(pose);
  memcpy(pose, &mod->pose, sizeof(mod->pose));
  
  //stg_pose_t* p;
  //size_t len = stg_model_get_property_data( mod, "pose", &p );  
  //assert( len == sizeof(stg_pose_t));
  //memcpy( pose, p, len );
}
*/


void stg_model_get_velocity( stg_model_t* mod, stg_velocity_t* dest )
{
  assert(mod);
  assert(dest);
  stg_velocity_t* v = stg_model_get_property_fixed( mod,
						    "velocity", 
						    sizeof(stg_velocity_t));
  memcpy( dest, v, sizeof(stg_velocity_t));
}


int stg_model_set_velocity( stg_model_t* mod, stg_velocity_t* vel )
{
  assert(mod);
  assert(vel);
  stg_model_set_property( mod, "velocity", vel, sizeof(stg_velocity_t));
  return 0; //ok
}

void stg_model_get_geom( stg_model_t* mod, stg_geom_t* dest )
{
  assert(mod);
  assert(dest);
  stg_geom_t* g = stg_model_get_property_fixed( mod,
						"geom", 
						sizeof(stg_geom_t));
  memcpy( dest, g, sizeof(stg_geom_t));
}


stg_polygon_t* stg_model_get_polygons( stg_model_t* mod, size_t* poly_count )
{
  size_t bytes = 0;
  stg_polygon_t* polys = (stg_polygon_t*)
    stg_model_get_property( mod, "polygons", &bytes );
  
  *poly_count = bytes / sizeof(stg_polygon_t);
  return polys;
}

void stg_property_destroy( stg_property_t* prop )
{
  // empty the list
  if( prop->callbacks )
    g_list_free( prop->callbacks );
  
  if( prop->data )
    free( prop->data );

  free( prop );
}


// call a property's callback func
void stg_property_callback_cb( gpointer data, gpointer user )
{
  stg_cbarg_t* cba = (stg_cbarg_t*)data;
  stg_property_t* prop = (stg_property_t*)user;  

  //printf( "calling callback %p data %p (%s)\n",
  //  cba->callback, cba->arg, (char*)cba->arg );
 
  if( ((stg_property_callback_t)cba->callback)( prop->mod, prop->name, prop->data, prop->len, cba->arg ) )
    // if the callback returned non-zero, remove it
    stg_model_remove_property_callback( prop->mod, prop->name, cba->arg );
}

stg_property_t* stg_model_set_property_ex( stg_model_t* mod, 
					   const char* propname, 
					   void* data, 
					   size_t len,
					   stg_property_storage_func_t func )
{
  stg_property_t* prop = stg_model_set_property( mod, propname, data, len );
  prop->storage_func = func;
  return prop;
}

stg_property_t* stg_model_set_property( stg_model_t* mod, 
					const char* propname, 
					void* data, 
					size_t len )
{
  stg_property_t* prop = g_datalist_get_data( &mod->props, propname );
  
  if( prop == NULL )
    {
      // create a property and stash it in the model with the right name
      //printf( "* adding model %s property %s size %d\n", 
      //      mod->token, propname, (int)len );
      
      
      prop = (stg_property_t*)calloc(sizeof(stg_property_t),1);      
      strncpy( prop->name, propname, STG_PROPNAME_MAX );
      prop->mod = mod;
      
      g_datalist_set_data( &mod->props, propname, (void*)prop );
      
      // test to make sure the property is there
      stg_property_t* find = g_datalist_get_data( &mod->props, propname );
      assert( find );
      assert( find == prop );      
    }

  // if there's a special storage function registered, call it
  if( prop->storage_func )
    {
      //printf( "calling special storage function for model \"%s\" property \"%s\"\n",
      //      prop->mod->token,
      //      prop->name );
      prop->storage_func( prop, data, len );
    }
  else
    storage_ordinary( prop, data, len );
  
  if( prop->callbacks ) 
      g_list_foreach( prop->callbacks, stg_property_callback_cb, prop );

  return prop; // ok
}


int stg_model_add_property_callback( stg_model_t* mod, 
				     const char* propname, 
				     stg_property_callback_t callback,
				     void* user )
{
  assert(mod);
  assert(propname);
  stg_property_t* prop = g_datalist_get_data( &mod->props, propname );
  
  
  if( ! prop )
    {
      PRINT_WARN2( "attempting to add a callback to a nonexistent property (%s:%s)",
		   mod->token, propname );
      return 1; // error
    }
  // else
  
  stg_cbarg_t* record = calloc(sizeof(stg_cbarg_t),1);
  record->callback = callback;
  record->arg = user;
  
  prop->callbacks = g_list_append( prop->callbacks, record );
  
  //printf( "added callback %p data %p (%s)\n",
  //  callback, user, (char*)user );

  return 0; //ok
}

int stg_model_remove_property_callback( stg_model_t* mod, 
					const char* propname, 
					stg_property_callback_t callback )
{
  stg_property_t* prop = g_datalist_get_data( &mod->props, propname );
  
  if( ! prop )
    {
      PRINT_WARN2( "attempting to remove a callback from a nonexistent property (%s:%s)",
		   mod->token, propname );
      return 1; // error
    }
  
  // else

  // find our callback in the list of stg_cbarg_t
  GList* el = NULL;
  
  // scan the list for the first matching callback
  for( el = g_list_first( prop->callbacks); 
       el;
       el = el->next )
    {
      if( ((stg_cbarg_t*)el->data)->callback == callback )
	break;      
    }

  if( el ) // if we found the matching callback, remove it
    prop->callbacks = g_list_remove( prop->callbacks, el->data);
 
  if( el && el->data )
    free( el->data );
 
  return 0; //ok
}



int stg_model_remove_property_callbacks( stg_model_t* mod,
					 const char* propname )
{
  stg_property_t* prop = g_datalist_get_data( &mod->props, propname );
  
  if( ! prop )
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

/** set the pose of model in global coordinates */
int stg_model_set_global_pose( stg_model_t* mod, stg_pose_t* gpose )
{

  if( mod->parent == NULL )
    {
      //printf( "setting pose directly\n");
      stg_model_set_property( mod, "pose", gpose, sizeof(stg_pose_t));
    }  
  else
    {
      stg_pose_t lpose;
      memcpy( &lpose, gpose, sizeof(lpose) );
      stg_model_global_to_local( mod->parent, &lpose );
      stg_model_set_property( mod, "pose", &lpose, sizeof(lpose));
    }

  //printf( "setting global pose %.2f %.2f %.2f = local pose %.2f %.2f %.2f\n",
  //      gpose->x, gpose->y, gpose->a, lpose.x, lpose.y, lpose.a );

  return 0; //ok
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


extern stg_rtk_fig_t* fig_debug_rays; 

int lines_raytrace_match( stg_model_t* mod, stg_model_t* hitmod )
{
  stg_obstacle_return_t* obs = 
    stg_model_get_property_fixed( hitmod, 
				  "obstacle_return", 
				  sizeof(stg_obstacle_return_t));
  
  // Ignore myself, my children, and my ancestors.
  if(  *obs && (!stg_model_is_related(mod,hitmod)) ) 
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
	  //mod->stall = 1;

	  // set velocity to zero
	  stg_velocity_t zero_v;
	  memset( &zero_v, 0, sizeof(zero_v));
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
      //mod->stall = 0;

      // now set the new pose
      stg_model_set_global_pose( mod, &gpose );
  
      // ignore acceleration in energy model for now, we just pay
      // something to move.	
      //stg_kg_t mass = *stg_model_get_mass( mod );
      //stg_model_energy_consume( mod, STG_ENERGY_COST_MOTIONKG * mass ); 
    }      
  
  return 0; // ok
}



void* stg_model_get_property( stg_model_t* mod, 
			      const char* name,
			      size_t* size )
{
  stg_property_t* prop = g_datalist_get_data( &mod->props, name );
  
  if( prop )
    {
      //assert( prop->data && prop->len == 0));
      //assert( prop->data == NULL && prop->len > 0 );

      *size = prop->len;      
      return prop->data;
    }

  *size = 0;
  return NULL;
}

// get a property of a known size. gets a valid pointer iff the
// property exists and contaisn data of the right size, else returns
// NULL
void* stg_model_get_property_fixed( stg_model_t* mod, 
				    const char* name,
				    size_t size )
{
  size_t actual_size=0;
  void* p = stg_model_get_property( mod, name, &actual_size );
  
  if( size == actual_size )
    return p; // right size: serve up the data
  else
    return NULL; // no data if it wasn't the right size
}



// set the named property with its
void stg_model_property_refresh( stg_model_t* mod, const char* propname )
{
  //stg_property_t* prop = g_datalist_get_data( &mod->props, propname );
  //if( prop )
    
  size_t len=0;
  void* data = stg_model_get_property( mod, propname, &len );
  stg_model_set_property( mod, propname, data, len );
}


void stg_model_load( stg_model_t* mod )
{
  //const char* typestr = wf_read_string( mod->id, "type", NULL );
  //PRINT_MSG2( "Model %p is type %s", mod, typestr );
  assert(mod);

  
  if( wf_property_exists( mod->id, "origin" ) )
    {
      stg_geom_t* now = 
	stg_model_get_property_fixed( mod, "geom", sizeof(stg_geom_t));      
      
      stg_geom_t geom;
      geom.pose.x = wf_read_tuple_length(mod->id, "origin", 0, now ? now->pose.x : 0 );
      geom.pose.y = wf_read_tuple_length(mod->id, "origin", 1, now ? now->pose.y : 0 );
      geom.pose.a = wf_read_tuple_length(mod->id, "origin", 2, now ? now->pose.a : 0 );
      geom.size.x = now ? now->size.x : 1;
      geom.size.y = now ? now->size.y : 1;
      stg_model_set_property( mod, "geom", &geom, sizeof(geom));
    }

  if( wf_property_exists( mod->id, "size" ) )
    {
      stg_geom_t* now = 
	stg_model_get_property_fixed( mod, "geom", sizeof(stg_geom_t));      
      
      stg_geom_t geom;
      geom.pose.x = now ? now->pose.x : 0;
      geom.pose.y = now ? now->pose.y : 0;
      geom.pose.a = now ? now->pose.a : 0;
      geom.size.x = wf_read_tuple_length(mod->id, "size", 0, now ? now->size.x : 1 );
      geom.size.y = wf_read_tuple_length(mod->id, "size", 1, now ? now->size.y : 1 );
      stg_model_set_property( mod, "geom", &geom, sizeof(geom));
    }

  if( wf_property_exists( mod->id, "pose" ))
    {
      stg_pose_t* now = 
	stg_model_get_property_fixed( mod, "pose", sizeof(stg_pose_t));
      
      stg_pose_t pose;
      pose.x = wf_read_tuple_length(mod->id, "pose", 0, now ? now->x : 0 );
      pose.y = wf_read_tuple_length(mod->id, "pose", 1, now ? now->y : 0 ); 
      pose.a = wf_read_tuple_angle(mod->id, "pose", 2,  now ? now->a : 0 );
      stg_model_set_property( mod, "pose", &pose, sizeof(pose));
    }
  
  if( wf_property_exists( mod->id, "velocity" ))
    {
      stg_velocity_t* now = 
	stg_model_get_property_fixed( mod, "velocity", sizeof(stg_velocity_t));
      
      stg_velocity_t vel;
      vel.x = wf_read_tuple_length(mod->id, "velocity", 0, now ? now->x : 0 );
      vel.y = wf_read_tuple_length(mod->id, "velocity", 1, now ? now->y : 0 );
      vel.a = wf_read_tuple_angle(mod->id, "velocity", 2,  now ? now->a : 0 );      
      stg_model_set_property( mod, "velocity", &vel, sizeof(vel) );
    }
  
  if( wf_property_exists( mod->id, "mass" ))
    {
      stg_kg_t* now = 
	stg_model_get_property_fixed( mod, "mass", sizeof(stg_kg_t));      
      stg_kg_t mass = wf_read_float(mod->id, "mass", now ? *now : 0 );      
      stg_model_set_property( mod, "mass", &mass, sizeof(mass) );
    }
  
  if( wf_property_exists( mod->id, "fiducial_return" ))
    {
      stg_fiducial_return_t* now = 
	stg_model_get_property_fixed( mod, "fiducial_return", sizeof(stg_fiducial_return_t));      
      stg_fiducial_return_t fid = 
	wf_read_int( mod->id, "fiducial_return", now ? *now : FiducialNone );     
      stg_model_set_property( mod, "fiducial_return", &fid, sizeof(fid) );
    }
  
  if( wf_property_exists( mod->id, "obstacle_return" ))
    {
      stg_obstacle_return_t* now = 
	stg_model_get_property_fixed( mod, "obstacle_return", sizeof(stg_obstacle_return_t));      
      stg_obstacle_return_t obs = 
	wf_read_int( mod->id, "obstacle_return", now ? *now : 0 );
      stg_model_set_property( mod, "obstacle_return", &obs, sizeof(obs) );
    }

  if( wf_property_exists( mod->id, "ranger_return" ))
    {
      stg_ranger_return_t* now = 
	stg_model_get_property_fixed( mod, "ranger_return", sizeof(stg_ranger_return_t));      
      
      stg_ranger_return_t rng = 
	wf_read_int( mod->id, "ranger_return", now ? *now : 0 );
      stg_model_set_property( mod, "ranger_return", &rng, sizeof(rng));
    }
  
  if( wf_property_exists( mod->id, "blob_return" ))
    {
      stg_blob_return_t* now = 
	stg_model_get_property_fixed( mod, "ranger_return", sizeof(stg_ranger_return_t));      
      stg_blob_return_t blb = 
	wf_read_int( mod->id, "blob_return", now ? *now : 0 );
      stg_model_set_property( mod, "blob_return", &blb, sizeof(blb));
    }
  
  if( wf_property_exists( mod->id, "laser_return" ))
    {
      stg_laser_return_t* now = 
	stg_model_get_property_fixed( mod, "laser_return", sizeof(stg_laser_return_t));      
      stg_laser_return_t lsr = 
	wf_read_int(mod->id, "laser_return", now ? *now : LaserVisible );      
      stg_model_set_property( mod, "laser_return", &lsr, sizeof(lsr));
    }
  
  if( wf_property_exists( mod->id, "gripper_return" ))
    {
      stg_blob_return_t* now = 
	stg_model_get_property_fixed( mod, "gripper_return", sizeof(stg_gripper_return_t));      
      stg_gripper_return_t grp = 
	wf_read_int( mod->id, "gripper_return", now ? *now : 0 );
      stg_model_set_property( mod, "gripper_return", &grp, sizeof(grp));
    }
  
  if( wf_property_exists( mod->id, "boundary" ))
    {
      stg_bool_t* now = 
	stg_model_get_property_fixed( mod, "boundary", sizeof(stg_bool_t));      
      stg_bool_t bdy =  
	wf_read_int(mod->id, "boundary", now ? *now : 0  ); 
      stg_model_set_property( mod, "boundary", &bdy, sizeof(bdy));
    }
  
  if( wf_property_exists( mod->id, "color" ))
    {
      //stg_color_t* now = 
      //stg_model_get_property_fixed( mod, "color", sizeof(stg_color_t));      
      
      stg_color_t col = 0xFF0000; // red;  
      const char* colorstr = wf_read_string( mod->id, "color", NULL );
      if( colorstr )
	{
	  col = stg_lookup_color( colorstr );  
	  stg_model_set_property( mod, "color", &col, sizeof(col));
	}
    }      
  
  //size_t polydata = 0;
  stg_polygon_t* polys = NULL;// = stg_model_get_property( mod, "polygons", &polydata );
  size_t polycount = -1 ;//polydata / sizeof(stg_polygon_t);;
  
  const char* bitmapfile = wf_read_string( mod->id, "bitmap", NULL );
  if( bitmapfile )
    {
      stg_rotrect_t* rects = NULL;
      int num_rects = 0;
      
      int image_width=0, image_height=0;
      
      char full[_POSIX_PATH_MAX];
      
      if( bitmapfile[0] == '/' )
	strcpy( full, bitmapfile );
      else
	{
	  char *tmp = strdup(wf_get_filename());
	  snprintf( full, _POSIX_PATH_MAX,
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
	  stg_geom_t* geom = 
	    stg_model_get_property_fixed( mod, "geom", sizeof(stg_geom_t));
	  assert(geom);
	  
	  geom->size.x = image_width *  bitmap_resolution;
	  geom->size.y = image_height *  bitmap_resolution; 	    
	  stg_model_set_property( mod, "geom", geom, sizeof(stg_geom_t));
	}
      
      //printf( "got %d rectangles from bitmap %s\n", num_rects, full );

      // convert rects to an array of polygons and upload the polygons
      polys = stg_rects_to_polygons( rects, num_rects );
      polycount = num_rects;

      // free rects, which was realloc()ed in stg_load_image
      free(rects);
    }
  
  int wf_polycount = wf_read_int( mod->id, "polygons", 0 );
  if( wf_polycount > 0 )
    {
      polycount = wf_polycount;
      //printf( "expecting %d polygons\n", polycount );
      
      char key[256];
      polys = stg_polygons_create( polycount );
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
    }
  /* else if( polys == NULL ) // create a unit rectangle by default?
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
      
      polys = stg_polygon_create();
      stg_polygon_set_points( polys, pts, 4 );  
      polycount = 1;
    }
  */

  // if we created any polygons
  if( polycount != -1 )
    {
      stg_model_set_property_ex( mod, "polygons",  
				 polys, polycount*sizeof(stg_polygon_t), storage_polygons );
      stg_model_property_refresh( mod, "polygons" );
    }

  stg_guifeatures_t* gf_now = 
    stg_model_get_property_fixed( mod, "gui_features", sizeof(stg_guifeatures_t));
  
  stg_guifeatures_t gf;
  gf.nose = wf_read_int(mod->id, "gui_nose", gf_now ? gf_now->nose : 0 );
  gf.grid = wf_read_int(mod->id, "gui_grid", gf_now ? gf_now->grid : 0 );
  gf.movemask = wf_read_int(mod->id, "gui_movemask", 
			    mod->parent ? 0 : gf_now ? gf_now->movemask : (STG_MOVE_TRANS | STG_MOVE_ROT)  );
  gf.outline = wf_read_int(mod->id, "gui_outline", gf_now ? gf_now->outline : 1 );
  
  stg_model_set_property( mod, "gui_features", &gf, sizeof(gf) );
  
  // if a type-specific load callback has been set
  if( mod->f_load )
    mod->f_load( mod ); // call the load function
}


void stg_model_save( stg_model_t* model )
{
  stg_pose_t* pose = 
    stg_model_get_property_fixed( model, "pose", sizeof(stg_pose_t));
  
  PRINT_DEBUG4( "saving model %s pose %.2f %.2f %.2f",
		model->token,
		pose->x,
		pose->y,
		pose->a );
  
  // right now we only save poses
  wf_write_tuple_length( model->id, "pose", 0, pose->x);
  wf_write_tuple_length( model->id, "pose", 1, pose->y);
  wf_write_tuple_angle( model->id, "pose", 2, pose->a);
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
