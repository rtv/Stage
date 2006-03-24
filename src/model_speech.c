///////////////////////////////////////////////////////////////////////////
//
// File: model_speech.cc
// Authors: Pooya Karimian
// Date: 16 March 2006
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/model_speech.c,v $
//  $Author: pooya $
//  $Revision: 1.1 $
//
///////////////////////////////////////////////////////////////////////////

/**
@ingroup model
@defgroup model_speech Speech model 

@todo revise
The speech model draws a speech bubble on the robot whenever 
a robot say a message. The bubble will go away with an empty message.
This can be used for debugging purposes.

<h2>Worldfile properties</h2>

@todo Fill this in

*/


//#include <sys/time.h>
#include <math.h>
#include "gui.h"

//#define DEBUG

#include "stage_internal.h"

// standard callbacks
int speech_update( stg_model_t* mod );
int speech_startup( stg_model_t* mod );
int speech_shutdown( stg_model_t* mod );
void speech_load( stg_model_t* mod );

int speech_render_data( stg_model_t* mod, void* userp );
int speech_render_cfg( stg_model_t* mod, void* userp );
int speech_unrender_data( stg_model_t* mod, void* userp );
int speech_unrender_cfg( stg_model_t* mod, void* userp );

void speech_load( stg_model_t* mod )
{

  // TODO: read speech params from the world file
  //  stg_model_set_cfg( mod, &cfg, sizeof(cfg));
}

int 
speech_init( stg_model_t* mod )
{
  // we don't consume any power until subscribed
  // mod->watts = 0.0; 
  
  // override the default methods; these will be called by the simualtion
  // engine
  mod->f_startup = speech_startup;
  mod->f_shutdown = speech_shutdown;
  mod->f_update =  speech_update;
  mod->f_load = speech_load;

  // sensible speech defaults; it doesn't take up any physical space
  stg_geom_t geom;
  geom.pose.x = 0.0;
  geom.pose.y = 0.0;
  geom.pose.a = 0.0;
  geom.size.x = 0.0;
  geom.size.y = 0.0;
  stg_model_set_geom( mod, &geom );
  
  // speech is invisible
  stg_model_set_obstacle_return( mod, 0 );
  stg_model_set_laser_return( mod, LaserTransparent );
  stg_model_set_blob_return( mod, 0 );
  stg_model_set_color( mod, (stg_color_t)0 );

  stg_speech_config_t spconf;
  memset(&spconf,0,sizeof(spconf));

  stg_model_set_cfg( mod, &spconf, sizeof(spconf) );

  // sensible starting command
  stg_speech_cmd_t cmd; 
  cmd.cmd = STG_SPEECH_CMD_NOP;
  cmd.string[0] = 0;
  stg_model_set_cmd( mod, &cmd, sizeof(cmd) ) ;

  stg_speech_data_t data;
  memset(&data,0,sizeof(data));
  stg_model_set_data( mod, &data, sizeof(data));

  // adds a menu item and associated on-and-off callbacks
//  stg_model_add_property_toggles( mod, &mod->cmd,
  stg_model_add_property_toggles( mod, &mod->data,
				  speech_render_data, NULL,
				  speech_unrender_data, NULL,
				  "speech_data",
				  "speech data",
				  TRUE );

  return 0;
}

int speech_update( stg_model_t* mod )
{   
  // no work to do if we're unsubscribed
  if( mod->subs < 1 )
    return 0;

  // Retrieve current configuration
  stg_speech_config_t cfg;
  memcpy(&cfg, mod->cfg, sizeof(cfg));

  // Retrieve current geometry
  stg_geom_t geom;
  stg_model_get_geom( mod, &geom );

  stg_speech_cmd_t* cmd = (stg_speech_cmd_t*)mod->cmd;
  
  switch (cmd->cmd) 
  {
    case 0: //NOP
      break; 

    case STG_SPEECH_CMD_SAY:
      strcpy(cfg.string,cmd->string);
      break;
      
    default:
      printf("unknown speech command %d\n",cmd->cmd);
  }
  
  // get the sensor's pose in global coords
  stg_pose_t pz;
  memcpy( &pz, &geom.pose, sizeof(pz) ); 
  stg_model_local_to_global( mod, &pz );

 // change the speech's configuration in response to the commands
  stg_model_set_cfg( mod, &cfg, sizeof(stg_speech_config_t));

  // also publish the data
  stg_speech_data_t data;
  strcpy(data.string,cfg.string);

  // publish the new stuff
  stg_model_set_data( mod, &data, sizeof(data));
  stg_model_set_cfg( mod,  &cfg, sizeof(cfg));

  // inherit standard update behaviour
  _model_update( mod );

  return 0; //ok
}

int speech_render_data(  stg_model_t* mod, void* userp )
{
//  puts( "speech render data" );

  // only draw if someone is using the speech
  if( mod->subs < 1 )
    return 0;

  stg_rtk_fig_t* fig = stg_model_get_fig( mod, "speech_data_fig" );
  
  if( ! fig ) {
    fig = stg_model_fig_create( mod, "speech_data_fig", "top", STG_LAYER_SPEECHDATA );
    stg_rtk_fig_color_rgb32( fig, stg_lookup_color( STG_SPEECH_COLOR ) );
  }
  else 
    stg_rtk_fig_clear( fig );

  stg_speech_data_t* data = (stg_speech_data_t*)mod->data;
  assert(data);
  
  stg_speech_config_t *cfg = (stg_speech_config_t*)mod->cfg;
  assert(cfg);

//  stg_geom_t *geom = &mod->geom;  

  // only draw if the string is not empty
  if ( data->string[0]==0 ) 
    return 0;
  
  stg_rtk_fig_text_bubble( fig, 0.0, 0.0, 0, data->string, 0.15, 0.15);

  return 0; 
}

int speech_render_cfg(  stg_model_t* mod, void* userp )
{
  puts( "speech render cfg" );


  return 0; 
}

int speech_startup( stg_model_t* mod )
{ 
  PRINT_DEBUG( "speech startup" );
/*
  stg_model_set_watts( mod, STG_WIFI_WATTS );
*/  
  return 0; // ok
}

int speech_shutdown( stg_model_t* mod )
{
  PRINT_DEBUG( "speech shutdown" );
/*
  stg_model_set_watts( mod, 0 );
*/
  
  // unrender the data
  stg_model_fig_clear( mod, "speech_data_fig" );
  return 0; // ok
}

int speech_unrender_data( stg_model_t* mod, void* userp )
{
  // CLEAR STUFF THAT YOU DREW
  stg_model_fig_clear( mod, "speech_data_fig" );
  return 1; // callback just runs one time
}

int speech_unrender_cfg( stg_model_t* mod, void* userp )
{
//
  return 1; // callback just runs one time
}


