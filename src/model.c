

//#define DEBUG

#include <assert.h>
#include <math.h>

#include "stage.h"
#include "gui.h"
#include "raytrace.h"

model_t* model_create(  world_t* world, 
			    stg_id_t id, 
			    char* token )
{
  PRINT_DEBUG3( "creating model %d:%d (%s)", world->id, id, token  );

  // calloc zeros the whole structure
  model_t* mod = calloc( sizeof(model_t),1 );

  mod->id = id;
  mod->token = strdup(token);
  mod->world = world;
  
  //mod->props = g_hash_table_new( g_int_hash, g_int_equal );
  
  mod->size.x = 0.4;
  mod->size.y = 0.4;
  
  mod->color = stg_lookup_color( "red" );
  
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
  
  // fit the rectangle inside the model's size
  stg_normalize_lines( lines, 4 );
  stg_scale_lines( lines, 4, mod->size.x, mod->size.y );
  stg_translate_lines( lines, 4, -mod->size.x/2.0, -mod->size.y/2.0 );
  
  // stash the lines
  mod->lines = g_array_new( FALSE, TRUE, sizeof(stg_line_t) );  
  g_array_append_vals(  mod->lines, lines, 4 );
  
  mod->movemask = STG_MOVE_TRANS | STG_MOVE_ROT;
  
  mod->ranger_return = LaserVisible;
  mod->ranger_data = g_array_new( FALSE, TRUE, sizeof(stg_ranger_sample_t));
  mod->ranger_config = g_array_new( FALSE, TRUE, sizeof(stg_ranger_config_t) );

  // configure the laser to sensible defaults
  mod->laser_geom.pose.x = 0;
  mod->laser_geom.pose.y = 0;
  mod->laser_geom.pose.a = 0;
  mod->laser_geom.size.x = 0.0; // invisibly small (so it's not rendered) by default
  mod->laser_geom.size.y = 0.0;
  
  mod->laser_config.range_min= 0.0;
  mod->laser_config.range_max = 8.0;
  mod->laser_config.samples = 180;
  mod->laser_config.fov = M_PI;
  
  mod->laser_return = LaserVisible;

  gui_model_create( mod );

  return mod;
}

void model_destroy( model_t* mod )
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
  model_destroy( (model_t*)mod );
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

void model_global_pose( model_t* mod, stg_pose_t* pose )
{ 
  //stg_pose_t glob;
  
  //if( mod->parent )
  //model_global_pose( mod->parent, &glob );
  
  //memcpy( &glob, &mod->pose, sizeof(glob) );
 
  
  //model_local_to_global( mod, &glob );

  //memset( &origin, 0, sizeof(origin) );
    
  //pose_sum( pose, &glob, &mod->pose );

  memcpy( pose, &mod->pose, sizeof(stg_pose_t) ); 
}

// should one day do all this with affine transforms for neatness
void model_local_to_global( model_t* mod, stg_pose_t* pose )
{  
  stg_pose_t origin;  
  //pose_sum( &origin, &mod->pose, &mod->origin );
  model_global_pose( mod, &origin );
  
  pose_sum( &origin, &origin, &mod->local_pose );
  pose_sum( pose, &origin, pose );
}

void model_global_rect( model_t* mod, stg_rotrect_t* glob, stg_rotrect_t* loc )
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


