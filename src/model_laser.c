///////////////////////////////////////////////////////////////////////////
//
// File: model_laser.c
// Author: Richard Vaughan
// Date: 10 June 2004
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/model_laser.c,v $
//  $Author: rtv $
//  $Revision: 1.26 $
//
///////////////////////////////////////////////////////////////////////////

#include <sys/time.h>
#include <math.h>

#define DEBUG

#include "stage.h"
#include "raytrace.h"


#include "gui.h"
extern rtk_fig_t* fig_debug;

#define TIMING 0
#define LASER_FILLED 1

  
void laser_init( model_t* mod )
{
  // sensible laser defaults
  mod->geom.pose.x = STG_DEFAULT_LASER_POSEX;
  mod->geom.pose.y = STG_DEFAULT_LASER_POSEY;
  mod->geom.pose.a = STG_DEFAULT_LASER_POSEA;
  mod->geom.size.x = STG_DEFAULT_LASER_SIZEX;
  mod->geom.size.y = STG_DEFAULT_LASER_SIZEY;

  // set up a laser-specific config structure
  stg_laser_config_t lconf;
  memset(&lconf,0,sizeof(lconf));
  
  lconf.range_min   = STG_DEFAULT_LASER_MINRANGE;
  lconf.range_max   = STG_DEFAULT_LASER_MAXRANGE;
  lconf.fov         = STG_DEFAULT_LASER_FOV;
  lconf.samples     = STG_DEFAULT_LASER_SAMPLES;
  
  model_putconfig( mod, &lconf, sizeof(lconf) );
}


int laser_update( model_t* mod )
{   
  PRINT_DEBUG1( "[%lu] laser update", mod->world->sim_time );
  
  stg_laser_config_t* cfg = mod->cfg;
  assert(cfg);
  stg_geom_t* geom = &mod->geom;

  // get the sensor's pose in global coords
  stg_pose_t pz;
  memcpy( &pz, &geom->pose, sizeof(pz) ); 
  model_local_to_global( mod, &pz );

  double sample_incr = cfg->fov / (double)cfg->samples;
  
  double bearing = pz.a - cfg->fov/2.0;
  
#if TIMING
  struct timeval tv1, tv2;
  gettimeofday( &tv1, NULL );
#endif
      
  if( fig_debug ) rtk_fig_clear( fig_debug );

  // make a scan buffer
  stg_laser_sample_t* scan = 
    calloc( sizeof(stg_laser_sample_t), cfg->samples );
  
  int t;
  for( t=0; t<cfg->samples; t++ )
    {
      
      itl_t* itl = itl_create( pz.x, pz.y, bearing, 
			       cfg->range_max, 
			       mod->world->matrix, 
			       PointToBearingRange );
      
      bearing += sample_incr;
      
      model_t* hitmod;
      double range = cfg->range_max;
      stg_laser_return_t hisreturn = LaserVisible;
      
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
	  //hisreturn = hitmdmodel_laser_return(hitmod);
	  
	  if( hitmod->laser_return != LaserTransparent) 
	    {
	      range = itl->range;
	      break;
	    }	
	}

      if( range < cfg->range_min )
	range = cfg->range_min;
            
      // record the range in mm
      scan[t].range = (uint32_t)( range * 1000.0 );
      // if the object is bright, it has a non-zero reflectance
      scan[t].reflectance = 
	(hitmod && (hitmod->laser_return >= LaserBright)) ? 1 : 0;

      itl_destroy( itl );

      //printf( "%d ", sample->range );
    }
  
  
  // new style
  model_putdata( mod, scan, sizeof(stg_laser_sample_t) * cfg->samples );
  
  free( scan );

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


