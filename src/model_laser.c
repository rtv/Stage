#include "stage.h"
#include "raytrace.h"
#include <sys/time.h>

#include "rtk.h"
extern rtk_fig_t* fig_debug;

#define TIMING 0

void model_laser_init( model_t* mod )
{
  // configure the laser to sensible defaults
  mod->laser_geom.pose.x = 0;
  mod->laser_geom.pose.y = 0;
  mod->laser_geom.pose.a = 0;
  mod->laser_geom.size.x = 0.0; // invisibly small (so it's not rendered) by default
  mod->laser_geom.size.y = 0.0;
  
  mod->laser_config.range_min= 0.0;
  mod->laser_config.range_max = 8.0;
  mod->laser_config.samples = 180;
  mod->laser_config.fov = M_PI;
  
  mod->laser_return = LaserVisible;
}


void model_update_laser( model_t* mod )
{   
  PRINT_DEBUG1( "[%lu] updating lasers", mod->world->sim_time );
  
  stg_laser_config_t* cfg = &mod->laser_config;
  stg_geom_t* geom = &mod->laser_geom;
  
  // we only allocate data space the first time we need it
  if( mod->laser_data == NULL )
    mod->laser_data = g_array_new( FALSE, TRUE, sizeof(stg_laser_sample_t) );
  
  // set the laser data array to the right length
  g_array_set_size( mod->laser_data, cfg->samples );
  
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
            
      stg_laser_sample_t* sample = 
	&g_array_index( mod->laser_data, stg_laser_sample_t, t );
 
      // record the range in mm
      sample->range = (uint32_t)( range * 1000.0 );
      sample->reflectance = 1;


      //printf( "%d ", sample->range );
    }
  
#if TIMING
  gettimeofday( &tv2, NULL );
  printf( " laser data update time %.6f\n",
	  (tv2.tv_sec + tv2.tv_usec / 1e6) - 
	  (tv1.tv_sec + tv1.tv_usec / 1e6) );	    
#endif
  



  //puts("");
  //puts("");
}
