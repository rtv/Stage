//////////////////////////////////////////////////////////////////////////
//
// File: model_props.c
// Author: Richard Vaughan
// Date: 26 August 2004
// Desc: get and set functions for a model_t's properties
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/model_props.c,v $
//  $Author: rtv $
//  $Revision: 1.4 $
//
///////////////////////////////////////////////////////////////////////////

//#define DEBUG

#define SHOW_GEOM 0

#include "model.h"

extern lib_entry_t derived[];

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
  assert( mod );

  if( data == NULL )
    assert( len == 0 );
  
  switch( propid )
    {
      // the generic buffers for derived types
      //
    case STG_PROP_CONFIG:
      model_set_config( mod, data, len );
      break;
      
    case STG_PROP_DATA:
      model_set_data( mod, data, len );
      break;
      
    case STG_PROP_COMMAND:
      model_set_command( mod, data, len );
      break;
      
      // the basic properties
      //
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

    case STG_PROP_FIDUCIALRETURN:  
      if( len == sizeof(stg_fiducial_return_t) ) 
	model_set_fiducialreturn( mod, (stg_fiducial_return_t*)data );
      else
	model_size_error( mod, propid, len, sizeof(stg_fiducial_return_t) );
      break;
      
    case STG_PROP_LASERRETURN: 
      if( len == sizeof(stg_laser_return_t) ) 
	model_set_laserreturn( mod, *(stg_laser_return_t*)data ); 
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

    case STG_PROP_ENERGYCONFIG: 
      if( len == sizeof(stg_energy_config_t) ) 
	model_set_energy_config( mod, (stg_energy_config_t*)data ); 
      else
	model_size_error( mod, propid, len, sizeof(stg_energy_config_t) );      
      break;

    case STG_PROP_ENERGYDATA: 
      if( len == sizeof(stg_energy_data_t) ) 
	model_set_energy_data( mod, (stg_energy_data_t*)data ); 
      else
	model_size_error( mod, propid, len, sizeof(stg_energy_data_t) );      
      break;
      
      // etc!

    default:
      PRINT_WARN3( "unhandled property %d(%s) of size %d bytes",
		   propid, stg_property_string(propid), (int)len );
      break;
    }

  return 0; // OK
}

int model_get_prop( model_t* mod, stg_id_t pid, void** data, size_t* len )
{
  if( pid != STG_PROP_TIME )
    PRINT_DEBUG4( "GET PROP model %d(%s) prop %d(%s)", 
		  mod->id, mod->token, pid, stg_property_string(pid) );
  
  switch( pid )
    {
    case STG_PROP_TIME:
      *data = (void*)&mod->world->sim_time;
      *len = sizeof(mod->world->sim_time);
      break;
    case STG_PROP_GEOM: 
      *data = (void*)model_get_geom( mod );
      *len = sizeof(stg_geom_t);
      break;
    case STG_PROP_DATA: 
      *data = model_get_data( mod, len );
      break;
    case STG_PROP_COMMAND: 
      *data = model_get_command( mod, len );
      break;
    case STG_PROP_CONFIG: 
      *data = model_get_config( mod, len );
      break;      
      
      // TODO -  more props here
      // etc

    default:
      PRINT_WARN4( "model %d(%s) unhandled property get %d(%s)",
		   mod->id, mod->token, pid, stg_property_string(pid)); 
    }
  
  if( *len == 0 )
    PRINT_WARN3( "no data available for model %d property %d(%s)",
		 mod->id, pid, stg_property_string(pid) );

  if( pid != STG_PROP_TIME )
    PRINT_DEBUG1( "got a %d byte property", (int)*len);
  
  return 0; // ok
}


// DATA/COMMAND/CONFIG ----------------------------------------------------
// these set and get the generic buffers for derived types
//



int _set_data( model_t* mod, void* data, size_t len )
{
  mod->data = realloc( mod->data, len );
  memcpy( mod->data, data, len );    
  mod->data_len = len;
  
  PRINT_DEBUG3( "model %d(%s) put data of %d bytes",
		mod->id, mod->token, (int)mod->data_len);
  return 0; //ok
}

