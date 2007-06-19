///////////////////////////////////////////////////////////////////////////
//
// File: model_laser.c
// Author: Richard Vaughan
// Date: 10 June 2004
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/model_laser.c,v $
//  $Author: rtv $
//  $Revision: 1.89.2.9 $
//
///////////////////////////////////////////////////////////////////////////


#include <sys/time.h>
#include <math.h>
#include "gui.h"


#include "stage_internal.h"

#define TIMING 0
#define LASER_FILLED 1

#define STG_LASER_WATTS 17.5 // laser power consumption

#define STG_LASER_SAMPLES_MAX 1024
#define STG_DEFAULT_LASER_SIZEX 0.15
#define STG_DEFAULT_LASER_SIZEY 0.15
#define STG_DEFAULT_LASER_SIZEZ 0.2
#define STG_DEFAULT_LASER_MINRANGE 0.0
#define STG_DEFAULT_LASER_MAXRANGE 8.0
#define STG_DEFAULT_LASER_FOV M_PI
#define STG_DEFAULT_LASER_SAMPLES 180

#define DEBUG 1


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
  watts 17.5 # approximately correct for SICK LMS200
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
- laser_sample_skip
  - Only calculate the true range of every nth laser sample. The missing samples are filled in with a linear interpolation. Generally it would be better to use fewer samples, but some (poorly implemented!) programs expect a fixed number of samples. Setting this number > 1 allows you to reduce the amount of computation required for your fixed-size laser vector.
*/

void laser_load( stg_model_t* mod, void* unused )
{
  
  stg_laser_config_t* cfg = (stg_laser_config_t*)mod->cfg;
  
  cfg->samples   = wf_read_int( mod->id, "samples", cfg->samples );
  cfg->range_min = wf_read_length( mod->id, "range_min", cfg->range_min);
  cfg->range_max = wf_read_length( mod->id, "range_max", cfg->range_max );
  cfg->fov       = wf_read_angle( mod->id, "fov",  cfg->fov );

  cfg->resolution = wf_read_int( mod->id, "laser_sample_skip",  cfg->resolution );
 
  if( cfg->resolution < 1 )
    {
      PRINT_WARN( "laser resolution set < 1. Forcing to 1" );
      cfg->resolution = 1;
    }
 
  model_change( mod, &mod->cfg );
}

int laser_update( stg_model_t* mod, void* unused );
int laser_startup( stg_model_t* mod, void* unused );
int laser_shutdown( stg_model_t* mod, void* unused );

// implented by the gui in some other file
void gui_laser_init( stg_model_t* mod );

int laser_init( stg_model_t* mod )
{
  // sensible laser defaults 
  stg_geom_t geom; 
  memset( &geom, 0, sizeof(geom));
  geom.size.x = STG_DEFAULT_LASER_SIZEX;
  geom.size.y = STG_DEFAULT_LASER_SIZEY;
  geom.size.z = STG_DEFAULT_LASER_SIZEZ;
  stg_model_set_geom( mod, &geom );



  // set default color
  stg_model_set_color( mod, stg_lookup_color(STG_LASER_GEOM_COLOR));

  // create a single rectangle body 
  //stg_polygon_t* square = stg_unit_polygon_create();
  //stg_model_set_polygons( mod, square, 1 );

  // set up a laser-specific config structure
  stg_laser_config_t lconf;
  memset(&lconf,0,sizeof(lconf));  
  lconf.range_min   = STG_DEFAULT_LASER_MINRANGE;
  lconf.range_max   = STG_DEFAULT_LASER_MAXRANGE;
  lconf.fov         = STG_DEFAULT_LASER_FOV;
  lconf.samples     = STG_DEFAULT_LASER_SAMPLES;  
  lconf.resolution = 1;
  stg_model_set_cfg( mod, &lconf, sizeof(lconf) );

/*   double vol[6]; */
/*   vol[0] = 0; */
/*   vol[1] = lconf.range_max; */
/*   vol[2] = -lconf.range_max; */
/*   vol[3] =  lconf.range_max; */
/*   vol[4] =  -0.1; */
/*   vol[5] =  0.1; */
/*   stg_model_set_sense_volume( mod, vol ); */


      
  stg_model_set_data( mod, NULL, 0 );

  // install the laser model functionality
  stg_model_add_startup_callback( mod, laser_startup, NULL );
  stg_model_add_shutdown_callback( mod, laser_shutdown, NULL );
  stg_model_add_load_callback( mod, laser_load, NULL );

  gui_laser_init( mod );

  return 0;
}

