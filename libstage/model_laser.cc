 ///////////////////////////////////////////////////////////////////////////
 //
 // File: model_laser.c
 // Author: Richard Vaughan
 // Date: 10 June 2004
 //
 // CVS info:
 //  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/libstage/model_laser.cc,v $
 //  $Author: rtv $
 //  $Revision: 1.6 $
 //
 ///////////////////////////////////////////////////////////////////////////

 //#define DEBUG 

 #include <sys/time.h>
 #include "stage_internal.hh"

// DEFAULT PARAMETERS FOR LASER MODEL
static const bool DEFAULT_LASER_FILLED = true;
static const stg_watts_t DEFAULT_LASER_WATTS = 17.5; // laser power consumption
static const stg_meters_t DEFAULT_LASER_SIZEX = 0.15;
static const stg_meters_t DEFAULT_LASER_SIZEY = 0.15;
static const stg_meters_t DEFAULT_LASER_SIZEZ = 0.2;
static const stg_meters_t DEFAULT_LASER_MINRANGE = 0.0;
static const stg_meters_t DEFAULT_LASER_MAXRANGE = 8.0;
static const stg_radians_t DEFAULT_LASER_FOV = M_PI;
static const unsigned int DEFAULT_LASER_SAMPLES = 180;
static const stg_msec_t DEFAULT_LASER_INTERVAL_MS = 100;
static const unsigned int DEFAULT_LASER_RESOLUTION = 1;

static const char LASER_GEOM_COLOR[] = "blue";
//static const char LASER_BRIGHT_COLOR[] = "blue";
//static const char LASER_CFG_COLOR[] = "light steel blue";
//static const char LASER_COLOR[] = "steel blue";
//static const char LASER_FILL_COLOR[] = "powder blue";

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
			      StgModel* parent,
			      stg_id_t id,
			      char* typestr )
  : StgModel( world, parent, id, typestr )
{
  PRINT_DEBUG2( "Constructing StgModelLaser %d (%s)\n", 
		id, typestr );
  
  // sensible laser defaults 
  interval = 1e3 * DEFAULT_LASER_INTERVAL_MS; 
  laser_return = LaserVisible;
  
  stg_geom_t geom; 
  memset( &geom, 0, sizeof(geom));
  geom.size.x = DEFAULT_LASER_SIZEX;
  geom.size.y = DEFAULT_LASER_SIZEY;
  geom.size.z = DEFAULT_LASER_SIZEZ;
  SetGeom( geom );
  
  // set default color
  SetColor( stg_lookup_color(LASER_GEOM_COLOR));
  
  range_min    = DEFAULT_LASER_MINRANGE;
  range_max    = DEFAULT_LASER_MAXRANGE;
  fov          = DEFAULT_LASER_FOV;
  sample_count = DEFAULT_LASER_SAMPLES;  
  resolution   = DEFAULT_LASER_RESOLUTION;
  
  // don't allocate sample buffer memory until Update() is called
  samples = NULL;
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

  Worldfile* wf = world->GetWorldFile();
  
  sample_count = wf->ReadInt( id, "samples", sample_count );        
  range_min = wf->ReadLength( id, "range_min", range_min);
  range_max = wf->ReadLength( id, "range_max", range_max );
  fov       = wf->ReadAngle( id, "fov",  fov );      
  resolution = wf->ReadInt( id, "laser_sample_skip",  resolution );
  
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
				  const void* zp )
{
  // Ignore the model that's looking and things that are invisible to
  // lasers

  double z = *(stg_meters_t*)zp;

  if( (testblock->Model() != finder) &&
      testblock->IntersectGlobalZ( z ) &&
      (testblock->Model()->GetLaserReturn() > 0 ) )
    return true; // match!

  return false; // no match
}	

void StgModelLaser::Update( void )
{     
  double bearing = -fov/2.0;
  double sample_incr = fov / (double)(sample_count-1);  
  
  stg_pose_t gpose = GetGlobalPose();
  stg_meters_t zscan = gpose.z + geom.size.z /2.0;

  samples = g_renew( stg_laser_sample_t, samples, sample_count );

  for( unsigned int t=0; t<sample_count; t += resolution )
    {
      stg_raytrace_sample_t sample;
      Raytrace( bearing, 
		range_max,
		laser_raytrace_match,
		&zscan, // height of scan line
		&sample );
      
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
  SetWatts( DEFAULT_LASER_WATTS );
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




void StgModelLaser::DataVisualize( void )
{
  if( ! (samples && sample_count) )
    return;
  
  glPushMatrix();
  glTranslatef( 0,0, geom.size.z/2.0 ); // shoot the laser beam out at the right height
  
  // pack the laser hit points into a vertex array for fast rendering
  static float* pts = NULL;
  pts = (float*)g_realloc( pts, 2 * (sample_count+1) * sizeof(float));
  
  pts[0] = 0.0;
  pts[1] = 0.0;
  
  PushColor( 0, 0, 1, 0.5 );
  
  for( unsigned int s=0; s<sample_count; s++ )
    {
      double ray_angle = (s * (fov / (sample_count-1))) - fov/2.0;  
      pts[2*s+2] = (float)(samples[s].range * cos(ray_angle) );
      pts[2*s+3] = (float)(samples[s].range * sin(ray_angle) );
      
      // if the sample is unusually bright, draw a little blob
      if( samples[s].reflectance > 0 )
	{
	  glPointSize( 4.0 );
	  glBegin( GL_POINTS );
	  glVertex2f( pts[2*s+2], pts[2*s+3] );
	  glEnd();
	}
      
    }
  PopColor();
  
  
  glEnableClientState( GL_VERTEX_ARRAY );
  glVertexPointer( 2, GL_FLOAT, 0, pts );   
  
  glDepthMask( GL_FALSE );
  glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
  
  // draw the filled polygon in transparent blue
  PushColor( 0, 0, 1, 0.1 );
  glDrawArrays( GL_POLYGON, 0, sample_count+1 );
  
  // draw the beam strike points in black
  PushColor( 0, 0, 0, 1.0 );
  glPointSize( 1.0 );
  glDrawArrays( GL_POINTS, 0, sample_count+1 );
  
  // reset
  PopColor();
  PopColor();      
  glDepthMask( GL_TRUE );
  glPopMatrix();      
}



