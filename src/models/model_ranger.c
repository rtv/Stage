///////////////////////////////////////////////////////////////////////////
//
// File: model_ranger.c
// Author: Richard Vaughan
// Date: 10 June 2004
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/models/model_ranger.c,v $
//  $Author: rtv $
//  $Revision: 1.1 $
//
///////////////////////////////////////////////////////////////////////////

#include <math.h>

//#define DEBUG

#include "stage.h"
#include "raytrace.h"
#include "gui.h"
extern rtk_fig_t* fig_debug;

void ranger_render_config( model_t* mod );
void ranger_render_data( model_t* mod );
int ranger_set_data( model_t* mod, void* data, size_t len );
int ranger_set_config( model_t* mod, void* config, size_t len );

void ranger_init( model_t* mod )
{
  // a ranger has no body 
  model_set_lines( mod, NULL, 0 ); 

  // set up sensible defaults
  stg_ranger_config_t cfg[16];
  size_t cfglen = 16*sizeof(cfg[0]);
  memset( cfg, 0, cfglen);

  stg_ranger_sample_t data[16];
  size_t datalen = 16*sizeof(data[0]);
  memset( data, 0, datalen);
  
  double offset = MIN(mod->geom.size.x, mod->geom.size.y) / 2.0;
  int c;
  for( c=0; c<16; c++ )
    {
      cfg[c].pose.a = M_PI/8 * c;
      cfg[c].pose.x = offset * cos( cfg[c].pose.a );
      cfg[c].pose.y = offset * sin( cfg[c].pose.a );


      cfg[c].size.x = 0.01; // a small device
      cfg[c].size.y = 0.04;
      
      cfg[c].bounds_range.min = 0;
      cfg[c].bounds_range.max = 5.0;
      cfg[c].fov = M_PI/6.0;
    }
  
  ranger_set_config( mod, cfg, cfglen );
  //ranger_set_data( mod, data, datalen );
}

int ranger_set_data( model_t* mod, void* data, size_t len )
{  
  // store the data
  _set_data( mod, data, len );
  
  // and redraw it
  ranger_render_data( mod );

  return 0; //OK
}

int ranger_set_config( model_t* mod, void* config, size_t len )
{  
  // store the config
  _set_cfg( mod, config, len );
  
  // and redraw it
  ranger_render_config( mod);

  return 0; //OK
}


int ranger_shutdown( model_t* mod )
{   
  // clear the data - this will unrender it too.
  ranger_set_data( mod, NULL, 0 );

  return 0;
}

int ranger_update( model_t* mod )
{   
  PRINT_DEBUG1( "[%.3f] updating rangers", mod->world->sim_time );
  
  size_t len = 0;
  stg_ranger_config_t* cfg = 
    (stg_ranger_config_t*)model_get_config(mod,&len);

  if( len < sizeof(stg_ranger_config_t) )
    return 0; // nothing to see here
  
  int rcount = len / sizeof(stg_ranger_config_t);
  
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
	  if (hitmod == mod || model_is_descendent(hitmod,mod) ) 
	    continue;
	  
	  // Stop looking when we see an obstacle
	  if( hitmod->obstacle_return ) 
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
  
  ranger_set_data( mod, ranges, sizeof(stg_ranger_sample_t) * rcount );

  //model_set_prop( mod, STG_PROP_RANGERDATA,
  //	  ranges, sizeof(stg_ranger_sample_t) * rcount );

  return 0;
}

void ranger_render_config( model_t* mod )
{
  //PRINT_DEBUG( "drawing rangers" );
  
  stg_geom_t* geom = model_get_geom(mod);
  
  rtk_fig_t* fig = mod->gui.propgeom[STG_PROP_CONFIG];  
  
  if( fig  )
    rtk_fig_clear(fig);
  else // create the figure, store it in the model and keep a local pointer
    fig = model_prop_fig_create( mod, mod->gui.propgeom, STG_PROP_CONFIG,
				 mod->gui.top, STG_LAYER_RANGERGEOM );
  
  rtk_fig_color_rgb32( fig, stg_lookup_color(STG_RANGER_GEOM_COLOR) );  
  rtk_fig_origin( fig, geom->pose.x, geom->pose.y, geom->pose.a );  
  
  rtk_fig_t* cfgfig = mod->gui.propdata[STG_PROP_CONFIG];  
  if( cfgfig  )
    rtk_fig_clear(cfgfig);
  else // create the figure, store it in the model and keep a local pointer
    cfgfig = model_prop_fig_create( mod, mod->gui.propdata, STG_PROP_CONFIG,
				 mod->gui.top, STG_LAYER_RANGERCONFIG );

  rtk_fig_color_rgb32( cfgfig, stg_lookup_color(STG_RANGER_CONFIG_COLOR) );  
  rtk_fig_origin( cfgfig, geom->pose.x, geom->pose.y, geom->pose.a );  
  
  //stg_property_t* prop = model_get_prop_generic( mod, STG_PROP_RANGERCONFIG );
  size_t len = 0;
  stg_ranger_config_t* cfg = (stg_ranger_config_t*)model_get_config(mod,&len);

  if( len < sizeof(stg_ranger_config_t) )
    return ; // nothing to render

  assert( cfg );

  int rcount = len / sizeof( stg_ranger_config_t );
  
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
			 rngr->size.x, rngr->size.y, 
			 mod->world->win->fill_polygons ); 
      
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

void ranger_render_data( model_t* mod )
{ 
  rtk_fig_t* fig = mod->gui.propdata[STG_PROP_DATA];  
  
  if( fig  )
    rtk_fig_clear(fig);
  else // create the figure, store it in the model and keep a local pointer
    fig = model_prop_fig_create( mod, mod->gui.propdata, STG_PROP_DATA,
				 mod->gui.top, STG_LAYER_RANGERDATA );
  
  size_t len = 0;
  stg_ranger_config_t* cfg = 
    (stg_ranger_config_t*)model_get_config(mod,&len);  
  
  if( len < sizeof(stg_ranger_config_t) )
    return;
  
  int rcount = len / sizeof( stg_ranger_config_t );
  
  len = 0;
  stg_ranger_sample_t* samples = 
    (stg_ranger_sample_t*)model_get_data(mod, &len );
  
  if( len != (rcount * sizeof(stg_ranger_sample_t) ))
    {
      PRINT_DEBUG3( "wrong data size %d/%d bytes in ranger %s",
		    (int)len, 
		    (int)(rcount * sizeof(stg_ranger_sample_t)), 
		    mod->token );
      return;
    }
 
  // should be ok by now!
  if( rcount > 0 && cfg && samples )
    {
      stg_geom_t *geom = model_get_geom(mod);
      
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

int register_ranger(void)
{
  register_init( STG_MODEL_RANGER, ranger_init );
  register_update( STG_MODEL_RANGER, ranger_update );
  register_shutdown( STG_MODEL_RANGER, ranger_shutdown );
  register_set_config( STG_MODEL_RANGER, ranger_set_config );
  register_set_data( STG_MODEL_RANGER, ranger_set_data );

  return 0; //ok
} 
