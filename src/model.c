

#include <assert.h>
#include <math.h>

//#define DEBUG
//#undef DEBUG

#include "stage.h"
#include "gui.h"
#include "raytrace.h"

extern lib_entry_t library[];
extern lib_entry_t derived[];


void prop_free( stg_property_t* prop )
{
  if( prop )
    {
      if( prop->data ) 
	free( prop->data );
      free( prop );
    }
}

void prop_free_cb( gpointer gp )
{
  prop_free( (stg_property_t*)gp);
}

model_t* model_create(  world_t* world, 
			model_t* parent,
			stg_id_t id, 
			stg_model_type_t type,
			char* token )
{
  PRINT_DEBUG3( "creating model %d:%d (%s)", world->id, id, token );

  if( parent )
    {
      PRINT_DEBUG2( "   a child of %d(%s)", parent->id, parent->token );
    }
  
  // calloc zeros the whole structure
  model_t* mod = calloc( sizeof(model_t),1 );

  mod->id = id;
  mod->token = strdup(token);
  mod->world = world;
  mod->parent = NULL;//parent; // TODO - fix parent stuff from here
		     //onwards - mainly geometry for matrix rendering

  // models store data in here, indexed by property id
  mod->props = g_hash_table_new_full( g_int_hash, g_int_equal, NULL, prop_free_cb );
  
  mod->type = type;

  mod->data = NULL;
  mod->data_len = 0;
  
  mod->cmd = NULL;
  mod->cmd_len = 0;

  PRINT_WARN2( "model %d type %d", mod->id, mod->type );

  gui_model_create( mod );

  int p;
  for( p=0; p<STG_PROP_COUNT; p++ )
    if( library[p].init )
      library[p].init(mod);

  // if this type of model has an init function, call it.
  if( derived[ mod->type ].init )
    derived[ mod->type ].init(mod);
  
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


void model_global_pose( model_t* mod, stg_pose_t* pose )
{ 
  //stg_pose_t glob;
  
  //if( mod->parent )
  //model_global_pose( mod->parent, &glob );
  
  //memcpy( &glob, &mod->pose, sizeof(glob) );
 
  
  //model_local_to_global( mod, &glob );

  //memset( &origin, 0, sizeof(origin) );
    
  //pose_sum( pose, &glob, &mod->pose );

  stg_pose_t* mypose = model_pose_get( mod );
  
  memcpy( pose, mypose, sizeof(stg_pose_t) ); 
  pose->a = NORMALIZE( pose->a );
}

// should one day do all this with affine transforms for neatness
void model_local_to_global( model_t* mod, stg_pose_t* pose )
{  
  stg_pose_t origin;  
  //pose_sum( &origin, &mod->pose, &mod->origin );
  model_global_pose( mod, &origin );
  
  stg_geom_t* geom = model_geom_get(mod); 

  pose_sum( &origin, &origin, &geom->pose );
  pose_sum( pose, &origin, pose );
}

void model_global_rect( model_t* mod, stg_rotrect_t* glob, stg_rotrect_t* loc )
{
  stg_geom_t* geom = model_geom_get(mod); 

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


// if render is true, render the model into the matrix, else unrender
// the model
void model_map( model_t* mod, gboolean render )
{
  assert( mod );
  
  size_t count=0;
  stg_line_t* lines = model_lines_get(mod, &count);

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
  



/* UPDATE everything */

// TODO - fix up the library so that models get
// subscription-independent updates then this can disappear.


int model_update( model_t* mod )
{
  //PRINT_DEBUG2( "updating model %d:%s", model->id, model->token );
  
  mod->interval_elapsed = 0;

  // call all registered update functions
  int p;
  for( p=0; p<STG_PROP_COUNT; p++ )
    if( library[p].update )
      library[p].update(mod);

  // if this type of model has an update function, call it.
  if( derived[ mod->type ].update )
    derived[ mod->type ].update(mod);

  return 0;
}

void model_update_cb( gpointer key, gpointer value, gpointer user )
{
  model_update( (model_t*)value );
}

int model_service_default( model_t* mod )
{
  PRINT_DEBUG1( "default service method mod %d", mod->id );

  // if this type of model has an service function, call it.
  if( derived[ mod->type ].service )
    derived[ mod->type ].service(mod);

  return 0;
}

int model_update_prop( model_t* mod, stg_id_t pid )
{
  if( library[pid].service )
    library[pid].service(mod);
  else
    model_service_default(mod);
  
  mod->update_times[pid] = mod->world->sim_time;
  
  return 0; // ok
}


int model_startup_default( model_t* mod )
{
  //PRINT_DEBUG( "default startup method" );
  PRINT_DEBUG1( "default startup method mod %d", mod->id );
  
  // if this type of model has a startup function, call it.
  if( derived[ mod->type ].startup )
   return  derived[ mod->type ].startup(mod);
  
  return 0;
}

void model_subscribe( model_t* mod, stg_id_t pid )
{
  mod->subs[pid]++;
  
  // if this is the first sub, call startup & render if there is one
  if( mod->subs[pid] == 1 )
    {
      if( library[pid].startup ) 
	library[pid].startup(mod);        
      else
	model_startup_default(mod);
    }
}

int model_shutdown_default( model_t* mod )
{
  PRINT_DEBUG1( "default shutdown method mod %d", mod->id );

  // if this type of model has a shutdown function, call it.
  if( derived[ mod->type ].shutdown )
   return  derived[ mod->type ].shutdown(mod);

  return 0;
}

void model_unsubscribe( model_t* mod, stg_id_t pid )
{
  mod->subs[pid]--;
  
  // if that was the last sub, call shutdown 
  if( mod->subs[pid] < 1 )
    {
      if( library[pid].shutdown ) 
	library[pid].shutdown(mod);  
      else
	model_shutdown_default(mod);
    }

  if( mod->subs[pid] < 0 )
    PRINT_ERR1( "subscription count has gone below zero (%d). weird.",
		mod->subs[pid] );
}

int model_get_prop_default( model_t* mod, stg_id_t pid, void** data, size_t* len )
{
  PRINT_DEBUG1( "default get method mod %d", mod->id );

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

  return 0; //ok
}

int model_getdata( model_t* mod, void** data, size_t* len )
{
  // if this type of model has an getdata function, call it.
  if( derived[ mod->type ].getdata )
    {
      derived[ mod->type ].getdata(mod, data, len);
      PRINT_WARN1( "used special getdata, returned %d bytes", (int)*len );
    }
  else
    { // we do a generic data copy
      *data = mod->data;
      *len = mod->data_len; 
      PRINT_WARN1( "used generic getdata, returned %d bytes", (int)*len );
    }
  return 0; //ok
}

int model_putcommand( model_t* mod, void* cmd, size_t len )
{
  mod->cmd = realloc( mod->cmd, len );
  memcpy( mod->cmd, cmd, len );
  mod->cmd_len = len;
  return 0; //ok
}

int model_get_prop( model_t* mod, stg_id_t pid, void** data, size_t* len )
{
  if( pid == STG_PROP_TIME ) // no data - just fetches a timestamp
    {
      *data = NULL;
      *len = 0;
      return 0;
    }
  
  // else
  if( library[pid].get ) 
    {
      *data = library[pid].get(mod, len);    
      return 0;
    }

  // else
  return model_get_prop_default( mod, pid, data, len );
}


int model_set_prop_generic( model_t* mod, stg_id_t propid, void* data, size_t len )
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

  return 0; // OK
}


int model_remove_prop_generic( model_t* mod, stg_id_t propid )
{
  g_hash_table_remove( mod->props, &propid );
  
  return 0; // OK
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
  
  if( prop == NULL )
    PRINT_WARN3( "no property entry found for model %d property %d(%s)",
		 mod->id, propid, stg_property_string(propid) );
  
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


  if( propid == STG_PROP_TIME )
    {
      PRINT_WARN3( "Ignoring attempt to set time for %d:%d(%s).", 	
		   mod->world->id, mod->id, mod->token );
      
      return 0;
    }
  
  // else
  if( library[propid].set )
    return library[propid].set(mod,data,len);
      
  // else        
  return model_set_prop_generic( mod, propid, data, len );            
}


void model_register_init( stg_id_t pid, func_init_t func )
{
  library[pid].init = func;
}

void model_register_startup( stg_id_t pid, func_startup_t func )
{
  library[pid].startup = func;
}

void model_register_shutdown( stg_id_t pid, func_shutdown_t func )
{
  library[pid].shutdown = func;
}

void model_register_update( stg_id_t pid, func_update_t func )
{
  library[pid].update = func;
}

void model_register_service( stg_id_t pid, func_service_t func )
{
  library[pid].service = func;
}

void model_register_set( stg_id_t pid, func_set_t func )
{
  library[pid].set = func;
}

void model_register_get( stg_id_t pid, func_get_t func )
{
  library[pid].get = func;
}

// new registration funcs
void register_init( stg_model_type_t type, func_init_t func )
{
  derived[type].init = func;
}

void register_startup( stg_model_type_t type, func_startup_t func )
{
  derived[type].startup = func;
}

void register_shutdown( stg_model_type_t type, func_shutdown_t func )
{
  derived[type].shutdown = func;
}

void register_getdata( stg_model_type_t type, func_getdata_t func )
{
  derived[type].getdata = func;
}

void register_update( stg_model_type_t type, func_update_t func )
{
  derived[type].update = func;
}

void register_putcommand( stg_model_type_t type, func_putcommand_t func )
{
  derived[type].putcommand = func;
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


