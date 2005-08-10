///////////////////////////////////////////////////////////////////////////
//
// File: model_laser.c
// Author: Richard Vaughan
// Date: 10 June 2004
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/model_laser.c,v $
//  $Author: rtv $
//  $Revision: 1.76 $
//
///////////////////////////////////////////////////////////////////////////


#include <sys/time.h>
#include <math.h>
#include "gui.h"

//#define DEBUG

#include "stage_internal.h"
extern stg_rtk_fig_t* fig_debug_rays;

#define TIMING 0
#define LASER_FILLED 1

#define STG_LASER_WATTS 17.5 // laser power consumption

#define STG_LASER_SAMPLES_MAX 1024
#define STG_DEFAULT_LASER_SIZEX 0.15
#define STG_DEFAULT_LASER_SIZEY 0.15
#define STG_DEFAULT_LASER_MINRANGE 0.0
#define STG_DEFAULT_LASER_MAXRANGE 8.0
#define STG_DEFAULT_LASER_FOV M_PI
#define STG_DEFAULT_LASER_SAMPLES 180

/**
@ingroup model
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

/** 
@ingroup stg_model_laser
@ingroup stg_model_props
@defgroup stg_model_laser_props Laser Properties

- "laser_cfg" stg_laser_config_t
- "laser_data" stg_laser_sample_t[]
*/

void laser_load( stg_model_t* mod )
{
  //if( wf_property_exists( mod->id, "

  stg_laser_config_t* now = 
    stg_model_get_property_fixed( mod, "laser_cfg", sizeof(stg_laser_config_t)); 

  stg_laser_config_t lconf;
  memset( &lconf, 0, sizeof(lconf));

  lconf.samples   = wf_read_int( mod->id, "samples", now ? now->samples : 361 );
  lconf.range_min = wf_read_length( mod->id, "range_min", now ? now->range_min : 0.0 );
  lconf.range_max = wf_read_length( mod->id, "range_max", now ? now->range_max : 8.0 );
  lconf.fov       = wf_read_angle( mod->id, "fov",  now ? now->fov : M_PI );
  
  stg_model_set_property( mod, "laser_cfg", &lconf, sizeof(lconf));
}

int laser_update( stg_model_t* mod );
int laser_startup( stg_model_t* mod );
int laser_shutdown( stg_model_t* mod );

int laser_render_data( stg_model_t* mod, char* name, 
		       void* data, size_t len, void* userp );
int laser_unrender_data( stg_model_t* mod, char* name,
			 void* data, size_t len, void* userp );
int laser_render_cfg( stg_model_t* mod, char* name, 
		      void* data, size_t len, void* userp );
//int laser_unrender_cfg( stg_model_t* mod, char* name,
//		void* data, size_t len, void* userp );


// not looking up color strings every redraw makes Stage 6% faster! (scanf is slow)
static int init = 0;
static stg_color_t laser_color=0, bright_color=0, fill_color=0, cfg_color=0, geom_color=0;

