
#define _GNU_SOURCE

#include <limits.h> 
#include <assert.h>
#include <math.h>
//#include <string.h> // for strdup(3)

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

extern stg_type_record_t typetable[];

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
  
  pose->x = sx;
  pose->y = sy;
  pose->a = sa;
  
  //printf( "g2l local pose %.2f %.2f %.2f\n",
  //  pose->x, pose->y, pose->a );
}


/** container for a callback function and a single argument, so
    they can be stored together in a list with a single pointer. */
typedef struct
{
  stg_model_callback_t callback;
  void* arg;
} stg_cb_t;


void stg_model_add_callback( stg_model_t* mod, 
			     void* address, 
			     stg_model_callback_t cb, 
			     void* user )
{
  int* key = malloc(sizeof(int));
  *key = address - (void*)mod;

  GList* cb_list = g_hash_table_lookup( mod->callbacks, key );
  
  stg_cb_t* cba = calloc(sizeof(stg_cb_t),1);
  cba->callback = cb;
  cba->arg = user;
  
  //printf( "installing callback in model %s with key %d\n",
  //  mod->token, *key );

  // add the callback & argument to the list
  cb_list = g_list_prepend( cb_list, cba );
  
  // and replace the list in the hash table
  g_hash_table_insert( mod->callbacks, key, cb_list );
}

int stg_model_remove_callback( stg_model_t* mod,
			       void* member,
			       stg_model_callback_t callback )
{
  int key = member - (void*)mod;

  GList* cb_list = g_hash_table_lookup( mod->callbacks, &key );
 
  // find our callback in the list of stg_cbarg_t
  GList* el = NULL;
  
  // scan the list for the first matching callback
  for( el = g_list_first( cb_list );
       el;
       el = el->next )
    {
      if( ((stg_cbarg_t*)el->data)->callback == callback )
	break;
    }

  if( el ) // if we found the matching callback, remove it
    {
      //puts( "removed callback" );

      cb_list = g_list_remove( cb_list, el->data);
      
      // store the new, shorter, list of callbacks
      g_hash_table_insert( mod->callbacks, &key, cb_list );

      // we're done with that
      //free( el->data );
    }
  else
    {
      //puts( "callback was not installed" );
    }
 
  return 0; //ok
}


void model_call_callbacks( stg_model_t* mod, void* address )
{
  assert( mod );
  assert( address );
  assert( address > (void*)mod );
  assert( address < (void*)( mod + sizeof(stg_model_t)));
  
  int key = address - (void*)mod;
  
  //printf( "Model %s has %d callbacks. Checking key %d\n", 
  //  mod->token, g_hash_table_size( mod->callbacks ), key );
  
  GList* cbs = g_hash_table_lookup( mod->callbacks, &key ); 
  
  //printf( "key %d has %d callbacks registered\n",
  //  key, g_list_length( cbs ) );
  
  // maintain a list of callbacks that should be cancelled
  GList* doomed = NULL;

  // for each callback in the list
  while( cbs )
    {  
      stg_cb_t* cba = (stg_cb_t*)cbs->data;
      
      //puts( "calling..." );
      
      if( (cba->callback)( mod, cba->arg ) )
	{
	  //printf( "callback returned TRUE - schedule removal from list\n" );
	  doomed = g_list_prepend( doomed, cba->callback );
	}
      else
	{
	  //printf( "callback returned FALSE - keep in list\n" );
	}
      
      cbs = cbs->next;
    }      

  if( doomed ) 	// delete all the callbacks that returned TRUE    
    {
      //printf( "removing %d doomed callbacks\n", g_list_length( doomed ) );
      
      for( ; doomed ; doomed = doomed->next )
	stg_model_remove_callback( mod, address, (stg_model_callback_t*)doomed->data );
      
      g_list_free( doomed );      
    }

}

