#include <assert.h>
#include <math.h>

#define DEBUG
//#undef DEBUG

#include "stage.h"
#include "gui.h"
#include "raytrace.h"

extern lib_entry_t derived[];


int model_create_name( model_t* mod )
{
  assert( mod );
  assert( mod->token );
  
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

model_t* model_create(  world_t* world, 
			model_t* parent,
			stg_id_t id, 
			stg_model_type_t type,
			char* token )
{  
  model_t* mod = calloc( sizeof(model_t),1 );

  mod->id = id;

  mod->world = world;
  mod->parent = parent; 
  mod->type = type;
  mod->token = strdup(token); // this will be immediately replaced by
			      // model_create_name  
  // create a default name for the model that's derived from its
  // ancestors' names and its worldfile token
  //model_create_name( mod );

  PRINT_WARN3( "creating model %d:%d(%s)", 
	       world->id, mod->id, mod->token );
  
  if( parent) 
    { 
      // add this model to it's parent's list of children
      g_ptr_array_add( parent->children, mod );       
      // count this type of model in its parent
      //parent->child_type_count[ type ]++;
    }
  //else
  //world->child_type_count[ type ]++;

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

  mod->guifeatures.boundary =  STG_DEFAULT_BOUNDARY;
  mod->guifeatures.nose =  STG_DEFAULT_NOSE;
  mod->guifeatures.grid = STG_DEFAULT_GRID;
  
  // by default, only top-level objects are draggable
  if( mod->parent )
    mod->guifeatures.movemask = 0;
  else
    mod->guifeatures.movemask = STG_DEFAULT_MOVEMASK;

  // zero velocity
  memset( &mod->velocity, 0, sizeof(mod->velocity) );
  
  // define a unit rectangle from 4 lines
  stg_line_t alines[4];
  alines[0].x1 = 0; 
  alines[0].y1 = 0;
  alines[0].x2 = 1; 
  alines[0].y2 = 0;
  
  alines[1].x1 = 1; 
  alines[1].y1 = 0;
  alines[1].x2 = 1; 
  alines[1].y2 = 1;
  
  alines[2].x1 = 1; 
  alines[2].y1 = 1;
  alines[2].x2 = 0; 
  alines[2].y2 = 1;
  
  alines[3].x1 = 0; 
  alines[3].y1 = 1;
  alines[3].x2 = 0; 
  alines[3].y2 = 0;
  
  // fit the rectangle inside the model's size
  stg_normalize_lines( alines, 4 );
  stg_scale_lines( alines, 4, mod->geom.size.x, mod->geom.size.y );
  stg_translate_lines( alines, 4, -mod->geom.size.x/2.0, -mod->geom.size.y/2.0 );  
  
  mod->lines_count = 4;
  size_t lines_len = mod->lines_count * sizeof(stg_line_t);
  assert( mod->lines = malloc( lines_len ) );
  memcpy( mod->lines, alines, lines_len );
  
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

  gui_model_create( mod );

  // if this type of model has an init function, call it.
  if( derived[ mod->type ].init )
    derived[ mod->type ].init(mod);

  PRINT_DEBUG2( "created model %d type %d.", mod->id, mod->type );
  
  return mod;
}

void model_destroy( model_t* mod )
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
  model_destroy( (model_t*)mod );
}

int model_is_antecedent( model_t* mod, model_t* testmod )
{
  if( mod == NULL )
    return FALSE;

  if( mod == testmod )
    return TRUE;
  
  if( model_is_antecedent( mod->parent, testmod ))
    return TRUE;
  
  // neither mod nor a child of mod matches testmod
  return FALSE;
}

int model_is_descendent( model_t* mod, model_t* testmod )
{
  if( mod == testmod )
    return TRUE;
  
  int ch;
  for(ch=0; ch < mod->children->len; ch++ )
    {
      model_t* child = g_ptr_array_index( mod->children, ch );
      if( model_is_descendent( child, testmod ))
	return TRUE;
    }
  
  // neither mod nor a child of mod matches testmod
  return FALSE;
}