void stg_laser_config_print( stg_laser_config_t* slc )
{
  printf( "slc fov: %.2f  range_min: %.2f range_max: %.2f samples: %d\n",
	  slc->fov,
	  slc->range_min,
	  slc->range_max,
	  slc->samples );
}

int laser_raytrace_match( stg_model_t* mod, stg_model_t* hitmod )
{           
  // Ignore my relatives and thiings that are invisible to lasers
  //return( (!stg_model_is_related(mod,hitmod)) && 
  //  (hitmod->laser_return > 0) );

  return(  1 );//hitmod == mod->parent ); 
}	

int laser_update( stg_model_t* mod, void* unused )
{   
  PRINT_DEBUG2( "[%lu] laser update (%d subs)", 
		mod->world->sim_time, mod->subs );
  printf( "[%lu] laser update (%d subs)\n", 
	  mod->world->sim_time, mod->subs );
  
  // no work to do if we're unsubscribed
  if( mod->subs < 1 )
    return 0;
  
  stg_laser_config_t* cfg = (stg_laser_config_t*)mod->cfg;
  assert(cfg);

  stg_geom_t geom;
  stg_model_get_geom( mod, &geom );

  // get the sensor's pose in global coords
  stg_pose_t gpose;
  stg_model_get_global_pose( mod, &gpose );

  PRINT_DEBUG3( "laser origin %.2f %.2f %.2f", gpose.x, gpose.y, gpose.a );

  double sample_incr = cfg->fov / (double)(cfg->samples-1);
  
  //double bearing = gpose.a - cfg->fov/2.0;
  
#if TIMING
  struct timeval tv1, tv2;
  gettimeofday( &tv1, NULL );
#endif
      
  
  // make a static scan buffer, so we only have to allocate memory
  // when the number of samples changes between calls - probably
  // unusual.
  static stg_laser_sample_t* scan = NULL;  
  scan = g_renew( stg_laser_sample_t, scan, cfg->samples );



  //stg_endpoint_t* ep = mod->world->endpts.x;
  //assert( ep );
  // wind along to the first point to the right of the model
  //while( ep->value <= gpose.x )
  // ep = ep->next;

  assert( mod->polys );

  //stg_endpoint_t* ep = ((stg_polygon_t*)mod->polys->data)->epts[1]; // x max

  
  

  // initialize the scan to max range
  double bearing_index0 = gpose.a - cfg->fov/2.0;
  double bearing = bearing_index0;
  int t;
  for( t=0; t<cfg->samples; t++ )
    {
      double shortest = cfg->range_max;


      // for each model in the world that is not me

      // for each polygon in that model

      // for each line in that polygon

      // see if the ray intersects it

      // if the range is the shortest yet, store the range


      scan[t].range = 1.0;
      scan[t].hitpoint.x = gpose.x + scan[t].range * cos(bearing);
      scan[t].hitpoint.y = gpose.y + scan[t].range * sin(bearing);
      
      bearing += sample_incr;
    }
     


/*   GList* hitlist = NULL; */
/*   int index = 0; */
/*   int hitcount = 0; */
/*   while( ep ) */
/*     { */
/*       //printf( "gpose x: %.2f\n", gpose.x ); */
/*       //printf( "ep: %s %.2f\n", ep->polygon->mod->token, ep->value ); */
/*       //printf( "ep next: %s %.2f\n", ep->next->polygon->mod->token, ep->next->value ); */
      
/*       double xmin=0,ymin=0; */
      
/*       if( ep->type == STG_BEGIN ) */
/* 	{ */
/* 	  xmin = ep->polygon->epts[0].value; */
/* 	  ymin = ep->polygon->epts[2].value;	  */
	  
/* 	  printf( "see polygon of %s starting at (%.2f,%.2f) ", */
/* 		  ep->polygon->mod->token, */
/* 		  xmin, ymin );	  	   */

	  
/* 	  double global_angle = atan2( ymin - gpose.y, xmin - gpose.x );  */
/* 	  double scan_angle =  global_angle - gpose.a + cfg->fov/2.0; */
/* 	  int next_index = (int)(scan_angle / sample_incr); */
	  
/* 	  double range = cfg->range_max; */
/* 	  if( hitlist ) */
/* 	    { */
/* 	      stg_pose_t hispose; */
/* 	      stg_model_get_global_pose( ((stg_polygon_t*)hitlist->data)->mod, &hispose ); */
/* 	      range = hypot( hispose.y - gpose.y, hispose.x - gpose.x ); */
/* 	    } */

/* 	  int i; */
/* 	  for( i=index; i<next_index; i++ ) */
/* 	    { */
/* 	      scan[i].range = range; */
/* 	      //scan[t].hitpoint.x = gpose.x + scan[t].range * cos(bearing); */
/* 	      //scan[t].hitpoint.y = gpose.y + scan[t].range * sin(bearing); */
/* 	    } */

/* 	  index = next_index; */

/* 	  if( index >= cfg->samples ) break; */

/* 	  hitlist = g_list_prepend( hitlist, ep->polygon ); */
/* 	  hitcount++; */
/* 	} */

/*       if( ep->type == STG_END ) */
/* 	{ */
/* 	  xmin = ep->polygon->epts[1].value; */
/* 	  ymin = ep->polygon->epts[3].value; */
	  
/* 	  printf( "see polygon of %s ending at (%.2f,%.2f) ", */
/* 		  ep->polygon->mod->token, */
/* 		  xmin, ymin );	   */

/* 	  hitlist = g_list_remove( hitlist, ep->polygon ); */
/* 	  hitcount--; */
/* 	} */




     /*  if( hitcount > 0 ) */
/* 	{ */
/* 	  double angle = atan2( ymin - gpose.y, xmin - gpose.x ) - gpose.a + cfg->fov/2.0; */
	  
/* 	  int index2 = (int)(angle / sample_incr); */

/* 	  //for( */

	  
/* 	  scan[index2].range = hypot( ymin - gpose.y, xmin - gpose.x ); */
/* 	  scan[index2].hitpoint.x = xmin; */
/* 	  scan[index2].hitpoint.y = ymin; */
	  
/* 	  printf( "angle %.2f index % d hits", RTOD(angle), index ); */
/* 	  GList* it; */
/* 	  for( it=hitlist; it; it=it->next ) */
/* 	    printf( " %s", ((stg_polygon_t*)it->data)->mod->token ); */
/* 	  puts(""); */
/* 	} */

 /*      ep = ep->next; */
/*     } */
 


 /*  for( int t=0; t<cfg->samples; t += cfg->resolution ) */
/*     {       */
/*       double bearing =  gpose.a - cfg->fov/2.0 + sample_incr * t; */
      
/*       itl_t* itl = itl_create2( mod, gpose.x, gpose.y,  */
/* 				bearing,  */
/* 			       cfg->range_max,  */
/* 			       mod->world->matrix,  */
/* 			       PointToBearingRange, */
/* 				laser_raytrace_match ); */
      
/*       //double range = cfg->range_max; */
      
/*       stg_model_t* hitmod = itl_next( itl ); */
  
/*       if( hitmod ) */
/* 	{        */
/* 	  scan[t].range = MAX( itl->range, cfg->range_min ); */
/* 	  scan[t].hitpoint.x = itl->x; */
/* 	  scan[t].hitpoint.y = itl->y; */
/* 	} */
/*       else */
/* 	{ */
/* 	  scan[t].range = cfg->range_max;	   */
/* 	  scan[t].hitpoint.x = gpose.x + scan[t].range * cos(bearing); */
/* 	  scan[t].hitpoint.y = gpose.y + scan[t].range * sin(bearing); */
/* 	} */

/*       itl_destroy( itl ); */

/*       // lower bound on range */
      
/*       // if the object is bright, it has a non-zero reflectance */
/*       if( hitmod ) */
/* 	{ */
/* 	  scan[t].reflectance =  */
/* 	    (hitmod->laser_return >= LaserBright) ? 1.0 : 0.0; */
/* 	} */
/*       else */
/* 	scan[t].reflectance = 0.0; */
/*     } */
  
/*   /\*  we may need to resolution the samples we skipped *\/ */
/*   if( cfg->resolution > 1 )  */
/*     { */
/*       for( int t=cfg->resolution; t<cfg->samples; t+=cfg->resolution )  */
/* 	for( int g=1; g<cfg->resolution; g++ ) */
/* 	  { */
/* 	    if( t >= cfg->samples )  */
/* 	      break; */
	    
/* 	    // copy the rightmost sample data into this point */
/* 	    memcpy( &scan[t-g],  */
/* 		    &scan[t-cfg->resolution],  */
/* 		    sizeof(stg_laser_sample_t)); */
	    
/* 	    double left = scan[t].range; */
/* 	    double right = scan[t-cfg->resolution].range; */
	    
/* 	    // linear range interpolation between the left and right samples */
/* 	    scan[t-g].range = (left-g*(left-right)/cfg->resolution); */
/* 	  } */
/*     } */
  
  stg_model_set_data( mod, scan, sizeof(stg_laser_sample_t) * cfg->samples);
  
  
#if TIMING
  gettimeofday( &tv2, NULL );
  printf( " laser data update time %.6f\n",
	  (tv2.tv_sec + tv2.tv_usec / 1e6) - 
	  (tv1.tv_sec + tv1.tv_usec / 1e6) );	    
#endif

  return 0; //ok
}


