///////////////////////////////////////////////////////////////////////////
//
// File: model_wifi.cc
// Authors: Benoit Morisset, Richard Vaughan
// Date: 2 March 2006
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/libstage/model_wifi.cc,v $
//  $Author: rtv $
//  $Revision: 1.2 $
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
#include "gui.h"

// Number pulled directly from my ass
#define STG_WIFI_WATTS 2.5

//#define DEBUG

#include "stage_internal.h"

// defined in model.c
int _model_update( stg_model_t* mod, void* unused );

// standard callbacks
extern "C" {

	// declare functions used as callbacks
	static int wifi_update( stg_model_t* mod, void* unused );
	static int wifi_startup( stg_model_t* mod, void* unused );
	static int wifi_shutdown( stg_model_t* mod, void* unused );
	static int wifi_load( stg_model_t* mod, void* unused );

	// implented by the gui in some other file
	void gui_wifi_init( stg_model_t* mod );


	int 
		wifi_init( stg_model_t* mod )
		{
			// we don't consume any power until subscribed
			mod->watts = 0.0;

			// override the default methods; these will be called by the simualtion
			// engine
			stg_model_add_callback( mod, &mod->startup, wifi_startup, NULL );
			stg_model_add_callback( mod, &mod->shutdown, wifi_shutdown, NULL );
			stg_model_add_callback( mod, &mod->load, wifi_load, NULL );

			// sensible wifi defaults; it doesn't take up any physical space
			Geom geom;
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
		wifi_update( stg_model_t* mod, void* unused )
		{   
			// no work to do if we're unsubscribed
			if( mod->subs < 1 )
				return 0;

			puts("wifi_update");

			// Retrieve current configuration
			stg_wifi_config_t cfg;
			memcpy(&cfg, mod->cfg, sizeof(cfg));

			// Retrieve current geometry
			Geom geom;
			stg_model_get_geom( mod, &geom );

			// get the sensor's pose in global coords
			Pose pz;
			memcpy( &pz, &geom.pose, sizeof(pz) );
			stg_model_local_to_global( mod, &pz );


			// We'll store current data here
			stg_wifi_data_t data;

			// DO RADIO PROPAGATION HERE, and fill in the data structure

			// publish the new stuff
			stg_model_set_data( mod, &data, sizeof(data));

			// inherit standard update behaviour
			_model_update( mod, NULL );

			return 0; //ok
		}

	int 
		wifi_startup( stg_model_t* mod, void* unused )
		{ 
			PRINT_DEBUG( "wifi startup" );
			stg_model_add_callback( mod, &mod->startup, wifi_startup, NULL );
			stg_model_set_watts( mod, STG_WIFI_WATTS );
			return 0; // ok
		}

	int 
		wifi_shutdown( stg_model_t* mod, void* unused )
		{
			PRINT_DEBUG( "wifi shutdown" );
			stg_model_remove_callback( mod, &mod->startup, wifi_startup );
			stg_model_set_watts( mod, 0 );
			return 0; // ok
		}

	int
		wifi_load( stg_model_t* mod, void* unused )
		{
			stg_wifi_config_t cfg;

			// TODO: read wifi params from the world file

			stg_model_set_cfg( mod, &cfg, sizeof(cfg));

			return 0;
		}

} // ends extern "C"
