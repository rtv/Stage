///////////////////////////////////////////////////////////////////////////
//
// File: model_ranger.c
// Author: Richard Vaughan
// Date: 10 June 2004
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/model_ranger.c,v $
//  $Author: rtv $
//  $Revision: 1.66 $
//
///////////////////////////////////////////////////////////////////////////

/**
@ingroup model
@defgroup model_ranger Ranger model 
The ranger model simulates an array of sonar or infra-red (IR) range sensors.

<h2>Worldfile properties</h2>

@par Summary and default values

@verbatim
ranger
(
  # ranger properties
  scount 16
  spose[0] [? ? ?]
  spose[1] [? ? ?]
  spose[2] [? ? ?]
  spose[3] [? ? ?]
  spose[4] [? ? ?]
  spose[5] [? ? ?]
  spose[6] [? ? ?]
  spose[7] [? ? ?]
  spose[8] [? ? ?]
  spose[9] [? ? ?]
  spose[10] [? ? ?]
  spose[11] [? ? ?]
  spose[12] [? ? ?]
  spose[13] [? ? ?]
  spose[14] [? ? ?]
  spose[15] [? ? ?]
   
  ssize [0.01 0.03]
  sview [0.0 5.0 5.0]

  # model properties
  watts 2.0
)
@endverbatim

@par Notes

The ranger model allows configuration of the pose, size and view parameters of each transducer seperately (using spose[index], ssize[index] and sview[index]). However, most users will set a common size and view (using ssize and sview), and just specify individual transducer poses.

@par Details
- scount int 
  - the number of range transducers
- spose[\<transducer index\>] [float float float]
  - [x y theta] 
  - pose of the transducer relative to its parent.
- ssize [float float]
  - [x y] 
  - size in meters. Has no effect on the data, but controls how the sensor looks in the Stage window.
- ssize[\<transducer index\>] [float float]
  - per-transducer version of the ssize property. Overrides the common setting.
- sview [float float float]
   - [range_min range_max fov] 
   - minimum range and maximum range in meters, field of view angle in degrees. Currently fov has no effect on the sensor model, other than being shown in the confgiuration graphic for the ranger device.
- sview[\<transducer index\>] [float float float]
  - per-transducer version of the sview property. Overrides the common setting.

*/

#include <math.h>
#include "stage_internal.h"
#include "gui.h"

extern stg_rtk_fig_t* fig_debug_rays;

#define STG_RANGER_WATTS 2.0 // ranger power consumption

int ranger_update( stg_model_t* mod );
int ranger_startup( stg_model_t* mod );
int ranger_shutdown( stg_model_t* mod );
void ranger_load( stg_model_t* mod );

int ranger_render_data( stg_model_t* mod, void* userp );
int ranger_unrender_data( stg_model_t* mod,void* userp );
int ranger_render_cfg( stg_model_t* mod, void* userp );
int ranger_unrender_cfg( stg_model_t* mod, void* userp );

int ranger_init( stg_model_t* mod )
{
  // override the default methods
  mod->f_startup = ranger_startup;
  mod->f_shutdown = ranger_shutdown;
  mod->f_update = NULL; // installed on startup & shutdown
  mod->f_load = ranger_load;
  
  stg_model_set_data( mod, NULL, 0 );
  
  // Set up sensible defaults
  stg_geom_t geom;
  memset( &geom, 0, sizeof(geom)); // no size
  stg_model_set_geom( mod, &geom );
  
  stg_color_t col = stg_lookup_color( STG_RANGER_CONFIG_COLOR );
  stg_model_set_color( mod, col );
  
  // remove the polygon: ranger has no body
  stg_model_set_polygons( mod, NULL, 0 );

  stg_ranger_config_t cfg[16];
  size_t cfglen = 16*sizeof(cfg[0]);
  memset( cfg, 0, cfglen);
  
  double offset = MIN(geom.size.x, geom.size.y) / 2.0;

  // create default ranger config
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
      cfg[c].ray_count = 3;
    }
  stg_model_set_cfg( mod, cfg, cfglen );
  
  //stg_model_add_callback( mod, &mod->data, ranger_render_data, NULL );
