///////////////////////////////////////////////////////////////////////////
//
// File: model_fiducial.c
// Author: Richard Vaughan
// Date: 10 June 2004
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/model_fiducial.c,v $
//  $Author: rtv $
//  $Revision: 1.37 $
//
///////////////////////////////////////////////////////////////////////////

//#define DEBUG

#include <assert.h>
#include <math.h>
#include "stage_internal.h"
#include "gui.h"

extern stg_rtk_fig_t* fig_debug_rays;

#define STG_FIDUCIALS_MAX 64
#define STG_DEFAULT_FIDUCIAL_RANGEMIN 0
#define STG_DEFAULT_FIDUCIAL_RANGEMAXID 5
#define STG_DEFAULT_FIDUCIAL_RANGEMAXANON 8
#define STG_DEFAULT_FIDUCIAL_FOV DTOR(180)

const double STG_FIDUCIAL_WATTS = 10.0;

/** @defgroup model_fiducial Fiducial detector model
The fiducial model simulates a fiducial-detecting device.

<h2>Worldfile properties</h2>

@par Summary and default values

@verbatim
fiducialfinder
(
  # fiducialfinder properties
  range_min 0.0
  range_max 8.0
  range_max_id 5.0
  fov 180.0

  # model properties
  size [0 0]
)
@endverbatim

@par Details
- range_min float
  - the minimum range reported by the sensor, in meters. The sensor will detect objects closer than this, but report their range as the minimum.
- range_max float
  - the maximum range at which the sensor can detect a fiducial, in meters. The sensor may not be able to uinquely identify the fiducial, depending on the value of range_max_id.
- range_max_id float
  - the maximum range at which the sensor can detect the ID of a fiducial, in meters.
- fov float
  - the angular field of view of the scanner, in degrees. 

*/

void fiducial_load( stg_model_t* mod )
{
  stg_fiducial_config_t cfg;
  memset( &cfg, 0, sizeof(cfg) );
  
  cfg.min_range = wf_read_length(mod->id, "range_min", STG_DEFAULT_FIDUCIAL_RANGEMIN );
  cfg.max_range_anon = wf_read_length(mod->id, "range_max", STG_DEFAULT_FIDUCIAL_RANGEMAXANON );
  cfg.fov = wf_read_angle(mod->id, "fov", STG_DEFAULT_FIDUCIAL_FOV );
  cfg.max_range_id = wf_read_length(mod->id, "range_max_id", STG_DEFAULT_FIDUCIAL_RANGEMAXID );
  
  stg_model_set_config( mod, &cfg, sizeof(cfg));
}

int fiducial_startup( stg_model_t* mod );
int fiducial_shutdown( stg_model_t* mod );
int fiducial_update( stg_model_t* mod );
void fiducial_render_cfg( stg_model_t* mod );
void fiducial_render_data( stg_model_t* mod );

stg_model_t* stg_fiducial_create( stg_world_t* world, 
				  stg_model_t* parent, 
				  stg_id_t id, 
				  char* token )
{
  stg_model_t* mod = stg_model_create( world, parent, id, STG_MODEL_FIDUCIAL, token );
  
  // override the default methods
  mod->f_startup = fiducial_startup;
  mod->f_shutdown = fiducial_shutdown;
  mod->f_update = NULL; // installed at startup/shutdown
  mod->f_render_data = fiducial_render_data;
  mod->f_render_cfg = fiducial_render_cfg;
  mod->f_load = fiducial_load;

  stg_geom_t geom;
  memset( &geom, 0, sizeof(geom));
  geom.size.x = 0.0;
  geom.size.y = 0.0;
  stg_model_set_geom( mod, &geom );  
  
  stg_color_t color = stg_lookup_color( "magenta" );
  stg_model_set_color( mod, &color );
  
  mod->obstacle_return = 0;
  mod->laser_return = LaserTransparent;
  
  // start with no data
  stg_model_set_data( mod, NULL, 0 );

  // default parameters
  stg_fiducial_config_t cfg; 
  cfg.min_range = STG_DEFAULT_FIDUCIAL_RANGEMIN;
  cfg.max_range_anon = STG_DEFAULT_FIDUCIAL_RANGEMAXANON;
  cfg.max_range_id = STG_DEFAULT_FIDUCIAL_RANGEMAXID;
  cfg.fov = STG_DEFAULT_FIDUCIAL_FOV;  
  stg_model_set_config( mod, &cfg, sizeof(cfg) );

  return mod;
}

