#include "stage.h"
#include "raytrace.h"

#include "gui.h"
extern rtk_fig_t* fig_debug;

void model_ranger_init( model_t* mod )
{
  mod->ranger_return = LaserVisible;
  mod->ranger_data = g_array_new( FALSE, TRUE, sizeof(stg_ranger_sample_t));
  mod->ranger_config = g_array_new( FALSE, TRUE, sizeof(stg_ranger_config_t) );
}


void model_update_rangers( model_t* mod )
{   
  //PRINT_DEBUG1( "[%.3f] updating rangers", mod->world->sim_time );
  
  assert( mod->ranger_config );
  assert( mod->ranger_data );
  
  int rcount = mod->ranger_config->len;
  
  if( rcount < 1 )
    return;

  //PRINT_WARN1( "wierd! %d rangers", rcount);
  
  // set the data array to the right number of samples
  g_array_set_size( mod->ranger_data, rcount );
  
  if( fig_debug ) rtk_fig_clear( fig_debug );

  int t;
  for( t=0; t<rcount; t++ )
    {
      stg_ranger_config_t* cfg = &g_array_index( mod->ranger_config, 
					   stg_ranger_config_t, t );
      
      g_assert( cfg );
      
      // get the sensor's pose in global coords
      stg_pose_t pz;
      memcpy( &pz, &cfg->pose, sizeof(pz) ); 
      model_local_to_global( mod, &pz );
      
      // todo - use bounds_range.min

      itl_t* itl = itl_create( pz.x, pz.y, pz.a, 
			       cfg->bounds_range.max, 
			       mod->world->matrix, 
			       PointToBearingRange );
      
      model_t * hitmod;
      double range = cfg->bounds_range.max;
      
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
	  if( hitmod->ranger_return != LaserTransparent) 
	    {
	      range = itl->range;
	      break;
	    }	
	}
      
      if( range < cfg->bounds_range.min )
	range = cfg->bounds_range.min;
      
      stg_ranger_sample_t* sample 
	= &g_array_index( mod->ranger_data, stg_ranger_sample_t, t );
      
      sample->range = range;
      //sample->error = TODO;
    }
}