int _set_cmd( model_t* mod, void* cmd, size_t len )
{
  mod->cmd = realloc( mod->cmd, len );
  memcpy( mod->cmd, cmd, len );    
  mod->cmd_len = len;
  
  PRINT_DEBUG3( "model %d(%s) put command of %d bytes",
		mod->id, mod->token, (int)mod->cmd_len);
  return 0; //ok
}

int _set_cfg( model_t* mod, void* cfg, size_t len )
{
  mod->cfg = realloc( mod->cfg, len );
  memcpy( mod->cfg, cfg, len );    
  mod->cfg_len = len;
  
  PRINT_DEBUG3( "model %d(%s) put command of %d bytes",
		mod->id, mod->token, (int)mod->cfg_len);
  return 0; //ok
}

int model_set_data( model_t* mod, void* data, size_t len )
{
  // if this type of model has a set_data function, call it.
  if( derived[ mod->type ].set_data )
    {
      derived[ mod->type ].set_data(mod, data, len);
      PRINT_DEBUG1( "used special set_data returned %d bytes", (int)len );
    }
  else
    _set_data( mod, data, len );
  
  return 0; //ok
}

int model_set_command( model_t* mod, void* cmd, size_t len )
{
  // if this type of model has a putcommand function, call it.
  if( derived[ mod->type ].set_command )
    {
      derived[ mod->type ].set_command(mod, cmd, len);
      PRINT_DEBUG1( "used special set_command, put %d bytes", (int)len );
    }
  else
    _set_cmd( mod, cmd, len );    

  return 0; //ok
}

int model_set_config( model_t* mod, void* config, size_t len )
{
  // if this type of model has a putconfig function, call it.
  if( derived[ mod->type ].set_config )
    {
      derived[ mod->type ].set_config(mod, config, len);
      PRINT_DEBUG1( "used special putconfig returned %d bytes", (int)len );
    }
  else
    _set_cfg( mod, config, len );
  
  return 0; //ok
}