#if INCLUDE_GNOME
  gc_ranger_init( mod );
#else
  // adds a menu item and associated on-and-off callbacks
  stg_model_add_property_toggles( mod, 
				  &mod->data,
				  ranger_render_data, // called when toggled on
				  NULL,
				  ranger_unrender_data, // called when toggled off
				  NULL,
				  "rangerdata",
				  "ranger data",
				  TRUE );  // initial state

  stg_model_add_property_toggles( mod, 
				  &mod->cfg,
				  ranger_render_cfg, // called when toggled on
				  NULL,
				  ranger_unrender_cfg, // called when toggled off
				  NULL,
				  "rangercfg",
				  "ranger config",
				  FALSE );  // initial state
#endif

  return 0;
}

int ranger_startup( stg_model_t* mod )
{
  PRINT_DEBUG( "ranger startup" );

  mod->f_update = ranger_update;

  stg_model_set_watts( mod, STG_RANGER_WATTS );

  return 0;
}


int ranger_shutdown( stg_model_t* mod )
{
  PRINT_DEBUG( "ranger shutdown" );

  mod->f_update = NULL;
  stg_model_set_watts( mod, 0 );
  
  // clear the data - this will unrender it too.
  stg_model_set_data( mod, NULL, 0 );

  return 0;
}

void ranger_load( stg_model_t* mod )
{
  // Load the geometry of a ranger array
  int scount = wf_read_int( mod->id, "scount", 0);
  if (scount > 0)
    {
      char key[256];
      stg_ranger_config_t* configs = (stg_ranger_config_t*)
	calloc( sizeof(stg_ranger_config_t), scount );
      
      stg_ranger_config_t* now = (stg_ranger_config_t*)mod->cfg;
      
      stg_size_t common_size;
      common_size.x = wf_read_tuple_length(mod->id, "ssize", 0, now->size.x );
      common_size.y = wf_read_tuple_length(mod->id, "ssize", 1, now->size.y );
      
      double common_min = wf_read_tuple_length(mod->id, "sview", 0, now->bounds_range.min );
      double common_max = wf_read_tuple_length(mod->id, "sview", 1, now->bounds_range.max );
      double common_fov = wf_read_tuple_angle(mod->id, "sview", 2, now->fov);

      int common_ray_count = wf_read_int(mod->id, "sraycount", now->ray_count );

      // set all transducers with the common settings
      int i;
      for(i = 0; i < scount; i++)
	{
	  configs[i].size.x = common_size.x;
	  configs[i].size.y = common_size.y;
	  configs[i].bounds_range.min = common_min;
	  configs[i].bounds_range.max = common_max;
	  configs[i].fov = common_fov;
	  configs[i].ray_count = common_ray_count;	  
	}

      // TODO - do this properly, without the hard-coded defaults
      // allow individual configuration of transducers
      for(i = 0; i < scount; i++)
	{
	  snprintf(key, sizeof(key), "spose[%d]", i);
	  configs[i].pose.x = wf_read_tuple_length(mod->id, key, 0, 0);
	  configs[i].pose.y = wf_read_tuple_length(mod->id, key, 1, 0);
	  configs[i].pose.a = wf_read_tuple_angle(mod->id, key, 2, 0);
	  
/* 	  snprintf(key, sizeof(key), "ssize[%d]", i); */
/* 	  configs[i].size.x = wf_read_tuple_length(mod->id, key, 0, 0.01); */
/* 	  configs[i].size.y = wf_read_tuple_length(mod->id, key, 1, 0.05); */
	  
/* 	  snprintf(key, sizeof(key), "sview[%d]", i); */
/* 	  configs[i].bounds_range.min = */
/* 	    wf_read_tuple_length(mod->id, key, 0, 0); */
/* 	  configs[i].bounds_range.max =   // set up sensible defaults */

/* 	    wf_read_tuple_length(mod->id, key, 1, 5.0); */
/* 	  configs[i].fov */
/* 	    = DTOR(wf_read_tuple_angle(mod->id, key, 2, 5.0 )); */


/* 	  configs[i].ray_count = common_ray_count; */

	}
      
      PRINT_DEBUG1( "loaded %d ranger configs", scount );	  

      stg_model_set_cfg( mod, configs, scount * sizeof(stg_ranger_config_t) );
      
      free( configs );
    }
}

