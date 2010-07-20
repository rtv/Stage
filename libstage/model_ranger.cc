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
   # ranger properties
   scount 16
   spose[0] [? ? ?]
   spose[1] [? ? ?]
   spose[2] [? ? ?]
   spose[3] [? ? ?]
   spose[4] [? ? ?]
   spose[5] [? ? ?]
   spose[6] [? ? ?]
   spose[7] [? ? ?]
   spose[8] [? ? ?]
   spose[9] [? ? ?]
   spose[10] [? ? ?]
   spose[11] [? ? ?]
   spose[12] [? ? ?]
   spose[13] [? ? ?]
   spose[14] [? ? ?]
   spose[15] [? ? ?]

   ssize [0.01 0.03]
   sview [0.0 5.0 5.0]

   # model properties
   watts 2.0
   )
   @endverbatim

   @par Notes

   The ranger model allows configuration of the pose, size and view parameters of each transducer seperately (using spose[index], ssize[index] and sview[index]). However, most users will set a common size and view (using ssize and sview), and just specify individual transducer poses.

   @par Details
 
   - scount <int>\n
   the number of range transducers
   - spose[\<transducer index\>] [ x:<float> y:<float> theta:<float> ]\n
   pose of the transducer relative to its parent.
   - ssize [float float]
   - [x y] 
   - size in meters. Has no effect on the data, but controls how the sensor looks in the Stage window.
   - ssize[\<transducer index\>] [float float]
   - per-transducer version of the ssize property. Overrides the common setting.
   - sview [float float float]
   - [range_min range_max fov] 
   - minimum range and maximum range in meters, field of view angle in degrees. Currently fov has no effect on the sensor model, other than being shown in the confgiuration graphic for the ranger device.
   - sview[\<transducer index\>] [float float float]
s   - per-transducer version of the sview property. Overrides the common setting.

*/

//#define DEBUG 1

#include "stage.hh"
#include "worldfile.hh"
#include "option.hh"
using namespace Stg;

static const watts_t RANGER_WATTSPERSENSOR = 0.2;
static const Size RANGER_SIZE( 0.15, 0.15, 0.2 ); // SICK LMS size

//static const Color RANGER_COLOR( 0,0,1 );
static const Color RANGER_CONFIG_COLOR( 0,0,0.5 );
//static const Color RANGER_GEOM_COLOR( 1,0,1 );

// static members
Option ModelRanger::Vis::showData( "Ranger ranges", "show_ranger", "", true, NULL );
Option ModelRanger::Vis::showTransducers( "Ranger transducers", "show_ranger_transducers", "", false, NULL );
Option ModelRanger::Vis::showArea( "Ranger scans", "show_ranger", "", true, NULL );
Option ModelRanger::Vis::showStrikes( "Ranger strikes", "show_ranger_strikes", "", false, NULL );
Option ModelRanger::Vis::showFov( "Ranger FOV", "show_ranger_fov", "", false, NULL );
Option ModelRanger::Vis::showBeams( "Ranger beams", "show_ranger_beams", "", false, NULL );


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
	
	//  sensors.resize( RANGER_SENSORCOUNT );
  
//   for( unsigned int c(0); c<sensors.size(); c++ )
//     {
// 			// change a few of the defaults now we know how many sensors
// 			// there are and where they are located
// 			// (spread the transducers around the ranger's body)
// 			Sensor::Cfg& cfg = sensors[c].cfg;
//       cfg.pose.a = (2.0*M_PI)/sensors.size() * c;
//       cfg.pose.z = geom.size.z / 2.0; // half way up 
//       cfg.fov = (2.0*M_PI)/(sensors.size()+1);
//     }

	AddVisualizer( &vis, true );  
}

ModelRanger::~ModelRanger()
{
}

void ModelRanger::Startup( void )
{
  Model::Startup();

  PRINT_DEBUG( "ranger startup" );

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
	//static int c=0;
	// printf( "ranger %s loading sensor %d\n", token.c_str(), c++ );
	
	Sensor s;
	s.cfg.pose.Load( wf, entity, "pose" );
	s.cfg.size.Load( wf, entity, "size" );
	s.cfg.range.Load( wf, entity, "range" );
	s.cfg.col.Load( wf, entity );		
	s.cfg.fov = wf->ReadAngle( entity, "fov", s.cfg.fov );
	s.cfg.sample_count = wf->ReadInt( entity, "samples", s.cfg.sample_count );
	sensors.push_back(s);
}

void ModelRanger::Load( void )
{
  Model::Load();
}