int laser_init( stg_model_t* mod )
{
  // we do this just the first time a laser is created
  if( init == 0 )
    {
      laser_color =   stg_lookup_color(STG_LASER_COLOR);
      bright_color = stg_lookup_color(STG_LASER_BRIGHT_COLOR);
      fill_color = 0xF0F0FF;// stg_lookup_color(STG_LASER_FILL_COLOR);
      geom_color = stg_lookup_color(STG_LASER_GEOM_COLOR);
      cfg_color = stg_lookup_color(STG_LASER_CFG_COLOR);
      init = 1;
    }

  // we don't consume any power until subscribed
  //mod->watts = 0.0; 
  
  // override the default methods
  mod->f_startup = laser_startup;
  mod->f_shutdown = laser_shutdown;
  mod->f_update =  NULL; // laser_update is installed startup, removed on shutdown
  mod->f_load = laser_load;

  // sensible laser defaults 
  stg_geom_t geom; 
  geom.pose.x = 0.0;
  geom.pose.y = 0.0;
  geom.pose.a = 0.0;
  geom.size.x = STG_DEFAULT_LASER_SIZEX;
  geom.size.y = STG_DEFAULT_LASER_SIZEY;
  stg_model_set_property( mod, "geom", &geom, sizeof(geom) );
      
  
  // create a single rectangle body 
  stg_polygon_t* square = stg_unit_polygon_create();
  stg_model_set_property( mod, "polygons", square, sizeof(stg_polygon_t));


  // set up a laser-specific config structure
  stg_laser_config_t lconf;
  memset(&lconf,0,sizeof(lconf));  
  lconf.range_min   = STG_DEFAULT_LASER_MINRANGE;
  lconf.range_max   = STG_DEFAULT_LASER_MAXRANGE;
  lconf.fov         = STG_DEFAULT_LASER_FOV;
  lconf.samples     = STG_DEFAULT_LASER_SAMPLES;  
  stg_model_set_property( mod, "laser_cfg", &lconf, sizeof(lconf) );
  
  
  // set default color
  stg_model_set_property( mod, "color", &geom_color, sizeof(geom_color));
  
  // when data is set, render it to the GUI
  // clear the data - this will unrender it too
  stg_model_set_property( mod, "laser_data", NULL, 0 );

  // adds a menu item and associated on-and-off callbacks
  stg_model_add_property_toggles( mod, "laser_data", 
				  laser_render_data, // called when toggled on
				  NULL, 
				  laser_unrender_data, // called when toggled off
				  NULL, 
				  "laser data",
				  TRUE );

  // TODO
  /* stg_model_add_property_toggles( mod, "laser_cfg",  */
/* 				  laser_render_cfg, // called when toggled on */
/* 				  NULL, */
/* 				  NULL,//stg_fig_clear_cb, */
/* 				  NULL, //gui->cfg,  */
/* 				  "laser config", */
/* 				  TRUE ); */
  
  return 0;
}


int laser_raytrace_match( stg_model_t* mod, stg_model_t* hitmod )
{           
  stg_laser_return_t* lr = 
    stg_model_get_property_fixed( hitmod, 
				       "laser_return", 
				       sizeof(stg_laser_return_t));
  
  
  // Ignore myself, my children, and my ancestors.
  if( (!stg_model_is_related(mod,hitmod))  &&  
      *lr != LaserTransparent) 
    return 1;
  
  // Stop looking when we see something
  //hisreturn = hitmdmodel_laser_return(hitmod);
  
  return 0; // no match
}	

int laser_update( stg_model_t* mod )
{   
  //puts( "laser update" );

  PRINT_DEBUG2( "[%lu] laser update (%d subs)", mod->world->sim_time, mod->subs );
  
  // no work to do if we're unsubscribed
  if( mod->subs < 1 )
    return 0;
  
  stg_laser_config_t* cfg = 
    stg_model_get_property_fixed( mod, "laser_cfg", sizeof(stg_laser_config_t));
  assert(cfg);

  stg_geom_t geom;
  stg_model_get_geom( mod, &geom );

  // get the sensor's pose in global coords
  stg_pose_t pz;
  memcpy( &pz, &geom.pose, sizeof(pz) ); 
  stg_model_local_to_global( mod, &pz );

  PRINT_DEBUG3( "laser origin %.2f %.2f %.2f", pz.x, pz.y, pz.a );

  double sample_incr = cfg->fov / (double)cfg->samples;
  
  double bearing = pz.a - cfg->fov/2.0;
  
#if TIMING
  struct timeval tv1, tv2;
  gettimeofday( &tv1, NULL );
#endif
      
  if( fig_debug_rays ) stg_rtk_fig_clear( fig_debug_rays );

  // make a scan buffer (static for speed, so we only have to allocate
  // memory when the number of samples changes).
  static stg_laser_sample_t* scan = 0;
  scan = realloc( scan, sizeof(stg_laser_sample_t) * cfg->samples );
  
  int t;
  // only compute every second sample, for speed
  //for( t=0; t<cfg.samples-1; t+=2 )
  

  //memset( scan, 0, sizeof(stg_laser_sample_t) * cfg.samples );
  //for( t=0; t<1; t++ )
  for( t=0; t<cfg->samples; t++ )
    {
      
      itl_t* itl = itl_create( pz.x, pz.y, bearing, 
			       cfg->range_max, 
			       mod->world->matrix, 
			       PointToBearingRange );
      
      bearing += sample_incr;
      
      stg_model_t* hitmod;
      double range = cfg->range_max;
      //stg_laser_return_t hisreturn = LaserVisible;
      
      hitmod = itl_first_matching( itl, laser_raytrace_match, mod );

      if( hitmod )
	range = itl->range;

      //printf( "%d:%.2f  ", t, range );

      if( range < cfg->range_min )
	range = cfg->range_min;
            
      // record the range in mm
      //scan[t+1].range = 
	scan[t].range = (uint32_t)( range * 1000.0 );
      // if the object is bright, it has a non-zero reflectance
      //scan[t+1].reflectance = 
	
	if( hitmod )
	  {
	    stg_laser_return_t* lr = 
	      stg_model_get_property_fixed( hitmod, 
					    "laser_return", 
					    sizeof(stg_laser_return_t));
	    
	    scan[t].reflectance = 
	      (*lr >= LaserBright) ? 1 : 0;
	  }
	else
	  scan[t].reflectance = 0;
	    

      itl_destroy( itl );
    }
  
  // new style
  stg_model_set_property( mod, "laser_data", scan, sizeof(stg_laser_sample_t) * cfg->samples);
  

#if TIMING
  gettimeofday( &tv2, NULL );
  printf( " laser data update time %.6f\n",
	  (tv2.tv_sec + tv2.tv_usec / 1e6) - 
	  (tv1.tv_sec + tv1.tv_usec / 1e6) );	    
#endif

  return 0; //ok
}


