
///////////////////////////////////////////////////////////////////////////
//
// File: model_energy.c
// Author: Richard Vaughan
// Date: 10 June 2004
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/model_energy.c,v $
//  $Author: rtv $
//  $Revision: 1.29 $
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


int energy_startup( stg_model_t* mod );
int energy_shutdown( stg_model_t* mod );
int energy_update( stg_model_t* mod );

void energy_load( stg_model_t* mod );

int energy_render_data( stg_model_t* mod, void* userp );
int energy_render_data_text( stg_model_t* mod, void* userp );
int energy_unrender_data( stg_model_t* mod, void* userp );
int energy_unrender_data_text( stg_model_t* mod, void* userp );
int energy_render_cfg( stg_model_t* mod, void* userp );
int energy_unrender_cfg( stg_model_t* mod, void* userp );

const double STG_ENERGY_PROBE_RANGE_DEFAULT = 0.5;
const double STG_ENERGY_GIVE_RATE_DEFAULT = 100.0;
const double STG_ENERGY_TAKE_RATE_DEFAULT = 100.0;
const double STG_ENERGY_CAPACITY_DEFAULT = 10000.0;
const double STG_ENERGY_SIZEX_DEFAULT = 0.08;
const double STG_ENERGY_SIZEY_DEFAULT = 0.18;
const double STG_ENERGY_POSEX_DEFAULT = 0.0;
const double STG_ENERGY_POSEY_DEFAULT = 0.0;
const double STG_ENERGY_POSEA_DEFAULT = 0.0;
const int STG_ENERGY_FIDUCIAL_DEFAULT = 255;

typedef struct
{
} stg_energy_model_t;



int energy_init( stg_model_t* mod ) 
{
  // override the default methods
  mod->f_startup = energy_startup;
  mod->f_shutdown = energy_shutdown;
  mod->f_update = energy_update;
  mod->f_load = energy_load;

  // batteries aren't obstacles or sensible to range sensors
  stg_model_set_obstacle_return( mod, FALSE );
  stg_model_set_ranger_return( mod, FALSE );
  stg_model_set_laser_return( mod, LaserTransparent );
  stg_model_set_fiducial_return( mod, FiducialNone );

  // XX ? does this make sense?
  stg_model_set_watts( mod, 10.0 );
  
  // sensible energy defaults
  stg_geom_t geom;
  geom.pose.x = STG_ENERGY_POSEX_DEFAULT;
  geom.pose.y = STG_ENERGY_POSEY_DEFAULT;
  geom.pose.a = STG_ENERGY_POSEA_DEFAULT;
  geom.size.x = STG_ENERGY_SIZEX_DEFAULT;
  geom.size.y = STG_ENERGY_SIZEY_DEFAULT;
  stg_model_set_geom( mod, &geom );
  
  // set up config structure
  stg_energy_config_t cfg;
  memset(&cfg,0,sizeof(cfg));  
  cfg.probe_range = STG_ENERGY_PROBE_RANGE_DEFAULT;
  cfg.give_rate = STG_ENERGY_GIVE_RATE_DEFAULT;
  cfg.take_rate = STG_ENERGY_TAKE_RATE_DEFAULT;
  cfg.capacity = STG_ENERGY_CAPACITY_DEFAULT;
  cfg.give = FALSE; // will not charge others by default
  stg_model_set_cfg( mod, &cfg, sizeof(cfg));

  // set up initial data structure
  stg_energy_data_t data;
  memset(&data, 0, sizeof(data));
  data.stored = STG_ENERGY_CAPACITY_DEFAULT;
  data.charging = FALSE;
  data.range = STG_ENERGY_PROBE_RANGE_DEFAULT;
  data.connections = g_ptr_array_new();  
  stg_model_set_data( mod, &data, sizeof(data));

  // set default color
  stg_color_t col = stg_lookup_color( "orange" ); 
  stg_model_set_color( mod, col );

  //stg_model_add_callback( mod, &mod->data, energy_render_data, NULL );

  // adds a menu item and associated on-and-off callbacks
  stg_model_add_property_toggles( mod, 
				  &mod->data,
				  energy_render_data, // called when toggled on
				  NULL,
				  energy_unrender_data, // called when toggled off
				  NULL,
				  "energy_bar",
				  "energy bar",
				  TRUE );  
  stg_model_add_property_toggles( mod, 
				  &mod->data,
				  energy_render_data_text, // called when toggled on
				  NULL,
				  energy_unrender_data_text, // called when toggled off
				  NULL,
				  "energy_text",
				  "energy text",
				  FALSE );  

  return 0;
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
  printf( "connecting %s to %s\n", sink->token, source->token );
  
  stg_energy_data_t* data = (stg_energy_data_t*)source->data;
  
  g_ptr_array_add( data->connections, sink );
}

