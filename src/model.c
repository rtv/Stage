
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

#if INCLUDE_GNOME
 #include "gnome.h"
 #define POLYGON_RENDER_CALLBACK model_render_polygons_gc
#else
 #define POLYGON_RENDER_CALLBACK model_render_polygons
#endif

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
#define STG_DEFAULT_MASK (STG_MOVE_TRANS | STG_MOVE_ROT)
#define STG_DEFAULT_NOSE FALSE
#define STG_DEFAULT_GRID FALSE
#define STG_DEFAULT_OUTLINE TRUE
#define STG_DEFAULT_MAP_RESOLUTION 0.1

//extern int _stg_disable_gui;

/** @addtogroup stage 
    @{ 
*/

/** @defgroup model Models
    
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

/** @} */  

/** 
@ingroup stg_model
@defgroup stg_model_props Model Properties

- "pose" stg_pose_t
- "geom" stg_geom_t
- "size" stg_size_t
- "velocity" stg_velocity_t
- "color" stg_color_t
- "fiducial_return" stg_fiducial_return_t
- "laser_return" stg_laser_return_t
- "obstacle_return" stg_obstacle_return_t
- "ranger_return" stg_ranger_return_t
- "gripper_return" stg_gripper_return_t
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
  
  
// callback functions that handle property changes, mostly for drawing stuff in the GUI
  
  int model_render_polygons( stg_model_t* mod, char* name, 
			     void* data, size_t len, void* userp );
int model_handle_mask( stg_model_t* mod, char* name, 
		       void* data, size_t len, void* userp );
int model_handle_outline( stg_model_t* mod, char* name, 
			  void* data, size_t len, void* userp );
int model_render_nose( stg_model_t* mod, char* name, 
		       void* data, size_t len, void* userp );
int model_render_grid( stg_model_t* mod, char* name, 
		       void* data, size_t len, void* userp );

// convert a global pose into the model's local coordinate system
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


// generate the default name for a model, based on the name of its
// parent, and its type. This can be overridden in the world file.
/* int stg_model_create_name( stg_model_t* mod ) */
/* { */
/*   assert( mod ); */
/*   assert( mod->token ); */
  
/*   PRINT_DEBUG1( "model's default name is %s", */
/* 		mod->token ); */
/*   if( mod->parent ) */
/*     PRINT_DEBUG1( "model's parent's name is %s", */
/* 		  mod->parent->token ); */

/*   char buf[512];   */
/*   if( mod->parent == NULL ) */
/*     snprintf( buf, 255, "%s:%d",  */
/* 	      mod->token,  */
/* 	      mod->world->child_type_count[mod->type] ); */
/*   else */
/*     snprintf( buf, 255, "%s.%s:%d",  */
/* 	      mod->parent->token, */
/* 	      mod->token,  */
/* 	      mod->parent->child_type_count[mod->type] ); */
  
/*   free( mod->token ); */
/*   // return a new string just long enough */
/*   mod->token = strndup(buf,512); */

/*   return 0; //ok */
/* } */



void storage_ordinary( stg_property_t* prop, 
		       void* data, size_t len )
{
  if( len > 0 )
    prop->data = realloc( prop->data, len );
  else
    {
      free( prop->data );
      prop->data = NULL;
    }
  
  prop->len = len;
  memcpy( prop->data, data, len );
}



