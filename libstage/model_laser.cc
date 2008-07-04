///////////////////////////////////////////////////////////////////////////
//
// File: model_laser.c
// Author: Richard Vaughan
// Date: 10 June 2004
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/libstage/model_laser.cc,v $
//  $Author: rtv $
//  $Revision: 1.7 $
//
///////////////////////////////////////////////////////////////////////////

//#define DEBUG 

#include <sys/time.h>
#include "stage_internal.hh"

// DEFAULT PARAMETERS FOR LASER MODEL
static const bool DEFAULT_FILLED = true;
static const stg_watts_t DEFAULT_WATTS = 17.5;
static const stg_size_t DEFAULT_SIZE = {0.15, 0.15, 0.2 };
static const stg_meters_t DEFAULT_MINRANGE = 0.0;
static const stg_meters_t DEFAULT_MAXRANGE = 8.0;
static const stg_radians_t DEFAULT_FOV = M_PI;
static const unsigned int DEFAULT_SAMPLES = 180;
static const stg_msec_t DEFAULT_INTERVAL_MS = 100;
static const unsigned int DEFAULT_RESOLUTION = 1;

static const char DEFAULT_GEOM_COLOR[] = "blue";
//static const char BRIGHT_COLOR[] = "blue";
//static const char CFG_COLOR[] = "light steel blue";
//static const char COLOR[] = "steel blue";
//static const char FILL_COLOR[] = "powder blue";

Option StgModelLaser::showLaserData( "Show Laser Data", "show_laser", "", true );
Option StgModelLaser::showLaserStrikes( "Show Laser Data", "show_laser_strikes", "", false );