stg_model_t* stg_model_create( stg_world_t* world, 
			       stg_model_t* parent,
			       stg_id_t id,
			       char* typestr )
{  
  
  stg_model_t* mod = calloc( sizeof(stg_model_t),1 );
  
  // look up the type name in the typetable
  int found = FALSE;
  int index=0;
  for( mod->typerec = typetable; mod->typerec->keyword; mod->typerec++ )
    {
      if( strcmp( typestr, mod->typerec->keyword ) == 0 )
	{
	  found = TRUE;
	  break;
	}
      index++;
    }

  if( !found )
    {
      PRINT_ERR1( "model type \"%s\" is not recognized. Check your worldfile.", 
		  typestr );
      exit( -1 );
    }

  assert(mod->typerec);

  mod->id = id;
  mod->disabled = FALSE;
  mod->world = world;
  mod->parent = parent; 

  //printf( "creating model %d.%d(%s)\n", 
  //  mod->world->id,
  //  mod->id, 
  //  mod->token  );
  
  //PRINT_DEBUG1( "original token: %s", token );

  // add this model to its parent's list of children (if any)
  if( parent) g_ptr_array_add( parent->children, mod );       
  
  // create this model's empty list of children
  mod->children = g_ptr_array_new();
  
  // generate a name and count this type in its parent (or world,
  // if it's a top-level object)
  if( parent == NULL )
    snprintf( mod->token, STG_TOKEN_MAX, "%s:%d", 
	      typestr, 
	      world->child_type_count[index]++);
  else
    snprintf( mod->token, STG_TOKEN_MAX, "%s.%s:%d", 
	      parent->token,
	      typestr, 
	      parent->child_type_count[index]++ );
  
  // initialize the table of callbacks that are triggered when a
  // model's fields change
  mod->callbacks = g_hash_table_new( g_int_hash, g_int_equal );

  // install the default functions
  mod->f_startup = NULL;
  mod->f_shutdown = NULL;
  mod->f_update = _model_update;

  mod->geom.size.x = 1.0;
  mod->geom.size.y = 1.0;

  mod->obstacle_return = 1;
  mod->ranger_return = 1;
  mod->blob_return = 1;
  mod->laser_return = LaserVisible;
  mod->gripper_return = 1;
  mod->boundary = 0;
  mod->color = 0xFF0000; // red;  
  mod->map_resolution = 0.1; // meters
  mod->gui_nose = STG_DEFAULT_NOSE;
  mod->gui_grid = STG_DEFAULT_GRID;
  mod->gui_outline = STG_DEFAULT_OUTLINE;
  mod->gui_mask = mod->parent ? 0 : STG_DEFAULT_MASK;

  // now it's safe to create the GUI components
  if( mod->world->win )
    gui_model_create( mod );

  // GUI callbacks - draw changes

  // changes in any of these properties require a redraw of the model
  stg_model_add_callback( mod, &mod->polygons, gui_model_polygons, NULL );
  stg_model_add_callback( mod, &mod->color, gui_model_polygons, NULL );
  stg_model_add_callback( mod, &mod->gui_nose, gui_model_polygons, NULL );
  stg_model_add_callback( mod, &mod->gui_outline, gui_model_polygons, NULL );
  stg_model_add_callback( mod, &mod->parent, gui_model_polygons, NULL );
  
  // these changes can be handled without a complete redraw
  stg_model_add_callback( mod, &mod->gui_grid, gui_model_grid, NULL );
  stg_model_add_callback( mod, &mod->pose, gui_model_move, NULL );
  stg_model_add_callback( mod, &mod->gui_mask, gui_model_mask, NULL );

  // force refresh
  //model_change( mod, &mod->gui_mask );


  // now we can add the basic square shape
  stg_polygon_t* square = stg_unit_polygon_create();
  stg_model_set_polygons( mod, square, 1 );

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
  
  PRINT_DEBUG3( "finished model %d.%d(%s)", 
		mod->world->id, 
		mod->id, 
		mod->token );
  
  // call the type-specific initializer function
  if( mod->typerec->initializer )
    mod->typerec->initializer(mod);
  
  return mod;
}