void storage_polygons( stg_property_t* prop, 
		       void* data, size_t len ) 
{
  assert(prop);
  
  if( data == NULL )
    assert( len == 0 );
  
  if( len == 0 )
    assert( data == NULL );
  
  if( data )
    assert( len > 0 );

  //printf( "model %d(%s) received %d bytes (%d polygons)\n", 
  //  prop->mod->id, prop->mod->token, (int)len, (int)(len / sizeof(stg_polygon_t)));
  
  if( len == 0 )
    {
      stg_model_map( prop->mod, 0 ); // unmap the model from the matrix
      storage_ordinary( prop, data, len );
    }
  else
    {
      assert(data);
      
      stg_model_map( prop->mod, 0 ); // unmap the model from the matrix

      stg_geom_t geom;
      stg_model_get_geom( prop->mod, &geom );
      
      stg_polygon_t* polys = (stg_polygon_t*)data;
      size_t count = len / sizeof(stg_polygon_t);
      
      // normalize the polygons to fit exactly in the model's body
      // rectangle (if the model has some non-zero size)
      if( geom.size.x > 0.0 && geom.size.y > 0.0 )
	stg_polygons_normalize( polys, count, geom.size.x, geom.size.y );
      //else
      //PRINT_WARN3( "setting polygons for model %s which has non-positive area (%.2f,%.2f)",
      //	     prop->mod->token, geom.size.x, geom.size.y );

      // store the polygons as a property
      storage_ordinary( prop, polys, len );
      
      // if the model has some non-zero
      stg_model_map( prop->mod, 1 ); // map the model into the matrix with the new polys
    }
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
  //stg_model_property_refresh( prop->mod, "nose" );
  
  // re-render int the matrix
  stg_model_map( prop->mod, 1 );  
}



void storage_color( stg_property_t* prop,
		    void* data, size_t len )
{
  assert( len == sizeof(stg_color_t));

  storage_ordinary( prop, data, len );
  
  // redraw the polygons in the new color
  stg_model_property_refresh( prop->mod, "polygons" );
}


// set the pose of a model in its parent's CS 
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


//model_add_callback( stg_model_t* mod, int key, 


//static int _model_init = TRUE;

