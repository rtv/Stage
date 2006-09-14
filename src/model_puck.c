///////////////////////////////////////////////////////////////////////////
//
// File: model_puck.c
// Author: Richard Vaughan
// Date: 15 Dec 2005
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/model_puck.c,v $
//  $Author: rtv $
//  $Revision: 1.1.4.1 $
//
///////////////////////////////////////////////////////////////////////////


#include <sys/time.h>
#include <math.h>
#include "gui.h"

//#define DEBUG

#include "stage_internal.h"

// declare functions used as callbacks
static void puck_load( stg_model_t* mod );
static int puck_update( stg_model_t* mod );

int puck_init( stg_model_t* mod )
{
  // override the default methods
  mod->f_update = puck_update;
  mod->f_load = puck_load;
  
  // sensible puck defaults 
  stg_geom_t geom; 
  geom.pose.x = 0.0;
  geom.pose.y = 0.0;
  geom.pose.a = 0.0;
  geom.size.x = 0.1;
  geom.size.y = 0.1;
  stg_model_set_geom( mod, &geom );
  
  // create a single rectangle body 
  stg_polygon_t* square = stg_unit_polygon_create();
  stg_model_set_polygons( mod, square, 1 );
  
  // set default color
  stg_model_set_color( mod, 0x00FF00 ); // green
  
  // we don't need special gui elements for a puck, so there's no gui_puck_init()

   return 0;
}

void puck_load( stg_model_t* mod )
{
  // nothing to do
}

int puck_raytrace_match( stg_model_t* mod, stg_model_t* hitmod )
{           
  
  // Ignore myself, my children, and my ancestors.
  if( (!stg_model_is_related(mod,hitmod))  &&  
      mod->puck_return != PuckTransparent) 
    return 1;
  
  // Stop looking when we see something
  //hisreturn = hitmdmodel_puck_return(hitmod);
  
  return 0; // no match
}	

// give me some velocity if something hits me
int puck_update( stg_model_t* mod )
{   
  //puts( "puck update" );

  PRINT_DEBUG2( "[%lu] puck update (%d subs)", mod->world->sim_time, mod->subs );
  
  // no work to do if we're unsubscribed
  if( mod->subs < 1 )
    return 0;
  
  stg_puck_config_t* cfg = (stg_puck_config_t*)mod->cfg;
  assert(cfg);

  stg_geom_t geom;
  stg_model_get_geom( mod, &geom );

  // get the sensor's pose in global coords
  stg_pose_t pz;
  memcpy( &pz, &geom.pose, sizeof(pz) ); 
  stg_model_local_to_global( mod, &pz );

  PRINT_DEBUG3( "puck origin %.2f %.2f %.2f", pz.x, pz.y, pz.a );

  double sample_incr = cfg->fov / (double)(cfg->samples-1);
  
  double bearing = pz.a - cfg->fov/2.0;
  
#if TIMING
  struct timeval tv1, tv2;
  gettimeofday( &tv1, NULL );
#endif
      
  // make a scan buffer (static for speed, so we only have to allocate
  // memory when the number of samples changes).
  static stg_puck_sample_t* scan = 0;
  scan = realloc( scan, sizeof(stg_puck_sample_t) * cfg->samples );
  
  int t;
  // only compute every second sample, for speed
  //for( t=0; t<cfg.samples-1; t+=2 )
  
  for( t=0; t<cfg->samples; t++ )
    {
      
      itl_t* itl = itl_create( pz.x, pz.y, bearing, 
			       cfg->range_max, 
			       mod->world->matrix, 
			       PointToBearingRange );
      
      bearing += sample_incr;
      
      stg_model_t* hitmod;
      double range = cfg->range_max;
      //stg_puck_return_t hisreturn = PuckVisible;
      
      hitmod = itl_first_matching( itl, puck_raytrace_match, mod );

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
	    scan[t].reflectance = 
	      (mod->puck_return >= PuckBright) ? 1 : 0;
	  }
	else
	  scan[t].reflectance = 0;
	    

      itl_destroy( itl );
    }
  
  // new style
  stg_model_set_data( mod, scan, sizeof(stg_puck_sample_t) * cfg->samples);
  

#if TIMING
  gettimeofday( &tv2, NULL );
  printf( " puck data update time %.6f\n",
	  (tv2.tv_sec + tv2.tv_usec / 1e6) - 
	  (tv1.tv_sec + tv1.tv_usec / 1e6) );	    
#endif

  return 0; //ok
}

