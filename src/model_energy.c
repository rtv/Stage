
///////////////////////////////////////////////////////////////////////////
//
// File: model_energy.c
// Author: Richard Vaughan
// Date: 10 June 2004
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/model_energy.c,v $
//  $Author: rtv $
//  $Revision: 1.13 $
//
///////////////////////////////////////////////////////////////////////////

#include <sys/time.h>
#include <math.h>

//#define DEBUG

#include "stage_internal.h"
#include "gui.h"
extern rtk_fig_t* fig_debug;

#define TIMING 0
#define ENERGY_FILLED 1

void  energy_render_config( stg_model_t* mod );
void  energy_render_data( stg_model_t* mod );
int energy_update( stg_model_t* mod );
void energy_load( stg_model_t* mod );

stg_model_t* stg_energy_create( stg_world_t* world, 
			       stg_model_t* parent, 
			       stg_id_t id, 
			       char* token )
{
  stg_model_t* mod = 
    stg_model_create( world, parent, id, STG_MODEL_ENERGY, token );
  
  // override the default methods
  //mod->f_shutdown = energy_shutdown;
  mod->f_update = energy_update;
  mod->f_render_data = energy_render_data;
  mod->f_render_cfg = energy_render_config;
  mod->f_load = energy_load;

  // sensible energy defaults
  stg_geom_t geom;
  geom.pose.x = 0.0;
  geom.pose.y = 0.0;
  geom.pose.a = 0.0;
  geom.size.x = 0.2;
  geom.size.y = 0.2;
  stg_model_set_geom( mod, &geom );

  // set up config structure
  stg_energy_config_t econf;
  memset(&econf,0,sizeof(econf));  
  econf.probe_range = 1.0;
  econf.give = 0.0;
  econf.take = 100.0;
  econf.capacity = 10000.0;
  stg_model_set_config( mod, &econf, sizeof(econf) );

  // set default color
  stg_color_t col = stg_lookup_color( "yellow" ); 
  stg_model_set_color( mod, &col );

  return mod;
}

 
void energy_load( stg_model_t* mod )
{
  stg_energy_config_t econf;
  memset( &econf, 0, sizeof(econf) );
  
  econf.capacity = wf_read_float( mod->id, "capacity", 10000.0 );
  econf.probe_range = wf_read_length( mod->id, "probe_range", 1.0 );
  econf.give = wf_read_float( mod->id, "give", 0 );
  econf.take = wf_read_float( mod->id, "take", 100 );

  stg_model_set_config( mod, &econf, sizeof(econf));
}


void stg_model_energy_consume( stg_model_t* mod, stg_watts_t rate )
{
  stg_energy_data_t* data = (stg_energy_data_t*)(mod->data);
  stg_energy_config_t* cfg = (stg_energy_config_t*)(mod->cfg);
  
  if( data && cfg && cfg->capacity > 0 )
    {
      data->stored -=  rate * (double)mod->world->sim_interval / 1000.0;
      data->stored = MAX( data->stored, 0 ); // not too little
      data->stored = MIN( data->stored, cfg->capacity );// not too much
    }
}

void stg_model_energy_absorb( stg_model_t* mod, stg_watts_t rate )
{
  stg_energy_data_t* data = (stg_energy_data_t*)mod->data;
  stg_energy_config_t* cfg = (stg_energy_config_t*)mod->cfg;;

  if( data && cfg && cfg->capacity > 0 )
    {
      data->stored +=  rate * (double)mod->world->sim_interval / 1000.0;

      //printf( "joules: %.2f max: %.2f\n", data->stored, cfg->capacity );

      data->stored = MIN( data->stored, cfg->capacity );
    }
}


int energy_update( stg_model_t* mod )
{     
  PRINT_DEBUG1( "energy service model %d", mod->id  );  
  
  stg_energy_data_t* data = (stg_energy_data_t*)mod->data;
  stg_energy_config_t* cfg = (stg_energy_config_t*)mod->cfg;;

  data->charging = FALSE;

  if( cfg->probe_range > 0 )
    {
      stg_pose_t pose;
      stg_model_get_global_pose( mod, &pose );
      
      itl_t* itl = itl_create( pose.x, pose.y, pose.a, cfg->probe_range, 
			       mod->world->matrix, 
			       PointToBearingRange );
      
      stg_model_t* him;

      //hitmod = itl_first_matching( itl, energy_raytrace_match, mod );

      //if( hitmod )
      //range = itl->range;



/*       while( (him = itl_next( itl )) )  */
/* 	{ */
/* 	  // if we hit something */
/* 	  if( him != mod ) */
/* 	    { */
/* 	      stg_energy_config_t* hiscfg =  1.0;//stg_model_get_energy_config(him); */
/* 	      //stg_energy_data_t* hisdata =  model_energy_data_get(him); */
	      
/* 	      //printf( "inspecting mod %d to see if he will give me juice\n", */
/* 	      //      him->id ); */
	      
/* 	      if( hiscfg->give > 0 && hiscfg->capacity != 0 ) */
/* 		{ */
/* 		  //puts( "TRADING ENERGY" ); */
		  
/* 		  stg_model_energy_absorb( mod, hiscfg->give );		   */
/* 		  stg_model_energy_consume( him, hiscfg->give ); */

/* 		  data->charging = TRUE; */
/* 		  data->range = itl->range; */
/* 		} */

/* 	      break; // we only charge the first energy-using thing we hit */
/* 	    } */
/* 	}       */
    }

  //stg_model_energy_consume( mod, cfg->trickle_rate );

  // re-publish our data (so it gets rendered on the screen)
  stg_energy_data_t nrg;(stg_energy_data_t*)
  memcpy(&nrg,data,sizeof(nrg));

  //model_set_prop( mod, STG_PROP_ENERGYDATA, &nrg, sizeof(nrg) );

  return 0; // ok
}


void energy_render_data( stg_model_t* mod )
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
      //model_get_global_pose( mod, &pose );
  
      //pose.x += 0.0;
      //pose.y -= 0.5;
      //pose.a = 0.0;
      //rtk_fig_origin( fig, pose.x, pose.y, pose.a );

      stg_energy_data_t* data = (stg_energy_data_t*)mod->data;
      stg_energy_config_t* cfg = (stg_energy_config_t*)mod->cfg;;
      
      /*  char buf[256];
      snprintf( buf, 256, "%.2f/%.2fJ %.2fW %s", 
		data->stored, 
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
		    data->stored, 
		    data->charging ? "charging" : "" );
	  
	  rtk_fig_text( fig, 0.0,0.0,0, buf ); 
	}
      else
	{
	  char buf[64];
	  snprintf( buf, 32, "supplying %.2f Watts", 
		    cfg->give );
	  
	  rtk_fig_text( fig, 0,0,0, buf ); 	  
	}

    }
}


void energy_render_config( stg_model_t* mod )
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

  stg_energy_config_t* cfg = stg_model_get_energy_config(mod);
  
  if( cfg && cfg->give > 0 )
    {  
      // draw the charging radius
      //double radius = cfg->probe_range;
      
      //rtk_fig_ellipse( fig,0,0,0, 2.0*radius, 2.0*radius, 0 );

      rtk_fig_arrow( mod->gui.cfg, 0,0,0, cfg->probe_range, 0.10 );
    }
}


