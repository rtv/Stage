///////////////////////////////////////////////////////////////////////////
//
// File: model_ranger.cc
// Author: Richard Vaughan
// Date: 10 June 2004
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/libstage/model_ranger.cc,v $
//  $Author: rtv $
//  $Revision: 1.4 $
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
   - per-transducer version of the sview property. Overrides the common setting.

*/

//#define DEBUG 1

#include "stage.hh"
#include "worldfile.hh"
#include "option.hh"
using namespace Stg;

static const stg_watts_t RANGER_WATTSPERSENSOR = 0.2;
static const Size RANGER_SIZE( 0.4, 0.4, 0.05 );
static const Size RANGER_TRANSDUCER_SIZE( 0.01, 0.04, 0.04 );
static const stg_meters_t RANGER_RANGEMAX = 5.0;
static const stg_meters_t RANGER_RANGEMIN = 0.0;
static const unsigned int RANGER_RAYCOUNT = 3;
static const unsigned int RANGER_SENSORCOUNT = 16;

static const char RANGER_COLOR[] = "gray75";
static const char RANGER_CONFIG_COLOR[] = "gray90";
static const char RANGER_GEOM_COLOR[] = "orange";

// static members
Option ModelRanger::showRangerData( "Ranger ranges", "show_ranger", "", true, NULL );
Option ModelRanger::showRangerTransducers( "Ranger transducers", "show_ranger_transducers", "", false, NULL );

