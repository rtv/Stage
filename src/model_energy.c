///////////////////////////////////////////////////////////////////////////
//
// File: model_energy.c
// Author: Richard Vaughan
// Date: 10 June 2004
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/model_energy.c,v $
//  $Author: rtv $
//  $Revision: 1.4 $
//
///////////////////////////////////////////////////////////////////////////

#include <sys/time.h>
#include <math.h>

//#define DEBUG

#include "stage.h"
#include "raytrace.h"
#include "model.h"

#include "gui.h"
extern rtk_fig_t* fig_debug;

#define TIMING 0
#define ENERGY_FILLED 1

void model_energy_config_init( model_t* mod );
int model_energy_config_set( model_t* mod, void* config, size_t len );

void model_energy_data_init( model_t* mod );
int model_energy_data_shutdown( model_t* mod );
int model_energy_data_service( model_t* mod );
int model_energy_data_set( model_t* mod, void* data, size_t len );

void model_energy_data_render( model_t* mod );
void model_energy_config_render( model_t* mod );

void model_energy_register(void)
{ 
  PRINT_DEBUG( "ENERGY INIT" );  
  
  model_register_init( STG_PROP_ENERGYCONFIG, model_energy_config_init );
  model_register_set( STG_PROP_ENERGYCONFIG, model_energy_config_set );

  model_register_init( STG_PROP_ENERGYDATA, model_energy_data_init );
  model_register_set( STG_PROP_ENERGYDATA, model_energy_data_set );
  model_register_shutdown( STG_PROP_ENERGYDATA, model_energy_data_shutdown );
  model_register_service( STG_PROP_ENERGYDATA, model_energy_data_service );
}

stg_energy_config_t* model_energy_config_get( model_t* mod )
{
  stg_energy_config_t* cfg = 
    model_get_prop_data_generic( mod, STG_PROP_ENERGYCONFIG );
  assert(cfg);
  return cfg;
}

stg_energy_data_t* model_energy_data_get( model_t* mod )
{
  stg_energy_data_t* data = (stg_energy_data_t*)
    model_get_prop_data_generic( mod, STG_PROP_ENERGYDATA );
  assert(data);
  return data;
}

void model_energy_config_init( model_t* mod )
{
  stg_energy_config_t cfg;
  memset(&cfg,0,sizeof(cfg));

  cfg.capacity = STG_DEFAULT_ENERGY_CAPACITY;
  cfg.give_rate = STG_DEFAULT_ENERGY_GIVERATE;
  cfg.probe_range = STG_DEFAULT_ENERGY_PROBERANGE;      
  cfg.trickle_rate = STG_DEFAULT_ENERGY_TRICKLERATE;

  model_set_prop( mod, STG_PROP_ENERGYCONFIG, &cfg,sizeof(cfg) );
}

void model_energy_data_init( model_t* mod )
{
  stg_energy_config_t* cfg = model_energy_config_get(mod);
  
  stg_energy_data_t data;
  memset(&data,0,sizeof(data));

  data.joules = cfg->capacity;
  data.watts = 0;
  data.charging = FALSE;
  data.range = cfg->probe_range;
  
  model_set_prop( mod, STG_PROP_ENERGYDATA, &data,sizeof(data) );
}

int model_energy_data_set( model_t* mod, void* data, size_t len )
{  
  // store the data
  model_set_prop_generic( mod, STG_PROP_ENERGYDATA, data, len );
  
  // and redraw it
  model_energy_data_render( mod );

  return 0; //OK
}
 
int model_energy_config_set( model_t* mod, void* config, size_t len )
{  
  // store the config
  model_set_prop_generic( mod, STG_PROP_ENERGYCONFIG, config, len );
  
  // TODO - need to think about this a little. I'll leave it for
  // now. - rtv set our actual energy level to be the same as the
  // capacity //stg_energy_data_t* data = model_energy_data_get( mod
  // ); //data->joules = ((stg_energy_config_t*)config)->capacity;
  // //model_set_prop( mod, STG_PROP_ENERGYDATA,
  // data,sizeof(stg_energy_data_t) );

  // and redraw it
  model_energy_config_render( mod);

  return 0; //OK
}


int model_energy_data_shutdown( model_t* mod )
{
  PRINT_DEBUG( "energy shutdown" );  
  // and redraw it
  model_energy_data_render( mod );
  return 0;
}


void model_energy_consume( model_t* mod, stg_watts_t rate )
{
  stg_energy_data_t* data = model_energy_data_get(mod);
  stg_energy_config_t* cfg = model_energy_config_get(mod);
  
  if( data && cfg && cfg->capacity > 0 )
    {
      data->joules -=  rate * (double)mod->world->sim_interval / 1000.0;
      data->joules = MAX( data->joules, 0 ); // not too little
      data->joules = MIN( data->joules, cfg->capacity );// not too much
    }
}

void model_energy_absorb( model_t* mod, stg_watts_t rate )
{
  stg_energy_data_t* data = model_energy_data_get(mod);
  stg_energy_config_t* cfg = model_energy_config_get(mod);

  if( data && cfg && cfg->capacity > 0 )
    {
      data->joules +=  rate * (double)mod->world->sim_interval / 1000.0;

      //printf( "joules: %.2f max: %.2f\n", data->joules, cfg->capacity );

      data->joules = MIN( data->joules, cfg->capacity );

    }
}


