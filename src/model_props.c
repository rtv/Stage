//////////////////////////////////////////////////////////////////////////
//
// File: model_props.c
// Author: Richard Vaughan
// Date: 26 August 2004
// Desc: get and set functions for a stg_model_t's properties
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/model_props.c,v $
//  $Author: rtv $
//  $Revision: 1.32 $
//
///////////////////////////////////////////////////////////////////////////

//#define DEBUG


#include "stage_internal.h"


int model_size_error( stg_model_t* mod, stg_id_t pid, 
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

// DATA/COMMAND/CONFIG ----------------------------------------------------
// these set and get the generic buffers for derived types
//

int _get_something( void* dest, size_t dest_len, 
		    void* src, size_t src_len, 
		    pthread_mutex_t* mutex )
{
  size_t copy_len = MIN( dest_len, src_len );
  pthread_mutex_lock( mutex );
  memcpy( dest, src, copy_len );
  pthread_mutex_unlock( mutex );
  return copy_len;
}

int stg_model_get_data( stg_model_t* mod, void* dest, size_t len )
{
  return _get_something( dest, len, mod->data, mod->data_len, &mod->data_mutex );
}

int stg_model_get_command( stg_model_t* mod, void* dest, size_t len )
{
  return _get_something( dest, len, mod->cmd, mod->cmd_len, &mod->cmd_mutex );
}

int stg_model_get_config( stg_model_t* mod, void* dest, size_t len )
{
  return _get_something( dest, len, mod->cfg, mod->cfg_len, &mod->cfg_mutex );
}

int stg_model_set_data( stg_model_t* mod, void* data, size_t len )
{
  pthread_mutex_lock( &mod->data_mutex );
  mod->data = realloc( mod->data, len );
  memcpy( mod->data, data, len );    
  mod->data_len = len;
  pthread_mutex_unlock( &mod->data_mutex );

  // if a callback was registered, call it
  if( mod->data_notify )
    {
      //PRINT_WARN2( "calling notify func for %s at time %d", 
      //	   mod->token, mod->world->sim_time ); 
      (*mod->data_notify)(mod->data_notify_arg);
    }
  
  // if a rendering callback was registered, and the gui wants to
  // render this type of data, call it
  if( mod->f_render_data && 
      mod->world->win->render_data_flag[mod->type] )
    (*mod->f_render_data)(mod,data,len);
  else
    {
      // remove any graphics that linger
      if( mod->gui.data )
	rtk_fig_clear( mod->gui.data );
      
      if( mod->gui.data_bg )
	rtk_fig_clear( mod->gui.data_bg );
    }
  
  
  PRINT_DEBUG3( "model %d(%s) put data of %d bytes",
		mod->id, mod->token, (int)mod->data_len);
  return 0; //ok
}

int stg_model_set_command( stg_model_t* mod, void* cmd, size_t len )
{
  pthread_mutex_lock( &mod->cmd_mutex );  
  mod->cmd = realloc( mod->cmd, len );
  memcpy( mod->cmd, cmd, len );    
  mod->cmd_len = len;  
  pthread_mutex_unlock( &mod->cmd_mutex );

  // if a rendering callback was registered, and the gui wants to
  // render this type of command, call it
  if( mod->f_render_cmd && 
      mod->world->win->render_cmd_flag[mod->type] )
    (*mod->f_render_cmd)(mod,cmd,len);
  else
    {
      // remove any graphics that linger
      if( mod->gui.cmd )
	rtk_fig_clear( mod->gui.cmd );
      
      if( mod->gui.cmd_bg )
	rtk_fig_clear( mod->gui.cmd_bg );
    }
  

  PRINT_DEBUG3( "model %d(%s) put command of %d bytes",
		mod->id, mod->token, (int)mod->cmd_len);

  //PRINT_WARN3( "model %d(%s) put command of %d bytes",
  //	mod->id, mod->token, (int)mod->cmd_len);
  return 0; //ok
}

int stg_model_set_config( stg_model_t* mod, void* cfg, size_t len )
{ 
  pthread_mutex_lock( &mod->cfg_mutex );  
  mod->cfg = realloc( mod->cfg, len );
  memcpy( mod->cfg, cfg, len );    
  mod->cfg_len = len;
  pthread_mutex_unlock( &mod->cfg_mutex );  
  
  //printf( "setting config for model %d", mod->id );
  
  // if a rendering callback was registered, and the gui wants to
  // render this type of cfg, call it
  if( mod->f_render_cfg && 
      mod->world->win->render_cfg_flag[mod->type] )
    (*mod->f_render_cfg)(mod,cfg,len);
  else
    {
      // remove any graphics that linger
      if( mod->gui.cfg )
	rtk_fig_clear( mod->gui.cfg );
      
      if( mod->gui.cfg_bg )
	rtk_fig_clear( mod->gui.cfg_bg );
    }
  
  PRINT_DEBUG3( "model %d(%s) put command of %d bytes",
		mod->id, mod->token, (int)mod->cfg_len);
  return 0; //ok
}

int _model_update( stg_model_t* mod )
{
  mod->interval_elapsed = 0;
  
  // now move the model if it has any velocity
  if( (mod->velocity.x || mod->velocity.y || mod->velocity.a ) )
    stg_model_update_pose( mod );

  return 0; //ok
}

int _model_startup( stg_model_t* mod )
{
  PRINT_DEBUG( "default startup proc" );
  return 0; //ok
}

int _model_shutdown( stg_model_t* mod )
{
  PRINT_DEBUG( "default shutdown proc" );
  return 0; //ok
}

int stg_model_update( stg_model_t* mod )
{
  assert( mod->f_update );
  int val = mod->f_update(mod);
  return val;
}


int stg_model_startup( stg_model_t* mod )
{
  assert(mod->f_startup );
  return mod->f_startup(mod);
}

int stg_model_shutdown( stg_model_t* mod )
{
  assert(mod->f_shutdown );
  return mod->f_shutdown(mod);
}



//------------------------------------------------------------------------
// basic model properties

void stg_model_get_pose( stg_model_t* model, stg_pose_t* pose )
{
  assert(model);
  assert(pose);
  memcpy( pose, &model->pose, sizeof(stg_pose_t));
}

void stg_model_get_velocity( stg_model_t* mod, stg_velocity_t* dest )
{
  assert(mod);
  assert(dest);
  memcpy( dest, &mod->velocity, sizeof(mod->velocity));
}

void stg_model_get_geom( stg_model_t* mod, stg_geom_t* dest )
{
  assert(mod);
  assert(dest);
  memcpy( dest, &mod->geom, sizeof(mod->geom));
}

void stg_model_get_color( stg_model_t* mod, stg_color_t* dest )
{
  assert(mod);
  assert(dest);
  memcpy( dest, &mod->color, sizeof(mod->color));
}

void stg_model_get_mass( stg_model_t* mod, stg_kg_t* dest )
{
  assert(mod);
  assert(dest);
  memcpy( dest, &mod->obstacle_return, sizeof(mod->obstacle_return));
}

void stg_model_get_guifeatures( stg_model_t* mod, stg_guifeatures_t* dest )
{
  assert(mod);
  assert(dest);
  memcpy( dest, &mod->guifeatures, sizeof(mod->guifeatures));
}

void stg_model_get_laserreturn( stg_model_t* mod, stg_laser_return_t* dest )
{
  assert(mod);
  assert(dest);
  memcpy( dest, &mod->laser_return, sizeof(mod->laser_return));
}

void stg_model_get_fiducialreturn( stg_model_t* mod,stg_fiducial_return_t* dest )
{
  assert(mod);
  assert(dest);
  memcpy( dest, &mod->fiducial_return, sizeof(mod->fiducial_return));
}

//void stg_model_get_friction( stg_model_t* mod,stg_friction_t* dest )
//{
//assert(mod);
//assert(dest);
//memcpy( dest, &mod->friction, sizeof(mod->friction));
//}


void stg_model_get_obstaclereturn( stg_model_t* mod, stg_obstacle_return_t* dest )
{
  assert(mod);
  assert(dest);
  memcpy( dest, &mod->obstacle_return, sizeof(mod->obstacle_return));
}


void stg_model_get_blobreturn( stg_model_t* mod, stg_blob_return_t* dest )
{
  assert(mod);
  assert(dest);
  memcpy( dest, &mod->blob_return, sizeof(mod->blob_return));
}

stg_polygon_t* stg_model_get_polygons( stg_model_t* mod, size_t* poly_count )
{
  *poly_count = mod->polygons->len;
  return (stg_polygon_t*)mod->polygons->data;
}

/* SET basic model properties


/** set the pose of a model in its parent's CS */
int stg_model_set_pose( stg_model_t* mod, stg_pose_t* pose )
{
  // if the new pose is different
  if( memcmp( &mod->pose, pose, sizeof(stg_pose_t) ))
    {
      // unrender from the matrix
      stg_model_map_with_children( mod, 0 );
      
      // copy the new pose
      memcpy( &mod->pose, pose, sizeof(mod->pose) );
      
      // render in the matrix
      stg_model_map_with_children( mod, 1 );
      
      // move the rtk figure to match
      gui_model_move( mod );      
    }
  
  return 0; // OK
}

/** set the pose of model in global coordinates */
int stg_model_set_global_pose( stg_model_t* mod, stg_pose_t* gpose )
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

  return 0; //ok
}

