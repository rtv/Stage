

#include <assert.h>
#include <math.h>

#include "stage.h"
#include "gui.h"
#include "raytrace.h"

#define DEBUG
//#undef DEBUG


void prop_free( stg_property_t* prop )
{
  if( prop )
    {
      if( prop->data ) 
	free( prop->data );
      free( prop );
    }
}

extern libitem_t library[];

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
  
  memset( &mod->energy, 0, sizeof(mod->energy) );
  mod->energy.joules = 3600000L; // 1 Amp hour
  mod->energy.move_cost = 1000; // one joule per move (quite high!)
  
  // models store data in here, indexed by property id
  mod->props = g_hash_table_new_full( g_int_hash, g_int_equal, NULL, prop_free );
  
  mod->laser_return = LaserVisible;
  // a sensible default fiducial return value is the model's id
  mod->fiducial_return = mod->id;
  mod->ranger_return = LaserVisible;

  mod->geom.pose.x = 0;
  mod->geom.pose.y = 0;
  mod->geom.pose.a = 0;
  mod->geom.size.x = 0.4;
  mod->geom.size.y = 0.4;
  
  mod->obstacle_return = 1;

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
  stg_scale_lines( lines, 4, mod->geom.size.x, mod->geom.size.y );
  stg_translate_lines( lines, 4, -mod->geom.size.x/2.0, -mod->geom.size.y/2.0 );
  
  // stash the lines
  mod->lines = g_array_new( FALSE, TRUE, sizeof(stg_line_t) );  
  g_array_append_vals(  mod->lines, lines, 4 );
  
  mod->movemask = STG_MOVE_TRANS | STG_MOVE_ROT;
  
  gui_model_create( mod );

  int prop;
  for( prop=0; prop< STG_PROP_COUNT; prop++ )
    if( library[prop].init )
      {
	//printf( "calling init for %s\n", stg_property_string(prop) );
	library[prop].init(mod);
      }

  return mod;
}

void model_destroy( model_t* mod )
{
  assert( mod );
  
  gui_model_destroy( mod );
  
  if( mod->token ) free( mod->token );

  if(mod->props) g_hash_table_destroy( mod->props );

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
  pose->a = NORMALIZE( pose->a );
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
  double w = mod->geom.size.x;
  double h = mod->geom.size.y;

  // scale first
  glob->x = ((loc->x + loc->w/2.0) * w) - w/2.0;
  glob->y = ((loc->y + loc->h/2.0) * h) - h/2.0;
  glob->w = loc->w * w;
  glob->h = loc->h * h;
  glob->a = loc->a;
  
  // now transform into local coords
  model_local_to_global( mod, (stg_pose_t*)glob );
}


void model_map( model_t* mod, gboolean render )
{
  assert( mod );

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
  if( library[ propid ].update )
    {
      //printf( "Calling update for %p %s\n", 
      //      mod, stg_property_string(propid) );
      
      library[ propid ].update( mod );
    }
  
  mod->update_times[ propid ] = mod->world->sim_time;
  
  return 0; // ok
}


void model_subscribe( model_t* mod, stg_id_t pid )
{
  mod->subs[pid]++;
  
  // if this is the first sub, call startup & render if there is one
  if( mod->subs[pid] == 1 )
    {
      if( library[pid].startup ) library[pid].startup(mod);  
      //if( library[pid].render)   library[pid].render(mod);  
    }
  
  gui_model_render( mod );
}

void model_unsubscribe( model_t* mod, stg_id_t pid )
{
  mod->subs[pid]--;
  
  // if that was the last sub, call shutdown and render (to unrender)
  if( mod->subs[pid] < 1 && library[pid].shutdown )
    {
      if( library[pid].render)   library[pid].render(mod);  
      if( library[pid].shutdown ) library[pid].shutdown(mod);        
    }

  gui_model_render( mod );
}

int model_get_prop( model_t* mod, stg_id_t pid, void** data, size_t* len )
{
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
    case STG_PROP_GEOM:
      *data = &mod->geom;
      *len = sizeof(mod->geom);
      break;
    case STG_PROP_VELOCITY:
      *data = &mod->velocity;
      *len = sizeof(mod->velocity);
      break;
    case STG_PROP_COLOR:
      *data = &mod->color;
      *len = sizeof(mod->color);
      break;
    case STG_PROP_ENERGY:
      *data = &mod->energy;
      *len = sizeof(mod->energy);
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

    default:
      {
	stg_property_t* prop = model_get_prop_generic( mod, pid);
	if( prop )
	  {
	    PRINT_DEBUG3( "data found for model %d property %d(%s)",
			  mod->id, pid, stg_property_string(pid) );
	    
	    *data = prop->data;
	    *len = prop->len;
	  }
	else
	  {
	    PRINT_WARN3( "no data available for model %d property %d(%s)",
			 mod->id, pid, stg_property_string(pid) );
	    *data = NULL;
	    *len = 0;
	  }
      }
      break;
    }

  return 0; //ok
}


