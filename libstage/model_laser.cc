// file: model_laser.c
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

#include "stage.hh"
#include "worldfile.hh"
#include "option.hh"
using namespace Stg;

// DEFAULT PARAMETERS FOR LASER MODEL
static const bool DEFAULT_FILLED = true;
static const stg_watts_t DEFAULT_WATTS = 17.5;
static const Size DEFAULT_SIZE( 0.15, 0.15, 0.2 );
static const stg_meters_t DEFAULT_MAXRANGE = 8.0;
static const stg_radians_t DEFAULT_FOV = M_PI;
static const unsigned int DEFAULT_SAMPLES = 180;
static const stg_msec_t DEFAULT_INTERVAL_MS = 100;
static const unsigned int DEFAULT_RESOLUTION = 1;
static const char* DEFAULT_COLOR = "blue";

//TODO make instance attempt to register an option (as customvisualizations do)
Option ModelLaser::Vis::showArea( "Laser scans", "show_laser", "", true, NULL );
Option ModelLaser::Vis::showStrikes( "Laser strikes", "show_laser_strikes", "", false, NULL );
Option ModelLaser::Vis::showFov( "Laser FOV", "show_laser_fov", "", false, NULL );
Option ModelLaser::Vis::showBeams( "Laser beams", "show_laser_beams", "", false, NULL );

/**
   @ingroup model
   @defgroup model_laser Laser model 
   The laser model simulates a scanning laser rangefinder

   API: Stg::ModelLaser

   <h2>Worldfile properties</h2>

   @par Summary and default values

   @verbatim
   laser
   (
   # laser properties
   samples 180
   range_max 8.0
   fov 3.14159
   resolution 1

   # model properties
   size [ 0.15 0.15 0.2 ]
   color "blue"
   )
   @endverbatim

   @par Details
 
   - samples <int>\n
   the number of laser samples per scan
   - range_max <float>\n
   the maximum range reported by the scanner, in meters. The scanner will not detect objects beyond this range.
   - fov <float>\n
   the angular field of view of the scanner, in radians. 
   - resolution <int>\n
   Only calculate the true range of every nth laser sample. The missing samples are filled in with a linear interpolation. Generally it would be better to use fewer samples, but some (poorly implemented!) programs expect a fixed number of samples. Setting this number > 1 allows you to reduce the amount of computation required for your fixed-size laser vector.
*/
  
// create static vis objects

ModelLaser::Vis::Vis( World* world ) 
  : Visualizer( "Laser", "laser_vis" )
{
  world->RegisterOption( &showArea );
  world->RegisterOption( &showStrikes );
  world->RegisterOption( &showFov );
  world->RegisterOption( &showBeams );		  
}

void ModelLaser::Vis::Visualize( Model* mod, Camera* cam ) 
{
  ModelLaser* laser = dynamic_cast<ModelLaser*>(mod);
  unsigned int sample_count = 0;
  stg_laser_sample_t* samples = laser->GetSamples( &sample_count );
    
  glPushMatrix();
  glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
  
  glTranslatef( 0,0, laser->GetGeom().size.z/2.0 ); // shoot the laser beam out at the right height
  
  // pack the laser hit points into a vertex array for fast rendering
  static float* pts = NULL;
  pts = (float*)g_realloc( pts, 2 * (sample_count+1) * sizeof(float));
  
  pts[0] = 0.0;
  pts[1] = 0.0;
  
  laser->PushColor( 1, 0, 0, 0.5 );
  glDepthMask( GL_FALSE );
  glPointSize( 2 );
  
  for( unsigned int s=0; s<sample_count; s++ )
	 {
		double ray_angle = (s * (laser->fov / (sample_count-1))) - laser->fov/2.0;
		pts[2*s+2] = (float)(samples[s].range * cos(ray_angle) );
		pts[2*s+3] = (float)(samples[s].range * sin(ray_angle) );
		
		// if the sample is unusually bright, draw a little blob
		if( samples[s].reflectance > 0 )
		  {
			 glBegin( GL_POINTS );
			 glVertex2f( pts[2*s+2], pts[2*s+3] );
			 glEnd();
		  }			 
	 }
  
  glVertexPointer( 2, GL_FLOAT, 0, pts );      
  
  laser->PopColor();
  
  if( showArea )
	 {
		// draw the filled polygon in transparent blue
		laser->PushColor( 0, 0, 1, 0.1 );		
		glDrawArrays( GL_POLYGON, 0, sample_count+1 );
		laser->PopColor();  
		//glDepthMask( GL_TRUE );
	 }
  
  glDepthMask( GL_TRUE );

  if( showStrikes )
	 {
		// draw the beam strike points
		laser->PushColor( 0, 0, 1, 0.8 );
		glDrawArrays( GL_POINTS, 0, sample_count+1 );
		laser->PopColor();
	 }
  
  if( showFov )
	 {
		for( unsigned int s=0; s<sample_count; s++ )
		  {
			 double ray_angle = (s * (laser->fov / (sample_count-1))) - laser->fov/2.0;
			 pts[2*s+2] = (float)(laser->range_max * cos(ray_angle) );
	      pts[2*s+3] = (float)(laser->range_max * sin(ray_angle) );			 
		  }
		
		glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
		laser->PushColor( 0, 0, 1, 0.5 );		
		glDrawArrays( GL_POLYGON, 0, sample_count+1 );
		laser->PopColor();
		//			 glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
	 }			 
  
  if( showBeams )
	 {
		laser->PushColor( 0, 0, 1, 0.5 );		
		glBegin( GL_LINES );
		
		for( unsigned int s=0; s<sample_count; s++ )
		  {
			 
			 glVertex2f( 0,0 );
			 double ray_angle = (s * (laser->fov / (sample_count-1))) - laser->fov/2.0;
			 glVertex2f( samples[s].range * cos(ray_angle), 
							 samples[s].range * sin(ray_angle) );
			 
	    }
		glEnd();
		laser->PopColor();
	 }	

  glPopMatrix();
}