int stg_model_set_geom( stg_model_t* mod, stg_geom_t* geom )
{ 
  // unrender from the matrix
  stg_model_map( mod, 0 );
  
  // store the geom
  memcpy( &mod->geom, geom, sizeof(mod->geom) );
  
  // we probably need to scale and re-render our polygons
  stg_normalize_polygons( mod->polygons->data, mod->polygons->len, 
			  mod->geom.size.x, mod->geom.size.y );
  
  stg_model_render_polygons( mod );
  
  // we may need to re-render our nose
  gui_model_features(mod);
  
  // re-render int the matrix
  stg_model_map( mod, 1 );

  return 0;
}


int stg_model_set_polygons( stg_model_t* mod, stg_polygon_t* polys, size_t poly_count )
{
  assert(mod);
  if( poly_count > 0 ) assert( polys );
  
  // background (used e.g for laser scan fill)
  PRINT_DEBUG3( "model %d(%s) received %d polygons", 
		mod->id, mod->token, (int)poly_count );

  // normalize the polygons to fit exactly in the model's body rectangle
  stg_normalize_polygons( polys, poly_count, mod->geom.size.x, mod->geom.size.y );
  
  stg_model_map( mod, 0 ); // unmap the model from the matrix
  
  // remove the old polygons
  g_array_set_size( mod->polygons, 0 );
  
  
  // append the new polys
  g_array_append_vals( mod->polygons, polys, poly_count );
  
  // if we need a boundary, add a unit rectangle to the polys
  //if( mod->guifeatures.boundary )
  //{
  //  PRINT_WARN1( "adding a boundary to mod %s", mod->token );
  //g_array_append_vals( mod->polygons, unit_polygon_create(), poly_count );
  // }

  stg_model_render_polygons( mod );

  stg_model_map( mod, 1 ); // map the model into the matrix with the new polys
} 

