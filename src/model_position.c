///////////////////////////////////////////////////////////////////////////
//
// File: model_laser.c
// Author: Richard Vaughan
// Date: 10 June 2004
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/model_position.c,v $
//  $Author: rtv $
//  $Revision: 1.1 $
//
///////////////////////////////////////////////////////////////////////////

#include <sys/time.h>
#include <math.h>

#define DEBUG

#include "model.h"
#include "gui.h"
extern rtk_fig_t* fig_debug;

  
void position_init( model_t* mod )
{
  PRINT_DEBUG( "position startup" );
  
  // sensible position defaults
  // TODO
  
  // init the command structure
  stg_position_cmd_t cmd;
  memset( &cmd, 0, sizeof(cmd));
  cmd.mode = STG_POSITION_CONTROL_VELOCITY; // vel control mode
  cmd.x = cmd.y = cmd.a = 0; // no speeds to start
  
  model_set_command( mod, &cmd, sizeof(cmd));
  
  // set up a laser-specific config structure
  stg_position_cfg_t cfg;
  memset(&cfg,0,sizeof(cfg));
  cfg.steer_mode =  STG_POSITION_STEER_DIFFERENTIAL;
  
  model_set_config( mod, &cfg, sizeof(cfg) );
}

int position_update( model_t* mod )
{   
  PRINT_DEBUG1( "[%lu] position update", mod->world->sim_time );

  assert( mod->cfg ); // position_init should have filled these
  assert( mod->cmd );
  
  // set the model's velocity here according to the position command

  stg_position_cmd_t* cmd = (stg_position_cmd_t*)mod->cmd;
  
  switch( cmd->mode )
    {
    case STG_POSITION_CONTROL_VELOCITY :
      {
	stg_velocity_t vel;
	vel.x = cmd->x;
	vel.y = cmd->y;
	vel.a = cmd->a;
	
	model_set_velocity( mod, &vel );
      } break;
      
    default:
      PRINT_ERR1( "unhandled position command mode %d", cmd->mode );
    }
  
  // set the data here
  stg_position_data_t data;
  
  // TODO - use an odometry estimate here
  memcpy( &data.pose, &mod->pose, sizeof(stg_pose_t));
  
  memcpy( &data.velocity, &mod->velocity, sizeof(stg_velocity_t));
  
  data.stall = mod->stall;
  
  // publish the data
  model_set_data( mod, &data, sizeof(data));
  
  return 0; //ok
}

int position_startup( model_t* mod )
{
  PRINT_DEBUG( "position startup" );
  return 0; // ok
}

int position_shutdown( model_t* mod )
{
  PRINT_DEBUG( "position shutdown" );
  
  // re-initialize the device - safety feature!
  position_init(mod);

  return 0; // ok
}


int register_position( void )
{
  register_init( STG_MODEL_POSITION, position_init );
  
  register_startup( STG_MODEL_POSITION, position_startup );
  register_shutdown( STG_MODEL_POSITION, position_shutdown );

  register_update( STG_MODEL_POSITION, position_update );

  return 0; //ok
} 