// remove sink from source's list of power connections
void energy_disconnect( stg_model_t* source, stg_model_t* sink )
{
  printf( "disconnecting %s to %s\n", sink->token, source->token );
  
  stg_energy_data_t* data = (stg_energy_data_t*)source->data;
  
  g_ptr_array_remove_fast( data->connections, sink );
}


void energy_load( stg_model_t* mod )
{
  stg_energy_config_t cfg;
  memcpy( &cfg, mod->cfg, sizeof(cfg));  
  cfg.capacity = wf_read_float( mod->id, "capacity", cfg.capacity );
  cfg.probe_range = wf_read_length( mod->id, "probe_range", cfg.probe_range );
  cfg.give_rate = wf_read_float( mod->id, "give_rate", cfg.give_rate );
  cfg.take_rate = wf_read_float( mod->id, "take_rate", cfg.take_rate );  
  cfg.give = wf_read_float( mod->id, "give", cfg.give );  
  stg_model_set_cfg( mod, &cfg, sizeof(cfg));
  
  // refill the tank with the new capacity - we start fully gassed up
  stg_energy_data_t data;
  memcpy( &data, mod->data, sizeof(data));  
  data.stored = cfg.capacity;
  stg_model_set_data( mod, &data, sizeof(data));
}

int energy_match( stg_model_t* mod, stg_model_t* hitmod )
{
  // connect only to energy models,ignoring myself, my children, and
  // my ancestors.
  if( hitmod->typerec == mod->typerec 
      && mod != hitmod 
      && (!stg_model_is_related(mod,hitmod)))
    return 1;
  
  return 0; // no match
}	


stg_watts_t energy_connection_sum( stg_model_t* mod, GPtrArray* cons )
{
  stg_watts_t watts = 0;
  
  // find the total current required, so I can do current limiting
  if( cons && cons->len )
    for( int i=0; i < cons->len; i++ )
      {
	stg_model_t* con = (stg_model_t*)g_ptr_array_index( cons, i );
	
	if( con == mod ) // skip myself
	  continue;
	
	watts += con->watts;
      }
  
  return watts;
}

void energy_transfer( stg_model_t* src, stg_model_t* sink )
{
  printf( "transferring energy from %s to %s\n", src->token, sink->token );
	    
  double timeslice =  src->world->sim_interval/1000.0;

  stg_energy_config_t* srccfg = (stg_energy_config_t*)src->cfg;
  stg_energy_data_t* srcdata = (stg_energy_data_t*)src->data;

  stg_energy_config_t* sinkcfg = (stg_energy_config_t*)sink->cfg;
  stg_energy_data_t* sinkdata = (stg_energy_data_t*)sink->data;
	    
  // how many joules does the sink need?
  stg_joules_t need = sinkcfg->capacity - sinkdata->stored;
  
  // src can give max this many joules per timestep
  stg_joules_t give_max = srccfg->give * timeslice;
  
  // if src is a finite supply, it may not have enough juice
  if( srccfg->capacity > 0 ) 
    give_max = MIN( give_max, srcdata->stored );
  
  // we give the smaller of these two to the sink
  stg_joules_t delivered_j = MIN( need, give_max );
	    
  // convert that to watts
  //stg_watts_t delivered_w = delivered_j / timeslice;
	
  srcdata->stored -= delivered_j;
  sinkdata->stored += delivered_j;
  
  srcdata->charging = FALSE;
  sinkdata->charging = ( delivered_j > 0 );
  
  printf( "%.1fW connection transferred %f joules in %.4f seconds\n", 
	  delivered_j / timeslice, delivered_j, timeslice ); 
}


