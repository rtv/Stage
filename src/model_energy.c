
///////////////////////////////////////////////////////////////////////////
//
// File: model_energy.c
// Author: Richard Vaughan
// Date: 10 June 2004
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/model_energy.c,v $
//  $Author: rtv $
//  $Revision: 1.14 $
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

const double STG_ENERGY_PROBE_RANGE_DEFAULT = 1.0;
const double STG_ENERGY_GIVE_DEFAULT = 0.0;
const double STG_ENERGY_TAKE_DEFAULT = 100.0;
const double STG_ENERGY_CAPACITY_DEFAULT = 10000.0;
const double STG_ENERGY_SIZEX_DEFAULT = 0.08;
const double STG_ENERGY_SIZEY_DEFAULT = 0.18;
const double STG_ENERGY_POSEX_DEFAULT = 0.0;
const double STG_ENERGY_POSEY_DEFAULT = 0.0;
const double STG_ENERGY_POSEA_DEFAULT = 0.0;
const int STG_ENERGY_FIDUCIAL_DEFAULT = 255;

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

  // batteries aren't obstacles or sensible to range sensors
  mod->obstacle_return = FALSE;
  //mod->ranger_return = FALSE; // there is no ranger return! why?
  mod->laser_return = LaserTransparent;
  mod->fiducial_return = STG_ENERGY_FIDUCIAL_DEFAULT;

  mod->watts = 10.0;

  // sensible energy defaults
  stg_geom_t geom;
  geom.pose.x = STG_ENERGY_POSEX_DEFAULT;
  geom.pose.y = STG_ENERGY_POSEY_DEFAULT;
  geom.pose.a = STG_ENERGY_POSEA_DEFAULT;
  geom.size.x = STG_ENERGY_SIZEX_DEFAULT;
  geom.size.y = STG_ENERGY_SIZEY_DEFAULT;
  stg_model_set_geom( mod, &geom );

  // set up config structure
  stg_energy_config_t econf;
  memset(&econf,0,sizeof(econf));  
  econf.probe_range = STG_ENERGY_PROBE_RANGE_DEFAULT;
  econf.give = STG_ENERGY_GIVE_DEFAULT;
  econf.take = STG_ENERGY_TAKE_DEFAULT;
  econf.capacity = STG_ENERGY_CAPACITY_DEFAULT;
  stg_model_set_config( mod, &econf, sizeof(econf) );

  // set up initial data structure
  stg_energy_data_t data;
  memset(&data, 0, sizeof(data));
  data.stored = STG_ENERGY_CAPACITY_DEFAULT;
  data.charging = FALSE;
  data.output = 0.0;
  data.input = 0.0;
  data.range = STG_ENERGY_PROBE_RANGE_DEFAULT;
  stg_model_set_data( mod, (void*)&data, sizeof(data));

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

int stg_model_energy_available( stg_model_t* mod )
{
  if( mod->type == STG_MODEL_ENERGY )
    return 1;
  
  return 0;
}

int energy_match( stg_model_t* mod, stg_model_t* hitmod )
{
  // Ignore myself, my children, and my ancestors.
  if( (!stg_model_is_related(mod,hitmod))  &&  
      stg_model_energy_available( hitmod ) )
    return 1;
  
  return 0; // no match
}	



int energy_update( stg_model_t* mod )
{     
  PRINT_DEBUG1( "energy service model %d", mod->id  );  
  
  stg_energy_data_t data;
  assert( stg_model_get_data( mod, &data, sizeof(data) ) == sizeof(data));
  
  stg_energy_config_t cfg;
  assert( stg_model_get_config( mod, &cfg, sizeof(cfg) ) == sizeof(cfg) );

  data.charging = FALSE;
  data.input = 0.0;
  
  if( cfg.probe_range > 0 )
    {
      stg_pose_t pose;
      stg_model_get_global_pose( mod, &pose );
      
      itl_t* itl = itl_create( pose.x, pose.y, pose.a, cfg.probe_range, 
			       mod->world->matrix, 
			       PointToBearingRange );
      
      stg_model_t* hitmod = 
	itl_first_matching( itl, energy_match, mod );
      
      if( hitmod )
	{
	  data.range = itl->range;
	  data.charging = TRUE;
	  
	  if( data.stored < cfg.capacity )
	    {	  
	      stg_energy_config_t hiscfg;
	      assert( stg_model_get_config( hitmod, &hiscfg, sizeof(hiscfg)) == sizeof(hiscfg));
	      
	      double energy_taken = MIN( cfg.take, hiscfg.give );
	      energy_taken = MIN( energy_taken, cfg.capacity-data.stored);
	      data.input = energy_taken;
	    }
	  //printf( "hit a power source at range %.2f\n", data.range );
	}
    }

  // consume some energy
  data.output = mod->watts;
  
  // if we're plugged in, we gain energy
  if( data.charging )
    data.stored += data.input * mod->world->sim_interval/1000.0;
  else
    data.stored -= data.output * mod->world->sim_interval/1000.0;
  
  // re-publish our data (so it gets rendered on the screen)
  stg_model_set_data( mod, &data, sizeof(data));

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
  //if(  mod->subs )
  if(  1 )
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
      
      if( data->input > 0.0 ) 
	rtk_fig_arrow( fig, 0,0,0, data->range, 0.3 );

      if( cfg->capacity > 0 )
	{
	  char buf[128];
	  snprintf( buf, 128, "%.0f/%.0fJ (%.0f%%)\noutput %.2fW\ninput %.2fW\n%s", 
		    data->stored, 
		    cfg->capacity, 
		    data->stored/cfg->capacity * 100,
		    data->output,
		    data->input,
		    ( data->input > data->output ) ? "charging" : "" );
	  
	  rtk_fig_text( fig, 0.6,0.0,0, buf ); 
	}
      else
	{
	  char buf[128];
	  snprintf( buf, 128, "mains supply\noutput %.2fW\n", 
		    data->output );
	  
	  rtk_fig_text( fig, 0.6,0,0, buf ); 	  
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

  stg_energy_config_t cfg;
  stg_model_get_config(mod, &cfg, sizeof(cfg) );

  if( cfg.take > 0 )
    {  

      rtk_fig_arrow( mod->gui.cfg, 0,0,0, cfg.probe_range, 0.10 );
    }
}


