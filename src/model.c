/**
@defgroup stg_model 

Implements a model - the basic object of Stage simulation

*/

#define PACKPOSE(P,X,Y,A) {P->x=X; P->y=Y; P->a=A;}

#include <assert.h>
#include <math.h>

//#define DEBUG
//#undef DEBUG

#include "stage.h"
//#include "gui.h"
//#include "raytrace.h"

/// convert a global pose into the model's local coordinate system
void stg_model_global_to_local( stg_model_t* mod, stg_pose_t* pose )
{
  //printf( "g2l global pose %.2f %.2f %.2f\n",
  //  pose->x, pose->y, pose->a );

  // get model's global pose
  stg_pose_t org;
  stg_model_global_pose( mod, &org );

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

/// create a new model
stg_model_t* stg_model_create( stg_world_t* world, 
			       stg_model_t* parent,
			       stg_id_t id, 
			       stg_model_type_t type,
			       char* token )
{  
  stg_model_t* mod = calloc( sizeof(stg_model_t),1 );

  mod->id = id;

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
  mod->blob_return = TRUE;
  
  // sensible defaults
  mod->stall = FALSE;

  mod->pose.x = STG_DEFAULT_POSEX;
  mod->pose.y = STG_DEFAULT_POSEY;
  mod->pose.a = STG_DEFAULT_POSEA;

  mod->geom.pose.x = STG_DEFAULT_GEOM_POSEX;
  mod->geom.pose.y = STG_DEFAULT_GEOM_POSEY;
  mod->geom.pose.a = STG_DEFAULT_GEOM_POSEA;
  mod->geom.size.x = STG_DEFAULT_GEOM_SIZEX;
  mod->geom.size.y = STG_DEFAULT_GEOM_SIZEY;
  
  stg_guifeatures_t gf;
  gf.show_data = 1;
  gf.show_cmd = 0;
  gf.show_cfg = 0;
  gf.boundary =  STG_DEFAULT_GUI_BOUNDARY;
  gf.nose =  STG_DEFAULT_GUI_NOSE;
  gf.grid = STG_DEFAULT_GUI_GRID;
  gf.movemask = STG_DEFAULT_GUI_MOVEMASK;  
  stg_model_set_guifeatures( mod, &gf );

  // zero velocity
  memset( &mod->velocity, 0, sizeof(mod->velocity) );
  
  memset(&mod->energy_config,0,sizeof(mod->energy_config));
  mod->energy_config.capacity = STG_DEFAULT_ENERGY_CAPACITY;
  mod->energy_config.give_rate = STG_DEFAULT_ENERGY_GIVERATE;
  mod->energy_config.probe_range = STG_DEFAULT_ENERGY_PROBERANGE;      
  mod->energy_config.trickle_rate = STG_DEFAULT_ENERGY_TRICKLERATE;

  memset(&mod->energy_data,0,sizeof(mod->energy_data));
  mod->energy_data.joules = mod->energy_config.capacity;
  mod->energy_data.watts = 0;
  mod->energy_data.charging = FALSE;
  mod->energy_data.range = mod->energy_config.probe_range;

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
		
  // initialize odometry
  memset( &mod->odom, 0, sizeof(mod->odom));
  
  mod->friction = 0.0;
  
  // install the default functions
   mod->f_startup = _model_startup;
  mod->f_shutdown = _model_shutdown;
  mod->f_update = _model_update;
  mod->f_set_data = _model_set_data;
  mod->f_get_data =  _model_get_data;
  mod->f_set_command = _model_set_cmd;
  mod->f_get_command = _model_get_cmd;
  mod->f_set_config = _model_set_cfg;
  mod->f_get_config = _model_get_cfg;
  mod->f_render_data = NULL;
  mod->f_render_cmd = NULL;
  mod->f_render_cfg = NULL;
  
  PRINT_DEBUG4( "finished model %d.%d(%s) type %s", 
		mod->world->id, mod->id, 
		mod->token, stg_model_type_string(mod->type) );
  
  return mod;
}

/// free the memory allocated for a model
void stg_model_destroy( stg_model_t* mod )
{
  assert( mod );
  
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
  stg_model_global_pose( mod, &gpose );

  double cosa = cos( gpose.a );
  double sina = sin( gpose.a );
  
  gvel->x = mod->velocity.x * cosa - mod->velocity.y * sina;
  gvel->y = mod->velocity.x * sina + mod->velocity.y * cosa;
  gvel->a = mod->velocity.a;

  //printf( "local velocity %.2f %.2f %.2f\nglobal velocity %.2f %.2f %.2f\n",
  //  mod->velocity.x, mod->velocity.y, mod->velocity.a,
  //  gvel->x, gvel->y, gvel->a );
}

/// set the model's velocity in the global frame
void stg_model_set_global_velocity( stg_model_t* mod, stg_velocity_t* gvel )
{
  // TODO - do this properly

  //stg_pose_t gpose;
  
  stg_velocity_t lvel;
  lvel.x = gvel->x;
  lvel.y = gvel->y;
  lvel.a = gvel->a;

  stg_model_set_velocity( mod, &lvel );
}

/// get the model's position in the global frame
void  stg_model_global_pose( stg_model_t* mod, stg_pose_t* gpose )
{ 
  stg_pose_t parent_pose;
  
  // find my parent's pose
  if( mod->parent )
    {
      stg_model_global_pose( mod->parent, &parent_pose );
      
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
  stg_model_global_pose( mod, &origin );
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
  
  if( count == 0 ) 
    return;
  
  if( polys )
    {
      // get model's global pose
      stg_pose_t org;
      memcpy( &org, &mod->geom.pose, sizeof(org));
      stg_model_local_to_global( mod, &org );
      
      //stg_model_global_pose( mod, &org );
      
      stg_matrix_polygons( mod->world->matrix, 
			   org.x, org.y, org.a,
			   polys, count, 
			   mod, render );  
    }
  else
    PRINT_ERR1( "expecting %d polygons but have no data", (int)count );
}


void model_update_cb( gpointer key, gpointer value, gpointer user )
{
  stg_model_update( (stg_model_t*)value );
}

void stg_model_subscribe( stg_model_t* mod )
{
  mod->subs++;
  
  // if this is the first sub, call startup
  if( mod->subs == 1 )
    stg_model_startup(mod);
}

void stg_model_unsubscribe( stg_model_t* mod )
{
  mod->subs--;
  
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