int energy_update( stg_model_t* mod )
{     
  PRINT_DEBUG1( "energy service model %d", mod->id  );  
  
  stg_energy_data_t *data = (stg_energy_data_t*)mod->data;
  stg_energy_config_t *cfg = (stg_energy_config_t*)mod->cfg;
  
  double timeslice =  mod->world->sim_interval/1000.0;
  
  //  max range until we know we hit something
  data->range = cfg->probe_range;
  data->charging = FALSE;

  if( cfg->probe_range > 0 )
    {
      stg_pose_t pose;
      stg_model_get_global_pose( mod, &pose );
      
      itl_t* itl = itl_create( pose.x, pose.y, pose.a, cfg->probe_range, 
			       mod->world->matrix, 
			       PointToBearingRange );
      
      stg_model_t* hitmod = 
	itl_first_matching( itl, energy_match, mod );
      
      
      if( hitmod )
	{
	  printf( "CONNECTING to %s\n", hitmod->token );
	  data->range = itl->range;
	  //data->charging = TRUE;
	  energy_connect( mod, hitmod );
	}
    }

  // if I have some juice to give
  if( data->stored > 0 )
    {
      // find all the locally connected devices
      GPtrArray* locals = g_ptr_array_new();
      stg_model_tree_to_ptr_array( stg_model_root(mod), locals);
      
      double watts_out = energy_connection_sum( mod, locals );
      //printf( "local devices consumed %.2fW\n", watts_out ); 
      
      data->stored -= watts_out * timeslice;
            
      // if we give to others, feed all the connected devices
      if( cfg->give && cfg->give_rate > 0 )
	for( int i=0; i < data->connections->len; i++ )
	  {      		
	    stg_model_t* con = (stg_model_t*)g_ptr_array_index( data->connections, i );	    
	    printf( "giving on connection %d to %s\n", i, con->token );
	    energy_transfer( mod, con );	    
	  }
    }
  
  if( ! cfg->give )// we don't give - we just take instead
    {
      for( int i=0; i < data->connections->len; i++ )
	{       
	  stg_model_t* con = (stg_model_t*)g_ptr_array_index( data->connections, i );		
	  printf( "taking on connection %d to %s\n", i, con->token );
	  energy_transfer( con, mod );
	}
    }
  
  // now disconnect everyone the fast way
  g_ptr_array_set_size( data->connections, 0 );
  
  model_change( mod, &mod->data );
  
  return 0; // ok
}

int energy_unrender_data( stg_model_t* mod, void* userp )
{ 
  stg_model_fig_clear( mod, "energy_data_fig" );
  stg_model_fig_clear( mod, "energy_data_figx" );
  return 1; // quit callback
}

int energy_unrender_data_text( stg_model_t* mod, void* userp )
{ 
  stg_model_fig_clear( mod, "energy_data_text_fig" );
  return 1; // quit callback
}

