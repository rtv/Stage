///////////////////////////////////////////////////////////////////////////
//
// File: model_laser.c
// Author: Richard Vaughan
// Date: 10 June 2004
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/model_laser.c,v $
//  $Author: rtv $
//  $Revision: 1.23 $
//
///////////////////////////////////////////////////////////////////////////

#include <sys/time.h>
#include <math.h>

#include "stage.h"
#include "raytrace.h"


#include "gui.h"
extern rtk_fig_t* fig_debug;

#define TIMING 0
#define LASER_FILLED 1

void model_laser_config_init( model_t* mod );
int model_laser_config_set( model_t* mod, void* config, size_t len );
int model_laser_data_shutdown( model_t* mod );
int model_laser_data_service( model_t* mod );
int model_laser_data_set( model_t* mod, void* data, size_t len );
void model_laser_return_init( model_t* mod );
void model_laser_config_render( model_t* mod );
void model_laser_data_render( model_t* mod );


void model_laser_register(void)
{ 
  PRINT_DEBUG( "LASER INIT" );  

  model_register_init( STG_PROP_LASERCONFIG, model_laser_config_init );
  model_register_set( STG_PROP_LASERCONFIG, model_laser_config_set );

  model_register_set( STG_PROP_LASERDATA, model_laser_data_set );
  model_register_shutdown( STG_PROP_LASERDATA, model_laser_data_shutdown );
  model_register_service( STG_PROP_LASERDATA, model_laser_data_service );

  model_register_init( STG_PROP_LASERRETURN, model_laser_return_init );

}

void model_laser_return_init( model_t* mod )
{
  stg_laser_return_t val = STG_DEFAULT_LASERRETURN;
  model_set_prop_generic( mod, STG_PROP_LASERRETURN, &val, sizeof(val) );
}

stg_laser_return_t model_laser_return( model_t* mod )
{
  stg_laser_return_t *val = 
    model_get_prop_data_generic( mod, STG_PROP_LASERRETURN );
  assert(val);
  return *val;
}

void model_laser_config_init( model_t* mod )
{
  stg_laser_config_t lconf;
  memset(&lconf,0,sizeof(lconf));
  
  lconf.geom.pose.x = STG_DEFAULT_LASER_POSEX;
  lconf.geom.pose.y = STG_DEFAULT_LASER_POSEY;
  lconf.geom.pose.a = STG_DEFAULT_LASER_POSEA;
  lconf.geom.size.x = STG_DEFAULT_LASER_SIZEX;
  lconf.geom.size.y = STG_DEFAULT_LASER_SIZEY;
  lconf.range_min   = STG_DEFAULT_LASER_MINRANGE;
  lconf.range_max   = STG_DEFAULT_LASER_MAXRANGE;
  lconf.fov         = STG_DEFAULT_LASER_FOV;
  lconf.samples     = STG_DEFAULT_LASER_SAMPLES;
  
  model_set_prop_generic( mod, STG_PROP_LASERCONFIG, &lconf,sizeof(lconf) );
}


stg_laser_config_t* model_laser_config_get( model_t* mod )
{
  stg_laser_config_t* laser = model_get_prop_data_generic( mod, STG_PROP_LASERCONFIG );
  assert(laser);
  return laser;
}


int model_laser_data_shutdown( model_t* mod )
{
  // clear the figure
  model_laser_data_render(mod); 

  // remove the stdata
  //model_remove_prop_generic( mod, STG_PROP_LASERDATA );

  return 0; // ok
}

int model_laser_data_service( model_t* mod )
{   
  PRINT_DEBUG( "laser update" );

  PRINT_DEBUG1( "[%lu] updating lasers", mod->world->sim_time );
  
  stg_laser_config_t* cfg = model_laser_config_get(mod);
  stg_geom_t* geom = &cfg->geom;

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
	  hisreturn = model_laser_return(hitmod);
	  
	  if( hisreturn != LaserTransparent) 
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
      scan[t].reflectance = hisreturn >= LaserBright ? 1 : 0;

      itl_destroy( itl );

      //printf( "%d ", sample->range );
    }
  

  // store the data
  model_set_prop( mod, STG_PROP_LASERDATA, 
		  scan, sizeof(stg_laser_sample_t) * cfg->samples );
  
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


int model_laser_data_set( model_t* mod, void* data, size_t len )
{  
  // store the data
  model_set_prop_generic( mod, STG_PROP_LASERDATA, data, len );
  
  // and redraw it
  //model_laser_data_render( mod, (stg_laser_data_t*)data, len / sizeof(stg_laser_sample_t) );
  model_laser_data_render( mod );

  return 0; //OK
}
 
int model_laser_config_set( model_t* mod, void* config, size_t len )
{  
  // store the config
  model_set_prop_generic( mod, STG_PROP_LASERCONFIG, config, len );
  
  // and redraw it
  model_laser_config_render( mod);

  return 0; //OK
}
 

void model_laser_data_render( model_t* mod )
{
  PRINT_DEBUG( "laser render" );

  rtk_fig_t* fig = mod->gui.propdata[STG_PROP_LASERDATA];  
  
  if( fig  )
    rtk_fig_clear(fig);
  else // create the figure, store it in the model and keep a local pointer
    fig = model_prop_fig_create( mod, mod->gui.propdata, STG_PROP_LASERDATA,
				 mod->gui.top, STG_LAYER_LASERDATA );
  
  // if this model has a laser subscription
  // and some data, we'll draw the data
  if(  mod->subs[STG_PROP_LASERDATA] )
    {
      stg_geom_t* geom = model_geom_get(mod);
      rtk_fig_origin( fig, geom->pose.x, geom->pose.y, geom->pose.a );  
      stg_laser_config_t* cfg = model_laser_config_get(mod);
      
      if( cfg == NULL )
	{
	  PRINT_WARN( "no laser config available" );
	  return;
	}

      stg_laser_sample_t* samples = (stg_laser_sample_t*)
	model_get_prop_data_generic( mod, STG_PROP_LASERDATA );
      
      if( samples == NULL )
	{
	  PRINT_WARN( "no laser data available" );
	  return;
	}

      double sample_incr = cfg->fov / cfg->samples;
      double bearing = cfg->geom.pose.a - cfg->fov/2.0;
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
}




void model_laser_config_render( model_t* mod )
{ 
  PRINT_DEBUG( "laser config render" );
  
  rtk_fig_t* fig = mod->gui.propdata[STG_PROP_LASERCONFIG];  
  
  if( fig  )
    rtk_fig_clear(fig);
  else // create the figure, store it in the model and keep a local pointer
    fig = model_prop_fig_create( mod, mod->gui.propdata, STG_PROP_LASERCONFIG,
				 mod->gui.top, STG_LAYER_LASERCONFIG );
  
  rtk_fig_t* geomfig = mod->gui.propgeom[STG_PROP_LASERCONFIG];  
  
  if( geomfig  )
    rtk_fig_clear(geomfig);
  else // create the figure, store it in the model and keep a local pointer
    geomfig = model_prop_fig_create( mod, mod->gui.propgeom, STG_PROP_LASERCONFIG,
				 mod->gui.top, STG_LAYER_LASERGEOM );

  stg_laser_config_t* cfg = model_laser_config_get(mod);

  
  stg_geom_t* geom = &cfg->geom;
  
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