void laser_render_data(  model_t* mod, void* data, size_t len )
{
  rtk_fig_t* fig = mod->gui.propdata[STG_PROP_DATA];  
  
  if( fig  )
    rtk_fig_clear(fig);
  else // create the figure, store it in the model and keep a local pointer
    fig = model_prop_fig_create( mod, mod->gui.propdata, STG_PROP_DATA,
				 mod->gui.top, STG_LAYER_LASERDATA );
  
  stg_geom_t* geom = &mod->geom;
  rtk_fig_origin( fig, geom->pose.x, geom->pose.y, geom->pose.a );  

  assert(mod->cfg);
  assert(mod->cfg_len == sizeof(stg_laser_config_t));
  
  stg_laser_config_t* cfg = mod->cfg;

  stg_laser_sample_t* samples = (stg_laser_sample_t*)data;
  
  if( samples == NULL )
    {
      PRINT_WARN( "no laser data available" );
      return;
    }
  
  double sample_incr = cfg->fov / cfg->samples;
  double bearing = geom->pose.a - cfg->fov/2.0;
  stg_point_t* points = calloc( sizeof(stg_point_t), cfg->samples + 1 );
  
  rtk_fig_color_rgb32(fig, stg_lookup_color(STG_LASER_BRIGHT_COLOR) );
  
  int s;
  for( s=0; s<cfg->samples; s++ )
    {
      // useful debug
      //rtk_fig_arrow( fig, 0, 0, bearing, (sample->range/1000.0), 0.01 );
      
      points[1+s].x = (samples[s].range/1000.0) * cos(bearing);
      points[1+s].y = (samples[s].range/1000.0) * sin(bearing);
      bearing += sample_incr;
    }
  
  rtk_fig_color_rgb32(fig, stg_lookup_color(STG_LASER_COLOR) );
  // hmm, what's the right cast to get rid of the compiler warning
  // for the points argument?
  rtk_fig_polygon( fig, 0,0,0, cfg->samples+1, points, LASER_FILLED );      
  
  
  rtk_fig_color_rgb32(fig, stg_lookup_color(STG_LASER_BRIGHT_COLOR) );
  
  // loop through again, drawing bright boxes on top of the polygon
  for( s=0; s<cfg->samples; s++ )
    {
      // if this hit point is bright, we draw a little box
      if( samples[s].reflectance > 0 )
	rtk_fig_rectangle( fig, 
			   points[1+s].x, points[1+s].y, 0,
			   0.04, 0.04, 1 );
    }
  
  
  
  free( points );
}

void laser_render_config( model_t* mod, stg_laser_config_t* cfg )
{ 
  PRINT_DEBUG( "laser config render" );
  
  rtk_fig_t* fig = mod->gui.propdata[STG_PROP_CONFIG];  
  
  if( fig  )
    rtk_fig_clear(fig);
  else // create the figure, store it in the model and keep a local pointer
    fig = model_prop_fig_create( mod, mod->gui.propdata, STG_PROP_CONFIG,
				 mod->gui.top, STG_LAYER_LASERGEOM );
  
  rtk_fig_t* geomfig = mod->gui.propgeom[STG_PROP_CONFIG];  
  
  if( geomfig  )
    rtk_fig_clear(geomfig);
  else // create the figure, store it in the model and keep a local pointer
    geomfig = model_prop_fig_create( mod, mod->gui.propgeom, STG_PROP_CONFIG,
				 mod->gui.top, STG_LAYER_LASERGEOM );

  stg_geom_t* geom = &mod->geom;
  
  stg_pose_t pose;
  memcpy( &pose, &geom->pose, sizeof(pose) );
  model_local_to_global( mod, &pose );
  
  // first draw the sensor body
  rtk_fig_color_rgb32(geomfig, stg_lookup_color(STG_LASER_GEOM_COLOR) );
  
  rtk_fig_rectangle( geomfig, 
		     geom->pose.x, 
		     geom->pose.y, 
		     geom->pose.a,
		     geom->size.x,
		     geom->size.y, 0 );
  
  
  // now draw the FOV and range lines
  rtk_fig_color_rgb32( fig, stg_lookup_color( STG_LASER_CFG_COLOR ));
  
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
  
  rtk_fig_line( fig, leftnearx, leftneary, leftfarx, leftfary );
  rtk_fig_line( fig, rightnearx, rightneary, rightfarx, rightfary );
  
  rtk_fig_ellipse_arc( fig,0,0,0, 
		       2.0*cfg->range_max,
		       2.0*cfg->range_max, 
		       mina, maxa );      
  
  rtk_fig_ellipse_arc( fig,0,0,0, 
		       2.0*cfg->range_min,
		       2.0*cfg->range_min, 
		       mina, maxa );      
}

int laser_putdata( model_t* mod, void* data, size_t len )
{
  PRINT_DEBUG( "laser putdata" );
  
  // put the data in the normal way
  stg_copybuf( &mod->data, &mod->data_len, data, len );
  
    // and render it
  laser_render_data( mod, data, len );
}

int laser_putconfig( model_t* mod, void* cfg, size_t len )
{
  PRINT_DEBUG( "laser putconfig" );
  
  // put the data in the normal way
  stg_copybuf( &mod->cfg, &mod->cfg_len, cfg, len );
  
  // and render it
  laser_render_config( mod, (stg_laser_config_t*)cfg );
}

int laser_shutdown( model_t* mod )
{
  // clear the figure
  rtk_fig_t* fig = mod->gui.propdata[STG_PROP_DATA];  
  
  if( fig  ) rtk_fig_clear(fig);
  
  return 0; // ok
}


int register_laser( lib_entry_t lib[] )
{
  register_init( STG_MODEL_LASER, laser_init );

  register_putdata( STG_MODEL_LASER, laser_putdata );
  //register_startup( STG_MODEL_LASER, laser_startup );
  register_shutdown( STG_MODEL_LASER, laser_shutdown );
  //register_getdata( STG_MODEL_LASER, laser_getdata );
  register_putconfig( STG_MODEL_LASER, laser_putconfig );
  register_update( STG_MODEL_LASER, laser_update );
} 