int laser_unrender_data( stg_model_t* mod, char* name, 
			 void* data, size_t len, void* userp )
{
  stg_model_fig_clear( mod, "laser_data_fig" );
  stg_model_fig_clear( mod, "laser_data_bg_fig" );  
  return 1; // callback just runs one time
}

int laser_render_data( stg_model_t* mod, char* name, 
		       void* data, size_t len, void* userp )
{
  
  stg_rtk_fig_t* bg = stg_model_get_fig( mod, "laser_data_bg_fig" );
  stg_rtk_fig_t* fg = stg_model_get_fig( mod, "laser_data_fig" );  
  
  if( ! bg )
    bg = stg_model_fig_create( mod, "laser_data_bg_fig", "top", STG_LAYER_BACKGROUND );
  
  if( ! fg )
    fg = stg_model_fig_create( mod, "laser_data_fig", "top", STG_LAYER_LASERDATA );  
  
  stg_rtk_fig_clear( bg );
  stg_rtk_fig_clear( fg );

  
  stg_laser_config_t *cfg = 
    stg_model_get_property_fixed( mod, "laser_cfg", sizeof(stg_laser_config_t));
  assert( cfg );
  
  
  stg_laser_sample_t* samples = (stg_laser_sample_t*)data; 
  size_t sample_count = len / sizeof(stg_laser_sample_t);
  
  if( samples && sample_count )
    {
      // now get on with rendering the laser data
      stg_pose_t pose;
      stg_model_get_global_pose( mod, &pose );
      
      stg_geom_t geom;
      stg_model_get_geom( mod, &geom );
      
      double sample_incr = cfg->fov / sample_count;
      double bearing = geom.pose.a - cfg->fov/2.0;
      stg_point_t* points = calloc( sizeof(stg_point_t), sample_count + 1 );
      
      //stg_rtk_fig_origin( fg, pose.x, pose.y, pose.a );  
      stg_rtk_fig_color_rgb32( fg, bright_color );
      int s;
      for( s=0; s<sample_count; s++ )
	{
	  // useful debug
	  //stg_rtk_fig_arrow( fig, 0, 0, bearing, (sample->range/1000.0), 0.01 );
	  
	  points[1+s].x = (samples[s].range/1000.0) * cos(bearing);
	  points[1+s].y = (samples[s].range/1000.0) * sin(bearing);
	  bearing += sample_incr;
	}
      
      // hmm, what's the right cast to get rid of the compiler warning
      // for the points argument? the function expects a double[][2] type. 
      
      if( mod->world->win->fill_polygons )
	{
	  stg_rtk_fig_color_rgb32( bg, fill_color );
	  stg_rtk_fig_polygon( bg, 0,0,0, sample_count+1, (double*)points, TRUE );
	}
      
      stg_rtk_fig_color_rgb32( fg, laser_color );
      stg_rtk_fig_polygon( fg, 0,0,0, sample_count+1, (double*)points, FALSE ); 	
      
      // loop through again, drawing bright boxes on top of the polygon
      for( s=0; s<sample_count; s++ )
	{      
	  // if this hit point is bright, we draw a little box
	  if( samples[s].reflectance > 0 )
	    {
	      stg_rtk_fig_color_rgb32( fg, bright_color );
	      stg_rtk_fig_rectangle( fg, 
				     points[1+s].x, points[1+s].y, 0,
				     0.04, 0.04, 1 );
	      stg_rtk_fig_color_rgb32( fg, laser_color );
	    }
	}
      
      free( points );
    }
  return 0; // callback runs until removed
}