stg_model_t* stg_model_create( stg_world_t* world, 
			       stg_model_t* parent,
			       stg_id_t id, 
			       char* token,
			       stg_model_initializer_t initializer )
{  

  stg_model_t* mod = calloc( sizeof(stg_model_t),1 );

  mod->initializer = initializer;

  // TODO?
  /*   int err = pthread_mutex_init( &mod->mutex, NULL ); */
  /*   if( err ) */
  /*     { */
  /*       PRINT_ERR1( "thread initialization failed with code %d\n", err ); */
  /*       exit(-1); */
  /*     } */
  
  /* if( _model_init ) */
  /*     { */
  /*       g_datalist_init( &mod->props ); */
  /*       _model_init = FALSE; */
  /*     } */
    
  mod->id = id;
  
  mod->disabled = FALSE;
  mod->world = world;
  mod->parent = parent; 
  mod->token = strdup(token); // this will be immediately replaced by
			      // model_create_name  

  // create a default name for the model that's derived from its
  // ancestors' names and its worldfile token
  //model_create_name( mod );

  /* experimental */
  mod->callbacks = g_hash_table_new( g_int_hash, g_int_equal );
  /* experimental end */

  PRINT_DEBUG3( "creating model %d.%d(%s)", 
		mod->world->id,
		mod->id, 
		mod->token  );
  
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
      
  int nose = STG_DEFAULT_NOSE;
  stg_model_set_property( mod, "nose", &nose, sizeof(nose) );

  int grid = STG_DEFAULT_GRID;
  stg_model_set_property( mod, "grid", &grid, sizeof(grid) );
  
  int outline = STG_DEFAULT_OUTLINE;
  stg_model_set_property( mod, "outline", &outline, sizeof(outline) );
  
  int mask = mod->parent ? 0 : STG_DEFAULT_MASK;
  stg_model_set_property( mod, "mask", &mask, sizeof(mask) );

  int stall = 0;
  stg_model_set_property( mod, "stall", &stall, sizeof(stall));
  
  double mres = 0.1; // meters
  stg_model_set_property( mod, "map_resolution", &mres, sizeof(mres));
  
  // now it's safe to create the GUI components
  if( mod->world->win )
    gui_model_create( mod );

#if INCLUDE_GNOME
  GnomeCanvasGroup* parent_grp =
    mod->parent ? mod->parent->grp : gnome_canvas_root( mod->world->win->gcanvas );
  
  mod->grp = GNOME_CANVAS_GROUP(
    gnome_canvas_item_new( parent_grp,
			   gnome_canvas_group_get_type(),
			   "x", pose.x,
			   "y", pose.y,
			   NULL ));

  gnome_canvas_item_raise_to_top( GNOME_CANVAS_ITEM(mod->grp) );
#endif

  // exterimental: creates a menu of models
  gui_add_tree_item( mod );
  
  stg_model_add_property_callback( mod, "polygons", POLYGON_RENDER_CALLBACK, NULL );
  stg_model_add_property_callback( mod, "mask", model_handle_mask, NULL );
  stg_model_add_property_callback( mod, "nose", model_render_nose, NULL );
  stg_model_add_property_callback( mod, "grid", model_render_grid, NULL );
  stg_model_add_property_callback( mod, "outline", model_handle_outline, NULL );
  
  // force redraws
  stg_model_property_refresh( mod, "polygons" ); 
  stg_model_property_refresh( mod, "mask" ); 
  stg_model_property_refresh( mod, "nose" ); 
  stg_model_property_refresh( mod, "grid" ); 

  
  PRINT_DEBUG3( "finished model %d.%d(%s)", 
		mod->world->id, 
		mod->id, 
		mod->token );
  
  if( mod->initializer )
    mod->initializer(mod);
  
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


// free the memory allocated for a model
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

// returns TRUE if model [testmod] is a descendent of model [mod]
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

// returns 1 if model [mod1] and [mod2] are in the same model tree
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

// get the model's velocity in the global frame
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
    
    

// convert a pose in this model's local coordinates into global
// coordinates
// should one day do all this with affine transforms for neatness?
void stg_model_local_to_global( stg_model_t* mod, stg_pose_t* pose )
{  
  stg_pose_t origin;   
  stg_model_get_global_pose( mod, &origin );
  stg_pose_sum( pose, &origin, pose );
}


// recursively map a model and all it's descendents
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

// if render is true, render the model into the matrix, else unrender
// the model
void stg_model_map( stg_model_t* mod, gboolean render )
{
  assert( mod );
  
  size_t count=0;
  stg_polygon_t* polys = stg_model_get_polygons(mod, &count);
  
  stg_geom_t geom;
  stg_model_get_geom( mod, &geom );
  
  // to be drawn, we must have a body and some extent greater than zero
  if( polys && geom.size.x > 0.0 && geom.size.y > 0.0 )
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
  mod->world->subs++;

  //printf( "subscribe %d\n", mod->subs );
  
  // if this is the first sub, call startup
  if( mod->subs == 1 )
    stg_model_startup(mod);
}

void stg_model_unsubscribe( stg_model_t* mod )
{
  mod->subs--;
  mod->world->subs--;

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
  if( mod->f_startup )
    return mod->f_startup(mod);
  else
    PRINT_WARN1( "model %s has no startup function registered (TODO: remove this warning before release)", mod->token ); 
  
  return 0; //ok
}

int stg_model_shutdown( stg_model_t* mod )
{
  if( mod->f_shutdown )
    return mod->f_shutdown(mod);
  else
    PRINT_WARN1( "model %s has no shutdown function registered (TODO: remove this warning before release)", mod->token ); 
  
  return 0; //ok
}


// experimental

void model_change( stg_model_t* mod, void* address, size_t size )
{
  int offset = address - (void*)mod;
  
  //printf( "model %s at %p change at address %p offset %d size %d\n",
  //  mod->token, mod, address, offset, (int)size );
  
  //printf( "would call callbacks in model's hash table with key $%d\n",
  //  offset );
}

//------------------------------------------------------------------------
// basic model properties

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

  // experimental
  model_change( mod, &mod->velocity, sizeof(mod->pose));

  return 0; //ok
}