int energy_render_data( stg_model_t* mod,
			 void* userp )
{
  PRINT_DEBUG( "energy data render" );
  
  stg_rtk_fig_t *fig, *figx;
  
  if( (fig = stg_model_get_fig( mod, "energy_data_fig" )))
    stg_rtk_fig_clear( fig );
  else
    fig = stg_model_fig_create( mod, "energy_data_fig", "top", STG_LAYER_ENERGYDATA );  
  
  if( (figx = stg_model_get_fig( mod, "energy_data_figx" )))
    stg_rtk_fig_clear( figx );
  else
    figx = stg_model_fig_create( mod, "energy_data_figx", "top", 90 );  
  
  // if this model has a energy subscription

  //if(  mod->subs )
  //if(  1 )
  {  
    
    stg_energy_data_t* data =  (stg_energy_data_t*)mod->data;    
    stg_energy_config_t* cfg = (stg_energy_config_t*)mod->cfg;

    stg_geom_t geom;
    stg_model_get_geom( mod, &geom );
    
    double box_height = geom.size.y; 
    double box_width = geom.size.x;
    double fraction = data->stored / cfg->capacity;
    double bar_height = box_height * fraction;
    double bar_width = box_width;
    
    stg_rtk_fig_color_rgb32(figx, 0xFFFFFF ); // white
    
    stg_rtk_fig_rectangle( figx, 
			   0,0,0, 
			   box_width, box_height,
			   TRUE );
    
    stg_color_t col;
    if( fraction > 0.75 )
      col = 0x00FF00; //  green
    else if( fraction > 0.5 )
      col = 0xFFFF00; // yellow
    else if( fraction > 0.25 )
      col = 0xFF9000; // orange
    else
      col = 0xFF0000; // red      
    
    stg_rtk_fig_color_rgb32(figx, col );
    
    stg_rtk_fig_rectangle( figx, 
			   0,
			   bar_height/2.0 - box_height/2.0,
			   0, 
			   bar_width,
			   bar_height,
			   TRUE );
    
    stg_rtk_fig_color_rgb32(figx, 0x0 ); // black
    
    stg_rtk_fig_rectangle( figx, 
			   0,
			   bar_height/2.0 - box_height/2.0,
			   0, 
			   bar_width,
			   bar_height,
			   FALSE );
    
    stg_rtk_fig_rectangle( figx, 
			   0,0,0, 
			   box_width,
			   box_height,
			   FALSE );
    
    //stg_rtk_fig_color_rgb32(fig, stg_lookup_color(STG_ENERGY_COLOR) );
    
    if( cfg->probe_range > 0 )
      {
	if( data->charging ) 
	  {
	    stg_rtk_fig_color_rgb32(figx, col ); // green
	    stg_rtk_fig_arrow_fancy( figx, 0,0,0, data->range, 0.15, 0.05, 1 );
	  }
	
	stg_rtk_fig_color_rgb32(figx, 0 ); // black
	stg_rtk_fig_arrow_fancy( figx, 0,0,0, data->range, 0.15, 0.05, 0 );
      }
  }
  
  return 0;
}

int energy_render_data_text( stg_model_t* mod, void* userp )
{
  stg_rtk_fig_t* fig = stg_model_get_fig( mod, "energy_data_text_fig" );  
  
  if( ! fig )
    fig = stg_model_fig_create( mod, "energy_data_text_fig", "top", STG_LAYER_ENERGYDATA );  
  else
    stg_rtk_fig_clear( fig );
  
  stg_energy_data_t* data = (stg_energy_data_t*)mod->data;
  stg_energy_config_t* cfg = (stg_energy_config_t*)mod->cfg;
  
  char buf[256];
  snprintf( buf, 128, "%.0f/%.0fJ (%.0f%%)\n%s", 
	    data->stored, 
	    cfg->capacity, 
	    data->stored/cfg->capacity * 100,
	    data->charging ? "charging" : "" );
  
  stg_rtk_fig_text( fig, 0.6,0.0,0, buf ); 
  
  //else
  //{
  //char buf[128];
  //snprintf( buf, 128, "mains supply\noutput %.2fW\n", 
  //    data->output );
  
  // stg_rtk_fig_text( fig, 0.6,0,0, buf ); 	  
  //

  return 0;
}

/* void energy_render_config( stg_model_t* mod ) */
/* {  */
/*   PRINT_DEBUG( "energy config render" ); */
  
  
/*   if( mod->gui.cfg  ) */
/*     stg_rtk_fig_clear(mod->gui.cfg); */
/*   else // create the figure, store it in the model and keep a local pointer */
/*     { */
/*       mod->gui.cfg = stg_rtk_fig_create( mod->world->win->canvas,  */
/* 				   mod->gui.top, STG_LAYER_ENERGYCONFIG ); */

/*       stg_rtk_fig_color_rgb32( mod->gui.cfg, stg_lookup_color( STG_ENERGY_CFG_COLOR )); */
/*     } */

/*   stg_energy_config_t cfg; */
/*   stg_model_get_config(mod, &cfg, sizeof(cfg) ); */

/*   if( cfg.take > 0 ) */
/*     //stg_rtk_fig_arrow_fancy( mod->gui.cfg, 0,0,0, cfg.probe_range, 0.25, 0.10, 1 ); */
/*     stg_rtk_fig_arrow( mod->gui.cfg, 0,0,0, cfg.probe_range, 0.25  ); */
  
/* } */