int laser_startup( stg_model_t* mod, void* unused )
{ 
  PRINT_DEBUG( "laser startup" );
  puts( "laser startup" );
  
  // start consuming power
  stg_model_set_watts( mod, STG_LASER_WATTS );

  // install the update function
  stg_model_add_update_callback( mod, laser_update, NULL );

  stg_pose_t gpose;
  stg_model_get_global_pose( mod, &gpose );
  
  stg_laser_config_t *cfg = (stg_laser_config_t*)mod->cfg;

  // add an AABB to keep track of the things I can see

/*   stg_endpoint_t* aabb = calloc( sizeof(stg_endpoint_t), 6 ); */
  
/*   int i; */
/*   for( i=0; i<6; i++ ) */
/*     { */
/*       aabb[i].mod = mod; */
/*       aabb[i].type = i % 2; // 0 is STG_BEGIN, 1 is STG_END */
/*       aabb[i].value = 0.0; */
/*     } */

/*   aabb[0].value = gpose.x - cfg->range_max; */
/*   aabb[1].value = gpose.x + cfg->range_max; */

/*   mod->world->endpts.x = add_endpoint_to_list( mod->world->endpts.x, &aabb[0]); */
/*   mod->world->endpts.x = add_endpoint_to_list( mod->world->endpts.x, &aabb[1]); */

  
  return 0; // ok
}

int laser_shutdown( stg_model_t* mod, void* unused )
{ 
  PRINT_DEBUG( "laser shutdown" );
  
  // remove the update function
  stg_model_remove_update_callback( mod, laser_update );

  // stop consuming power
  stg_model_set_watts( mod, 0 );
  
  // clear the data - this will unrender it too
  stg_model_set_data( mod, NULL, 0 );

  return 0; // ok
}


