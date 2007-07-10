///////////////////////////////////////////////////////////////////////////
//
// File: model_laser.c
// Author: Richard Vaughan
// Date: 10 June 2004
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/model_laser.cc,v $
//  $Author: rtv $
//  $Revision: 1.1.2.4 $
//
///////////////////////////////////////////////////////////////////////////


//#define DEBUG 

#include <sys/time.h>
#include <math.h>
#include <GL/gl.h>

#include "gui.h"
#include "model.hh"


#define TIMING 0
#define LASER_FILLED 1


#define STG_DEFAULT_LASER_WATTS 17.5 // laser power consumption
#define STG_DEFAULT_LASER_SIZEX 0.15
#define STG_DEFAULT_LASER_SIZEY 0.15
#define STG_DEFAULT_LASER_SIZEZ 0.2
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

StgModelLaser::StgModelLaser( stg_world_t* world, 
			      StgModel* parent,
			      stg_id_t id,
			      CWorldFile* wf )
  : StgModel( world, parent, id, wf )
{
  PRINT_DEBUG2( "Constructing StgModelLaser %d (%s)\n", 
		id, wf->GetEntityType( id ) );
  
  // sensible laser defaults 
  this->update_interval_ms = 200; // common for a SICK LMS200

  stg_geom_t geom; 
  memset( &geom, 0, sizeof(geom));
  geom.size.x = STG_DEFAULT_LASER_SIZEX;
  geom.size.y = STG_DEFAULT_LASER_SIZEY;
  geom.size.z = STG_DEFAULT_LASER_SIZEZ;
  this->SetGeom( &geom );

  // set default color
  this->SetColor( stg_lookup_color(STG_LASER_GEOM_COLOR));

  // set up a laser-specific config structure
  stg_laser_config_t lconf;
  memset(&lconf,0,sizeof(lconf));  
  lconf.range_min   = STG_DEFAULT_LASER_MINRANGE;
  lconf.range_max   = STG_DEFAULT_LASER_MAXRANGE;
  lconf.fov         = STG_DEFAULT_LASER_FOV;
  lconf.samples     = STG_DEFAULT_LASER_SAMPLES;  
  lconf.resolution = 1;
  this->SetCfg( &lconf, sizeof(lconf) );

  this->SetData( NULL, 0 );
}

StgModelLaser::~StgModelLaser( void )
{
  // empty
}

void StgModelLaser::Load( void )
{  
  StgModel::Load(); 
  
  if( wf ) // if we have a valid worldfile pointer, load from the file
    {
      stg_laser_config_t* cfg = (stg_laser_config_t*)this->cfg;
      
      cfg->samples   = wf->ReadInt( this->id, "samples", cfg->samples );
      cfg->range_min = wf->ReadLength( this->id, "range_min", cfg->range_min);
      cfg->range_max = wf->ReadLength( this->id, "range_max", cfg->range_max );
      cfg->fov       = wf->ReadAngle( this->id, "fov",  cfg->fov );
      
      cfg->resolution = wf->ReadInt( this->id, "laser_sample_skip",  cfg->resolution );
      
      if( cfg->resolution < 1 )
	{
	  PRINT_WARN( "laser resolution set < 1. Forcing to 1" );
	  cfg->resolution = 1;
	}
 
      this->SetCfg( cfg, sizeof(stg_laser_config_t));
    }
}

void stg_laser_config_print( stg_laser_config_t* slc )
{
  printf( "slc fov: %.2f  range_min: %.2f range_max: %.2f samples: %d\n",
	  slc->fov,
	  slc->range_min,
	  slc->range_max,
	  slc->samples );
}

int laser_raytrace_match( StgModel* mod, StgModel* hitmod )
{           
  stg_laser_return_t ret;
  hitmod->GetLaserReturn( &ret );

  // Ignore my relatives and thiings that are invisible to lasers
  return( //(! stg_model_is_related(mod,hitmod)) && 
	 (mod != hitmod) && (ret > 0) );
}	