int model_energy_data_service( model_t* mod )
{     
  PRINT_DEBUG1( "energy service model %d", mod->id  );  
  
  stg_energy_config_t* cfg = model_energy_config_get(mod);
  stg_energy_data_t* data = model_energy_data_get(mod);

  data->charging = FALSE;

  if( cfg->probe_range > 0 )
    {
      stg_pose_t pose;
      model_global_pose( mod, &pose );
      
      itl_t* itl = itl_create( pose.x, pose.y, pose.a, cfg->probe_range, 
			       mod->world->matrix, 
			       PointToBearingRange );
      
      model_t* him;
      while( (him = itl_next( itl )) ) 
	{
	  // if we hit something
	  if( him != mod )
	    {
	      stg_energy_config_t* hiscfg =  model_energy_config_get(him);
	      //stg_energy_data_t* hisdata =  model_energy_data_get(him);
	      
	      //printf( "inspecting mod %d to see if he will give me juice\n",
	      //      him->id );
	      
	      if( hiscfg->give_rate > 0 && hiscfg->capacity != 0 )
		{
		  //puts( "TRADING ENERGY" );
		  
		  model_energy_absorb( mod, hiscfg->give_rate );		  
		  model_energy_consume( him, hiscfg->give_rate );

		  data->charging = TRUE;
		  data->range = itl->range;
		}

	      break; // we only charge the first energy-using thing we hit
	    }
	}      
    }

  model_energy_consume( mod, cfg->trickle_rate );

  // re-publish our data (so it gets rendered on the screen)
  stg_energy_data_t nrg;
  memcpy(&nrg,data,sizeof(nrg));

  model_set_prop( mod, STG_PROP_ENERGYDATA, &nrg, sizeof(nrg) );

  return 0; // ok
}


void model_energy_data_render( model_t* mod )
{
  PRINT_DEBUG( "energy data render" );

  rtk_fig_t* fig = mod->gui.propdata[STG_PROP_ENERGYDATA];  
  
  if( fig  )
    rtk_fig_clear(fig);
  else // create the figure, store it in the model and keep a local pointer
    {
      fig = model_prop_fig_create( mod, mod->gui.propdata, STG_PROP_ENERGYDATA,
				   mod->gui.top, STG_LAYER_ENERGYDATA );
      rtk_fig_color_rgb32(fig, stg_lookup_color(STG_ENERGY_COLOR) );
    }
  
  // if this model has a energy subscription
  // and some data, we'll draw the data
  if(  mod->subs[STG_PROP_ENERGYDATA] )
    {
      // place the visualization a little away from the device
      //stg_pose_t pose;
      //model_global_pose( mod, &pose );
  
      //pose.x += 0.0;
      //pose.y -= 0.5;
      //pose.a = 0.0;
      //rtk_fig_origin( fig, pose.x, pose.y, pose.a );

      stg_energy_data_t* data = model_energy_data_get(mod);
      stg_energy_config_t* cfg = model_energy_config_get(mod);
      
      /*  char buf[256];
      snprintf( buf, 256, "%.2f/%.2fJ %.2fW %s", 
		data->joules, 
		cfg->capacity, 
		data->watts,
		data->charging ? "charging" : "" );
      */

      
      if( data->charging ) 
	rtk_fig_arrow( fig, 0,0,0, data->range, 0.3 );

      if( cfg->capacity > 0 )
	{
	  char buf[64];
	  snprintf( buf, 32, "%.2f %s", 
		    data->joules, 
		    data->charging ? "charging" : "" );
	  
	  rtk_fig_text( fig, 0.0,0.0,0, buf ); 
	}
      else
	{
	  char buf[64];
	  snprintf( buf, 32, "supplying %.2f Watts", 
		    cfg->give_rate );
	  
	  rtk_fig_text( fig, 0,0,0, buf ); 	  
	}

    }
}




void model_energy_config_render( model_t* mod )
{ 
  PRINT_DEBUG( "energy config render" );
  
  rtk_fig_t* fig = mod->gui.propdata[STG_PROP_ENERGYCONFIG];  
  
  if( fig  )
    rtk_fig_clear(fig);
  else // create the figure, store it in the model and keep a local pointer
    {
      fig = model_prop_fig_create( mod, mod->gui.propdata, STG_PROP_ENERGYCONFIG,
				   mod->gui.top, STG_LAYER_ENERGYCONFIG );
      rtk_fig_color_rgb32( fig, stg_lookup_color( STG_ENERGY_CFG_COLOR ));
    }
  
  stg_energy_config_t* cfg = model_energy_config_get(mod);
  
  if( cfg && cfg->give_rate > 0 )
    {  
      // draw the charging radius
      //double radius = cfg->probe_range;
      
      //rtk_fig_ellipse( fig,0,0,0, 2.0*radius, 2.0*radius, 0 );

      rtk_fig_arrow( fig, 0,0,0, cfg->probe_range, 0.10 );
    }
}


