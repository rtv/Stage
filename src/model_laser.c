#include <sys/time.h>
#include <math.h>

#include "stage.h"
#include "raytrace.h"


#include "gui.h"
extern rtk_fig_t* fig_debug;

#define TIMING 0
#define LASER_FILLED 1

/* void model_laser_init( model_t* mod ) */
/* {      */
/*   PRINT_DEBUG( "laser init" ); */

/*   // configure the laser to sensible defaults   */
/*   stg_laser_config_t lcfg; */
/*   memset(&lcfg,0,sizeof(lcfg)); */
/*   lcfg.range_min= 0.0; */
/*   lcfg.range_max = 8.0; */
/*   lcfg.samples = 180; */
/*   lcfg.fov = M_PI; */
  
/*   model_set_prop_generic( mod, STG_PROP_LASERCONFIG,  */
/* 			  &lcfg, sizeof(lcfg) ); */
  
/*   // start off with a full set of zeroed data */
/*   stg_laser_sample_t* scan = calloc( sizeof(stg_laser_sample_t), lcfg.samples ); */
/*   model_set_prop_generic( mod, STG_PROP_LASERDATA,  */
/* 			  scan, sizeof(stg_laser_sample_t) * lcfg.samples ); */
/*   free(scan); */
/* } */


void model_laser_shutdown( model_t* mod )
{
  // remove the data
  model_remove_prop_generic( mod, STG_PROP_LASERDATA );
}

void model_laser_update( model_t* mod )
{   
  PRINT_DEBUG( "laser update" );

  PRINT_DEBUG1( "[%lu] updating lasers", mod->world->sim_time );
  
  stg_laser_config_t* cfg = (stg_laser_config_t*)
    model_get_prop_data_generic( mod, STG_PROP_LASERCONFIG ); 
  assert(cfg);

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
      scan[t].reflectance = 1;


      //printf( "%d ", sample->range );
    }
  

  // store the data
  model_set_prop_generic( mod, STG_PROP_LASERDATA, 
			  scan, sizeof(stg_laser_sample_t) * cfg->samples );

  free( scan );

#if TIMING
  gettimeofday( &tv2, NULL );
  printf( " laser data update time %.6f\n",
	  (tv2.tv_sec + tv2.tv_usec / 1e6) - 
	  (tv1.tv_sec + tv1.tv_usec / 1e6) );	    
#endif
}


/* STG_PROP_LASERCONFIG  gmod->top = rtk_fig_create( win->canvas, parent_fig, STG_LAYER_BODY ); */
/*   gmod->geom =  rtk_fig_create( win->canvas, gmod->top, STG_LAYER_GEOM ); */
/*   gmod->ranger_cfg = rtk_fig_create( win->canvas, gmod->top, STG_LAYER_SENSORCONFIG ); */
/*   gmod->ranger_data = rtk_fig_create( win->canvas, gmod->top, STG_LAYER_DATA); */
/*   gmod->laser_geom = rtk_fig_create( win->canvas, gmod->top, STG_LAYER_SENSORGEOM ); */
/*   gmod->laser_data = rtk_fig_create( win->canvas, gmod->top, STG_LAYER_DATA ); */
/*   gmod->laser_cfg = rtk_fig_create( win->canvas, gmod->top, STG_LAYER_SENSORCONFIG ); */
/* #ifdef LASER_FILLED */
/*   gmod->laser_data = rtk_fig_create( win->canvas, gmod->top, STG_LAYER_BACKGROUND); */
/* #else */
/*   gmod->laser_data = rtk_fig_create( win->canvas, gmod->top, STG_LAYER_DATA); */
/* #endif */

/*   gmod->blob_data = rtk_fig_create( win->canvas, parent_fig, STG_LAYER_DATA); */
/*   gmod->blob_cfg = rtk_fig_create( win->canvas, gmod->top, STG_LAYER_SENSORCONFIG); */
/*   gmod->fiducial_data = rtk_fig_create( win->canvas, gmod->top, STG_LAYER_DATA); */
/*   gmod->fiducial_cfg = rtk_fig_create( win->canvas, gmod->top, STG_LAYER_SENSORCONFIG); */



rtk_fig_t* model_prop_fig_create( model_t* mod, 
				  rtk_fig_t* array[],
				  stg_id_t propid, 
				  rtk_fig_t* parent,
				  int layer )
{
  rtk_fig_t* fig =  rtk_fig_create( mod->world->win->canvas, 
				    parent, 
				    layer ); 
  array[propid] = fig;
  
  return fig;
}


void model_laser_render( model_t* mod )
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
      rtk_fig_color_rgb32(fig, stg_lookup_color(STG_LASER_COLOR) );
      rtk_fig_origin( fig, mod->local_pose.x, mod->local_pose.y, mod->local_pose.a );  
      stg_laser_config_t* cfg = (stg_laser_config_t*)
	model_get_prop_data_generic( mod, STG_PROP_LASERCONFIG );
      
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
      stg_laser_sample_t* sample = NULL;
      
      stg_point_t* points = calloc( sizeof(stg_point_t), cfg->samples + 1 );

      int s;
      for( s=0; s<cfg->samples; s++ )
	{
	  // useful debug
	  //rtk_fig_arrow( fig, 0, 0, bearing, (sample->range/1000.0), 0.01 );
	  
	  points[1+s].x = (samples[s].range/1000.0) * cos(bearing);
	  points[1+s].y = (samples[s].range/1000.0) * sin(bearing);
	  bearing += sample_incr;
	}

      // hmm, what's the right cast to get rid of the compiler warning
      // for the points argument?
      rtk_fig_polygon( fig, 0,0,0, cfg->samples+1, points, LASER_FILLED );      

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

  stg_laser_config_t* cfg = (stg_laser_config_t*) 
    model_get_prop_data_generic( mod, STG_PROP_LASERCONFIG );
  
  if( cfg )
    {  
      stg_geom_t* geom = &cfg->geom;
      
      stg_pose_t pose;
      memcpy( &pose, &geom->pose, sizeof(pose) );
      model_local_to_global( mod, &pose );
      
      printf( "%d x %.2f y %.2f a %.2f w %.2f h %.2f fov %.2f min %.2f max %.2f\n",
	      mod->id, geom->pose.x, geom->pose.y, geom->pose.a, geom->size.x, geom->size.y, cfg->fov, cfg->range_min, cfg->range_max );
      
      
      // first draw the sensor body
      rtk_fig_color_rgb32(geomfig, stg_lookup_color(STG_LASER_TRANSDUCER_COLOR) );
      
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
}