void StgModelLaser::Update( void )
{     
  StgModel::Update();

  puts( "LASER UPDATE" );

  // no work to do if we're unsubscribed
  if( this->subs < 1 )
    return;
  
  stg_laser_config_t* cfg = (stg_laser_config_t*)this->cfg;
  assert(cfg);

  stg_geom_t geom;
  this->GetGeom( &geom );

  // get the sensor's pose in global coords
  stg_pose_t gpose;
  this->GetGlobalPose( &gpose );

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

  for( int t=0; t<cfg->samples; t += cfg->resolution )
    {
      double bearing =  gpose.a - cfg->fov/2.0 + sample_incr * t;
      
      itl_t* itl = itl_create( gpose.x, gpose.y, gpose.z,
			       bearing,
			       cfg->range_max,
			       this->world->matrix,
			       PointToBearingRange );
      //				laser_raytrace_match );
      
      double range = cfg->range_max;
      
      StgModel* hitmod;      
      if((hitmod = itl_first_matching( itl, 
				       laser_raytrace_match, 
				       this ) ))
	{
	  range = itl->range;
	}
          
      if( hitmod )
	{
	  //printf( "hit model %s\n", hitmod->Token() );
	  scan[t].range = MAX( itl->range, cfg->range_min );
	  scan[t].hitpoint.x = itl->x;
	  scan[t].hitpoint.y = itl->y;
	}
      else
	{
	  scan[t].range = cfg->range_max;
	  scan[t].hitpoint.x = gpose.x + scan[t].range * cos(bearing);
	  scan[t].hitpoint.y = gpose.y + scan[t].range * sin(bearing);
	}
      
      itl_destroy( itl );
      
      // todo - lower bound on range
      
      // if the object is bright, it has a non-zero reflectance
      if( hitmod )
	{
	  stg_laser_return_t ret;
	  hitmod->GetLaserReturn( &ret );
	  scan[t].reflectance =
	    (ret >= LaserBright) ? 1.0 : 0.0;
	}
      else
	scan[t].reflectance = 0.0;
    }
  
  /*  we may need to resolution the samples we skipped */
  if( cfg->resolution > 1 )
    {
      for( int t=cfg->resolution; t<cfg->samples; t+=cfg->resolution )
	for( int g=1; g<cfg->resolution; g++ )
	  {
	    if( t >= cfg->samples )
	      break;
	    
	    // copy the rightmost sample data into this point
	    memcpy( &scan[t-g],
		    &scan[t-cfg->resolution],
		    sizeof(stg_laser_sample_t));
	    
	    double left = scan[t].range;
	    double right = scan[t-cfg->resolution].range;
	    
	    // linear range interpolation between the left and right samples
	    scan[t-g].range = (left-g*(left-right)/cfg->resolution);
	  }
    }
  
  this->SetData( scan, sizeof(stg_laser_sample_t) * cfg->samples);
  
  
#if TIMING
  gettimeofday( &tv2, NULL );
  printf( " laser data update time %.6f\n",
	  (tv2.tv_sec + tv2.tv_usec / 1e6) - 
	  (tv1.tv_sec + tv1.tv_usec / 1e6) );	    
#endif
}


void StgModelLaser::Startup(  void )
{ 
  StgModel::Startup();

  PRINT_DEBUG( "laser startup" );
  puts( "laser startup" );
  
  // start consuming power
  this->SetWatts( STG_DEFAULT_LASER_WATTS );

  // install the update function
  //stg_model_add_update_callback( mod, laser_update, NULL );
}

void StgModelLaser::Shutdown( void )
{ 
  StgModel::Shutdown();

  PRINT_DEBUG( "laser shutdown" );
  
  // remove the update function
  //stg_model_remove_update_callback( mod, laser_update );

  // stop consuming power
  this->SetWatts( 0 );
  
  // clear the data - this will unrender it too
  this->SetData( NULL, 0 );
}


// void StgModelLaser::Draw( void )
// { 
//   StgModel::Draw();
// }

void StgModelLaser::GuiGenerateData( void )
{
  puts ("GL laser data" );

  glNewList( this->dl_data, GL_COMPILE );

  stg_laser_sample_t* samples = (stg_laser_sample_t*)this->data;
  size_t sample_count = this->data_len / sizeof(stg_laser_sample_t);
  stg_laser_config_t *cfg = (stg_laser_config_t*)this->cfg;
  assert( cfg );
  
  if( samples && sample_count )
    {
      // do alpha properly
      glDepthMask( GL_FALSE );
      
      glPushMatrix();
      glTranslatef( 0,0, this->geom.size.z/2.0 ); // shoot the laser beam out at the right height
      
      // pack the laser hit points into a vertex array for fast rendering
      static float* pts = NULL;
      pts = (float*)g_realloc( pts, 2 * (sample_count+1) * sizeof(float));
      
      pts[0] = 0.0;
      pts[1] = 0.0;
      
      for( int s=0; s<sample_count; s++ )
	{
	  double ray_angle = (s * (cfg->fov / (sample_count-1))) - cfg->fov/2.0;  
	  pts[2*s+2] = (float)(samples[s].range * cos(ray_angle) );
	  pts[2*s+3] = (float)(samples[s].range * sin(ray_angle) );
	}
      
      glEnableClientState( GL_VERTEX_ARRAY );
      glVertexPointer( 2, GL_FLOAT, 0, pts );
      
      glColor4f( 0, 0, 1, 0.1 );
      glPolygonMode( GL_FRONT_AND_BACK, world->win->show_alpha ? GL_FILL : GL_LINES );
      glDrawArrays( GL_POLYGON, 0, sample_count+1 );
      
      glPopMatrix();
      glDepthMask( GL_TRUE );
    }
  
  glEndList();
  
  world->win->dirty = true;

  //make_dirty(mod);

/*   // loop through again, drawing bright boxes on top of the polygon */
/*   for( s=0; s<sample_count; s++ ) */
/*     {       */
/*       // if this hit point is bright, we draw a little box */
/*       if( samples[s].reflectance > 0 ) */
/* 	{ */
 /* 	  stg_rtk_fig_color_rgb32( fg, bright_color ); */
/* 	  stg_rtk_fig_rectangle( fg,  */
/* 				 points[1+s].x, points[1+s].y, 0, */
/* 				 0.04, 0.04, 1 ); */
/* 	  stg_rtk_fig_color_rgb32( fg, laser_color ); */
/* 	} */
/*     } */
}


