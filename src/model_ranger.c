///////////////////////////////////////////////////////////////////////////
//
// File: model_ranger.c
// Author: Richard Vaughan
// Date: 10 June 2004
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/model_ranger.c,v $
//  $Author: rtv $
//  $Revision: 1.18 $
//
///////////////////////////////////////////////////////////////////////////

#include <math.h>

//#define DEBUG

#include "stage.h"
#include "raytrace.h"
#include "gui.h"
extern rtk_fig_t* fig_debug;

void model_ranger_config_init( model_t* mod );
int model_ranger_config_set( model_t* mod, void* config, size_t len );
int model_ranger_data_shutdown( model_t* mod );
int model_ranger_data_service( model_t* mod );
int model_ranger_data_set( model_t* mod, void* data, size_t len );
void model_ranger_return_init( model_t* mod );
void model_ranger_data_render( model_t* mod );
void model_ranger_config_render( model_t* mod );

void model_ranger_register(void)
{ 
  PRINT_DEBUG( "RANGER INIT" );  

  model_register_init( STG_PROP_RANGERCONFIG, model_ranger_config_init );
  model_register_set( STG_PROP_RANGERCONFIG, model_ranger_config_set );

  model_register_set( STG_PROP_RANGERDATA, model_ranger_data_set );
  model_register_shutdown( STG_PROP_RANGERDATA, model_ranger_data_shutdown );
  model_register_service( STG_PROP_RANGERDATA, model_ranger_data_service );

  model_register_init( STG_PROP_RANGERRETURN, model_ranger_return_init );

}

void model_ranger_return_init( model_t* mod )
{
  stg_bool_t val = STG_DEFAULT_RANGERRETURN;
  model_set_prop_generic( mod, STG_PROP_RANGERRETURN, &val, sizeof(val) );
}


void model_ranger_config_init( model_t* mod )
{
  model_set_prop_generic( mod, STG_PROP_RANGERCONFIG, NULL, 0 );
}

stg_bool_t model_ranger_return( model_t* mod )
{
  stg_bool_t* val = model_get_prop_data_generic( mod, STG_PROP_RANGERRETURN );
  assert(val);
  return *val;
}


stg_ranger_config_t* model_ranger_config_get( model_t* mod, int* count )
{
  stg_property_t* prop = model_get_prop_generic( mod, STG_PROP_RANGERCONFIG );
  assert(prop);
  
  stg_ranger_config_t* cfgs = prop->data;
  *count = prop->len / sizeof(stg_ranger_config_t);
  return cfgs;
}

int model_ranger_data_set( model_t* mod, void* data, size_t len )
{  
  // store the data
  model_set_prop_generic( mod, STG_PROP_RANGERDATA, data, len );
  
  // and redraw it
  model_ranger_data_render( mod );

  return 0; //OK
}

int model_ranger_config_set( model_t* mod, void* config, size_t len )
{  
  // store the config
  model_set_prop_generic( mod, STG_PROP_RANGERCONFIG, config, len );
  
  // and redraw it
  model_ranger_config_render( mod);

  return 0; //OK
}


int model_ranger_data_shutdown( model_t* mod )
{ 
  // undraw the data
  model_ranger_data_render(mod);

  //model_remove_prop_generic( mod, STG_PROP_RANGERDATA );

  return 0;
}

int model_ranger_data_service( model_t* mod )
{   
  //PRINT_DEBUG1( "[%.3f] updating rangers", mod->world->sim_time );
  
  int rcount=0;
  stg_ranger_config_t* cfg = model_ranger_config_get(mod, &rcount);
    
  stg_ranger_sample_t* ranges = (stg_ranger_sample_t*)
    calloc( sizeof(stg_ranger_sample_t), rcount );
  
  if( fig_debug ) rtk_fig_clear( fig_debug );

  int t;
  for( t=0; t<rcount; t++ )
    {
      // get the sensor's pose in global coords
      stg_pose_t pz;
      memcpy( &pz, &cfg[t].pose, sizeof(pz) ); 
      model_local_to_global( mod, &pz );
      
      // todo - use bounds_range.min

      itl_t* itl = itl_create( pz.x, pz.y, pz.a, 
			       cfg[t].bounds_range.max, 
			       mod->world->matrix, 
			       PointToBearingRange );
      
      model_t * hitmod;
      double range = cfg[t].bounds_range.max;
      
      while( (hitmod = itl_next( itl )) ) 
	{
	  //printf( "model %d %p   hit model %d %p\n",
	  //  mod->id, mod, hitmod->id, hitmod );
	  
	  // Ignore myself, things which are attached to me, and
	  // things that we are attached to (except root) The latter
	  // is useful if you want to stack beacons on the laser or
	  // the laser on somethine else.
	  if (hitmod == mod )//|| modthis->IsDescendent(ent) )//|| 
	    continue;
	  
	  // Stop looking when we see something
	  if( model_ranger_return(hitmod) != LaserTransparent) 
	    {
	      range = itl->range;
	      break;
	    }	
	}
      
      if( range < cfg[t].bounds_range.min )
	range = cfg[t].bounds_range.min;
      
      ranges[t].range = range;
      //ranges[t].error = TODO;

      itl_destroy( itl );
    }
  
  model_set_prop( mod, STG_PROP_RANGERDATA,
		  ranges, sizeof(stg_ranger_sample_t) * rcount );

  return 0;
}