void model_set_prop_generic( model_t* mod, stg_id_t propid, void* data, size_t len )
{
  assert( mod );
  assert( mod->props );
  if( len > 0 ) assert( data );
  
  stg_property_t* prop = calloc( sizeof(stg_property_t), 1 );
  assert(prop);
  prop->id = propid;
  prop->data = calloc( len, 1 );
  memcpy( prop->data, data, len );
  prop->len = len;
  
  //printf( "setting %p %d:%d(%s) with %d bytes\n", 
  //  mod->props, mod->id, propid, stg_property_string(propid), (int)len );
  
  g_hash_table_replace( mod->props, &prop->id, prop );
  
  if( library[ propid ].render )
    {
      //printf( "Calling render for %p %s\n", 
      //      mod, stg_property_string(propid) );
      
      library[ propid ].render( mod );
    }

  //gui_model_update( mod, propid );
}


void model_remove_prop_generic( model_t* mod, stg_id_t propid )
{
  g_hash_table_remove( mod->props, &propid );
}

stg_property_t* model_get_prop_generic( model_t* mod, stg_id_t propid )
{
  //printf( "getting %p %d:%d(%s)\n", 
  //mod->props, mod->id, propid, stg_property_string(propid) );
  
  stg_property_t* prop = 
    (stg_property_t*)g_hash_table_lookup( mod->props, &propid );
  
  //if( prop )
  //printf( "found %d bytes\n", (int)prop->len );
  //else
  //puts( "not found" );  
  
  return prop;
}

// if there is data for this property, return just a pointer to the
// data, else NULL. This is only useful if we know how large the data
// should be (or can find out somehow)
void* model_get_prop_data_generic( model_t* mod, stg_id_t propid )
{
  stg_property_t* prop = model_get_prop_generic( mod, propid );
  
  if( prop == NULL )
    {
      PRINT_WARN3( "attempted to get data for a property that was "
		   "never created (model %p property %d(%s)",
		   mod, propid, stg_property_string( propid ) );
      return NULL;
    }
  // else  
  return prop->data;
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
      
    case STG_PROP_VELOCITY:
      if( len == sizeof(mod->velocity) )
	memcpy( &mod->velocity, data, len );
      else PRINT_WARN2( "ignoring bad velocity data (%d/%d bytes)", 
		       (int)len, (int)sizeof(mod->velocity) );
      break;
      
    case STG_PROP_GEOM:
      if( len == sizeof(mod->geom) )
	{
	  model_map( mod, 0 );
	  memcpy( &mod->geom, data, len );
	  
	  // force the body lines to fit inside this new rectangle
	  stg_normalize_lines( (stg_line_t*)mod->lines->data, mod->lines->len );
	  stg_scale_lines( (stg_line_t*)mod->lines->data, mod->lines->len, 
			   mod->geom.size.x, mod->geom.size.y );
	  stg_translate_lines( (stg_line_t*)mod->lines->data, mod->lines->len,
			       -mod->geom.size.x/2.0, -mod->geom.size.y/2.0 );
	  model_map( mod, 1 );
	}
      else PRINT_WARN2( "ignoring bad size data (%d/%d bytes)", 
		       (int)len, (int)sizeof(mod->geom) );
      break;
      
    case STG_PROP_COLOR:
      if( len == sizeof(mod->color) )
	memcpy( &mod->color, data, len );
      else PRINT_WARN2( "ignoring bad color data (%d/%d bytes)", 
		       (int)len, (int)sizeof(mod->color) );
      break;
      
    case STG_PROP_ENERGY:
      if( len == sizeof(mod->energy) )
	memcpy( &mod->energy, data, len );
      else PRINT_WARN2( "ignoring bad energy data (%d/%d bytes)", 
			(int)len, (int)sizeof(mod->energy) );
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
      
      
    default:
      // accept random prop types and stash data in hash table
      model_set_prop_generic( mod, propid, data, len );            
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

	void* data;
	size_t len;
	
	// reply with the requested property
	if( model_get_prop( model, tgt->prop, &data, &len ) )      
	  PRINT_WARN2( "failed to service request for property %d(%s)",
		       tgt->prop, stg_property_string(tgt->prop) );
	else
	  {
	    PRINT_DEBUG( "SENDING A REPLY" );	 	    
	    stg_fd_msg_write( fd, STG_MSG_CLIENT_REPLY, data, len );
	  }
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