int fiducial_startup( stg_model_t* mod )
{
  PRINT_DEBUG( "fiducial startup" );  
  
  mod->f_update = fiducial_update;
  mod->watts = STG_FIDUCIAL_WATTS;
  
  return 0;
}

int fiducial_shutdown( stg_model_t* mod )
{
  mod->f_update = NULL;
  mod->watts = 0.0;

  // this will undrender the data
  stg_model_set_data( mod, NULL, 0 );
  
  return 0;
}


typedef struct 
{
  stg_model_t* mod;
  stg_pose_t pose;
  stg_fiducial_config_t cfg;
  GArray* fiducials;
} model_fiducial_buffer_t;


int fiducial_raytrace_match( stg_model_t* mod, stg_model_t* hitmod )
{
  // Ignore myself, my children, and my ancestors.
  return( !stg_model_is_related(mod,hitmod) );
}	


void model_fiducial_check_neighbor( gpointer key, gpointer value, gpointer user )
{
  model_fiducial_buffer_t* mfb = (model_fiducial_buffer_t*)user;
  stg_model_t* him = (stg_model_t*)value;
  
  //PRINT_DEBUG2( "checking model %s - he has fiducial value %d",
  //	him->token, him->fiducial_return );
		
  // dont' compare a model with itself, and don't consider models with
  // invalid returns
  if( him == mfb->mod || him->fiducial_return == FiducialNone ) 
    return; 

  stg_pose_t hispose;
  stg_model_get_global_pose( him, &hispose );
  
  double dx = hispose.x - mfb->pose.x;
  double dy = hispose.y - mfb->pose.y;
  
  // are we within range?
  double range = hypot( dy, dx );
  if( range > mfb->cfg.max_range_anon )
    {
      //PRINT_DEBUG1( "  but model %s is outside my range", him->token);
      return;
    }

  
  // is he in my field of view?
  double hisbearing = atan2( dy, dx );
  double dif = mfb->pose.a - hisbearing;

  if( fabs(NORMALIZE(dif)) > mfb->cfg.fov/2.0 )
    {
      //PRINT_DEBUG1( "  but model %s is outside my FOV", him->token);
      return;
    }
   
  // now check if we have line-of-sight
  itl_t *itl = itl_create( mfb->pose.x, mfb->pose.y,
			   hispose.x, hispose.y, 
			   him->world->matrix, PointToPoint );
  
  stg_model_t* hitmod = itl_first_matching( itl, fiducial_raytrace_match, mfb->mod );
  
  itl_destroy( itl );


  //PRINT_DEBUG( "finished raytracing" );
  
  //if( hitmod )
  //PRINT_DEBUG2( "I saw %s with fid %d",
  //	 hitmod->token, hitmod->fiducial_return );

  // if it was him, we can see him
  if( hitmod == him )
    {
      stg_geom_t hisgeom;
      stg_model_get_geom(him,&hisgeom);

      // record where we saw him and what he looked like
      stg_fiducial_t fid;      
      fid.range = range;
      fid.bearing = NORMALIZE( hisbearing - mfb->pose.a);
      fid.geom.x = hisgeom.size.x;
      fid.geom.y = hisgeom.size.y;
      fid.geom.a = NORMALIZE( hispose.a - mfb->pose.a);
      
      // if he's within ID range, get his fiducial.return value, else
      // we see value 0
      fid.id = range < mfb->cfg.max_range_id ? 
	him->fiducial_return : 0;

      //PRINT_DEBUG2( "adding %s's value %d to my list of fiducials",
      //	    him->token, him->fiducial_return );

      g_array_append_val( mfb->fiducials, fid );
    }
}


