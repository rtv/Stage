///////////////////////////////////////////////////////////////////////////
//
// File: model_bumper.c
// Author: Richard Vaughan
// Date: 28 March 2006
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/model_bumper.c,v $
//  $Author: rtv $
//  $Revision: 1.1 $
//
///////////////////////////////////////////////////////////////////////////

/**
@ingroup model
@defgroup model_bumper Bumper/Whisker  model 
The bumper model simulates an array of binary touch sensors.

<h2>Worldfile properties</h2>

@par Summary and default values

@verbatim
bumper 
(
  # bumper properties
  bcount 1
  bpose[0] [ 0 0 0 ]
  blength 0.1
)
@endverbatim

@par Notes

The bumper model allows configuration of the pose and length parameters of each transducer seperately using bpose[index] and blength[index]. For convenience, the length of all bumpers in the array can be set at once with the blength property. If a blength with no index is specified, this global setting is applied first, then specific blengh[index] properties are applied afterwards. Note that the order in the worldfile is ignored.

@par Details
- bcount int 
  - the number of bumper transducers
- bpose[\<transducer index\>] [float float float]
  - [x y theta] 
  - pose of the center of the transducer relative to its parent.
- blength float
  - sets the length in meters of all transducers in the array
- blength[\<transducer index\>] float
  - length in meters of a specific transducer. This is applied after the global setting above.

*/

#include <math.h>
#include "stage_internal.h"
#include "gui.h"

extern stg_rtk_fig_t* fig_debug_rays;

static const stg_watts_t STG_BUMPER_WATTS = 0.1; // bumper power consumption

static const stg_color_t STG_BUMPER_HIT_RGB = 0xFF0000; // red
static const stg_color_t STG_BUMPER_NOHIT_RGB = 0x00FF00; // green
static const stg_meters_t STG_BUMPER_HIT_THICKNESS = 0.02;
static const stg_meters_t STG_BUMPER_NOHIT_THICKNESS = 0.01;

static int bumper_update( stg_model_t* mod );
static int bumper_startup( stg_model_t* mod );
static int bumper_shutdown( stg_model_t* mod );
static void bumper_load( stg_model_t* mod );
static int bumper_render_data( stg_model_t* mod, void* userp );
static int bumper_unrender_data( stg_model_t* mod,void* userp );

int bumper_init( stg_model_t* mod )
{
  // override the default methods
  mod->f_startup = bumper_startup;
  mod->f_shutdown = bumper_shutdown;
  mod->f_update = NULL; // installed on startup & shutdown
  mod->f_load = bumper_load;
  
  stg_model_set_data( mod, NULL, 0 );
  
  // Set up sensible defaults
  stg_geom_t geom;
  memset( &geom, 0, sizeof(geom)); // no size
  stg_model_set_geom( mod, &geom );
    
  // remove the polygon: bumper has no body
  stg_model_set_polygons( mod, NULL, 0 );

  stg_bumper_config_t cfg;
  memset( &cfg, 0, sizeof(cfg));
  cfg.length = 0.1;
  stg_model_set_cfg( mod, &cfg, sizeof(cfg) );
  
  // adds a menu item and associated on-and-off callbacks
  stg_model_add_property_toggles( mod, 
				  &mod->data,
				  bumper_render_data, // called when toggled on
				  NULL,
				  bumper_unrender_data, // called when toggled off
				  NULL,
				  "bumperdata",
				  "bumper data",
				  TRUE );  // initial state

  return 0;
}

int bumper_startup( stg_model_t* mod )
{
  PRINT_DEBUG( "bumper startup" );

  mod->f_update = bumper_update;

  stg_model_set_watts( mod, STG_BUMPER_WATTS );

  return 0;
}


int bumper_shutdown( stg_model_t* mod )
{
  PRINT_DEBUG( "bumper shutdown" );

  mod->f_update = NULL;
  stg_model_set_watts( mod, 0 );
  
  // clear the data - this will unrender it too.
  stg_model_set_data( mod, NULL, 0 );

  return 0;
}

void bumper_load( stg_model_t* mod )
{
  // Load the geometry of a bumper array
  int bcount = wf_read_int( mod->id, "bcount", 0);
  if (bcount > 0)
    {
      char key[256];
      
      size_t cfglen = bcount * sizeof(stg_bumper_config_t); 
      stg_bumper_config_t* cfg = (stg_bumper_config_t*)
	calloc( sizeof(stg_bumper_config_t), bcount ); 
      
      double common_length = wf_read_length(mod->id, "blength", cfg[0].length );

      // set all transducers with the common setting
      int i;
      for(i = 0; i < bcount; i++)
	{
	  cfg[i].length = common_length;
	}

      // allow individual configuration of transducers
      for(i = 0; i < bcount; i++)
	{
	  snprintf(key, sizeof(key), "bpose[%d]", i);
	  cfg[i].pose.x = wf_read_tuple_length(mod->id, key, 0, 0);
	  cfg[i].pose.y = wf_read_tuple_length(mod->id, key, 1, 0);
	  cfg[i].pose.a = wf_read_tuple_angle(mod->id, key, 2, 0);	 

	  snprintf(key, sizeof(key), "blength[%d]", i);
	  cfg[i].length = wf_read_length(mod->id, key, common_length );	 
	}
      
      stg_model_set_cfg( mod, cfg, cfglen );      
      free( cfg );
    }
}

