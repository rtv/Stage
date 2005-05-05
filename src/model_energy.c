
///////////////////////////////////////////////////////////////////////////
//
// File: model_energy.c
// Author: Richard Vaughan
// Date: 10 June 2004
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/model_energy.c,v $
//  $Author: rtv $
//  $Revision: 1.22 $
//
///////////////////////////////////////////////////////////////////////////

#include <sys/time.h>
#include <math.h>
#include <assert.h>

//#define DEBUG

#include "stage_internal.h"
#include "gui.h"
extern stg_rtk_fig_t* fig_debug;

#define TIMING 0
#define ENERGY_FILLED 1

void energy_render_config( stg_model_t* mod );
void energy_render_data( stg_model_t* mod );
int energy_startup( stg_model_t* mod );
int energy_shutdown( stg_model_t* mod );
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

typedef struct
{
  GPtrArray* connections;
} stg_model_energy_t;

stg_model_t* stg_energy_create( stg_world_t* world, 
			       stg_model_t* parent, 
			       stg_id_t id, 
			       char* token )
{
  stg_model_t* mod = 
    stg_model_create( world, parent, id, STG_MODEL_ENERGY, token );
  
  // add a property to this model
  GPtrArray* a = NULL;
  assert( (a = g_ptr_array_new()) );
  stg_model_set_prop( mod, "connections", a );
  
  double* d = calloc( sizeof(double), 1 );
  *d = 0.0;
  stg_model_set_prop( mod, "inputwatts", d );
  
  // override the default methods
  mod->f_startup = energy_startup;
  mod->f_shutdown = energy_shutdown;
  mod->f_update = energy_update;
  mod->f_render_data = energy_render_data;
  mod->f_render_cfg = energy_render_config;
  mod->f_load = energy_load;

  // batteries aren't obstacles or sensible to range sensors
  mod->obstacle_return = FALSE;
  mod->ranger_return = FALSE;
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
  data.output_joules = 0.0;
  data.input_joules = 0.0;
  data.output_watts = 0.0;
  data.input_watts = 0.0;
  data.range = STG_ENERGY_PROBE_RANGE_DEFAULT;
  stg_model_set_data( mod, (void*)&data, sizeof(data));

  // set default color
  stg_color_t col = stg_lookup_color( "orange" ); 
  stg_model_set_color( mod, &col );

  return mod;
}

int energy_startup( stg_model_t* mod )
{
  return 0; // ok
}

int energy_shutdown( stg_model_t* mod )
{
  return 0; // ok
}

// add sink to source's list of power connections
void energy_connect( stg_model_t* source, stg_model_t* sink )
{
  //printf( "connecting %s to %s\n", sink->token, source->token );

  GPtrArray* a = NULL;
  assert( (a = stg_model_get_prop( source, "connections" )) );
  g_ptr_array_add( a, sink );
}

// remove sink from source's list of power connections
void energy_disconnect( stg_model_t* source, stg_model_t* sink )
{
  //printf( "disconnecting %s to %s\n", sink->token, source->token );

  GPtrArray* a = NULL;
  assert( (a = stg_model_get_prop( source, "connections" )) );
  g_ptr_array_remove_fast( a, sink );
}


void energy_load( stg_model_t* mod )
{
  stg_energy_config_t cfg;
  stg_model_get_config( mod, &cfg, sizeof(cfg));
  
  cfg.capacity = wf_read_float( mod->id, "capacity", cfg.capacity );
  cfg.probe_range = wf_read_length( mod->id, "range", cfg.probe_range );
  cfg.give = wf_read_float( mod->id, "give", cfg.give );
  cfg.take = wf_read_float( mod->id, "take", cfg.take );

  stg_model_set_config( mod, &cfg, sizeof(cfg));

  // refill the tank with the new capacity - we start fully gassed up
  stg_energy_data_t data;
  assert( stg_model_get_data( mod, &data, sizeof(data) ) == sizeof(data));
  data.stored = cfg.capacity;
  stg_model_set_data( mod, &data, sizeof(data));
}

