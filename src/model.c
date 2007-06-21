
#define _GNU_SOURCE

#include <limits.h> 
#include <assert.h>
#include <math.h>
//#include <string.h> // for strdup(3)

//#define DEBUG
//#undef DEBUG

#include "stage_internal.h"
#include "gui.h"

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

extern stg_type_record_t typetable[];

// RV tmp
stg_model_t* stg_model_test_collision2( stg_model_t* mod, 
					double* hitx, double* hity );

stg_endpoint_t* bubble( stg_endpoint_t* head, stg_endpoint_t* ep );

//extern int _stg_disable_gui;

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

  # unit square body shape
  polygons 1
  polygon[0].points 4
  polygon[0].point[0] [0 0]
  polygon[0].point[1] [0 1]
  polygon[0].point[2] [1 1]
  polygon[0].point[3] [1 0]

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
  
int model_unrender_velocity( stg_model_t* mod, void* userp );
int model_render_velocity( stg_model_t* mod, void* enabled );


stg_d_draw_t* stg_d_draw_create( stg_d_type_t type,
				 stg_vertex_t* verts,
				 size_t vert_count )
{
  size_t vert_mem_size = vert_count * sizeof(stg_vertex_t);
  
  // allocate space for the draw structure and the vertex data behind it
  stg_d_draw_t* d = 
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
  GList* it;
  for( it=list; it; it=it->next )
    g_free( it->data );
  
  // free the list elements themselves
  g_list_free( list );
}

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

// default update function that implements velocities for all objects
void model_move_due_to_velocity( stg_model_t* mod )
{
  stg_velocity_t* vel = &mod->velocity;
  
  // now move the model if it has any velocity
  if( vel && (vel->x || vel->y || vel->a ) )
    stg_model_update_pose( mod );
}


int endpoint_sort( stg_endpoint_t* a, stg_endpoint_t* b )
{
  if( a->value > b->value ) return 1;
  if( b->value < a->value ) return -1;
  return 0;
}


  // careful: no error checking - this must be fast
//static inline 
stg_endpoint_t* endpoint_right( stg_endpoint_t* head, stg_endpoint_t* link )
{
  assert( head );
  assert( link );
  assert( link->next );

  stg_endpoint_t *a = link->prev;
  stg_endpoint_t *b = link;
  stg_endpoint_t *c = link->next;
  stg_endpoint_t *d = link->next->next;

  if( a ) a->next = c; else head = c;
  if( d ) d->prev = b;

  b->prev = c;
  b->next = d;
  c->prev = a;
  c->next = b;
 
  return head;
}

// careful: no error checking - this must be fast
stg_endpoint_t* endpoint_left( stg_endpoint_t* head, stg_endpoint_t* link ) 
{
  return( endpoint_right( head, link->prev ) );
}


/* // careful: no error checking - this must be fast */
/* //static inline  */
/* GList* list_element_right( GList* head, GList* link ) */
/* { */
/*   assert( head ); */
/*   assert( link ); */
/*   assert( link->next ); */

/*   GList *a = link->prev; */
/*   GList *b = link; */
/*   GList *c = link->next; */
/*   GList *d = link->next->next; */

/*   if( a ) a->next = c; else head = c; */
/*   if( d ) d->prev = b; */

/*   b->prev = c; */
/*   b->next = d; */
/*   c->prev = a; */
/*   c->next = b; */
 
/*   return head; */
/* } */



/* void print_endpoint_list( stg_endpoint_t* head, char* prefix ) */
/* { */
/*   puts( prefix ); */
/*   for( ; head; head=head->next ) */
/*     printf( "\ttype:%d poly:%p value:%.2f\n",  */
/* 	    head->type, */
/* 	    head->polygon, */
/* 	    head->value ); */
/* } */

stg_endpoint_t* prepend_endpoint( stg_endpoint_t* head, stg_endpoint_t* ep )
{
  if( head )
    head->prev = ep;
  ep->next = head;
  return ep;
}

stg_endpoint_t* insert_endpoint_before( stg_endpoint_t* head, 
					stg_endpoint_t* before, 
					stg_endpoint_t* inserted )
{
  if( before == head ) // case when we're inserting at the head
    {
      head = prepend_endpoint( head, inserted );
    }
  else if( before == NULL ) // case when we're inserting at the tail    
    {
      stg_endpoint_t* last = head;
      while( last->next ) // spin to the last element
	last = last->next;
      
      last->next = inserted;
      inserted->prev = last;
      inserted->next = NULL;
    }
  else  // general case
    {
      inserted->next = before;
      inserted->prev = before->prev;
      inserted->prev->next = inserted;
      before->prev = inserted;
    }

  return head;
}

stg_endpoint_t* insert_endpoint_sorted( stg_endpoint_t* head, 
					stg_endpoint_t* ep )
{
  // zip down the list looking for the first value larger than
  // ep->value
  stg_endpoint_t* it;
  for( it=head; it; it=it->next )
    if( it->value > ep->value )
      break;
  
  // and insert the new ep before that larger endpoint
  return insert_endpoint_before( head, it, ep );
}

      
stg_endpoint_t* remove_endpoint( stg_endpoint_t* head, stg_endpoint_t* ep )
{
  if( ep->next )
    ep->next->prev = ep->prev;

  if( ep->prev )
    ep->prev->next = ep->next;
  
  if( head == ep ) // if we were at the head, there's a new head;
    return ep->next;
  else
    return head;
} 


// TODO - 3D-enable this
void stg_model_set_sense_volume( stg_model_t* mod, double vol[6] )
{
  assert( mod->sense_poly == NULL );
  
  stg_point_t pts[4];
  pts[0].x = vol[0];
  pts[1].x = vol[1];
  pts[2].x = vol[1];
  pts[3].x = vol[0];
  pts[0].y = vol[2];
  pts[1].y = vol[2];
  pts[2].y = vol[3];
  pts[3].y = vol[3];

  
  mod->sense_poly = g_new0( stg_polygon_t, 1 );  
  mod->sense_poly->mod = mod;
  mod->sense_poly->color = stg_lookup_color( "orange" );
  mod->sense_poly->unfilled = 0;
  mod->sense_poly->intersectors = NULL; 
  mod->sense_poly->accum  = g_hash_table_new( NULL, NULL );
  //mod->sense_poly->accum  = g_tree_new( accum_cmp );
  mod->sense_poly->points = g_array_new( FALSE, TRUE, sizeof(stg_point_t));
  g_array_append_vals( mod->sense_poly->points, pts, 4 );
  
  // endpoints
  // this is clunky
  for( int e=0; e<6; e++ )
    {
      mod->sense_poly->epts[e].polygon = mod->sense_poly;
      mod->sense_poly->epts[e].type = (e % 2) + 2; // STG_SENSOR_BEGIN or STG_SENSOR_END
      mod->sense_poly->epts[e].value = 0;
      mod->sense_poly->epts[e].next = NULL;      
      mod->sense_poly->epts[e].prev = NULL;      
    }  

  polygon_local_bounds_calc( mod->sense_poly );
  
}

