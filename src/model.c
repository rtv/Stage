

#define DEBUG

#include <assert.h>
#include <math.h>

#include "stage.h"
#include "gui.h"
#include "raytrace.h"

stg_model_t* model_create(  stg_world_t* world, 
			    stg_id_t id, 
			    char* token )
{
  PRINT_DEBUG3( "creating model %d:%d (%s)", world->id, id, token  );
  
  stg_model_t* mod = calloc( sizeof(stg_model_t),1 );

  mod->id = id;
  mod->token = strdup(token);
  mod->world = world;
  
  // initially zero subscriptions
  memset( mod->subs, 0, STG_PROP_COUNT * sizeof(mod->subs[0]) );

  //mod->props = g_hash_table_new( g_int_hash, g_int_equal );
  
  mod->pose.x = 0.0;
  mod->pose.y = 0.0;
  mod->pose.a = 0.0;  
  
  mod->size.x = 0.5;
  mod->size.y = 0.5;
  
  mod->color = stg_lookup_color( "red" );
  
  mod->velocity.x = 0.0;
  mod->velocity.y = 0.0;
  mod->velocity.a = 0.0;  
  
  mod->local_pose.x = 0;
  mod->local_pose.y = 0;
  mod->local_pose.a = 0;

  mod->rects = calloc( sizeof(stg_rotrect_t), 1 );
  mod->rect_count = 1;
  
  mod->boundary = 0;
  
  mod->rects[0].x = 0;
  mod->rects[0].y = 0;
  mod->rects[0].a = 0;
  mod->rects[0].w = 1.0;
  mod->rects[0].h = 1.0;
  
  mod->movemask = STG_MOVE_TRANS | STG_MOVE_ROT;
  mod->nose = FALSE;
  
  mod->parent = NULL;
  
  
  mod->ranger_return = LaserVisible;
  mod->ranger_data = g_array_new( FALSE, TRUE, sizeof(stg_ranger_sample_t));
  mod->ranger_config = g_array_new( FALSE, TRUE, sizeof(stg_ranger_config_t) );

  /*  
  stg_ranger_config_t rng;
  
  int RNGCOUNT = 16;
  int r;
  for( r=0; r<RNGCOUNT; r++ )
    {
      memset( &rng, 0, sizeof(rng) );
      
      rng.fov = M_PI / RNGCOUNT;
      rng.size.x = 0.02;
      rng.size.y = 0.02;
      rng.bounds_range.min = 0.0;
      rng.bounds_range.max = 5.0;
      rng.pose.a = r * 2.0 * M_PI/RNGCOUNT;
      rng.pose.x = mod->size.x/2.0 * cos(rng.pose.a);
      rng.pose.y = mod->size.y/2.0 * sin(rng.pose.a);
      
      g_array_append_val( mod->ranger_config, rng );
    }
  */

  memset( &mod->laser_config, 0, sizeof(mod->laser_config) );

  mod->laser_config.range_min= 0.0;
  mod->laser_config.range_max = 8.0;
  mod->laser_config.samples = 180;
  mod->laser_config.fov = M_PI;
  mod->laser_config.pose.x = 0;
  mod->laser_config.pose.y = 0;
  mod->laser_config.pose.a = 0;
  mod->laser_config.size.x = 0.15;
  mod->laser_config.size.y = 0.15;
  
  mod->laser_data = g_array_new( FALSE, TRUE, sizeof(stg_laser_sample_t) );
				 
  mod->laser_return = LaserVisible;

  gui_model_create( mod );

  return mod;
}

void model_destroy( stg_model_t* mod )
{
  assert( mod );
  
  gui_model_destroy( mod );
  
  if( mod->token ) free( mod->token );
  if( mod->ranger_config ) g_array_free( mod->ranger_config, TRUE );
  if( mod->ranger_data ) g_array_free( mod->ranger_data, TRUE );
  if( mod->laser_data ) g_array_free( mod->laser_data, TRUE );

  free( mod );
}


