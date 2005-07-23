///////////////////////////////////////////////////////////////////////////
//
// File: model_ranger.c
// Author: Richard Vaughan
// Date: 10 June 2004
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/model_ranger.c,v $
//  $Author: rtv $
//  $Revision: 1.53 $
//
///////////////////////////////////////////////////////////////////////////


#include <math.h>

//#define DEBUG


#include "stage_internal.h"
#include "gui.h"

extern stg_rtk_fig_t* fig_debug_rays;

#define STG_RANGER_DATA_MAX 64

#define STG_RANGER_WATTS 2.0 // ranger power consumption

/**
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



void ranger_load( stg_model_t* mod )
{
  // Load the geometry of a ranger array
  int scount = wf_read_int( mod->id, "scount", 0);
  if (scount > 0)
    {
      char key[256];
      stg_ranger_config_t* configs = (stg_ranger_config_t*)
	calloc( sizeof(stg_ranger_config_t), scount );
      
      stg_size_t common_size;
      common_size.x = wf_read_tuple_length(mod->id, "ssize", 0, 0.01 );
      common_size.y = wf_read_tuple_length(mod->id, "ssize", 1, 0.03 );
      
      double common_min = wf_read_tuple_length(mod->id, "sview", 0, 0.0);
      double common_max = wf_read_tuple_length(mod->id, "sview", 1, 5.0);
      double common_fov = wf_read_tuple_angle(mod->id, "sview", 2, 5.0);

      // set all transducers with the common settings
      int i;
      for(i = 0; i < scount; i++)
	{
	  configs[i].size.x = common_size.x;
	  configs[i].size.y = common_size.y;
	  configs[i].bounds_range.min = common_min;
	  configs[i].bounds_range.max = common_max;
	  configs[i].fov = common_fov;
	}

      // allow individual configuration of transducers
      for(i = 0; i < scount; i++)
	{
	  snprintf(key, sizeof(key), "spose[%d]", i);
	  configs[i].pose.x = wf_read_tuple_length(mod->id, key, 0, 0);
	  configs[i].pose.y = wf_read_tuple_length(mod->id, key, 1, 0);
	  configs[i].pose.a = wf_read_tuple_angle(mod->id, key, 2, 0);
	  
	  snprintf(key, sizeof(key), "ssize[%d]", i);
	  configs[i].size.x = wf_read_tuple_length(mod->id, key, 0, 0.01);
	  configs[i].size.y = wf_read_tuple_length(mod->id, key, 1, 0.05);
	  
	  snprintf(key, sizeof(key), "sview[%d]", i);
	  configs[i].bounds_range.min = 
	    wf_read_tuple_length(mod->id, key, 0, 0);
	  configs[i].bounds_range.max =   // set up sensible defaults

	    wf_read_tuple_length(mod->id, key, 1, 5.0);
	  configs[i].fov 
	    = DTOR(wf_read_tuple_angle(mod->id, key, 2, 5.0 ));
	}
      
      PRINT_DEBUG1( "loaded %d ranger configs", scount );	  

      stg_model_set_property( mod, "ranger_cfg", configs, scount * sizeof(stg_ranger_config_t) );
      
      free( configs );
    }
}

//void ranger_render_cfg( stg_model_t* mod );
//void ranger_render_data( stg_model_t* mod ) ;

int ranger_update( stg_model_t* mod );
int ranger_startup( stg_model_t* mod );
int ranger_shutdown( stg_model_t* mod );

int ranger_render_data( stg_model_t* mod, char* name, 
			void* data, size_t len, void* userp );
int ranger_unrender_data( stg_model_t* mod, char* name, 
			  void* data, size_t len, void* userp );
int ranger_render_cfg( stg_model_t* mod, char* name, 
		       void* data, size_t len, void* userp );

int ranger_init( stg_model_t* mod )
{
  // override the default methods
  mod->f_startup = ranger_startup;
  mod->f_shutdown = ranger_shutdown;
  mod->f_update = NULL; // installed on startup & shutdown
  mod->f_load = ranger_load;
  
  stg_model_set_property( mod, "ranger_data", NULL, 0 );
  
  // Set up sensible defaults
  stg_geom_t geom;
  memset( &geom, 0, sizeof(geom)); // no size
  stg_model_set_property( mod, "geom", &geom, sizeof(geom) );
  
  stg_color_t col = stg_lookup_color( STG_RANGER_CONFIG_COLOR );
  stg_model_set_property( mod, "color", &col, sizeof(col) );
  
  // remove the polygon: ranger has no body
  stg_model_set_property( mod, "polygons", NULL, 0 );


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
    }
  stg_model_set_property( mod, "ranger_cfg", cfg, cfglen );
  
  // adds a menu item and associated on-and-off callbacks
  stg_model_add_property_toggles( mod, "ranger_data", 
				  ranger_render_data, // called when toggled on
				  NULL,
				  ranger_unrender_data,
				  NULL,
				  "ranger data",
				  TRUE );
  
/*   stg_model_add_property_toggles( mod, "ranger_cfg",  */
/* 				  ranger_render_cfg, // called when toggled on */
/* 				  NULL, */
/* 				  stg_model_fig_clear_cb, // called when toggled off */
/* 				  gui->cfg, // arg to stg_fig_clear_cb */
/* 				  "ranger config", */
/* 				  TRUE ); */
  


  return 0;
}

int ranger_startup( stg_model_t* mod )
{
  PRINT_DEBUG( "ranger startup" );

  mod->f_update = ranger_update;
  //mod->watts = STG_RANGER_WATTS;

  return 0;
}


