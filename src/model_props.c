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
//  $Revision: 1.20 $
//
///////////////////////////////////////////////////////////////////////////

//#define DEBUG

#define SHOW_GEOM 0

#include "stage.h"

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

int _set_data( stg_model_t* mod, void* data, size_t len )
{
  mod->data = realloc( mod->data, len );
  memcpy( mod->data, data, len );    
  mod->data_len = len;
  
  // if a callback was registered, call it
  if( mod->data_notify )
    {
      //PRINT_WARN2( "calling notify func for %s at time %d", 
      //	   mod->token, mod->world->sim_time ); 
      (*mod->data_notify)(mod->data_notify_arg);
    }
  
  PRINT_DEBUG3( "model %d(%s) put data of %d bytes",
		mod->id, mod->token, (int)mod->data_len);
  return 0; //ok
}

int _set_cmd( stg_model_t* mod, void* cmd, size_t len )
{
  mod->cmd = realloc( mod->cmd, len );
  memcpy( mod->cmd, cmd, len );    
  mod->cmd_len = len;
  
  PRINT_DEBUG3( "model %d(%s) put command of %d bytes",
		mod->id, mod->token, (int)mod->cmd_len);
  return 0; //ok
}

int _set_cfg( stg_model_t* mod, void* cfg, size_t len )
{
  mod->cfg = realloc( mod->cfg, len );
  memcpy( mod->cfg, cfg, len );    
  mod->cfg_len = len;
  
  PRINT_DEBUG3( "model %d(%s) put command of %d bytes",
		mod->id, mod->token, (int)mod->cfg_len);
  return 0; //ok
}

int stg_model_set_data( stg_model_t* mod, void* data, size_t len )
{
  // if this type of model has a set_data function, call it.
  if( mod->lib->set_data )
    {
      mod->lib->set_data(mod, data, len);
      PRINT_DEBUG1( "used special set_data returned %d bytes", (int)len );
    }
  else
    _set_data( mod, data, len );
  
  return 0; //ok
}

int stg_model_set_command( stg_model_t* mod, void* cmd, size_t len )
{
  // if this type of model has a putcommand function, call it.
  if( mod->lib->set_command )
    {
      mod->lib->set_command(mod, cmd, len);
      PRINT_DEBUG1( "used special set_command, put %d bytes", (int)len );
    }
  else
    _set_cmd( mod, cmd, len );    

  return 0; //ok
}

int stg_model_set_config( stg_model_t* mod, void* config, size_t len )
{
  // if this type of model has a putconfig function, call it.
  if( mod->lib->set_config )
    {
      mod->lib->set_config(mod, config, len);
      PRINT_DEBUG1( "used special putconfig returned %d bytes", (int)len );
    }
  else
    _set_cfg( mod, config, len );
  
  return 0; //ok
}


void* stg_model_get_data( stg_model_t* mod, size_t* len )
{
  void* data = NULL; 

  // if this type of model has a getdata function, call it.
  if( mod->lib->get_data )
    {
      data = mod->lib->get_data(mod, len);
      PRINT_DEBUG1( "used special get_data, returned %d bytes", (int)*len );
    }
  else
    { // we do a generic data copy
      data = mod->data;
      *len = mod->data_len; 
      PRINT_DEBUG3( "model %d(%s) generic get_data, returned %d bytes", 
		   mod->id, mod->token, (int)*len );
    }

  return data;
}


void* stg_model_get_command( stg_model_t* mod, size_t* len )
{
  void* command = NULL;
  
  // if this type of model has a getcommand function, call it.
  if( mod->lib->get_command )
    {
      command = mod->lib->get_command(mod, len);
      PRINT_DEBUG1( "used special get_command, returned %d bytes", (int)*len );
    }
  else
    { // we do a generic command copy
      command = mod->cmd;
      *len = mod->cmd_len; 
      PRINT_DEBUG1( "used generic get_command, returned %d bytes", (int)*len );
    }

  return command; 
}


void* stg_model_get_config( stg_model_t* mod, size_t* len )
{
  void* config = NULL;
  
  // if this type of model has an getconfig function, call it.
  if( mod->lib->get_config )
    {
      config = mod->lib->get_config(mod, len);
      PRINT_DEBUG1( "used special get_config returned %d bytes", (int)*len );
    }
  else
    { // we do a generic data copy
      config = mod->cfg;
      *len = mod->cfg_len; 
      PRINT_DEBUG3( "model %d(%s) generic get_config, returned %d bytes", 
		   mod->id, mod->token, (int)*len );
    }

  return config; 
}

//------------------------------------------------------------------------
// basic model properties

stg_bool_t stg_model_get_obstaclereturn( stg_model_t* mod   )
{
  return mod->obstacle_return;
}

int stg_model_set_obstaclereturn( stg_model_t* mod, stg_bool_t* val )
{
  mod->obstacle_return = *val;
  return 0;
}

stg_bool_t stg_model_get_blobreturn( stg_model_t* mod   )
{
  return mod->blob_return;
}

int stg_model_set_blobreturn( stg_model_t* mod, stg_bool_t val )
{
  mod->blob_return = val;
  return 0;
}

stg_pose_t* stg_model_get_odom( stg_model_t* model )
{
  return &model->odom;
}

int stg_model_set_odom( stg_model_t* mod, stg_pose_t* pose )
{
  memcpy( &mod->odom, pose, sizeof(stg_pose_t) );
  return 0;
}

int stg_model_set_friction( stg_model_t* mod, stg_friction_t* fricp )
{
  mod->friction = *fricp;
  return 0;
}