void model_map( model_t* mod, gboolean render )
{
  // todo - speed this up by transforming all points in a single function call
  int l;
  for( l=0; l<mod->lines->len; l++ )
    {
      stg_line_t* line = &g_array_index( mod->lines, stg_line_t, l );

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
  

/* UPDATE */
  
void model_update_velocity( model_t* model )
{  

}


/* UPDATE everything */

void model_update( model_t* model )
{
  //PRINT_DEBUG2( "updating model %d:%s", model->id, model->token );
  
  // some things must always be calculated
  model_update_velocity( model );
  model_update_pose( model );
}

void model_update_cb( gpointer key, gpointer value, gpointer user )
{
  model_update( (model_t*)value );
}

int model_update_prop( model_t* mod, stg_id_t propid )
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


void model_subscribe( model_t* mod, stg_id_t pid )
{
  mod->subs[pid]++;
  gui_model_render( mod );
}

void model_unsubscribe( model_t* mod, stg_id_t pid )
{
  mod->subs[pid]--;
  gui_model_render( mod );
}

int model_get_prop( model_t* mod, stg_id_t pid, void** data, size_t* len )
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

      /*    case STG_PROP_GEOM:
      mod->geom.x = mod->pose.x;
      mod->geom.y = mod->pose.y;
      mod->geom.a = mod->pose.a;
      mod->geom.w = mod->size.x;
      mod->geom.h = mod->size.y;
      *data = &mod->geom;
      *len = sizeof(mod->geom);
      break	
      */

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
    case STG_PROP_NOSE:
      *data = &mod->nose;
      *len = sizeof(mod->nose);
      break;
    case STG_PROP_GRID:
      *data = &mod->grid;
      *len = sizeof(mod->grid);
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
      {
	// laser data packet starts with its config
	*len = sizeof(stg_laser_config_t);
	*data = calloc( *len, 1 );
	memcpy( *data, &mod->laser_config,*len);

	// and continues with the data
	if( mod->laser_data )
	  {
	    size_t ranges_len = mod->laser_data->len * sizeof(stg_laser_sample_t);
	    *len += ranges_len;
	    *data = realloc( *data, *len );
	    memcpy( *data + sizeof(stg_laser_config_t), mod->laser_data->data, ranges_len );
	  }
      }
      break;

    case STG_PROP_LASERCONFIG:
      *data = &mod->laser_config;
      *len = sizeof(stg_laser_config_t);
      break;
      
    case STG_PROP_LASERGEOM:
      *data = &mod->laser_geom;
      *len = sizeof(stg_geom_t);
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

int model_set_prop( model_t* mod, 
		     stg_id_t propid, 
		     void* data, 
		     size_t len )
{
  PRINT_DEBUG4( "setting property %d:%d:%d(%s)",
		mod->world->id, mod->id, propid, stg_property_string(propid) );

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
	  
	  // force the body lines to fit inside this new rectangle
	  stg_normalize_lines( (stg_line_t*)mod->lines->data, mod->lines->len );
	  stg_scale_lines( (stg_line_t*)mod->lines->data, mod->lines->len, mod->size.x, mod->size.y );
	  stg_translate_lines( (stg_line_t*)mod->lines->data, mod->lines->len,
			       -mod->size.x/2.0, -mod->size.y/2.0 );
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
      break;
      
    case STG_PROP_MOVEMASK:
      if( len == sizeof(mod->movemask) )
	memcpy( &mod->movemask, data, len );
      else PRINT_WARN2( "ignoring bad movemask data (%d/%d bytes)", 
		       (int)len, (int)sizeof(mod->movemask) );
      break;
      
    case STG_PROP_LINES:
      if( len > 0 )
	{
	  model_map( mod, 0 );
	  g_array_set_size( mod->lines, 0 );
	  g_array_append_vals( mod->lines, data, len / sizeof(stg_line_t) );
	  // todo - normalize the lines into the model's size rectangle
	  // todo - if boundary is set we add a unit rectangle
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

    case STG_PROP_GRID:
      if( len == sizeof(stg_bool_t) )
	mod->grid = *(stg_bool_t*)data;
      else PRINT_WARN2( "ignoring bad grid data (%d/%d bytes)", 
			(int)len, (int)sizeof(stg_bool_t) );
      break;
      
    case STG_PROP_LASERCONFIG:
      if( len == sizeof(stg_laser_config_t) )
	memcpy( &mod->laser_config, data, sizeof(stg_laser_config_t) );
      else PRINT_WARN2( "ignoring bad laser config data (%d/%d bytes)", 
			(int)len, (int)sizeof(stg_laser_config_t) );
      break;
      
    case STG_PROP_LASERGEOM:
      if( len == sizeof(stg_geom_t) )
	memcpy( &mod->laser_geom, data, sizeof(stg_geom_t) );
      else PRINT_WARN2( "ignoring bad laser geometry data (%d/%d bytes)", 
			(int)len, (int)sizeof(stg_geom_t) );
      break;
      
    case STG_PROP_RANGERCONFIG:
      g_array_set_size( mod->ranger_config, 0 );
      g_array_append_vals( mod->ranger_config, data, len/sizeof(stg_ranger_config_t) );
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

void model_handle_msg( model_t* model, int fd, stg_msg_t* msg )
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

	stg_msg_t* reply = NULL;
	switch( msg->response )
	  {
	  case STG_RESPONSE_NONE:
	    reply = NULL;
	    break;
	    
	  case STG_RESPONSE_ACK:
	    reply = stg_msg_create( STG_MSG_MODEL_ACK, 
				    STG_RESPONSE_NONE,
				    NULL, 0 );
	    break;
	    
	  case STG_RESPONSE_REPLY:
	    {
	      void* reply_data = NULL;
	      size_t reply_data_len = 0;
	      
	      if( model_get_prop( model, mp->prop, &reply_data, &reply_data_len ) )      
		PRINT_WARN2( "failed to find a reply for property %d(%s)",
			     mp->prop, stg_property_string(mp->prop) );
	      else
		{
		  stg_print_laser_config( reply_data );

		  reply = stg_msg_create( STG_MSG_MODEL_REPLY, 
					  STG_RESPONSE_NONE,
					  reply_data, reply_data_len );
		  free( reply_data );
		}
	    }
	    break;
	    
	  default:
	    PRINT_WARN1( "ignoring unrecognized response type %d", msg->response );
	    reply = NULL;
	    break;
	  }

	if( reply )
	  {
	    stg_fd_msg_write( fd, reply );
	    free( reply );
	  }

      }
      break;

    case STG_MSG_MODEL_REQUEST:
      {
	PRINT_WARN( "RECEIVED A REQUEST" );

	stg_target_t* tgt = (stg_target_t*)msg->payload;

	void* data;
	size_t len;
	
	if( model_get_prop( model, tgt->prop, &data, &len ) )      
	  PRINT_WARN2( "failed to service request for property %d(%s)",
		       tgt->prop, stg_property_string(tgt->prop) );
	else
	  {
	    PRINT_WARN( "SENDING A REPLY" );
	    
	    stg_msg_t* reply = stg_msg_create( STG_MSG_MODEL_REPLY, 
					       STG_RESPONSE_NONE,
					       data, len );
	    stg_fd_msg_write( fd, reply );
	    free( reply );
	  }
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

void model_print( model_t* mod )
{
  printf( "   model %d:%d:%s\n", mod->world->id, mod->id, mod->token );
}

void model_print_cb( gpointer key, gpointer value, gpointer user )
{
  model_print( (model_t*)value );
}