///////////////////////////////////////////////////////////////////////////
// Update the beacon data
//
int fiducial_update( stg_model_t* mod )
{
  PRINT_DEBUG( "fiducial update" );

  if( mod->subs < 1 )
    return 0;

  if( fig_debug_rays ) stg_rtk_fig_clear( fig_debug_rays );
  
  model_fiducial_buffer_t mfb;
  memset( &mfb, 0, sizeof(mfb) );
  
  mfb.mod = mod;
  
  size_t len = stg_model_get_config( mod, &mfb.cfg, sizeof(mfb.cfg) );
  assert(len==sizeof(mfb.cfg));
	 
  mfb.fiducials = g_array_new( FALSE, TRUE, sizeof(stg_fiducial_t) );
  stg_model_get_global_pose( mod, &mfb.pose );
  
  // for all the objects in the world
  g_hash_table_foreach( mod->world->models, model_fiducial_check_neighbor, &mfb );

  //PRINT_DEBUG2( "model %s saw %d fiducials", mod->token, mfb.fiducials->len );

  stg_model_set_data( mod, 
		  mfb.fiducials->data, 
		  mfb.fiducials->len * sizeof(stg_fiducial_t) );
  
  g_array_free( mfb.fiducials, TRUE );

  return 0;
}

void fiducial_render_data( stg_model_t* mod )
{ 
  if(  mod->gui.data  )
    stg_rtk_fig_clear( mod->gui.data);
  else // create the figure, store it in the model and keep a local pointer
    {
      mod->gui.data = stg_rtk_fig_create( mod->world->win->canvas,
				      mod->gui.top, STG_LAYER_NEIGHBORDATA );
      stg_rtk_fig_color_rgb32( mod->gui.data, stg_lookup_color( STG_FIDUCIAL_COLOR ) );
    }
  
  stg_fiducial_t fids[ STG_FIDUCIALS_MAX ];
  size_t max_len = sizeof(fids[0]) * STG_FIDUCIALS_MAX;
  size_t len = stg_model_get_data( mod, fids, max_len );
  
  int bcount =len / sizeof(stg_fiducial_t);
  
  char text[32];

  int b;
  for( b=0; b < bcount; b++ )
    {
      // the location of the target
      double pa = fids[b].bearing;
      double px = fids[b].range * cos(pa); 
      double py = fids[b].range * sin(pa);
      
      stg_rtk_fig_line(  mod->gui.data, 0, 0, px, py );	
      
      // the size and heading of the target
      double wx = fids[b].geom.x;
      double wy = fids[b].geom.y;
      double wa = fids[b].geom.a;
      
      stg_rtk_fig_rectangle( mod->gui.data, px, py, wa, wx, wy, 0);
      stg_rtk_fig_arrow( mod->gui.data, px, py, wa, wy, 0.10);
      
      if( fids[b].id > 0 )
	{
	  snprintf(text, sizeof(text), "  %d", fids[b].id);
	  stg_rtk_fig_text( mod->gui.data, px, py, pa, text);
	}
    }  
}

void fiducial_render_cfg( stg_model_t* mod )
{ 
  
  if( mod->gui.cfg  )
    stg_rtk_fig_clear(mod->gui.cfg);
  else // create the figure, store it in the model and keep a local pointer
    {
      mod->gui.cfg = stg_rtk_fig_create( mod->world->win->canvas,
				     mod->gui.top, STG_LAYER_NEIGHBORCONFIG );
      stg_rtk_fig_color_rgb32( mod->gui.cfg, stg_lookup_color( STG_FIDUCIAL_CFG_COLOR ));
    }
  
  stg_fiducial_config_t cfg;
  assert( stg_model_get_config( mod, &cfg, sizeof(cfg) ) 
	  == sizeof(stg_fiducial_config_t) );
  
  double mina = -cfg.fov / 2.0;
  double maxa = +cfg.fov / 2.0;
  
  double dx =  cfg.max_range_anon * cos(mina);
  double dy =  cfg.max_range_anon * sin(mina);
  double ddx = cfg.max_range_anon * cos(maxa);
  double ddy = cfg.max_range_anon * sin(maxa);
  
  stg_rtk_fig_line( mod->gui.cfg, 0,0, dx, dy );
  stg_rtk_fig_line( mod->gui.cfg, 0,0, ddx, ddy );
  
  // max range
  stg_rtk_fig_ellipse_arc( mod->gui.cfg,
		       0,0,0,
		       2.0*cfg.max_range_anon,
		       2.0*cfg.max_range_anon, 
		       mina, maxa );      
  
  // max range that IDs can be, er... identified	  
  stg_rtk_fig_ellipse_arc( mod->gui.cfg, 
		       0,0,0,
		       2.0*cfg.max_range_id,
		       2.0*cfg.max_range_id, 
		       mina, maxa );      
}