void model_destroy_cb( gpointer mod )
{
  model_destroy( (stg_model_t*)mod );
}


// sets [result] to the pose of [p2] in [p1]'s coordinate system
void pose_sum( stg_pose_t* result, stg_pose_t* p1, stg_pose_t* p2 )
{
  double cosa = cos(p1->a);
  double sina = sin(p1->a);
  
  double tx = p1->x + p2->x * cosa - p2->y * sina;
  double ty = p1->y + p2->x * sina + p2->y * cosa;
  double ta = p1->a + p2->a;
  
  result->x = tx;
  result->y = ty;
  result->a = ta;
}

void model_global_pose( stg_model_t* mod, stg_pose_t* pose )
{ 
  stg_pose_t glob;
  
  //if( mod->parent )
  //model_global_pose( mod->parent, &glob );
  
  //memcpy( &glob, &mod->pose, sizeof(glob) );
 
  
  //model_local_to_global( mod, &glob );

  //memset( &origin, 0, sizeof(origin) );
    
  //pose_sum( pose, &glob, &mod->pose );

  memcpy( pose, &mod->pose, sizeof(stg_pose_t) ); 
}

// should one day do all this with affine transforms for neatness
void model_local_to_global( stg_model_t* mod, stg_pose_t* pose )
{  
  stg_pose_t origin;  
  //pose_sum( &origin, &mod->pose, &mod->origin );
  model_global_pose( mod, &origin );
  
  pose_sum( &origin, &origin, &mod->local_pose );
  pose_sum( pose, &origin, pose );
}

void model_global_rect( stg_model_t* mod, stg_rotrect_t* glob, stg_rotrect_t* loc )
{
  // scale first
  glob->x = ((loc->x + loc->w/2.0) * mod->size.x) - mod->size.x/2.0;
  glob->y = ((loc->y + loc->h/2.0) * mod->size.y) - mod->size.y/2.0;
  glob->w = loc->w * mod->size.x;
  glob->h = loc->h * mod->size.y;
  glob->a = loc->a;
  
  // now transform into local coords
  model_local_to_global( mod, (stg_pose_t*)glob );
}


void model_map( stg_model_t* mod, gboolean render )
{
  int r;
  for( r=0; r<mod->rect_count; r++ )
    {
      stg_rotrect_t* rr = &mod->rects[r];
      
      // convert the rectangle into global coordinates;
      stg_rotrect_t grr;
      model_global_rect( mod, &grr, rr );
     
      stg_matrix_rectangle( mod->world->matrix, 
			    grr.x, grr.y, grr.a, grr.w, grr.h,
			    mod, render );
    }      

  // optionally, add a bounding rectangle
  if( mod->boundary )
    {
      stg_pose_t pz;
      memcpy( &pz, &mod->pose, sizeof(pz) );
      model_local_to_global( mod, &pz );

      stg_matrix_rectangle( mod->world->matrix,
			    pz.x, pz.y, pz.a,
			    mod->size.x, mod->size.y, mod, render );
    }
}
  
  /* UPDATE */
  
void model_update_velocity( stg_model_t* model )
{  

}


/* UPDATE everything */

void model_update( stg_model_t* model )
{
  //PRINT_DEBUG2( "updating model %d:%s", model->id, model->token );
  
  // some things must always be calculated
  model_update_velocity( model );
  model_update_pose( model );
}

void model_update_cb( gpointer key, gpointer value, gpointer user )
{
  model_update( (stg_model_t*)value );
}

int model_update_prop( stg_model_t* mod, stg_id_t propid )
{
  switch( propid )
    {
    case STG_PROP_RANGERDATA:
      model_update_rangers( mod );
      gui_model_rangers_data( mod );
      break;

    case STG_PROP_LASERDATA:
      model_update_laser( mod );
      gui_model_laser_data( mod );
      break;

    default:
      //PRINT_DEBUG1( "no update function for property type %s", 
      //	    stg_property_string(propid) );
      break;
    }
  
  
  mod->update_times[ propid ] = mod->world->sim_time;
  
  return 0; // ok
}