ModelRanger::ModelRanger( World* world, 
								  Model* parent,
								  const std::string& type ) 
  : Model( world, parent, type )
{
  PRINT_DEBUG2( "Constructing ModelRanger %d (%s)\n", 
					 id, type );
  
  // Set up sensible defaults
  
  // assert that Update() is reentrant for this derived model
  thread_safe = true;
  
  this->SetColor( Color( RANGER_CONFIG_COLOR ) );
  
  // remove the polygon: ranger has no body
  this->ClearBlocks();
  
  this->SetGeom( Geom( Pose(), RANGER_SIZE ));
  
  // spread the transducers around the ranger's body
  double offset = std::min(geom.size.x, geom.size.y) / 2.0;
  
  sensors.resize( RANGER_SENSORCOUNT );
  
  // create default ranger config
  for( unsigned int c(0); c<sensors.size(); c++ )
    {
      sensors[c].pose.a = (2.0*M_PI)/sensors.size() * c;
      sensors[c].pose.x = offset * cos( sensors[c].pose.a );
      sensors[c].pose.y = offset * sin( sensors[c].pose.a );
      sensors[c].pose.z = geom.size.z / 2.0; // half way up

      sensors[c].size = RANGER_TRANSDUCER_SIZE;

      sensors[c].bounds_range.min = RANGER_RANGEMIN;
      sensors[c].bounds_range.max = RANGER_RANGEMAX;;
      sensors[c].fov = (2.0*M_PI)/(sensors.size()+1);
      sensors[c].ray_count = RANGER_RAYCOUNT;

		sensors[c].range = 0; // invalid range
    }
	
  RegisterOption( &showRangerData );
  RegisterOption( &showRangerTransducers );
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

void ModelRanger::Load( void )
{
  Model::Load();

  if( wf->PropertyExists( wf_entity, "scount" ) )
    {
      PRINT_DEBUG( "Loading ranger array" );

      // Load the geometry of a ranger array
      int sensor_count = wf->ReadInt( wf_entity, "scount", 0);
      assert( sensor_count > 0 );
		
      char key[256];
	 
		sensors.resize( sensor_count );

      Size common_size;
      common_size.x = wf->ReadTupleLength( wf_entity, "ssize", 0, RANGER_SIZE.x );
      common_size.y = wf->ReadTupleLength( wf_entity, "ssize", 1, RANGER_SIZE.y );

      double common_min = wf->ReadTupleLength( wf_entity, "sview", 0, RANGER_RANGEMIN );
      double common_max = wf->ReadTupleLength( wf_entity, "sview", 1, RANGER_RANGEMAX );
      double common_fov = wf->ReadTupleAngle( wf_entity, "sview", 2, M_PI / (double)sensor_count );

      int common_ray_count = wf->ReadInt( wf_entity, "sraycount", sensors[0].ray_count );
			
      // set all transducers with the common settings
      for( unsigned int i = 0; i < sensors.size(); i++)
		  {
			 sensors[i].size.x = common_size.x;
			 sensors[i].size.y = common_size.y;
			 sensors[i].bounds_range.min = common_min;
			 sensors[i].bounds_range.max = common_max;
			 sensors[i].fov = common_fov;
			 sensors[i].ray_count = common_ray_count;	  
			 sensors[i].range = 0.0;
		  }

      // TODO - do this properly, without the hard-coded defaults
      // allow individual configuration of transducers
      for( unsigned int i = 0; i < sensors.size(); i++)
		  {
			 snprintf(key, sizeof(key), "spose[%d]", i);
			 sensors[i].pose.x = wf->ReadTupleLength( wf_entity, key, 0, sensors[i].pose.x );
			 sensors[i].pose.y = wf->ReadTupleLength( wf_entity, key, 1, sensors[i].pose.y );
			 sensors[i].pose.z = 0.0;
			 sensors[i].pose.a = wf->ReadTupleAngle( wf_entity, key, 2, sensors[i].pose.a );
					
			 snprintf(key, sizeof(key), "spose3[%d]", i);
			 sensors[i].pose.x = wf->ReadTupleLength( wf_entity, key, 0, sensors[i].pose.x );
			 sensors[i].pose.y = wf->ReadTupleLength( wf_entity, key, 1, sensors[i].pose.y );
			 sensors[i].pose.z = wf->ReadTupleLength( wf_entity, key, 2, sensors[i].pose.z );
			 sensors[i].pose.a = wf->ReadTupleAngle( wf_entity, key, 3, sensors[i].pose.a );
					
			 /* 	  snprintf(key, sizeof(key), "ssize[%d]", i); */
			 /* 	  sensors[i].size.x = wf->ReadTuplelength(mod->id, key, 0, 0.01); */
			 /* 	  sensors[i].size.y = wf->ReadTuplelength(mod->id, key, 1, 0.05); */
					
			 /* 	  snprintf(key, sizeof(key), "sview[%d]", i); */
			 /* 	  sensors[i].bounds_range.min = */
			 /* 	    wf->ReadTuplelength(mod->id, key, 0, 0); */
			 /* 	  sensors[i].bounds_range.max =   // set up sensible defaults */
					
			 /* 	    wf->ReadTuplelength(mod->id, key, 1, 5.0); */
			 /* 	  sensors[i].fov */
			 /* 	    = DTOR(wf->ReadTupleangle(mod->id, key, 2, 5.0 )); */
					
					
			 /* 	  sensors[i].ray_count = common_ray_count; */
					
		  }
			
      PRINT_DEBUG1( "loaded %d ranger configs", (int)sensor_count );	  
    }
}

static bool ranger_match( Model* candidate, 
								  Model* finder, 
								  const void* dummy )
{
  // Ignore myself, my children, and my ancestors.
  return( candidate->vis.ranger_return && !candidate->IsRelated( finder ) );
}	

void ModelRanger::Update( void )
{     
  Model::Update();

  if( subs < 1 )
	 return;

  if( sensors.size() < 1 )
	 return;

  // raytrace new range data for all sensors
	FOR_EACH( it, sensors )
    {
			Sensor& s = *it;
			
      // TODO - reinstate multi-ray rangers
      //for( int r=0; r<sensors[t].ray_count; r++ )
      //{	  
      stg_raytrace_result_t ray = Raytrace( s.pose,
																						s.bounds_range.max,
																						ranger_match,
																						NULL );
			
      s.range = std::max( ray.range, s.bounds_range.min );
    }   
}

void ModelRanger::Print( char* prefix )
{
  Model::Print( prefix );

  printf( "\tRanges[ " );

  for( unsigned int i=0; i<sensors.size(); i++ )
    printf( "%.2f ", sensors[i].range );
  puts( " ]" );
}

void ModelRanger::DataVisualize( Camera* cam )
{
  if( sensors.size() < 1 )
    return;
	
  if( showRangerTransducers )
    {
      glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
      PushColor( 0,0,0,1 );
			
      for( unsigned int s=0; s<sensors.size(); s++ ) 
		  { 
			 Sensor& rngr = sensors[s];

			 glPointSize( 4 );
			 glBegin( GL_POINTS );
			 glVertex3f( rngr.pose.x, rngr.pose.y, rngr.pose.z );
			 glEnd();

			 char buf[8];
			 snprintf( buf, 8, "%d", s );
			 Gl::draw_string( rngr.pose.x, rngr.pose.y, rngr.pose.z, buf );
			 
		  }
      PopColor();
    }
  
  if ( !showRangerData )
    return;
	
  // if sequential models have the same number of sensors, this is
  // fast as it will probably not use a system call
  static std::vector<GLfloat> pts;
  pts.resize( 9 * sensors.size() );
	
  // calculate a triangle for each non-zero sensor range
  for( unsigned int s=0; s<sensors.size(); s++ ) 
    { 
		Sensor& rngr = sensors[s];
			
      if( rngr.range > 0.0 ) 
		  { 
			 // sensor FOV 
			 double sidelen = rngr.range;
			 double da = rngr.fov/2.0;
					
			 unsigned int index = s*9;
			 pts[index+0] = rngr.pose.x;
			 pts[index+1] = rngr.pose.y;
			 pts[index+2] = rngr.pose.z;
					
			 pts[index+3] = rngr.pose.x + sidelen*cos(rngr.pose.a - da );
			 pts[index+4] = rngr.pose.y + sidelen*sin(rngr.pose.a - da );
			 pts[index+5] = rngr.pose.z;
					
			 pts[index+6] = rngr.pose.x + sidelen*cos(rngr.pose.a + da );
			 pts[index+7] = rngr.pose.y + sidelen*sin(rngr.pose.a + da );
			 pts[index+8] = rngr.pose.z;
		  }
    }
	
  // draw the filled triangles in transparent pale green
  glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
  glDepthMask( GL_FALSE );
  PushColor( 0, 1, 0, 0.1 );     
  glVertexPointer( 3, GL_FLOAT, 0, &pts[0] );
  glDrawArrays( GL_TRIANGLES, 0, 3 * sensors.size() );

  // restore state 
  glDepthMask( GL_TRUE );
  PopColor();
}