rtk_fig_t* model_prop_fig_create( model_t* mod, 
				  rtk_fig_t* array[],
				  stg_id_t propid, 
				  rtk_fig_t* parent,
				  int layer )
{
  rtk_fig_t* fig =  rtk_fig_create( mod->world->win->canvas, 
				    parent, 
				    layer ); 
  array[propid] = fig;
  
  return fig;
}


void model_global_pose( model_t* mod, stg_pose_t* gpose )
{ 
  // start from zero
  //memset( pose, 0, sizeof(stg_pose_t));
  
  stg_pose_t parent_pose;
  memset( &parent_pose, 0, sizeof(parent_pose));
  
  // find my parent's pose
  if( mod->parent )
    model_global_pose( mod->parent, &parent_pose );

  // Compute our pose in the global cs
  gpose->x = parent_pose.x + mod->pose.x * cos(parent_pose.a) - mod->pose.y * sin(parent_pose.a);
  gpose->y = parent_pose.y + mod->pose.x * sin(parent_pose.a) + mod->pose.y * cos(parent_pose.a);
  gpose->a = NORMALIZE(parent_pose.a + mod->pose.a);
}

// should one day do all this with affine transforms for neatness
void model_local_to_global( model_t* mod, stg_pose_t* pose )
{  
  stg_pose_t origin;   
  //stg_pose_sum( &origin, &mod->pose, &mod->origin );
  model_global_pose( mod, &origin );
  
  stg_geom_t* geom = model_get_geom(mod); 

  stg_pose_sum( &origin, &origin, &geom->pose );
  stg_pose_sum( pose, &origin, pose );
}

void model_global_rect( model_t* mod, stg_rotrect_t* glob, stg_rotrect_t* loc )
{
  stg_geom_t* geom = model_get_geom(mod); 

  double w = geom->size.x;
  double h = geom->size.y;

  // scale first
  glob->pose.x = ((loc->pose.x + loc->size.x/2.0) * w) - w/2.0;
  glob->pose.y = ((loc->pose.y + loc->size.y/2.0) * h) - h/2.0;
  glob->pose.a = loc->pose.a;
  glob->size.x = loc->size.x * w;
  glob->size.y = loc->size.y * h;
  
  // now transform into local coords
  model_local_to_global( mod, (stg_pose_t*)glob );
}


void model_map_with_children(  model_t* mod, gboolean render )
{
  // call this function for all the model's children
  int ch;
  for( ch=0; ch<mod->children->len; ch++ )
    model_map_with_children( (model_t*)g_ptr_array_index(mod->children, ch), 
			     render);  
  // now map the model
  model_map( mod, render );
}

// if render is true, render the model into the matrix, else unrender
// the model
void model_map( model_t* mod, gboolean render )
{
  assert( mod );
  
  size_t count=0;
  stg_line_t* lines = model_get_lines(mod, &count);
  
  if( count == 0 ) 
    return;
  
  if( lines == NULL )
    {
      PRINT_ERR1( "expecting %d lines but have no data", count );
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
      
      model_local_to_global( mod, &p1 );
      model_local_to_global( mod, &p2 );
      
      stg_matrix_line( mod->world->matrix, p1.x, p1.y, p2.x, p2.y, mod, render );
    }
}
  
int model_update( model_t* mod )
{
  //PRINT_DEBUG2( "updating model %d:%s", mod->id, mod->token );
  
  mod->interval_elapsed = 0;

  // if this type of model has an update function, call it.
  if( derived[ mod->type ].update && mod->subs[STG_PROP_DATA] > 0 )
    derived[ mod->type ].update(mod);

  // now move the model if it has any velocity
  model_update_pose( mod );

  return 0;
}

void model_update_cb( gpointer key, gpointer value, gpointer user )
{
  model_update( (model_t*)value );
}

int model_service( model_t* mod )
{
  PRINT_DEBUG1( "default service method mod %d", mod->id );

  // if this type of model has an service function, call it.
  if( derived[ mod->type ].service )
    derived[ mod->type ].service(mod);

  return 0;
}

