#include "stage.h"
#include "raytrace.h"
#include <sys/time.h>

#include "rtk.h"
extern rtk_fig_t* fig_debug;

#define TIMING 0

void model_laser_init( model_t* mod )
{     
  // configure the laser to sensible defaults
  
  stg_geom_t lgeom;
  lgeom.pose.x = 0;
  lgeom.pose.y = 0;
  lgeom.pose.a = 0;
  lgeom.size.x = 0.0; // invisibly small (so it's not rendered) by default
  lgeom.size.y = 0.0;
  
  model_set_prop_generic( mod, STG_PROP_LASERGEOM, 
			  &lgeom, sizeof(lgeom) );
  
  stg_laser_config_t lcfg;
  lcfg.range_min= 0.0;
  lcfg.range_max = 8.0;
  lcfg.samples = 180;
  lcfg.fov = M_PI;
  
  model_set_prop_generic( mod, STG_PROP_LASERCONFIG, 
			  &lcfg, sizeof(lcfg) );
  
  // start off with a full set of zeroed data
  stg_laser_sample_t* scan = calloc( sizeof(stg_laser_sample_t), lcfg.samples );
  model_set_prop_generic( mod, STG_PROP_LASERDATA, 
			  scan, sizeof(stg_laser_sample_t) * lcfg.samples );
  
  mod->laser_return = LaserVisible;
}


void model_laser_update( model_t* mod )
{   
  PRINT_DEBUG1( "[%lu] updating lasers", mod->world->sim_time );
  
  stg_laser_config_t* cfg = (stg_laser_config_t*)
    model_get_prop_data_generic( mod, STG_PROP_LASERCONFIG ); 
  
  stg_geom_t* geom = (stg_geom_t*)
    model_get_prop_data_generic( mod, STG_PROP_LASERGEOM ); 

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
  



  //puts("");
  //puts("");
}