int stg_model_set_laserreturn( stg_model_t* mod, stg_laser_return_t* val )
{
  memcpy( &mod->laser_return, val, sizeof(mod->laser_return));
  return 0;
}

int stg_model_set_obstaclereturn( stg_model_t* mod, stg_obstacle_return_t* val )
{
  memcpy( &mod->obstacle_return, val, sizeof(mod->obstacle_return));
  return 0;
}


int stg_model_set_velocity( stg_model_t* mod, stg_velocity_t* val )
{
  memcpy( &mod->velocity, val, sizeof(mod->velocity) );
  return 0;
}

int stg_model_set_blobreturn( stg_model_t* mod, stg_blob_return_t* val )
{
  memcpy( &mod->blob_return, val, sizeof(mod->blob_return) );
  return 0;
}

int stg_model_set_color( stg_model_t* mod, stg_color_t* col )
{
  memcpy( &mod->color, col, sizeof(mod->color) );

  // redraw my image
  stg_model_render_polygons( mod );
    
  return 0; // OK
}

int stg_model_set_mass( stg_model_t* mod, stg_kg_t* mass )
{
  memcpy( &mod->mass, mass, sizeof(mod->mass) );
  return 0;
}


int stg_model_set_guifeatures( stg_model_t* mod, stg_guifeatures_t* gf )
{
  memcpy( &mod->guifeatures, gf, sizeof(mod->guifeatures));
  
  // override the movemask flags - only top-level objects are moveable
  if( mod->parent )
    mod->guifeatures.movemask = 0;

  // redraw the fancy features
  gui_model_features( mod );

  return 0;
}

int stg_model_set_fiducialreturn( stg_model_t* mod, stg_fiducial_return_t* val )     
{
  memcpy( &mod->fiducial_return, val, sizeof(mod->fiducial_return) );
  return 0;
}

