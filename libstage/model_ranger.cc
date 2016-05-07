///////////////////////////////////////////////////////////////////////////
//
// File: model_ranger.cc
// Author: Richard Vaughan
// Date: 10 June 2004
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/libstage/model_ranger.cc,v $
//  $Author: rtv $
//  $Revision$
//
///////////////////////////////////////////////////////////////////////////

/**
   @ingroup model
   @defgroup model_ranger Ranger model 
   The ranger model simulates an array of sonar or infra-red (IR) range sensors.

   API: Stg::ModelRanger

   <h2>Worldfile properties</h2>

   @par Summary and default values

   @verbatim
   ranger
   (
   # ranger-specific properties

   sensor (
   pose [ x y z a ]
   size [  x y z ]
   fov a
   range [min max]
   noise [range_const range_prop angular]
   )

   # generic model properties with non-default values
   watts 2.0
   color_rgba [ 0 1 0 0.15 ]
   )

   @endverbatim

   @par Notes

   The ranger model allows configuration of the pose, size and view parameters of each transducer seperately (using spose[index], ssize[index] and sview[index]). However, most users will set a common size and view (using ssize and sview), and just specify individual transducer poses.

   @par Details
 
   - pose[\<transducer index\>] [ x:<float> y:<float> theta:<float> ]\n
   pose of the transducer relative to its parent.
   - ssize [float float]
   - [x y] 
   - size in meters. Has no effect on the data, but controls how the sensor looks in the Stage window.
   - size[\<transducer index\>] [float float]
   -  per-transducer version of the ssize property. Overrides the common setting.
   - sview [float float float]
   - [range_min range_max fov] 
   - minimum range and maximum range in meters, field of view angle in degrees. Currently fov has no effect on the sensor model, other than being shown in the confgiuration graphic for the ranger device.
   - noise [range_const range_prop angular]
   - noise for range readings: constant noise in meters, proportional noise, angular noise in degrees
   - sview[\<transducer index\>] [float float float]
   - per-transducer version of the sview property. Overrides the common setting.

*/

//#define DEBUG 1

#include "stage.hh"
#include "worldfile.hh"
#include "option.hh"

using namespace Stg;

static const watts_t RANGER_WATTSPERSENSOR = 0.2;
static const Stg::Size RANGER_SIZE( 0.15, 0.15, 0.2 ); // SICK LMS size

//static const Color RANGER_COLOR( 0,0,1 );
static const Color RANGER_CONFIG_COLOR( 0,0,0.5 );
//static const Color RANGER_GEOM_COLOR( 1,0,1 );

// static members
Option ModelRanger::Vis::showTransducers( "Ranger transducers", "show_ranger_transducers", "", false, NULL );
Option ModelRanger::Vis::showArea( "Ranger area", "show_ranger_ranges", "", true, NULL );
Option ModelRanger::Vis::showStrikes( "Ranger strikes", "show_ranger_strikes", "", false, NULL );
Option ModelRanger::Vis::showFov( "Ranger FOV", "show_ranger_fov", "", false, NULL );
//Option ModelRanger::Vis::showBeams( "Ranger beams", "show_ranger_beams", "", false, NULL );


ModelRanger::ModelRanger( World* world, 
			  Model* parent,
			  const std::string& type ) 
  : Model( world, parent, type ),
    vis( world ) 
{
  PRINT_DEBUG2( "Constructing ModelRanger %d (%s)\n", 
		id, type );
  
  // Set up sensible defaults
  
  // assert that Update() is reentrant for this derived model
  thread_safe = true;
  
  this->SetColor( RANGER_CONFIG_COLOR );
  
  // remove the polygon: ranger has no body
  this->ClearBlocks();
  
  this->SetGeom( Geom( Pose(), RANGER_SIZE ));  
	
  AddVisualizer( &vis, true );  
}

ModelRanger::~ModelRanger()
{
}