stg_friction_t* stg_model_get_friction( stg_model_t* mod )
{
  return &mod->friction;
}


stg_pose_t* stg_model_get_pose( stg_model_t* model )
{
  return &model->pose;
}

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
      gui_render_pose( mod );
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
}


stg_geom_t* stg_model_get_geom( stg_model_t* mod )
{
  return &mod->geom;
}

int stg_model_set_geom( stg_model_t* mod, stg_geom_t* geom )
{ 
  if( memcmp( &mod->geom, geom, sizeof(stg_geom_t) ))
    {
      // unrender from the matrix
      stg_model_map( mod, 0 );
      
      // store the geom
      memcpy( &mod->geom, geom, sizeof(mod->geom) );
      
      size_t count=0;
      stg_line_t* lines = stg_model_get_lines( mod, &count );
      
      // force the body lines to fit inside this new rectangle
      stg_normalize_lines( lines, count );
      stg_scale_lines(  lines, count, geom->size.x, geom->size.y );
      stg_translate_lines( lines, count, -geom->size.x/2.0, -geom->size.y/2.0 );
      
      // set the new lines (this will cause redraw)
      stg_model_set_lines( mod, lines, count );

      // we may need to re-render our nose
      gui_model_features(mod);

#if SHOW_GEOM
      gui_render_geom( mod );
#endif
     
      // re-render int the matrix
      stg_model_map( mod, 1 );
    }

  return 0;
}

stg_line_t* stg_model_get_lines( stg_model_t* mod, size_t* lines_count )
{
  
  *lines_count = mod->lines_count;
  return mod->lines;
}


int stg_model_set_lines( stg_model_t* mod, stg_line_t* lines, size_t lines_count )
{
  assert(mod);
  if( lines_count > 0 ) assert( lines );

  PRINT_DEBUG3( "model %d(%s) received %d lines", 
		mod->id, mod->token, (int)lines_count );
  
  stg_model_map( mod, 0 );
  
  stg_geom_t* geom = stg_model_get_geom(mod); 
  
  // fit the rectangle inside the model's size
  stg_normalize_lines( lines, lines_count );
  stg_scale_lines( lines, lines_count, geom->size.x, geom->size.y );
  stg_translate_lines( lines, lines_count, -geom->size.x/2.0, -geom->size.y/2.0 );

  size_t len = sizeof(stg_line_t)*lines_count;

  if( len > 0 )
    {
      mod->lines = realloc( mod->lines,len);
      assert( mod->lines );
    }
  else
    {
      if( mod->lines ) free( mod->lines ); mod->lines = NULL;
    }
  
  mod->lines_count = lines_count;
  memcpy( mod->lines, lines, len );

  if( mod->polypoints )
    { 
      free( mod->polypoints ); 
      mod->polypoints = NULL; 
    }

  if( mod->lines_count > 1 )
    {
      // TODO - make multiple polygons from sets of lines
      // or extend models to have polygons AND lines

      // attempt to make a polygon from the lines
      int is_polygon = 1;
      
      // a valid polygon has lines that join
      int p;
      for( p=1; p<mod->lines_count; p++ )
	{
	  if( mod->lines[p].x1 != mod->lines[p-1].x2 ||
	      mod->lines[p].y1 != mod->lines[p-1].y2 )
	    {
	      is_polygon = 0;
	      break;
	    }
	}
      
      if( is_polygon )
	{
	  mod->polypoints = malloc( sizeof(double) * 2 * mod->lines_count );
	  
	  for( p=0; p<mod->lines_count; p++ )
	  {
	    mod->polypoints[2*p  ] = mod->lines[p].x1;
	    mod->polypoints[2*p+1] = mod->lines[p].y1;
	  }

	  //PRINT_WARN2( "model %s has a valid polygon of %d points",
	  //       mod->token, mod->lines_count );
	}  
    }
    
  // redraw my image 
  stg_model_map( mod, 1 );

  model_render_lines( mod );
 
  return 0; // OK
}

int stg_model_set_laserreturn( stg_model_t* mod, stg_laser_return_t* val )
{
  mod->laser_return = *val;
  return 0;
}


stg_velocity_t* stg_model_get_velocity( stg_model_t* mod )
{
  return &mod->velocity;
}

int stg_model_set_velocity( stg_model_t* mod, stg_velocity_t* vel )
{
  memcpy( &mod->velocity, vel, sizeof(mod->velocity) );
  return 0;
}


stg_color_t stg_model_get_color( stg_model_t* mod )
{
  return mod->color;
}


int stg_model_set_color( stg_model_t* mod, stg_color_t* col )
{
  memcpy( &mod->color, col, sizeof(mod->color) );

  // redraw my image
  model_render_lines(mod);

  return 0; // OK
}

stg_kg_t* stg_model_get_mass( stg_model_t* mod )
{
  return &mod->mass;
}

int stg_model_set_mass( stg_model_t* mod, stg_kg_t* mass )
{
  memcpy( &mod->mass, mass, sizeof(mod->mass) );
  return 0;
}

stg_guifeatures_t* stg_model_get_guifeatures( stg_model_t* mod )
{
  return &mod->guifeatures;
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


stg_fiducial_return_t* stg_model_get_fiducialreturn( stg_model_t* mod )
{
  return &mod->fiducial_return;
}

int stg_model_set_fiducialreturn( stg_model_t* mod, stg_fiducial_return_t* val )     
{
  memcpy( &mod->fiducial_return, val, sizeof(mod->fiducial_return) );
  return 0;
}