/**
   @ingroup model
   @defgroup model_laser Laser model 
   The laser model simulates a scanning laser rangefinder

   API: Stg::StgModelLaser

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

StgModelLaser::StgModelLaser( StgWorld* world, 
										StgModel* parent )
  : StgModel( world, parent, MODEL_TYPE_LASER )
{
  PRINT_DEBUG2( "Constructing StgModelLaser %d (%s)\n", 
					 id, typestr );
  
  // sensible laser defaults 
  interval = DEFAULT_INTERVAL_MS * (int)thousand;
  laser_return = LaserVisible;

  stg_geom_t geom;
  memset( &geom, 0, sizeof(geom));
  geom.size = DEFAULT_SIZE;
  SetGeom( geom );

  // set default color
  SetColor( stg_lookup_color(DEFAULT_GEOM_COLOR));

  range_min    = DEFAULT_MINRANGE;
  range_max    = DEFAULT_MAXRANGE;
  fov          = DEFAULT_FOV;
  sample_count = DEFAULT_SAMPLES;
  resolution   = DEFAULT_RESOLUTION;
  data_dirty = true;

  // don't allocate sample buffer memory until Update() is called
  samples = NULL;
	
  data_dl = glGenLists(1);
       
  registerOption( &showLaserData );
  registerOption( &showLaserStrikes );
}


StgModelLaser::~StgModelLaser( void )
{
  if(samples)
    {
      g_free( samples );
      samples = NULL;
    }
}

void StgModelLaser::Load( void )
{  
  StgModel::Load();
  sample_count = wf->ReadInt( wf_entity, "samples", sample_count );
  range_min = wf->ReadLength( wf_entity, "range_min", range_min);
  range_max = wf->ReadLength( wf_entity, "range_max", range_max );
  fov       = wf->ReadAngle( wf_entity, "fov",  fov );
  resolution = wf->ReadInt( wf_entity, "laser_sample_skip",  resolution );

  if( resolution < 1 )
    {
      PRINT_WARN( "laser resolution set < 1. Forcing to 1" );
      resolution = 1;
    }
}

stg_laser_cfg_t StgModelLaser::GetConfig()
{ 
  stg_laser_cfg_t cfg;
  cfg.sample_count = sample_count;
  cfg.range_bounds.min = range_min;
  cfg.range_bounds.max = range_max;
  cfg.fov = fov;
  cfg.resolution = resolution;
  return cfg;
}

void StgModelLaser::SetConfig( stg_laser_cfg_t cfg )
{ 
  range_min = cfg.range_bounds.min;
  range_max = cfg.range_bounds.max;
  fov = cfg.fov;
  resolution = cfg.resolution;
}

static bool laser_raytrace_match( StgBlock* testblock, 
											 StgModel* finder,
											 const void* dummy )
{
  // Ignore the model that's looking and things that are invisible to
  // lasers

  if( (testblock->Model() != finder) &&
      (testblock->Model()->GetLaserReturn() > 0 ) )
    return true; // match!

  return false; // no match
}	

void StgModelLaser::Update( void )
{     
  double bearing = -fov/2.0;
  double sample_incr = fov / (double)(sample_count-1);

  samples = g_renew( stg_laser_sample_t, samples, sample_count );

  stg_pose_t rayorg = geom.pose;
  bzero( &rayorg, sizeof(rayorg));
  rayorg.z += geom.size.z/2;

  for( unsigned int t=0; t<sample_count; t += resolution )
    {
      stg_raytrace_sample_t sample;

      rayorg.a = bearing;

      Raytrace( rayorg, 
					 range_max,
					 laser_raytrace_match,
					 NULL,
					 &sample,
					 true ); // z testing enabled
		
      samples[t].range = sample.range;

      // if we hit a model and it reflects brightly, we set
      // reflectance high, else low
      if( sample.block && ( sample.block->Model()->GetLaserReturn() >= LaserBright ) )	
		  samples[t].reflectance = 1;
      else
		  samples[t].reflectance = 0;

      // todo - lower bound on range      
      bearing += sample_incr;
    }

  // we may need to interpolate the samples we skipped 
  if( resolution > 1 )
    {
      for( unsigned int t=resolution; t<sample_count; t+=resolution )
		  for( unsigned int g=1; g<resolution; g++ )
			 {
				if( t >= sample_count )
				  break;

				// copy the rightmost sample data into this point
				memcpy( &samples[t-g],
						  &samples[t-resolution],
						  sizeof(stg_laser_sample_t));

				double left = samples[t].range;
				double right = samples[t-resolution].range;

				// linear range interpolation between the left and right samples
				samples[t-g].range = (left-g*(left-right)/resolution);
			 }
    }

  data_dirty = true;

  StgModel::Update();
}


void StgModelLaser::Startup(  void )
{ 
  StgModel::Startup();

  PRINT_DEBUG( "laser startup" );

  // start consuming power
  SetWatts( DEFAULT_WATTS );
}

void StgModelLaser::Shutdown( void )
{ 
  PRINT_DEBUG( "laser shutdown" );

  // stop consuming power
  SetWatts( 0 );

  // clear the data  
  if(samples)
    {
      g_free( samples );
      samples = NULL;
    }

  StgModel::Shutdown();
}

void StgModelLaser::Print( char* prefix )
{
  StgModel::Print( prefix );

  printf( "\tRanges[ " );

  if( samples )
    for( unsigned int i=0; i<sample_count; i++ )
      printf( "%.2f ", samples[i].range );
  else
    printf( "<none until subscribed>" );
  puts( " ]" );

  printf( "\tReflectance[ " );

  if( samples )
    for( unsigned int i=0; i<sample_count; i++ )
      printf( "%.2f ", samples[i].reflectance );
  else
    printf( "<none until subscribed>" );

  puts( " ]" );
}


stg_laser_sample_t* StgModelLaser::GetSamples( uint32_t* count )
{ 
  if( count ) *count = sample_count;
  return samples;
}

void StgModelLaser::SetSamples( stg_laser_sample_t* samples, uint32_t count)
{ 
  this->samples = g_renew( stg_laser_sample_t, this->samples, sample_count );
  memcpy( this->samples, samples, sample_count * sizeof(stg_laser_sample_t));
  this->sample_count = count;
  this->data_dirty = true;
}


void StgModelLaser::DataVisualize( void )
{
  if( ! (samples && sample_count) )
    return;

  if ( ! (showLaserData || showLaserStrikes) )
    return;
    
	glPushMatrix();
	
  // we only regenerate the list if there's new data
  if( data_dirty )
    {	    
      data_dirty = false;

      glNewList( data_dl, GL_COMPILE );
		//glEnableClientState( GL_VERTEX_ARRAY );     
      glTranslatef( 0,0, geom.size.z/2.0 ); // shoot the laser beam out at the right height
      
      PushColor( 0, 0, 1, 0.5 );
      
      glPointSize( 4.0 );

		// DEBUG - draw the origin of the laser beams
		glBegin( GL_POINTS );
		glVertex2f( 0,0 );
		glEnd();
		
      // pack the laser hit points into a vertex array for fast rendering
      static float* pts = NULL;
      pts = (float*)g_realloc( pts, 2 * (sample_count+1) * sizeof(float));
      
      pts[0] = 0.0;
      pts[1] = 0.0;
      
      
      glVertexPointer( 2, GL_FLOAT, 0, pts );
      
      for( unsigned int s=0; s<sample_count; s++ )
		  {
			 double ray_angle = (s * (fov / (sample_count-1))) - fov/2.0;
			 pts[2*s+2] = (float)(samples[s].range * cos(ray_angle) );
			 pts[2*s+3] = (float)(samples[s].range * sin(ray_angle) );
	  
			 // if the sample is unusually bright, draw a little blob
			 if( samples[s].reflectance > 0 )
				{
				  glBegin( GL_POINTS );
				  glVertex2f( pts[2*s+2], pts[2*s+3] );
				  glEnd();
	      
				  // why doesn't this work?
				  //glDrawArrays( GL_POINTS, 2*s+2, 1 );
				}
			 
		  }
      PopColor();
      

      glDepthMask( GL_FALSE );
      glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
      
      // draw the filled polygon in transparent blue
      PushColor( 0, 0, 1, 0.1 );

		
      glDrawArrays( GL_POLYGON, 0, sample_count+1 );
  
      // reset
      PopColor();

		if( showLaserStrikes )
		  {
			 // draw the beam strike points in black
			 PushColor( 0, 0, 0, 1.0 );
			 glPointSize( 1.0 );
			 glDrawArrays( GL_POINTS, 0, sample_count+1 );
			 PopColor();
		  }

      glDepthMask( GL_TRUE );


		//glDisableClientState( GL_VERTEX_ARRAY );           
      glEndList();
    }
  
  glCallList( data_dl );
	    
	glPopMatrix();
}