int bumper_raytrace_match( stg_model_t* mod, stg_model_t* hitmod )
{
  // Ignore myself, my children, and my ancestors.
  return( hitmod->obstacle_return && !stg_model_is_related(mod,hitmod) );
}	


int bumper_update( stg_model_t* mod )
{     
  //if( mod->subs < 1 )
  //return 0;

  //PRINT_DEBUG1( "[%d] updating bumpers", mod->world->sim_time );
  size_t len = mod->cfg_len;
  stg_bumper_config_t *cfg = (stg_bumper_config_t*)mod->cfg;

  if( len < sizeof(stg_bumper_config_t) )
    return 0; // nothing to see here
  
  int rcount = len / sizeof(stg_bumper_config_t);
  
  static stg_bumper_sample_t* data = NULL;
  static size_t last_datalen = 0;

  size_t datalen = sizeof(stg_bumper_sample_t) *  rcount; 

  if( datalen != last_datalen )
    {
      data = (stg_bumper_sample_t*)realloc( data, datalen );
      last_datalen = datalen;
    }

  memset( data, 0, datalen );

  if( fig_debug_rays ) stg_rtk_fig_clear( fig_debug_rays );
  
  int t;
  for( t=0; t<rcount; t++ )
    {
      // get the sensor's pose in global coords
      stg_pose_t pz;
      memcpy( &pz, &cfg[t].pose, sizeof(pz) );

      pz.a = pz.a + M_PI/2.0;
      pz.x -= cfg[t].length/2.0 * cos(pz.a);
      //pz.y -= cfg[t].length/2.0 * sin(cfg[t].pose.a );
      
      stg_model_local_to_global( mod, &pz );
      
      itl_t* itl = itl_create( pz.x, pz.y, pz.a,
			       cfg[t].length,
			       mod->world->matrix,
			       PointToBearingRange );
      
      stg_model_t * hitmod = 
	itl_first_matching( itl, bumper_raytrace_match, mod );
      
      if( hitmod )
	{
	  data[t].hit = hitmod;
	  data[t].hit_point.x = itl->x;
	  data[t].hit_point.y = itl->y;
	}

      itl_destroy( itl );
    }
  
  stg_model_set_data( mod, data, datalen );

  return 0;
}

int bumper_unrender_data( stg_model_t* mod, void* userp )
{
  stg_model_fig_clear( mod, "bumper_data_fig" );
  return 1; // quit callback
}

int bumper_render_data( stg_model_t* mod, void* userp )
{
  PRINT_DEBUG( "bumper render data" );
  
  stg_rtk_fig_t* fig = stg_model_get_fig( mod, "bumper_data_fig" );

  if( !fig )
    {
      fig = stg_model_fig_create( mod, "bumper_data_fig", "top", STG_LAYER_BUMPERDATA );
    }

  stg_rtk_fig_clear(fig);
  
  stg_bumper_config_t* cfg = (stg_bumper_config_t*)mod->cfg;  
  int rcount =  mod->cfg_len / sizeof( stg_bumper_config_t );
  if( rcount < 1 ) // no samples
    return 0;
  
  size_t dlen = mod->data_len;
  stg_bumper_sample_t *data = (stg_bumper_sample_t*)mod->data;
  
  // iff we have the right amount of data
  if( dlen == rcount * sizeof(stg_bumper_sample_t) )
    {
      int s;
      for( s=0; s<rcount; s++ )
	{
	  stg_meters_t thick = STG_BUMPER_NOHIT_THICKNESS;
	  stg_color_t col = STG_BUMPER_NOHIT_RGB;
	  
	  if( data[s].hit )
	    {
	      col = STG_BUMPER_HIT_RGB;
	      thick = STG_BUMPER_HIT_THICKNESS;
	    }
	  
	  stg_rtk_fig_color_rgb32(fig, col );	      
	  stg_rtk_fig_rectangle( fig,
				 cfg[s].pose.x,
				 cfg[s].pose.y,
				 cfg[s].pose.a - M_PI/2.0,
				 cfg[s].length,
				 thick,
				 1 );
	  
	  stg_rtk_fig_color_rgb32(fig, 0 ); //black	      
	  stg_rtk_fig_rectangle( fig,
				 cfg[s].pose.x,
				 cfg[s].pose.y,
				 cfg[s].pose.a - M_PI/2.0,
				 cfg[s].length,
				 thick,
				 0 );
	  
	}
    }
  else
    if( dlen > 0 )
      PRINT_WARN2( "data size doesn't match configuation (%d/%d bytes)",
		   (int)dlen,  (int)rcount * sizeof(stg_bumper_sample_t) );
  
  return 0; // keep running
}