void* model_get_data( model_t* mod, size_t* len )
{
  void* data = NULL; 

  // if this type of model has a getdata function, call it.
  if( derived[ mod->type ].get_data )
    {
      data = derived[ mod->type ].get_data(mod, len);
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


void* model_get_command( model_t* mod, size_t* len )
{
  void* command = NULL;
  
  // if this type of model has a getcommand function, call it.
  if( derived[ mod->type ].get_command )
    {
      command = derived[ mod->type ].get_command(mod, len);
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


void* model_get_config( model_t* mod, size_t* len )
{
  void* config = NULL;
  
  // if this type of model has an getconfig function, call it.
  if( derived[ mod->type ].get_config )
    {
      config = derived[ mod->type ].get_config(mod, len);
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

stg_bool_t model_get_obstaclereturn( model_t* mod   )
{
  return mod->obstacle_return;
}

int model_set_obstaclereturn( model_t* mod, stg_bool_t val )
{
  mod->obstacle_return = val;
  return 0;
}

stg_pose_t* model_get_pose( model_t* model )
{
  //stg_pose_t* pose = model_get_prop_data_generic( model, STG_PROP_POSE );
  //assert(pose);
  return &model->pose;
}

int model_set_pose( model_t* mod, stg_pose_t* pose )
{
  // if the new pose is different
  if( memcmp( &mod->pose, pose, sizeof(stg_pose_t) ))
    {
      // unrender from the matrix
      model_map_with_children( mod, 0 );
      
      // copy the new pose
      memcpy( &mod->pose, pose, sizeof(mod->pose) );
      
      // render in the matrix
      model_map_with_children( mod, 1 );
      
      // move the rtk figure to match
      gui_render_pose( mod );
    }
  
  return 0; // OK
}


stg_geom_t* model_get_geom( model_t* mod )
{
  return &mod->geom;
}

int model_set_geom( model_t* mod, stg_geom_t* geom )
{ 
  
  // if the new geom is different
  if( memcmp( &mod->geom, geom, sizeof(stg_geom_t) ))
    {
      // unrender from the matrix
      model_map( mod, 0 );
      
      // store the geom
      memcpy( &mod->geom, geom, sizeof(mod->geom) );
      
      size_t count=0;
      stg_line_t* lines = model_get_lines( mod, &count );
      
      // force the body lines to fit inside this new rectangle
      stg_normalize_lines( lines, count );
      stg_scale_lines(  lines, count, geom->size.x, geom->size.y );
      stg_translate_lines( lines, count, -geom->size.x/2.0, -geom->size.y/2.0 );
      
      // set the new lines (this will cause redraw)
      model_set_lines( mod, lines, count );

#if SHOW_GEOM
      gui_render_geom( mod );
#endif
     
      // re-render int the matrix
      model_map( mod, 1 );
    }

  return 0;
}

stg_line_t* model_get_lines( model_t* mod, size_t* lines_count )
{
  
  *lines_count = mod->lines_count;
  return mod->lines;
}


int model_set_lines( model_t* mod, stg_line_t* lines, size_t lines_count )
{
  assert(mod);
  if( lines_count > 0 ) assert( lines );

  PRINT_DEBUG3( "model %d(%s) received %d lines", 
		mod->id, mod->token, (int)lines_count );
  
  model_map( mod, 0 );
  
  stg_geom_t* geom = model_get_geom(mod); 
  
  // fit the rectangle inside the model's size
  stg_normalize_lines( lines, lines_count );
  stg_scale_lines( lines, lines_count, geom->size.x, geom->size.y );
  stg_translate_lines( lines, lines_count, -geom->size.x/2.0, -geom->size.y/2.0 );
  
  size_t len = sizeof(stg_line_t)*lines_count;
  assert( mod->lines = realloc( mod->lines,len) );
  mod->lines_count = lines_count;
  memcpy( mod->lines, lines, len );

  // redraw my image 
  model_map( mod, 1 );

  model_render_lines( mod );
 
  return 0; // OK
}

int model_set_laserreturn( model_t* mod, stg_laser_return_t val )
{
  mod->laser_return = val;
  return 0;
}


stg_velocity_t* model_get_velocity( model_t* mod )
{
  return &mod->velocity;
}

int model_set_velocity( model_t* mod, stg_velocity_t* vel )
{
  memcpy( &mod->velocity, vel, sizeof(mod->velocity) );
  return 0;
}


stg_color_t model_get_color( model_t* mod )
{
  return mod->color;
}


int model_set_color( model_t* mod, stg_color_t* col )
{
  memcpy( &mod->color, col, sizeof(mod->color) );

  // redraw my image
  model_render_lines(mod);

  return 0; // OK
}

stg_kg_t* model_get_mass( model_t* mod )
{
  return &mod->mass;
}

int model_set_mass( model_t* mod, stg_kg_t* mass )
{
  memcpy( &mod->mass, mass, sizeof(mod->mass) );
  return 0;
}

stg_guifeatures_t* model_get_guifeatures( model_t* mod )
{
  return &mod->guifeatures;
}


int model_set_guifeatures( model_t* mod, stg_guifeatures_t* gf )
{
  memcpy( &mod->guifeatures, gf, sizeof(mod->guifeatures));
  
  // override the movemask flags - only top-level objects are moveable
  if( mod->parent )
    mod->guifeatures.movemask = 0;

  // redraw the fancy features
  gui_model_features( mod );

  return 0;
}


stg_fiducial_return_t* model_get_fiducialreturn( model_t* mod )
{
  return &mod->fiducial_return;
}

int model_set_fiducialreturn( model_t* mod, stg_fiducial_return_t* val )     
{
  PRINT_WARN1( "fid value set %d", *val );

  memcpy( &mod->fiducial_return, val, sizeof(mod->fiducial_return) );

  PRINT_WARN1( "fid value stored %d", mod->fiducial_return );
  return 0;
}

