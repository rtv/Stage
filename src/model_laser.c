///////////////////////////////////////////////////////////////////////////
//
// File: model_laser.c
// Author: Richard Vaughan
// Date: 10 June 2004
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/model_laser.c,v $
//  $Author: rtv $
//  $Revision: 1.63 $
//
///////////////////////////////////////////////////////////////////////////


#include <sys/time.h>
#include <math.h>
#include "gui.h"

//#define DEBUG

#include "stage_internal.h"
extern rtk_fig_t* fig_debug_rays;

#define TIMING 0
#define LASER_FILLED 1

#define STG_LASER_SAMPLES_MAX 1024
#define STG_DEFAULT_LASER_SIZEX 0.15
#define STG_DEFAULT_LASER_SIZEY 0.15
#define STG_DEFAULT_LASER_MINRANGE 0.0
#define STG_DEFAULT_LASER_MAXRANGE 8.0
#define STG_DEFAULT_LASER_FOV M_PI
#define STG_DEFAULT_LASER_SAMPLES 180

/** 
@defgroup model_laser Laser model 
The laser model simulates a scanning laser rangefinder

<h2>Worldfile properties</h2>

@par Summary and default values

@verbatim
laser
(
  # laser properties
  samples 180
  range_min 0.0
  range_max 8.0
  fov 180.0

  # model properties
  size [0.15 0.15]
  color "blue"
)
@endverbatim

@par Details
- samples int
  - the number of laser samples per scan
- range_min float
  -  the minimum range reported by the scanner, in meters. The scanner will detect objects closer than this, but report their range as the minimum.
- range_max float
  - the maximum range reported by the scanner, in meters. The scanner will not detect objects beyond this range.
- fov float
  - the angular field of view of the scanner, in degrees. 

*/

void laser_load( stg_model_t* mod )
{
  stg_laser_config_t lconf;
  memset( &lconf, 0, sizeof(lconf) );
  
  lconf.samples = wf_read_int( mod->id, "samples", STG_DEFAULT_LASER_SAMPLES);
  lconf.range_min = wf_read_length( mod->id, "range_min", STG_DEFAULT_LASER_MINRANGE);
  lconf.range_max = wf_read_length( mod->id, "range_max", STG_DEFAULT_LASER_MAXRANGE);
  lconf.fov = wf_read_angle( mod->id, "fov", STG_DEFAULT_LASER_FOV);

  stg_model_set_config( mod, &lconf, sizeof(lconf));
}

int laser_update( stg_model_t* mod );
int laser_shutdown( stg_model_t* mod );
void laser_render_data(  stg_model_t* mod );
void laser_render_cfg( stg_model_t* mod );

stg_model_t* stg_laser_create( stg_world_t* world, 
			       stg_model_t* parent, 
			       stg_id_t id, 
			       char* token )
{
  stg_model_t* mod = 
    stg_model_create( world, parent, id, STG_MODEL_LASER, token );
  
  // override the default methods
  mod->f_shutdown = laser_shutdown;
  mod->f_update = laser_update;
  mod->f_render_data = laser_render_data;
  mod->f_render_cfg = laser_render_cfg;
  mod->f_load = laser_load;

  // sensible laser defaults
  stg_geom_t geom;
  geom.pose.x = 0.0;
  geom.pose.y = 0.0;
  geom.pose.a = 0.0;
  geom.size.x = STG_DEFAULT_LASER_SIZEX;
  geom.size.y = STG_DEFAULT_LASER_SIZEY;
  stg_model_set_geom( mod, &geom );

  // set up a laser-specific config structure
  stg_laser_config_t lconf;
  memset(&lconf,0,sizeof(lconf));  
  lconf.range_min   = STG_DEFAULT_LASER_MINRANGE;
  lconf.range_max   = STG_DEFAULT_LASER_MAXRANGE;
  lconf.fov         = STG_DEFAULT_LASER_FOV;
  lconf.samples     = STG_DEFAULT_LASER_SAMPLES;  
  stg_model_set_config( mod, &lconf, sizeof(lconf) );

  // set default color
  stg_color_t col = stg_lookup_color( STG_LASER_GEOM_COLOR ); 
  stg_model_set_color( mod, &col );

  return mod;
}



int laser_raytrace_match( stg_model_t* mod, stg_model_t* hitmod )
{
  // Ignore myself, my children, and my ancestors.
  if( (!stg_model_is_related(mod,hitmod))  &&  
      hitmod->laser_return != LaserTransparent) 
    return 1;
  
  // Stop looking when we see something
  //hisreturn = hitmdmodel_laser_return(hitmod);
  
  return 0; // no match
}	