int energy_match( stg_model_t* mod, stg_model_t* hitmod )
{
  // Ignore myself, my children, and my ancestors.
  if( (!stg_model_is_related(mod,hitmod))  &&  
      hitmod->type == STG_MODEL_ENERGY )
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

  // CHARGING - am I connected to something for the next timestep?
  data.charging = FALSE;
  double joules_required = cfg.capacity - data.stored;
  
  if( joules_required > 0 && cfg.probe_range > 0 )
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
	  energy_connect( hitmod, mod );
	}
    }
  
  // INPUT
  // see how much power is being supplied to me by others this timestep.
  // They will have poked it into my accumulator property
  double* accum = NULL;
  assert( (accum = stg_model_get_prop( mod, "inputwatts" )));
  
  data.input_watts = *accum;

  double input_joules =  data.input_watts * mod->world->sim_interval/1000.0;
  data.input_joules += input_joules;
  data.stored += input_joules;
  
  // clear the accumulator
  *accum = 0.0;

  // OUTPUT
  
  // if I give output to others and I have some juice to give
  //if( cfg.give > 0  && data.stored > 0 )
    {
      // add up the power required by all connected devices
      stg_watts_t watts = 0.0;
      
      GPtrArray* a = NULL;
      assert( (a = stg_model_get_prop( mod, "connections" )) );
      

      //printf( "Energy model %s\n", mod->token );
      
      // add all the locally connected devices
      int added = stg_model_tree_to_ptr_array( stg_model_root(mod),
					       a );
      
      //  printf( "added %d models to connected list of %s\n",
      //  added, mod->token );
      
      // find the total current required, so I can do current limiting
      int i;
      for( i=0; i < a->len; i++ )
	{
	  stg_model_t* con = (stg_model_t*)g_ptr_array_index( a, i );
	  
	  if( con == mod ) // skip myself
	    continue;
	  
	  watts += con->watts;
	}
      
      //printf( "total watts required %.2f\n", watts ); 
      
      // todo - current limiting on this line
      //double watts_per_connection = watts;
      //double joules_per_connection = 
      //watts_per_connection * mod->world->sim_interval/1000.0;
      
      // feed all the connected devices
      for( i=0; i < a->len; i++ )
	{      
	  stg_model_t* con = (stg_model_t*)g_ptr_array_index( a, i );
	  
	  if( con == mod ) // skip myself
	    continue;
	  
	  //	  if( con->watts )
	  //printf( "%s -> %.2fW -> %s\n", 
	  //    mod->token,
	  //    con->watts,
	  //    con->token );

	  
	  // if the connected unit is an energy device, we poke some
	  // energy into it. if it's not an energy device, the energy just
	  // disappears into the entropic void
	  if( con->type == STG_MODEL_ENERGY)
	    {
	      // todo - current limiting
	      stg_energy_config_t* concfg= (stg_energy_config_t*)(con->cfg);
	      double watts = concfg->take;
	      
	      //stg_energy_data_t* condata = (stg_energy_data_t*)(con->data);	  
	      //double watts = con->watts;
	      
	      // dump the watts in the model's accumulator
	      double* accum = NULL;
	      assert( (accum = stg_model_get_prop( con, "inputwatts" )));
	      *accum += watts;
	      
	      //printf( " (stored %.2fW)", watts );
	    }
	  
	  //puts( "" );
	}
      
      // now disconnect everyone the fast way
      g_ptr_array_set_size( a, 0 );
      
      double joules = watts * mod->world->sim_interval/1000.0;
      joules = MIN( joules, data.stored );
      data.output_watts = watts;
      data.output_joules += joules;
      data.stored -= joules;      
    }
  
  // re-publish our data (so it gets rendered on the screen)
  stg_model_set_data( mod, &data, sizeof(data));

  return 0; // ok
}


