
///////////////////////////////////////////////////////////////////////////
//
// File: model_energy.c
// Author: Richard Vaughan
// Date: 10 June 2004
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/model_energy.c,v $
//  $Author: rtv $
//  $Revision: 1.9 $
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

void  model_energy_config_render( model_t* mod );
void  model_energy_data_render( model_t* mod );

stg_energy_config_t* model_get_energy_config( model_t* mod )
{
  return &mod->energy_config;
}

stg_energy_data_t* model_get_energy_data( model_t* mod )
{
  return &mod->energy_data;
}

int model_set_energy_data( model_t* mod, stg_energy_data_t* data )
{  
  memcpy( &mod->energy_data, data, sizeof(mod->energy_data));
  
  // and redraw it
  model_energy_data_render( mod );

  return 0; //OK
}

int model_set_energy_config( model_t* mod, stg_energy_config_t* config )
{  
  memcpy( &mod->energy_config, config, sizeof(mod->energy_config));
  
  // and redraw it
  model_energy_config_render( mod );

  return 0; //OK
}
 

void model_energy_consume( model_t* mod, stg_watts_t rate )
{
  stg_energy_data_t* data = model_get_energy_data(mod);
  stg_energy_config_t* cfg = model_get_energy_config(mod);
  
  if( data && cfg && cfg->capacity > 0 )
    {
      data->joules -=  rate * (double)mod->world->sim_interval / 1000.0;
      data->joules = MAX( data->joules, 0 ); // not too little
      data->joules = MIN( data->joules, cfg->capacity );// not too much
    }
}

void model_energy_absorb( model_t* mod, stg_watts_t rate )
{
  stg_energy_data_t* data = model_get_energy_data(mod);
  stg_energy_config_t* cfg = model_get_energy_config(mod);

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
  
  stg_energy_config_t* cfg = model_get_energy_config(mod);
  stg_energy_data_t* data = model_get_energy_data(mod);

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
	      stg_energy_config_t* hiscfg =  model_get_energy_config(him);
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

  //model_set_prop( mod, STG_PROP_ENERGYDATA, &nrg, sizeof(nrg) );

  return 0; // ok
}


void model_energy_data_render( model_t* mod )
{
  PRINT_DEBUG( "energy data render" );

  
  if( mod->gui.data  )
    rtk_fig_clear(mod->gui.data);
  else // create the figure, store it in the model and keep a local pointer
    {
      mod->gui.data = rtk_fig_create( mod->world->win->canvas, 
				      mod->gui.top, STG_LAYER_ENERGYDATA );
      
      rtk_fig_color_rgb32(mod->gui.data, stg_lookup_color(STG_ENERGY_COLOR) );
    }
  
  rtk_fig_t* fig = mod->gui.data;  
  
  // if this model has a energy subscription
  // and some data, we'll draw the data
  if(  mod->subs )
    {
      // place the visualization a little away from the device
      //stg_pose_t pose;
      //model_global_pose( mod, &pose );
  
      //pose.x += 0.0;
      //pose.y -= 0.5;
      //pose.a = 0.0;
      //rtk_fig_origin( fig, pose.x, pose.y, pose.a );

      stg_energy_data_t* data = model_get_energy_data(mod);
      stg_energy_config_t* cfg = model_get_energy_config(mod);
      
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
  
  
  if( mod->gui.cfg  )
    rtk_fig_clear(mod->gui.cfg);
  else // create the figure, store it in the model and keep a local pointer
    {
      mod->gui.cfg = rtk_fig_create( mod->world->win->canvas, 
				   mod->gui.top, STG_LAYER_ENERGYCONFIG );

      rtk_fig_color_rgb32( mod->gui.cfg, stg_lookup_color( STG_ENERGY_CFG_COLOR ));
    }

  stg_energy_config_t* cfg = model_get_energy_config(mod);
  
  if( cfg && cfg->give_rate > 0 )
    {  
      // draw the charging radius
      //double radius = cfg->probe_range;
      
      //rtk_fig_ellipse( fig,0,0,0, 2.0*radius, 2.0*radius, 0 );

      rtk_fig_arrow( mod->gui.cfg, 0,0,0, cfg->probe_range, 0.10 );
    }
}