int model_get_prop( stg_model_t* mod, stg_id_t pid, void** data, size_t* len )
{
  //double time_since_calc =
  //mod->world->sim_time -  mod->update_times[pid];
  
  switch( pid )
    {
    case STG_PROP_TIME: // no data - just fetches a timestamp
      *data = NULL;
      *len = 0;
      break;
    case STG_PROP_POSE:
      *data = &mod->pose;
      *len = sizeof(mod->pose);
      break;
    case STG_PROP_SIZE:
      *data = &mod->size;
      *len = sizeof(mod->size);
      break;
    case STG_PROP_VELOCITY:
      *data = &mod->velocity;
      *len = sizeof(mod->velocity);
      break;
    case STG_PROP_ORIGIN:
      *data = &mod->local_pose;
      *len = sizeof(mod->local_pose);
      break;
    case STG_PROP_COLOR:
      *data = &mod->color;
      *len = sizeof(mod->color);
      break;
    case STG_PROP_RECTS:
      *data = mod->rects;
      *len = mod->rect_count * sizeof(stg_rotrect_t);
      break;
    case STG_PROP_NOSE:
      *data = &mod->nose;
      *len = sizeof(mod->nose);
      break;
    case STG_PROP_MOVEMASK:
      *data = &mod->movemask;
      *len = sizeof(mod->movemask);
      break;
    case STG_PROP_RANGERCONFIG:
      *data = mod->ranger_config->data;
      *len = mod->ranger_config->len * sizeof(stg_ranger_config_t);
      break;
    case STG_PROP_RANGERDATA:
      *data = mod->ranger_data->data;
      *len = mod->ranger_data->len * sizeof(stg_ranger_sample_t);
      break;
    case STG_PROP_LASERDATA:
      *data = mod->laser_data->data;
      *len = mod->laser_data->len * sizeof(stg_laser_sample_t);
      break;
    case STG_PROP_LASERCONFIG:
      *data = &mod->laser_config;
      *len = sizeof(stg_laser_config_t);
      break;
     
    default:
      // todo - add random props to a model
      // g_hash_table_lookup( model->props, &pid );
      
      PRINT_WARN2( "request for unknown property %d(%s)",
		   pid, stg_property_string(pid) );
      *data = NULL;
      *len = 0;
      
      return 1; //error
      break;
    }

  return 0; //ok
}