void ModelRanger::Startup( void )
{
  Model::Startup();
  this->SetWatts( RANGER_WATTSPERSENSOR * sensors.size() );
}


void ModelRanger::Shutdown( void )
{
  PRINT_DEBUG( "ranger shutdown" );

  this->SetWatts( 0 );

  Model::Shutdown();
}


void ModelRanger::LoadSensor( Worldfile* wf, int entity )
{
  Sensor s;
  s.Load( wf, entity );
  sensors.push_back(s); 
}


void ModelRanger::Sensor::Load( Worldfile* wf, int entity )
{
  pose.Load( wf, entity, "pose" );
  size.Load( wf, entity, "size" );
  range.Load( wf, entity, "range" );
  fov = wf->ReadAngle( entity, "fov", fov );
  sample_count = wf->ReadInt( entity, "samples", sample_count );

  wf->ReadTuple( entity, "noise", 0, 3, "lfa", &range_noise_const, &range_noise, &angle_noise);
  color.Load( wf, entity );
}


static bool ranger_match( Model* hit, 
			  Model* finder,
			  const void* dummy )
{
  (void)dummy; // avoid warning about unused var
  
  // Ignore the model that's looking and things that are invisible to
  // rangers 
  
  // small optimization to avoid recursive Model::IsRelated call in common cases
  if( (hit == finder->Parent()) || (hit == finder) ) return false;
  
  return( (!hit->IsRelated( finder )) && (sgn(hit->vis.ranger_return) != -1 ) );
}	

// Returns random numbers in range [-1.0, 1.0)
double simpleNoise()
{
	return 2*(rand()%1000 * 0.001 - 0.5);
}

#define TWO_PI (M_PI+M_PI)
// Returns gaussian noise
// taken from http://en.wikipedia.org/wiki/Box%E2%80%93Muller_transform
template<class Scalar_t> Scalar_t generateGaussianNoise(Scalar_t variance)
{
	static bool haveSpare = false;
	static Scalar_t rand1, rand2;

	if(haveSpare)
	{
		haveSpare = false;
		return sqrt(variance * rand1) * sin(rand2);
	}

	haveSpare = true;

	rand1 = rand() / ((Scalar_t) RAND_MAX);
	if(rand1 < 1e-100) rand1 = 1e-100;
	rand1 = -2 * log(rand1);
	rand2 = (rand() / ((Scalar_t) RAND_MAX)) * TWO_PI;

	return sqrt(variance * rand1) * cos(rand2);
}


void ModelRanger::Update( void )
{     
  // raytrace new range data for all sensors
  FOR_EACH( it, sensors )
    it->Update( this );
  
  Model::Update();
}

void ModelRanger::Sensor::Update( ModelRanger* mod )
{
  // these sizes change very rarely, so this is very cheap
  ranges.resize( sample_count );
  intensities.resize( sample_count );
  bearings.resize( sample_count );

  //printf( "update sensor, has ranges size %u\n", (unsigned int)ranges.size() );
  // make the first and last rays exactly at the extremes of the FOV
  const double sample_incr( fov / std::max(sample_count-1, (unsigned int)1) );
  
  // find the global origin of our first emmitted ray
  const double start_angle = (sample_count > 1 ? -fov/2.0 : 0.0);

  // find the origin and heading of the first ray
  Pose rayorg(pose);
  rayorg.a += start_angle;
  rayorg.z += size.z/2.0;
  rayorg = mod->LocalToGlobal(rayorg);
  
  // set up a ray to trace
  Ray ray( mod, rayorg, range.max, ranger_match, NULL, true );

  // trace the ray, incrementing its heading for each sample
  for( size_t t(0); t<sample_count; t++ )
    {
	  float savedAngle = ray.origin.a;
	  float distortedAngle = ray.origin.a + sample_incr*angle_noise*simpleNoise()*0.5;
	  ray.origin.a = distortedAngle;
      const RaytraceResult res = mod->world->Raytrace( ray);
      ray.origin.a = savedAngle;

      /// Apply noise only if it is in valid range
      if(res.range < this->range.max)
    	  ranges[t] = res.range + res.range*range_noise*simpleNoise() + generateGaussianNoise(range_noise_const);
      else
    	  ranges[t] = res.range;

      intensities[t] = res.mod ? res.mod->vis.ranger_return : 0.0;
      bearings[t] = start_angle + ((double)t) * sample_incr;
		
      // point the ray to the next angle:
      ray.origin.a += sample_incr;			
    }
}