int laser_render_cfg( stg_model_t* mod, char* name, void* data, size_t len, void* userp )
{
  //puts( "laser render cfg" );
  
  stg_rtk_fig_t* fig = stg_model_get_fig( mod, "laser_cfg_fig" );
  
  if( !fig )
    fig = stg_model_fig_create( mod, "laser_config_fig", "top", STG_LAYER_LASERCONFIG );
  
  stg_rtk_fig_clear(fig);
  
  // draw the FOV and range lines
  stg_rtk_fig_color_rgb32( fig, cfg_color );
  
  stg_laser_config_t *cfg = (stg_laser_config_t*)data;
  assert( cfg );
  
  double mina = cfg->fov / 2.0;
  double maxa = -cfg->fov / 2.0;
  
  double leftfarx = cfg->range_max * cos(mina);
  double leftfary = cfg->range_max * sin(mina);
  double rightfarx = cfg->range_max * cos(maxa);
  double rightfary = cfg->range_max * sin(maxa);  
  double leftnearx = cfg->range_min * cos(mina);  
  double leftneary = cfg->range_min * sin(mina);
  double rightnearx = cfg->range_min * cos(maxa);
  double rightneary = cfg->range_min * sin(maxa);
  
  stg_rtk_fig_line( fig, leftnearx, leftneary, leftfarx, leftfary );
  stg_rtk_fig_line( fig, rightnearx, rightneary, rightfarx, rightfary );
  
  stg_rtk_fig_ellipse_arc( fig,0,0,0, 
			   2.0*cfg->range_max,
			   2.0*cfg->range_max, 
			   mina, maxa );      
  
  stg_rtk_fig_ellipse_arc( fig,0,0,0, 
			   2.0*cfg->range_min,
			   2.0*cfg->range_min, 
			   mina, maxa );      
  return 0;
}


/* int stg_model_unrender_cfg( stg_model_t* mod, char* name,  */
/* 			    void* data, size_t len, void* userp ) */
/* { */
/*   gui_model_t* gui =  */
/*     stg_model_get_property_fixed( mod, "gui", sizeof(gui_model_t)); */
  
/*   if( gui && gui->cfg  ) */
/*     stg_rtk_fig_clear(gui->cfg); */
  
/*   return 1; // callback just runs one time */
/* } */

int laser_startup( stg_model_t* mod )
{ 
  PRINT_DEBUG( "laser startup" );
  
  // start consuming power
  //mod->watts = STG_LASER_WATTS;
  
  // install the update function
  mod->f_update = laser_update;

  return 0; // ok
}

int laser_shutdown( stg_model_t* mod )
{ 
  PRINT_DEBUG( "laser shutdown" );
  
  // uninstall the update function
  mod->f_update = NULL;

  // stop consuming power
  //mod->watts = 0.0;
  
  // clear the data - this will unrender it too
  stg_model_set_property( mod, "laser_data", NULL, 0 );

  return 0; // ok
}


