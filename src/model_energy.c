#include <sys/time.h>
#include <math.h>

#include "stage.h"
#include "raytrace.h"


#include "gui.h"
extern rtk_fig_t* fig_debug;

#define TIMING 0
#define ENERGY_FILLED 1


void model_energy_shutdown( model_t* mod )
{
  //model_set_prop_generic( mod, STG_PROP_ENERGYDATA, NULL, 0 );
}

void model_energy_consume( model_t* mod, stg_watts_t rate )
{
  stg_energy_data_t* data = (stg_energy_data_t*)
    model_get_prop_data_generic( mod, STG_PROP_ENERGYDATA );

  stg_energy_config_t* cfg = (stg_energy_config_t*)
    model_get_prop_data_generic( mod, STG_PROP_ENERGYCONFIG );
  
  if( data && cfg && cfg->capacity > 0 )
    {
      data->joules -=  rate / (double)mod->world->sim_interval;
      data->joules = MAX( data->joules, 0 );
    }
}

void model_energy_absorb( model_t* mod, stg_watts_t rate )
{
  stg_energy_data_t* data = (stg_energy_data_t*)
    model_get_prop_data_generic( mod, STG_PROP_ENERGYDATA );
  
  stg_energy_config_t* cfg = (stg_energy_config_t*)
    model_get_prop_data_generic( mod, STG_PROP_ENERGYCONFIG );
  
  if( data && cfg && cfg->capacity > 0 )
    {
      data->joules +=  rate / (double)mod->world->sim_interval;
      data->joules = MIN( data->joules, cfg->capacity );
    }
}


void model_energy_update( model_t* mod )
{     
  PRINT_DEBUG( "energy update" );  

  stg_energy_config_t* cfg = (stg_energy_config_t*)
    model_get_prop_data_generic( mod, STG_PROP_ENERGYCONFIG );
  
  if( cfg == NULL )
    {
      PRINT_ERR( "no energy config available" );
      return;
    }
  
  stg_energy_data_t* data = (stg_energy_data_t*)
    model_get_prop_data_generic( mod, STG_PROP_ENERGYDATA );
  
  if( data == NULL )
    {
      PRINT_ERR( "no energy data available" );
      return;
    }
  
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
	      stg_energy_config_t* hiscfg = 
		model_get_prop_data_generic( him, STG_PROP_ENERGYCONFIG );

	      if( hiscfg == NULL )
		break;
	      
	      stg_energy_data_t* hisdata = 
		model_get_prop_data_generic( him, STG_PROP_ENERGYDATA );
	      
	      if( hisdata == NULL )
		break;
	      
	      printf( "inspecting mod %d to see if he will give me juice\n",
		      him->id );
	      
	      if( hiscfg->give_rate > 0 && hiscfg->capacity != 0 )
		{
		  puts( "TRADING ENERGY" );
		  
		  model_energy_absorb( mod, hiscfg->give_rate );		  
		  model_energy_consume( him, hiscfg->give_rate );

		  data->charging = TRUE;
		  data->range = itl->range;
		}

	      break; // we only charge the first energy-using thing we hit
	    }
	}      
    }

  model_energy_consume( mod, STG_ENERGY_COST_TRICKLE );

  // re-publish our data (so it gets rendered on the screen)
  stg_energy_data_t nrg;
  memcpy(&nrg,data,sizeof(nrg));

  model_set_prop_generic( mod, STG_PROP_ENERGYDATA, &nrg, sizeof(nrg) );
}


void model_energy_render( model_t* mod )
{
  PRINT_DEBUG( "energy render" );

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
      //rtk_fig_origin( fig, mod->geom.pose.x, mod->geom.pose.y, mod->geom.pose.a );  

      // place the visualization a little away from the device
      //stg_pose_t pose;
      //model_global_pose( mod, &pose );
  
      //pose.x += 0.0;
      //pose.y += 0.5;
      //pose.a = 0.0;
      //rtk_fig_origin( fig, pose.x, pose.y, pose.a );

      stg_energy_data_t* data = (stg_energy_data_t*)
	model_get_prop_data_generic( mod, STG_PROP_ENERGYDATA );
      
      if( data == NULL )
	{
	  PRINT_WARN( "no energy data available" );
	  return;
	}

      stg_energy_config_t* cfg = (stg_energy_config_t*)
	model_get_prop_data_generic( mod, STG_PROP_ENERGYCONFIG );
      
      if( cfg == NULL )
	{
	  PRINT_WARN( "no energy config available" );
	  return;
	}
      
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
	  
	  rtk_fig_text( fig, 0,0,0, buf ); 
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
  
  stg_energy_config_t* cfg = (stg_energy_config_t*) 
    model_get_prop_data_generic( mod, STG_PROP_ENERGYCONFIG );
  
  if( cfg && cfg->give_rate > 0 )
    {  
      // draw the charging radius
      //double radius = cfg->probe_range;
      
      //rtk_fig_ellipse( fig,0,0,0, 2.0*radius, 2.0*radius, 0 );

      rtk_fig_arrow( fig, 0,0,0, cfg->probe_range, 0.10 );
    }
}