void energy_render_data( stg_model_t* mod )
{
  PRINT_DEBUG( "energy data render" );
  
  if( mod->gui.data  )
    stg_rtk_fig_clear(mod->gui.data);
  else // create the figure, store it in the model and keep a local pointer
    {
      mod->gui.data = stg_rtk_fig_create( mod->world->win->canvas, 
				      mod->gui.top, 
				      //NULL,
				      STG_LAYER_ENERGYDATA );      
    }
  
  if( mod->gui.data_extra  )
    stg_rtk_fig_clear(mod->gui.data_extra);
  else // create the figure, store it in the model and keep a local pointer
    {
      mod->gui.data_extra = stg_rtk_fig_create( mod->world->win->canvas, 
					    NULL,
					    STG_LAYER_ENERGYDATA );      
    }
  
  //stg_rtk_fig_t* fig = mod->gui.data;  
  
  // if this model has a energy subscription
  // and some data, we'll draw the data
  //if(  mod->subs )
  if(  1 )
    {  

      // place the visualization a little away from the device
      stg_pose_t pose;
      stg_model_get_global_pose( mod, &pose );
      stg_rtk_fig_origin( mod->gui.data_extra, pose.x-0.3, pose.y-0.3, 0 );
      
      stg_energy_data_t* data = (stg_energy_data_t*)mod->data;
      stg_energy_config_t* cfg = (stg_energy_config_t*)mod->cfg;;
      
      double box_offset_x = -0.3;
      double box_offset_y = -0.3;
      double box_height = 0.5; 
      double box_width = 0.1;
      double fraction = data->stored / cfg->capacity;
      double bar_height = box_height * fraction;
      double bar_width = box_width;

      stg_rtk_fig_color_rgb32(mod->gui.data_extra, 0xFFFFFF ); // white

      stg_rtk_fig_rectangle( mod->gui.data_extra, 
			 0,0,0, 
			 box_width, box_height,
			 TRUE );
      
      if( fraction > 0.5 )
	stg_rtk_fig_color_rgb32(mod->gui.data_extra, 0x00FF00 ); // green
      else if( fraction > 0.1 )
	stg_rtk_fig_color_rgb32(mod->gui.data_extra, 0xFFFF00 ); // yellow
      else
	stg_rtk_fig_color_rgb32(mod->gui.data_extra, 0xFF0000 ); // red      

      stg_rtk_fig_rectangle( mod->gui.data_extra, 
			 0,
			 bar_height/2.0 - box_height/2.0,
			 0, 
			 bar_width,
			 bar_height,
			 TRUE );

      stg_rtk_fig_color_rgb32(mod->gui.data_extra, 0x0 ); // black

      stg_rtk_fig_rectangle( mod->gui.data_extra, 
			 0,
			 bar_height/2.0 - box_height/2.0,
			 0, 
			 bar_width,
			 bar_height,
			 FALSE );

      stg_rtk_fig_rectangle( mod->gui.data_extra, 
			 0,0,0, 
			 box_width,
			 box_height,
			 FALSE );

      // place the visualization a little away from the device
      //stg_pose_t pose;
      //model_get_global_pose( mod, &pose );
  
      //pose.x += 0.0;
      //pose.y -= 0.5;
      //pose.a = 0.0;
      //stg_rtk_fig_origin( fig, pose.x, pose.y, pose.a );

      //stg_rtk_fig_color_rgb32(mod->gui.data, stg_lookup_color(STG_ENERGY_COLOR) );

      if( data->charging ) 
	{
	  stg_rtk_fig_color_rgb32(mod->gui.data, 0x00BB00 ); // green
	  stg_rtk_fig_arrow_fancy( mod->gui.data, 0,0,0, data->range, 0.25, 0.10, 1 );

	  //stg_rtk_fig_color_rgb32(mod->gui.data, 0 ); // black
	  //stg_rtk_fig_arrow_fancy( mod->gui.data, 0,0,0, data->range, 0.25, 0.10, 0 );
	}

      if( 0 )
      //if( cfg->capacity > 0 )
	{
	  char buf[256];
	  snprintf( buf, 128, "%.0f/%.0fJ (%.0f%%)\noutput %.2fW %.2fJ\ninput %.2fW %.2fJ\n%s", 
		    data->stored, 
		    cfg->capacity, 
		    data->stored/cfg->capacity * 100,
		    data->output_watts,
		    data->output_joules,
		    data->input_watts,
		    data->input_joules,
		    data->charging ? "charging" : "" );
	  
	  //stg_rtk_fig_text( mod->gui.data, 0.6,0.0,0, buf ); 
	}
	//else
	//{
	//char buf[128];
	//snprintf( buf, 128, "mains supply\noutput %.2fW\n", 
	//    data->output );
	  
	// stg_rtk_fig_text( fig, 0.6,0,0, buf ); 	  
	//}

    }
}


void energy_render_config( stg_model_t* mod )
{ 
  PRINT_DEBUG( "energy config render" );
  
  
  if( mod->gui.cfg  )
    stg_rtk_fig_clear(mod->gui.cfg);
  else // create the figure, store it in the model and keep a local pointer
    {
      mod->gui.cfg = stg_rtk_fig_create( mod->world->win->canvas, 
				   mod->gui.top, STG_LAYER_ENERGYCONFIG );

      stg_rtk_fig_color_rgb32( mod->gui.cfg, stg_lookup_color( STG_ENERGY_CFG_COLOR ));
    }

  stg_energy_config_t cfg;
  stg_model_get_config(mod, &cfg, sizeof(cfg) );

  if( cfg.take > 0 )
    //stg_rtk_fig_arrow_fancy( mod->gui.cfg, 0,0,0, cfg.probe_range, 0.25, 0.10, 1 );
    stg_rtk_fig_arrow( mod->gui.cfg, 0,0,0, cfg.probe_range, 0.25  );
  
}


