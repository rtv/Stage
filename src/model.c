#include <assert.h>
#include <math.h>

//#define DEBUG
//#undef DEBUG

#include "stage.h"
#include "gui.h"
#include "raytrace.h"

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
  
  mod->cfg = NULL;
  mod->cfg_len = 0;

  mod->laser_return = LaserVisible;
  mod->obstacle_return = TRUE;

  // sensible defaults
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
  assert( mod->lines = malloc( mod->lines_count * sizeof(stg_line_t)) );

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


  PRINT_WARN2( "model %d type %d", mod->id, mod->type );

  gui_model_create( mod );

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

  stg_pose_t* mypose = model_get_pose( mod );
  
  memcpy( pose, mypose, sizeof(stg_pose_t) ); 
  pose->a = NORMALIZE( pose->a );
}

// should one day do all this with affine transforms for neatness
void model_local_to_global( model_t* mod, stg_pose_t* pose )
{  
  stg_pose_t origin;  
  //pose_sum( &origin, &mod->pose, &mod->origin );
  model_global_pose( mod, &origin );
  
  stg_geom_t* geom = model_get_geom(mod); 

  pose_sum( &origin, &origin, &geom->pose );
  pose_sum( pose, &origin, pose );
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


// if render is true, render the model into the matrix, else unrender
// the model
void model_map( model_t* mod, gboolean render )
{
  assert( mod );
  
  size_t count=0;
  stg_line_t* lines = model_get_lines(mod, &count);

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

int model_update( model_t* mod )
{
  //PRINT_DEBUG2( "updating model %d:%s", model->id, model->token );
  
  mod->interval_elapsed = 0;

  // if this type of model has an update function, call it.
  if( derived[ mod->type ].update && mod->subs[STG_PROP_DATA] > 0 )
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
      if( derived[mod->type].startup ) 
	derived[mod->type].startup(mod);        
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
      if( derived[mod->type].shutdown ) 
	derived[mod->type].shutdown(mod);  
      else
	model_shutdown_default(mod);
    }

  if( mod->subs[pid] < 0 )
    PRINT_ERR1( "subscription count has gone below zero (%d). weird.",
		mod->subs[pid] );
}

int model_get_prop( model_t* mod, stg_id_t pid, void** data, size_t* len )
{
  PRINT_DEBUG1( "default get method mod %d", mod->id );

  stg_property_t* prop = NULL;
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

// TODO
//stg_ms_t* model_get_time( model_t* mod )
//{
//return & 



void model_set_laserreturn( model_t* mod, stg_laser_return_t* val )
{
  memcpy( &mod->laser_return, val, sizeof(mod->laser_return) );
}

int model_size_error( model_t* mod, stg_id_t pid, 
		      size_t actual, size_t correct )
{
  PRINT_WARN5( "model %d(%s) received wrong size %s (%d/%d bytes)",
	       mod->id, 
	       mod->token, 
	       stg_property_string(pid), 
	       (int)actual, 
	       (int)correct );

  return 1; // always returns an error
}

int model_set_prop( model_t* mod, stg_id_t propid, void* data, size_t len )
{
  switch( propid )
    {
    case STG_PROP_POSE:  
      if( len == sizeof(stg_pose_t) ) 
	model_set_pose( mod, (stg_pose_t*)data );
      else
	model_size_error( mod, propid, len, sizeof(stg_pose_t) );
      break;
      
    case STG_PROP_GEOM: 
       if( len == sizeof(stg_geom_t) ) 
	 model_set_geom( mod, (stg_geom_t*)data );
      else
	model_size_error( mod, propid, len, sizeof(stg_geom_t) );
       break;

    case STG_PROP_VELOCITY:  
      if( len == sizeof(stg_velocity_t) ) 
	model_set_velocity( mod, (stg_velocity_t*)data );
      else
	model_size_error( mod, propid, len, sizeof(stg_velocity_t) );
      break;

    case STG_PROP_COLOR:  
      if( len == sizeof(stg_color_t) ) 
	model_set_color( mod, (stg_color_t*)data );
      else
	model_size_error( mod, propid, len, sizeof(stg_color_t) );
      break;

    case STG_PROP_MASS:  
      if( len == sizeof(stg_kg_t) ) 
	model_set_mass( mod, (stg_kg_t*)data );
      else
	model_size_error( mod, propid, len, sizeof(stg_kg_t) );
      break;
      
    case STG_PROP_LASERRETURN: 
      if( len == sizeof(stg_laser_return_t) ) 
	model_set_laserreturn( mod, (stg_laser_return_t*)data ); 
      else
	model_size_error( mod, propid, len, sizeof(stg_laser_return_t) );      
      break;

    case STG_PROP_LINES:
      model_set_lines( mod, (stg_line_t*)data, len / sizeof(stg_line_t) );
      break;

    case STG_PROP_GUIFEATURES: 
      if( len == sizeof(stg_guifeatures_t) ) 
	model_set_guifeatures( mod, (stg_guifeatures_t*)data ); 
      else
	model_size_error( mod, propid, len, sizeof(stg_guifeatures_t) );      
      break;

      
      
      // etc!

    default:
      PRINT_WARN3( "unhandled property %d(%s) of size %d bytes",
		   propid, stg_property_string(propid), len );
      break;
    }

  return 0; // OK
}


int model_remove_prop_generic( model_t* mod, stg_id_t propid )
{
  g_hash_table_remove( mod->props, &propid );
  
  return 0; // OK
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

/*     case STG_MSG_MODEL_CMD: */
/*       // commands don't need a reply */
/*       { */
/* 	stg_prop_t* mp = (stg_prop_t*)msg->payload; */
	
/* 	PRINT_DEBUG4( "command %d:%d type %d with %d bytes", */
/* 		      mp->world, */
/* 		      mp->model, */
/* 		      model->type, */
/* 		      (int)mp->datalen ); */
	
/* 	model_putcommand( model, mp->data, mp->datalen );  */
/*       } */
/*       break; */

/*     case STG_MSG_MODEL_DATA: */
/*       { */
/* 	stg_prop_t* mp = (stg_prop_t*)msg->payload; */
	
/* 	PRINT_DEBUG3( "getdata request %d:%d type %d ", */
/* 		      mp->world, */
/* 		      mp->model, */
/* 		      model->type ); */

/* 	void* data; */
/* 	size_t len; */

/* 	if( model_getdata( model, &data, &len ) ) */
/* 	  PRINT_WARN2( "failed to service request for data %d:%d", */
/* 		       mp->world, mp->model ); */
/* 	else */
/* 	  stg_fd_msg_write( fd, STG_MSG_CLIENT_REPLY, data, len );	 */
/*       } */
/*       break; */

      // everything else needs a reply
    case STG_MSG_MODEL_PROPGET:
      {
	PRINT_DEBUG( "RECEIVED A PROPGET REQUEST" );
	
	stg_target_t* tgt = (stg_target_t*)msg->payload;
	
	void* data;
	size_t len;
	
	switch( tgt->prop )
	  {
	  case STG_PROP_DATA: 
	    assert( model_getdata( model, &data, &len ) == 0 );
	    break;
	  case STG_PROP_COMMAND: 
	    assert( model_getcommand( model, &data, &len ) == 0 );
	    break;
	  case STG_PROP_CONFIG: 
	    assert( model_getconfig( model, &data, &len ) == 0 );
	    break;
	    
	  default:
	    // reply with the requested property
	    if( model_get_prop( model, tgt->prop, &data, &len ) )      
	      PRINT_WARN2( "failed to service request for property %d(%s)",
			   tgt->prop, stg_property_string(tgt->prop) );
	  }
	
	PRINT_DEBUG( "SENDING A REPLY" );	 	    
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