int laser_update( stg_model_t* mod )
{   
  PRINT_DEBUG1( "[%lu] laser update", mod->world->sim_time );
  
  // no work to do if we're unsubscribed
  if( mod->subs < 1 )
    return 0;
    
  stg_laser_config_t cfg;
  stg_model_get_config( mod, &cfg, sizeof(cfg));
  
  stg_geom_t geom;
  stg_model_get_geom( mod, &geom );

  // get the sensor's pose in global coords
  stg_pose_t pz;
  memcpy( &pz, &geom.pose, sizeof(pz) ); 
  stg_model_local_to_global( mod, &pz );

  PRINT_DEBUG3( "laser origin %.2f %.2f %.2f", pz.x, pz.y, pz.a );

  double sample_incr = cfg.fov / (double)cfg.samples;
  
  double bearing = pz.a - cfg.fov/2.0;
  
#if TIMING
  struct timeval tv1, tv2;
  gettimeofday( &tv1, NULL );
#endif
      
  if( fig_debug_rays ) rtk_fig_clear( fig_debug_rays );

  // make a scan buffer (static for speed, so we only have to allocate
  // memory when the number of samples changes).
  static stg_laser_sample_t* scan = 0;
  scan = realloc( scan, sizeof(stg_laser_sample_t) * cfg.samples );
  
  int t;
  // only compute every second sample, for speed
  //for( t=0; t<cfg.samples-1; t+=2 )
  for( t=0; t<cfg.samples; t++ )
    {
      
      itl_t* itl = itl_create( pz.x, pz.y, bearing, 
			       cfg.range_max, 
			       mod->world->matrix, 
			       PointToBearingRange );
      
      bearing += sample_incr;
      
      stg_model_t* hitmod;
      double range = cfg.range_max;
      //stg_laser_return_t hisreturn = LaserVisible;
      
      hitmod = itl_first_matching( itl, laser_raytrace_match, mod );

      if( hitmod )
	range = itl->range;

      //printf( "%d:%.2f  ", t, range );

      if( range < cfg.range_min )
	range = cfg.range_min;
            
      // record the range in mm
      //scan[t+1].range = 
	scan[t].range = (uint32_t)( range * 1000.0 );
      // if the object is bright, it has a non-zero reflectance
      //scan[t+1].reflectance = 
	scan[t].reflectance = 
	  (hitmod && (hitmod->laser_return >= LaserBright)) ? 1 : 0;

      itl_destroy( itl );
    }
  
  // new style
  stg_model_set_data( mod, scan, sizeof(stg_laser_sample_t) * cfg.samples );


#if TIMING
  gettimeofday( &tv2, NULL );
  printf( " laser data update time %.6f\n",
	  (tv2.tv_sec + tv2.tv_usec / 1e6) - 
	  (tv1.tv_sec + tv1.tv_usec / 1e6) );	    
#endif
  
  // laser costs some energy
  //model_energy_consume( mod, STG_ENERGY_COST_LASER );

  return 0; //ok
}