static bool ranger_match( Model* hit, 
													Model* finder,
													const void* dummy )
{
  (void)dummy; // avoid warning about unused var
	
  // Ignore the model that's looking and things that are invisible to
  // rangers 
  return( (!hit->IsRelated( finder )) && (sgn(hit->vis.ranger_return) != -1 ) );
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
	//printf( "update sensor\n" );

	samples.resize( cfg.sample_count );
	
	double bearing( -cfg.fov/2.0 );
	// make the first and last rays exactly at the extremes of the FOV
	double sample_incr( cfg.fov / std::max(cfg.sample_count-1, (unsigned int)1) );
	
  // find the global origin of our first emmitted ray
  Pose rayorg( cfg.pose );//mod->GetPose() );
  rayorg.z += cfg.size.z/2.0;
  rayorg = mod->LocalToGlobal(rayorg);
  
  // set up a ray to trace
  Ray ray( mod, rayorg, cfg.range.max, ranger_match, NULL, true );
  
	World* world = mod->GetWorld();

  // trace the ray, incrementing its heading for each sample
  for( unsigned int t(0); t<cfg.sample_count; t++ )
    {
			const RaytraceResult& r ( world->Raytrace( ray ) );
			samples[t].range = r.range;

			samples[t].reflectance = r.mod ? r.mod->vis.ranger_return : 0.0;
			
			// point the ray to the next angle
			ray.origin.a += sample_incr;			

			//printf( "ranger %s sensor %p pose %s sample %d range %.2f ref %.2f\n",
			//			mod->Token(), 
			//			this, 
			//			cfg.pose.String().c_str(),
			//			t, 
			//			samples[t].range, 
			//			samples[t].reflectance );
    }
}

