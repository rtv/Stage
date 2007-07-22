 ///////////////////////////////////////////////////////////////////////////
 //
 // File: model_laser.c
 // Author: Richard Vaughan
 // Date: 10 June 2004
 //
 // CVS info:
 //  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/model_laser.cc,v $
 //  $Author: rtv $
 //  $Revision: 1.1.2.8 $
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
 #define STG_DEFAULT_LASER_INTERVAL_MS 200
 #define STG_DEFAULT_LASER_RESOLUTION 1

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
   update_interval_ms = STG_DEFAULT_LASER_INTERVAL_MS; 
   laser_return = LaserVisible;

    stg_geom_t geom; 
    memset( &geom, 0, sizeof(geom));
    geom.size.x = STG_DEFAULT_LASER_SIZEX;
    geom.size.y = STG_DEFAULT_LASER_SIZEY;
    geom.size.z = STG_DEFAULT_LASER_SIZEZ;
    SetGeom( &geom );

   // set default color
   SetColor( stg_lookup_color(STG_LASER_GEOM_COLOR));

   range_min    = STG_DEFAULT_LASER_MINRANGE;
   range_max    = STG_DEFAULT_LASER_MAXRANGE;
   fov          = STG_DEFAULT_LASER_FOV;
   sample_count = STG_DEFAULT_LASER_SAMPLES;  
   resolution = STG_DEFAULT_LASER_RESOLUTION;

   samples = g_new0( stg_laser_sample_t, sample_count );
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
       sample_count   = wf->ReadInt( id, "samples", sample_count );       
       samples = g_renew( stg_laser_sample_t, samples, sample_count );
       bzero( samples, sizeof(stg_laser_sample_t) * sample_count );

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
 }

 int laser_raytrace_match( StgModel* mod, StgModel* hitmod )
 {           
   // Ignore myself and things that are invisible to lasers
   return( (mod != hitmod) && (hitmod->LaserReturn() > 0) );
 }	

 void StgModelLaser::Update( void )
 {     
   StgModel::Update();

   // get the sensor's pose in global coords
   stg_pose_t gpose;
   GetGlobalPose( &gpose );

   //PRINT_DEBUG3( "laser origin %.2f %.2f %.2f", gpose.x, gpose.y, gpose.a );

   double sample_incr = fov / (double)(sample_count-1);

 #if TIMING
   struct timeval tv1, tv2;
   gettimeofday( &tv1, NULL );
 #endif

   assert(samples);
   
   double bearing = gpose.a - fov/2.0;

   for( unsigned int t=0; t<sample_count; t += resolution )
     {
       stg_meters_t hitrange, hitx, hity;      
       StgModel* hitmod;

       if( (hitmod = stg_first_model_on_ray( gpose.x, gpose.y, gpose.z,
					     bearing, 
					     range_max,
					     world->matrix,
					     PointToBearingRange,
					     laser_raytrace_match,
					     this,
					     &hitrange, &hitx, &hity )))
	 {
	   //printf( "hit model %s\n", hitmod->Token() );
	   samples[t].range = MAX( hitrange, range_min );
	   samples[t].hitpoint.x = hitx;
	   samples[t].hitpoint.y = hity;
	   samples[t].reflectance =
	     (hitmod->LaserReturn() >= LaserBright) ? 1.0 : 0.0;	  
	 }
       else
	 {
	   samples[t].range = range_max;
	   samples[t].hitpoint.x = gpose.x + samples[t].range * cos(bearing);
	   samples[t].hitpoint.y = gpose.y + samples[t].range * sin(bearing);
	   samples[t].reflectance = 0.0;
	 }           
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
  
  // start consuming power
  SetWatts( STG_DEFAULT_LASER_WATTS );
}

void StgModelLaser::Shutdown( void )
{ 

  PRINT_DEBUG( "laser shutdown" );
  
  // stop consuming power
  SetWatts( 0 );
  
  // clear the data - this will unrender it too
  g_free( samples );
  samples = NULL;
  
  StgModel::Shutdown();
}

void StgModelLaser::Print( char* prefix )
{
  StgModel::Print( prefix );
    
  printf( "\tRanges[ " );
  
  for( unsigned int i=0; i<sample_count; i++ )
    printf( "%.2f ", samples[i].range );
  puts( " ]" );

  printf( "\tReflectance[ " );
  
  for( unsigned int i=0; i<sample_count; i++ )
    printf( "%.2f ", samples[i].reflectance );
  puts( " ]" );
}




void StgModelLaser::DListData( void )
{
  // rebuild the graphics for this data
  glNewList( dl_data, GL_COMPILE );
  
  if( samples && sample_count )
    {
      
      glPushMatrix();
      glTranslatef( 0,0, geom.size.z/2.0 ); // shoot the laser beam out at the right height
      
      // pack the laser hit points into a vertex array for fast rendering
      static float* pts = NULL;
      pts = (float*)g_realloc( pts, 2 * (sample_count+1) * sizeof(float));
      
      pts[0] = 0.0;
      pts[1] = 0.0;
      
      push_color_rgba( 0, 0, 1, 0.5 );

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
      pop_color();

      glPointSize( 1.0 );

      glEnableClientState( GL_VERTEX_ARRAY );
      glVertexPointer( 2, GL_FLOAT, 0, pts );   


      if( world->win->show_alpha )
	{   
	  glDepthMask( GL_FALSE );

	  glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
	  push_color_rgba( 0, 0, 1, 0.1 );
	  glDrawArrays( GL_POLYGON, 0, sample_count+1 );
	  pop_color();

	  push_color_rgba( 0, 0, 0, 1.0 );
	  glDrawArrays( GL_POINTS, 0, sample_count+1 );
	  pop_color();

	  glDepthMask( GL_TRUE );
	}
      else
	{
	  glPolygonMode( GL_FRONT_AND_BACK, GL_LINES );
	  push_color_rgb( 0, 0, 1 );
	  glDrawArrays( GL_POLYGON, 0, sample_count+1 );
	  pop_color();
	}
	  
      glPopMatrix();      
    }
  
  glEndList();
}



