///////////////////////////////////////////////////////////////////////////
//
// File: model_wifi.cc
// Authors: Benoit Morisset
// Date: 2 March 2006
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/model_wifi.cc,v $
//  $Author: rtv $
//  $Revision: 1.1.4.1 $
//
///////////////////////////////////////////////////////////////////////////

/**
@ingroup model
@defgroup model_wifi Wifi model 

@todo Fill this in

<h2>Worldfile properties</h2>

@todo Fill this in

*/


#include <sys/time.h>
#include <math.h>
#include "gui.h"

// Number pulled directly from my ass
#define STG_WIFI_WATTS 2.5

//#define DEBUG

#include "stage_internal.h"

// standard callbacks
extern "C" {

// declare functions used as callbacks
int wifi_update( stg_model_t* mod );
int wifi_startup( stg_model_t* mod );
int wifi_shutdown( stg_model_t* mod );
void wifi_load( stg_model_t* mod );

// implented by the gui in some other file
void gui_wifi_init( stg_model_t* mod );
  
void 
wifi_load( stg_model_t* mod )
{
  stg_wifi_config_t cfg;

  // TODO: read wifi params from the world file
  
  stg_model_set_cfg( mod, &cfg, sizeof(cfg));
}

int 
wifi_init( stg_model_t* mod )
{
  // we don't consume any power until subscribed
  mod->watts = 0.0; 
  
  // override the default methods; these will be called by the simualtion
  // engine
  mod->f_startup = wifi_startup;
  mod->f_shutdown = wifi_shutdown;
  mod->f_update =  wifi_update;
  mod->f_load = wifi_load;

  // sensible wifi defaults; it doesn't take up any physical space
  stg_geom_t geom;
  geom.pose.x = 0.0;
  geom.pose.y = 0.0;
  geom.pose.a = 0.0;
  geom.size.x = 0.0;
  geom.size.y = 0.0;
  stg_model_set_geom( mod, &geom );
  
  // wifi is invisible
  stg_model_set_obstacle_return( mod, 0 );
  stg_model_set_laser_return( mod, LaserTransparent );
  stg_model_set_blob_return( mod, 0 );
  stg_model_set_color( mod, (stg_color_t)0 );

  gui_wifi_init(mod);

  return 0;
}

int 
wifi_update( stg_model_t* mod )
{   
  // no work to do if we're unsubscribed
  if( mod->subs < 1 )
    return 0;

  puts("wifi_update");
    
  // Retrieve current configuration
  stg_wifi_config_t cfg;
  memcpy(&cfg, mod->cfg, sizeof(cfg));

  // Retrieve current geometry
  stg_geom_t geom;
  stg_model_get_geom( mod, &geom );
  
  // get the sensor's pose in global coords
  stg_pose_t pz;
  memcpy( &pz, &geom.pose, sizeof(pz) ); 
  stg_model_local_to_global( mod, &pz );

  
  // We'll store current data here
  stg_wifi_data_t data;

  // DO RADIO PROPAGATION HERE, and fill in the data structure

  // publish the new stuff
  stg_model_set_data( mod, &data, sizeof(data));

  // inherit standard update behaviour
  _model_update( mod );

  return 0; //ok
}

int 
wifi_startup( stg_model_t* mod )
{ 
  PRINT_DEBUG( "wifi startup" );
  stg_model_set_watts( mod, STG_WIFI_WATTS );
  return 0; // ok
}

int 
wifi_shutdown( stg_model_t* mod )
{
  PRINT_DEBUG( "wifi shutdown" );
  stg_model_set_watts( mod, 0 );
  
  // unrender the data
  //stg_model_fig_clear( mod, "wifi_data_fig" );
  return 0; // ok
}


}
