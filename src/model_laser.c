#include "stage.h"
#include "raytrace.h"

void model_update_laser( stg_model_t* mod )
{   
  //PRINT_DEBUG1( "[%.3f] updating lasers", mod->world->sim_time );
  
  stg_laser_config_t* cfg = &mod->laser_config;
  
  // we only allocate data space the first time we need it
  if( mod->laser_data == NULL )
    mod->laser_data = g_array_new( FALSE, TRUE, sizeof(stg_laser_sample_t) );
  
  // set the laser data array to the right length
  g_array_set_size( mod->laser_data, cfg->samples );
  
  // get the sensor's pose in global coords
  stg_pose_t pz;
  memcpy( &pz, &cfg->pose, sizeof(pz) ); 
  model_local_to_global( mod, &pz );

  double sample_incr = cfg->fov / (double)cfg->samples;
  
  double bearing = pz.a - cfg->fov/2.0;
  
  int t;
  for( t=0; t<cfg->samples; t++ )
    {
      itl_t* itl = itl_create( pz.x, pz.y, bearing, 
			       cfg->range_max, 
			       mod->world->matrix, 
			       PointToBearingRange );
      
      bearing += sample_incr;

      stg_model_t* hitmod;
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
    }
}