void model_ranger_config_render( model_t* mod )
{
  //PRINT_DEBUG( "drawing rangers" );

  stg_geom_t* geom = model_geom_get(mod);
  
  rtk_fig_t* fig = mod->gui.propgeom[STG_PROP_RANGERCONFIG];  
  
  if( fig  )
    rtk_fig_clear(fig);
  else // create the figure, store it in the model and keep a local pointer
    fig = model_prop_fig_create( mod, mod->gui.propgeom, STG_PROP_RANGERCONFIG,
				 mod->gui.top, STG_LAYER_RANGERGEOM );
  
  rtk_fig_color_rgb32( fig, stg_lookup_color(STG_RANGER_GEOM_COLOR) );  
  rtk_fig_origin( fig, geom->pose.x, geom->pose.y, geom->pose.a );  
  
  rtk_fig_t* cfgfig = mod->gui.propdata[STG_PROP_RANGERCONFIG];  
  if( cfgfig  )
    rtk_fig_clear(cfgfig);
  else // create the figure, store it in the model and keep a local pointer
    cfgfig = model_prop_fig_create( mod, mod->gui.propdata, STG_PROP_RANGERCONFIG,
				 mod->gui.top, STG_LAYER_RANGERCONFIG );

  rtk_fig_color_rgb32( cfgfig, stg_lookup_color(STG_RANGER_CONFIG_COLOR) );  
  rtk_fig_origin( cfgfig, geom->pose.x, geom->pose.y, geom->pose.a );  
  
  stg_property_t* prop = model_get_prop_generic( mod, STG_PROP_RANGERCONFIG );
  
  if( prop == NULL ) // no rangers to update!
    return;
  //else
  
  stg_ranger_config_t* cfg = (stg_ranger_config_t*)prop->data;
  assert( cfg );

  int rcount = prop->len / sizeof( stg_ranger_config_t );
  
  if( rcount < 1 )
    return;


  //rtk_fig_t* geomfig = gui_model_figs(mod)->ranger_ge;

  // add rects showing ranger positions
  int s;
  for( s=0; s<rcount; s++ )
    {
      stg_ranger_config_t* rngr = &cfg[s];
      //printf( "drawing a ranger rect (%.2f,%.2f,%.2f)[%.2f %.2f][%.2f %.2f %.2f]\n",
      //      rngr->pose.x, rngr->pose.y, rngr->pose.a,
      //      rngr->size.x, rngr->size.y,
      //      rngr->bounds_range.min, rngr->bounds_range.max, rngr->fov );
      
      // sensor pose
      rtk_fig_rectangle( fig, 
			 rngr->pose.x, rngr->pose.y, rngr->pose.a,
			 rngr->size.x, rngr->size.y, 0 ); 
      
      // TODO - FIX THIS

      // sensor FOV
      double sidelen = rngr->bounds_range.max;
      double da = rngr->fov/2.0;

      double x1= rngr->pose.x + sidelen*cos(rngr->pose.a - da );
      double y1= rngr->pose.y + sidelen*sin(rngr->pose.a - da );
      double x2= rngr->pose.x + sidelen*cos(rngr->pose.a + da );
      double y2= rngr->pose.y + sidelen*sin(rngr->pose.a + da );
      
      rtk_fig_line( cfgfig, rngr->pose.x, rngr->pose.y, x1, y1 );
      rtk_fig_line( cfgfig, rngr->pose.x, rngr->pose.y, x2, y2 );	
      
      rtk_fig_ellipse_arc( cfgfig, rngr->pose.x, rngr->pose.y, rngr->pose.a,
			   2.0*cfg->bounds_range.max,
			   2.0*cfg->bounds_range.max, 
			   -da, da );
    }
}

void model_ranger_data_render( model_t* mod )
{ 
   rtk_fig_t* fig = mod->gui.propdata[STG_PROP_RANGERDATA];  
  
  if( fig  )
    rtk_fig_clear(fig);
  else // create the figure, store it in the model and keep a local pointer
    fig = model_prop_fig_create( mod, mod->gui.propdata, STG_PROP_RANGERDATA,
				 mod->gui.top, STG_LAYER_RANGERDATA );

  if(  mod->subs[STG_PROP_RANGERDATA] )
    {
      
      stg_property_t* prop = model_get_prop_generic( mod, STG_PROP_RANGERCONFIG );
      
      if( prop == NULL ) // no rangers to update!
	return;
      
      stg_ranger_config_t* cfg = (stg_ranger_config_t*)prop->data;  
      int rcount = prop->len / sizeof( stg_ranger_config_t );
      
      stg_ranger_sample_t* samples = (stg_ranger_sample_t*)
	model_get_prop_data_generic( mod, STG_PROP_RANGERDATA );
      
      if( rcount > 0 && cfg && samples )
	{
	  stg_geom_t *geom = model_geom_get(mod);

	  rtk_fig_color_rgb32(fig, stg_lookup_color(STG_RANGER_COLOR) );
	  rtk_fig_origin( fig, geom->pose.x, geom->pose.y, geom->pose.a );	  
	  // draw the range  beams
	  int s;
	  for( s=0; s<rcount; s++ )
	    {
	      if( samples[s].range > 0.0 )
		{
		  stg_ranger_config_t* rngr = &cfg[s];
		  
		  rtk_fig_arrow( fig, rngr->pose.x, rngr->pose.y, rngr->pose.a, 			       samples[s].range, 0.02 );
		}
	    }
	}
    }
}