int ranger_raytrace_match( stg_model_t* mod, stg_model_t* hitmod )
{
  // Ignore myself, my children, and my ancestors.
  return( hitmod->ranger_return && !stg_model_is_related(mod,hitmod) );
}	


int ranger_update( stg_model_t* mod )
{     
  if( mod->subs < 1 )
    return 0;

  //PRINT_DEBUG1( "[%d] updating rangers", mod->world->sim_time );
  size_t len = mod->cfg_len;
  stg_ranger_config_t *cfg = (stg_ranger_config_t*)mod->cfg;

  if( len < sizeof(stg_ranger_config_t) )
    return 0; // nothing to see here
  
  int rcount = len / sizeof(stg_ranger_config_t);
  
  static stg_ranger_sample_t* ranges = NULL;
  static size_t old_len = 0;
  size_t data_len = sizeof(stg_ranger_sample_t) * rcount;
  
  if( old_len != data_len )
    {
      ranges = (stg_ranger_sample_t*)
	realloc( ranges, sizeof(stg_ranger_sample_t) * rcount );
      old_len = data_len;
    }
    
  memset( ranges, 0, data_len );

  if( fig_debug_rays ) stg_rtk_fig_clear( fig_debug_rays );

  int t;
  for( t=0; t<rcount; t++ )
    {
      // get the sensor's pose in global coords
      stg_pose_t pz;
      memcpy( &pz, &cfg[t].pose, sizeof(pz) ); 
      stg_model_local_to_global( mod, &pz );
      
      double range = cfg[t].bounds_range.max;
      
      int r;
      for( r=0; r<cfg[t].ray_count; r++ )
	{	  
	  double angle_per_ray = cfg[t].fov / cfg[t].ray_count;
	  double ray_angle = -cfg[t].fov/2.0 + angle_per_ray * r + angle_per_ray/2.0
	    ;
	  
	  itl_t* itl = itl_create( pz.x, pz.y, pz.a + ray_angle, 
				   cfg[t].bounds_range.max, 
				   mod->world->matrix, 
				   PointToBearingRange );
	  
	  stg_model_t * hitmod;
	  
	  hitmod = itl_first_matching( itl, ranger_raytrace_match, mod );
	  
	  if( hitmod )
	    {
	      //printf( "model %d %p   hit model %d %p\n",
	      //  mod->id, mod, hitmod->id, hitmod );
	      
	      if( itl->range < range )
		range = itl->range;
	    }
	     
	  itl_destroy( itl );
	} 
	  
      // low-threshold the range
      if( range < cfg[t].bounds_range.min )
	range = cfg[t].bounds_range.min;	
      
      ranges[t].range = range;
      //ranges[t].error = TODO;
    } 
  
  
  stg_model_set_data( mod, ranges, sizeof(stg_ranger_sample_t) * rcount );
  
  return 0;
}

/*
int ranger_noise_test( stg_ranger_sample_t* data, size_t count,  )
{
  int s;
  for( s=0; s<count; s++ )
    {
      // add 10mm random error
      ranges[s].range *= 0.1 * drand48();
    }
}
*/

int ranger_unrender_cfg( stg_model_t* mod, void* userp )
{
  stg_model_fig_clear( mod, "ranger_cfg_fig" );
  return 1; // quit callback
}