std::string ModelRanger::Sensor::String() const
{
  char buf[256];
  snprintf( buf, 256, "[ samples %u, range [%.2f %.2f] fov %.2f color [%.2f %.2f %.2f %.2f]", 
	    sample_count, range.min, range.max, fov, color.r, color.g, color.b, color.a );
  return( std::string( buf ) );
}

void ModelRanger::Sensor::Visualize( ModelRanger::Vis* vis, ModelRanger* rgr ) const
{
  size_t sample_count( this->sample_count );
	
  //glTranslatef( 0,0, ranger->GetGeom().size.z/2.0 ); // shoot the ranger beam out at the right height
			
  // pack the ranger hit points into a vertex array for fast rendering
  GLfloat pts[2*(sample_count+1)];
  glVertexPointer( 2, GL_FLOAT, 0, &pts[0] );       
	
  pts[0] = 0.0;
  pts[1] = 0.0;
  
  glDepthMask( GL_FALSE );
  glPointSize( 2 );
	
  glPushMatrix();

  Gl::pose_shift( pose );

  if( vis->showTransducers )
    {
      // draw the sensor body as a rectangle in a darker version of the body color
      // Color col( rgr->color );
      // col.r /= 3.0;
      // col.g /= 3.0;
      // col.b /= 3.0;
      rgr->PushColor( color );

      glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );			
      glRectf( -size.x/2.0, -size.y/2.0, size.x/2.0, size.y/2.0 );
      rgr->PopColor();
    }

  if( vis->showFov )
    {
      for( size_t s(0); s<sample_count; s++ )
	{
	  double ray_angle((s * (fov / (sample_count-1))) - fov/2.0);
	  pts[2*s+2] = (float)(range.max * cos(ray_angle) );
	  pts[2*s+3] = (float)(range.max * sin(ray_angle) );			 
	}
			
      glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );

      Color c = color;
      c.a = 0.5;
      rgr->PushColor( c );		
      glDrawArrays( GL_POLYGON, 0, sample_count+1 );
      rgr->PopColor();
    }			 	

  if( ranges.size()  ) // if we have some data
    {
      rgr->PushColor( color );
      glPolygonMode( GL_FRONT, GL_FILL );			
      
      if( sample_count == 1 )
	{
	  // only one sample, so we fake up some beam width for beauty
	  const double sidelen = ranges[0];
	  const double da = fov/2.0;
	  
	  sample_count = 3;
	  
	  pts[2] = sidelen*cos(-da );
	  pts[3] = sidelen*sin(-da );
	  
	  pts[4] = sidelen*cos(+da );
	  pts[5] = sidelen*sin(+da );
	}	
      else
	{
	  for( size_t s(0); s<sample_count+1; s++ )
	    {
	      double ray_angle = (s * (fov / (sample_count-1))) - fov/2.0;
	      pts[2*s+2] = (float)(ranges[s] * cos(ray_angle) );
	      pts[2*s+3] = (float)(ranges[s] * sin(ray_angle) );
	    }
	}
      
      if( vis->showArea )
	{
	  if( sample_count > 1 )
	    // draw the filled polygon in transparent blue
	    glDrawArrays( GL_POLYGON, 0, sample_count );
	}
      
      glDepthMask( GL_TRUE );
      
      if( vis->showStrikes )
	{
	  // TODO - paint the stike point in a color based on intensity
	  // 			// if the sample is unusually bright, draw a little blob
	  // 			if( intensities[s] > 0.0 )
	  // 				{
	  // 					// this happens rarely so we can do it in immediate mode
	  // 					glBegin( GL_POINTS );
	  // 					glVertex2f( pts[2*s+2], pts[2*s+3] );
	  // 					glEnd();
	  // 				}			 
	  
	  // draw the beam strike points
	  //c.a = 0.8;
	  rgr->PushColor( Color::blue );
	  glDrawArrays( GL_POINTS, 0, sample_count+1 );
	  rgr->PopColor();
	}
      
      // if( vis->showBeams )
      // 	{
      // 	  Color c = color;
	  
      // 	  // darker version of the same color
      // 	  c.r /= 2.0;
      // 	  c.g /= 2.0;
      // 	  c.b /= 2.0;
      // 	  c.a = 1.0;
	  
      // 	  rgr->PushColor( c );		
      // 	  glBegin( GL_LINES );
	  
      // 	  for( size_t s(0); s<sample_count; s++ )
      // 	    {    
      // 	      glVertex2f( 0,0 );
      // 	      double ray_angle( sample_count == 1 ? 0 : (s * (fov / (sample_count-1))) - fov/2.0 );
      // 	      glVertex2f( ranges[s] * cos(ray_angle), 
      // 			  ranges[s] * sin(ray_angle) );
	      
      // 	    }
      // 	  glEnd();
	
      // 	  rgr->PopColor();
      // 	}	

      rgr->PopColor();
    }
  
  glPopMatrix();
}
	