stg_model_t* stg_model_create( stg_world_t* world, 
			       stg_model_t* parent,
			       stg_id_t id,
			       char* typestr )
{  
  
  stg_model_t* mod = g_new0( stg_model_t, 1 );
  
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

  mod->d_list = NULL;
  
  //printf( "creating model %d.%d(%s)\n", 
  //  mod->world->id,
  //  mod->id, 
  //  mod->token  );
  
  //PRINT_DEBUG1( "original token: %s", token );
  
  // add this model to its parent's list of children (if any)
  if( parent) parent->children = g_list_append( parent->children, mod );      

  // create this model's empty list of children
  mod->children = NULL;
  
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

  printf( "created model %s.\n", mod->token );
  
  // initialize the table of callbacks that are triggered when a
  // model's fields change
  mod->callbacks = g_hash_table_new( g_int_hash, g_int_equal );

  //stg_model_add_callback( mod, &mod->update, _model_update, NULL );

  mod->geom.size.x = STG_DEFAULT_GEOM_SIZEX;
  mod->geom.size.y = STG_DEFAULT_GEOM_SIZEX;
  mod->geom.size.z = STG_DEFAULT_GEOM_SIZEX;
  
  mod->obstacle_return = 1;
  mod->ranger_return = 1;
  mod->blob_return = 1;
  mod->laser_return = LaserVisible;
  mod->gripper_return = 0;
  mod->boundary = 0;
  mod->color = 0xFF0000; // red;  
  mod->map_resolution = 0.1; // meters
  mod->gui_nose = STG_DEFAULT_NOSE;
  mod->gui_grid = STG_DEFAULT_GRID;
  mod->gui_outline = STG_DEFAULT_OUTLINE;
  mod->gui_mask = mod->parent ? 0 : STG_DEFAULT_MASK;


  // now we can add the basic square shape
  mod->polys = NULL;
  stg_point_t* square = stg_unit_square_points_create();
  stg_model_add_polygon( mod, square, 4, mod->color, FALSE );


  //  stg_model_set_sense_volume( mod, vol );

  // now it's safe to create the GUI components
  if( mod->world->win )
    gui_model_init( mod );

  // exterimental: creates a menu of models
  // gui_add_tree_item( mod );
  
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
  if( mod->parent && mod->parent->children ) 
    mod->parent->children = g_list_remove( mod->parent->children, mod );
  
  // free all local stuff 
  // TODO - make sure this is up to date - check leaks with valgrind
  if( mod->world->win ) gui_model_destroy( mod );
  if( mod->children ) g_list_free( mod->children );
  if( mod->callbacks ) g_hash_table_destroy( mod->callbacks );
  if( mod->data ) g_free( mod->data );
  if( mod->cmd ) g_free( mod->cmd );
  if( mod->cfg ) g_free( mod->cfg );
  
  // and zap, we're gone
  g_free( mod );
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

  for( GList* it=mod->children; it; it=it->next )
    {
      if( stg_model_is_descendent(  (stg_model_t*)it->data, testmod ))
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
void stg_model_get_global_velocity( stg_model_t* mod, stg_velocity_t* gv )
{
  stg_pose_t gpose;
  stg_model_get_global_pose( mod, &gpose );

  double cosa = cos( gpose.a );
  double sina = sin( gpose.a );
  
  stg_velocity_t* lvel = &mod->velocity;
  gv->x = lvel->x * cosa - lvel->y * sina;
  gv->y = lvel->x * sina + lvel->y * cosa;
  gv->a = lvel->a;
      
  //printf( "local velocity %.2f %.2f %.2f\nglobal velocity %.2f %.2f %.2f\n",
  //  mod->velocity.x, mod->velocity.y, mod->velocity.a,
  //  gvel->x, gvel->y, gvel->a );
}

// set the model's velocity in the global frame
void stg_model_set_global_velocity( stg_model_t* mod, stg_velocity_t* gv )
{
  stg_pose_t gpose;
  stg_model_get_global_pose( mod, &gpose );
  
  double cosa = cos( gpose.a );
  double sina = sin( gpose.a );

  stg_velocity_t lv;
  lv.x = gv->x * cosa + gv->y * sina; 
  lv.y = -gv->x * sina + gv->y * cosa; 
  lv.a = gv->a;

  stg_model_set_velocity( mod, &lv );
}

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

      // no 3Dg geometry, as we can only rotate about the z axis (yaw)
      gpose->z = parent_pose.z + mod->parent->geom.size.z + pose->z;
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
  for( GList* it=mod->children; it; it=it->next )
    stg_model_map_with_children( (stg_model_t*)it->data, 
				 render);  
  // now map the model
  stg_model_map( mod, render );
}

// if render is true, render the model into the matrix, else unrender
// the model
void stg_model_map( stg_model_t* mod, gboolean render )
{
  assert( mod );
  
 /*  // to be drawn, we must have a body and some extent greater than zero */
/*   if( (mod->lines || mod->polygons) &&  */
/*       mod->geom.size.x > 0.0 && mod->geom.size.y > 0.0 ) */
/*     { */
/*       if( render ) */
/* 	{       */
/* 	  // get model's global pose */
/* 	  stg_pose_t org; */
/* 	  memcpy( &org, &mod->geom.pose, sizeof(org)); */
/* 	  stg_model_local_to_global( mod, &org ); */
	  
/* 	  if( mod->polygons && mod->polygons_count )     */
/* 	    stg_matrix_polygons( mod->world->matrix,  */
/* 				 org.x, org.y, org.a, */
/* 				 mod->polygons, mod->polygons_count,  */
/* 				 mod );   */
	  
/* 	  if( mod->lines && mod->lines_count )     */
/* 	    stg_matrix_polylines( mod->world->matrix,  */
/* 				  org.x, org.y, org.a, */
/* 				  mod->lines, mod->lines_count,  */
/* 				  mod );   */
	  
/* 	  if( mod->boundary )     */
/* 	    stg_matrix_rectangle( mod->world->matrix, */
/* 				  org.x, org.y, org.a, */
/* 				  mod->geom.size.x, */
/* 				  mod->geom.size.y, */
/* 				  mod );  */
/* 	} */
/*       else */
/* 	stg_matrix_remove_object( mod->world->matrix, mod ); */
/*     } */
}



void stg_model_subscribe( stg_model_t* mod )
{
  mod->subs++;
  mod->world->subs++;

  printf( "subscribe to %s %d\n", mod->token, mod->subs );
  
  //model_change( mod, &mod->lines );

  // if this is the first sub, call startup
  if( mod->subs == 1 )
    stg_model_startup(mod);
}

void stg_model_unsubscribe( stg_model_t* mod )
{
  mod->subs--;
  mod->world->subs--;

  printf( "unsubscribe from %s %d\n", mod->token, mod->subs );

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

void stg_model_print( stg_model_t* mod, char* prefix )
{
  printf( "%s model %d:%d:%s\n", 
	  prefix ? prefix : "", 
	  mod->world->id, 
	  mod->id, 
	  mod->token );
}

void model_print_cb( gpointer key, gpointer value, gpointer user )
{
  stg_model_print( (stg_model_t*)value, NULL );
}

// stg_model_load() is implemented in model_load.c
// stg_model_save() is implemented in model_load.c

void stg_model_update( stg_model_t* mod )
{
  //printf( "Updating model %s\n", mod->token );
  model_call_callbacks( mod, &mod->update );

  //GList* it;
  //for( it=mod->children; it; it=it->next )
  //stg_model_update( (stg_model_t*)it->data ); 
}

void stg_model_startup( stg_model_t* mod )
{
  //printf( "Startup model %s\n", mod->token );
  model_call_callbacks( mod, &mod->startup );
}

void stg_model_shutdown( stg_model_t* mod )
{
  //printf( "Shutdown model %s\n", mod->token );
  model_call_callbacks( mod, &mod->shutdown );
}

void model_change( stg_model_t* mod, void* address )
{
  //int offset = address - (void*)mod;
  //printf( "model %s at %p change at address %p offset %d \n",
  //  mod->token, mod, address, offset );
  
  model_call_callbacks( mod, address );
}

//------------------------------------------------------------------------
// basic model properties

void stg_model_set_data( stg_model_t* mod, void* data, size_t len )
{
  mod->data = g_realloc( mod->data, len );
  memcpy( mod->data, data, len );
  mod->data_len = len;

  //printf( "&mod->data %d %p\n",
  //  &mod->data, &mod->data );
  model_change( mod, &mod->data );
}

void stg_model_set_cmd( stg_model_t* mod, void* cmd, size_t len )
{
  mod->cmd = g_realloc( mod->cmd, len );
  memcpy( mod->cmd, cmd, len );
  mod->cmd_len = len;
  model_change( mod, &mod->cmd );
}

void stg_model_set_cfg( stg_model_t* mod, void* cfg, size_t len )
{
  mod->cfg = g_realloc( mod->cfg, len );
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


/** Given the end points of the two line segments (p1x,p1y) to
    (p2x,p2y) and (p3x,p3y) to (p4x,p4y), compute the intersection
    point of the lines. If the intersection point lies within both
    line segments return TRUE else FALSE.
 */
int line_segment_intersect( double p1x, double p1y,
			    double p2x, double p2y,
			    double p3x, double p3y,
			    double p4x, double p4y,
			    double *hitx, double *hity )
{
  double denom = (p4y-p3y)*(p2x-p1x) - (p4x-p3x)*(p2y-p1y);

  double ua = ((p4x-p3x)*(p1y-p3y)-(p4y-p3y)*(p1x-p3x)) / denom;
  double ub = ((p2x-p1x)*(p1y-p3y)-(p2y-p1y)*(p1x-p3x)) / denom;
  
  // store the point at which the lines intersect
  *hitx = p1x + ua * (p2x - p1x );
  *hity = p1y + ua * (p2y - p1y );
  
  // TRUE iff the point is within the specified line segments
  return( ua > 0 && ua <= 1 && ub > 0 && ub <= 1 );
}


void print_endpoint_list( stg_endpoint_t* ep, char* prefix )
{
  printf( "%s (list at %p)\n", prefix, ep );
  
  //  int i=0;

  for( int i=0; ep; ep=ep->next )
    {
      //assert( i < 30 );

      printf( "\t%d %p %.4f %d %p %d %s\n", 
	      i++, 
	      ep,
	      ep->value,
	      ep->type,
	      ep->polygon,
	      ep->polygon->points->len,
	      ep->polygon->mod->token );
      
    }
  puts(""); 
}

/* // careful: no error checking - this must be fast */
/* GList* list_link_left( GList* head, GList* link ) */
/* { */
/*   assert( head ); */
/*   assert( link ); */
/*   assert( link->prev ); */
  
/*   GList *a = link->prev->prev; */
/*   GList *b = link->prev; */
/*   GList *c = link; */
/*   GList *d = link->next; */

/*   if( a ) a->next = c; else head = c; */
/*   if( d ) d->prev = b; */

/*   b->prev = c; */
/*   b->next = d; */
/*   c->prev = a; */
/*   c->next = b; */

/*   return head; */
/* } */

/*   // careful: no error checking - this must be fast */
/* GList* list_link_right( GList* head, GList* link ) */
/* { */
/*   assert( head ); */
/*   assert( link ); */
/*   assert( link->next ); */

/*   GList *a = link->prev; */
/*   GList *b = link; */
/*   GList *c = link->next; */
/*   GList *d = link->next->next; */

/*   if( a ) a->next = c; else head = c; */
/*   if( d ) d->prev = b; */

/*   b->prev = c; */
/*   b->next = d; */
/*   c->prev = a; */
/*   c->next = b; */
 
/*   return head; */
/* } */


void polygon_local_bounds_calc( stg_polygon_t* poly )
{
  // assuming the polygon fits in a square +/- one billion units
  //double minx, miny, maxx, maxy;

  poly->minx = poly->miny = BILLION;
  poly->maxx = poly->maxy = -BILLION;
  
  stg_point_t *pt;

  //printf( "  local polygon bbox %p in model %s\n", 
  //  poly, poly->mod->token );

  for( int r=0; r<poly->points->len; r++ )
    {
      pt = & g_array_index( poly->points, stg_point_t, r );

      //printf( "point %.2f %.2f\n", pt->x, pt->y );

      poly->minx = MIN( poly->minx, pt->x );
      poly->maxx = MAX( poly->maxx, pt->x );
      poly->miny = MIN( poly->miny, pt->y );
      poly->maxy = MAX( poly->maxy, pt->y );
      //poly->minz = MIN( poly->minz, pt->z );
      //poly->maxz = MAX( poly->maxz, pt->z );
    }

  /* printf( "    minx %.2f  maxx %.2f  miny %.2f  maxy %.2f\n",  */
/* 	  poly->minx, poly->maxx,  */
/* 	  poly->miny, poly->maxy );  */
/*   printf( "    size  x %.2f  y %.2f [%.2f %.2f]\n",  */
/* 	  poly->maxx - poly->minx, */
/* 	  poly->maxy - poly->miny, */
/* 	  poly->mod->geom.size.x, */
/* 	  poly->mod->geom.size.y ); */
}  

void polygon_local_bounds_calc_cb( stg_polygon_t* poly, gpointer unused )
{
  polygon_local_bounds_calc( poly );
}

void polygon_global_bounds_calc( stg_polygon_t* poly )
{
  //printf( "  global polygon bbox %p in model %s\n", 
  //  poly, poly->mod->token );

  // compute the global positions of the four corners of the local
  // polygon bounding box
  stg_pose_t gpose;
  stg_model_get_global_pose( poly->mod, &gpose );

  double cosa = cos(gpose.a);
  double sina = sin(gpose.a);

  double dx1 = poly->minx * cosa - poly->miny * sina;
  double dx2 = poly->maxx * cosa - poly->miny * sina;
  double dx3 = poly->maxx * cosa - poly->maxy * sina;
  double dx4 = poly->minx * cosa - poly->maxy * sina;

  double dy1 = poly->minx * sina + poly->miny * cosa;
  double dy2 = poly->maxx * sina + poly->miny * cosa;
  double dy3 = poly->maxx * sina + poly->maxy * cosa;
  double dy4 = poly->minx * sina + poly->maxy * cosa;

  // find the global bounding box of the local bounding box
  double minx = MIN( dx1, MIN( dx2, MIN( dx3, dx4 )));
  double maxx = MAX( dx1, MAX( dx2, MAX( dx3, dx4 )));
  double miny = MIN( dy1, MIN( dy2, MIN( dy3, dy4 )));
  double maxy = MAX( dy1, MAX( dy2, MAX( dy3, dy4 )));

  // the endpoints store the global bounding box. We keep track of
  // overlapping global bboxes to prune object intersection tests.
  poly->epts[0].value = gpose.x + minx;
  poly->epts[1].value = gpose.x + maxx;
  poly->epts[2].value = gpose.y + miny;
  poly->epts[3].value = gpose.y + maxy;
  poly->epts[4].value = gpose.z;
  poly->epts[5].value = gpose.z + poly->mod->geom.size.z;



  // if our endpoints have been listed, we might need to shift
  // endpoints to a new position in the list

  if( poly->epts[0].next ||  poly->epts[0].prev )
    {
      stg_world_t* world = poly->mod->world;
      world->endpts.x = bubble( world->endpts.x, &poly->epts[0] );
      world->endpts.x = bubble( world->endpts.x, &poly->epts[1] );
      world->endpts.y = bubble( world->endpts.y, &poly->epts[2] );
      world->endpts.y = bubble( world->endpts.y, &poly->epts[3] );
      world->endpts.z = bubble( world->endpts.z, &poly->epts[4] );
      world->endpts.z = bubble( world->endpts.z, &poly->epts[5] );
    }

 /*  Printf( "    minx %.2f  maxx %.2f  miny %.2f  maxy %.2f  minz %.2f  maxz %.2f\n",  */
/* 	  poly->epts[0].value, */
/* 	  poly->epts[1].value, */
/* 	  poly->epts[2].value, */
/* 	  poly->epts[3].value, */
/* 	  poly->epts[4].value, */
/* 	  poly->epts[5].value ); */

/*   printf( "    size  x %.2f  y %.2f  z %.2f  [%.2f %.2f]\n",  */
/* 	  poly->epts[1].value - poly->epts[0].value, */
/* 	  poly->epts[3].value - poly->epts[2].value, */
/* 	  poly->epts[5].value - poly->epts[4].value, */
/* 	  poly->mod->geom.size.x, */
/* 	  poly->mod->geom.size.y ); */
}  

void polygon_global_bounds_calc_cb( stg_polygon_t* poly, gpointer unused )
{
  polygon_global_bounds_calc( poly );
}

void polygon_global_bounds_calc_and_bubble( stg_polygon_t* poly )
{
  polygon_global_bounds_calc( poly );

  // we might need to shift endpoints to a new position in the list
  stg_world_t* world = poly->mod->world;
  world->endpts.x = bubble( world->endpts.x, &poly->epts[0] );
  world->endpts.x = bubble( world->endpts.x, &poly->epts[1] );
  world->endpts.y = bubble( world->endpts.y, &poly->epts[2] );
  world->endpts.y = bubble( world->endpts.y, &poly->epts[3] );
  world->endpts.z = bubble( world->endpts.z, &poly->epts[4] );
  world->endpts.z = bubble( world->endpts.z, &poly->epts[5] );
}

void polygon_global_bounds_calc_and_bubble_cb( stg_polygon_t* poly, 
					       gpointer unused )
{
  polygon_global_bounds_calc( poly );
}

int polygon_compare( stg_polygon_t* p1, stg_polygon_t* p2 )
{
  if( p1 < p2 )
    return -1;

  if( p1 > p2 )
    return 1;

  return 0; // same
}

void stg_model_add_polygon( stg_model_t* mod, 
			    stg_point_t* pts, 
			    size_t pt_count, 
			    stg_color_t color,
			    stg_bool_t unfilled )
{
  // block experiment
  mod->blocks = g_list_prepend( mod->blocks, 
				stg_block_create( 0,
						  0,
						  0,
						  0,
						  mod->geom.size.y,  
						  mod->color,
						  pts,
						  pt_count ));
  

/*   printf( "adding a polygon to the %d polygons of model %s\n",   */
/* 	  g_list_length(mod->polys),  */
/* 	  mod->token ); */
  
  stg_polygon_t* poly = g_new0( stg_polygon_t, 1 );
  
  poly->mod = mod;
  poly->color = color;
  poly->unfilled = unfilled;
  poly->intersectors = NULL; 
  
  // create a hash table using pointers directly as keys
  //poly->accumulator = g_hash_table_new( NULL, NULL );
  poly->accum  = g_hash_table_new( NULL, NULL );
  
  // copy the points into the poly
  poly->points = g_array_new( FALSE, TRUE, sizeof(stg_point_t));
  g_array_append_vals( poly->points, pts, pt_count );
  
  stg_vertex_t* verts = g_new( stg_vertex_t, pt_count );

  int i;
  for( i=0; i<poly->points->len; i++ )
    {
      stg_point_t* pt = &g_array_index( poly->points, stg_point_t, i );
      verts[i].x = pt->x;
      verts[i].y = pt->y; 
      verts[i].z = 0; 
    }

  // endpoints
  // this is clunky
  for( int e=0; e<6; e++ )
    {
      poly->epts[e].polygon = poly;
      poly->epts[e].type = e % 2; // STG_BEGIN or STG_END
      poly->epts[e].value = 0;
      poly->epts[e].next = NULL;      
      poly->epts[e].prev = NULL;      
    }
  
  // experimental 
  mod->d_list = 
    g_list_append( mod->d_list,  
		   stg_d_draw_create( STG_D_DRAW_POLYGON, verts, pt_count ) );

  g_free( verts ); // we're done with this now
  
  // add this model's endpoints to the world's lists
/*   stg_endpoint3_t* ep3 = &mod->world->endpts; */
/*   ep3->x = insert_endpoint( ep3->x, &poly->epts[0] );  */
/*   ep3->x = insert_endpoint( ep3->x, &poly->epts[1] );   */
/*   ep3->y = insert_endpoint( ep3->y, &poly->epts[2] );  */
/*   ep3->y = insert_endpoint( ep3->y, &poly->epts[3] );  */
/*   ep3->z = insert_endpoint( ep3->z, &poly->epts[4] );  */
/*   ep3->z = insert_endpoint( ep3->z, &poly->epts[5] );  */

  //print_endpoint_list( ep3->x, "inserted new endpoints pair into X list. List now:" );
  //print_endpoint_list( ep3->y, "inserted new endpoints pair into Y list. List now:" );
  //print_endpoint_list( ep3->z, "inserted new endpoints pair into Z list. List now:" );
  
  mod->polys = g_list_prepend( mod->polys, poly );  
}


void print_intersector( stg_polygon_t* poly, char* separator )
{
  printf( "%s:%p:(%.2f,%.2f)(%.2f,%.2f)(%.2f,%.2f)%s",
	  poly->mod->token,
	  poly,
	  poly->epts[0].value,
	  poly->epts[1].value,
	  poly->epts[2].value,
	  poly->epts[3].value,
	  poly->epts[4].value,
	  poly->epts[5].value,
	  separator );
}

void print_pointer( gpointer p, char* separator )
{
  printf( "%p%s", p, separator) ;
}

gboolean print_tree( stg_polygon_t* poly, int* val, gpointer unused )
{
  printf( "(%s %p %d) ", 
	  poly->mod->token, 
	  poly, 
	  *val );
  return FALSE;
}

gboolean untangle( stg_polygon_t* poly, int* val, stg_polygon_t* doomed_poly )
{
  // remove doomed from acc_tree
  g_hash_table_remove( poly->accum, doomed_poly );

  // remove doomed from intercept list
  poly->intersectors = g_list_remove( poly->intersectors, doomed_poly );

  printf( "untangled [%s %p] accum ", poly->mod->token, poly );
  g_hash_table_foreach( poly->accum, print_tree, NULL );
  puts("");

  return FALSE;
}


void stg_polygon_destroy( stg_polygon_t* p )
{
  printf( "DESTROY polygon %s %p\n", p->mod->token, p );

  //printf( "polys to untangle from: " );
  //g_hash_table_foreach( p->accum, print_tree, NULL );
  //puts("");

  g_hash_table_foreach( p->accum, untangle, p );

  if( p->points )
    g_array_free( p->points, TRUE );
  
  if( p->intersectors )
    g_list_free( p->intersectors );

  if( p->accum )
    g_hash_table_destroy( p->accum );
  
  // remove the endpoints
  stg_endpoint3_t* ep3 = &p->mod->world->endpts;
  ep3->x = remove_endpoint( ep3->x, &p->epts[0] );
  ep3->x = remove_endpoint( ep3->x, &p->epts[1] ); 
  ep3->y = remove_endpoint( ep3->y, &p->epts[2] ); 
  ep3->y = remove_endpoint( ep3->y, &p->epts[3] ); 
  ep3->z = remove_endpoint( ep3->z, &p->epts[4] ); 
  ep3->z = remove_endpoint( ep3->z, &p->epts[5] ); 
  
  //print_endpoint_list( ep3->x, "removed endpoint pair. X List now:" );      
  //print_endpoint_list( ep3->y, "removed endpoint pair. Y List now:" );      
  //print_endpoint_list( ep3->z, "removed endpoint pair. Z List now:" );      
  g_free( p );
}


void stg_model_clear_polygons( stg_model_t* mod )
{
  for( GList* it=mod->polys; it; it=it->next )
    stg_polygon_destroy( (stg_polygon_t*)it->data );
  
  g_list_free( mod->polys );

  list_gfree( mod->d_list );
  mod->d_list = NULL;

  // todo - free the blocks memory
  mod->blocks = NULL;


  mod->polys = NULL;
}


//static inline 
void stg_polygons_intersect_incr( stg_polygon_t* poly1, stg_polygon_t* poly2 )
{
/*   printf( "* INCR %s:%p - %s:%p\n", */
/* 	  poly1->mod->token, poly1, */
/* 	  poly2->mod->token, poly2 ); */

  // increment mod's accumulator for the hit model, and see if we
  // freshly overlap with it
  int* val = (int*)g_hash_table_lookup( poly1->accum, poly2 );

  if( val == NULL )
    {
      val = g_new0( int, 1 );
      g_hash_table_insert( poly1->accum, poly2, val );
      g_hash_table_insert( poly2->accum, poly1, val );
    }
  // increment the intersection axis counters for the two polygons
  (*val)++;

  assert( *val < 4 ); 

  if( *val == 3 ) // a new 3-axis overlap!
    {
      poly1->intersectors = g_list_prepend( poly1->intersectors, poly2 );
      poly2->intersectors = g_list_prepend( poly2->intersectors, poly1 );
    }
}



//static inline 
void stg_polygons_intersect_decr( stg_polygon_t* poly1, stg_polygon_t* poly2 )
{
 /*  printf( "* DECR %s:%p - %s:%p\n", */
/* 	  poly1->mod->token, poly1, */
/* 	  poly2->mod->token, poly2 ); */

  int* val = (int*)g_hash_table_lookup( poly1->accum, poly2 );

  if( val == NULL )
    return;

  // val should exist, be symmetrical, and > 0
  assert( val );
  assert( val == (int*)g_hash_table_lookup( poly2->accum, poly1 ));
  assert( *val > 0 ); 
  
  // decrement the intersection counter for the two polygons
  (*val)--;
 
  switch( *val )
    {
    case 2:
      // the polys no longer overlap in 3 axes
      poly1->intersectors = g_list_remove( poly1->intersectors, poly2 );
      poly2->intersectors = g_list_remove( poly2->intersectors, poly1 );
      break;

    case 0:
      g_hash_table_remove( poly1->accum, poly2 );
      g_hash_table_remove( poly2->accum, poly1 );
      g_free( val );
      break;

    default:
      break;
    }
}


//static inline 
stg_endpoint_t* endpoint_right_intersect( stg_endpoint_t* head,  stg_endpoint_t* ep )
{
  stg_endpoint_t* r_ep = ep->next;
  
  if( r_ep->polygon != ep->polygon ) // TODO - remove this test?
    {
      if( (ep->type == STG_END) && (r_ep->type == STG_BEGIN) )
	  stg_polygons_intersect_incr( ep->polygon, r_ep->polygon );
      else if( (ep->type == STG_BEGIN) && (r_ep->type == STG_END) )
	  stg_polygons_intersect_decr( ep->polygon, r_ep->polygon );
    }
  
  return endpoint_right( head, ep );
}    

// TODO - move loop inside function calls
// locally shifts the endpoint into order in its list. returns the
// head of the list, which may have changed
//static inline 
stg_endpoint_t* bubble( stg_endpoint_t* head, stg_endpoint_t* ep )
{
  while( ep->next && (ep->next->value < ep->value) )
    head = endpoint_right_intersect( head, ep );
  
  while( ep->prev && (ep->prev->value > ep->value) )
    head = endpoint_right_intersect( head, ep->prev );
  
  return head;
}


/* static inline void world_move_endpoint( stg_world_t* world, stg_endpoint_t* epts ) */
/* { */
/*   // bubble sort the end points of the bounding box in the lists along */
/*   // each axis - local bubble sort is VERY fast because the list is
already almost perfectly sorted, so we usually don't */
/*   // move, but occasionally move 1 place left or right */

/*   world->endpts.x = bubble( world->endpts.x, &epts[0] ); */
/*   world->endpts.x = bubble( world->endpts.x, &epts[1] ); */
/*   world->endpts.y = bubble( world->endpts.y, &epts[2] ); */
/*   world->endpts.y = bubble( world->endpts.y, &epts[3] ); */
/*   world->endpts.z = bubble( world->endpts.z, &epts[4] ); */
/*   world->endpts.z = bubble( world->endpts.z, &epts[5] ); */
/* } */


/* void model_update_bbox( stg_model_t* mod ) */
/* { */
/*   //printf( "UPDATE BBOX for model %s\n", mod->token ); */

/*   stg_pose_t gpose; */
/*   stg_model_get_global_pose( mod, &gpose ); */

/*   double dx_x =  fabs(mod->geom.size.x * cos(gpose.a));  */
/*   double dx_y =  fabs(mod->geom.size.y * sin(gpose.a)); */
/*   double dx = dx_x + dx_y; */

/*   double dy_x = fabs(mod->geom.size.x * sin(gpose.a)); */
/*   double dy_y = fabs(mod->geom.size.y * cos(gpose.a)); */
/*   double dy = dy_x + dy_y; */

/*   double dz = mod->geom.size.z; */

/*   // stuff these data into the endpoint structures */
/* /\*   mod->epbbox.x.min.value = gpose.x - dx/2.0; *\/ */
/* /\*   mod->epbbox.x.max.value = gpose.x + dx/2.0; *\/ */
/* /\*   mod->epbbox.y.min.value = gpose.y - dy/2.0; *\/ */
/* /\*   mod->epbbox.y.max.value = gpose.y + dy/2.0; *\/ */
/* /\*   mod->epbbox.z.min.value = gpose.z; *\/ */
/* /\*   mod->epbbox.z.max.value = gpose.z + dz; *\/ */
  
/* /\*   world_move_endpoint_bbox( mod->world, &mod->epbbox ); *\/ */


/*   //world_intercept_array_print( mod->world ); */
/* } */



void stg_model_set_pose( stg_model_t* mod, stg_pose_t* pose )
{
/*   printf( "MODEL \"%s\" SET POSE (%.2f %.2f %.2f)\n", */
/* 	  mod->token, pose->x, pose->y, pose->a ); */

  assert(mod);
  assert(pose);


  // if the pose has changed, we need to do some work
  if( memcmp( &mod->pose, pose, sizeof(stg_pose_t) ) != 0 )
    {
      // unrender from the matrix
      stg_model_map_with_children( mod, 0 );
      
      memcpy( &mod->pose, pose, sizeof(stg_pose_t));
      
      // TODO - can we do this less frequently? maybe not...
      for( GList* it=mod->children; it; it=it->next )
	//model_update_bbox( (stg_model_t*)it->data );
	g_list_foreach( ((stg_model_t*)it->data)->polys, 
			(GFunc)polygon_global_bounds_calc_cb, NULL );

      double hitx, hity;
      stg_model_test_collision2( mod, &hitx, &hity );

      // render in the matrix
      stg_model_map_with_children( mod, 1 );
    }

  g_list_foreach( mod->polys, (GFunc)polygon_global_bounds_calc_cb, NULL );
  
  // also deal with the sensor volume
  if( mod->sense_poly )
    {
      //      printf( "model %s sense global \n", mod->token );
      polygon_global_bounds_calc( mod->sense_poly );
    }

  // register a model change even if the pose didn't actually change
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
  printf( "MODEL \"%s\" SET GEOM (%.2f %.2f %.2f)[%.2f %.2f]\n",
	  mod->token,
	  geom->pose.x, geom->pose.y, geom->pose.a,
	  geom->size.x, geom->size.y );

  assert(mod);
  assert(geom);
 
  // unrender from the matrix
  stg_model_map( mod, 0 );

  memcpy( &mod->geom, geom, sizeof(stg_geom_t));
  
  model_change( mod, &mod->geom );

  // we probably need to scale and re-render our polygons

  // we can do it in-place
  if( mod->polys )
    stg_polygons_normalize( mod->polys,
			    geom->size.x, 
			    geom->size.y );
  
  g_list_foreach( mod->polys, (GFunc)polygon_local_bounds_calc_cb, NULL );
  g_list_foreach( mod->polys, (GFunc)polygon_global_bounds_calc_cb, NULL );
  
  // re-render int the matrix
  stg_model_map( mod, 1 );  
  
  model_change( mod, &mod->polys );
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

void stg_model_set_fiducial_key( stg_model_t* mod, int key )
{
  assert(mod);
  mod->fiducial_key = key;
  model_change( mod, &mod->fiducial_key );
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


GList* stg_model_get_polygons( stg_model_t* mod )
{
  assert(mod);
  return mod->polys;
}

/* void stg_model_set_polygons( stg_model_t* mod, */
/* 			     stg_polygon_t* polys,  */
/* 			     size_t poly_count ) */
/* { */
/*   if( poly_count == 0 ) */
/*     { */
/*       stg_model_map( mod, 0 ); // unmap the model from the matrix */
      
/*       free( mod->polygons ); */
/*       mod->polygons_count = 0;       */
/*     } */
/*   else */
/*     { */
/*       assert(polys); */
      
/*       stg_model_map( mod, 0 ); // unmap the model from the matrix */
      
/*       // normalize the polygons to fit exactly in the model's body */
/*       // rectangle (if the model has some non-zero size) */
/*       if( mod->geom.size.x > 0.0 && mod->geom.size.y > 0.0 ) */
/* 	stg_polygons_normalize( polys, poly_count,  */
/* 				mod->geom.size.x,  */
/* 				mod->geom.size.y ); */
/*       //else */
/*       //PRINT_WARN3( "setting polygons for model %s which has non-positive area (%.2f,%.2f)", */
/*       //	     prop->mod->token, geom.size.x, geom.size.y ); */
      
/*       // zap our old polygons */
/*       stg_polygons_destroy( mod->polygons, mod->polygons_count ); */
      
/*       mod->polygons = polys; */
/*       mod->polygons_count = poly_count; */
      
/*       // if the model has some non-zero */
/*       stg_model_map( mod, 1 ); // map the model into the matrix with the new polys */

/*       // add the polys to the endpoint lists */

/*       int p; */
/*       for ( p=0; p<poly_count; p++ ) */
/* 	{ */
/* 	  endpoint_bbox_init( &polys[p].epbbox, mod, 0,1,0,1,0,1 ); */
/* 	  world_add_endpoint_bbox( mod->world, &polys[p].epbbox ); */
/* 	}       */

/*     } */

  
/*   model_change( mod, &mod->polygons ); */
/* } */



void stg_polyline_print( stg_polyline_t* l )
{
  assert(l);

  printf( "Polyline %p contains %d points [",
	  l, (int)l->points_count );

  for( int p=0; p<l->points_count; p++ )
    printf( "[%.2f,%.2f] ", l->points[p].x, l->points[p].y );
  
  puts( "]" );
}

void stg_polylines_print( stg_polyline_t* l, size_t p_count )
{
  assert( l );
  printf( "Polyline array %p contains %d polylines\n",
	  l, (int)p_count );

  for( int a=0; a<p_count; a++ )
    stg_polyline_print( &l[a] );
}

void stg_model_set_polylines( stg_model_t* mod,
			      stg_polyline_t* lines, 
			      size_t lines_count )
{
  //stg_polylines_print( lines, lines_count );
  
  if( lines_count == 0 )
    {
      stg_model_map( mod, 0 ); // unmap the model from the matrix
      
      free( mod->lines );
      mod->lines_count = 0;      
    }
  else
    {
      assert(lines);
      
      stg_model_map( mod, 0 ); // unmap the model from the matrix
      
      // normalize the polygons to fit exactly in the model's body
      // rectangle (if the model has some non-zero size)
      //if( mod->geom.size.x > 0.0 && mod->geom.size.y > 0.0 )
      //stg_polygons_normalize( polys, poly_count, 
      //			mod->geom.size.x, 
      //			mod->geom.size.y );
      //else
      //PRINT_WARN3( "setting polygons for model %s which has non-positive area (%.2f,%.2f)",
      //	     prop->mod->token, geom.size.x, geom.size.y );
      
      // zap our old polygons
      //stg_polygons_destroy( mod->polygons, mod->polygons_count );
      
      free( mod->lines );
      
      mod->lines = lines;
      mod->lines_count = lines_count;
      
      // if the model has some non-zero
      stg_model_map( mod, 1 ); // map the model into the matrix with the new polys
    }
  
  model_change( mod, &mod->lines );
  model_change( mod, &mod->lines_count );
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
    mod->parent->children = g_list_remove( mod->parent->children, mod );

  if( newparent )
    newparent->children = g_list_append( newparent->children, mod );

  // link from the model to its new parent
  mod->parent = newparent;
  
  // HACK - completely rebuild the GUI elements - it's too complex to patch up the tree
  gui_model_destroy( mod );
  gui_model_create( mod );
  model_change( mod, &mod->parent );

  return 0; //ok
}



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
stg_model_t* stg_model_test_collision( stg_model_t* mod, 
				       //stg_pose_t* pose, 
				       double* hitx, double* hity )
{
  return 0;
  
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
}



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

stg_model_t* stg_model_test_collision2( stg_model_t* mod, 
					//stg_pose_t* pose, 
					double* hitx, double* hity )
{
 /*  int ch; */
/*   stg_model_t* child_hit = NULL; */
/*   for(ch=0; ch < mod->children->len; ch++ ) */
/*     { */
/*       stg_model_t* child = g_ptr_array_index( mod->children, ch ); */
/*       child_hit = stg_model_test_collision2( child, hitx, hity ); */
/*       if( child_hit ) */
/* 	return child_hit; */
/*     } */
  

  stg_pose_t pose;
  memcpy( &pose, &mod->geom.pose, sizeof(pose));
  stg_model_local_to_global( mod, &pose );
  
  //return NULL;
  
  // raytrace along all our rectangles. expensive, but most vehicles
  // will just be a single rect, grippers 3 rects, etc. not too bad.
  
  //size_t count=0;
  //stg_polygon_t* polys = stg_model_get_polygons(mod, &count);

  // no body? no collision
  //if( count < 1 )
  //return NULL;

  // first find the models whose bounding box overlaps with this one

/*   printf ("COLLISION TESTING MODEL %s\n", mod->token ); */
  GList* candidates = NULL;//stg_model_overlappers( mod, mod->world->children ); */

/*   if( candidates ) */
/*     printf( "mod %s overlappers: %d\n", mod->token, g_list_length( candidates )); */
/*   else */
/*     printf( "mod %s has NO overlappers\n", mod->token ); */


  // now test every line segment in the candidates for intersection
  // and record the closest hitpoint
/*   GList* model_segments = model_append_segments( mod, NULL ); */
  
/*   GList* obstacle_segments = NULL; */
  
/*   GList* it; */
/*   for( it=candidates; it; it=it->next ) */
/*     obstacle_segments =  */
/*       model_append_segments( (stg_model_t*)it->data, obstacle_segments ); */
 

  if( candidates )
    g_list_free( candidates );
	  


  return NULL;  // done 
}



int stg_model_update_pose( stg_model_t* mod )
{ 
  PRINT_DEBUG4( "pose update model %d (vel %.2f, %.2f %.2f)", 
		mod->id, mod->velocity.x, mod->velocity.y, mod->velocity.a );
 
  stg_velocity_t gvel;
  stg_model_get_global_velocity( mod, &gvel );
      
  stg_pose_t gpose;
  stg_model_get_global_pose( mod, &gpose );

  // convert msec to sec
  double interval = (double)mod->world->sim_interval / 1000.0;
  

  stg_pose_t old_pose;
  memcpy( &old_pose, &mod->pose, sizeof(old_pose));

  // compute new pose
  //gpose.x += gvel.x * interval;
  //gpose.y += gvel.y * interval;
  //gpose.a += gvel.a * interval;
  mod->pose.x += (mod->velocity.x * cos(mod->pose.a) - mod->velocity.y * sin(mod->pose.a)) * interval;
  mod->pose.y += (mod->velocity.x * sin(mod->pose.a) + mod->velocity.y * cos(mod->pose.a)) * interval;
  mod->pose.a += mod->velocity.a * interval;

  // check this model and all it's children at the new pose
  double hitx=0, hity=0, hita=0;
  stg_model_t* hitthing =
    //NULL; //stg_model_test_collision( mod, &hitx, &hity );
    stg_model_test_collision2( mod, &hitx, &hity );

  int stall = 0;
      
  if( hitthing )
    {
      // grippable objects move when we hit them
      if(  hitthing->gripper_return ) 
	{
	  //PRINT_WARN( "HIT something grippable!" );
	  
	  stg_velocity_t hitvel;
	  //hitvel.x = gvel.x;
	  //hitvel.y = gvel.y;
	  //hitvel.a = 0.0;

	  stg_pose_t hitpose;
	  stg_model_get_global_pose( hitthing, &hitpose );
	  double hita = atan2( hitpose.y - hity, hitpose.x - hitx );
	  double mag = 0.3;
	  hitvel.x =  mag * cos(hita);
	  hitvel.y = mag * sin(hita);
	  hitvel.a = 0;

	  stg_model_set_global_velocity( hitthing, &hitvel );
	  
	  stall = 0;
	  // don't make the move - reset the pose
	  //memcpy( &mod->pose, &old_pose, sizeof(mod->pose));
	}
      else // other objects block us totally
	{
	  hitthing = NULL;
	  // move back to where we started
	  memcpy( &mod->pose, &old_pose, sizeof(mod->pose));
	  interval = 0.2 * interval; // slow down time
	  
	  // inch forward until we collide
	  do
	    {
	      memcpy( &old_pose, &mod->pose, sizeof(old_pose));
	      
	      mod->pose.x += 
		(mod->velocity.x * cos(mod->pose.a) - mod->velocity.y * sin(mod->pose.a)) * interval;
	      mod->pose.y += 
		(mod->velocity.x * sin(mod->pose.a) + mod->velocity.y * cos(mod->pose.a)) * interval;
	      mod->pose.a += mod->velocity.a * interval;
	      
	      hitthing = stg_model_test_collision( mod, &hitx, &hity );
	      
	    } while( hitthing == NULL );
	  
	  //PRINT_WARN( "HIT something immovable!" );
	  
	  stall = 1;
	  
	  // set velocity to zero
	  stg_velocity_t zero_v;
	  memset( &zero_v, 0, sizeof(zero_v));
	  stg_model_set_velocity( mod, &zero_v );

	  // don't make the move - reset the pose
	  memcpy( &mod->pose, &old_pose, sizeof(mod->pose));

	}
    }

  // if the pose changed, we need to set it.
  if( memcmp( &old_pose, &mod->pose, sizeof(old_pose) ))
      stg_model_set_pose( mod, &mod->pose );

  stg_model_set_stall( mod, stall );
  
  /* trails */
  //if( fig_trails )
  //gui_model_trail()


  // if I'm a pucky thing, slow down for next time - simulates friction
  if( mod->gripper_return )
    {
      double slow = 0.08;

      if( mod->velocity.x > 0 )
	{
	  mod->velocity.x -= slow;
	  mod->velocity.x = MAX( 0, mod->velocity.x );
	}
      else if ( mod->velocity.x < 0 )
	{
	  mod->velocity.x += slow;
	  mod->velocity.x = MIN( 0, mod->velocity.x );
	}
      
      if( mod->velocity.y > 0 )
	{
	  mod->velocity.y -= slow;
	  mod->velocity.y = MAX( 0, mod->velocity.y );
	}
      else if( mod->velocity.y < 0 )
	{
	  mod->velocity.y += slow;
	  mod->velocity.y = MIN( 0, mod->velocity.y );
	}

      model_change( mod, &mod->velocity );
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
  
  GList* it;
  for(it=root->children; it; it=it->next )
    {
      stg_model_t* child = (stg_model_t*)it->data;
      added += stg_model_tree_to_ptr_array( child, array );
    }
  
  return added;
}


#define GLOBAL_VECTORS 0


#define MATCH(A,B) (strcmp(A,B)== 0)

/*int ISPROP( char* name, char* match )
{
  return( strcmp( name, match ) == 0 );
}
*/


int stg_model_set_property( stg_model_t* mod,
			    char* key,
			    void* data )
{
  // see if the key has the predefined-property prefix
  if( strncmp( key, STG_MP_PREFIX, strlen(STG_MP_PREFIX)) == 0 )
    {
      PRINT_DEBUG1( "Looking up model core property \"%s\"\n", key );
      
      if( MATCH(key,STG_MP_FIDUCIAL_RETURN) )
	{
	  stg_model_set_fiducial_return( mod, *(int*)data );
	  return 0;
	}
      if( MATCH( key, STG_MP_LASER_RETURN ) )
	{
	  stg_model_set_laser_return( mod, *(int*)data );
	  return 0;
	}
      if( MATCH( key, STG_MP_OBSTACLE_RETURN ) )
	{
	  stg_model_set_obstacle_return( mod, *(int*)data );
	  return 0;
	}
      if( MATCH( key, STG_MP_RANGER_RETURN ) )
	{
	  stg_model_set_ranger_return( mod, *(int*)data );
	  return 0;
	}
      if( MATCH( key, STG_MP_GRIPPER_RETURN ) )
	{
	  stg_model_set_gripper_return( mod, *(int*)data );
	  return 0;
	}
      if( MATCH( key, STG_MP_COLOR ) )
	{
	  stg_model_set_color( mod, *(int*)data );
	  return 0;
	}
      if( MATCH( key, STG_MP_MASS ) )
	{
	  stg_model_set_mass( mod, *(double*)data );
	  return 0;
	}
      if( MATCH( key, STG_MP_WATTS ) )
	{
	  stg_model_set_watts( mod, *(double*)data );
	  return 0;
	}
      
      PRINT_ERR1( "Attempt to set non-existent model core property \"%s\"", key );
      return 1; // error code
    }

  // otherwise it's an arbitary property and we store the pointer
  g_datalist_set_data( &mod->props, key, data ); 
  return 0; // ok
}

void* stg_model_get_property( stg_model_t* mod, char* key )
{
  // see if the key has the predefined-property prefix
  if( strncmp( key, STG_MP_PREFIX, strlen(STG_MP_PREFIX)) == 0 )
    {
      if( MATCH( key, STG_MP_COLOR))            return (void*)&mod->color;
      if( MATCH( key, STG_MP_MASS))             return (void*)&mod->mass;
      if( MATCH( key, STG_MP_WATTS))            return (void*)&mod->watts;
      if( MATCH( key, STG_MP_FIDUCIAL_RETURN))  return (void*)&mod->fiducial_return;
      if( MATCH( key, STG_MP_LASER_RETURN))     return (void*)&mod->laser_return;
      if( MATCH( key, STG_MP_OBSTACLE_RETURN))  return (void*)&mod->obstacle_return;
      if( MATCH( key, STG_MP_RANGER_RETURN))    return (void*)&mod->ranger_return;
      if( MATCH( key, STG_MP_GRIPPER_RETURN))   return (void*)&mod->gripper_return;

      PRINT_WARN1( "Requested non-existent model core property \"%s\"", key );
      return NULL;
    }
  
  // otherwise it may be an arbitrary named property
  return g_datalist_get_data( &mod->props, key );
}

void stg_model_unset_property( stg_model_t* mod, char* key )
{
  if( strncmp( key, STG_MP_PREFIX, strlen(STG_MP_PREFIX)) == 0 )
    PRINT_WARN1( "Attempt to unset a model core property \"%s\" has no effect", key );
  else
    g_datalist_remove_data( &mod->props, key );
}