int ranger_shutdown( stg_model_t* mod )
{
  PRINT_DEBUG( "ranger shutdown" );

  mod->f_update = NULL;
  //mod->watts = 0.0; // stop consuming power
  
  // clear the data - this will unrender it too.
  stg_model_set_property( mod, "ranger_data", NULL, 0 );

  return 0;
}

int ranger_raytrace_match( stg_model_t* mod, stg_model_t* hitmod )
{
  stg_ranger_return_t* rr = 
    stg_model_get_property_fixed( hitmod, 
				  "ranger_return", 
				  sizeof(stg_ranger_return_t));
  
  // Ignore myself, my children, and my ancestors.
  return( *rr && !stg_model_is_related(mod,hitmod) );
}	


int ranger_update( stg_model_t* mod )
{     
  if( mod->subs < 1 )
    return 0;

  //PRINT_DEBUG1( "[%d] updating rangers", mod->world->sim_time );
  
  size_t len = 0;
  stg_ranger_config_t *cfg = 
    stg_model_get_property( mod, "ranger_cfg", &len );
  
  if( len < sizeof(stg_ranger_config_t) )
    return 0; // nothing to see here
  
  int rcount = len / sizeof(stg_ranger_config_t);
  
  stg_ranger_sample_t* ranges = (stg_ranger_sample_t*)
    calloc( sizeof(stg_ranger_sample_t), rcount );
  
  if( fig_debug_rays ) stg_rtk_fig_clear( fig_debug_rays );

  int t;
  for( t=0; t<rcount; t++ )
    {
      // get the sensor's pose in global coords
      stg_pose_t pz;
      memcpy( &pz, &cfg[t].pose, sizeof(pz) ); 
      stg_model_local_to_global( mod, &pz );
      
      // todo - use bounds_range.min

      itl_t* itl = itl_create( pz.x, pz.y, pz.a, 
			       cfg[t].bounds_range.max, 
			       mod->world->matrix, 
			       PointToBearingRange );
      
      stg_model_t * hitmod;
      double range = cfg[t].bounds_range.max;
      
      hitmod = itl_first_matching( itl, ranger_raytrace_match, mod );
      
      if( hitmod )
	{
	  //printf( "model %d %p   hit model %d %p\n",
	  //  mod->id, mod, hitmod->id, hitmod );
	  
	  range = itl->range;
	  
	  // low-threshold the range
	  if( range < cfg[t].bounds_range.min )
	    range = cfg[t].bounds_range.min;	
	}
      
      
      ranges[t].range = range;
      //ranges[t].error = TODO;

      itl_destroy( itl );
    }
  
  stg_model_set_property( mod, "ranger_data", ranges, sizeof(stg_ranger_sample_t) * rcount );
  
  free( ranges );

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

int ranger_render_cfg( stg_model_t* mod, char* name, void* data, size_t len, void* userp )
{
  stg_ranger_config_t *cfg = (stg_ranger_config_t*)data;
  int rcount = len / sizeof( stg_ranger_config_t );

  if( cfg == NULL || rcount < 1 )
    return 0; // nothing to draw
  
  stg_rtk_fig_t* fig = stg_model_get_fig( mod, "ranger_cfg_fig" );

  if( !fig )
    fig = stg_model_fig_create( mod, "ranger_cfg_fig", "top", STG_LAYER_RANGERCONFIG );
  
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


int ranger_unrender_data( stg_model_t* mod, char* name, void* data, size_t len, void* userp )
{ 
  stg_model_fig_clear( mod, "ranger_data_fig" );
  return 1; // quit callback
}


int ranger_render_data( stg_model_t* mod, char* name, void* data, size_t len, void* userp )
{ 
  PRINT_DEBUG( "ranger render data" );
  
  stg_rtk_fig_t* fig = stg_model_get_fig( mod, "ranger_data_fig" );

  if( !fig )
    fig = stg_model_fig_create( mod, "ranger_data_fig", "top", STG_LAYER_RANGERDATA );

  stg_rtk_fig_clear(fig);

  size_t clen=0;
  stg_ranger_config_t* cfg = stg_model_get_property(mod, "ranger_cfg", &clen );
  
  // any samples at all?
  if( clen < sizeof(stg_ranger_config_t) )
    return 0;
  
  int rcount = clen / sizeof( stg_ranger_config_t );
  
  if( rcount < 1 ) // no samples 
    return 0; 
  
  size_t dlen = 0;
  stg_ranger_sample_t *samples = 
    stg_model_get_property( mod, "ranger_data", &dlen );
  
  // iff we have the right amount of data
  if( dlen == rcount * sizeof(stg_ranger_sample_t) )
    {       
      stg_geom_t geom;
      stg_model_get_geom(mod,&geom);
      
      stg_rtk_fig_color_rgb32(fig, stg_lookup_color(STG_RANGER_COLOR) );
      stg_rtk_fig_origin( fig, geom.pose.x, geom.pose.y, geom.pose.a );	  
      
      // draw the range  beams
      int s;
      for( s=0; s<rcount; s++ )
	{
	  if( samples[s].range > 0.0 )
	    {
	      stg_ranger_config_t* rngr = &cfg[s];
	      
	      stg_rtk_fig_arrow( fig, 
				 rngr->pose.x, rngr->pose.y, rngr->pose.a, 	
				 samples[s].range, 0.02 );
	    }
	}
    }
  else
    if( dlen > 0 )
      PRINT_WARN2( "data size doesn't match configuation (%d/%d bytes)",
		   dlen,  rcount * sizeof(stg_ranger_sample_t) );
  
  return 0; // keep running
}