// free the memory allocated for a model
void stg_model_destroy( stg_model_t* mod )
{
  assert( mod );
  
  // remove from parent, if there is one
  if( mod->parent && mod->parent->children ) g_ptr_array_remove( mod->parent->children, mod );
  
  // free all local stuff
  if( mod->world->win ) gui_model_destroy( mod );
  if( mod->children ) g_ptr_array_free( mod->children, FALSE );
  if( mod->callbacks ) g_hash_table_destroy( mod->callbacks );
  if( mod->data ) free( mod->data );
  if( mod->cmd ) free( mod->cmd );
  if( mod->cfg ) free( mod->cfg );
  
  // and zap, we're gone
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
  
  stg_velocity_t* lvel = &mod->velocity;
  
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
  stg_pose_t* pose = &mod->pose;
  
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
	  
	  if( mod->boundary )    
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

  stg_velocity_t* vel = &mod->velocity;
  
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

void model_change( stg_model_t* mod, void* address )
{
  int offset = address - (void*)mod;
  
  //printf( "model %s at %p change at address %p offset %d \n",
  //  mod->token, mod, address, offset );
  
  model_call_callbacks( mod, address );
}

//------------------------------------------------------------------------
// basic model properties

void stg_model_set_data( stg_model_t* mod, void* data, size_t len )
{
  mod->data = realloc( mod->data, len );
  memcpy( mod->data, data, len );
  mod->data_len = len;

  //printf( "&mod->data %d %p\n",
  //  &mod->data, &mod->data );
  model_change( mod, &mod->data );
}

void stg_model_set_cmd( stg_model_t* mod, void* cmd, size_t len )
{
  mod->cmd = realloc( mod->cmd, len );
  memcpy( mod->cmd, cmd, len );
  mod->cmd_len = len;
  model_change( mod, &mod->cmd );
}

void stg_model_set_cfg( stg_model_t* mod, void* cfg, size_t len )
{
  mod->cfg = realloc( mod->cfg, len );
  memcpy( mod->cfg, cfg, len );
  mod->cfg_len = len;
  model_change( mod, &mod->cfg );  
}

void* stg_model_get_cfg( stg_model_t* mod, size_t* lenp )
{
  assert(mod);
  if(lenp) *lenp = mod->cfg_len;
  return mod->cfg;
}

void* stg_model_get_data( stg_model_t* mod, size_t* lenp )
{
  assert(mod);
  if(lenp) *lenp = mod->data_len;
  return mod->data;
}

void* stg_model_get_cmd( stg_model_t* mod, size_t* lenp )
{
  assert(mod);
  if(lenp) *lenp = mod->cmd_len;
  return mod->cmd;
}

void stg_model_get_velocity( stg_model_t* mod, stg_velocity_t* dest )
{
  assert(mod);
  assert(dest);
  memcpy( dest, &mod->velocity, sizeof(stg_velocity_t));
}

void stg_model_set_velocity( stg_model_t* mod, stg_velocity_t* vel )
{
  assert(mod);
  assert(vel);  
  memcpy( &mod->velocity, vel, sizeof(stg_velocity_t));
  model_change( mod, &mod->velocity );
}

void stg_model_set_pose( stg_model_t* mod, stg_pose_t* pose )
{
  assert(mod);
  assert(pose);
  
  // unrender from the matrix
  stg_model_map_with_children( mod, 0 );
    
  memcpy( &mod->pose, pose, sizeof(stg_pose_t));

  // render in the matrix
  stg_model_map_with_children( mod, 1 );
  
  model_change( mod, &mod->pose );
}

void stg_model_get_geom( stg_model_t* mod, stg_geom_t* dest )
{
  assert(mod);
  assert(dest);
  memcpy( dest, &mod->geom, sizeof(stg_geom_t));
}

void stg_model_set_geom( stg_model_t* mod, stg_geom_t* geom )
{
  assert(mod);
  assert(geom);
 
  // unrender from the matrix
  stg_model_map( mod, 0 );

  memcpy( &mod->geom, geom, sizeof(stg_geom_t));
  
  // we probably need to scale and re-render our polygons

  // we can do it in-place
  stg_polygons_normalize( mod->polygons, mod->polygons_count, 
			  geom->size.x, geom->size.y );
  model_change( mod, &mod->polygons );
    
  // re-render int the matrix
  stg_model_map( mod, 1 );  
  
  model_change( mod, &mod->geom );
}

void stg_model_set_color( stg_model_t* mod, stg_color_t col )
{
  assert(mod);
  mod->color = col;
  model_change( mod, &mod->color );
}

void stg_model_set_mass( stg_model_t* mod, stg_kg_t mass )
{
  assert(mod);
  mod->mass = mass;
  model_change( mod, &mod->mass );
}

void stg_model_set_stall( stg_model_t* mod, stg_bool_t stall )
{
  assert(mod);
  mod->stall = stall;
  //model_change( mod, &mod->stall );
}

void stg_model_set_gripper_return( stg_model_t* mod, int val )
{
  assert(mod);
  mod->gripper_return = val;
  model_change( mod, &mod->gripper_return );
}

void stg_model_set_fiducial_return( stg_model_t* mod, int val )
{
  assert(mod);
  mod->fiducial_return = val;
  model_change( mod, &mod->fiducial_return );
}

void stg_model_set_laser_return( stg_model_t* mod, int val )
{
  assert(mod);
  mod->laser_return = val;
  model_change( mod, &mod->laser_return );
}

void stg_model_set_obstacle_return( stg_model_t* mod, int val )
{
  assert(mod);
  mod->obstacle_return = val;
  model_change( mod, &mod->obstacle_return );
}

void stg_model_set_blob_return( stg_model_t* mod, int val )
{
  assert(mod);
  mod->blob_return = val;
  model_change( mod, &mod->blob_return );
}

void stg_model_set_ranger_return( stg_model_t* mod, int val )
{
  assert(mod);
  mod->ranger_return = val;
  model_change( mod, &mod->ranger_return );
}

void stg_model_set_boundary( stg_model_t* mod, int val )
{
  assert(mod);
  mod->boundary = val;
  model_change( mod, &mod->boundary );
}

void stg_model_set_gui_nose( stg_model_t* mod, int val )
{
  assert(mod);
  mod->gui_nose = val;
  model_change( mod, &mod->gui_nose );
}

void stg_model_set_gui_mask( stg_model_t* mod, int val )
{
  assert(mod);
  mod->gui_mask = val;
  model_change( mod, &mod->gui_mask );
}

void stg_model_set_gui_grid( stg_model_t* mod, int val )
{
  assert(mod);
  mod->gui_grid = val;
  model_change( mod, &mod->gui_grid );
}

void stg_model_set_gui_outline( stg_model_t* mod, int val )
{
  assert(mod);
  mod->gui_outline = val;
  model_change( mod, &mod->gui_outline );
}

void stg_model_set_watts( stg_model_t* mod, stg_watts_t watts )
{
  assert(mod);
  mod->watts = watts;
  model_change( mod, &mod->watts );
}

void stg_model_set_map_resolution( stg_model_t* mod, stg_meters_t res )
{
  assert(mod);
  mod->map_resolution = res;
  model_change( mod, &mod->map_resolution );
}


void stg_model_get_pose( stg_model_t* mod, stg_pose_t* dest )
{
  assert(mod);
  assert(dest);
  memcpy( dest, &mod->pose, sizeof(stg_pose_t));
}


stg_polygon_t* stg_model_get_polygons( stg_model_t* mod, size_t* poly_count )
{
  assert(mod);
  assert(poly_count);
  *poly_count = mod->polygons_count;
  return mod->polygons;
}

void stg_model_set_polygons( stg_model_t* mod,
			     stg_polygon_t* polys, 
			     size_t poly_count )
{
  if( poly_count == 0 )
    {
      stg_model_map( mod, 0 ); // unmap the model from the matrix
      
      free( mod->polygons );
      mod->polygons_count = 0;      
    }
  else
    {
      assert(polys);
      
      stg_model_map( mod, 0 ); // unmap the model from the matrix
      
      // normalize the polygons to fit exactly in the model's body
      // rectangle (if the model has some non-zero size)
      if( mod->geom.size.x > 0.0 && mod->geom.size.y > 0.0 )
	stg_polygons_normalize( polys, poly_count, 
				mod->geom.size.x, 
				mod->geom.size.y );
      //else
      //PRINT_WARN3( "setting polygons for model %s which has non-positive area (%.2f,%.2f)",
      //	     prop->mod->token, geom.size.x, geom.size.y );
      
      // zap our old polygons
      stg_polygons_destroy( mod->polygons, mod->polygons_count );
      
      mod->polygons = polys;
      mod->polygons_count = poly_count;
      
      // if the model has some non-zero
      stg_model_map( mod, 1 ); // map the model into the matrix with the new polys
    }

  
  model_change( mod, &mod->polygons );
}



// set the pose of model in global coordinates 
void stg_model_set_global_pose( stg_model_t* mod, stg_pose_t* gpose )
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
  
  // HACK - completely rebuild the GUI elements - it's too complex to patch up the tree
  gui_model_destroy( mod );
  gui_model_create( mod );
  
  model_change( mod, &mod->parent );

  return 0; //ok
}


extern stg_rtk_fig_t* fig_debug_rays; 

int lines_raytrace_match( stg_model_t* mod, stg_model_t* hitmod )
{
  // Ignore invisible things, myself, my children, and my ancestors.
  if(  hitmod->obstacle_return  && (!stg_model_is_related(mod,hitmod)) ) 
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
	  if(  mod->gripper_return )
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
	  
	  stg_model_set_stall( mod, 1 );


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
      
      //int stall = 0;
      //stg_model_set_property( mod, "stall", &stall, sizeof(stall));
        
      stg_model_set_stall( mod, 0 );

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