int model_set_prop( stg_model_t* mod, 
		     stg_id_t propid, 
		     void* data, 
		     size_t len )
{
  switch( propid )
    {
    case STG_PROP_TIME:
      PRINT_WARN3( "Ignoring attempt to set time for %d:%d(%s).", 
		   mod->world->id, mod->id, mod->token );
      break;
      
    case STG_PROP_POSE:
      if( len == sizeof(mod->pose) )
	{
	  model_map( mod, 0 );
	  memcpy( &mod->pose, data, len );
	  model_map( mod, 1 );
	}
      else PRINT_WARN2( "ignoring bad pose data (%d/%d bytes)", 
		       (int)len, (int)sizeof(mod->pose) );
      break;
      
    case STG_PROP_ORIGIN:
      if( len == sizeof(mod->local_pose) )
	{
	  model_map( mod, 0 );
	  memcpy( &mod->local_pose, data, len );
	  model_map( mod, 1 );
	}
      else PRINT_WARN2( "ignoring bad origin data (%d/%d bytes)", 
		       (int)len, (int)sizeof(mod->local_pose) );
      break;
      
    case STG_PROP_VELOCITY:
      if( len == sizeof(mod->velocity) )
	memcpy( &mod->velocity, data, len );
      else PRINT_WARN2( "ignoring bad velocity data (%d/%d bytes)", 
		       (int)len, (int)sizeof(mod->velocity) );
      break;
      
    case STG_PROP_SIZE:
      if( len == sizeof(mod->size) )
	{
	  model_map( mod, 0 );
	  memcpy( &mod->size, data, len );
	  model_map( mod, 1 );
	}
      else PRINT_WARN2( "ignoring bad size data (%d/%d bytes)", 
		       (int)len, (int)sizeof(mod->size) );
      break;
      
    case STG_PROP_COLOR:
      if( len == sizeof(mod->color) )
	memcpy( &mod->color, data, len );
      else PRINT_WARN2( "ignoring bad color data (%d/%d bytes)", 
		       (int)len, (int)sizeof(mod->color) );

      PRINT_WARN1( "color is now %X", mod->color );
      PRINT_WARN1( "data was %X", data );
      PRINT_WARN1( "data was %X", *(stg_color_t*)data );
      break;
      
    case STG_PROP_MOVEMASK:
      if( len == sizeof(mod->movemask) )
	memcpy( &mod->movemask, data, len );
      else PRINT_WARN2( "ignoring bad movemask data (%d/%d bytes)", 
		       (int)len, (int)sizeof(mod->movemask) );
      break;
      
    case STG_PROP_RECTS:
      if( len > 0 )
	{
	  model_map( mod, 0 );
	  mod->rects = realloc( mod->rects, len );
	  memcpy( mod->rects, data, len );
	  mod->rect_count = len / sizeof(stg_rotrect_t);
	  stg_normalize_rects( mod->rects, mod->rect_count );
	  model_map( mod, 1 );
	}
      break;

    case STG_PROP_PARENT:
      if( len == sizeof(stg_id_t) )
	{
	  stg_id_t parent_id = *(stg_id_t*)data;
	  PRINT_DEBUG2( "model %d parent %d", mod->id, parent_id );
	  mod->parent = world_get_model( mod->world, parent_id );
	  PRINT_DEBUG2( "model %p parent %p", mod, mod->parent );
	}
      else PRINT_WARN2( "ignoring bad parent data (%d/%d bytes)", 
			(int)len, (int)sizeof(stg_id_t) );
      break;
      
    case STG_PROP_BOUNDARY:
      if( len == sizeof(stg_bool_t) )
	mod->boundary = *(stg_bool_t*)data;
      else PRINT_WARN2( "ignoring bad boundary data (%d/%d bytes)", 
			(int)len, (int)sizeof(stg_bool_t) );
      break;
      
    case STG_PROP_NOSE:
      if( len == sizeof(stg_bool_t) )
	mod->nose = *(stg_bool_t*)data;
      else PRINT_WARN2( "ignoring bad nose data (%d/%d bytes)", 
			(int)len, (int)sizeof(stg_bool_t) );
      break;
      
    case STG_PROP_LASERCONFIG:
      if( len == sizeof(stg_laser_config_t) )
	memcpy( &mod->laser_config, data, sizeof(stg_laser_config_t) );
      else PRINT_WARN2( "ignoring bad laser config data (%d/%d bytes)", 
			(int)len, (int)sizeof(stg_laser_config_t) );
      break;
      
    case STG_PROP_RANGERCONFIG:
      g_array_set_size( mod->ranger_config, 0 );
      g_array_append_vals( mod->ranger_config, data, len/sizeof(stg_ranger_config_t) );

      PRINT_WARN2( "configured %d (%d) rangers", 
		   len/sizeof(stg_ranger_config_t),
		   mod->ranger_config->len );
      break;

    default:
      // TODO - accept random prop types and stash data in hash table
      PRINT_WARN2( "ignoring unknown property type %d(%s)",
		   propid, stg_property_string(propid) );

      return 1; //error
      break;
    }
  
  gui_model_update( mod, propid ); 

  return 0; //ok
}

void model_handle_msg( stg_model_t* model, int fd, stg_msg_t* msg )
{
  assert( model );
  assert( msg );

  switch( msg->type )
    {
    case STG_MSG_MODEL_PROPERTY:
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

void model_print( stg_model_t* mod )
{
  printf( "   model %d:%d:%s\n", mod->world->id, mod->id, mod->token );
}

void model_print_cb( gpointer key, gpointer value, gpointer user )
{
  model_print( (stg_model_t*)value );
}