int ranger_render_cfg( stg_model_t* mod, void* userp )
{
  stg_ranger_config_t *cfg = (stg_ranger_config_t*)mod->cfg;
  int rcount = mod->cfg_len / sizeof( stg_ranger_config_t );

  if( cfg == NULL || rcount < 1 )
    return 0; // nothing to draw
  
  stg_rtk_fig_t* fig = stg_model_get_fig( mod, "ranger_cfg_fig" );

  if( !fig )
    {
      fig = stg_model_fig_create( mod, "ranger_cfg_fig", "top", STG_LAYER_RANGERCONFIG );
      stg_rtk_fig_color_rgb32( fig, 0x00AA00 ); // green
    }

  stg_geom_t geom;
  stg_model_get_geom(mod,&geom);
  
  stg_rtk_fig_clear(fig);
  stg_rtk_fig_origin( fig, geom.pose.x, geom.pose.y, geom.pose.a );

  
  // add rects showing ranger positions
  int s;
  for( s=0; s<rcount; s++ )
    {
      stg_ranger_config_t* rngr = &cfg[s];
      
      // sensor pose
      stg_rtk_fig_rectangle( fig,
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
      
      stg_rtk_fig_line( fig, rngr->pose.x, rngr->pose.y, x1, y1 );
      stg_rtk_fig_line( fig, rngr->pose.x, rngr->pose.y, x2, y2 );
      
      stg_rtk_fig_ellipse_arc( fig,
			       rngr->pose.x, rngr->pose.y, rngr->pose.a,
			       2.0*cfg->bounds_range.max,
			       2.0*cfg->bounds_range.max,
			       -da, da );
    }

  return 0;
}


int ranger_unrender_data( stg_model_t* mod, void* userp )
{
  stg_model_fig_clear( mod, "ranger_data_fig" );
  return 1; // quit callback
}


int ranger_render_data( stg_model_t* mod, void* userp )
{
  PRINT_DEBUG( "ranger render data" );
  
  stg_rtk_fig_t* fig = stg_model_get_fig( mod, "ranger_data_fig" );

  if( !fig )
    {
      fig = stg_model_fig_create( mod, "ranger_data_fig", "top", STG_LAYER_RANGERDATA);
      stg_rtk_fig_color_rgb32(fig, stg_lookup_color(STG_RANGER_COLOR) );
    }

  stg_rtk_fig_clear(fig);

  size_t clen = mod->cfg_len;
  stg_ranger_config_t* cfg = (stg_ranger_config_t*)mod->cfg;
  
  // any samples at all?
  if( clen < sizeof(stg_ranger_config_t) )
    return 0;
  
  int rcount = clen / sizeof( stg_ranger_config_t );
  
  if( rcount < 1 ) // no samples
    return 0;
  
  size_t dlen = mod->data_len;
  stg_ranger_sample_t *samples = (stg_ranger_sample_t*)mod->data;
  
  // iff we have the right amount of data
  if( dlen == rcount * sizeof(stg_ranger_sample_t) )
    {
      stg_geom_t geom;
      stg_model_get_geom(mod,&geom);
      
      stg_rtk_fig_origin( fig, geom.pose.x, geom.pose.y, geom.pose.a );
      
      // draw the range  beams
      int s;
      for( s=0; s<rcount; s++ )
	{
	  if( samples[s].range > 0.0 )
	    {
	      stg_ranger_config_t* rngr = &cfg[s];
	      
	      //stg_rtk_fig_arrow( fig,
	      //		 rngr->pose.x, rngr->pose.y, rngr->pose.a,
	      //		 samples[s].range, 0.02 );
	      	      
	      // sensor FOV
	      double sidelen = samples[s].range;
	      double da = rngr->fov/2.0;
	      
	      double x1= rngr->pose.x + sidelen*cos(rngr->pose.a - da );
	      double y1= rngr->pose.y + sidelen*sin(rngr->pose.a - da );
	      double x2= rngr->pose.x + sidelen*cos(rngr->pose.a + da );
	      double y2= rngr->pose.y + sidelen*sin(rngr->pose.a + da );
	      
	      stg_rtk_fig_line( fig, rngr->pose.x, rngr->pose.y, x1, y1 );
	      stg_rtk_fig_line( fig, rngr->pose.x, rngr->pose.y, x2, y2 );
	      
	      stg_rtk_fig_ellipse_arc( fig,
				       rngr->pose.x, rngr->pose.y, rngr->pose.a,
				       2.0*sidelen,
				       2.0*sidelen,
				       -da, da );
	    }
	}
    }
  else
    if( dlen > 0 )
      PRINT_WARN2( "data size doesn't match configuation (%d/%d bytes)",
		   (int)dlen,  (int)rcount * sizeof(stg_ranger_sample_t) );
  
  return 0; // keep running
}