void ModelRanger::Print( char* prefix ) const
{
  Model::Print( prefix );
	
  printf( "\tRanges " );
  for( size_t i(0); i<sensors.size(); i++ )
    {
      printf( "[ " );
      for( size_t j(0); j<sensors[i].ranges.size(); j++ )
	printf( "%.2f ", sensors[i].ranges[j] );
			
      printf( " ]" );
    }
	
  printf( "\n\tIntensities " );
  for( size_t i(0); i<sensors.size(); i++ )
    {
      printf( "[ " );
      for( size_t j(0); j<sensors[i].intensities.size(); j++ )
	printf( "%.2f ", sensors[i].intensities[j] );
			
      printf( " ]" );
    }
  puts("");
}



// VIS -------------------------------------------------------------------

ModelRanger::Vis::Vis( World* world ) 
  : Visualizer( "Ranger", "ranger_vis" )
{
  world->RegisterOption( &showArea );
  world->RegisterOption( &showStrikes );
  world->RegisterOption( &showFov );
  //world->RegisterOption( &showBeams );		  
  world->RegisterOption( &showTransducers );
}

void ModelRanger::Vis::Visualize( Model* mod, Camera* cam ) 
{
  (void)cam; // avoid warning about unused var

  ModelRanger* ranger( dynamic_cast<ModelRanger*>(mod) );

  const std::vector<Sensor>& sensors( ranger->GetSensors() );    
	
  FOR_EACH( it, sensors )
    it->Visualize( this, ranger );
	
  const size_t sensor_count = sensors.size();

  if( showTransducers )
    {
      glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
      ranger->PushColor( 0,0,0,1 );
			
      for( size_t s(0); s<sensor_count; s++ ) 
	{ 
	  const Sensor& rngr(sensors[s]);
					
	  glPointSize( 4 );
	  glBegin( GL_POINTS );
	  glVertex3f( rngr.pose.x, rngr.pose.y, rngr.pose.z );
	  glEnd();
					
	  char buf[8];
	  snprintf( buf, 8, "%d", (int)s );
	  Gl::draw_string( rngr.pose.x, rngr.pose.y, rngr.pose.z, buf );
					
	}
      ranger->PopColor();
    }

}