int model_startup( model_t* mod )
{
  PRINT_DEBUG1( "default startup method mod %d", mod->id );
  
  // if this type of model has a startup function, call it.
  if( derived[ mod->type ].startup )
   return  derived[ mod->type ].startup(mod);
  
  return 0;
}

int model_shutdown( model_t* mod )
{
  PRINT_DEBUG1( "default shutdown method mod %d", mod->id );
  
  // if this type of model has a shutdown function, call it.
  if( derived[ mod->type ].shutdown )
    return  derived[ mod->type ].shutdown(mod);
  
  return 0;
}

void model_subscribe( model_t* mod, stg_id_t pid )
{
  mod->subs[pid]++;
  
  // if this is the first sub, call startup & render if there is one
  if( mod->subs[pid] == 1 )
    model_startup(mod);
}

void model_unsubscribe( model_t* mod, stg_id_t pid )
{
  mod->subs[pid]--;
  
  // if that was the last sub, call shutdown 
  if( mod->subs[pid] < 1 )
    model_shutdown(mod);
  
  if( mod->subs[pid] < 0 )
    PRINT_ERR1( "subscription count has gone below zero (%d). weird.",
		mod->subs[pid] );
}

void model_handle_msg( model_t* model, int fd, stg_msg_t* msg )
{
  assert( model );
  assert( msg );

  switch( msg->type )
    {
    case STG_MSG_MODEL_DELTA:
      // deltas don't need a reply
      {
	stg_prop_t* mp = (stg_prop_t*)msg->payload;
	
	PRINT_DEBUG5( "set property %d:%d:%d(%s) with %d bytes",
		      mp->world,
		      mp->model,
		      mp->prop,
		      stg_property_string( mp->prop ),
		      (int)mp->datalen );
	
	model_set_prop( model, mp->prop, mp->data, mp->datalen ); 
      }
      break;

      // everything else needs a reply
    case STG_MSG_MODEL_PROPGET:
      {
	PRINT_DEBUG( "RECEIVED A PROPGET REQUEST" );
	
	stg_target_t* tgt = (stg_target_t*)msg->payload;
	
	// reply with the requested property
	void* data = NULL;
	size_t len = 0;	
	model_get_prop( model, tgt->prop, &data, &len );	
	stg_fd_msg_write( fd, STG_MSG_CLIENT_REPLY, data, len );
      }
      break;

      // TODO
    case STG_MSG_MODEL_PROPSET:
      PRINT_WARN( "STG_MSG_MODEL_PROPSET" );
      {
	stg_prop_t* mp = (stg_prop_t*)msg->payload;
	
	PRINT_WARN5( "set property %d:%d:%d(%s) with %d bytes",
		     mp->world,
		     mp->model,
		     mp->prop,
		     stg_property_string( mp->prop ),
		     (int)mp->datalen );
	
	model_set_prop( model, mp->prop, mp->data, mp->datalen ); 	
	
	PRINT_DEBUG( "SENDING A REPLY" );	 	   
	int ack = STG_ACK;
	stg_fd_msg_write( fd, STG_MSG_CLIENT_REPLY, &ack, sizeof(ack) );
      }
      break;
      
    case STG_MSG_MODEL_PROPGETSET:
      PRINT_WARN( "STG_MSG_MODEL_PROPGETSET not yet implemented" );
      break;
      
    case STG_MSG_MODEL_PROPSETGET:
      PRINT_WARN( "STG_MSG_MODEL_PROPSETGET not yet implemented" );
      break;
      
    default:
      PRINT_WARN1( "Ignoring unrecognized model message type %d.",
		   msg->type & STG_MSG_MASK_MINOR );
      break;
    }
}
  
void pose_invert( stg_pose_t* pose )
{
  pose->x = -pose->x;
  pose->y = -pose->y;
  pose->a = pose->a;
}

void model_print( model_t* mod )
{
  printf( "   model %d:%d:%s\n", mod->world->id, mod->id, mod->token );
}

void model_print_cb( gpointer key, gpointer value, gpointer user )
{
  model_print( (model_t*)value );
}

