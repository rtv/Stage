/**
@defgroup stg_model 

Implements a model - the basic object of Stage simulation

*/

#include <assert.h>
#include <math.h>

//#define DEBUG
//#undef DEBUG

#include "stage.h"
//#include "gui.h"
//#include "raytrace.h"

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

stg_model_t* stg_model_create( stg_world_t* world, 
			       stg_model_t* parent,
			       stg_id_t id, 
			       stg_model_type_t type,
			       stg_lib_entry_t* lib,       
			       char* token )
{  
  stg_model_t* mod = calloc( sizeof(stg_model_t),1 );

  mod->id = id;

  mod->world = world;
  mod->parent = parent; 
  mod->type = type;
  mod->lib = lib;
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

  mod->polypoints = NULL;

  // define a unit rectangle from 4 lines
  stg_line_t lines[4];
  lines[0].x1 = 0; 
  lines[0].y1 = 0;
  lines[0].x2 = 1; 
  lines[0].y2 = 0;
  
  lines[1].x1 = 1; 
  lines[1].y1 = 0;
  lines[1].x2 = 1; 
  lines[1].y2 = 1;
  
  lines[2].x1 = 1; 
  lines[2].y1 = 1;
  lines[2].x2 = 0; 
  lines[2].y2 = 1;
  
  lines[3].x1 = 0; 
  lines[3].y1 = 1;
  lines[3].x2 = 0; 
  lines[3].y2 = 0;
  
  // this normalizes the lines to fit inside our geometry rectangle
  // and also causes a redraw
  stg_model_set_lines( mod, lines, 4 );
		
  // initialize odometry
  memset( &mod->odom, 0, sizeof(mod->odom));
  
  mod->friction = 0.0;

  // if model has an init function, call it.
  if( mod->lib->init )
    mod->lib->init(mod);
  
  PRINT_DEBUG4( "finished model %d.%d(%s) type %s", 
		mod->world->id, mod->id, 
		mod->token, stg_model_type_string(mod->type) );
  
  return mod;
}

void stg_model_destroy( stg_model_t* mod )
{
  assert( mod );
  
  gui_model_destroy( mod );

  if( mod->parent && mod->parent->children ) g_ptr_array_remove( mod->parent->children, mod );
  if( mod->children ) g_ptr_array_free( mod->children, FALSE );
  if( mod->lines ) free( mod->lines );
  if( mod->data ) free( mod->data );
  if( mod->cmd ) free( mod->cmd );
  if( mod->cfg ) free( mod->cfg );
  if( mod->token ) free( mod->token );

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

// returns 1 if mod1 and mod2 are in the same tree
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

void  stg_model_global_pose( stg_model_t* mod, stg_pose_t* gpose )
{ 
  stg_pose_t parent_pose;
  memset( &parent_pose, 0, sizeof(parent_pose));
  
  // find my parent's pose
  if( mod->parent )
    stg_model_global_pose( mod->parent, &parent_pose );

  // Compute our pose in the global cs
  gpose->x = parent_pose.x + mod->pose.x * cos(parent_pose.a) - mod->pose.y * sin(parent_pose.a);
  gpose->y = parent_pose.y + mod->pose.x * sin(parent_pose.a) + mod->pose.y * cos(parent_pose.a);
  gpose->a = NORMALIZE(parent_pose.a + mod->pose.a);
}

// should one day do all this with affine transforms for neatness
void stg_model_local_to_global( stg_model_t* mod, stg_pose_t* pose )
{  
  stg_pose_t origin;   
  //stg_pose_sum( &origin, &mod->pose, &mod->origin );
  stg_model_global_pose( mod, &origin );
  
  stg_geom_t* geom = stg_model_get_geom(mod); 

  stg_pose_sum( &origin, &origin, &geom->pose );
  stg_pose_sum( pose, &origin, pose );
}

void stg_model_global_rect( stg_model_t* mod, stg_rotrect_t* glob, stg_rotrect_t* loc )
{
  stg_geom_t* geom = stg_model_get_geom(mod); 

  double w = geom->size.x;
  double h = geom->size.y;

  // scale first
  glob->pose.x = ((loc->pose.x + loc->size.x/2.0) * w) - w/2.0;
  glob->pose.y = ((loc->pose.y + loc->size.y/2.0) * h) - h/2.0;
  glob->pose.a = loc->pose.a;
  glob->size.x = loc->size.x * w;
  glob->size.y = loc->size.y * h;
  
  // now transform into local coords
  stg_model_local_to_global( mod, (stg_pose_t*)glob );
}


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
  stg_line_t* lines = stg_model_get_lines(mod, &count);
  
  if( count == 0 ) 
    return;
  
  if( lines == NULL )
    {
      PRINT_ERR1( "expecting %d lines but have no data", (int)count );
      return;
    }
  
  // todo - speed this up by processing all points in a single function call
  int l;
  for( l=0; l<count; l++ )
    {
      stg_line_t* line = &lines[l];

      stg_pose_t p1, p2;
      p1.x = line->x1;
      p1.y = line->y1;
      p1.a = 0;

      p2.x = line->x2;
      p2.y = line->y2;
      p2.a = 0;
      
      stg_model_local_to_global( mod, &p1 );
      stg_model_local_to_global( mod, &p2 );
      
      stg_matrix_line( mod->world->matrix, p1.x, p1.y, p2.x, p2.y, mod, render );
    }
}
  
int stg_model_update( stg_model_t* mod )
{
  //PRINT_DEBUG2( "updating model %d:%s", mod->id, mod->token );
  
  mod->interval_elapsed = 0;

  // if this type of model has an update function, call it.
  if( mod->lib->update )
    mod->lib->update(mod);
  
  // now move the model if it has any velocity
  stg_model_update_pose( mod );

  return 0;
}

void model_update_cb( gpointer key, gpointer value, gpointer user )
{
  stg_model_update( (stg_model_t*)value );
}

int stg_model_startup( stg_model_t* mod )
{
  PRINT_DEBUG1( "default startup method mod %d", mod->id );
  
  // if this type of model has a startup function, call it.
  if(mod->lib->startup )
   return mod->lib->startup(mod);
  
  return 0;
}

int stg_model_shutdown( stg_model_t* mod )
{
  PRINT_DEBUG1( "default shutdown method mod %d", mod->id );
  
  // if this type of model has a shutdown function, call it.
  if(mod->lib->shutdown )
    return mod->lib->shutdown(mod);
  
  return 0;
}

void stg_model_subscribe( stg_model_t* mod )
{
  mod->subs++;
  
  // if this is the first sub, call startup & render if there is one
  if( mod->subs == 1 )
    stg_model_startup(mod);
}

void stg_model_unsubscribe( stg_model_t* mod )
{
  mod->subs--;
  
  // if this is the first sub, call startup & render if there is one
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


stg_lib_entry_t model_entry = { 
  NULL,  // init
  NULL,  // startup
  NULL,  // shutdown
  NULL,  // update
  NULL,  // set data
  NULL,  // get data
  NULL,  // set command
  NULL,  // get command
  NULL,  // set config
  NULL   // get config
};