ModelLaser::ModelLaser( World* world, 
			Model* parent )
  : Model( world, parent, MODEL_TYPE_LASER ),
	 vis( world ),
    data_dl(0),
    data_dirty( true ),
    sample_count( DEFAULT_SAMPLES ),
    samples(),
    range_max( DEFAULT_MAXRANGE ),
    fov( DEFAULT_FOV ),
    resolution( DEFAULT_RESOLUTION ),
	 rays()
{
  
  PRINT_DEBUG2( "Constructing ModelLaser %d (%s)\n", 
		id, typestr );
  

  // Model data members
  interval = DEFAULT_INTERVAL_MS * (int)thousand;
  
  Geom geom;
  memset( &geom, 0, sizeof(geom));
  geom.size = DEFAULT_SIZE;
  SetGeom( geom );
  
  rays.resize( sample_count );
  samples.resize( sample_count );

  // assert that Update() is reentrant for this derived model
  thread_safe = true;

  // set default color
  SetColor( stg_lookup_color(DEFAULT_COLOR));

  AddVisualizer( &vis, true );
}


ModelLaser::~ModelLaser( void )
{
}

void ModelLaser::Load( void )
{  
  sample_count = wf->ReadInt( wf_entity, "samples", sample_count );
  range_max = wf->ReadLength( wf_entity, "range_max", range_max );
  fov       = wf->ReadAngle( wf_entity, "fov",  fov );
  resolution = wf->ReadInt( wf_entity, "resolution",  resolution );
  
  //showLaserData.Load( wf, wf_entity );
  //showLaserStrikes.Load( wf, wf_entity );

  if( resolution < 1 )
    {
      PRINT_WARN( "laser resolution set < 1. Forcing to 1" );
      resolution = 1;
    }

  Model::Load();
  
  rays.resize(sample_count);
  samples.resize(sample_count);
}

stg_laser_cfg_t ModelLaser::GetConfig()
{ 
  stg_laser_cfg_t cfg;
  cfg.sample_count = sample_count;
  cfg.range_bounds.max = range_max;
  cfg.fov = fov;
  cfg.resolution = resolution;
  cfg.interval = interval;
  return cfg;
}

void ModelLaser::SetConfig( stg_laser_cfg_t cfg )
{ 
  range_max = cfg.range_bounds.max;
  fov = cfg.fov;
  resolution = cfg.resolution;
  interval = cfg.interval;
  sample_count = cfg.sample_count;

  samples.resize( sample_count );
  rays.resize( sample_count );
}

static bool laser_raytrace_match( Model* hit, 
				  Model* finder,
				  const void* dummy )
{
  // Ignore the model that's looking and things that are invisible to
  // lasers  
  return( (hit != finder) && (hit->vis.laser_return > 0 ) );
}	

// void ModelLaser::SetSampleCount( unsigned int count )
// {
//   sample_count = count;
  
//   samples.resize( count );
//   rays.resize( count );

void ModelLaser::Update( void )
{     
  double bearing = -fov/2.0;
  double sample_incr = fov / (double)(sample_count-1);

  Pose rayorg = geom.pose;
  bzero( &rayorg, sizeof(rayorg));
  rayorg.z += geom.size.z/2;
  
  assert( samples.size() == sample_count );
  
  for( unsigned int t=0; t<sample_count; t += resolution )
    {
      rayorg.a = bearing;
		
		// set up the ray
		rays[t].origin = LocalToGlobal(rayorg);

		// todo - do this constant stuff in advance
		rays[t].func = laser_raytrace_match;
		rays[t].arg = NULL;
		rays[t].range = range_max;
		rays[t].arg = NULL;
		rays[t].ztest = true;
		rays[t].mod = this;
		
      bearing += sample_incr;
	 }
  
  // do the raytracing of all rays in one go (allows parallel implementation)
  world->Raytrace( rays );
  
  // now process the results and fill in our samples
  for( unsigned int t=0; t<sample_count; t += resolution )
    {
		stg_raytrace_result_t* r = &rays[t].result;
		assert( r );

      samples[t].range = r->range;
		
      // if we hit a model and it reflects brightly, we set
      // reflectance high, else low
      if( r->mod && ( r->mod->vis.laser_return >= LaserBright ) )	
		  samples[t].reflectance = 1;
      else
		  samples[t].reflectance = 0;		
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
  
  Model::Update();
}


void ModelLaser::Startup(  void )
{ 
  Model::Startup();

  PRINT_DEBUG( "laser startup" );

  // start consuming power
  SetWatts( DEFAULT_WATTS );
}

void ModelLaser::Shutdown( void )
{ 
  PRINT_DEBUG( "laser shutdown" );

  // stop consuming power
  SetWatts( 0 );

  
  Model::Shutdown();
}

void ModelLaser::Print( char* prefix )
{
  Model::Print( prefix );

  printf( "\tRanges[ " );
  for( unsigned int i=0; i<sample_count; i++ )
      printf( "%.2f ", samples[i].range );
  puts( " ]" );

  printf( "\tReflectance[ " );
  for( unsigned int i=0; i<sample_count; i++ )
	 printf( "%.2f ", samples[i].reflectance );
  puts( " ]" );
}


stg_laser_sample_t* ModelLaser::GetSamples( uint32_t* count )
{ 
  if( count ) *count = sample_count;
  return &samples[0];
}