void ModelRanger::Sensor::Visualize( ModelRanger::Vis* vis, ModelRanger* rgr ) const
{
	const size_t sample_count( samples.size() );
	
	glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );			
	//glTranslatef( 0,0, ranger->GetGeom().size.z/2.0 ); // shoot the ranger beam out at the right height
			
	// pack the ranger hit points into a vertex array for fast rendering
	static std::vector<GLfloat> pts;
	pts.resize( 2 * (sample_count+1) );
	
	pts[0] = 0.0;
	pts[1] = 0.0;
	
	rgr->PushColor( 1, 0, 0, 0.5 );
	glDepthMask( GL_FALSE );
	glPointSize( 2 );
	
	for( unsigned int s(0); s<sample_count; s++ )
		{
			double ray_angle = (s * (cfg.fov / (sample_count-1))) - cfg.fov/2.0;
			pts[2*s+2] = (float)(samples[s].range * cos(ray_angle) );
			pts[2*s+3] = (float)(samples[s].range * sin(ray_angle) );
			
			// if the sample is unusually bright, draw a little blob
			if( samples[s].reflectance > 0 )
				{
					// this happens rarely so we can do it in immediate mode
					glBegin( GL_POINTS );
					glVertex2f( pts[2*s+2], pts[2*s+3] );
					glEnd();
				}			 
		}
	
	glVertexPointer( 2, GL_FLOAT, 0, &pts[0] );       
	
	rgr->PopColor();
	
	if( vis->showArea )
		{
			// draw the filled polygon in transparent blue
			rgr->PushColor( 0, 0, 1, 0.1 );		
			glDrawArrays( GL_POLYGON, 0, sample_count+1 );
			rgr->PopColor();  
		}
	
	glDepthMask( GL_TRUE );
	
	if( vis->showStrikes )
		{
			// draw the beam strike points
			rgr->PushColor( 0, 0, 1, 0.8 );
			glDrawArrays( GL_POINTS, 0, sample_count+1 );
			rgr->PopColor();
		}
	
	if( vis->showFov )
		{
			for( unsigned int s(0); s<sample_count; s++ )
				{
					double ray_angle((s * (cfg.fov / (sample_count-1))) - cfg.fov/2.0);
					pts[2*s+2] = (float)(cfg.range.max * cos(ray_angle) );
					pts[2*s+3] = (float)(cfg.range.max * sin(ray_angle) );			 
				}
			
			glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
			rgr->PushColor( 0, 0, 1, 0.5 );		
			glDrawArrays( GL_POLYGON, 0, sample_count+1 );
			rgr->PopColor();
		}			 
	
	if( vis->showBeams )
		{
			rgr->PushColor( 0, 0, 1, 0.5 );		
			glBegin( GL_LINES );
			
			for( unsigned int s(0); s<sample_count; s++ )
				{
					
					glVertex2f( 0,0 );
					double ray_angle((s * (cfg.fov / (sample_count-1))) - cfg.fov/2.0);
					glVertex2f( samples[s].range * cos(ray_angle), 
											samples[s].range * sin(ray_angle) );
					
				}
			glEnd();
			rgr->PopColor();
		}	
	
}
	
	void ModelRanger::Print( char* prefix ) const
	{
		Model::Print( prefix );
		
  printf( "\tRanges " );
  for( unsigned int i=0; i<sensors.size(); i++ )
		{
			printf( "[ " );
			for( unsigned int j=0; j<sensors[i].samples.size(); j++ )
				printf( "%.2f ", sensors[i].samples[j].range );
			
			printf( " ]" );
		}
	
  printf( "\n\tReflectance " );
  for( unsigned int i=0; i<sensors.size(); i++ )
		{
			printf( "[ " );
			for( unsigned int j=0; j<sensors[i].samples.size(); j++ )
				printf( "%.2f ", sensors[i].samples[j].reflectance );
			
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
  world->RegisterOption( &showBeams );		  
  world->RegisterOption( &showData );
  world->RegisterOption( &showTransducers );
}

void ModelRanger::Vis::Visualize( Model* mod, Camera* cam ) 
{
  (void)cam; // avoid warning about unused var

  ModelRanger* ranger( dynamic_cast<ModelRanger*>(mod) );

  const std::vector<Sensor>& sensors( ranger->GetSensors() );    
	
	//glPushMatrix();
	
	//FOR_EACH( it, sensors )
	//		it->Visualize( this, ranger );

	//glPopMatrix();
				
	const unsigned int sensor_count = sensors.size();

	if( showData )
		{			
			glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
			glDepthMask( GL_FALSE );
			
			// figure out how many triangles we'll need			
// 			unsigned int tri = 0;			
// 			FOR_EACH( it, sensors )
// 				tri += it->samples.size();

// 			std::vector<GLfloat> pts( tri * 9 );
			
// 			//	printf( "triangles %u\n", tri );
			
// 			unsigned int offset = 0;

			// calculate a triangle for each non-zero sensor range
			for( unsigned int s=0; s<sensor_count; s++ ) 
				{ 
					const Sensor& sensor = sensors[s];																
					const unsigned int sample_count = sensor.samples.size();

					ranger->PushColor( sensor.cfg.col );     
					
					//printf( "s = %u samples %u", s, sample_count );
					
					// sensor FOV 
					double da = sensor.cfg.fov/2.0;

					std::vector<GLfloat> pts( sample_count * 9 );
					
					// calculate a triangle for each non-zero sensor range
					for( unsigned int t=0; t<sample_count; t++ ) 						
						{ 								
							double sidelen = sensor.samples[t].range;
							
							//unsigned int index = offset + t*9;							
							unsigned int index = t*9;
							
							//printf( "filling index %u\n", index );

							pts[index+0] = sensor.cfg.pose.x;
							pts[index+1] = sensor.cfg.pose.y;
							pts[index+2] = sensor.cfg.pose.z;
							
							pts[index+3] = sensor.cfg.pose.x + sidelen*cos(sensor.cfg.pose.a - da );
							pts[index+4] = sensor.cfg.pose.y + sidelen*sin(sensor.cfg.pose.a - da );
							pts[index+5] = sensor.cfg.pose.z;
							
							pts[index+6] = sensor.cfg.pose.x + sidelen*cos(sensor.cfg.pose.a + da );
							pts[index+7] = sensor.cfg.pose.y + sidelen*sin(sensor.cfg.pose.a + da );
							pts[index+8] = sensor.cfg.pose.z;
						}
										
					// draw the filled triangles in transparent pale green
					glVertexPointer( 3, GL_FLOAT, 0, &pts[0] );
					glDrawArrays( GL_TRIANGLES, 0, 3 * sample_count );
					
					ranger->PopColor();
					
			//offset += 9 * sample_count;
				}
			
			// draw the filled triangles in transparent pale green
			//glVertexPointer( 3, GL_FLOAT, 0, &pts[0] );
			//glDrawArrays( GL_TRIANGLES, 0, 3 * tri );
			

			// restore state 
			glDepthMask( GL_TRUE );
		}			
	
	if( showTransducers )
		{
			glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
			ranger->PushColor( 0,0,0,1 );
			
			for( unsigned int s=0; s<sensor_count; s++ ) 
				{ 
					const Sensor& rngr = sensors[s];
					
					glPointSize( 4 );
					glBegin( GL_POINTS );
					glVertex3f( rngr.cfg.pose.x, rngr.cfg.pose.y, rngr.cfg.pose.z );
					glEnd();
					
					char buf[8];
					snprintf( buf, 8, "%d", s );
					Gl::draw_string( rngr.cfg.pose.x, rngr.cfg.pose.y, rngr.cfg.pose.z, buf );
					
				}
			ranger->PopColor();
		}

}