int stg_model_set_pose( stg_model_t* mod, stg_pose_t* pose )
{
  assert(mod);
  assert(pose);
  stg_model_set_property( mod, "pose", pose, sizeof(stg_pose_t));

  // experimental
  model_change( mod, &mod->pose, sizeof(mod->pose));
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

int stg_model_set_geom( stg_model_t* mod, stg_geom_t* src )
{
  assert(mod);
  assert(src);
  stg_model_set_property( mod, "geom", src, sizeof(stg_geom_t));
  return 0;
}

void stg_model_get_pose( stg_model_t* mod, stg_pose_t* dest )
{
  assert(mod);
  assert(dest);
  stg_pose_t* p = stg_model_get_property_fixed( mod,
						"pose", 
						sizeof(stg_pose_t));
  memcpy( dest, p, sizeof(stg_pose_t));
}


stg_polygon_t* stg_model_get_polygons( stg_model_t* mod, size_t* poly_count )
{
  size_t bytes = 0;
  stg_polygon_t* polys = (stg_polygon_t*)
    stg_model_get_property( mod, "polygons", &bytes );
  
  *poly_count = bytes / sizeof(stg_polygon_t);
  return polys;
}

void stg_model_set_polygons( stg_model_t* mod,
			     stg_polygon_t* polys, 
			     size_t poly_count )
{
  size_t bytes = poly_count * sizeof(stg_polygon_t);
  stg_model_set_property( mod, "polygons", polys, bytes );
  
  // experimental
  model_change( mod, &mod->polygons, sizeof(stg_polygon_t) * poly_count );
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

  if( ((stg_property_callback_t)cba->callback)( prop->mod, prop->name, prop->data, prop->len, cba->arg ) )
    {
      // the callback returned true, which means we should remove it
      stg_model_remove_property_callback( prop->mod, prop->name, cba->callback );
    }
}


// TODO?

/* void stg_model_lock( stg_model_t* mod ) */
/* { */
/*   pthread_mutex_lock(&mod->mutex); */
/* } */

/* void stg_model_unlock( stg_model_t* mod ) */
/* { */
/*   pthread_mutex_unlock(&mod->mutex); */
/* } */

void stg_model_set_property_ex( stg_model_t* mod, 
				const char* propname, 
				void* data, 
				size_t len,
				stg_property_storage_func_t func )
{
  assert(mod);
  assert(propname);
  
  // make sure the data and len fields agree
  if( data == NULL ){ assert( len == 0 ); }
  if( len == 0 )    { assert( data == NULL ); }
  
  stg_model_set_property( mod, propname, data, len );
  
  stg_property_t* prop = g_datalist_get_data( &mod->props, propname );
  assert(prop);
  prop->storage_func = func;
}

void stg_model_set_property( stg_model_t* mod, 
			     const char* propname, 
			     void* data, 
			     size_t len )
{
  assert(mod);
  assert(propname);
  
  // make sure the data and len fields agree
  if( data == NULL ){ assert( len == 0 ); }
  if( len == 0 )    { assert( data == NULL ); }
     
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

// set the pose of model in global coordinates 
int stg_model_set_global_pose( stg_model_t* mod, stg_pose_t* gpose )
{

  if( mod->parent == NULL )
    {
      //printf( "setting pose directly\n");
      //stg_model_set_property( mod, "pose", gpose, sizeof(stg_pose_t));
      stg_model_set_pose( mod, gpose );
    }  
  else
    {
      stg_pose_t lpose;
      memcpy( &lpose, gpose, sizeof(lpose) );
      stg_model_global_to_local( mod->parent, &lpose );
      //stg_model_set_property( mod, "pose", &lpose, sizeof(lpose));
      stg_model_set_pose( mod, &lpose );
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
  
  // forces a redraw
  stg_model_property_refresh( mod, "polygons" );

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

  // check this model and all it's children at the new pose
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
      //if( hitthing )


      if( 0 )
	{
	  // HACK!
	  stg_gripper_return_t grip = *(stg_gripper_return_t*)
	    stg_model_get_property_fixed( hitthing, 
					  "gripper_return", 
					  sizeof(stg_gripper_return_t));   
	  if( grip )
	    {
	      stg_velocity_t vel;
	      vel.x = 0.05;
	      vel.y = 0;
	      vel.a = 0.05;
	      
	      stg_model_set_velocity( hitthing, &vel );
	    }
	  
	}
      // TODO - friction simulation
      //if( hitthing->friction == 0 ) // hit an immovable thing
	{
	  PRINT_DEBUG( "HIT something immovable!" );
	  
	  int stall = 1;
	  stg_model_set_property( mod, "stall", &stall, sizeof(stall));


	  // set velocity to zero
	  stg_velocity_t zero_v;
	  memset( &zero_v, 0, sizeof(zero_v));
	  //stg_model_set_velocity( mod, &zero_v );
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
		      given.x, given.y );x

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
      
      int stall = 0;
      stg_model_set_property( mod, "stall", &stall, sizeof(stall));
        
      // ignore acceleration in energy model for now, we just pay
      // something to move.	
      //stg_kg_t mass = *stg_model_get_mass( mod );
      //stg_model_energy_consume( mod, STG_ENERGY_COST_MOTIONKG * mass ); 

      /* trails */
      //if( fig_trails )
      //gui_model_trail( mod );

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
      if( size ) *size = prop->len;      

      return prop->data;
    }
  
  if(size) *size = 0;
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

  void* buf = NULL;
  
  if( data == NULL )
    { assert( len == 0 ); }

  if( len == 0 )
    { assert( data == NULL ); }

  if( len > 0 )
    {
      buf = malloc(len);
      memcpy(buf,data,len);
    }
  
  stg_model_set_property( mod, propname, buf, len );

  if( buf ) 
    free(buf);
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
  stg_polygon_t* polys = NULL;
  size_t polycount = -1 ;//polydata / sizeof(stg_polygon_t);;
  
  const char* bitmapfile = wf_read_string( mod->id, "bitmap", NULL );
  if( bitmapfile )
    {
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
      
      polys = stg_polygons_from_image_file( full, &polycount );
      
      if( ! polys )
	PRINT_ERR1( "Failed to load polygons from image file \"%s\"", full );
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
 
  // if we created any polygons
  if( polycount != -1 )
    {
      if( polycount == 0 )
	PRINT_WARN1( "model \"%s\" has zero polygons", mod->token );
      stg_model_set_property_ex( mod, "polygons",  
				 polys, polycount*sizeof(stg_polygon_t), storage_polygons );
      stg_model_property_refresh( mod, "polygons" );
    }

  // some simple integer properties
  int *now = NULL;
  int val = 0;
  
  now = stg_model_get_property_fixed( mod, "nose", sizeof(val));
  val = wf_read_int(mod->id, "gui_nose", now ? *now : STG_DEFAULT_NOSE );  
  stg_model_set_property( mod, "nose", &val, sizeof(val) );
  
  now = stg_model_get_property_fixed( mod, "grid", sizeof(val));
  val = wf_read_int(mod->id, "gui_grid", now ? *now : STG_DEFAULT_GRID );  
  stg_model_set_property( mod, "grid", &val, sizeof(val) );
  
  now = stg_model_get_property_fixed( mod, "mask", sizeof(val));
  val = wf_read_int(mod->id, "gui_movemask", now ? *now : STG_DEFAULT_MASK);  
  stg_model_set_property( mod, "mask", &val, sizeof(val) );
  
  now = stg_model_get_property_fixed( mod, "outline", sizeof(val));
  val = wf_read_int(mod->id, "gui_outline", now ? *now : STG_DEFAULT_OUTLINE);  
  stg_model_set_property( mod, "outline", &val, sizeof(val) );

  // float properties
  double* dnow = NULL;
  double dval = 0.0;
  dnow = stg_model_get_property_fixed( mod, "map_resolution", sizeof(dval));
  dval = wf_read_float(mod->id, "map_resolution", dnow ? *dnow : STG_DEFAULT_MAP_RESOLUTION );  
  stg_model_set_property( mod, "map_resolution", &dval, sizeof(dval) );

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


int model_render_polygons( stg_model_t* mod, char* name, 
			   void* data, size_t len, void* userp )
{
  stg_rtk_fig_t* fig = stg_model_get_fig( mod, "top" );
  assert( fig );

  stg_rtk_fig_clear( fig );
    
  stg_color_t *cp = 
    stg_model_get_property_fixed( mod, "color", sizeof(stg_color_t));
  
  stg_color_t col = cp ? *cp : 0; // black unless we were given a color

  size_t count = len / sizeof(stg_polygon_t);
  stg_polygon_t* polys = (stg_polygon_t*)data;
  
  stg_geom_t geom;
  stg_model_get_geom(mod, &geom);
  
  stg_rtk_fig_color_rgb32( fig, col );
  
  stg_pose_t* pose = 
    stg_model_get_property_fixed( mod, "pose", sizeof(stg_pose_t));

  if( polys && len > 0 )
    {
      

	if( ! mod->world->win->fill_polygons )
	{
	  
	  int p;
	  for( p=0; p<count; p++ )
	    stg_rtk_fig_polygon( fig,
				 geom.pose.x,
				 geom.pose.y,
				 geom.pose.a,
				 polys[p].points->len,
				 polys[p].points->data,
				 0 );
	}
      else
	{
	  int p;
	  for( p=0; p<count; p++ )
	    stg_rtk_fig_polygon( fig,
				 geom.pose.x,
				 geom.pose.y,
				 geom.pose.a,
				 polys[p].points->len,
				 polys[p].points->data,
				 1 );
	  
	  int* outline = 
	    stg_model_get_property_fixed( mod, "outline", sizeof(int) );
	  if( outline && *outline )
	    {
	      stg_rtk_fig_color_rgb32( fig, 0 ); // black
	      
	      for( p=0; p<count; p++ )
		stg_rtk_fig_polygon( fig,
				     geom.pose.x,
				     geom.pose.y,
				     geom.pose.a,
				     polys[p].points->len,
				     polys[p].points->data,
				     0 );

	      stg_rtk_fig_color_rgb32( fig, col ); // change back
	    }
	  
	}
    }
  
  stg_bool_t* boundary = 
    stg_model_get_property_fixed( mod, "boundary", sizeof(stg_bool_t));
  
  if( boundary && *boundary )
    {      
      stg_rtk_fig_rectangle( fig, 
			     geom.pose.x, geom.pose.y, geom.pose.a, 
			     geom.size.x, geom.size.y, 0 ); 
    }

  int* nose = 
    stg_model_get_property_fixed( mod, "nose", sizeof(int) );
  if( nose && *nose )
    {       
      // draw an arrow from the center to the front of the model
      stg_rtk_fig_arrow( fig, geom.pose.x, geom.pose.y, geom.pose.a, 
			 geom.size.x/2.0, 0.05 );
    }
    
  return 0;
}

int model_render_nose( stg_model_t* mod, char* name, void* data, size_t len, void* userp )
{
  // nose drawing is handled by the polygon handler
  stg_model_property_refresh( mod, "polygons" );
  return 0;
}

int model_handle_mask( stg_model_t* mod, char* name, void* data, size_t len, void* userp )
{
  assert( len == sizeof(int));
  int* mask = (int*)data;
  
  stg_rtk_fig_t* fig = stg_model_get_fig(mod, "top" );
  assert(fig);
  
  stg_rtk_fig_movemask( fig, *mask);  
  
  // only install a mouse handler if the object needs one
  //(TODO can we remove mouse handlers dynamically?)
  if( *mask )    
    stg_rtk_fig_add_mouse_handler( fig, gui_model_mouse );
  
  return 0;
}

int model_handle_outline( stg_model_t* mod, char* name, void* data, size_t len, void* userp )
{
  // outline drawing is handled by the polygon handler
  stg_model_property_refresh( mod, "polygons" );
  return 0;
}
  

int model_render_grid( stg_model_t* mod, char* name, void* data, size_t len, void* userp )
{
  assert( len == sizeof(int));
  int* usegrid = (int*)data;
  
  stg_geom_t geom;
  stg_model_get_geom(mod, &geom);
  
  stg_rtk_fig_t* grid = stg_model_get_fig(mod,"grid"); 
  
  // if we need a grid and don't have one, make one
  if( *usegrid  )
    {    
      if( ! grid ) 
	grid = stg_model_fig_create( mod, "grid", "top", STG_LAYER_GRID); 
      
      stg_rtk_fig_clear( grid );
      stg_rtk_fig_color_rgb32( grid, stg_lookup_color(STG_GRID_MAJOR_COLOR ) );      
      stg_rtk_fig_grid( grid, 
			geom.pose.x, geom.pose.y, 
			geom.size.x, geom.size.y, 1.0  ) ;
    }
  else
    if( grid ) // if we have a grid and don't need one, clear it but keep the fig around      
      stg_rtk_fig_clear( grid );
  
  return 0;
}