void laser_render_data(  stg_model_t* mod )
{
  // allocate this buffer just once - hopefully that's faster than
  // doing it each call.
  static stg_laser_sample_t* samples = NULL;
  static size_t max_len = STG_LASER_SAMPLES_MAX * sizeof(stg_laser_sample_t );
  
  if( samples == NULL )
    samples = (stg_laser_sample_t*)malloc( max_len );
  
  if( mod->gui.data  )
    rtk_fig_clear(mod->gui.data);
  else 
    {
      mod->gui.data = rtk_fig_create( mod->world->win->canvas,
				      NULL, STG_LAYER_LASERDATA );
      
      rtk_fig_color_rgb32( mod->gui.data, stg_lookup_color(STG_LASER_COLOR) );
    }
  
  if( mod->gui.data_bg )
    rtk_fig_clear( mod->gui.data_bg );
  else // create the data background
    {
      mod->gui.data_bg = rtk_fig_create( mod->world->win->canvas,
					 mod->gui.data, STG_LAYER_BACKGROUND );      
      rtk_fig_color_rgb32( mod->gui.data_bg, 
			   stg_lookup_color( STG_LASER_FILL_COLOR ));
    }
  
  stg_laser_config_t cfg;
  assert( stg_model_get_config( mod, &cfg, sizeof(cfg))
	  == sizeof(stg_laser_config_t ));
    
  // get the data
  size_t len = stg_model_get_data( mod, samples, max_len );
  
  int sample_count = len / sizeof(stg_laser_sample_t);

  // now get on with rendering the laser data
  stg_pose_t pose;
  stg_model_get_global_pose( mod, &pose );
  
  stg_geom_t geom;
  stg_model_get_geom( mod, &geom );
  
  double sample_incr = cfg.fov / sample_count;
  double bearing = geom.pose.a - cfg.fov/2.0;
  stg_point_t* points = calloc( sizeof(stg_point_t), sample_count + 1 );
  
  rtk_fig_origin( mod->gui.data, pose.x, pose.y, pose.a );  
  rtk_fig_color_rgb32( mod->gui.data, stg_lookup_color(STG_LASER_BRIGHT_COLOR) );  
  int s;
  for( s=0; s<sample_count; s++ )
    {
      // useful debug
      //rtk_fig_arrow( fig, 0, 0, bearing, (sample->range/1000.0), 0.01 );
      
      points[1+s].x = (samples[s].range/1000.0) * cos(bearing);
      points[1+s].y = (samples[s].range/1000.0) * sin(bearing);
      bearing += sample_incr;
    }
  
  // hmm, what's the right cast to get rid of the compiler warning
  // for the points argument? the function expects a double[][2] type. 
  
  if( mod->world->win->fill_polygons )
    rtk_fig_polygon( mod->gui.data_bg, 0,0,0, sample_count+1, points, TRUE );
  
  rtk_fig_polygon( mod->gui.data, 0,0,0, sample_count+1, points, FALSE ); 	
  
  // loop through again, drawing bright boxes on top of the polygon
  for( s=0; s<sample_count; s++ )
    {      
      // if this hit point is bright, we draw a little box
      if( samples[s].reflectance > 0 )
	{
	  rtk_fig_color_rgb32( mod->gui.data, stg_lookup_color(STG_LASER_BRIGHT_COLOR) );
	  rtk_fig_rectangle( mod->gui.data, 
			     points[1+s].x, points[1+s].y, 0,
			     0.04, 0.04, 1 );
	  rtk_fig_color_rgb32( mod->gui.data, stg_lookup_color(STG_LASER_COLOR) );
	}
    }
  
  free( points );
}

void laser_render_cfg( stg_model_t* mod )
{ 
  if( mod->gui.cfg  )
    rtk_fig_clear(mod->gui.cfg);
  else // create the figure, store it in the model and keep a local pointer
    mod->gui.cfg = rtk_fig_create( mod->world->win->canvas, 
				   mod->gui.top, STG_LAYER_LASERCONFIG );
  
  // draw the FOV and range lines
  rtk_fig_color_rgb32( mod->gui.cfg, stg_lookup_color( STG_LASER_CFG_COLOR ));
  
  // get the config and make sure it's the right size
  stg_laser_config_t cfg;
  size_t len = stg_model_get_config( mod, &cfg, sizeof(cfg));
  assert( len == sizeof(cfg) );

  double mina = cfg.fov / 2.0;
  double maxa = -cfg.fov / 2.0;
  
  double leftfarx = cfg.range_max * cos(mina);
  double leftfary = cfg.range_max * sin(mina);
  double rightfarx = cfg.range_max * cos(maxa);
  double rightfary = cfg.range_max * sin(maxa);  
  double leftnearx = cfg.range_min * cos(mina);  
  double leftneary = cfg.range_min * sin(mina);
  double rightnearx = cfg.range_min * cos(maxa);
  double rightneary = cfg.range_min * sin(maxa);
  
  rtk_fig_line( mod->gui.cfg, leftnearx, leftneary, leftfarx, leftfary );
  rtk_fig_line( mod->gui.cfg, rightnearx, rightneary, rightfarx, rightfary );
  
  rtk_fig_ellipse_arc( mod->gui.cfg,0,0,0, 
		       2.0*cfg.range_max,
		       2.0*cfg.range_max, 
		       mina, maxa );      
  
  rtk_fig_ellipse_arc( mod->gui.cfg,0,0,0, 
		       2.0*cfg.range_min,
		       2.0*cfg.range_min, 
		       mina, maxa );      
}

int laser_shutdown( stg_model_t* mod )
{ 
  // clear the data - this will unrender it too
  stg_model_set_data( mod, NULL, 0 );
  return 0; // ok
}

